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
* $Id: upgrade_file_versioning.php,v 1.3 2004/11/16 03:53:38 lwright Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Upgrade menu design areas
*
* @author  Greg Sherwood <greg@squiz.net>
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

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));
	
// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

$db = &$GLOBALS['SQ_SYSTEM']->db;

// rename each file versioning related table
foreach(Array('file_versioning_file', 'file_versioning_file_history', 'file_versioning_file_lock') as $table_name) {
	printName('Rename "'.$table_name.'"');
	
	// find out of the table exists by trying to run a query on it and see whether it
	// returns a 'no such table' error. This should not be too much penalty because
	// most optimisers recognise the impossible WHERE clause and short-circuit it
	$result = $db->query('SELECT * FROM '.SQ_TABLE_PREFIX.$table_name.' WHERE 1=0');
	if (DB::isError($result) && ($result->getCode() == DB_ERROR_NOSUCHTABLE)) {
		// "No Such Table" error = the renamed table doesn't exist
		$result = $db->query('ALTER TABLE fudge_'.$table_name.' RENAME TO '.SQ_TABLE_PREFIX.$table_name);
		
		if (DB::isError($result) && ($result->getCode() == DB_ERROR_NOSUCHTABLE)) {
			// old table does not exist!!
			printUpdateStatus('FAIL DNE');
		} else if (DB::isError($result)) {
			// miscellaneous error
			printUpdateStatus('FAIL');
			print '('.$result->getMessage().")\n";
		} else {
			// table successfully renamed
			printUpdateStatus('OK');
		}

	} else if (DB::isError($result)) {
		// miscellaneous error
		printUpdateStatus('FAIL');
		print '('.$result->getMessage().")\n";
	} else {
		// no error = table already exists!
		printUpdateStatus('ALREADY EXISTS');
	
	}
	
}


// "No Such Table" error = the renamed table doesn't exist
printName('Drop repository field');
$result = $db->query('ALTER TABLE '.SQ_TABLE_PREFIX.'file_versioning_file DROP COLUMN repository');
	
if (DB::isError($result) && ($result->getCode() == DB_ERROR_NOSUCHFIELD)) {
	// old table does not exist!!
	printUpdateStatus('FAIL FIELD DNE');
} else if (DB::isError($result) && ($result->getCode() == DB_ERROR_NOSUCHTABLE)) {
	// old table does not exist!!
	printUpdateStatus('FAIL TABLE DNE');
} else if (DB::isError($result)) {
	// miscellaneous error
	printUpdateStatus('FAIL');
	print '('.$result->getMessage().")\n";
} else {
	// field successfully dropped
	printUpdateStatus('OK');
}



  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(40 - strlen($name)).'s', $name, ''); 
	
}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";
	
}//end printUpdateStatus()


?>
