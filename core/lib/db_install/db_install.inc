<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: db_install.inc,v 1.12 2005/02/21 10:24:04 brobertson Exp $
*
*/


/**
* Database Install Fn, reads XML file and creates DB tables and sequences
*
* @author  Blair Robertson <blair@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
* @subpackage lib
*/


/**
* Takes an XML file with the table definitions and attempts to create the
*
* @param string				$xml_file		the text that should be displayed for the submit button
* @param boolean			$quiet			if true then no echo out some progress strings
*
* @return boolean	indicating success
* @access public
*/
function db_install($xml_file, $quiet=false)
{
	require_once 'XML/Tree.php';
	require_once SQ_FUDGE_PATH.'/general/file_system.inc';

	$data_path = SQ_DATA_PATH.'/private/db';
	$new_table_names = Array();

	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	$db = &$GLOBALS['SQ_SYSTEM']->db;

	$tables_in_db = $db->getCol($db->getSpecialQuery('tables'));
	assert_valid_db_result($tables_in_db);

	// create database tables
	$input = new XML_Tree($xml_file);
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error($root->getMessage().'<br/>'.$root->getUserInfo(), E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}

	if ($root->name != 'schema' || $root->children[0]->name != 'tables' || $root->children[1]->name != 'sequences') {
		trigger_error('Invalid Schema for DB install', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}

	if (is_file($data_path.'/table_columns.inc')) {
		require $data_path.'/table_columns.inc';
		$cached_table_columns = $tables;
		unset($tables);
	} else {
		$cached_table_columns = Array();
	}

	for ($i = 0; $i < count($root->children[0]->children); $i++) {
		$table = &$root->children[0]->children[$i];
		$table_name = $table->attributes['name'];
		$require_rollback = (($table->attributes['require_rollback'] == 1) ? true : false);

		$table_exists  = (!empty($cached_table_columns[$table_name])) || in_array(SQ_TABLE_PREFIX.$table_name, $tables_in_db);

		$table_changed = false;

		// TODO: At some point in the future we could make it so that this script goes into an update and alter
		//       mode if the table exists (it's a nice dream anyway :)
		if (!$table_exists) {
			$cached_table_columns[$table_name] = Array('rollback' => false, 'columns' => Array(), 'primary_key' => Array());
		}
		$cached_table_columns[$table_name]['rollback'] = $require_rollback;

		// buffer the columns so we can use them in the rollback table
		// as well, if needed
		ob_start();

		// loop through the table columns
		$table_cols = &$table->children[0]->children;
		for ($j = 0; $j < count($table_cols); $j++) {
			$table_column = &$table_cols[$j];
			$column_name = $table_column->attributes['name'];
			$allow_null = (($table_column->attributes['allow_null'] == 1) ? true : false);
			$type = null;
			$default = null;

			$column_exists = ($table_exists && in_array($column_name, $cached_table_columns[$table_name]['columns']));
			if (!$column_exists) {
				$cached_table_columns[$table_name]['columns'][] = $column_name;

				// TODO: Have this check the type, default and null status of the column before setting to true
				$table_changed = true;
			}

			for ($k = 0; $k < count($table_column->children); $k++) {
				$column_var = &$table_column->children[$k];

				switch (strtolower($column_var->name)) {
					case 'type' :
						// set the type of the column if it hasnt already been
						// set in a variation (this is the default column type)
						if (is_null($type)) $type = $column_var->content;
						break;
					case 'type_variations' :
						// check for varitions of the column type for his database
						for ($l = 0; $l < count($column_var->children); $l++) {
							$variation = &$column_var->children[$l];
							if ($variation->name == $db->phptype) {
								$type = $variation->content;
								break;
							}
						}
						break;
					case 'default' :
						if (trim($column_var->content) != '') $default = $column_var->content;
						break;
					default :
						continue;
						break;
				}

			}// end for

			echo "$column_name $type".((!$allow_null) ? ' NOT NULL' : '').((!is_null($default)) ? " DEFAULT $default" : '').',';

		}// end for

		$table_columns_string = ob_get_contents();
		ob_end_clean();
		$table_columns_string = substr($table_columns_string,0,strlen($table_columns_string)-1);

		// work out the keys
		$primary_key = '';
		$rollback_primary_key = '';
		$other_keys = Array();
		$other_rollback_keys = Array();

		$table_keys = &$table->children[1]->children;
		for ($j = 0; $j < count($table_keys); $j++) {
			$table_key = &$table_keys[$j];

			// work out the columns in this key
			$key_columns = Array();
			for ($k = 0; $k < count($table_key->children); $k++) {
				$col_name = $table_key->children[$k]->attributes['name'];
				$key_columns[] = $col_name;

				// cache the primary key columns for this table
				if ($table_key->name == 'primary_key' && !in_array($col_name, $cached_table_columns[$table_name]['primary_key'])) {
					$cached_table_columns[$table_name]['primary_key'][] = $col_name;
				}
			}

			switch (strtolower($table_key->name)) {
				case 'primary_key' :
					// a primary key for the table
					$primary_key = ',PRIMARY KEY('.implode(',',$key_columns).')';
					$rollback_primary_key = ',PRIMARY KEY('.SQ_TABLE_PREFIX.'eff_from, '.implode(', ',$key_columns).')';
				break;
				case 'unique_key' :
					$other_keys[] = 'UNIQUE('.implode(', ',$key_columns).')';
					$other_rollback_keys[] = 'UNIQUE('.SQ_TABLE_PREFIX.'eff_from, '.implode(', ',$key_columns).')';
				break;
				default :
					continue;
				break;
			}

		}//end for

		if ($table_exists) {
			if ($table_changed) {
				echo 'The Table "'.$table_name.'" already exists but has changed (from the version in our cache at least). It is up to you to manualy update this table '.(($require_rollback) ? ' and it\'s rollback table' : '');
			}
		} else {


//--        NORMAL TABLE DEFINITION        --//


			ob_start();
				echo 'CREATE TABLE '.SQ_TABLE_PREFIX.$table_name.' (';
				echo $table_columns_string;

				echo $primary_key;
				if (!empty($other_keys)) {
					echo ',';
					echo implode(',', $other_keys);
				}
				echo ')';
				$table_sql = ob_get_contents();
			ob_end_clean();

			$new_table_names[] = $table_name;

			$result = $db->query($table_sql);
			assert_valid_db_result($result);


//--        ROLLBACK TABLE DEFINITION        --//


			if ($require_rollback) {
				ob_start();
					echo 'CREATE TABLE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' (';
					echo SQ_TABLE_PREFIX.'eff_from TIMESTAMP NOT NULL,';
					echo SQ_TABLE_PREFIX.'eff_to   TIMESTAMP,';
					echo $table_columns_string;

					echo $rollback_primary_key;
					if (!empty($other_rollback_keys)) {
						echo ',';
						echo implode(',', $other_rollback_keys);
					}
					echo ')';
					$table_sql = ob_get_contents();
				ob_end_clean();

				$result = $db->query($table_sql);
				assert_valid_db_result($result);
			}


//--        TABLE INDEXES        --//


			// check for any indexes that need creating
			$table_indexes = &$table->children[2]->children;
			for ($j = 0; $j < count($table_indexes); $j++) {
				$table_index = &$table_indexes[$j];

				// work out the columns in this index
				for ($k = 0; $k < count($table_index->children); $k++) {
					$index_col_name = $table_index->children[$k]->attributes['name'];
				}

				// work out the name of the index
				$index_name = array_get_index($table_index->attributes, 'name', $index_col_name);

				$index_sql = 'CREATE INDEX '.SQ_TABLE_PREFIX.$table_name.'_'.$index_name.' ON '.SQ_TABLE_PREFIX.$table_name.' ('.$index_col_name.')';
				$result = $db->query($index_sql);
				assert_valid_db_result($result);

				if ($require_rollback) {
					$index_sql = str_replace(SQ_TABLE_PREFIX, SQ_TABLE_ROLLBACK_PREFIX, $index_sql);
					$result = $db->query($index_sql);
					assert_valid_db_result($result);
				}
			}//end for

		}//end if table exists

	}//end for tables

	// write a new cache file with all the table names and their columns
	$cached_table_columns_string = '<'.'?php $tables = '.var_export($cached_table_columns, true).'; ?'.'>';
	if (!string_to_file($cached_table_columns_string, $data_path.'/table_columns.inc')) {
		trigger_error('Failed writing database table column cache file', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}
	unset($cached_table_columns);
	unset($cached_table_columns_string);

	if (!$quiet) {
		$table_names = empty($new_table_names)?"No new tables added":"New Tables:\n".implode($new_table_names,"\n");
		pre_echo("TABLE CREATION COMPLETE\n".$table_names);
	}

//--        SEQUENCES        --//


	$sql = 'SELECT c.relname
			FROM pg_class c, pg_user u
			WHERE c.relowner = u.usesysid
			  AND c.relkind = \'S\'
			  AND NOT EXISTS (SELECT 1 FROM pg_views WHERE viewname = c.relname)
			  AND c.relname !~ '.$db->quote('^(pg_|sql_)');
	$sequences_in_db = $db->getCol($sql);
	assert_valid_db_result($sequences_in_db);


	if (is_file($data_path.'/sequences.inc')) {
		require $data_path.'/sequences.inc';
		$cached_sequences = $sequences;
		unset($sequences);
	} else {
		$cached_sequences = Array();
	}

	for ($i = 0; $i < count($root->children[1]->children); $i++) {
		$sequence = &$root->children[1]->children[$i];
		$sequence_name = $sequence->attributes['name'];

		if (!in_array($sequence_name, $cached_sequences)) $cached_sequences[] = $sequence_name;
		if (in_array(SQ_TABLE_PREFIX.$sequence_name.'_seq', $sequences_in_db)) continue;

		$result = $db->createSequence(SQ_TABLE_PREFIX.$sequence_name);
		assert_valid_db_result($result);
	}

	$cached_sequences_string = '<'.'?php $sequences = '.var_export($cached_sequences, true).'; ?'.'>';
	if (!string_to_file($cached_sequences_string, $data_path.'/sequences.inc')) {
		trigger_error('Failed writing database sequence column cache file', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}
	unset($cached_sequences);
	unset($cached_sequences_string);


//--        PGSQL GRANT_ACCESS()        --//


	if (!$quiet) pre_echo('SEQUENCE CREATION COMPLETE');

	// If this is PostgreSQL we need to do a couple of other things for the secondary user
	if ($db->phptype == 'pgsql') {

		$psql_functions = $db->getListOf('functions');
		if (DB::isError($psql_functions)) {
			trigger_error($psql_functions->getMessage().'<br/>'.$psql_functions->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!in_array(SQ_TABLE_PREFIX.'grant_access', $psql_functions)) {
			$function_sql = "
			CREATE OR REPLACE FUNCTION ".SQ_TABLE_PREFIX."grant_access(character varying) RETURNS TEXT
			AS '
			DECLARE
				user_name ALIAS FOR $1;
				table RECORD;
				tablename TEXT;
			BEGIN
				FOR table IN SELECT c.relname AS name FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE c.relkind IN (''r'',''v'',''S'','''') AND n.nspname NOT IN (''pg_catalog'', ''pg_toast'') AND pg_catalog.pg_table_is_visible(c.oid) LOOP
					tablename=table.name;
					RAISE NOTICE ''tablename is %'', tablename;
					EXECUTE ''GRANT ALL ON '' || quote_ident(tablename) || '' TO '' || quote_ident(user_name::text);
				END LOOP;
				RETURN ''access granted.'';
			END;
			'
			LANGUAGE plpgsql;
			";
			$result = $db->query($function_sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if (!$quiet) pre_echo('PGSQL GRANT ACCESS FUNCTION CREATED');

		}//end if

		$primary_dsn   = DB::parseDSN(SQ_CONF_DB_DSN);
		$secondary_dsn = DB::parseDSN(SQ_CONF_DB2_DSN);

		if ($primary_dsn['username'] != $secondary_dsn['username']) {
			$grant_sql = 'SELECT '.SQ_TABLE_PREFIX.'grant_access('.$db->quote($secondary_dsn['username']).')';
			$result = $db->query($grant_sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end if
		if (!$quiet) pre_echo('PGSQL SECONDARY USER PERMISSIONS FIXED');

	}//end if

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	return true;

}//end db_install()

?>
