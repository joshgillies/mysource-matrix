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
* $Id: ldap_user_group_edit_fns.inc,v 1.1.4.1 2004/10/07 04:48:49 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group_edit_fns.inc';

/**
* Ldap_User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Group_Edit_Fns extends User_Group_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ldap_User_Group_Edit_Fns()
	{
		$this->User_Group_Edit_Fns();

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
	* Paint the interface for editing group details
	*
	* @param object LDAP_User_Group		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean (false always)
	* @access public
	*/
	function paintDetails(&$asset, &$o, $prefix)
	{
		$info = $asset->vars;
		if (empty($info)) return;

		foreach ($info as $field_name => $field_data) {
			$count = count($field_data);
			if ($count > 1) $field_name .= ' ('.$count.' values)';
			$o->openField($field_name);
				echo implode('<br />', $field_data);
			$o->closeField();
		}

		// no committing allowed
		return false;

	}//end paintDetails()


	/**
	* Processes the interface for editing group details
	*
	* Editing the details of an LDAP group is not yet implemented
	*
	* @param object LDAP_User_Group		&$asset	the asset to which we belong
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