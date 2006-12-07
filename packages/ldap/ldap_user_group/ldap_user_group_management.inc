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
* $Id: ldap_user_group_management.inc,v 1.6 2006/12/07 01:00:46 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Group_Management
*
* @see Asset_Management
*
* @author  Greg Serwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Group_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ldap_User_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			echo 'UPGRADING LDAP USER GROUP TO v0.2';
			// move the data path data
			$bridge_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('ldap_bridge');
			foreach ($bridge_ids as $bridge_id) {
				$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridge_id);
				$groups = $bridge->getChildren($bridge_id);
				foreach ($groups as $groupid => $type) {
					if ($type != 'ldap_user_group') continue;
					$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($groupid, 'ldap_user_group');
					$dn = substr($asset->id, strpos($asset->id, ':') + 1);
					$old_data_path = $bridge->data_path.'/'.$dn;
					$old_data_path_public = $bridge->data_path_public.'/'.$dn;
					if (is_dir($old_data_path)) {
						copy_directory($old_data_path, $asset->data_path);
						delete_directory($old_data_path);
					}
					if (is_dir($old_data_path_public)) {
						copy_directory($old_data_path_public, $asset->data_path_public);
						delete_directory($old_data_path);
					}
				}
			}
		}

		return $res;

	}//end _upgrade()


}//end class

?>