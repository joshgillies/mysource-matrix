<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ldap_user_group_edit_fns.inc,v 1.5 2005/08/16 23:58:08 dheppell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group_edit_fns.inc';

/**
* Ldap_User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
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

		$this->static_screens = Array(
									'details'		=> Array(
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
