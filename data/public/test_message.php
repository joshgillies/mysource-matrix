<?php
/**
 * This test message will return one of the following HTTP codes
 * 200 | Application is fully functional
 * 500 | Database server is not available (only when using checkdb=1)
 * 501 | File replication is not working, DB replication working (only when using checkreplication=1)
 * 502 | File replication working, DB replication not working (only when using checkreplication=1)
 * 503 | File replication not working, DB replication not working (only when using checkreplication=1)
 *
 * If interrogate=1 is passed to the script, it performs different checks.
 * It goes through the dsn's from the main.inc/db.inc (depending on version)
 * and checks how long it takes to run a query.
 * If it's a postgres system, it will also check for slon and
 * the name of the slon schema will be shown along with it's replication lag.
 *
 * Any error = soft failure, (output continues), with an http status of 500
 *
 * A value of -1 within the DSN tests indicates a failure.
 */

/**
 * Check it's a valid request first.
 * The old check was for 'checkdb' or 'checkreplication' in the query string
 * The new check is for 'interrogate' in the query string and it's set to 1
 */
$valid_request = FALSE;
// check the legacy use first
if (isset($_GET['checkdb']) || isset($_GET['checkreplication'])) {
	$valid_request = TRUE;
}

// check new use
if (isset($_GET['interrogate']) && $_GET['interrogate'] == 1) {
	$valid_request = TRUE;
}

$LOCK_FILE = dirname(__FILE__) . '/.test_message.lock';
// not a valid request? exit!
if (!$valid_request) {
	header('HTTP/1.0 200 OK');
	echo 'the return code was 200';
	exit;
}

if (is_file($LOCK_FILE)) {
	# if the file was last modified < 55 secs ago, then show a 500 error.
	# we want to limit it to 1 request every minute
	# (55 secs used so we have a little leeway in case a request comes in slightly early)
	$one_min_ago = time() - 55;
	if (filemtime($LOCK_FILE) > $one_min_ago) {
		header('HTTP/1.0 500 Internal Server Error');
		exit;
	}
}

touch($LOCK_FILE);

/**
 * Check for legacy use first.
 */
if (!empty($_GET['checkdb']) || !empty($_GET['checkreplication'])) {

	$return_code = '200';

	require_once '../../core/include/init.inc';
	require SQ_SYSTEM_ROOT . '/data/private/conf/db.inc';

	if (isset($_GET['checkdb']) && $_GET['checkdb'] == 1) {
		// we are checking for database availablity

		$error = FALSE;
		try {
			$dsn=NULL;
			MatrixDAL::dbConnect($dsn, 'db');
		} catch (Exception $e) {
			$return_code = '500';
			$error = TRUE;
		}

		if (!$error) {
			$query = MatrixDAL::preparePdoQuery('SELECT assetid FROM sq_ast WHERE assetid = \'1\'');
			try {
				$res = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				$return_code = '500';
			}
		}
	}

	if (isset($_GET['checkreplication']) && $_GET['checkreplication'] == 1) {
		// we are checking that replication is working
		$file_system_okay = TRUE;

		error_reporting(15);
		// find the last line in the rsync.log file and parse it for an error
		// we need to do a bit of extra work so that we dont read the whole file
		// into memory as its likely to be big.
		$fp = fopen('/var/log/rsync.log', 'r');

		if ($fp) {
			$info = fstat($fp);
			// if the modified time of the file is greater than 3 minutes
			// then rsyncing has not occured

			if (time() > $info['mtime'] + (3 * 60)) {
				$file_system_okay = FALSE;
			} else {
				// set the file pointer to the end of the file
				fseek($fp, 0, SEEK_END);
				$c = null;
				// find the beginning of the last line
				while ($c != "\n") {
					$c = fgetc($fp);
					fseek($fp, ftell($fp) - 2);
				}
				// we are at the last char of the last line so go forward two places
				fseek($fp, ftell($fp) + 2);
				$last_line = fgets($fp);

				list($date, $time, $pid, $msg) = preg_split('/\s+/', $last_line, 4);

				$error_msg = 'rsync error';
				if (substr(trim($msg), 0, strlen($error_msg)) == $error_msg) {
					$file_system_ok = FALSE;
				}
			}
		} else {
			$file_system_okay = FALSE;
		}

		$db_okay = TRUE;

		// connect to the database as the REPADM user
		$dsn_info = isset($db_conf['db'][0]) ? $db_conf['db'][0] :  $db_conf['db'];

		$dsn_info['username'] = 'repadm';
		$dsn_info['password'] = 'repadm';

		try {
			MatrixDAL::dbConnect($dsn_info, 'dbrepl');
			$db = MatrixDAL::getDb('dbrepl');
			$conn_ok = TRUE;
		} catch (Exception $e) {
			$db_okay = FALSE;
			$conn_ok = FALSE;
		}

		if ($conn_ok) {
			// check whether there is a normal status
			$status = '';
			$query = MatrixDAL::preparePdoQuery('SELECT status FROM sys.dba_repcat');
			try {
				$status = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				$db_okay = FALSE;
			}

			if (strtoupper($status) !== 'NORMAL') {
				$db_okay = FALSE;
			}

			// verify that there are no errors in the queue
			$sql = 'SELECT count(*) FROM deferror';
			$errors = -1;
			$query = MatrixDAL::preparePdoQuery($sql);
			try {
				$errors = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				$db_okay = FALSE;
			}

			if ($errors != 0) {
				$db_okay = FALSE;
			}

			// verify that there are no transactions that have not been processed in 5 minutes
			$sql = 'SELECT count(*) FROM deftrandest';
			$trans = -1;

			$query = MatrixDAL::preparePdoQuery($sql);
			try {
				$trans = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				$db_okay = FALSE;
			}

			// since the default is -1, we need to check both conditions.
			if ($trans < 0 || $trans > 200) {
				$db_okay = FALSE;
			}
		}

		if ($file_system_okay) {
			if (!$db_okay) {
				$return_code = '502';
			}
		} else {
			if ($db_okay) {
				$return_code = '501';
			} else {
				$return_code = '503';
			}
		}
	}

	if ($return_code == '200') {
		header('HTTP/1.0 200 OK');
	} else {
		header('HTTP/1.0 '.$return_code.' Internal Server Error');
	}
	echo 'the return code was '.$return_code;

	$logged_in = ($GLOBALS['SQ_SYSTEM']->user && !($GLOBALS['SQ_SYSTEM']->user instanceof Public_User));
	if (!$logged_in) {
		session_destroy();
	}

	exit;
}

/**
 * Extended testing functionality for monitoring purposes.
 * Tests each defined DSN for connectivity and response.
 *
 * Any error = soft failure, (output continues), with an http status of 500
 *
 * Output:
 *  1) List of all DSNs followed by time to connect and time to run a query.
 *  2) If a slon schema is discovered, it's name and replication lag will be printed.
 *
 * A value of -1 within the DSN tests indicates a failure.
 * A value of
 */

$output = '';

$return_code = '200';

require_once dirname(__FILE__).'/../../core/include/init.inc';

require SQ_SYSTEM_ROOT . '/data/private/conf/db.inc';

$dsn_list = array(
	'db'       => 'SELECT count(msgid) FROM sq_internal_msg', //sq_internal_msg is a replicated target and commonly updated
	'db2'      => 'UPDATE sq_ast SET updated = NOW() WHERE type_code = \'root_user\'', //check write perms for main DSN
	'db3'      => 'UPDATE sq_ast SET updated = NOW() WHERE type_code = \'root_user\'', //check write perms for secondary DSN
	'dbcache'  => 'SELECT count(assetid) FROM sq_cache', //check cache size and accessibility
	'dbsearch' => 'SELECT count(assetid) FROM sq_sch_idx'
);

$db_type = isset($db_conf['db'][0]) ? $db_conf['db'][0]['type'] : $db_conf['db']['type'];

/**
 * Each db will also run these extra queries.
 */
$all_db_queries = array();

/**
 * If it's postgres,
 *
 * - check for idle connections
 */
if ($db_type == 'pgsql') {
	$sql = 'SELECT version FROM version()';
	$query = MatrixDAL::preparePdoQuery($sql);
	$result = MatrixDAL::executePdoOne($query);
	$matches = Array();
	preg_match('/[0-9\.]+/', $result, $matches);
	if(version_compare($matches[0], '9.2.0') >= 0) {
	    $all_db_queries[] = "SELECT count(*) as count from pg_stat_activity where state = 'idle'";
	}
	else {
	    $all_db_queries[] = "SELECT count(*) as count from pg_stat_activity where current_query = '<IDLE>'";
	}
}

/**
 * Oracle doesn't have a 'NOW()', but we can use SYSDATE.
 */
if ($db_type == 'oci8' || $db_type == 'oci') {
	$dsn_list['db2'] = str_replace('NOW()', 'SYSDATE', $dsn_list['db2']);
	$dsn_list['db3'] = str_replace('NOW()', 'SYSDATE', $dsn_list['db3']);
}

$last_dsn = null;
foreach ($dsn_list as $dsn_name => $query) {
	$start_time = time();
	$output .= $dsn_name.':';

	/**
	 * some dsn's may not be defined
	 * or may be set to NULL (unused)
	 * if they are, we still need to check the database
	 * but we do it through an alternative name.
	 */
	switch ($dsn_name) {
		case 'dbcache':
		case 'dbsearch':
			if ($dsn_name == 'dbcache') {
				$alt_name = 'db2';
			}
			if ($dsn_name == 'dbsearch') {
				$alt_name = 'db';
			}

			if (!isset($db_conf[$dsn_name])) {
				$dsn_name = $alt_name;
				break;
			}
			if ($db_conf[$dsn_name] === null) {
				$dsn_name = $alt_name;
				break;
			}
		break;
	}

	$dsn = isset($db_conf['db'][0]) ? $db_conf['db'][0]['DSN'] : $db_conf[$dsn_name]['DSN'];

	$restoreConnection = FALSE;
	if ($dsn !== $last_dsn) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection($dsn_name);
		$restoreConnection = TRUE;
	}

	// Do explicit transactions for the database connection, particularly
	// important for oracle where it self-starts transactions.
	// Otherwise, it can cause deadlocks.
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	$qry = MatrixDAL::preparePdoQuery($query);
	try {
		// if it's a select query, use executePdoOne
		// if it's not, use execPdoQuery - so it will return the number of affected rows.

		if (strtolower(substr($query, 0, 6)) === 'select') {
			$res = MatrixDAL::executePdoOne($qry);
		} else {
			$res = MatrixDAL::execPdoQuery($qry);
		}
	} catch (Exception $e) {
		$res = '-1';
		$return_code = '500';
	}

	if ($res === NULL && ($dsn_name == 'db2' || $dsn_name == 'db3')) {
		$res = 1;
	}

	$output .= $res.':'.ceil(time() - $start_time);

	/**
	 * now we have run the main check query, run the extra queries as well.
	 */
	foreach ($all_db_queries as $qry) {
		$query = MatrixDAL::preparePdoQuery($qry);
		try {
			$res = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			$return_code = '500';
			$res = '-1';
		}
		$output .= ':' . $res;
	}

	$output .="\n";

	$last_dsn = $dsn;

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	// Switch the db back if required.
	if ($restoreConnection) {
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}
}

if ($db_type == 'pgsql') {
	// fairly broad brush used here, we know that slon uses '<schema_name>_(logtrigger|denyaccess)_[0-9]', so we'll use that to sniff for slon and it's schema name. denyaccess indicates slave.
	$slon_schema_query = 'SELECT REGEXP_REPLACE(trigger_name, \'(_logtrigger_|_denyaccess_)[0-9]+\', \'\') FROM information_schema.triggers WHERE (trigger_name LIKE \'%_logtrigger_%\' OR trigger_name LIKE \'%_denyaccess_%\') limit 1';

	// {SLON_SCHEMA} is replaced after the schema name is worked out.
	$slon_schema_lag_query = 'SELECT cast(extract(epoch from st_lag_time) as int8) FROM {SLON_SCHEMA}.sl_status WHERE st_origin = (SELECT last_value FROM {SLON_SCHEMA}.sl_local_node_id)';

	$output .= 'slon:';
	$lag = 0;

	// Slon test
	$db_conf['dbslon'] = $db_conf['db2'];
	MatrixDAL::dbConnect($db_conf['dbslon'], 'dbslon');
	$db = MatrixDAL::getDb('dbslon');

	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	$start_time = time();
	$query = MatrixDAL::preparePdoQuery($slon_schema_query);
	try {
		$slon_schema = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		$slon_schema = '';
		$return_code = '500';
	}

	if (!empty($slon_schema)) {
		$output .= $slon_schema.':';
		$query = MatrixDAL::preparePdoQuery(str_replace('{SLON_SCHEMA}', $slon_schema, $slon_schema_lag_query));
		try {
			$lag = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			$lag = '-1';
			$return_code = '500';
		}
	} else {
		$output .= '0:';
	}
	$output .= $lag.':'.ceil(time() - $start_time) ."\n";

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
}

if ($return_code == '200') {
	header('HTTP/1.0 200 OK');
} else {
	header('HTTP/1.0 '.$return_code.' Internal Server Error');
}

print $output;


$logged_in = ($GLOBALS['SQ_SYSTEM']->user && !($GLOBALS['SQ_SYSTEM']->user instanceof Public_User));
if (!$logged_in) {
	session_destroy();
}

?>
