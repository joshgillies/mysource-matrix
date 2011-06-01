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
* $Id: ldap_user_edit_fns.inc,v 1.15 2007/12/04 05:08:48 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user_edit_fns.inc';

/**
* Ldap_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Edit_Fns extends User_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ldap_User_Edit_Fns()
	{
		$this->User_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'linking'		=> Array(
														'name'			=> 'Linking',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'links',
													   ),
								);

	}//end constructor


	/**
	* Paint the interface for ldap_user details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDetails(&$asset, &$o, $prefix)
	{
		$info = $asset->vars;
		if (empty($info)) return;

		if (!$asset->_ldapid) return Array();

		$ldap_bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return Array();

		$ldap_bridge->paintUserDetails($info, $o);

		// no committing allowed
		return FALSE;

	}//end paintDetails()


	/**
	* Processes the interface for editing user details
	*
	* Editing the details of an LDAP user is not yet implemented
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDetails(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processDetails()


}//end class

?>
