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
** $Id: delete_redundent_shadow_links.php,v 1.1 2009/08/11 07:19:20 cupreti Exp $
*/

/**
* Remove all the unnecessary shadow asset links created due to bug #3828
*
* @author  Chiranjivi Upreti <cupreti@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (count($argv) != 2) {
	echo "Usage:\n";
	echo "delete_redundent_shadow_links.php <matrix_root>\n";
	exit;
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
    trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
//echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
//$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
    echo "ERROR: The root password entered was incorrect\n";
    exit();
}

$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

delete_redundent_shadow_links();

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

// End


/**
* Clears the db of shadow links of which major asset does not exists in system
*
*
* @return void
* @access public
*/
function delete_redundent_shadow_links()
{
	$db = MatrixDAL::getDb();
	$sql = "
		SELECT 
			linkid, majorid 
		FROM 
			sq_shdw_ast_lnk";

	try {
		$shadow_links = MatrixDAL::executeSqlAssoc($sql);
	} catch (Exception $e) {
		throw new Exception('Unable to get info from shadow link table: '.$e->getMessage());
	}
	
	echo "Deleting redundent shadow asset links...\n";
	
	foreach($shadow_links as $shadow_link) {
		$linkid = $shadow_link['linkid'];
		$majorid = $shadow_link['majorid'];

		$sql = "
			SELECT 
				assetid
			FROM
				sq_ast
			WHERE
				assetid = :majorid";
		
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'majorid', $majorid);
			$result = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get info from asset table: '.$e->getMessage());
		}
		
		// Major asset id in the link doesnt exits, hence delete the shadow link
		if (!count($result)) {
			$sql = "
				DELETE FROM
					sq_shdw_ast_lnk
				WHERE
					linkid = :del_linkid";
		
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'del_linkid', $linkid);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to delete the redundent shadow link: '.$e->getMessage());
			}			
		}		

	}// end foreach shadow link

	echo "Completed.\n";

}//delete_redundent_shadow_links()

?>
