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
* $Id: workspace.inc,v 1.33 2012/08/30 01:09:17 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Workspace
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workspace extends Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Workspace($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Gets the user that this workspace is attached to
	*
	* @return object
	* @access private
	*/
	function _getUser()
	{
		$user = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user', FALSE, '', 'minor', '1');
		if (empty($link)) return $user;
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
		return $user;

	}//end _getUser()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		$user = $this->_getUser();

		if (is_null($user)) {
			return parent::_checkPermissionAccess($perm, $assetids);
		}

		return $user->_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


}//end class

?>
