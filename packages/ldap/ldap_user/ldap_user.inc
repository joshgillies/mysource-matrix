<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ldap_user.inc,v 1.25 2012/08/30 00:58:58 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user.inc';

/**
* Ldap_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User extends User
{

	/**
	* The ID of the LDAP bridge we are from
	*
	* @var int
	*/
	var $_ldapid = 0;


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the LDAP bridge that links to the LDAP directory for this user
	* @param array	$data	the LDAP entry for this user (contains all user data)
	*
	*/
	function Ldap_User($ldapid=0, $data=Array())
	{
		$this->_ser_attrs = TRUE;
		$this->_loadVars();

		if (!$ldapid || empty($data)) {
			return;
		}

		$ldap_bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return;

		$ldap_bridge->initUser($this, $data);


	}//end constructor

	
	/**
	* ldap user doesn't need to reload because it's shadow asset
	* otherwise parrent::reload will just break because it assumes asset is not shadow
	*
	* @return void
	* @access public
	*/
	function reload()
	{	
	}//end reload()
	

	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Returns name of the user
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return 'LDAP User';

	}//end _getName()


	/**
	* Check if this user can log in
	*
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin()
	{
		return TRUE;

	}//end canLogin()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$info = $this->vars;
		if (empty($info)) return;

		if (!$this->_ldapid) return Array();
		$ldap_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return Array();

		$null = NULL;
		$ldap_bridge->paintUserDetails($info, $null);

	}//end printBody()


	/**
	* Returns NULL as LDAP users do not have a workspace
	*
	* @return object
	* @access public
	*/
	function &getWorkspace()
	{
		$null = NULL;
		return $null;

	}//end getWorkspace()


	/**
	* Generate a prefix to be used by paint() and process()
	*
	* @return string
	* @access public
	*/
	public function getPrefix()
	{
		return str_replace(':', '_', str_replace(' ', '_', str_replace('.' , '_', $this->type().'_'.$this->id)));

	}//end getPrefix()


}//end class

?>
