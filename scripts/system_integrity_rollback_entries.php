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
* $Id: system_integrity_rollback_entries.php,v 1.2 2004/07/07 06:38:18 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Adds entries into rollback tables where there are no entries. This will occur
* when rollback has been enabled sometime after the system was installed.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once 'XML/Tree.php';
require_once SQ_DATA_PATH.'/private/db/table_columns.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

// get the tables from table_columns into a var
// that will not clash with other vars
$SQ_TABLE_COLUMNS = $tables;


// first, make sure that rollback is actually turned on
require_once $SYSTEM_ROOT.'/data/private/conf/main.inc';
if (!SQ_CONF_ROLLBACK_ENABLED) echo "*** WARNING!!! Rollback is not enabled on this system. ***\n";

$tables = getRollbackTableNames();

echo 'Enter the date to set as the effective from date in the format: YYYY-MM-DD HH:MM:SS :';
$date = rtrim(fgets(STDIN, 4094));

echo 'Set effective from date to '.date('d F Y h:i:s A', strtotime($date)).'? y/n? ';
$answer = rtrim(fgets(STDIN, 4094));

if (strtolower($answer) != 'y' && strtolower($answer) != 'yes') {
	echo 'Integrity check aborted'."\n";
	exit();
}

// The number of rows to limit to as to avoid an out of memory error
// MUST be greater than 1
$LIMIT_ROWS = 500;

$db = &$GLOBALS['SQ_SYSTEM']->db;
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

foreach ($tables as $table) {
	$rollback_table = SQ_TABLE_ROLLBACK_PREFIX.$table;
	$non_prefix_table = $table;
	$table = SQ_TABLE_PREFIX.$table;

	// check to make sure that there is a entry in the table columns
	// so that we can get the primary key of the rollback table so we
	// can update the oldest record
	if (!isset($SQ_TABLE_COLUMNS[$non_prefix_table])) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		trigger_error('Could not find a table entry for table '.$table.' in table_columns.inc', E_USER_ERROR);
	}
	
	// if we have any unique keys, these will override the primary keys
	if (isset($SQ_TABLE_COLUMNS[$non_prefix_table]['unique_key'])) {
		$primary_keys = $SQ_TABLE_COLUMNS[$non_prefix_table]['unique_key'];
	} else {
		$primary_keys = $SQ_TABLE_COLUMNS[$non_prefix_table]['primary_key'];
	}
	
	$check_str = 'checking '.$rollback_table;
	echo $check_str;

	// the current number of row number
	// that we are starting at
	$current_row_offset = 0;

	// TRUE if the number of rows returned is the number
	// of rows that we are limiting to
	$has_limit_rows = true;

	// TRUE if the rollback table has an entry for
	// the current row that we are parsing
	$has_entry = false;

	// delete any rows that are before or equal to the specified date
	$sql = 'DELETE FROM '.$rollback_table.' WHERE '
		.SQ_TABLE_PREFIX.'effective_to <= '.$db->quote($date);
	$delete_result = $db->query($sql);
	assert_valid_db_result($delete_result);

	while ($has_limit_rows) {

		$sql = 'SELECT '.implode(',', $primary_keys).' FROM '.$table;
		$limit_result = $db->limitQuery($sql, $current_row_offset, $current_row_offset + $LIMIT_ROWS);

		assert_valid_db_result($limit_result);

		if ($limit_result->numRows() < $LIMIT_ROWS) $has_limit_rows = false;
		$current_row_offset += $LIMIT_ROWS;

		while ($limit_result->fetchInto($pk_values, DB_FETCHMODE_ASSOC)) {

			$sql = 'SELECT COUNT(*) FROM '.$rollback_table.' WHERE ';
			$pk_string = '';
			$i = 0;
			foreach ($pk_values as $column => $value) {
				$pk_string .= $column.'='.$db->quote($value);
				if (++$i < count($pk_values)) $pk_string .= ' AND ';
			}
			$sql .= $pk_string.' AND '.SQ_TABLE_PREFIX.'effective_to <= '.$db->quote($date);

			$invalid_entries = $db->getOne($sql);
			assert_valid_db_result($invalid_entries);

			// if there is no corresponding row in the rollback table,
			// create an entry using the specified date as effective from and
			// NULL as effective to to make it current
			
			if (!$invalid_entries) {

				$sql = 'SELECT COUNT(*) FROM '.$rollback_table.' WHERE '.$pk_string;

				$has_rollback_entries = $db->getOne($sql);
				assert_valid_db_result($has_rollback_entries);

				if (!$has_rollback_entries) {

					$columns = $SQ_TABLE_COLUMNS[$non_prefix_table]['columns'];
					$insert = 'INSERT INTO '.$rollback_table.' ('.implode(', ', $columns).
						', '.SQ_TABLE_PREFIX.'effective_from, '.SQ_TABLE_PREFIX.'effective_to)';
					$select = 'SELECT '.implode(',', $columns).','.$db->quote($date).', NULL FROM '.$table.' WHERE '.$pk_string;

					$result = db_extras_insert_select($db, $insert, $select);
					assert_valid_db_result($result);

					// continue to the next row
					continue;
				}

			}
		}//end while row
	}//end while not complete

	$update_concat = '';
	$select_concat = '';
	
	if ($db->phptype == 'mysql') {
		$update_concat = 'CONCAT('.implode(',', $primary_keys).', '.SQ_TABLE_PREFIX.'effective_from)';
		$select_concat = 'CONCAT('.implode(',', $primary_keys).', MIN('.SQ_TABLE_PREFIX.'effective_from))';
	} else {
		$update_concat = '('.implode(' || ', $primary_keys).' || '.SQ_TABLE_PREFIX.'effective_from)';
		$select_concat = '('.implode(' || ', $primary_keys).' || MIN('.SQ_TABLE_PREFIX.'effective_from))';
	}

	$sql = 'UPDATE '.$rollback_table.' SET '.SQ_TABLE_PREFIX.'effective_from = '.$db->quote($date);
	$where = ' WHERE '.$update_concat.' IN (~SQ0~)';

	$subs = Array('SELECT '.$select_concat.' FROM '.$rollback_table.' GROUP BY '.implode(',', $primary_keys));
	$where_clause = db_extras_subquery($db, $where, $subs);
	
	$result = $db->query($sql.$where_clause);
	assert_valid_db_result($result);
	
	echo str_repeat(' ', 80 - strlen($check_str));
	echo ' [ OK ]';
	echo "\n";

}//end foreach table

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');


/**
* Returns the table names of the database tables that require rollback
*
* @return Array()
* @access public
*/
function getRollbackTableNames() {
	global $SYSTEM_ROOT;
	$table_names = Array();

	if (!file_exists($SYSTEM_ROOT.'/core/db/tables.xml')) {
		trigger_error('tables.xml does not exist', E_USER_ERROR);
	}

	$tree = new XML_Tree($SYSTEM_ROOT.'/core/db/tables.xml');
	$root = &$tree->getTreeFromFile();

	for ($i = 0; $i < count($root->children); $i++) {
		if ($root->children[$i]->attributes['require_rollback']) {
			$table_name = $root->children[$i]->attributes['name'];
			array_push($table_names, $table_name);
		}
	}
	return $table_names;

}//end getRollbackTableNames()
