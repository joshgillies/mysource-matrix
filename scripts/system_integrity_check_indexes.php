<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: system_integrity_check_indexes.php,v 1.1 2009/03/12 04:55:00 csmith Exp $
*
*/

/**
 * This script will look through the database to check for missing indexes.
 * If it finds any, it will print out an sql query to run.
 *
 * It does not run the queries itself, because there may be more involved in fixing the problem.
 *
 * For example, duplicate keys where there shouldn't be.
 * Something like that definitely needs more investigation.
 */

/**
* @author  Chris Smith <csmith@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
* @subpackage scripts
*/

ini_set('memory_limit', -1);
error_reporting(E_ALL);

/**
 * You shouldn't need to edit anything below this.
 */
if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the source System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/data/private/conf/db.inc';
require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/core/lib/DAL/DAL.inc';
require_once $SYSTEM_ROOT.'/core/lib/MatrixDAL/MatrixDAL.inc';

$db_error = false;
try {
	$db_connection = MatrixDAL::dbConnect($db_conf['db']);
} catch (Exception $e) {
	echo "Unable to connect to the db: " . $e->getMessage() . "\n";
	$db_error = true;
}

if ($db_error) {
	exit;
}

MatrixDAL::changeDb('db');

/**
 * flip the array so we can just do an isset check
 * instead of doing an in_array check - slightly quicker :)
 */
$index_list = array_flip(getIndexes());

bam('Checking Indexes');

$sql_commands = array();

$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

foreach ($packages as $_pkgid => $pkg_details) {
	$pkg_name = $pkg_details['code_name'];
	if ($pkg_name == '__core__') {
		$file = $SYSTEM_ROOT.'/core/assets/tables.xml';
	} else {
		$file = SQ_PACKAGES_PATH . '/' . $pkg_name . '/tables.xml';
	}

	/**
	 * If the package doesn't have a tables.xml,
	 * it doesn't have any particular db tables to check.
	 */
	if (!is_file($file)) {
		continue;
	}
	$info = parse_tables_xml($file, $db_conf['db']['type']);

	foreach($info['tables'] as $tablename => $table_info) {
		$tables = array ($tablename);
		if ($table_info['rollback']) {
			$tables[] = 'rb_' . $tablename;
		}

		if (!empty($table_info['primary_key'])) {
			foreach ($tables as $tablename) {
				/**
				 * There are 4 ways a primary key is named:
				 * $tablename . '_pkey',
				 * $tablename . '_pk',
				 * 'sq_' . $tablename . '_pkey',
				 * 'sq_' . $tablename . '_pk',
				 *
				 * (seems to be a postgres thing,
				 * maybe different versions name things differently)
				 */
				$full_idx_names = array (
					$tablename . '_pkey',
					$tablename . '_pk',
					'sq_' . $tablename . '_pkey',
					'sq_' . $tablename . '_pk',
				);

				printName('Checking table ' . $tablename . ' for a primary key');
				$error = true;
				foreach ($full_idx_names as $idx_name) {
					if (isset($index_list[$idx_name])) {
						$error = false;
					}
				}

				if ($error) {
					printUpdateStatus('Missing');

					$idx_columns = $table_info['primary_key'];
					if (substr($tablename, 0, 3) == 'rb_') {
						array_unshift($idx_columns, 'sq_eff_from');
					}
					$sql_commands[] = create_index_sql($tablename, $idx_columns, 'sq_' . $tablename . '_pkey', $index_info['type'], true);
				} else {
					printUpdateStatus('OK');
				}
			}
		}

		if (!empty($table_info['indexes'])) {
			foreach ($table_info['indexes'] as $index_col => $index_info) {
				foreach ($tables as $tablename) {
					$full_idx_name = 'sq_' . $tablename . '_' . $index_info['name'];
					printName('Checking for index ' . $full_idx_name);

					if (!isset($index_list[$full_idx_name])) {
						printUpdateStatus('Missing');
					} else {
						printUpdateStatus('OK');
						continue;
					}

					$sql_commands[] = create_index_sql($tablename, $index_info['columns'], $index_info['name'], $index_info['type']);
				}

			}// end foreach
		}//end if
	}
}

bam('Check complete');

if (!empty($sql_commands)) {
	bam("Some expected indexes were missing.\nTo fix the database, please run the following queries:\n\n" . implode("\n", $sql_commands));
}

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
		$root = new SimpleXMLElement($xml_file, LIBXML_NOCDATA, TRUE);
	} catch (Exception $e) {
		throw new Exception('Could not parse tables XML file: '.$e->getMessage());
	}

	if (($root->getName() != 'schema') || !isset($root->tables) || !isset($root->sequences)) {
		throw new Exception('Tables XML file is not valid.');
		trigger_localised_error('SYS0012', E_USER_WARNING);
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
 * create_index_sql
 * Creates a 'CREATE INDEX' statement
 *
 * @param String $tablename Name of the table
 * @param Mixed $column This can either be a single column name or an array of columns if it's a multi-column index.
 * @param String $index_name Name of the index if you want a specific name. Defaults to the name of the column
 * @param String $index_type Used only by oracle in case it needs a specific index type.
 * @param Boolean $primary_key Whether this should be a primary key index. Defaults to no.
 *
 * @return String Returns a 'CREATE INDEX' statement ready to run.
 */
function create_index_sql($tablename, $column, $index_name=null, $index_type=null, $primary_key=false)
{
	if (is_array($column)) {
		$column = implode(',', $column);
	}

	if (is_null($index_name)) {
		$index_name = str_replace(',', '_', $column);
	}

	if (!$primary_key) {
		$sql = 'CREATE INDEX sq_'.$tablename.'_'.$index_name.' ON sq_'.$tablename;
		if (!empty($index_type)) {
				$sql .= '('.$column.') indextype is '.$index_type;
		} else {
			$sql .= ' ('.$column.')';
		}
		return $sql.';';
	}

	$sql = 'ALTER TABLE sq_' . $tablename . ' ADD PRIMARY KEY (' . $column . ');';

	return $sql;

}//end create_index_sql()


  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


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
			$sql = 'SELECT indexname from pg_indexes where tablename like \'sq_%\'';
		break;
	}

	$idx_list = array();
	if ($sql !== false) {
		$indexes = MatrixDAL::executeSqlAll($sql);
		foreach($indexes as $key => $value) {
			$idx_list[] = strtolower($value[0]);
		}
	}
	return $idx_list;
}

/**
 * _getDbType
 * Returns the type of db
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

