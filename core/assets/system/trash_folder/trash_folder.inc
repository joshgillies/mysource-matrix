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
* $Id: trash_folder.inc,v 1.64 2013/08/25 23:20:24 asharpe Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Trash_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.64 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trash_Folder extends Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Trash_Folder($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Return either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(Array &$link)
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
	* Perform any additional processing required during the creation of this asset
	*
	* Trash folder grants public read access when it is created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set public read access in the DB so it comes up in the asset map
		// but we have to do this the hard way, because the system assets list won't
		// have been saved yet
		$public_userids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('public_user');
		return $GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $public_userids[0], SQ_PERMISSION_READ, TRUE);

	}//end _createAdditional()


	/**
	* Prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object	&$o	the Backend Outputter object
	*
	* @return void
	* @access public
	*/
	function paintBackend(Backend_Outputter $o)
	{
		$prefix = $this->getPrefix($this);
		if (isset($_REQUEST[$prefix.'_purge_complete']) && (int)$_REQUEST[$prefix.'_purge_complete'] === 1) {
			$o->addOnLoad('	if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) {
								parent.frames["sq_sidenav"].reload_assets(["'.$this->id.'"]);
							}');
		}

		parent::paintBackend($o);

	}//end paintBackend()


	/**
	* returns FALSE always because we don't want to be able to clone a trash folder
	*
	* @return boolean
	* @access public
	*/
	function canClone()
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
	function morph($new_type_code)
	{
		trigger_localised_error('CORE0091', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
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
	function setAttrValue($name, $value)
	{
		if ($name == 'name') {
			trigger_localised_error('CORE0111', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	* @see Asset::moveLinkPos()
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_localised_error('CORE0019', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return FALSE;

	}//end moveLinkPos()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(Asset $minor, $link_type, $exclusive)
	{
		if (!$this->id) return FALSE;
		if (!($minor instanceof Asset)) {
			return translate('minor_not_asset');
		}
		return TRUE;

	}//end canCreateLink()


	/**
	* Returns TRUE if the passed assetid has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess(Array $assetids=Array())
	{
		return TRUE;

	}//end readAccess()


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
	function writeAccess($lock_type='', Array $assetids=Array(), $only_workflow=TRUE)
	{
		return TRUE;

	}//end writeAccess()


	/**
	* Returns TRUE if the current user can purge the trash
	*
	* @return boolean
	* @access public
	*/
	function purgeAccess()
	{
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return TRUE;
		if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			return TRUE;
		}
		return FALSE;

	}//end purgeAccess()


}//end class

?>
