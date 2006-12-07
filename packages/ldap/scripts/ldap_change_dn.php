<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ldap_change_dn.php,v 1.11 2006/12/07 01:00:47 bcaldwell Exp $
*
*/

/**
* Alter the database to reflect that the DN of a user has changed
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix
* @subpackage ldap
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

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

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed loging in as root user\n", E_USER_ERROR);
}

// get a list of all LDAP Bridges to help the user select the correct bridge ID
$bridge_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('ldap_bridge', true);
$bridge_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($bridge_ids, 'ldap_bridge');

// ask for the bridge ID
echo "\n*** The following bridges are available in the system ***\n";
foreach ($bridge_info as $assetid => $asset_info) {
	echo "[$assetid] - ".$asset_info['name']."\n";
}
echo 'Enter the ID of the bridge to apply changes to: ';
$bridge_id = rtrim(fgets(STDIN, 4094));
if (!in_array($bridge_id, $bridge_ids)) {
	echo "Supplied bridge ID was not valid. No DN changes were made\n";
	exit();
}

// ask for the old DN
echo 'Enter the DN to change: ';
$old_dn = rtrim(fgets(STDIN, 4094));

// ask for the new DN
echo 'Enter the new DN: ';
$new_dn = rtrim(fgets(STDIN, 4094));


echo "\n*** Please confirm the following information is correct ***\n";
echo "[BRIDGE] $bridge_id\n";
echo "[OLD DN] $old_dn\n";
echo "[NEW DN] $new_dn\n";
echo 'Is this correct [y/n]: ';
$confirm = rtrim(fgets(STDIN, 4094));

if (strtolower($confirm) != 'y') {
	echo "No DN changes were made\n";
	exit();
}
echo "\n";

$old_dn = $bridge_id.':'.$old_dn;
$new_dn = $bridge_id.':'.$new_dn;

$db =& $GLOBALS['SQ_SYSTEM']->db;
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


	printActionName('Changing asset ownership');
	$sql = 'UPDATE sq_ast
			SET created_userid = '.$db->quote($new_dn).'
			WHERE created_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_ast
	        SET updated_userid = '.$db->quote($new_dn).'
			WHERE updated_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_ast
	        SET published_userid = '.$db->quote($new_dn).'
			WHERE published_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_ast
	        SET status_changed_userid = '.$db->quote($new_dn).'
			WHERE status_changed_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_ast_lnk
	        SET updated_userid = '.$db->quote($new_dn).'
			WHERE updated_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	printActionStatus('OK');


	printActionName('Changing asset ownership (rollback)');
	$sql = 'UPDATE sq_rb_ast
			SET created_userid = '.$db->quote($new_dn).'
			WHERE created_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_rb_ast
			SET updated_userid = '.$db->quote($new_dn).'
			WHERE updated_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_rb_ast
			SET published_userid = '.$db->quote($new_dn).'
			WHERE published_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_rb_ast
			SET status_changed_userid = '.$db->quote($new_dn).'
			WHERE status_changed_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_rb_ast_lnk
			SET updated_userid = '.$db->quote($new_dn).'
			WHERE updated_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing shadow links');
	$sql = 'UPDATE sq_shdw_ast_lnk
			SET minorid = '.$db->quote($new_dn).'
			WHERE minorid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_shdw_ast_lnk
	        SET updated_userid = '.$db->quote($new_dn).'
			WHERE updated_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing shadow links (rollback)');
	$sql = 'UPDATE sq_rb_shdw_ast_lnk
		    SET minorid = '.$db->quote($new_dn).'
			WHERE minorid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);

	$sql = 'UPDATE sq_rb_shdw_ast_lnk
			SET updated_userid = '.$db->quote($new_dn).'
			WHERE updated_userid = '.$db->quote($old_dn);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing asset permissions');
		$sql = 'UPDATE sq_ast_perm
				SET userid = '.$db->quote($new_dn).'
				WHERE userid = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing asset permissions (rollback)');
		$sql = 'UPDATE sq_rb_ast_perm
				SET userid = '.$db->quote($new_dn).'
				WHERE userid = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing internal messages');
		$sql = 'UPDATE sq_internal_msg
				SET userto = '.$db->quote($new_dn).'
				WHERE userto = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'UPDATE sq_internal_msg
				SET userfrom = '.$db->quote($new_dn).'
				WHERE userfrom = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing screen access');
		$sql = 'UPDATE sq_ast_edit_access
				SET userid = '.$db->quote($new_dn).'
				WHERE userid = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing screen access (rollback)');
		$sql = 'UPDATE sq_rb_ast_edit_access
				SET userid = '.$db->quote($new_dn).'
				WHERE userid = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);
	printActionStatus('OK');

	printActionName('Changing locks');
		$sql = 'UPDATE sq_lock
				SET userid = '.$db->quote($new_dn).'
				WHERE userid = '.$db->quote($old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'UPDATE sq_lock
				SET lockid = '.$db->quote('asset.'.$new_dn).'
				WHERE lockid = '.$db->quote('asset.'.$old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'UPDATE sq_lock
				SET source_lockid = '.$db->quote('asset.'.$new_dn).'
				WHERE source_lockid = '.$db->quote('asset.'.$old_dn);
		$result = $db->query($sql);
		assert_valid_db_result($result);
	printActionStatus('OK');

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');


/**
* Prints the name of the action currently being performed as a padded string
*
* Pads action to 40 columns
*
* @param string	$str	the action being performed
*
* @return void
* @access public
*/
function printActionName($str)
{
	printf ('%s%'.(40 - strlen($str)).'s', $str,'');

}//end printActionName()


/**
* Prints the status of the performed action
*
* @param string	status	the status of the actions
*
* @return void
* @access public
*/
function printActionStatus($status)
{
	echo "[ $status ]\n";

}//end printActionStatus()


?>
