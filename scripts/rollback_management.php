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
* $Id: rollback_management.php,v 1.21 2008/05/16 07:00:56 mbrydon Exp $
*
*/


/**
* Adds entries into rollback tables where there are no entries. This will occur
* when rollback has been enabled sometime after the system was installed.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = 'd:p:s:q::f:';
$longopt = Array('enable-rollback', 'disable-rollback', 'reset-rollback');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);

if (empty($options[0])) usage();

$PURGE_FV_DATE = '';
$ROLLBACK_DATE = '';
$SYSTEM_ROOT = '';
$ENABLE_ROLLBACK = FALSE;
$DISABLE_ROLLBACK = FALSE;
$RESET_ROLLBACK = FALSE;
$QUIET = FALSE;

foreach ($options[0] as $option) {

	switch ($option[0]) {
		case 'd':
			if (!empty($ROLLBACK_DATE)) usage();
			if (empty($option[1])) usage();
			if (!preg_match('|^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}$|', $option[1])) {
				usage();
			}
			$ROLLBACK_DATE = $option[1];
		break;

		case 'p':
			if (!empty($ROLLBACK_DATE)) usage();
			if (empty($option[1])) usage();
			$matches = Array();
			if (!preg_match('|^(\d+)([hdwmy])$|', $option[1], $matches)) {
				usage();
			}

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
					$time_units = 'year';
				break;
			}
			if ($time_num > 1) $time_units .= 's';
			$ROLLBACK_DATE = date('Y-m-d H:i:s', strtotime('-'.$time_num.' '.$time_units));
		break;

		case 'f':
			if (!empty($PURGE_FV_DATE)) usage();
			if (empty($option[1])) usage();
			$matches = Array();
			if (!preg_match('|^(\d+)([hdwmy])$|', $option[1], $matches)) {
				usage();
			}

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
					$time_units = 'year';
				break;
			}
			if ($time_num > 1) $time_units .= 's';
			$PURGE_FV_DATE = date('Y-m-d H:i:s', strtotime('-'.$time_num.' '.$time_units));
		break;

		case 's':
			if (empty($option[1])) usage();
			if (!is_dir($option[1])) usage();
			$SYSTEM_ROOT = $option[1];
		break;

		case '--enable-rollback':
			if ($DISABLE_ROLLBACK || $RESET_ROLLBACK) {
				usage();
			}
			$ENABLE_ROLLBACK = TRUE;
		break;

		case '--disable-rollback':
			if ($ENABLE_ROLLBACK || $RESET_ROLLBACK) {
				usage();
			}
			$DISABLE_ROLLBACK = TRUE;
		break;

		case '--reset-rollback':
			if ($ENABLE_ROLLBACK || $DISABLE_ROLLBACK) {
				usage();
			}
			$RESET_ROLLBACK = TRUE;
		break;

		case 'q':
			$QUIET = TRUE;
		break;
		default:
			echo 'Invalid option - '.$option[0];
			usage();
	}//end switch

}//end foreach arguments

if ($ENABLE_ROLLBACK || $DISABLE_ROLLBACK || $RESET_ROLLBACK) {
	if (!empty($ROLLBACK_DATE) || !empty($PURGE_FV_DATE)) {
		usage();
	}
	$ROLLBACK_DATE = date('Y-m-d H:i:s');
}

if (!empty($ROLLBACK_DATE) && !empty($PURGE_FV_DATE)) {
	usage();
}

if (empty($SYSTEM_ROOT)) usage();

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/rollback_management.inc';
require SQ_DATA_PATH.'/private/db/table_columns.inc';

// get the tables from table_columns into a var
// that will not clash with other vars
$SQ_TABLE_COLUMNS = $tables;

$tables = get_rollback_table_names();

// the number of rows to limit to as to avoid an out of memory error
// MUST be greater than 1
$LIMIT_ROWS = 500;

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db = MatrixDAL::getDb();

if ($PURGE_FV_DATE) {
	$affected_rows = purge_file_versioning($PURGE_FV_DATE);
	if (!$QUIET) {
		echo $affected_rows.' FILE VERSIONING FILES AND ENTRIES DELETED'."\n";
	}
} else {

	if ($RESET_ROLLBACK) {
		// truncate toll back tables here then enable rollback
		foreach ($tables as $table) {
			truncate_rollback_entries($table);
			if (!$QUIET) {
				echo 'Rollback table sq_rb_'.$table." truncated.\n";
			}
		}
		$ENABLE_ROLLBACK = TRUE;
		echo "\nEnabling rollback...\n\n";
	}

	foreach ($tables as $table) {

		if ($ENABLE_ROLLBACK) {
			$affected_rows = open_rollback_entries($table, $SQ_TABLE_COLUMNS, $ROLLBACK_DATE);
			if (!$QUIET) {
				echo $affected_rows.' ENTRIES OPENED IN sq_rb_'.$table."\n";
			}

			continue;
		}
		if ($DISABLE_ROLLBACK) {
			$affected_rows = close_rollback_entries($table, $ROLLBACK_DATE);
			if (!$QUIET) {
				echo $affected_rows.' ENTRIES CLOSED IN sq_rb_'.$table."\n";
			}

			continue;
		}
		if ($ROLLBACK_DATE) {
			$affected_rows = delete_rollback_entries($table, $ROLLBACK_DATE);
			if (!$QUIET) {
				echo $affected_rows.' ENTRIES DELETED IN sq_rb_'.$table."\n";
			}

			$affected_rows = align_rollback_entries($table, $ROLLBACK_DATE);
			if (!$QUIET) {
				echo $affected_rows.' ENTRIES ALIGNED IN sq_rb_'.$table."\n";
			}

			continue;
		}//end if
	}//end foreach
}//end else

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


/**
* Prints the usage for this script and exits
*
* @return void
* @access public
*/
function usage()
{
	echo "\nUSAGE: rollback_management.php -s <system_root> [-d <date>] [-p <period>] [--enable-rollback] [--disable-rollback] [--reset-rollback] [-q --quiet]\n".
		"--enable-rollback  Enables rollback in MySource Matrix\n".
		"--disable-rollback Disables rollback in MySource Matrix\n".
		"--reset-rollback Removes all rollback information and enables rollback in MySource Matrix\n".
		"-q No output will be sent\n".
		"-d The date to set rollback entries to in the format YYYY-MM-DD HH:MM:SS\n".
		"-p The period to purge rollback entries before\n".
		"-f The period to purge file versioning entries and files before\n".
		"(For -p and -f, the period is in the format nx where n is the number of units and x is one of:\n".
		" h - hours\t\n d - days\t\n w - weeks\t\n m - months\t\n y - years\n".
		"\nNOTE: only one of [-d -p -f --enable-rollback --disable-rollback --reset-rollback] option is allowed to be specified\n";
	exit();

}//end usage()


?>
