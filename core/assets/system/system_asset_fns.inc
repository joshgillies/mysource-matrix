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
* $Id: system_asset_fns.inc,v 1.11 2006/12/05 23:40:32 bcaldwell Exp $
*
*/

/**
* Functions that are used by system assets to clean-up any issues that arrise because they are not "normal" assets.
* Ideally this would have been done with some sort of interface (or "implements") structure on the classes
* but this is not to be
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/


/**
* Called before a create to ensure that there is only one version of this type of asset
*
* @param object	&$asset	the system asset that we are attempting to create
*
* @return boolean
* @access public
*/
function system_asset_fns_create_pre_check(&$asset)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	$sql = 'SELECT COUNT(*)
			FROM '.SQ_TABLE_RUNNING_PREFIX.'ast
			';
	$where = 'type_code = '.$db->quote($asset->type());
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
	$exists = $db->getOne($sql.$where);
	assert_valid_db_result($exists);

	if ($exists) {
		trigger_localised_error('CORE0085', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'));
		return FALSE;
	}

	return TRUE;

}//end system_asset_fns_create_pre_check()


/**
* Called at the end of a create to remove any general asset stuff that we don't want as "System Assets"
*
* @param object	&$asset	the system asset that we are attempting to create
*
* @return boolean
* @access public
*/
function system_asset_fns_create_cleanup(&$asset)
{
	// remove any write and admin permissions that have been set on this asset and it's dependants
	$assetids = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);
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