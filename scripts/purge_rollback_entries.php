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
* $Id: purge_rollback_entries.php,v 1.3 2004/07/05 09:32:14 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Purges rollback entries from the given date
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

$tables = getRollbackTableNames();
echo 'Enter the date to purge to in the format: YYYY-MM-DD HH:MM:SS : ';
$date = rtrim(fgets(STDIN, 4094));

echo 'Delete all entries previous to '.date('d F Y h:i:sA', strtotime($date)).'? y/n? ';
$answer = rtrim(fgets(STDIN, 4094));

if (strtolower($answer) != 'y' && strtolower($answer) != 'yes') {
	echo 'Purging rollback entries aborted'."\n";
	exit();
}

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db = &$GLOBALS['SQ_SYSTEM']->db;

foreach ($tables as $tablename) {	
	$sql = 'DELETE FROM '.$tablename.' 
			WHERE '.SQ_TABLE_PREFIX.'effective_from < '.$db->quote($date).'
			  AND '.SQ_TABLE_PREFIX.'effective_to IS NOT NULL';
	$result = $db->query($sql);

	echo 'Deleting records from '.$tablename.'... ';
	
	if (DB::isError($result)) {
		trigger_error('DB Error:'.$result->getMessage(), E_USER_ERROR);
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	}

	echo $db->affectedRows().' rows deleted'."\n";
}

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
			array_push($table_names, SQ_TABLE_ROLLBACK_PREFIX.$table_name);
		}
	}
	return $table_names;

}//end getRollbackTableNames()

?>
