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
* $Id: ldap_user_edit_fns.inc,v 1.2 2004/03/19 05:09:44 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user_edit_fns.inc';

/**
* Ldap_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Edit_Fns extends Backend_User_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ldap_User_Edit_Fns()
	{
		$this->Backend_User_Edit_Fns();

		$this->static_screens = Array(	'details'		=> Array(
															'name'			=> 'Details',
															'force_unlock'	=> true,
															'lock_type'		=> 'none',
															),
										'dependants'	=> Array(
															'name'			=> 'Dependants',
															'force_unlock'	=> true,
															'lock_type'		=> 'none',
															),
									);

	}//end constructor


	/**
	* Paint the interface for editing user details
	*
	* @param object LDAP_User			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean (false always)
	* @access public
	*/
	function paintDetails(&$asset, &$o, $prefix)
	{
		$info = $asset->getUserData();
		if (empty($info)) return;
		
		if (!$asset->_ldapid) return Array();
		$ldap_bridge = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return Array();

		$show = $ldap_bridge->attr('user_attributes');
		$show = $show['show'];

		for ($i = 0; $i < $info['count']; $i++) {
			if (!isset($show[$info[$i]])) continue;
			$field_name = $show[$info[$i]];

			if ($info[$info[$i]]['count'] > 1) $field_name .= ' ('.$info[$info[$i]]['count'].' values)';
			$o->openField($field_name);
			unset($info[$info[$i]]['count']);
			foreach ($info[$info[$i]] as $id => $value) {
				echo $value.'<br/>';
			}
		}

		// no committing allowed
		return false;

	}//end paintDetails()


	/**
	* Processes the interface for editing user details
	*
	* Editing the details of an LDAP user is not yet implemented
	*
	* @param object LDAP_User			&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean (false always)
	* @access public
	*/
	function processDetails(&$asset, &$o, $prefix)
	{
		return false;

	}//end processDetails()


}//end class

?>