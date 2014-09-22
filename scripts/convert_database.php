<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: convert_database.php,v 1.12 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
* @subpackage scripts
*/

error_reporting(E_ALL);
ini_set('memory_limit', '256M');

/**
 * This script will copy data from an existing system into a new db
 * It can be used to copy either:
 * - from postgresql, to oracle
 * - from oracle, to postgresql
 * (or even from postgresql to postgresql & from oracle to oracle)
 *
 * To use it, edit the file and set the source_dsn and destination_dsn
 * to their appropriate settings.
 *
 * The destinstation_dsn must already have the db tables/views etc created
 * This script does not create them for you.
 *
 * To do that, edit the data/private/conf/db.inc, set the details
 * and run
 * php install/step_02.php /path/to/mysource_matrix
 *
 * Once this script has run successfully, you will need to run rebake.php
 * to fix up the baked out xml queries so they use the appropriate
 * database type's syntaxes.
 */

/**
 * The source & destination dsn's are the same formats as the dsn's
 * used in the data/private/conf/db.inc file.
 *
 * Postgresql example:
 * $dsn = array (
 * 	'DSN'      => 'pgsql:dbname=db;host=ip.address;port=5432',
 * 	'user      => 'db_username',
 * 	'password' => 'db_password',
 * 	'type'     => 'pgsql',
 * );
 *
 * Oracle example:
 * $dsn = array (
 *  'DSN' => 'host/db',
 *  'user' => 'db_username',
 *  'password' => 'db_password',
 *  'type' => 'oci',
 *  'encoding' => 'oracle_encoding', // optional extra
 * );
 *
 * The oracle dsn can also be a full tnsname like dsn:
 * 'DSN' => '(
 *   DESCRIPTION=(
 *     ADDRESS_LIST=(
 *       ADDRESS=
 *       (PROTOCOL=TCP)
 *       (HOST=ip.address)
 *       (PORT=1521)
 *     )
 *   )
 *   (
 *     CONNECT_DATA=
 *     (SID=dbname)
 *     (SERVER=DEDICATED)
 *   )
 * )';
 */

$source_dsn = Array(
				'DSN'      => '',
				'user'     => '',
				'password' => '',
				'type'     => '',
			 );

$destination_dsn = Array(
				'DSN'      => '',
				'user'     => '',
				'password' => '',
				'type'     => '',
			);

/**
 * You shouldn't need to edit anything below this.
 */

if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the source System Root as the first argument\n";
	exit();
}

/**
 * Do a basic check to make sure the dsn's are available.
 */
if (empty($source_dsn) || empty($destination_dsn)) {
	echo "ERROR: Please fill in the source & destination dsn's\n";
	exit;
}

if (empty($source_dsn['DSN']) || empty($destination_dsn['DSN'])) {
	echo "ERROR: Please fill in the source & destination dsn's\n";
	exit;
}

require_once $SYSTEM_ROOT.'/fudge/dev/dev.inc';
require_once $SYSTEM_ROOT.'/fudge/db_extras/db_extras.inc';
require_once $SYSTEM_ROOT.'/core/include/general.inc';
require_once $SYSTEM_ROOT.'/core/lib/DAL/DAL.inc';
require_once $SYSTEM_ROOT.'/core/lib/MatrixDAL/MatrixDAL.inc';

/**
 * So we can use the same database type as the source and dest, serialize
 * the whole array up.
 * If we just used the db type, we couldn't read from and write to the same type.
 * This is actually quite handy - reading from oracle encoding 8859p1, then
 * writing to utf8.
 */
$dest_db = serialize($destination_dsn);
$source_db = serialize($source_dsn);

$db_error = false;
try {
	$source_db_connection = MatrixDAL::dbConnect($source_dsn, $source_db);
} catch (Exception $e) {
	echo "Unable to connect to the source db: " . $e->getMessage() . "\n";
	$db_error = true;
}

try {
	$dest_db_connection = MatrixDAL::dbConnect($destination_dsn, $dest_db);
} catch (Exception $e) {
	echo "Unable to connect to the destination db: " . $e->getMessage() . "\n";
	$db_error = true;
}

/**
 * If we got an error connecting to either system, just stop.
 * The error(s) have already been displayed.
 */
if ($db_error) {
	exit;
}

MatrixDAL::changeDb($dest_db);

$dest_exists = checkDestinationDb();
if (!$dest_exists) {
	echo "Before running the conversion script, you need to run step_02.php from the installer to create the destination tables.\n";
	exit;
}

$xml_files = array (
	$SYSTEM_ROOT . '/core/assets/tables.xml',
);

/**
 * Look for packages that have a tables.xml file.
 */
$packages = scandir($SYSTEM_ROOT . '/packages');
foreach ($packages as $package) {
	$xml_path = $SYSTEM_ROOT . '/packages/' . $package . '/tables.xml';
	if (is_file($xml_path)) {
		$xml_files[] = $xml_path;
	}
}

/**
 * This is a list of tables where nulls
 * will be converted to an empty string.
 *
 * (no 'sq_' at the start!)
 */
$nulls_convert_list = array(
	'ast_lnk',
);


/**
 * If it's the first run, we'll do some extra work
 * - drop sequences
 * - drop indexes
 *
 * They are rebuilt as xml files are processed
 */
$first_run = true;

foreach ($xml_files as $xml_filename) {
	$info = parse_tables_xml($xml_filename, $dest_db);

	if (empty($info)) {
		continue;
	}

	/**
	 * get sequence values from the source db
	 */
	MatrixDAL::changeDb($source_db);
	foreach ($info['sequences'] as $sequence) {
		$sequence_values[$sequence] = getSequenceValue($sequence);
	}

	/**
	 * Switch to the destination db to
	 * - drop sequences
	 * - drop indexes
	 * - truncate data tables
	 */
	MatrixDAL::restoreDb();

	if ($first_run) {
		pre_echo('Dropping destination sequences');

		$del_seqs = getSequences();

		foreach ($del_seqs as $sequence) {
			printName('Dropping: '.strtolower($sequence));
			$sql = 'DROP SEQUENCE ' . $sequence;
			$ok = MatrixDAL::executeSql($sql);
			if ($ok === false) {
				printUpdateStatus('Failure, unable to run query: ' . $sql);
				exit;
			}
			printUpdateStatus('OK');
		}

		pre_echo('Dropping destination indexes');

		$del_indexes = getIndexes();
		foreach($del_indexes as $index) {
			if (substr($index, 0, 3) == 'sq_') {
				printName('Dropping: '.$index);
				$sql = 'DROP INDEX ' . $index;
				$ok = MatrixDAL::executeSql($sql);
				if ($ok === false) {
					printUpdateStatus('Failure, unable to run query: ' . $sql);
					exit;
				}
				printUpdateStatus('OK');
			}//end if
		}//end foreach
	}

	// Empty out the destination tables
	pre_echo('Truncating destination tables');

	foreach ($info['tables'] as $tablename => $table_info) {
		printName('Truncating: sq_'.$tablename);
		$sql = 'TRUNCATE TABLE sq_'.$tablename;
		$ok = MatrixDAL::executeSql($sql);
		if ($ok === false) {
			printUpdateStatus('Failure, unable to run query: ' . $sql);
			exit;
		}
		printUpdateStatus('OK');
	}//end foreach

	/**
	 * Go through each table,
	 * grab the data from the source table
	 * and put it in the destination table.
	 */
	foreach ($info['tables'] as $tablename => $table_info) {
		if ($tablename === 'sch_idx') {
			pre_echo('Skipping search table - you will need to re-index');
			continue;
		}

		pre_echo('Starting table: sq_'.$tablename);

		$columns = array_keys($table_info['columns']);
		$sql = generateSQL($tablename, $columns);

		/**
		 * Switch to the source db connector to get the data..
		 */
		MatrixDAL::changeDb($source_db);

		if (!isset($table_info['primary_key'])) {
			$msg  = "This table (sq_${tablename}) cannot be converted since it doesn't have a primary key.\n";
			$msg .= "A primary key is required to guarantee the chunking of data doesn't\n";
			$msg .= "miss anything and all data is fetched correctly from the table.\n";
			$msg .= "Skipping table.\n";
			pre_echo($msg);

			/**
			 * 'restore' to the dest db
			 * even though we switch straight back to the source
			 * otherwise dal gets confused about which is which.
			 */
			MatrixDAL::restoreDb();
			continue;
		}

		$start = 0;
		$num_to_fetch = 10000;

		$primary_key = implode(',', $table_info['primary_key']);

		$source_sql = 'SELECT * FROM sq_' . $tablename . ' ORDER BY ' . $primary_key;

		$fetch_source_sql = db_extras_modify_limit_clause($source_sql, $source_dsn['type'], $num_to_fetch, $start);

		$source_data = MatrixDAL::executeSqlAll($fetch_source_sql);

		if (empty($source_data)) {
			printName('Table is empty');
			printUpdateStatus('OK');

			/**
			 * 'restore' to the dest db
			 * even though we switch straight back to the source
			 * otherwise dal gets confused about which is which.
			 */
			MatrixDAL::restoreDb();
			continue;
		}

		$count = count($source_data);

		while (!empty($source_data)) {
			MatrixDAL::restoreDb();

			MatrixDAL::beginTransaction();

			$trans_count = 0;

			$prepared_sql = MatrixDAL::preparePdoQuery($sql);

			printName('Inserting Data (' . number_format($start) . ' - ' . number_format($start + $count) . ' rows)');

			printUpdateStatus("..", "\r");

			foreach($source_data as $key => $data) {
				foreach ($data as $data_key => $data_value) {
					// ignore 0 based indexes..
					if (is_numeric($data_key)) {
						continue;
					}

					/**
					 * if the key isn't in the new tables columns, skip it.
					 * this causes a problem with the sq_thes_lnk table
					 * where the 'relation' column was renamed to 'relid'
					 * but this change is not in the upgrade guides anywhere.
					 */
					if (!in_array($data_key, $columns)) {
						continue;
					}

					/**
					 * Oracle treats '' as NULL, so if the source is oracle
					 * change NULL to an empty string - but only for certain tables.
					 */
					if (in_array($tablename, $nulls_convert_list)) {
						if ($source_dsn['type'] === 'oci' && $data_value === NULL) {
							$data_value = '';
						}
					}

					/**
					 * bytea fields from postgres are returned as resources
					 * Convert them from resources into actual text content
					 *
					 * See http://www.php.net/manual/en/pdo.lobs.php
					 */
					if (is_resource($data_value)) {
						$stream = $data_value;
						$data_value = stream_get_contents($stream);
						fclose($stream);
					}

					MatrixDAL::bindValueToPdo($prepared_sql, $data_key, $data_value);
				}
				MatrixDAL::execPdoQuery($prepared_sql);

				$trans_count++;

				if ($trans_count % 10000 == 0) {
					MatrixDAL::commit();
					MatrixDAL::beginTransaction();
					$trans_count = 0;
				}
			}

			printName('Inserting Data (' . number_format($start) . ' - ' . number_format($start + $count) . ' rows)');
			printUpdateStatus('OK', "\r");

			MatrixDAL::commit();

			/**
			 * Switch to the source db connector to get the data..
			 */
			MatrixDAL::changeDb($source_db);

			/**
			 * we got less than the limit?
			 * no point hitting the db again, we won't get anything.
			 */
			if (count($source_data) < $num_to_fetch) {
				break;
			}

			$start += $num_to_fetch;

			$fetch_source_sql = db_extras_modify_limit_clause($source_sql, $source_dsn['type'], $num_to_fetch, $start);

			$source_data = MatrixDAL::executeSqlAll($fetch_source_sql);

			$count = count($source_data);
		}

		printUpdateStatus(null, "\n");

		/**
		 * switch back to the dest db
		 */
		MatrixDAL::restoreDb();

	}//end foreach

	pre_echo('Rebuilding Indexes');

	foreach($info['tables'] as $tablename => $table_info) {
		if (!empty($table_info['indexes'])) {
			foreach ($table_info['indexes'] as $index_col => $index_info) {
				printName('Creating index sq_'.$tablename.'_'.$index_info['name']);
				$sql = create_index_sql($tablename, $index_info['columns'], $index_info['name'], $index_info['type']);
				$ok = MatrixDAL::executeSql($sql);
				if ($ok === false) {
					printUpdateStatus('Failure, unable to run query: ' . $sql);
					exit;
				}
				printUpdateStatus('OK');

				if ($table_info['rollback']) {
						printName('Creating index sq_rb_'.$tablename.'_'.$index_info['name']);
						$sql = create_index_sql('rb_'.$tablename, $index_info['columns'], $index_info['name'], $index_info['type']);
						$ok = MatrixDAL::executeSql($sql);
						if ($ok === false) {
							printUpdateStatus('Failure, unable to run query: ' . $sql);
							exit;
						}
						printUpdateStatus('OK');
				}
			}// end foreach
		}//end if
	}

	pre_echo('Rebuilding Sequences');

	foreach ($info['sequences'] as $sequence) {
		$new_seq_start = $sequence_values[$sequence];

		printName('Creating sq_'.$sequence.'_seq ('.$new_seq_start.')');
		$sql = 'CREATE SEQUENCE sq_'.$sequence.'_seq START WITH '.$new_seq_start;
		$ok = MatrixDAL::executeSql($sql);
		if ($ok === false) {
			printUpdateStatus('Failure, unable to run query: ' . $sql);
			exit;
		}
		printUpdateStatus('OK');
	}

	$first_run = false;
}

/**
 * Lastly, close all the db connections.
 */
MatrixDAL::restoreDb();
MatrixDAL::dbClose($dest_db);
MatrixDAL::dbClose($source_db);

pre_echo('Conversion is complete');

echo "\n";
$rebakeCmd = "/usr/bin/php ${SYSTEM_ROOT}/scripts/rebake.php ${SYSTEM_ROOT}";
echo "rebake.php now needs to run. Do this now [Y/n] ? ";
$response = strtolower(trim(fgets(STDIN)));
if (empty($response) === TRUE) {
    $response = 'y';
}

if ($response != 'y') {
    pre_echo("You will need to run ${rebakeCmd} manually.\n");
    exit;
}

echo "Running rebake.php now .. \n";
$output = array();
$rc     = -1;
exec($rebakeCmd, $output, $rc);
pre_echo("Rebake returned the following:\n".implode("\n", $output)."\n");
pre_echo('This script is now finished.');

/**
 * parse_tables_xml
 * Parse the appropriate xml file based on the type of db we're dealing with.
 *
 * @param String $xml_file The name of the xml file to parse
 * @param String $dest_db The destination db type ('pgsql', 'oci').
 *
 * @return Array Returns a large array of the tables, sequences, indexes appropriate for that db type.
 */
function parse_tables_xml($xml_file, $dest_db)
{
	try {
		$root = simplexml_load_string(file_get_contents($xml_file), 'SimpleXMLElement', LIBXML_NOCDATA);
	} catch (Exception $e) {
		throw new Exception('Could not parse tables XML file: '.$e->getMessage());
	}

	if (($root->getName() != 'schema') || !isset($root->tables) || !isset($root->sequences)) {
		throw new Exception('Tables XML file is not valid.');
		trigger_localised_error('SYS0012', translate('Invalid Schema for DB install'), E_USER_WARNING);
		return FALSE;
	}

	$info = Array();
	$info['tables'] = Array();
	$info['sequences'] = Array();

	//--        TABLES        --//

	foreach ($root->tables->table as $table) {
		$table_name = (string)$table->attributes()->name;

		$info['tables'][$table_name] = Array();
		$info['tables'][$table_name]['rollback'] = (($table->attributes()->{'require_rollback'} == 1) ? TRUE : FALSE);

		//--        TABLE COLUMNS        --//
		$info['tables'][$table_name]['columns'] = Array();

		foreach ($table->columns->column as $table_column) {
			$column_name = (string)$table_column->attributes()->name;

			$info['tables'][$table_name]['columns'][$column_name] = Array();
			$info['tables'][$table_name]['columns'][$column_name]['allow_null'] = (($table_column->attributes()->{'allow_null'} == 1) ? TRUE : FALSE);

			//--        TABLE COLUMN VARS        --//

			$type    = NULL;
			$default = NULL;

			foreach ($table_column->children() as $column_var) {
				switch (strtolower($column_var->getName())) {
					case 'type' :
						// set the type of the column if it hasnt already been
						// set in a variation (this is the default column type)
						if (is_null($type)) $type = (string)$column_var;
					break;
					case 'type_variations' :
						// check for varitions of the column type for his database
						foreach ($column_var->children() as $variation) {
							if ($variation->getName() == _getDbType(false, $dest_db)) {
								$type = (string)$variation;
								break;
							}
						}
					break;
					case 'default' :
						if (trim((string)$column_var) != '') {
							$default = (string)$column_var;
						}
					break;
					default :
						continue;
					break;
				}
			}
			$info['tables'][$table_name]['columns'][$column_name]['type'] = $type;
			$info['tables'][$table_name]['columns'][$column_name]['default'] = $default;

			//--        KEYS        --//

			$info['tables'][$table_name]['primary_key'] = Array();
			$info['tables'][$table_name]['unique_key'] = Array();

			if (isset($table->keys) && (count($table->keys->children()) > 0)) {
				foreach ($table->keys->children() as $table_key) {
					$index_db_type = $table_key->attributes()->db;
					if (!is_null($index_db_type) && ((string)$index_db_type != _getDbType(false, $dest_db))) {
						continue;
					}

					// work out the columns in this key
					$key_columns = Array();
					foreach ($table_key->column as $table_key_column) {
						$col_name = (string)$table_key_column->attributes()->name;
						$key_columns[] = $col_name;

						// cache the primary key columns for this table
						if ($table_key->getName() == 'primary_key') {
							$info['tables'][$table_name]['primary_key'][] = $col_name;
						}
						if ($table_key->getName() == 'unique_key') {
							$info['tables'][$table_name]['unique_key'][] = $col_name;
						}
					}//end foreach
				}//end foreach
			}//end if

			//--        INDEXES        --//

			// check for any indexes that need creating
			if (!empty($table->indexes->index)) {
				foreach ($table->indexes->index as $table_index) {

					// work out the columns in this index
					$index_cols = Array();
					foreach ($table_index->column as $table_index_column) {
						$index_cols[] = (string)$table_index_column->attributes()->name;
					}

					// work out the name of the index
					$index_name    = isset($table_index->attributes()->name) ? (string)$table_index->attributes()->name : reset($index_cols);
					$index_type    = isset($table_index->attributes()->type) ? (string)$table_index->attributes()->type : NULL;
					$index_db_type = isset($table_index->attributes()->db) ? (string)$table_index->attributes()->db : NULL;

					$index_info = Array(
									'name'      => $index_name,
									'columns'	=> $index_cols,
									'type'		=> $index_type,
									'db_type'	=> $index_db_type,
								  );
					$info['tables'][$table_name]['indexes'][$index_name] = $index_info;
				}//end for
			}//end if
		}//end for
	}//end for

	foreach ($root->sequences->sequence as $sequence) {
		$sequence_name = (string)$sequence->attributes()->name;
		$info['sequences'][] = $sequence_name;
	}

	return $info;

}//end parse_tables_xml()

/**
 * generateSQL
 * Generates a prepared statement based on the tablename and columns passed in
 *
 * @param String $tablename The name of the table
 * @param Array $columns The names of the columns to prepare the query for.
 *
 * @return String Returns an sql query ready to be prepared.
 */
function generateSQL($tablename, $columns)
{
	asort($columns);

	$sql = 'INSERT INTO sq_'.$tablename . ' (' . implode(', ', $columns) . ') values (:' . implode(', :', $columns) . ')';

	return $sql;

}//end function

/**
 * create_index_sql
 * Creates a 'CREATE INDEX' statement
 *
 * @param String $tablename Name of the table
 * @param Mixed $column This can either be a single column name or an array of columns if it's a multi-column index.
 * @param String $index_name Name of the index if you want a specific name. Defaults to the name of the column
 * @param String $index_type Used only by oracle in case it needs a specific index type.
 *
 * @return String Returns a 'CREATE INDEX' statement ready to run.
 */
function create_index_sql($tablename, $column, $index_name=null, $index_type=null)
{
	if (is_array($column)) {
		$column = implode(',', $column);
	}

	if (is_null($index_name)) {
		$index_name = str_replace(',', '_', $column);
	}

	$dbtype = _getDbType();

	$sql = 'CREATE INDEX sq_'.$tablename.'_'.$index_name.' ON sq_'.$tablename;

	if (!empty($index_type)) {
		$sql .= '('.$column.')';
		if ($dbtype == 'oci') {
	   		$sql .= ' indextype is '.$index_type;
		}
	} else {
		$sql .= ' ('.$column.')';
	}

	return $sql;

}//end create_index_sql()


  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


function printUpdateStatus($status, $newline="\n")
{
	if ($status !== null) {
		echo "[ $status ]";
	}
	echo $newline;

}//end printUpdateStatus()


/**
 * checkDestinationDb
 * Makes sure the destination db source has at least had 'step_02' of the installer run
 * so there are tables created.
 *
 * @return Boolean Returns false if there are no tables, returns true if there are.
 */
function checkDestinationDb()
{
	$dbtype = _getDbType();

	/**
	 * Make sure the dest version has been installed first.
	 */
	switch ($dbtype) {
		case 'oci':
			$sql = 'select table_name from user_tables';
		break;
		case 'pgsql':
			$sql = 'select table_name from information_schema.tables WHERE table_schema NOT IN (\'pg_catalog\', \'information_schema\')';
		break;
	}
	$dest_tables = MatrixDAL::executeSqlAll($sql);
	if (empty($dest_tables)) {
		return false;
	}
	return true;
}

/**
 * getSequences
 * Returns an array of sequences based on the type of db it's talking to
 *
 * @return Array Returns an array of sequence names
 */
function getSequences()
{
	$dbtype = _getDbType();

	switch ($dbtype) {
		case 'oci':
			$sql = 'SELECT sequence_name FROM user_sequences';
		break;
		case 'pgsql':
			$sql = 'SELECT relname FROM pg_catalog.pg_statio_user_sequences WHERE schemaname = \'public\'';
		break;
	}

	$sequence_names = array();
	$sequences = MatrixDAL::executeSqlAll($sql);
	foreach($sequences as $key => $value) {
		$sequence_names[] = $value[0];
	}
	return $sequence_names;
}

/**
 * getSequenceValue
 * Returns the next value from the sequence based on the db type
 * and the sequence name
 *
 * @param String $sequence The name of the sequence to get the next value for
 *
 * @return Int Returns the next value in the sequence
 */
function getSequenceValue($sequence='')
{
	$dbtype = _getDbType();

	switch ($dbtype) {
		case 'oci':
			$sql = 'SELECT sq_'.$sequence.'_seq.nextval FROM DUAL';
		break;
		case 'pgsql':
			$sql = 'SELECT nextval(\'sq_' . $sequence . '_seq\') AS nextval';
		break;
	}
	$value = MatrixDAL::executeSqlAll($sql);
	return $value[0]['nextval'];
}

/**
 * getIndexes
 * Returns an array of indexes based on the type of db it's talking to
 *
 * @return Array Returns an array of index names
 */
function getIndexes()
{
	$dbtype = _getDbType();

	switch ($dbtype) {
		case 'oci':
			$sql = 'SELECT index_name FROM user_indexes';
		break;
		case 'pgsql':
			$sql = 'SELECT indexname from pg_indexes where tablename like \'sq_%\' and indexdef not ilike \'create unique index %\'';
		break;
	}

	$del_idx = array();
	if ($sql !== false) {
		$indexes = MatrixDAL::executeSqlAll($sql);
		foreach($indexes as $key => $value) {
			$del_idx[] = strtolower($value[0]);
		}
	}
	return $del_idx;
}

/**
 * _getDbType
 * Returns the type of db for the destination db.
 * It's in it's own function because the connection can either be
 * a native 'resource' (oci) or it can be a PDO type
 * If it's a pdo type, it's changed to it's driver name.
 *
 * @return String Returns the type of db connection
 */
function _getDbType()
{
	$dbtype = MatrixDAL::GetDbType();

	if ($dbtype instanceof PDO) {
		$dbtype = $dbtype->getAttribute(PDO::ATTR_DRIVER_NAME);
	}
	return strtolower($dbtype);
}

