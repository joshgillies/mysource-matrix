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
* $Id: ldap_user_group_edit_fns.inc,v 1.7 2006/12/07 01:00:46 bcaldwell Exp $
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
* @version $Revision: 1.7 $
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
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint the interface for editing group details
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

		foreach ($info as $field_name => $field_data) {
			$count = count($field_data);
			if ($count > 1) $field_name .= ' ('.$count.' values)';
			$o->openField($field_name);
				echo implode('<br />', $field_data);
			$o->closeField();
		}

		// no committing allowed
		return FALSE;

	}//end paintDetails()


	/**
	* Processes the interface for editing group details
	*
	* Editing the details of an LDAP group is not yet implemented
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
