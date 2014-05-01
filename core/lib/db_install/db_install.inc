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
* $Id: db_install.inc,v 1.74 2013/02/21 05:37:18 cupreti Exp $
*
*/



/**
* Database Install Fn, reads XML file and creates DB tables and sequences and other db objects
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Blair Robertson <blair@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.74 $
* @package MySource_Matrix
* @subpackage lib`
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
	$db = MatrixDAL::getDb();

	try {
		$root = simplexml_load_string(file_get_contents($xml_file), 'SimpleXMLElement', LIBXML_NOCDATA);
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
							if ($variation->getName() == MatrixDAL::getDbType()) {
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
					if (!is_null($index_db_type) && ((string)$index_db_type != MatrixDAL::getDbType())) {
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
	$db = MatrixDAL::getDb();
	if (is_null($index_name)) $index_name = implode('_', $columns);

	// Truncate any index names to 30 characters to satisfy Oracle restrictions
	// The '27' value takes out the 'sq_'
	$sql = 'CREATE INDEX sq_'.substr($tablename.'_'.$index_name, 0, 27).' ON sq_'.$tablename;
	if (!empty($index_type)) {
		if (DAL::getDbType() == 'oci') {
			$sql .= '('.implode(', ', $columns).') indextype is '.$index_type;
		} else if (DAL::getDbType() == 'pgsql') {
			$sql .= ' USING '.$index_type.'('.implode(', ', $columns).')';
		}
	} else {
		$sql .= ' ('.implode(', ', $columns).')';
	}

	try {
		DAL::executeSql($sql);
	} catch (DALException $e) {
		throw $e;
	}

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
	$db  = MatrixDAL::getDb();
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
	if (DAL::getDbType() == 'oci') {
		$sql .= ' ROWDEPENDENCIES';
	}

	try {
		DAL::executeSql($sql);
	} catch (DALException $e) {
		throw $e;
	}

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
	$db      = MatrixDAL::getDb();
	$db_type = MatrixDAL::getDbType();

	if ($db_type == 'pgsql') {
		$sql = 'SELECT
					indexname
				FROM
					pg_indexes
				WHERE
					tablename = :std_table';
		if ($include_rollback) {
			$sql .= ' OR tablename = :rollback_table';
		}
	} else if ($db_type == 'oci') {
		$sql = 'SELECT
					index_name
				FROM
					user_indexes
				WHERE
					LOWER(table_name) = :std_table';
		if ($include_rollback) {
			$sql .= ' OR LOWER(table_name) = :rollback_table';
		}
	}

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'std_table',      'sq_'.$tablename);
		MatrixDAL::bindValueToPdo($query, 'rollback_table', 'sq_rb_'.$tablename);
		$indexes_in_db = MatrixDAL::executePdoAssoc($query, 0);
	} catch (DALException $e) {
		throw $e;
	}

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
	$db  = MatrixDAL::getDb();
	$db_type = MatrixDAL::getDbType();

	if ($db_type == 'pgsql') {
		$sql = 'SELECT c.relname
				FROM pg_class c, pg_user u
				WHERE c.relowner = u.usesysid
				  AND c.relkind = \'S\'
				  AND NOT EXISTS (SELECT 1 FROM pg_views WHERE viewname = c.relname)
				  AND c.relname !~ :pattern';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'pattern', '^(pg_|sql_)', PDO::PARAM_STR);
		} catch (DALException $e) {
			throw $e;
		}

	} else if ($db_type == 'oci') {
		$sql = 'SELECT sequence_name FROM user_sequences';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
		} catch (DALException $e) {
			throw $e;
		}
	}

	try {
		$sequences_in_db = MatrixDAL::executePdoAssoc($query, 0);
	} catch (DALException $e) {
		throw $e;
	}

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
	$db  = MatrixDAL::getDb();
	$db_type = MatrixDAL::getDbType();

	if ($db_type == 'pgsql') {
		// formerly DB::getSpecialQuery('tables')
		$sql = 'SELECT c.relname
					FROM pg_class c, pg_user u
					WHERE c.relowner = u.usesysid
					AND c.relkind = \'r\'
					AND NOT EXISTS (SELECT 1 FROM pg_views WHERE viewname = c.relname)
					AND c.relname !~ :pattern
				UNION
				SELECT c.relname
					FROM pg_class c
					WHERE c.relkind = \'r\'
					AND NOT EXISTS (SELECT 1 FROM pg_views WHERE viewname = c.relname)
					AND NOT EXISTS (SELECT 1 FROM pg_user WHERE usesysid = c.relowner)
					AND c.relname !~ \'^pg_\'';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'pattern', '^(pg_|sql_)', PDO::PARAM_STR);
		} catch (DALException $e) {
			throw $e;
		}

	} else if ($db_type == 'oci') {
		$sql = 'SELECT table_name FROM user_tables';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
		} catch (DALException $e) {
			throw $e;
		}
	}

	try {
		$tables_in_db = MatrixDAL::executePdoAssoc($query, 0);
	} catch (DALException $e) {
		throw $e;
	}

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
	$db           = MatrixDAL::getDb();
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
			$date_type = (DAL::getDbType() == 'oci') ? 'DATE' : 'TIMESTAMP';

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
					foreach ($rb_table_info['primary_key'] as $field) {
						// Create individual index on each field of the primary
						// key (needed for some triggers)
						$table_info['rb_indexes'][$field] = Array(
																'columns'	=> Array($field),
																'type'		=> NULL,
																'db_type'	=> NULL,
														   );
					}
					array_unshift($rb_table_info['primary_key'], 'sq_eff_from');
				}
				if (!empty($rb_table_info['unique_key'])) {
					foreach ($rb_table_info['unique_key'] as $field) {
						// Create individual index on each field of the unique key
						$table_info['rb_indexes'][$field] = Array(
																'columns'	=> Array($field),
																'type'		=> NULL,
																'db_type'	=> NULL,
														   );
					}
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
				if (is_null($index_info['db_type']) || $index_info['db_type'] == MatrixDAL::getDbType()) {
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

		// reload the list of indexes, to prevent cases where a rollback
		// index overlaps one created above (which would cause a duplicate
		// index error)
		$indexes_in_db = get_database_indexes($tablename, TRUE);

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

		// TODO: replacement for CREATE SEQUENCE?
		try {
			$sql = 'CREATE SEQUENCE sq_'.$sequence_name.'_seq';
			DAL::executeSql($sql);
		} catch (DALException $e) {
			throw $e;
		}
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
* @throws Exception If there is a database error.
* @return void
* @access public
*/
function grant_secondary_user_perms($verbose=TRUE)
{
	if (DAL::getDbType() == 'pgsql') {
		$db_conf = require(SQ_SYSTEM_ROOT.'/data/private/conf/db.inc');

		// 'db' connection can be configured with multiple DSNs. But we only get the first DSN in the list for now.
		$primary_dsn   = !isset($db_conf['db']['DSN']) && isset($db_conf['db'][0]) ? $db_conf['db'][0] : $db_conf['db'];
		$secondary_dsn = $db_conf['db2'];
		$tertiary_dsn  = $db_conf['db3'];
		$cache_dsn     = $db_conf['dbcache'];
		$search_dsn    = $db_conf['dbsearch'];
		$grant_sql     = 'SELECT sq_grant_access(:user, :access_type)';

		// grant to the primary user
		try {
			$query = MatrixDAL::preparePdoQuery($grant_sql);
			MatrixDAL::bindValueToPdo($query, 'user', $primary_dsn['user']);
			MatrixDAL::bindValueToPdo($query, 'access_type', 'ALL');
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw $e;
		}

		// grant to the secondary user, if different
		if ($primary_dsn['user'] != $secondary_dsn['user']) {
			try {
				$query = MatrixDAL::preparePdoQuery($grant_sql);
				MatrixDAL::bindValueToPdo($query, 'user', $secondary_dsn['user']);
				MatrixDAL::bindValueToPdo($query, 'access_type', 'ALL');
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw $e;
			}
		}

		// grant to the tertiary user, if different to the first two
		if (($primary_dsn['user'] != $tertiary_dsn['user']) && ($secondary_dsn['user'] != $tertiary_dsn['user'])) {
			try {
				$query = MatrixDAL::preparePdoQuery($grant_sql);
				MatrixDAL::bindValueToPdo($query, 'user', $tertiary_dsn['user']);
                MatrixDAL::bindValueToPdo($query, 'access_type', 'ALL');
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw $e;
			}
		}

		if ($verbose) {
			pre_echo('PGSQL SECONDARY AND TERTIARY USER PERMISSIONS FIXED');
		}

		$postgres_db = DAL::getDbType() == 'pgsql';
		$diff_dbcache_user = $primary_dsn['user'] != $cache_dsn['user'] && $secondary_dsn['user'] != $cache_dsn['user'] && $tertiary_dsn['user'] != $cache_dsn['user'];;
		$diff_dbcache_dsn = $primary_dsn['DSN'] != $cache_dsn['DSN'];

		// grant to the cache user, if dsn is defined and is differnet from primary dns or if the cache user is different
		if ($postgres_db && !is_null($cache_dsn) && ($diff_dbcache_user || $diff_dbcache_dsn)) {
			// Change db connection only if the cache dns is differnet from the primary dsn
			if ($diff_dbcache_dsn) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			}

			try {
				$query = MatrixDAL::preparePdoQuery($grant_sql);
				MatrixDAL::bindValueToPdo($query, 'user', $cache_dsn['user']);
				MatrixDAL::bindValueToPdo($query, 'access_type', 'ALL');
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw $e;
			}

			if ($verbose) {
				pre_echo('PGSQL CACHE USER PERMISSIONS FIXED');
			}

			if ($diff_dbcache_dsn) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}
		}

		$diff_dbsearch_user = $primary_dsn['user'] != $search_dsn['user'] && $secondary_dsn['user'] != $search_dsn['user'] && $tertiary_dsn['user'] != $search_dsn['user'] && $cache_dsn['user'] != $search_dsn['user'];
		$diff_dbsearch_dsn = $primary_dsn['DSN'] != $search_dsn['DSN'];

		// grant to the search user, if dsn is defined and is differnet from primary dns or if the search user is different
		if ($postgres_db && !is_null($search_dsn) && ($diff_dbsearch_user || $diff_dbsearch_dsn)) {

			// Change db connection only if the search dns is differnet from the primary dsn
			if ($diff_dbsearch_dsn) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			}

			try {
				$query = MatrixDAL::preparePdoQuery($grant_sql);
				MatrixDAL::bindValueToPdo($query, 'user', $search_dsn['user']);
				MatrixDAL::bindValueToPdo($query, 'access_type', 'SELECT');
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw $e;
			}

			if ($verbose) {
				pre_echo('PGSQL SEARCH USER PERMISSIONS FIXED');
			}

			if ($diff_dbsearch_dsn) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}
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
* @throws Exception Throws an exception if there is a problem with the xml file.
* @throws Exception Throws an exception if there is a problem with running the sql required.
* @return void
* @access public
*/
function install_stored_relations($relations, $package=NULL, $verbose=TRUE)
{
	$db = MatrixDAL::getDb();
	$db_type = MatrixDAL::getDbType();

	if (is_null($package)) {
		$subdir = 'install';
	} else {
		$subdir = 'packages/'.$package;
	}
	$fns_file = SQ_SYSTEM_ROOT.'/'.$subdir.'/'.$db_type.'_'.$relations.'.xml';

	// if the file does not exist, check to see if there is a common file
	// for all database types
	if (!file_exists($fns_file)) {
		$fns_file = SQ_SYSTEM_ROOT.'/'.$subdir.'/common_'.$relations.'.xml';
	}

	if (file_exists($fns_file)) {

		try {
			// Load stored relations file as SimpleXML
			$root = simplexml_load_string(file_get_contents($fns_file), 'SimpleXMLElement', LIBXML_NOCDATA);
		} catch (Exception $e) {
			throw new Exception('Could not parse stored relations file: '.$e->getMessage());
		}

		if ($root->getName() != 'sql') {
			throw new Exception('Cannot install stored relations file: expected root element "sql", found "'.$root->getName().'".');
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return FALSE;
		}

		$display_names = Array();

		foreach ($root->children() as $sql_element) {
			$element_name = $sql_element->getName();
			if ($sql_element->getName() != 'sql_element') {
				throw new Exception('Cannot install stored relations file: expected element "sql_element", found "'.$sql_element->getName().'".');
			}

			$display_names[] = isset($sql_element->attributes()->{'display_name'}) ? $sql_element->attributes()->{'display_name'} : '<Unknown Element>';
			$sql = trim(xml_entity_decode((string)$sql_element));

			if (!empty($sql)) {
				try {
					$result = MatrixDAL::executeSql($sql);
				} catch (DALException $e) {
					throw $e;
				}
			}
		}

		if ($verbose) {
			if (is_null($package)) {
				pre_echo(strtoupper($db_type).' STORED '.strtoupper($relations).' CREATED'."\n".implode("\n", $display_names));
			} else {
				pre_echo(strtoupper($db_type).' STORED '.strtoupper($relations).' FOR PACKAGE '.$package.' CREATED'."\n".implode("\n", $display_names));
			}
		}
	}//end if

}//end install_stored_relations()


/**
* Returns the installed rollback triggers in the database
*
* @return array
* @access public
*/
function get_installed_rollback_triggers()
{
	$db = MatrixDAL::getDb();

	if (DAL::getDbType() == 'oci') {
		$sql = 'SELECT trigger_name FROM user_triggers WHERE LOWER(trigger_name)';
	} else if (DAL::getDbType() == 'pgsql') {
		$sql = 'SELECT tgname FROM pg_trigger WHERE tgname';
	}
	$sql .= ' LIKE :trigger_pattern';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'trigger_pattern', 'sq_%_rb_trg', PDO::PARAM_STR);
		$triggers = MatrixDAL::executePdoAssoc($query, 0);
	} catch (DALException $e) {
		throw $e;
	}

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
	$db = MatrixDAL::getDb();
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

		if (DAL::getDbType() == 'oci') {
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

		} else if (DAL::getDbType() == 'pgsql') {
			// if we are postgres then we want to define the trigger to call a
			// procedure, end the trigger and begin the prodecure declaration
			$sql .= 'EXECUTE PROCEDURE '.$trigger_name.'_fn();';

			// because postgres triggers can only execute a trigger function, we need to
			// store the actual trigger and execute it AFTER the creation of the trigger function
			$trigger_sql = $sql;
			$sql  = '';
			$sql .= 'CREATE OR REPLACE FUNCTION '.$trigger_name."_fn() RETURNS trigger AS '";
			$sql .= ' DECLARE ';
			$sql .= 'rollback_timestamp timestamp;';
			$sql .= ' BEGIN ';

			// We don't need to set the timestamp for the session,
			// sq_get_rollback_timestamp does that for us already.
			$sql .= "EXECUTE ''SELECT sq_get_rollback_timestamp();'' INTO rollback_timestamp;";

			$if_inserting     = "IF TG_OP = ''INSERT'' THEN ";
			$if_updating      = "IF TG_OP = ''UPDATE'' THEN ";
			$if_deleting      = "IF TG_OP = ''DELETE'' THEN ";

			// This is just a function variable, but since oracle is still calling a
			// function it's easier to just use the variable here so we don't end up
			// with complicated code.
			$get_timestamp_fn = 'rollback_timestamp';
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
			if (DAL::getDbType() == 'oci') $primary_key_where .= ':';

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
			if (DAL::getDbType() == 'oci') {
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
		// If the last column was unique it leaves a nasty comma on the end, remove it
		$update_string = rtrim($update_string, ',');

		if (DAL::getDbType() == 'oci') {
			$not_found = 'SQL%NOTFOUND';
		} else if (DAL::getDbType() == 'pgsql') {
			$not_found = 'NOT FOUND';
		}

		// remove any constraint collisions
		// we don't do an update because you cant update primary
		// key columns in oracle
		$remove_rollback_collision = 'DELETE FROM '.
									 $rollback_table.
									 ' WHERE sq_eff_from = '.$get_timestamp_fn.
									 ' AND '.$new_primary_key_where;

		// if we are inserting then we just want to insert the new
		// row into the rollback table

		$sql .= $if_inserting.
				$remove_rollback_collision.
				'INSERT INTO '.$rollback_table.
				$col_string.' VALUES '.$new_val_string;

		if (DAL::getDbType() == 'pgsql') $sql .= ' RETURN NEW; ';
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
		// and if primary key has been changed, we will need do a new insert
		$replace_sql = 'UPDATE '.$rollback_table.
					  ' SET '.$update_string.
					  ' WHERE sq_eff_from = '.$get_timestamp_fn.' AND '.$new_primary_key_where;

		if (trim($update_string) != '') {
			$sql .= $if_updating.
						$replace_sql.
						' IF '.$not_found.' THEN '.
							$update_sql.
							'INSERT INTO '.$rollback_table.
							$col_string.' VALUES '.$new_val_string.
						'END IF;';
			if (DAL::getDbType() == 'pgsql') $sql .= 'RETURN NEW;';
			$sql .= 'END IF;';
		} else {
			// if there is no update string, it means that every
			// column is either a primary or unique key. Therefore
			// you cannot update anything on this table - only insert and delete
			if (DAL::getDbType() == 'pgsql') {
				$sql .= $if_updating.'RETURN NEW;END IF;';
			}
		}

		// if we are deleting then we just close off
		// the rollback entry
		$sql .= $if_deleting.$update_sql;
		if (DAL::getDbType() == 'pgsql') $sql .= ' RETURN OLD; ';
		$sql .= 'END IF;';

		// end begin
		$sql .= 'END;';

		if (DAL::getDbType() == 'pgsql') {
			$sql .= "' LANGUAGE plpgsql;";
			// before we finish we will re-order the sql
			// so that the trigger sql is AFTER the actual trigger function
			// sql so that postgres doesn't complain about functions that does exist
			if (!$override) $sql = $sql.$trigger_sql;
		}

		try {
			DAL::executeSql($sql);
		} catch (DALException $e) {
			throw $e;
		}

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
		$db = MatrixDAL::getDb();
		foreach ($triggers as $trigger_name) {
			// we need to cascade the dropping of the trigger
			// so that the postgres trigger functions are also dropped
			$sql = 'DROP TRIGGER '.$trigger_name;
			if (DAL::getDbType() == 'pgsql') {
				$matches = Array();
				preg_match('/^sq_(.*)_rb_trg$/i', $trigger_name, $matches);
				$sql .= ' on sq_'.$matches[1].' CASCADE';
			}

			try {
				DAL::executeSql($sql);
			} catch (DALException $e) {
				throw $e;
			}
		}
	}

}//end uninstall_rollback_triggers()


?>
