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
* $Id: system_management_folder.inc,v 1.24 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* System_Management_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_Management_Folder extends Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Return either linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* returns FALSE always because we don't want to be able to clone the system management folder
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	* @see Asset::morph()
	*/
	public function morph($new_type_code)
	{
		trigger_localised_error('CORE0091', sprintf(translate('Unable to morph %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
		return FALSE;

	}//end morph()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		if ($name == 'name') {
			trigger_localised_error('CORE0111', sprintf(translate('You are not able to alter the name for the %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
			return FALSE;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param Asset	$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param string	$value		the value that is to be associated with this link
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if null or less than zero places at end of list
	* @param string	$dependant	'0' / '1' on whether the this asset is dependant on the
	*							asset that will be linked by this new link
	* @param string	$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*							to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	public function createLink(Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0')
	{
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
			trigger_localised_error('CORE0117', sprintf(translate('You cannot create a new link to the %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
			return 0;
		} else {
			return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		}

	}//end createLink()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	public function deleteLink($linkid)
	{
		trigger_localised_error('CORE0118', sprintf(translate('You cannot delete anything from the %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
		return FALSE;

	}//end deleteLink()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed_links[SQ_LINK_TYPE_1] = Array();

		$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
		foreach ($packages as $data) {
			$package = $data['code_name'];
			if ($package == '__core__') {
				require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
				$class = 'package_manager_core';
			} else {
				if(is_file(SQ_PACKAGES_PATH.'/'.$package.'/package_manager_'.$package.'.inc')) {
					require_once SQ_PACKAGES_PATH.'/'.$package.'/package_manager_'.$package.'.inc';
					$class = 'package_manager_'.$package;
				}
			}

			eval('$sys_asset_types = '.$class.'::getSystemAssetTypes();');
			foreach ($sys_asset_types as $type_code) {
					$allowed_links[SQ_LINK_TYPE_1][$type_code] = Array('card' => '1', 'exclusive' => TRUE);
			}//end foreach

		}//end foreach

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Returns TRUE if the passed assetid has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	public function writeAccess($lock_type='', $assetids=Array())
	{
		// only root can write to this folder so we add this extra checking
		// before going up to our parent to check access
		if (empty($assetids)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
				return parent::writeAccess($lock_type, $assetids);
			}
		} else {
			$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			if (in_array($root_user->id, $assetids)) {
				return parent::writeAccess($lock_type, $assetids);
			}
		}
		return FALSE;

	}//end writeAccess()

	/**
	* Provides extra checking and disable invalid status change on errors
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	* @param boolean	$run_updated	specify it we want to run _upated
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=TRUE, $run_updated=TRUE)
	{
		// left blank to overide processStatusChange
	}//end processStatusChange()

}//end class
?>
