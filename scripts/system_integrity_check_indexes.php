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
* $Id: system_integrity_check_indexes.php,v 1.19.4.1 2013/08/05 01:39:03 csmith Exp $
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
* @version $Revision: 1.19.4.1 $
* @package MySource_Matrix
* @subpackage scripts
*/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);

/**
 * You shouldn't need to edit anything below this.
 */
if (php_sapi_name() != 'cli') trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
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
 * A couple of indexes for postgres do some automatic casting
 * - sq_ast_attr_val_concat
 * - sq_rb_ast_attr_val_concat
 *
 * Postgres does this automatically.
 *
 * It changes this:
 * ((assetid || '~' || attrid));
 * to
 * ((((assetid)::text||'~'::text)||attrid))
 *
 * So just skip checking those definitions.
 */

$skip_definition_checks = array (
	'sq_ast_attr_val_concat',
	'sq_rb_ast_attr_val_concat',
);


/**
 * Keep this here so we can reference the index definition if necessary
 */

$dbtype = _getDbType();

$index_info = getIndexes();
$full_index_list = $index_info['index_list'];
$parallel_list = $index_info['parallel_list'];
$constraint_list = $index_info['constraint_list'];

pre_echo('Checking Indexes');

$sql_commands = array();
$bad_indexes = array();
$parallel_warnings = array();
$postgres_primary_key_warnings = array();

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
				 * we expect the name to be
				 * $tablename . '_pk'
				 * if it's not, throw a warning
				 * as upgrade guides and possibly other scripts
				 * expect it to be named this way
				 */
				$expected_idx_name = $tablename . '_pk';
				$full_idx_names = array (
					$expected_idx_name,
					$tablename . '_pkey',
					'sq_' . $tablename . '_pkey',
					'sq_' . $tablename . '_pk',
				);

				$tablename = 'sq_' . $tablename;

				printName('Checking table ' . $tablename . ' for a primary key');
				$error = true;
				$idx_name = null;
				foreach ($full_idx_names as $_idx_pos => $idx_name) {
					if (in_array($idx_name, array_keys($full_index_list[$tablename]))) {
						$error = false;
						break;
					}
				}

				$idx_columns = $table_info['primary_key'];
				if (substr($tablename, 3, 3) == 'rb_') {
					array_unshift($idx_columns, 'sq_eff_from');
				}

				/**
				 * Oracle has different semantics.
				 * You can have unnamed keys, in which case it gives it a random name.
				 * Eg 'sys_c0013508'
				 * So we have to basically trawl through "unnamed" keys to see if they match.
				 */
				if ($error) {
					$temp_idx_cols = implode(',', $idx_columns);
					if (in_array($temp_idx_cols, $full_index_list[$tablename])) {
						$idx_name = array_search($temp_idx_cols, $full_index_list[$tablename]);
						$error = false;
					}
				}

				/**
				 * if we found an index,
				 * make sure it's the one we expect to find.
				 * otherwise keep it for fixing later.
				 */
				if (!$error) {
					if ($db_conf['db']['type'] == 'pgsql') {
						if ($idx_name !== $expected_idx_name) {
							$postgres_primary_key_warnings[$tablename] = array (
								'current' => $idx_name,
								'expected' => $expected_idx_name,
								'fields' => $idx_columns,
							);
						}
					}
				}

				$create_index_statement = create_index_sql($tablename, $idx_columns, $tablename . '_pk', NULL, true);

				if ($error) {
					printUpdateStatus('Missing');
					$sql_commands[] = $create_index_statement;
				} else {

					check_index_parallel($tablename, $idx_name);

					$index_definition = $full_index_list[$tablename][$idx_name];
					$found_index_columns = explode(',', $index_definition);
					if ($found_index_columns === $idx_columns) {
						printUpdateStatus('OK');
					} else {
						printUpdateStatus('Incorrect');
						$bad_indexes[] = array('table_name' => $tablename, 'index_name' => $idx_name, 'expected' => implode(',', $idx_columns), 'found' => $index_definition, 'primary_key' => true);
						$sql_commands[] = $create_index_statement;
					}
					unset($full_index_list[$tablename][$idx_name]);
				}
			}
		}

		if (!empty($table_info['indexes'])) {
			foreach ($tables as $tablename) {
				$tablename = 'sq_' . $tablename;
				$indexes   = $table_info['indexes'];

				if (substr($tablename, 0, 6) === 'sq_rb_') {
					// If it's a rollback table, also check the
					// eff_to (automatically created) index.
					// Don't need to check eff_from because it's part
					// of the tables primary key - which is already checked.
					$indexes['efto'] = array(
						                'name'    => 'efto',
						                'columns' => array(
							                          'sq_eff_to',
						                             ),
						                'type'    => NULL,
						                'db_type' => NULL,
						               );
				}

				foreach ($indexes as $index_col => $index_info) {
					/**
					 * If the index is for a specific db type (eg the oracle search index),
					 * check the index db type & current db type match.
					 */
					if (isset($index_info['db_type'])) {
						if ($index_info['db_type'] !== $db_conf['db']['type']) {
							continue;
						}
					}

					$full_idx_name = $tablename . '_' . $index_info['name'];
					printName('Checking for index ' . $full_idx_name);

					$error = true;
					if (isset($full_index_list[$tablename][$full_idx_name])) {
						$error = false;
					} else {
						/**
						 * Oracle has different semantics.
						 * You can have unnamed keys, in which case it gives it a random name.
						 * Eg 'sys_c0013508'
						 * So we have to basically trawl through "unnamed" keys to see if they match.
						 */
						$temp_idx_cols = implode(',', $index_info['columns']);
						if (in_array($temp_idx_cols, $full_index_list[$tablename])) {
							$full_idx_name = array_search($temp_idx_cols, $full_index_list[$tablename]);
							$error = false;
						}
					}

					if ($error) {
						printUpdateStatus('Missing');
					} else {

						if (in_array($full_idx_name, $skip_definition_checks)) {
							unset($full_index_list[$tablename][$full_idx_name]);
							printUpdateStatus('OK');
							continue;
						}

						check_index_parallel($tablename, $full_idx_name);

						$index_definition = $full_index_list[$tablename][$full_idx_name];
						$found_index_columns = explode(',', $index_definition);

						unset($full_index_list[$tablename][$full_idx_name]);

						if ($found_index_columns === $index_info['columns']) {
							printUpdateStatus('OK');
							continue;
						} else {
							printUpdateStatus('Incorrect');
							$bad_indexes[] = array('index_name' => $full_idx_name, 'expected' => implode(',', $index_info['columns']), 'found' => $index_definition);
							continue;
						}
					}

					$sql_commands[] = create_index_sql($tablename, $index_info['columns'], $index_info['name'], $index_info['type']);
				}
			}// end foreach
		}//end if

		if (!empty($table_info['unique_key'])) {
			foreach ($tables as $tablename) {

				$tablename = 'sq_' . $tablename;

				$idx_columns = $table_info['unique_key'];
				if (substr($tablename, 3, 3) == 'rb_') {
					array_unshift($idx_columns, 'sq_eff_from');
				}

				/**
				 * Indexes are named after the tablename _ first_col _key
				 * eg
				 * "sq_rb_ast_lnk_sq_eff_from_key" => (sq_eff_from, minorid, majorid, link_type, value)
				 */
				$full_idx_name = $tablename . '_' . $idx_columns[0] . '_key';
				printName('Checking for index ' . $full_idx_name);

				$create_index_statement = create_index_sql($tablename, $idx_columns, $full_idx_name, NULL, false, true);

				$error = true;
				if (isset($full_index_list[$tablename][$full_idx_name])) {
					$error = false;
				} else {
					/**
					 * Oracle has different semantics.
					 * You can have unnamed keys, in which case it gives it a random name.
					 * Eg 'sys_c0013508'
					 * So we have to basically trawl through "unnamed" keys to see if they match.
					 */
					$temp_idx_cols = implode(',', $idx_columns);
					if (in_array($temp_idx_cols, $full_index_list[$tablename])) {
						$full_idx_name = array_search($temp_idx_cols, $full_index_list[$tablename]);
						$error = false;
					}
				}

				if ($error) {
					printUpdateStatus('Missing');
					$sql_commands[] = $create_index_statement;
					continue;
				} else {
					if (in_array($full_idx_name, $skip_definition_checks)) {
						unset($full_index_list[$tablename][$full_idx_name]);
						printUpdateStatus('OK');
						continue;
					}

					check_index_parallel($tablename, $full_idx_name);

					$index_definition = $full_index_list[$tablename][$full_idx_name];
					$found_index_columns = explode(',', $index_definition);

					unset($full_index_list[$tablename][$full_idx_name]);

					if ($found_index_columns === $idx_columns) {
						printUpdateStatus('OK');
						continue;
					}

					printUpdateStatus('Incorrect');

					$bad_indexes[] = array('index_name' => $full_idx_name, 'expected' => implode(',', $idx_columns), 'found' => $index_definition);
					$sql_commands[] = $create_index_statement;
				}
			}
		}// end foreach
	}
}

$extra_message_shown = false;
$dbtype = _getDbType();
if ($dbtype === 'pgsql') {
	pre_echo('Checking locale settings');
	$locale_query = "select setting from pg_settings where name='lc_ctype'";
	$locale_info = MatrixDAL::executeSqlAll($locale_query);
	$locale = $locale_info[0]['setting'];
	if ($locale !== 'C') {
		$extra_message_shown = true;
		$msg = "Your database 'locale' setting is " . $locale . "\n";
		$msg .= "You may get some performance improvements changing this to 'C'\n";
		$msg .= "However changing this requires re-initializing the whole db cluster.\n";
		$msg .= "See http://www.postgresql.org/docs/8.1/static/charset.html for more information.";
		pre_echo($msg);
	}
}

pre_echo('Check complete');
if (!empty($bad_indexes)) {
	$extra_message_shown = true;
	$msg = "Some indexes had incorrect definitions.\n";
	$msg .= "To fix these, you will need to drop the old indexes before re-adding them:\n\n";
	foreach ($bad_indexes as $details) {
		// oracle is case sensitive ..
		if ($dbtype == 'oci') {
			$details['index_name'] = strtoupper($details['index_name']);
		}

		/**
		 * if it's a postgres db, check if it's a primary key.
		 * if it's oracle, see if it's an explicit constraint
		 *
		 * if either is true, we need to drop the constraint.
		 * otherwise we can drop the index.
		 */
		if (
			($dbtype == 'pgsql' && isset($details['primary_key'])) ||
			($dbtype == 'oci' && isset($constraint_list[$details['index_name']]))
		) {
			$tablename = $details['table_name'];
			if (substr($tablename, 0, 3) != 'sq_') {
				$tablename = 'sq_' . $tablename;
			}
			$msg .= "ALTER TABLE " . $details['table_name'] . " DROP CONSTRAINT " . $details['index_name'] . ";\n";
			continue;
		}
		$msg .= "DROP INDEX " . $details['index_name'] . ";\n";
	}
	pre_echo($msg);
}

if (!empty($sql_commands)) {
	$extra_message_shown = true;
	$msg = "Some expected indexes were missing or incorrect.\n";
	$msg .= "To fix the database, please run the following queries:\n\n" . implode("\n", $sql_commands);
	pre_echo($msg);
}

if (!empty($parallel_warnings)) {
	$extra_message_shown = true;
	$msg = "Some indexes may have issues.\n";
	$msg .= "If these changes have been made deliberately, ignore this warning.\n\n";
	$msg .= "The following indexes have been made to run in parallel:\n";
	foreach ($parallel_warnings as $row) {
		foreach ($row as $idx_name => $parallel_setting) {
			$msg .= $idx_name . " (" . $parallel_setting . ")\n";
		}
	}
	$msg .= "This can lead to performance loss.\n\n";

	$msg .= "To reset these, run the following command(s):\n";
	foreach ($parallel_warnings as $row) {
		foreach ($row as $idx_name => $parallel_setting) {
			$msg .= "ALTER INDEX " . $idx_name . " REBUILD PARALLEL 1;\n";
		}
	}
	pre_echo($msg);
}

if (!empty($postgres_primary_key_warnings)) {
	$extra_message_shown = true;
	$msg = "Some tables have incorrect names for primary keys.\n";
	$msg .= "This won't cause any problems, but upgrade guides may be harder to follow\n";
	$msg .= "as they expect the names to be a certain way.\n";
	$msg .= "To fix these tables, run the following commands\n";
	$msg .= "They can take a while depending on the size of the table.\n\n";
	$msg .= "BEGIN;\n";
	foreach ($postgres_primary_key_warnings as $tablename => $details) {
		$msg .= "ALTER TABLE ONLY " . $tablename . " DROP CONSTRAINT " . $details['current'] . ";\n";
		$msg .= "ALTER TABLE " . $tablename . " ADD CONSTRAINT " . $details['expected'] . " PRIMARY KEY (" . implode(',', $details['fields']) . ");\n";
	}
	$msg .= "COMMIT;\n";
	pre_echo($msg);
}

$show_extra_indexes_message = false;
$extra_indexes_message = "Extra indexes have been found on the following tables.\n";
$extra_indexes_message .= "If these changes have been made deliberately, ignore this warning.\n";
$extra_indexes_message .= "This can lead to performance loss.\n";
$extra_indexes_message .= "To remove these, run the following command(s):\n\n";
foreach ($full_index_list as $tablename => $info) {
	if (strpos($tablename, 'sq_rb_') !== FALSE) {
		continue;
	}
	if (empty($info) === TRUE) {
		continue;
	}
	$extra_message_shown = true;
	$show_extra_indexes_message = true;
	$extra_indexes_message .= "These were found on table ".$tablename.":\n";
	foreach ($info as $index_name => $fields) {
		$extra_indexes_message .= "\tDROP INDEX ".$index_name.";\n";
	}
	$extra_indexes_message .= "\n";
}

if ($show_extra_indexes_message) {
	pre_echo(trim($extra_indexes_message));
}

if (!$extra_message_shown) {
	pre_echo('Everything has been checked and no problems were found.');
}


/**
 * parse_tables_xml
 * Parse the appropriate xml file based on the type of db we're dealing with.
 *
 * @param String $xml_file The name of the xml file to parse
 * @param String $db_type The db type ('pgsql', 'oci').
 *
 * @return Array Returns a large array of the tables, sequences, indexes appropriate for that db type.
 */
function parse_tables_xml($xml_file, $db_type)
{

	$dbtype = _getDbType();

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
							if ($variation->getName() == $dbtype) {
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
					if (!is_null($index_db_type) && ((string)$index_db_type != $dbtype)) {
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
 * @param Boolean $unique_key Whether this should be a unique index. Defaults to no.
 *
 * @return String Returns a 'CREATE INDEX' statement ready to run.
 */
function create_index_sql($tablename, $column, $index_name=null, $index_type=null, $primary_key=false, $unique_key=false)
{
	if (substr($tablename, 0, 3) != 'sq_') {
		$tablename = 'sq_' . $tablename;
	}

	if (is_array($column)) {
		$column = implode(',', $column);
	}

	if (is_null($index_name)) {
		$index_name = str_replace(',', '_', $column);
	}

	if (!$primary_key && !$unique_key) {
		$sql = 'CREATE INDEX '.$tablename.'_'.$index_name.' ON '.$tablename;
		if (!empty($index_type)) {
				$sql .= '('.$column.') indextype is '.$index_type;
		} else {
			$sql .= ' ('.$column.')';
		}
		return $sql.';';
	}

	if ($primary_key) {
		/**
		 * the constraint name should be $tablename_pk
		 * without the 'sq_' at the start.
		 */
		$constraint_name = $tablename . '_pk';
		if (substr($constraint_name, 0, 3) == 'sq_') {
			$constraint_name = substr($constraint_name, 3);
		}

		return 'ALTER TABLE ' . $tablename . ' ADD CONSTRAINT ' . $constraint_name . ' PRIMARY KEY (' . $column . ');';
	}

	if ($unique_key) {
		return 'CREATE UNIQUE INDEX ' . $index_name . ' ON ' . $tablename . '(' . $column . ');';
	}

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
	global $db_conf;

	$dbtype = _getDbType();

	switch ($dbtype) {
		case 'oci':
			$sql = "SELECT u.table_name as tablename, u.index_name as indexname, DBMS_METADATA.GET_DDL('INDEX',u.index_name) AS indexdef, TRIM(u.degree) AS parallel, CASE WHEN c.constraint_name IS NULL THEN 0 ELSE 1 END AS constraint FROM USER_INDEXES u LEFT JOIN user_constraints c ON (u.index_name=c.constraint_name) WHERE u.TABLE_NAME LIKE 'SQ_%' ORDER BY u.table_name";
		break;
		case 'pgsql':
			$sql = 'SELECT tablename, indexname, indexdef from pg_indexes where tablename like \'sq_%\'';
		break;
	}

	$constraint_list = array();

	$idx_list = array();

	$parallel_list = array();

	if ($sql !== false) {
		$indexes = MatrixDAL::executeSqlAll($sql);
		foreach($indexes as $key => $value) {
			$tablename = strtolower($value['tablename']);
			if (!isset($idx_list[$tablename])) {
				$idx_list[$tablename] = array();
			}
			switch ($dbtype) {
				case 'oci':
					$idx_def = $value['indexdef'];
					$idx_columns = '';

					$idx_name = $value['indexname'];

					/**
					 * Indexes in oracle look like this
						CREATE [UNIQUE] INDEX "username"."SQ_AST_PUBLISHED" ON "username"."SQ_AST" ("PUBLISHED") ..
					 */
					preg_match('/index "' . $db_conf['db']['user'] . '"."' . $idx_name . '" on "' . $db_conf['db']['user'] . '"."(.*?)" \((.*?)\)/i', $idx_def, $matches);

					if (!empty($matches) && !empty($matches[2])) {
						$idx_columns = str_replace(array(' ', '"'), '', strtolower($matches[2]));
					}

					$idx_name = strtolower($idx_name);

					/**
					 * The way oracle handles unique indexes is to create a sub-index
					 * to enforce things. It's still an index but it has no columns.
					 */
					if (empty($idx_columns) === TRUE) {
						break;
					}

					$idx_list[$tablename][$idx_name] = $idx_columns;

					$parallel_list[$tablename][$idx_name] = $value['parallel'];

					/**
					 * if there's a constraint in user_constraints
					 * remember that for later in case it needs to be dropped.
					 */
					if ($value['constraint'] == 1) {
						$constraint_list[strtoupper($idx_name)] = 1;
					}
				break;

				case 'pgsql':
					$idx_def = $value['indexdef'];
					$idx_columns = '';

					/**
					 * All postgres indexes are btree indexes,
					 * we don't use any other types in matrix.
					 *
					 * The format of which is:
					 * CREATE [UNIQUE] INDEX thes_term_note_pk ON sq_thes_term_note USING btree (termid, name, thesid, value);
					 * ("UNIQUE" is optional)
					 */
					preg_match('/USING btree \((.*?)\)$/i', $idx_def, $matches);
					if (!empty($matches[1])) {
						// clean up special quoted names (like "type" in 8.1 dbs)
						$idx_columns = str_replace(' ', '', str_replace('"', '', $matches[1]));
					}
					$idx_list[$tablename][strtolower($value['indexname'])] = $idx_columns;
				break;
			}
		}
	}
	return array (
		'index_list' => $idx_list,
		'parallel_list' => $parallel_list,
		'constraint_list' => $constraint_list,
	);
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

function check_index_parallel($tablename=null, $idx_name=null)
{

	if ($tablename === null || $idx_name === null) {
		return;
	}

	$dbtype = _getDbType();

	global $parallel_list;
	global $parallel_warnings;

	if ($dbtype == 'oci') {
		$parallel_check = $parallel_list[$tablename][$idx_name];
		if ($parallel_check > 1) {
			$parallel_warnings[] = array($idx_name => $parallel_check);
		}
	}
}

