<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: workspace.inc,v 1.22 2004/04/14 04:31:25 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Workspace
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	*   <li>TRUE if the asset was created but $link was empty</li>
	* </ul>
	*
	* @param Array	&$link	information used to create the initial link<br/>
	* <PRE>
	* Array ('asset'      => [ref major asset to create link under],
	*        'link_type'  => SQ_LINK_?,
	*        'value'      => [link value],
	*        'sort_order' => [link sort order],
	*        'dependant'  => [0|1],
	*        'exclusive'  => [0|1]
	*        )
	* </PRE>
	*
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		
		$username = $link['asset']->attr('username');
		
		// check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($username));
		$name = array_shift($valid_names);

		// we should theretically not be able to get into here
		if ($name != $username) {
			trigger_error('Username is not Unique', E_USER_WARNING);
			return false;
		}

		// make sure the new web path is not already is use
		$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
		if (!empty($bad_paths)) {
			trigger_error('Unable to create file with name "'.$name.'", web path already exists?', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', '~'.$username);

		return parent::create($link);
		
	}//end create()
	

	/**
	* Gets the user that this workspace is attached to
	*
	* @return object User
	* @access private
	*/
	function &_getUser()
	{
		$user = null;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user', false, '', 'minor', '1');
		if (empty($link)) return $user;
		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
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
		return false;

	}//end canDelete()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
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
		$user = &$this->_getUser();
		
		if (is_null($user)) {
			return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		}

		if (empty($assetids)) {
			// root can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			if ($GLOBALS['SQ_SYSTEM']->userRoot($user)) {
				// we have to be root to view root's workspace
				return false;
			} else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user)) {
				// this is the workspace of a sysadmin so they need to
				// be root or the sysadmin themselves
				if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;

				if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
				return false;
			} else {
				// any other user's workspace
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
				if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserId()) return true;
			}
		} else {
			// a user always has full access to their own workspace
			if (in_array($user->id, $assetids)) return true;

			return parent::_checkPermissionAccess($perm, $assetids);
		}

	}//end checkAccess()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		bam('WORKSPACE printBody(): NOT DONE YET');

	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return 'Workspace';

	}//end _getName();


	/**
	* event listener function that gets called when the user of this workspace gets updated
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	some information passed about the user
	*
	* @return boolean
	* @access public
	*/
	function onUsernameUpdated(&$broadcaster, $vars=Array())
	{
		if (empty($vars)) return false;
		if (!isset($vars['username'])) return false;
		if (!$this->setAttrValue('name', '~'.$vars['username'])) return false;
		
		return $this->saveAttributes(false);
	
	}//end onUsernameUpdated()


	/**
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;

		$is_old_path = false;
		if (isset($this->_tmp['vars_set']['name']['old_value'])) {
			$is_old_path = true;
			$old_path = $this->_tmp['vars_set']['name']['old_value'];
		}
		$new_path = $this->attr('name');

		// try and set the new filename - return false if it fails
		if (!parent::saveAttributes($dont_run_updated)) return false;

		// the name has changed so we need to save the web paths
		if (!$this->saveWebPaths(Array($new_path))) {
			trigger_error('Failed updating web path. Path already exists?', E_USER_WARNING);
			if ($is_old_path) parent::setAttrValue('name', $old_path);
			return false;
		}

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()

	
}//end class

?>