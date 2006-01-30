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
* $Id: ldap_user_edit_fns.inc,v 1.11 2006/01/30 04:54:02 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user_edit_fns.inc';

/**
* Ldap_User_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
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
	* Paint the interface for editing user details
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
		$ldap_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return Array();

		$show = $ldap_bridge->attr('user_attributes');
		if (!isset($show['show'])) {
			$show = Array();
		} else {
			$show = $show['show'];
		}

		foreach ($info as $field_name => $field_data) {
			if (!isset($show[$field_name])) continue;
			$field_name = $show[$field_name];
			$count = count($field_data['value']);
			if ($count > 1) $field_name .= ' ('.$count.' values)';

			$o->openField($field_name);
				if (isset($field_data['value']) && is_array($field_data['value'])) {
					$field_data['value'] = implode('<br />', $field_data['value']);
				}

				echo $field_data['value'];

				// TODO: Leaving this in by request, until we are sure all implementations work
				echo '<br />';
				if (isset($field_data['attrid'])) {
					echo '<span style="color:#666">(attrid = '.$field_data['attrid'].')</span> ';
				}
				echo '<span style="color:#666">(type = '.$field_data['type'].')</span>';

			$o->closeField();
		}

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
