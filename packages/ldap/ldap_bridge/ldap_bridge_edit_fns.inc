
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
* $Id: ldap_bridge_edit_fns.inc,v 1.3 2004/02/10 16:35:43 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge_edit_fns.inc';

/**
* LDAP_Bridge_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class LDAP_Bridge_Edit_Fns extends Bridge_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function LDAP_Bridge_Edit_Fns()
	{
		$this->Bridge_Edit_Fns();

		$this->static_screens = Array(	'details'		=> Array(
															'name'			=> 'Details',
															'force_unlock'	=> true,
															'lock_type'		=> 'attributes',
																),
										'permissions'	=> Array(
															'name'			=> 'Permissions',
															'force_unlock'	=> true,
															'lock_type'		=> 'permissions',
																),
										'dependants'	=> Array(
															'name'			=> 'Directory Tree',
															'force_unlock'	=> true,
															'lock_type'		=> 'none',
																),
										'linking'		=> Array(
															'name'			=> 'Linking',
															'force_unlock'	=> true,
															'lock_type'		=> 'links',
																),
										'history'		=> Array(
															'name'			=> 'History',
															'force_unlock'	=> true,
															'lock_type'		=> 'none',
																),
										'logs'			=> Array(
															'name'			=> 'Logs',
															'force_unlock'	=> true,
															'lock_type'		=> 'none',
																),
									);

	}//end constructor


	/**
	* Paint the interface for editing schema attributes
	*
	* @param object LDAP_Bridge			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAttributes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('attributes');
		$required = Array('uid'   => 'User ID',
						  'cn'    => 'Common Name',
						  'fn'    => 'First Name',
						  'sn'    => 'Last Name',
						  'email' => 'Email Address',
						  'ou'    => 'Group Name',
						);

		if ($write_access) require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		foreach ($required as $code => $name) {
			$o->openField($name);
				$default = (isset($current[$code])) ? $current[$code] : '';
				if ($write_access) {
					text_box($prefix.'_attributes['.$code.']', $default, 20);
				} else {
					echo $default;
				}
			$o->closeField();
		}

		return $write_access;

	}//end paintAttributes()


	/**
	* Processes the interface for editing schema attributes
	*
	* @param object LDAP_Bridge			&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAttributes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_attributes'])) {
			$new_attributes = Array();
			foreach ($_POST[$prefix.'_attributes'] as $code => $name) {
				$new_attributes[(string)$code] = (string)$name;
			}
			return $asset->setAttrValue('attributes', $new_attributes);
		}

		return false;

	}//end processAttributes()


}//end class

?>