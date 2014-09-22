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
* $Id: authentication.inc,v 1.12 2012/08/30 01:09:10 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Authentication
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Authentication extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Authentication($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Morphs this asset to either one of its parent type or one of its children types
	*
	* Only succeeds if $type_code assets are allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CORE0091', sprintf(translate('Unable to morph %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
		return FALSE;

	}//end morph()


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
	* returns FALSE always because we don't want to be able to clone a authentication
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Given a username, attempts to locate and return a user object.
	*
	* @param string	$username	The username of the user to locate
	*
	* @return object
	* @access public
	*/
	function &locateUser($username)
	{
		$null = NULL;
		return $null;

	}//end locateUser()


	/**
	* Attempts to find a user that matches the passed username and password
	*
	* If the wrong password is supllied, the user account will be locked after
	* a system defined number of incorrect login attempts
	*
	* @param string	$username	the user name to search for
	* @param string	$password	the password to match
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateUser($username, $password)
	{
		$null = NULL;
		return $null;

	}//end authenticateUser()


	/**
	* Given a username supplied by HTTP, attempts to locate and return a user object
	*
	* This function is used when processing a pre-authenticated username supplied
	* via HTTP headers or server vars.
	*
	* @param string	$username	The username to search for
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &authenticateHttpUser($username)
	{
		$null = NULL;
		return $null;

	}//end authenticateHttpUser()


}//end class

?>