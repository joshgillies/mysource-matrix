<?php

// This test message will return one of the following HTTP codes
// 200 | Application is fully functional
// 500 | Database server is not available (only when using checkdb=1)
// 501 | File replication is not working, DB replication working (only when using checkreplication=1)
// 502 | File replication working, DB replication not working (only when using checkreplication=1)
// 503 | File replication not working, DB replication not working (only when using checkreplication=1)

define('SQ_SYSTEM_ROOT', '/app/matrix/');
define('SQ_LOG_PATH', SQ_SYSTEM_ROOT.'/data/private/logs');
$return_code = '200';

if (isset($_GET['checkdb']) && $_GET['checkdb'] == 1) {
	// we are checking for database availablity

	include_once '/app/matrix/data/private/conf/main.inc';
	include_once 'DB.php';

	$conn = DB::connect(SQ_CONF_DB_DSN);
	if (DB::isError($conn)) {
		$return_code = '500';
	} else {
		// try and select something from the sq_ast table
		$assetid = $conn->getOne('SELECT assetid FROM sq_ast WHERE assetid = 1');
		if (DB::isError($assetid)) {
			$return_code = '500';
		}
		$conn->disconnect();
	}
}

if (isset($_GET['checkreplication']) && $_GET['checkreplication'] == 1) {
	// we are checking that replication is working
	$file_system_okay = true;

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
			$file_system_okay = false;
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
				$file_system_ok = false;
			}
		}
	} else {
		$file_system_okay = false;
	}
	

	$db_okay = true;
	
	// connect to the database as the REPADM user
	
	include_once '/app/matrix/data/private/conf/main.inc';
	include_once 'DB.php';
	$dsn_info = DB::parseDSN(SQ_CONF_DB_DSN);
	
	$dsn_info['username'] = 'repadm';
	$dsn_info['password'] = 'repadm';
	
	$conn = DB::connect($dsn_info);
	if (DB::isError($conn)) {
		$db_okay = FALSE;
	} else {

		// check that the status of replication is normal
		$status = $conn->getOne('SELECT status FROM sys.dba_repcat');
		if (DB::isError($status) || strtoupper($status) != 'NORMAL') {
			$db_okay = FALSE;
		}
	
		// verify that there are no errors in the queue
		$sql = 'SELECT count(*) FROM deferror';
		$errors = $conn->getOne($sql);
		if (DB::isError($errors) || $errors != 0) {
			$db_okay = FALSE;
		}
	
		// verify that there are no transactions that have not been processed in 5 minutes
		$sql = 'SELECT 
				count(*) 
			FROM 
				deftrandest'; 

		$trans = $conn->getOne($sql);
		if (DB::isError($trans) || $trans > 200) {
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

?>
