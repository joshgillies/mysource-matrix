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
* $Id: db_install.inc,v 1.42 2007/01/18 00:23:43 tbarrett Exp $
*
*/



/**
* Database Install Fn, reads XML file and creates DB tables and sequences and other db objects
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Blair Robertson <blair@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.42 $
* @package MySource_Matrix
* @subpackage lib
*/


/**
* Parses the tables xml and returns an array of information specific to
* the current database type
*
* @param string	$xml_file	the tables xml file to parse
*
* @return array
* @access public
*/
function parse_tables_xml($xml_file)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$input =& new XML_Tree($xml_file);
	$root =& $input->getTreeFromFile();

	if (PEAR::isError($root)) {
		trigger_error($root->getMessage().'<br/>'.$root->getUserInfo(), E_USER_WARNING);
		return FALSE;
	}
	if ($root->name != 'schema' || $root->children[0]->name != 'tables' || $root->children[1]->name != 'sequences') {
		trigger_localised_error('SYS0012', E_USER_WARNING);
		return FALSE;
	}

	$info = Array();
	$info['tables'] = Array();
	$info['sequences'] = Array();

	//--        TABLES        --//

	for ($i = 0; $i < count($root->children[0]->children); $i++) {
		$table      =&$root->children[0]->children[$i];
		$table_name = $table->attributes['name'];

		$info['tables'][$table_name] = Array();
		$info['tables'][$table_name]['rollback'] = (($table->attributes['require_rollback'] == 1) ? TRUE : FALSE);
		$table_cols =& $table->children[0]->children;

		//--        TABLE COLUMNS        --//

		$info['tables'][$table_name]['columns'] = Array();

		for ($j = 0; $j < count($table_cols); $j++) {
			$table_column =& $table_cols[$j];
			$column_name  = $table_column->attributes['name'];

			$info['tables'][$table_name]['columns'][$column_name] = Array();
			$info['tables'][$table_name]['columns'][$column_name]['allow_null'] = (($table_column->attributes['allow_null'] == 1) ? TRUE : FALSE);

			//--        TABLE COLUMN VARS        --//

			$type    = NULL;
			$default = NULL;

			for ($k = 0; $k < count($table_column->children); $k++) {
				$column_var =& $table_column->children[$k];

				switch (strtolower($column_var->name)) {
					case 'type' :
						// set the type of the column if it hasnt already been
						// set in a variation (this is the default column type)
						if (is_null($type)) $type = $column_var->content;
					break;
					case 'type_variations' :
						// check for varitions of the column type for his database
						for ($l = 0; $l < count($column_var->children); $l++) {
							$variation =& $column_var->children[$l];
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
			}
			$info['tables'][$table_name]['columns'][$column_name]['type'] = $type;
			$info['tables'][$table_name]['columns'][$column_name]['default'] = $default;

			//--        KEYS        --//

			$info['tables'][$table_name]['primary_key'] = Array();
			$info['tables'][$table_name]['unique_key'] = Array();

			$table_keys =& $table->children[1]->children;
			for ($jj = 0; $jj < count($table_keys); $jj++) {
				$table_key =& $table_keys[$jj];

				$index_db_type = array_get_index($table_key->attributes, 'db', NULL);
				if (!is_null($index_db_type) && ($index_db_type != $db->phptype)) {
					continue;
				}

				// work out the columns in this key
				$key_columns = Array();
				for ($k = 0; $k < count($table_key->children); $k++) {
					$col_name = $table_key->children[$k]->attributes['name'];
					$key_columns[] = $col_name;

					// cache the primary key columns for this table
					if ($table_key->name == 'primary_key') {
						$info['tables'][$table_name]['primary_key'][] = $col_name;
					}
					if ($table_key->name == 'unique_key') {
						$info['tables'][$table_name]['unique_key'][] = $col_name;
					}
				}
			}

			//--        INDEXES        --//

			// check for any indexes that need creating
			$table_indexes =& $table->children[2]->children;
			for ($kk = 0; $kk < count($table_indexes); $kk++) {
				$table_index =& $table_indexes[$kk];

				// work out the columns in this index
				$index_cols = Array();
				for ($k = 0; $k < count($table_index->children); $k++) {
					$index_cols[] = $table_index->children[$k]->attributes['name'];
				}

				// work out the name of the index
				$index_name    = array_get_index($table_index->attributes, 'name', reset($index_cols));
				$index_type    = array_get_index($table_index->attributes, 'type', NULL);
				$index_db_type = array_get_index($table_index->attributes, 'db', NULL);

				$index_info = Array(
								'columns'	=> $index_cols,
								'type'		=> $index_type,
								'db_type'	=> $index_db_type,
							  );
				$info['tables'][$table_name]['indexes'][$index_name] = $index_info;
			}//end for
		}//end for
	}//end for

	for ($i = 0; $i < count($root->children[1]->children); $i++) {
		$sequence =& $root->children[1]->children[$i];
		$sequence_name = $sequence->attributes['name'];
		$info['sequences'][] = $sequence_name;
	}

	return $info;

}//end parse_tables_xml()


/**
* Creates an index on the specified table and column
*
* @param string	$tablename	the table to create this index on
* @param string	$column		the column to create this index on
* @param string	$index_name	the name of the index
* @param string	$index_type	the type of index to use
*
* @return void
* @access public
*/
function create_index($tablename, $columns, $index_name=NULL, $index_type=NULL)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	if (is_null($index_name)) $index_name = $column;

	$sql = 'CREATE INDEX sq_'.$tablename.'_'.$index_name.' ON sq_'.$tablename;
	if (!empty($index_type)) {
		if ($db->phptype == 'oci8') {
			$sql .= '('.$column.') indextype is '.$index_type;
		} else if ($db->phptype == 'pgsql') {
			$sql .= ' USING '.$index_type.'('.$column.')';
		}
	} else {
		$sql .= ' ('.implode(', ', $columns).')';
	}

	$result = $db->query($sql);
	assert_valid_db_result($result);

}//end create_index()


/**
* Creates a table with the specified tablename and table_info
*
* All tables will be created with the sq_ prefix
*
* @param string	$tablename	the name of the new table
* @param array	$table_info	the information about the table to create
*
* @return void
* @access public
*/
function create_table($tablename, $table_info)
{
	$db  =& $GLOBALS['SQ_SYSTEM']->db;
	$sql = 'CREATE TABLE sq_'.$tablename.'(';

	$i = 0;
	// build the columns string
	foreach ($table_info['columns'] as $col_name => $col_info) {
		if ($i++ != 0) $sql .= ',';
		$sql .= $col_name.' '.$col_info['type'];
		if (!is_null($col_info['default'])) {
			$sql .= ' DEFAULT '.$col_info['default'];
		}
		if (!$col_info['allow_null']) $sql .= ' NOT NULL';
	}

	// build the keys string
	if (!empty($table_info['primary_key'])) {
		$sql .= ', CONSTRAINT '.$tablename.'_pk PRIMARY KEY ('.implode(',', $table_info['primary_key']).')';
	}
	if (!empty($table_info['unique_key'])) {
		$sql .= ', UNIQUE ('.implode(',', $table_info['unique_key']).')';
	}
	$sql .= ')';

	// oracle replication requires row dependencies so that the replicating
	// server knows what order to create the new entries
	if ($db->phptype == 'oci8') {
		$sql .= ' ROWDEPENDENCIES';
	}

	$result = $db->query($sql);
	assert_valid_db_result($result);

}//end create_table()


/**
* Returns the names of the indexes that have been applied to the specified table
*
* @param string		$tablename			the tablename that contains the wanted indexes
*										without the sq_ prefix
* @param boolean	$include_rollback	if TRUE the indexes on the rollback will also be returned
*
* @return array
* @access public
*/
function get_database_indexes($tablename, $include_rollback=TRUE)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	if ($db->phptype == 'pgsql') {
		$sql = 'SELECT
					indexname
				FROM
					pg_indexes
				WHERE
					tablename = '.$db->quoteSmart('sq_'.$tablename);
		if ($include_rollback) {
			$sql .= ' OR tablename = '.$db->quoteSmart('sq_rb_'.$tablename);
		}
	} else if ($db->phptype == 'oci8') {
		$sql = 'SELECT
					index_name
				FROM
					user_indexes
				WHERE
					table_name = '.$db->quoteSmart(strtoupper('sq_'.$tablename));
		if ($include_rollback) {
			$sql .= ' OR table_name = '.$db->quoteSmart(strtoupper('sq_rb_'.$tablename));
		}
	}

	$indexes_in_db = $db->getCol($sql);
	assert_valid_db_result($indexes_in_db);

	for (reset($indexes_in_db); NULL !== ($key = key($indexes_in_db)); next($indexes_in_db)) {
		$indexes_in_db[$key] = strtolower($indexes_in_db[$key]);
	}

	return $indexes_in_db;

}//end get_database_indexes()


/**
* Returns the user sequence names from the database
*
* @return array
* @access public
*/
function get_database_sequences()
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	if ($db->phptype == 'pgsql') {
		$sql = 'SELECT c.relname
				FROM pg_class c, pg_user u
				WHERE c.relowner = u.usesysid
				  AND c.relkind = \'S\'
				  AND NOT EXISTS (SELECT 1 FROM pg_views WHERE viewname = c.relname)
				  AND c.relname !~ '.$db->quoteSmart('^(pg_|sql_)');

		$sequences_in_db = $db->getCol($sql);
		assert_valid_db_result($sequences_in_db);

	} else if ($db->phptype == 'oci8') {
		$sql = 'SELECT sequence_name FROM user_sequences';
	}

	$sequences_in_db = $db->getCol($sql);
	assert_valid_db_result($sequences_in_db);
	for (reset($sequences_in_db); NULL !== ($key = key($sequences_in_db)); next($sequences_in_db)) {
		$sequences_in_db[$key] = strtolower($sequences_in_db[$key]);
	}

	return $sequences_in_db;

}//end get_database_sequences()


/**
* Returns a list of tables that the current database has
*
* @return array
* @access public
*/
function get_database_tables()
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$query = $db->getSpecialQuery('tables');
	assert_valid_db_result($query);
	$tables_in_db = $db->getCol($query);
	assert_valid_db_result($tables_in_db);

	for (reset($tables_in_db); NULL !== ($key = key($tables_in_db)); next($tables_in_db)) {
		$tables_in_db[$key] = strtolower($tables_in_db[$key]);
	}
	return $tables_in_db;

}//end get_database_tables()


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
function db_install($xml_file, $verbose=TRUE)
{
	$db           =& $GLOBALS['SQ_SYSTEM']->db;
	$info         = parse_tables_xml($xml_file);
	$tables_in_db = get_database_tables();

	// we need to get the cached tables file so that when this function gets called
	// for each of the packages, we keep appending the tables installed for the current package.
	// It won't exist for the first call to this function as step_02 unlinks it
	$current_cached_tables = Array();
	if (file_exists(SQ_DATA_PATH.'/private/db/table_columns.inc')) {
		require SQ_DATA_PATH.'/private/db/table_columns.inc';
		$current_cached_tables = $tables;
	}

	$created_tables  = Array();
	$created_indexes = Array();

	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	foreach ($info['tables'] as $tablename => $table_info) {

		//--        TABLES        --//

		if ($tablename == 'cache') {
			// change the db connection
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			// check existing tables in the new connection
			$old_tables_in_db = $tables_in_db;
			$tables_in_db = get_database_tables();
		}

		// create this table if it doesn't exist in the database
		if (!in_array('sq_'.$tablename, $tables_in_db)) {
			create_table($tablename, $table_info);
			$created_tables[] = 'sq_'.$tablename;
		}

		if ($table_info['rollback']) {

			// use the date type for oracle so we can manipulate its format
			// with the NLS_DATE_FORMAT session variable
			$date_type = ($db->phptype == 'oci8') ? 'DATE' : 'TIMESTAMP';

			// create this rollback table if it does not exist in the database
			if (!in_array('sq_rb_'.$tablename, $tables_in_db)) {
				$rb_table_info = $table_info;
				$rb_col['sq_eff_from'] = Array(
											'type'			=> $date_type,
											'default'		=> NULL,
											'allow_null'	=> FALSE,
										 );
				$rb_col['sq_eff_to'] = Array(
										'type'			=> $date_type,
										'default'		=> NULL,
										'allow_null'	=> TRUE,
									   );

				// append these elements to the front of the column array so they appear
				// as the top of the table
				$rb_table_info['columns'] = array_merge($rb_col, $rb_table_info['columns']);

				if (!empty($rb_table_info['primary_key'])) {
					array_unshift($rb_table_info['primary_key'], 'sq_eff_from');
				}
				if (!empty($rb_table_info['unique_key'])) {
					array_unshift($rb_table_info['unique_key'], 'sq_eff_from');
				}

				// prepare indexes for the "sq_eff_from" and "sq_eff_to" columns
				$table_info['rb_indexes']['effrm'] = Array(
															'columns'	=> Array('sq_eff_from'),
															'type'		=> NULL,
															'db_type'	=> NULL,
														   );
				$table_info['rb_indexes']['efto'] = Array(
															'columns'	=> Array('sq_eff_to'),
															'type'		=> NULL,
															'db_type'	=> NULL,
														 );

				create_table('rb_'.$tablename, $rb_table_info);
				$created_tables[] = 'sq_rb_'.$tablename;
			}//end if
		}//end if rollback

		//--        INDEXES        --//

		// create the database indexes for this table if they don't allready exist
		$indexes_in_db = get_database_indexes($tablename, TRUE);

		if (!empty($table_info['indexes'])) {
			foreach ($table_info['indexes'] as $index_name => $index_info) {
				if (is_null($index_info['db_type']) || $index_info['db_type'] == $db->phptype) {
					if (!in_array('sq_'.$tablename.'_'.$index_name, $indexes_in_db)) {
						create_index($tablename, $index_info['columns'], $index_name, $index_info['type']);
						$created_indexes[] = 'sq_'.$tablename.'_'.$index_name;
					}
					if ($table_info['rollback']) {
						if (!in_array('sq_rb_'.$tablename.'_'.$index_name, $indexes_in_db)) {
							create_index('rb_'.$tablename, $index_info['columns'], $index_name, $index_info['type']);
							$created_indexes[] = 'sq_rb_'.$tablename.'_'.$index_name;
						}
					}
				}
			}// end foreach

		}//end if

		// create indexes (prepared earlier) which are specific to rollback and not required to be replicated for "non-rollback" tables
		if (($table_info['rollback']) && (!empty($table_info['rb_indexes']))) {
			foreach ($table_info['rb_indexes'] as $index_name => $index_info) {
				if (!in_array('sq_rb_'.$tablename.'_'.$index_name, $indexes_in_db)) {
					create_index('rb_'.$tablename, $index_info['columns'], $index_name, $index_info['type']);
					$created_indexes[] = 'sq_rb_'.$tablename.'_'.$index_name;
				}
			}// end foreach
		}// end if

		if ($tablename == 'cache') {
			// restore the db connection
			$tables_in_db = $old_tables_in_db;
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		}

	}//end foreach tables

	if ($verbose) {
		$table_names = empty($created_tables) ? 'No new tables added' : 'New Tables:'."\n".implode($created_tables, "\n");
		pre_echo("TABLE CREATION COMPLETE\n".$table_names);

		$index_names = empty($created_indexes) ? 'No Indexes added' : 'New Indexes:'."\n".implode($created_indexes, "\n");
		pre_echo("INDEX CREATION COMPLETE\n".$index_names);
	}

	//--        SEQUENCES        --//

	$sequences_in_db = get_database_sequences();
	foreach ($info['sequences'] as $sequence_name) {
		if (in_array('sq_'.$sequence_name.'_seq', $sequences_in_db)) {
			continue;
		}
		$result = $db->createSequence('sq_'.$sequence_name);
		assert_valid_db_result($result);
	}

	if ($verbose) pre_echo('SEQUENCE CREATION COMPLETE');

	//--        CACHED FILES        --//

	$cached_sequences_string = '<'.'?php $sequences = '.var_export($info['sequences'], TRUE).'; ?'.'>';
	if (!string_to_file($cached_sequences_string, SQ_DATA_PATH.'/private/db/sequences.inc')) {
		trigger_localised_error('SYS0010', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return FALSE;
	}

	// we don't need the detailed column information, so just
	// create an array of the actual column names and cache that
	$cached_table_info = $info['tables'];
	foreach ($cached_table_info as $table => $info) {
		$cached_table_info[$table]['columns'] = array_keys($info['columns']);
	}
	$cached_table_info = array_merge($current_cached_tables, $cached_table_info);

	$cached_tables_string = '<'.'?php $tables = '.var_export($cached_table_info, TRUE).'; ?'.'>';
	if (!string_to_file($cached_tables_string, SQ_DATA_PATH.'/private/db/table_columns.inc')) {
		trigger_localised_error('SYS0011', E_USER_WARNING);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return FALSE;
	}

	// if rollback is enabled, install the rollback triggers, otherwise remove them
	if (SQ_CONF_ROLLBACK_ENABLED) {
		install_rollback_triggers($cached_table_info, $verbose);
	} else {
		uninstall_rollback_triggers();
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	return TRUE;

}//end db_install()


/**
* Grants permisson on the tables created for the secondary (db2) and tertiary (db3) user.
*
* This function should be called after the creation of all the tables in the system
*
* @param boolean	$verbose	TRUE if we should show messages
*
* @return void
* @access public
*/
function grant_secondary_user_perms($verbose=TRUE)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	if ($db->phptype == 'pgsql') {
		$primary_dsn   = DB::parseDSN(SQ_CONF_DB_DSN);
		$secondary_dsn = DB::parseDSN(SQ_CONF_DB2_DSN);
		$tertiary_dsn = DB::parseDSN(SQ_CONF_DB3_DSN);
		$cache_dsn = Array();
		if (SQ_CONF_DBCACHE_DSN != '') {
			$cache_dsn = DB::parseDSN(SQ_CONF_DBCACHE_DSN);
		}

		// grant to the primary user
		$grant_sql = 'SELECT sq_grant_access('.$db->quote($primary_dsn['username']).')';
		$result = $db->query($grant_sql);
		assert_valid_db_result($result);

		// grant to the secondary user, if different
		if ($primary_dsn['username'] != $secondary_dsn['username']) {
			$grant_sql = 'SELECT sq_grant_access('.$db->quote($secondary_dsn['username']).')';
			$result = $db->query($grant_sql);
			assert_valid_db_result($result);
		}

		// grant to the tertiary user, if different to the first two
		if (($primary_dsn['username'] != $tertiary_dsn['username']) && ($secondary_dsn['username'] != $tertiary_dsn['username'])) {
			$grant_sql = 'SELECT sq_grant_access('.$db->quote($tertiary_dsn['username']).')';
			$result = $db->query($grant_sql);
			assert_valid_db_result($result);
		}

		if ($verbose) {
			pre_echo('PGSQL SECONDARY AND TERTIARY USER PERMISSIONS FIXED');
		}

		// grant to the cache user, if dsn is defined
		if (!empty($cache_dsn)) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			if ($db->phptype == 'pgsql') {
				$grant_sql = 'SELECT sq_grant_access('.$db->quote($cache_dsn['username']).')';
				$result = $db->query($grant_sql);
				assert_valid_db_result($result);

				if ($verbose) {
					pre_echo('PGSQL CACHE USER PERMISSIONS FIXED');
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}


	}//end if

}//end grant_secondary_user_perms()


/**
* Installs the stored relations needed by each database type
*
* @param string		$relations	the type of relations to install. Eg. functions|views
* @param string		$package	the package name
* @param boolean	$verbose	TRUE if we should display messages
*
* @return void
* @access public
*/
function install_stored_relations($relations, $package=NULL, $verbose=TRUE)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	if (is_null($package)) {
		$subdir = 'install';
	} else {
		$subdir = 'packages/'.$package;
	}
	$fns_file = SQ_SYSTEM_ROOT.'/'.$subdir.'/'.$db->phptype.'_'.$relations.'.xml';

	// if the file does not exist, check to see if there is a common file
	// for all database types
	if (!file_exists($fns_file)) {
		$fns_file = SQ_SYSTEM_ROOT.'/'.$subdir.'/common_'.$relations.'.xml';
	}

	if (file_exists($fns_file)) {

		$input =& new XML_Tree($fns_file);
		$root  =& $input->getTreeFromFile();

		if (PEAR::isError($root)) {
			trigger_error($root->getMessage().'<br/>'.$root->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return FALSE;
		}

		if ($root->name != 'sql') {
			trigger_localised_error('SYS0012', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return FALSE;
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
			if (is_null($package)) {
				pre_echo(strtoupper($db->phptype).' STORED '.strtoupper($relations).' CREATED'."\n".implode("\n", $element_names));
			} else {
				pre_echo(strtoupper($db->phptype).' STORED '.strtoupper($relations).' FOR PACKAGE '.$package.' CREATED'."\n".implode("\n", $element_names));
			}
		}
	}//end if

}//end install_stored_relations()


/**
* Replaces elements that are considered XML elements, but not html elements.
*
* @param string	$string	the string that needs cleaning
*
* @return string
* @access private
*/
function xml_entity_decode($string)
{
	$string = html_entity_decode($string);
	$string = str_replace('&#124;', '|', $string);
	$string = str_replace('&apos;', "'", $string);
	$string = str_replace('&#42;', '*', $string);
	$string = str_replace('&#92;', '\\', $string);
	$string = str_replace('&#126;', '~', $string);

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
	$db =& $GLOBALS['SQ_SYSTEM']->db;
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
function install_rollback_triggers($table_columns, $verbose=TRUE, $override=FALSE)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$curr_triggers      = Array();
	$installed_triggers = Array();

	// get the current triggers in the system, as we don't want
	// to try to re-install them as postgres does not support the OR REPLACE
	// clause when defining triggers

	$curr_triggers = get_installed_rollback_triggers();

	for (reset($curr_triggers); NULL !== ($key = key($curr_triggers)); next($curr_triggers)) {
		$curr_triggers[$key] = strtolower($curr_triggers[$key]);
	}

	foreach ($table_columns as $table_name => $table_info) {
		// we only care about rollback tables
		if (!isset($table_info['rollback']) || !$table_info['rollback']) {
			continue;
		}
		$rollback_table = 'sq_rb_'.$table_name;
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
			$sql .= 'sq_common_pkg.sq_set_rollback_timestamp;';

			$if_inserting = 'IF INSERTING THEN ';
			$if_updating  = 'IF UPDATING THEN ';
			$if_deleting  = 'IF DELETING THEN ';
			// oracle packages are cool, check them out ;)
			$get_timestamp_fn = 'sq_common_pkg.sq_get_rollback_timestamp';

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
							$col_string.' VALUES '.$new_val_string.
						'END IF;';
			if ($db->phptype == 'pgsql') $sql .= 'RETURN NEW;';
			$sql .= 'END IF;';
		} else {
			// if there is no update string, it means that every
			// column is either a primary or unique key. Therefore
			// you cannot update anything on this table - only insert and delete
			if ($db->phptype == 'pgsql') {
				$sql .= $if_updating.'RETURN NEW;END IF;';
			}
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		foreach ($triggers as $trigger_name) {
			// we need to cascade the dropping of the trigger
			// so that the postgres trigger functions are also dropped
			$sql = 'DROP TRIGGER '.$trigger_name;
			if ($db->phptype == 'pgsql') {
				preg_match('/^sq_(.*)_rb_trg$/i', $trigger_name, $matches=Array());
				$sql .= ' on sq_'.$matches[1].' CASCADE';
			}

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}
	}

}//end uninstall_rollback_triggers()


?>
