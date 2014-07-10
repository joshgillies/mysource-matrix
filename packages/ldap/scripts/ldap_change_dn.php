<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ldap_change_dn.php,v 1.18 2013/01/11 01:39:55 cupreti Exp $
*
*/

/**
* Alter the database to reflect that the DN of a user has changed
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18 $
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
	// Change created ownership
	$bind_vars = Array(
					'old_userid'	=> $old_dn,
					'new_userid'	=> $new_dn,
				 );

	MatrixDAL::executeQuery('core', 'changeCreatedAssetDateUser',       $bind_vars);
	MatrixDAL::executeQuery('core', 'changeUpdatedAssetDateUser',       $bind_vars);
	MatrixDAL::executeQuery('core', 'changePublishedAssetDateUser',     $bind_vars);
	MatrixDAL::executeQuery('core', 'changeStatusChangedAssetDateUser', $bind_vars);
	MatrixDAL::executeQuery('core', 'changeLinkUpdatedDateUser',       $bind_vars);
	printActionStatus('OK');

	printActionName('Changing asset ownership (rollback)');
	$sql = 'UPDATE sq_rb_ast
			SET created_userid = '.MatrixDAL::quote($new_dn).'
			WHERE created_userid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);

	$sql = 'UPDATE sq_rb_ast
			SET updated_userid = '.MatrixDAL::quote($new_dn).'
			WHERE updated_userid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);

	$sql = 'UPDATE sq_rb_ast
			SET published_userid = '.MatrixDAL::quote($new_dn).'
			WHERE published_userid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);

	$sql = 'UPDATE sq_rb_ast
			SET status_changed_userid = '.MatrixDAL::quote($new_dn).'
			WHERE status_changed_userid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);

	$sql = 'UPDATE sq_rb_ast_lnk
			SET updated_userid = '.MatrixDAL::quote($new_dn).'
			WHERE updated_userid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);
	printActionStatus('OK');

	printActionName('Changing shadow links');
	// find out any links that has already be re-created..possibly from backend
	$sql = 'SELECT majorid FROM sq_shdw_ast_lnk where minorid = '.MatrixDAL::quote($new_dn);
	$existing_links = MatrixDAL::executeSqlAssoc($sql);

	foreach ($existing_links as $index => $existing_link) {
		$existing_links[] = $existing_links[$index]['majorid'];
		unset($existing_links[$index]);
	}

	// update the dn here but be sure to not to try to insert
	// duplicate entries incase the updated ldap user is already
	// linked to target asset from backend for more info see bug
	// 5686 LDAP update script fails if unique constraint violated
	$sql = 'UPDATE sq_shdw_ast_lnk
			SET minorid = '.MatrixDAL::quote($new_dn).'
			WHERE minorid = '.MatrixDAL::quote($old_dn).'
			AND majorid NOT IN (SELECT s.majorid FROM sq_shdw_ast_lnk s where s.minorid = '.MatrixDAL::quote($new_dn).')';
	$result = MatrixDAL::executeSql($sql);

	MatrixDAL::executeQuery('core', 'changeShadowLinkUpdatedDateUser', $bind_vars);
	printActionStatus('OK');

	printActionName('Changing shadow links (rollback)');
	$sql = 'UPDATE sq_rb_shdw_ast_lnk
			SET minorid = '.MatrixDAL::quote($new_dn).'
			WHERE minorid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);

	$sql = 'UPDATE sq_rb_shdw_ast_lnk
			SET updated_userid = '.MatrixDAL::quote($new_dn).'
			WHERE updated_userid = '.MatrixDAL::quote($old_dn);
	$result = MatrixDAL::executeSql($sql);
	printActionStatus('OK');

	printActionName('Changing asset permissions');
		$sql = 'UPDATE sq_ast_perm
				SET userid = '.MatrixDAL::quote($new_dn).'
				WHERE userid = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing asset permissions (rollback)');
		$sql = 'UPDATE sq_rb_ast_perm
				SET userid = '.MatrixDAL::quote($new_dn).'
				WHERE userid = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing internal messages');
		$sql = 'UPDATE sq_internal_msg
				SET userto = '.MatrixDAL::quote($new_dn).'
				WHERE userto = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);

		$sql = 'UPDATE sq_internal_msg
				SET userfrom = '.MatrixDAL::quote($new_dn).'
				WHERE userfrom = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing screen access');
		$sql = 'UPDATE sq_ast_edit_access
				SET userid = '.MatrixDAL::quote($new_dn).'
				WHERE userid = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing screen access (rollback)');
		$sql = 'UPDATE sq_rb_ast_edit_access
				SET userid = '.MatrixDAL::quote($new_dn).'
				WHERE userid = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing locks');
		$class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($class_name);

		try {
			eval('return '.$class_name.'::changeLockOwner($old_dn, $new_dn);');
		} catch (Exception $e) {
			trigger_error('Unable to change owner of existing locks, '.$e->getMessage(), E_USER_ERROR);
		}

	// ??? This doesn't look correct...
		$sql = 'UPDATE sq_lock
				SET lockid = '.MatrixDAL::quote('asset.'.$new_dn).'
				WHERE lockid = '.MatrixDAL::quote('asset.'.$old_dn);
		$result = MatrixDAL::executeSql($sql);

		$sql = 'UPDATE sq_lock
				SET source_lockid = '.MatrixDAL::quote('asset.'.$new_dn).'
				WHERE source_lockid = '.MatrixDAL::quote('asset.'.$old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	// update the sq_ast_attr val table for all the attributes value of type "assetid", coz if the old dn is not going to be available
	// henceforth then there is no point keeping it in our database
	// see #5608 ldap_change_dn.php does  not update running_as attribute  for cron jobs
	printActionName('Changing asset attributes ');

		$sql = 'UPDATE sq_ast_attr_val
				SET custom_val = '.MatrixDAL::quote($new_dn).'
				WHERE '.(MatrixDAL::getDbType() === 'oci' ? 'DBMS_LOB.SUBSTR(custom_val, 2000, 1)' : 'custom_val').' = '.MatrixDAL::quote($old_dn).'
				AND attrid IN (SELECT attrid FROM sq_ast_attr WHERE type=\'assetid\')';
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing asset roles');
		$sql = 'UPDATE sq_ast_role
				SET userid = '.MatrixDAL::quote($new_dn).'
				WHERE userid = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

	printActionName('Changing asset roles (rollback)');
		$sql = 'UPDATE sq_rb_ast_role
				SET userid = '.MatrixDAL::quote($new_dn).'
				WHERE userid = '.MatrixDAL::quote($old_dn);
		$result = MatrixDAL::executeSql($sql);
		printActionStatus('OK');

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');


if (!empty($existing_links)) {
	echo "Few links in sq_shdw_ast_lnk table were already updated prior to running script.\n";
	echo "Links for old DN under following parents need to be fixed manually\n";
	foreach ($existing_links as $index => $link) {
		echo "$index) $link\n";
	}
}
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
