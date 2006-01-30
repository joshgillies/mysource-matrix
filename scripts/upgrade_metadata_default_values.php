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
* $Id: upgrade_metadata_default_values.php,v 1.4 2006/01/30 00:31:08 lwright Exp $
*
*/

/**
* Script to upgrade the metadata storage to take advantage of the new default
* metadata value table
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
* @since   MySource 3.5.0
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$db = &$GLOBALS['SQ_SYSTEM']->db;
$am = &$GLOBALS['SQ_SYSTEM']->am;
$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

$am->includeAsset('metadata_field');

// first check that the new table we need exists
$tables = $db->getListOf('tables');
assert_valid_db_result($tables);
if (!in_array('sq_ast_metadata_dflt_val', $tables)) {
	trigger_error('You need to run install/step_02.php to install the new table required for the new metadata storage', E_USER_ERROR);
}
unset($tables);

$metadata_fields = $am->getTypeAssetids('metadata_field', false, true);
foreach ($metadata_fields as $assetid => $type_code) {
	$asset = &$am->getAsset($assetid, $type_code);
	if (is_null($asset)) continue;

	$default = $asset->attr('default');
	$value_components = $asset->attr('value_components');
	$result = Metadata_Field::encodeValueString($default, $value_components);

	printName($asset->name.' (Id: #'.$asset->id.')');

	// if the default looks like a timestamp, let's update it
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	$sql = 'INSERT INTO
				sq_ast_metadata_dflt_val
				(
					assetid,
					default_val
				)
			VALUES
				(
					'.$db->quoteSmart($assetid).',
					'.$db->quoteSmart($result).'
				)';
	$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);
	assert_valid_db_result($result);

	$am->releaseLock($asset->id, 'attributes');

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	printUpdateStatus('OK');

}//end foreach

exit();


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


?>
