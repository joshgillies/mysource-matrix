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
* $Id: upgrade_design_linking.php,v 1.5 2006/12/06 05:39:52 bcaldwell Exp $
*
*/

/**
* Convert TYPE_3 linked designs to NOTICE
*
* @author  Greg Sherwood <greg@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix
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

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$db = &$GLOBALS['SQ_SYSTEM']->db;
$am = &$GLOBALS['SQ_SYSTEM']->am;

// first update the sq_ast_lnk table
printName('Converting asset links');
$sql = 'UPDATE sq_ast_lnk SET link_type = '.SQ_LINK_NOTICE.'
		WHERE minorid IN (
						  SELECT a.assetid FROM sq_ast a INNER JOIN sq_ast_lnk l ON a.assetid = l.minorid
						  WHERE l.link_type = '.SQ_LINK_TYPE_3.'
						    AND a.type_code IN (\'design\', \'design_customisation\')
						  )
		  AND link_type = '.SQ_LINK_TYPE_3;
$result = $db->query($sql);
assert_valid_db_result($result);
printUpdateStatus('OK');

// now do the rollback version of the table
printName('Converting asset rollback links');
$sql = 'UPDATE sq_rb_ast_lnk SET link_type = '.SQ_LINK_NOTICE.'
		WHERE minorid IN (
						  SELECT a.assetid FROM sq_rb_ast a INNER JOIN sq_rb_ast_lnk l ON a.assetid = l.minorid
						  WHERE l.link_type = '.SQ_LINK_TYPE_3.'
						    AND a.type_code IN (\'design\', \'design_customisation\')
						  )
		  AND link_type = '.SQ_LINK_TYPE_3;
$result = $db->query($sql);
assert_valid_db_result($result);
printUpdateStatus('OK');

// now clean out the sq_ast_lnk_tree
printName('Converting link tree');
$sql = 'SELECT DISTINCT t.treeid
		FROM sq_ast_lnk l INNER JOIN sq_ast a ON l.minorid = a.assetid
		  INNER JOIN sq_ast_lnk_tree t ON t.linkid = l.linkid
		WHERE l.link_type = '.SQ_LINK_NOTICE.'
		  AND a.type_code IN (\'design\', \'design_customisation\')';
$treeids = $db->getCol($sql);
assert_valid_db_result($result);

$delete_sql = 'DELETE FROM sq_ast_lnk_tree WHERE treeid LIKE ';
$update_sql = 'UPDATE sq_ast_lnk_tree SET linkid = 0 WHERE treeid = ';
foreach ($treeids as $treeid) {
	$result = $db->query($delete_sql.$db->quote($treeid.'%'));
	assert_valid_db_result($result);

	$result = $db->query($update_sql.$db->quote($treeid));
	assert_valid_db_result($result);
}

printUpdateStatus('OK');

printName('Converting rollback link tree');
$sql = 'SELECT DISTINCT t.treeid
		FROM sq_rb_ast_lnk l INNER JOIN sq_rb_ast a ON l.minorid = a.assetid
		  INNER JOIN sq_rb_ast_lnk_tree t ON t.linkid = l.linkid
		WHERE l.link_type = '.SQ_LINK_NOTICE.'
		  AND a.type_code IN (\'design\', \'design_customisation\')';
$treeids = $db->getCol($sql);
assert_valid_db_result($result);

$delete_sql = 'DELETE FROM sq_rb_ast_lnk_tree WHERE treeid LIKE ';
$update_sql = 'UPDATE sq_rb_ast_lnk_tree SET linkid = 0 WHERE treeid = ';
foreach ($treeids as $treeid) {
	$result = $db->query($delete_sql.$db->quote($treeid.'%'));
	assert_valid_db_result($result);

	$result = $db->query($update_sql.$db->quote($treeid));
	assert_valid_db_result($result);
}
printUpdateStatus('OK');

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
