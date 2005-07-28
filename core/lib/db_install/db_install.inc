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
* $Id: db_install.inc,v 1.23 2005/07/28 04:53:27 mmcintyre Exp $
*
*/


/**
* Database Install Fn, reads XML file and creates DB tables and sequences and other db objects
*
* @author  Blair Robertson <blair@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix
* @subpackage lib
*/


/**
* Takes an XML file with the table definitions and attempts to create the
* tables and sequences defined in that file
*
* @param string		$xml_file	the xml file from the package that we are currently processing
* @param boolean	$verbose	TRUE if we should display messages
*
* @return boolean
* @access public
*/
function db_install($xml_file, $verbose=true)
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
	$input =& new XML_Tree($xml_file);
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error($root->getMessage().'<br/>'.$root->getUserInfo(), E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}

	if ($root->name != 'schema' || $root->children[0]->name != 'tables' || $root->children[1]->name != 'sequences') {
		trigger_localised_error('SYS0012', E_USER_WARNING);
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

		$table_exists  = (!empty($cached_table_columns[$table_name])) || in_array('sq_'.$table_name, $tables_in_db);
		$table_changed = false;

		if (!$table_exists) {
			$cached_table_columns[$table_name] = Array('rollback' => false, 'columns' => Array(), 'primary_key' => Array(), 'unique_key' => Array());
		}
		$cached_table_columns[$table_name]['rollback'] = $require_rollback;

		// buffer the columns so we can use them in the rollback table
		// as well, if needed
		ob_start();

		// loop through the table columns
		$table_cols = &$table->children[0]->children;
		for ($j = 0; $j < count($table_cols); $j++) {

			$table_column = &$table_cols[$j];
			$column_name  = $table_column->attributes['name'];
			$allow_null   = (($table_column->attributes['allow_null'] == 1) ? true : false);
			$type         = null;
			$default      = null;

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
						if (trim($column_var->content) != '') {
							$default = $column_var->content;
						}
					break;
					default :
						continue;
					break;
				}
			}// end for

			echo "$column_name $type".((!is_null($default)) ? " DEFAULT $default" : '').((!$allow_null) ? ' NOT NULL' : '').',';

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
				if ($table_key->name == 'unique_key' && !in_array($col_name, $cached_table_columns[$table_name]['unique_key'])) {
					$cached_table_columns[$table_name]['unique_key'][] = $col_name;
				}
			}

			switch (strtolower($table_key->name)) {
				case 'primary_key' :
					// a primary key for the table
					$primary_key = ',PRIMARY KEY('.implode(',',$key_columns).')';
					$rollback_primary_key = ',PRIMARY KEY(sq_eff_from, '.implode(', ',$key_columns).')';
				break;
				case 'unique_key' :
					$other_keys[] = 'UNIQUE('.implode(', ',$key_columns).')';
					$other_rollback_keys[] = 'UNIQUE(sq_eff_from, '.implode(', ',$key_columns).')';
				break;
				default :
					continue;
				break;
			}

		}//end for

		if ($table_exists) {
			if ($table_changed) {
				echo 'The Table "'.$table_name.'" already exists but has changed (from the version in our cache at least).';
				echo 'It is up to you to manualy update this table '.(($require_rollback) ? ' and it\'s rollback table' : '');
			}
		} else {

			//--        NORMAL TABLE DEFINITION        --//

			ob_start();
				echo 'CREATE TABLE sq_'.$table_name.' (';
				echo $table_columns_string;

				echo $primary_key;
				if (!empty($other_keys)) {
					echo ',';
					echo implode(',', $other_keys);
				}
				echo ')';

				// oracle repication requires row dependencies so
				// that the replicating server knows what order to create
				// the new entries
				if ($db->phptype == 'oci8') echo ' ROWDEPENDENCIES';
				$table_sql = ob_get_contents();
			ob_end_clean();

			$new_table_names[] = $table_name;

			$result = $db->query($table_sql);
			assert_valid_db_result($result);

			//--        ROLLBACK TABLE DEFINITION        --//

			if ($require_rollback) {
				ob_start();
					echo 'CREATE TABLE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' (';
					echo 'sq_eff_from TIMESTAMP NOT NULL,';
					echo 'sq_eff_to   TIMESTAMP,';
					echo $table_columns_string;

					echo $rollback_primary_key;
					if (!empty($other_rollback_keys)) {
						echo ',';
						echo implode(',', $other_rollback_keys);
					}
					echo ')';
					if ($db->phptype == 'oci8') echo ' ROWDEPENDENCIES';

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
				$index_type = array_get_index($table_index->attributes, 'type', '');
				$index_db_type = array_get_index($table_index->attributes, 'db', '');

				if (!empty($index_db_type) && $index_db_type != $db->phptype) {
					continue;
				}
				$index_sql = 'CREATE INDEX sq_'.$table_name.'_'.$index_name.' ON sq_'.$table_name;

				if (!empty($index_type)) {
					if ($db->phptype == 'oci8') {
						$index_sql .= '('.$index_col_name.') indextype is '.$index_type;
					} else if ($db->phptype == 'pgsql') {
						$index_sql .= ' USING '.$index_type.'('.$index_col_name.')';
					}
				} else {
					$index_sql .= ' ('.$index_col_name.')';
				}

				$result = $db->query($index_sql);
				assert_valid_db_result($result);

				if ($require_rollback) {
					$index_sql = str_replace('sq_', SQ_TABLE_ROLLBACK_PREFIX, $index_sql);
					$result = $db->query($index_sql);
					assert_valid_db_result($result);
				}
			}//end for

		}//end if table exists

	}//end for tables

	// write a new cache file with all the table names and their columns
	$cached_table_columns_string = '<'.'?php $tables = '.var_export($cached_table_columns, true).'; ?'.'>';
	if (!string_to_file($cached_table_columns_string, $data_path.'/table_columns.inc')) {
		trigger_localised_error('SYS0011', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}

	if ($verbose) {
		$table_names = empty($new_table_names) ? 'No new tables added' : "New Tables:\n".implode($new_table_names,"\n");
		pre_echo("TABLE CREATION COMPLETE\n".$table_names);
	}

	//--        SEQUENCES        --//

	if ($db->phptype == 'pgsql') {
		$sql = 'SELECT c.relname
				FROM pg_class c, pg_user u
				WHERE c.relowner = u.usesysid
				  AND c.relkind = \'S\'
				  AND NOT EXISTS (SELECT 1 FROM pg_views WHERE viewname = c.relname)
				  AND c.relname !~ '.$db->quote('^(pg_|sql_)');

		$sequences_in_db = $db->getCol($sql);
		assert_valid_db_result($sequences_in_db);
	} else if ($db->phptype == 'oci8') {
		$sql = 'SELECT sequence_name FROM user_sequences';

		$sequences_in_db = $db->getCol($sql);
		assert_valid_db_result($sequences_in_db);

		for (reset($sequences_in_db); null !== ($key = key($sequences_in_db)); next($sequences_in_db)) {
			$sequences_in_db[$key] = strtolower($sequences_in_db[$key]);
		}
	}

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

		if (!in_array($sequence_name, $cached_sequences)) {
			$cached_sequences[] = $sequence_name;
		}
		if (in_array('sq_'.$sequence_name.'_seq', $sequences_in_db)) {
			continue;
		}
		$result = $db->createSequence('sq_'.$sequence_name);
		assert_valid_db_result($result);
	}

	$cached_sequences_string = '<'.'?php $sequences = '.var_export($cached_sequences, true).'; ?'.'>';
	if (!string_to_file($cached_sequences_string, $data_path.'/sequences.inc')) {
		trigger_localised_error('SYS0010', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return false;
	}

	if ($verbose) pre_echo('SEQUENCE CREATION COMPLETE');

	// if rollback is enabled, install the rollback triggers
	// otherwise remove them
	if (SQ_CONF_ROLLBACK_ENABLED) {
		install_rollback_triggers($cached_table_columns, $verbose);
	} else {
		uninstall_rollback_triggers();
	}

	unset($cached_sequences);
	unset($cached_sequences_string);
	unset($cached_table_columns);
	unset($cached_table_columns_string);

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	return true;

}//end db_install()


/**
* Grants permisson on the tables created for the secondary user.
*
* This function should be called after the creation of all the tables in the system
*
* @param boolean	$verbose	TRUE if we should show messages
*
* @return void
* @access public
*/
function grant_secondary_user_perms($verbose=true)
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	if ($db->phptype == 'pgsql') {
		$primary_dsn   = DB::parseDSN(SQ_CONF_DB_DSN);
		$secondary_dsn = DB::parseDSN(SQ_CONF_DB2_DSN);

		if ($primary_dsn['username'] != $secondary_dsn['username']) {
			$grant_sql = 'SELECT sq_grant_access('.$db->quote($secondary_dsn['username']).')';
			$result = $db->query($grant_sql);
			assert_valid_db_result($result);
		}

		if ($verbose) {
			pre_echo('PGSQL SECONDARY USER PERMISSIONS FIXED');
		}
	}

}//end grant_secondary_user_perms()


/**
* Installs the stored procedures needed by each database type
*
* @param boolean	$verbose	TRUE if we should display messages
*
* @return void
* @access public
*/
function install_stored_procedures($verbose=true)
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	$fns_file = SQ_SYSTEM_ROOT.'/install/'.$db->phptype.'_functions.xml';

	if (file_exists($fns_file)) {

		$input =& new XML_Tree($fns_file);
		$root  = &$input->getTreeFromFile();

		if (PEAR::isError($root)) {
			trigger_error($root->getMessage().'<br/>'.$root->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($root->name != 'sql') {
			trigger_localised_error('SYS0012', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$element_names = Array();
		foreach ($root->children as $sql_element) {
			$element_names[] = array_get_index($sql_element->attributes, 'display_name', '<Unknown Element>');
			$sql = trim(xml_entity_decode($sql_element->content));
			if (!empty($sql)) {
				$result = $db->query($sql);
				assert_valid_db_result($result);
			}
		}

		if ($verbose) {
			pre_echo(strtoupper($db->phptype).' STORED PROCEDURES CREATED'."\n".implode("\n", $element_names));
		}
	}

}//end install_stored_procedures()


/**
* Replaces elements that are considered XML elements, but not html elements.
*
* @param string	$string	the string that needs cleaning
*
* @access private
* @return string
*/
function xml_entity_decode($string)
{
	$string = html_entity_decode($string);
	$string = str_replace('&#124;', '|', $string);
	$string = str_replace('&apos;', "'", $string);
	$string = str_replace('&#42;', '*', $string);
	$string = str_replace('&#92;', '\\', $string);

	return $string;

}//end xml_entity_decode()


/**
* Returns the installed rollback triggers in the database
*
* @return array
* @access public
*/
function get_installed_rollback_triggers()
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	$trigger_like_sql = $db->quote('sq_%_rb_trg');

	if ($db->phptype == 'oci8') {
		$sql = 'SELECT trigger_name FROM user_triggers WHERE trigger_name';
		$trigger_like_sql = strtoupper($trigger_like_sql);
	} else if ($db->phptype == 'pgsql') {
		$sql = 'SELECT tgname FROM pg_trigger WHERE tgname';
	}
	$sql .= ' LIKE '.$trigger_like_sql;
	$triggers = $db->getCol($sql);
	assert_valid_db_result($triggers);

	return $triggers;

}//end get_installed_rollback_triggers()


/**
* Installs the triggers that insert and update rollback entries
* as those tables that require rollback are updated
*
* This function should be called for each of the packages tables.xml files
* are being processed
*
* @param array		$table_columns	the table column information from the tables xml file
* @param boolean	$verbose		TRUE if we should display messages
* @param boolean	$override		if TRUE override any current triggers with new ones
*
* @return void
* @access public
*/
function install_rollback_triggers($table_columns, $verbose=true, $override=false)
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	$curr_triggers      = Array();
	$installed_triggers = Array();

	// get the current triggers in the system, as we don't want
	// to try to re-install them as postgres does not support the OR REPLACE
	// clause when defining triggers

	$curr_triggers = get_installed_rollback_triggers();

	for (reset($curr_triggers); null !== ($key = key($curr_triggers)); next($curr_triggers)) {
		$curr_triggers[$key] = strtolower($curr_triggers[$key]);
	}

	foreach ($table_columns as $table_name => $table_info) {
		// we only care about rollback tables
		if (!isset($table_info['rollback']) || !$table_info['rollback']) {
			continue;
		}
		$rollback_table = SQ_TABLE_ROLLBACK_PREFIX.$table_name;
		$trigger_name   = strtolower('sq_'.$table_name.'_rb_trg');
		$table          = 'sq_'.$table_name;

		// don't install triggers that have allready been installed
		if (!$override) {
			if (in_array($trigger_name, $curr_triggers)) continue;
		}

		$sql = 'CREATE TRIGGER '.$trigger_name.
			  ' BEFORE INSERT OR UPDATE OR DELETE ON '.$table.
			  ' FOR EACH ROW ';

		if ($db->phptype == 'oci8') {
			$sql .= ' DECLARE BEGIN ';
			// if we are oracle make sure that we are not a remote
			// replication server as the rows will get replicated for us

			$sql .= 'IF DBMS_REPUTIL.FROM_REMOTE = TRUE THEN
						RETURN;
					END IF;';

			// set the timestamp for this session
			// note that the timestamp is set only once per session
			$sql .= 'common_pkg.set_timestamp;';

			$if_inserting = 'IF INSERTING THEN ';
			$if_updating  = 'IF UPDATING THEN ';
			$if_deleting  = 'IF DELETING THEN ';
			// oracle packages are cool, check them out ;)
			$get_timestamp_fn = 'common_pkg.get_timestamp';

		} else if ($db->phptype == 'pgsql') {
			// if we are postgres then we want to define the trigger to call a
			// procedure, end the trigger and begin the prodecure declaration
			$sql .= 'EXECUTE PROCEDURE '.$trigger_name.'_fn();';

			// because postgres triggers can only execute a trigger function, we need to
			// store the actual trigger and execute it AFTER the creation of the trigger function
			$trigger_sql = $sql;
			$sql  = '';
			$sql .= 'CREATE OR REPLACE FUNCTION '.$trigger_name."_fn() RETURNS trigger AS '";
			$sql .= ' DECLARE BEGIN ';

			// set the timestamp for this session
			// note that the timestamp is set only once per session
			$sql .= 'PERFORM sq_set_rollback_timestamp();';

			$if_inserting     = "IF TG_OP = ''INSERT'' THEN ";
			$if_updating      = "IF TG_OP = ''UPDATE'' THEN ";
			$if_deleting      = "IF TG_OP = ''DELETE'' THEN ";
			$get_timestamp_fn = 'sq_get_rollback_timestamp()';
		}

		$old_primary_key_where = '';
		$new_primary_key_where = '';
		$primary_key_where = '';

		$i = 0;

		foreach ($table_info['primary_key'] as $key) {
			$primary_key_where = '';
			if ($i++ != 0) $primary_key_where = ' AND ';
			$primary_key_where .= $key.' = ';
			// old and new vars are treated like bind variables in oracle
			// and therefore require a colon before them
			if ($db->phptype == 'oci8') $primary_key_where .= ':';

			$old_primary_key_where .= $primary_key_where.'OLD.'.$key;
			$new_primary_key_where .= $primary_key_where.'NEW.'.$key;
		}
		$old_primary_key_where .= ';';
		$new_primary_key_where .= ';';

		// construct a column string for the column names
		// and two strings for inserting - one for inserting the values
		// that were just inserted into the non-rollback table and another
		// for inserting the values of any rows that were effected by an update

		$col_string     = ' (sq_eff_from, sq_eff_to,';
		$new_val_string = ' ('.$get_timestamp_fn.',null,';
		$old_val_string = $new_val_string;
		$update_string  = '';

		$i = 0;

		foreach ($table_info['columns'] as $column) {
			// we want to remove the primary and unique keys from the SET
			// sql as they will cause constraint violations
			$is_unique_key  = (isset($table_info['primary_key']) && in_array($column, $table_info['primary_key']));
			$is_unique_key |= (isset($table_info['unique_key'])  && in_array($column, $table_info['unique_key']));
			$col_string    .= $column;

			if (!$is_unique_key) $update_string .= $column.' = ';

			// old and new vars are treated like bind variables in oracle
			// and therefore require a colon before them
			if ($db->phptype == 'oci8') {
				$new_val_string .= ':';
				$old_val_string .= ':';
				if (!$is_unique_key) $update_string  .= ':';
			}
			$new_val_string .= 'NEW.'.$column;
			$old_val_string .= 'OLD.'.$column;
			if (!$is_unique_key) $update_string  .= 'NEW.'.$column;

			if (++$i != count($table_info['columns'])) {
				$col_string     .= ',';
				$new_val_string .= ',';
				$old_val_string .= ',';
				if (!$is_unique_key) $update_string  .= ',';
			}
		}//end foreach

		$col_string     .= ')';
		$new_val_string .= ');';
		$old_val_string .= ');';

		if ($db->phptype == 'oci8') {
			$not_found = 'SQL%NOTFOUND';
		} else if ($db->phptype == 'pgsql') {
			$not_found = 'NOT FOUND';
		}

		// remove any constraint collisions
		// we don't do an update because you cant update primary
		// key columns in oracle
		$remove_rollback_collision = 'DELETE FROM '.
									 $rollback_table.
									 ' WHERE sq_eff_to = '.$get_timestamp_fn.
									 ' AND '.$new_primary_key_where;

		// if we are inserting then we just want to insert the new
		// row into the rollback table

		$sql .= $if_inserting.
				$remove_rollback_collision.
				'INSERT INTO '.$rollback_table.
				$col_string.' VALUES '.$new_val_string;

		if ($db->phptype == 'pgsql') $sql .= ' RETURN NEW; ';
		$sql .= 'END IF;';

		// if we are updating then we want to align the most
		// recent entry in the rollback table and insert any
		// affected rows from the non-rollback table into rollback

		$update_sql = 'UPDATE '.$rollback_table.
					 ' SET sq_eff_to = '.$get_timestamp_fn.
					 ' WHERE sq_eff_to IS NULL AND '.
						$old_primary_key_where;

		// update any entries that were performed in the current session
		// where the timestamp is the same as the session timestamp
		$replace_sql = 'UPDATE '.$rollback_table.
					  ' SET '.$update_string.
					  ' WHERE sq_eff_from = '.$get_timestamp_fn.' AND '.$old_primary_key_where;

		if (trim($update_string) != '') {
			$sql .= $if_updating.
						$replace_sql.
						' IF '.$not_found.' THEN '.
							$update_sql.
							'INSERT INTO '.$rollback_table.
							$col_string.' VALUES '.$old_val_string.
						'END IF;';
			if ($db->phptype == 'pgsql') $sql .= 'RETURN NEW;';
			$sql .= 'END IF;';
		} else {
			// if there is no update string, it means that every
			// column is either a primary or unique key. Therefore
			// you cannot update anything on this table - only insert and delete
			if ($db->phptype == 'pgsql') $sql .= 'RETURN NEW;';
		}

		// if we are deleting then we just close off
		// the rollback entry
		$sql .= $if_deleting.$update_sql;
		if ($db->phptype == 'pgsql') $sql .= ' RETURN OLD; ';
		$sql .= 'END IF;';

		// end begin
		$sql .= 'END;';

		if ($db->phptype == 'pgsql') {
			$sql .= "' LANGUAGE plpgsql;";
			// before we finish we will re-order the sql
			// so that the trigger sql is AFTER the actual trigger function
			// sql so that postgres doesn't complain about functions that does exist
			if (!$override) $sql = $sql.$trigger_sql;
		}

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$installed_triggers[] = $trigger_name;

	}//end foreach tables

	if ($verbose) {
		$trigger_names = (!empty($installed_triggers)) ? 'New Triggers:'."\n".implode("\n", $installed_triggers) : 'No Triggers Installed';
		pre_echo('DATABASE TRIGGER CREATION COMPLETE'."\n".$trigger_names);
	}

}//end install_rollback_triggers()


/**
* Uninstalls any rollback triggers that are installed in the database
*
* @return void
* @access public
*/
function uninstall_rollback_triggers()
{
	$triggers = get_installed_rollback_triggers();
	if (!empty($triggers)) {
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		foreach ($triggers as $trigger_name) {
			// we need to cascade the dropping of the trigger
			// so that the postgres trigger functions are also dropped
			preg_match('/^sq_(.*)_rb_trg$/i', $trigger_name, $matches=Array());
			$sql = 'DROP TRIGGER '.$trigger_name.' on sq_'.$matches[1].' CASCADE';
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}
	}
}//end uninstall_rollback_triggers()


?>
