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
* $Id: rollback_management.php,v 1.7.2.1 2005/07/27 04:48:43 gsherwood Exp $
*
*/

/**
* Adds entries into rollback tables where there are no entries. This will occur
* when rollback has been enabled sometime after the system was installed.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.7.2.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

require_once 'Console/Getopt.php';

$shortopt = 'd:p:s:q::';
$longopt = Array('enable-rollback', 'disable-rollback');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);

if (empty($options[0])) usage();

$ROLLBACK_DATE = '';
$SYSTEM_ROOT = '';
$ENABLE_ROLLBACK = false;
$DISABLE_ROLLBACK = false;
$QUIET = false;

foreach ($options[0] as $option) {

	switch ($option[0]) {
		case 'd':
			if (!empty($ROLLBACK_DATE)) usage();
			if (empty($option[1])) usage();
			if (!preg_match('|^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}$|', $option[1])) usage();
			$ROLLBACK_DATE = $option[1];
		break;

		case 'p':
			if (!empty($ROLLBACK_DATE)) usage();
			if (empty($option[1])) usage();
			$matches = Array();
			if (!preg_match('|^(\d+)([hdwmy])$|', $option[1], $matches)) usage();

			$time_num = (int)$matches[1];
			$time_units = '';
			switch ($matches[2]) {
				case 'h' :
					$time_units = 'hour';
				break;
				case 'd' :
					$time_units = 'day';
				break;
				case 'w' :
					$time_units = 'week';
				break;
				case 'm' :
					$time_units = 'month';
				break;
				case 'y' :
					$time_unit = 'year';
				break;
			}
			if ($time_num > 1) $time_units .= 's';
			$ROLLBACK_DATE = date('Y-m-d H:i:s', strtotime('-'.$time_num.' '.$time_units));
		break;

		case 's':
			if (empty($option[1])) usage();
			if (!is_dir($option[1])) usage();
			$SYSTEM_ROOT = $option[1];
		break;

		case '--enable-rollback':
			if ($DISABLE_ROLLBACK) usage();
			$ENABLE_ROLLBACK = true;
		break;

		case '--disable-rollback':
			if ($ENABLE_ROLLBACK) usage();
			$DISABLE_ROLLBACK = true;
		break;
		case 'q':
			$QUIET = true;
		break;
		default:
			echo "Invalid option - ".$option[0];
			usage();
	}//end switch

}//end foreach arguments

if ($ENABLE_ROLLBACK || $DISABLE_ROLLBACK) {
	if (!empty($ROLLBACK_DATE)) usage();
	$ROLLBACK_DATE = date('Y-m-d H:i:s');
}

if (empty($SYSTEM_ROOT)) usage();

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once 'XML/Tree.php';
require SQ_DATA_PATH.'/private/db/table_columns.inc';

// get the tables from table_columns into a var
// that will not clash with other vars
$SQ_TABLE_COLUMNS = $tables;

$tables = get_rollback_table_names();

// the number of rows to limit to as to avoid an out of memory error
// MUST be greater than 1
$LIMIT_ROWS = 500;

$db = &$GLOBALS['SQ_SYSTEM']->db;
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

foreach ($tables as $table) {

	if ($ENABLE_ROLLBACK) {
		open_rollback_entries($table, $ROLLBACK_DATE);
		continue;
	}
	if ($DISABLE_ROLLBACK) {
		close_rollback_entries($table, $ROLLBACK_DATE);
		continue;
	}
	if ($ROLLBACK_DATE) {
		delete_rollback_entries($table, $ROLLBACK_DATE);
		align_rollback_entries($table, $ROLLBACK_DATE);
		continue;
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');


/**
* Closes of rollback entries to the specified date
*
* @param string $table_name 	the tablename to update
* @param string $date 		the date to close to
*
* @return void
* @access public
*/
function close_rollback_entries($table_name, $date)
{
	global $db, $QUIET;

	$sql = 'UPDATE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' SET '.SQ_TABLE_PREFIX.'eff_to = '.$db->quote($date).
		' WHERE '.SQ_TABLE_PREFIX.'eff_to IS NULL';
	$result = $db->query($sql);
	assert_valid_db_result($result);
	$affected_rows = $db->affectedRows();
	assert_valid_db_result($affected_rows);

	if (!$QUIET) echo $affected_rows.' ENTRIES CLOSED IN '.SQ_TABLE_ROLLBACK_PREFIX.$table_name."\n";

}//end close_rollback_entries()


/**
* Opens any rollback entries that have not allready been opened
*
* @param string $table_name 	the tablename to update
* @param string $date 		the date to close to
*
* @return void
* @access public
*/
function open_rollback_entries($table_name, $date)
{
	global $SQ_TABLE_COLUMNS, $db, $QUIET;

	$columns = $SQ_TABLE_COLUMNS[$table_name]['columns'];
	$sql = 'INSERT INTO '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' ('.implode(', ', $columns).
		', '.SQ_TABLE_PREFIX.'eff_from, '.SQ_TABLE_PREFIX.'eff_to)
		SELECT '.implode(',', $columns).','.$db->quote($date).', NULL FROM '.SQ_TABLE_PREFIX.$table_name;

	$result = $db->query($sql);
	assert_valid_db_result($result);
	$affected_rows = $db->affectedRows();
	assert_valid_db_result($affected_rows);

	if (!$QUIET) echo $affected_rows.' ENTRIES OPENED IN '.SQ_TABLE_ROLLBACK_PREFIX.$table_name."\n";

}//end open_rollback_entries()


/**
* Aligns all the minimum eff_from entries in the specified rollback table so
* they all start at a specified date
*
* @param string $table_name 	the tablename to update
* @param string $date 		the date to close to
*
* @return void
* @access public
*/
function align_rollback_entries($table_name, $date)
{
	global $db, $QUIET;

	$sql = 'UPDATE '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.'
			SET '.SQ_TABLE_PREFIX.'eff_from = '.$db->quote($date).'
			WHERE '.SQ_TABLE_PREFIX.'eff_from < '.$db->quote($date);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	$affected_rows = $db->affectedRows();
	assert_valid_db_result($affected_rows);

	if (!$QUIET) echo $affected_rows.' ENTRIES ALIGNED IN '.SQ_TABLE_ROLLBACK_PREFIX.$table_name."\n";

}//end align_rollback_entries()


/**
* Deletes the rollback entries that started before the specified date
*
* @param string $table_name 	the tablename to update
* @param string $date 		the date to close to
*
* @return void
* @access public
*/
function delete_rollback_entries($table_name, $date)
{
	global $db, $QUIET;

	$sql = 'DELETE FROM '.SQ_TABLE_ROLLBACK_PREFIX.$table_name.' WHERE '
		.SQ_TABLE_PREFIX.'eff_to <= '.$db->quote($date);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	$affected_rows = $db->affectedRows();
	assert_valid_db_result($affected_rows);

	if (!$QUIET) echo $affected_rows.' ENTRIES DELETED IN '.SQ_TABLE_ROLLBACK_PREFIX.$table_name."\n";

}//end delete_rollback_entries()


/**
* Returns the table names of the database tables that require rollback
*
* @return Array()
* @access public
*/
function get_rollback_table_names()
{
	global $SYSTEM_ROOT;
	$table_names = Array();

	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	if (empty($packages_installed)) return Array();

	foreach ($packages_installed as $package_array) {
		if ($package_array['code_name'] == '__core__') {
			$table_file = SQ_CORE_PACKAGE_PATH.'/tables.xml';
		} else {
			$table_file = SQ_PACKAGES_PATH.'/'.$package_array['code_name'].'/tables.xml';
		}

		if (!file_exists($table_file)) continue;

		$tree = new XML_Tree($table_file);
		$root = &$tree->getTreeFromFile();

		if ($root->name != 'schema' || $root->children[0]->name != 'tables') {
			trigger_error('Invalid table schema for file "'.$table_file.'"', E_USER_ERROR);
		}

		$table_root = $root->children[0];

		for ($i = 0; $i < count($table_root->children); $i++) {
			if ($table_root->children[$i]->attributes['require_rollback']) {
				$table_name = $table_root->children[$i]->attributes['name'];
				array_push($table_names, $table_name);
			}
		}
	}

	return $table_names;

}//end get_rollback_table_names()


/**
* Prints the usage for this script and exits
*
* @access public
* @return void
*/
function usage()
{
	echo "\nUSAGE: rollback_management.php -s <system_root> [-d <date>] [-p <period>] [--enable-rollback] [--disable-rollback] [-q --quiet]\n".
		"--enable-rollback  Enables rollback in MySource Matrix\n".
		"--disable-rollback Disables rollback in MySource Matrix\n".
		"-q No output will be sent\n".
		"-d The date to set rollback entries to in the format YYYY-MM-DD HH:MM:SS\n".
		"-p The period to purge before in the format nx where n is the number of units and x is one of:\n".
		" h - hours\t\n d - days\t\n w - weeks\t\n m - months\t\n y - years\n".
		"\nNOTE: only one of [-d -p --enable-rollback --disable-rollback] option is allowed to be specified\n";
	exit();

}//end usage()


?>
