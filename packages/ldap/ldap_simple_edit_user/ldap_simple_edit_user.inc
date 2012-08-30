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
* $Id: ldap_simple_edit_user.inc,v 1.4 2012/08/30 00:58:58 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ldap/ldap_user/ldap_user.inc';

/**
* Ldap_Simple_Edit_User
*
* Purpose
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_Simple_Edit_User extends Ldap_User
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
	function Ldap_Simple_Edit_User($ldapid=0, $data=Array())
	{
		$this->Ldap_User($ldapid, $data);

	}//end constructor


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
		return 'LDAP Simple Edit User';

	}//end _getName()


	/**
	* Check if this user can access the backend interface
	*
	* This will be disabled by default for a standard user, and enabled for
	* users who are allowed backend access (eg. backend user).
	*
	* @return boolean
	* @access public
	*/
	function canAccessBackend()
	{	
		// Simple_Edit_User must not access the backend interface however it can access limbo
		// This function is called before limbo access is granted because traditionally, backend access is
		// required for limbo
		// Permit 'backend' access only if this user is requesting a limbo page
		// Allow simple edit user to have backend interface access when in cron
		// we also have to consider the EES/JS API. Here the State wouldnt be in SQ_IN_LIMBO
		// but never the less we are in simple edit. 
		// Lets be a bit chicky here and check the referer to decide if we are indeed in LIMBO
		// We also have to make sure that the user we are currently deling with isnt the user
		// who initiated the workflow. or else he deserves to be known
		$in_limbo = $workflow_starter = FALSE;
		if(isset($_SERVER['HTTP_REFERER']) && !empty($_SERVER['HTTP_REFERER'])) {
			$in_limbo = (strpos($_SERVER['HTTP_REFERER'],SQ_CONF_LIMBO_SUFFIX) !== FALSE) ? TRUE : FALSE;
		}

		// we are not in limbo or EES see if the user was a workflow starter and if the workflow has been canceled
		// see workflow_manager::cancelWorkflow()
		if(!$in_limbo && (isset($this->_tmp['starter_of_workflow']) && $this->_tmp['starter_of_workflow'])) $workflow_starter = TRUE;

		return (SQ_IN_LIMBO || SQ_IN_CRON || $in_limbo || $workflow_starter);

	}//end canAccessBackend()
	
	
	/**
	* Because this asset does not extend the Matrix Simple Edit User, is this a Shadow Simple Edit User?
	* This is a type check only. DO NOT use to grant access to Simple Edit mode.
	* 
	* @return boolean
	* @access public
	*/
	function isShadowSimpleEditUser()
	{
		return TRUE;
		
	}//end isShadowSimpleEditUser()


}//end class

?>