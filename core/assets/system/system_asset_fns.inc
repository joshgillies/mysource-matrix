<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: system_asset_fns.inc,v 1.1 2003/10/15 11:05:14 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Functions that are used by system assets to clean-up any issues that arrise because they are not "normal" assets.
* Ideally this would have been done with some sort of interface (or "implements") structure on the classes 
* but this is not to be
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Called before a create to ensure that there is only one version of this type of asset
*
* @param object Asset	&$asset	the system asset that we are attempting to create
*
* @return boolean
* @access public
*/
function system_asset_fns_create_pre_check(&$asset)
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;

	$sql = 'SELECT COUNT(*) 
			FROM '.SQ_TABLE_RUNNING_PREFIX.'asset 
			';
	$where = 'type_code = '.$db->quote($asset->type());
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
	$exists = $db->getOne($sql.$where);
	if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

	if ($exists) {
		trigger_error('Unable to create another "'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name').'", it is a System Asset and you are only allowed one', E_USER_WARNING);
		return false;
	}

	return true;

}//end system_asset_fns_create_pre_check()


/**
* Called at the end of a create to remove any general asset stuff that we don't want as "System Assets"
*
* @param object Asset	&$asset	the system asset that we are attempting to create
*
* @return boolean
* @access public
*/
function system_asset_fns_create_cleanup(&$asset)
{
	//// Remove any write and admin permissions that have been set on this asset and it's dependants ////
	$assetids = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);
	foreach($assetids as $assetid) {
		foreach (Array(SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
			$unset_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, $perm, null, false, false, true);
			foreach ($unset_perms as $userid => $access) {
#				echo "REMOVE $assetid : $perm -> $userid : $access\n";
				if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($assetid, $userid, $perm)) return false;
			}// end foreach
		}// end foreach
	}// end foreeach

	return true;

}//end system_asset_fns_create_cleanup()



?>