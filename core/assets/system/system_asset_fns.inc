<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: system_asset_fns.inc,v 1.16 2012/08/30 01:09:09 ewang Exp $
*
*/

/**
* Functions that are used by system assets to clean-up any issues that arrise because they are not "normal" assets.
* Ideally this would have been done with some sort of interface (or "implements") structure on the classes
* but this is not to be
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/


/**
* Called before a create to ensure that there is only one version of this type of asset
*
* @param Asset	$asset	the system asset that we are attempting to create
*
* @return boolean
* @access public
*/
function system_asset_fns_create_pre_check(Asset $asset)
{
	$db = DAL::getDb();

	$sql = 'SELECT COUNT(*)
			FROM '.SQ_TABLE_RUNNING_PREFIX.'ast
			';
	$where = 'type_code = :type_code';
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
	$sql  .= $where;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'type_code', $asset->type(), PDO::PARAM_STR);
		$exists = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to check for existing system asset of type "'.$type_code.'" due to database error: '.$e->getMessage());
	}

	if ($exists) {
		trigger_localised_error('CORE0085', sprintf(translate('Unable to create another "%s", it is a System Asset and you are only allowed one'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name')), E_USER_WARNING)
		return FALSE;
	}

	return TRUE;

}//end system_asset_fns_create_pre_check()


/**
* Called at the end of a create to remove any general asset stuff that we don't want as "System Assets"
*
* @param Asset	$asset	the system asset that we are attempting to create
*
* @return boolean
* @access public
*/
function system_asset_fns_create_cleanup(Asset $asset)
{
	// remove any write and admin permissions that have been set on this asset and it's dependants
	$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id));
	foreach ($assetids as $assetid) {
		foreach (Array(SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
			$unset_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, $perm, NULL, FALSE, FALSE, TRUE);
			foreach ($unset_perms as $userid => $access) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($assetid, $userid, $perm)) {
					return FALSE;
				}
			}
		}
	}

	return TRUE;

}//end system_asset_fns_create_cleanup()


?>