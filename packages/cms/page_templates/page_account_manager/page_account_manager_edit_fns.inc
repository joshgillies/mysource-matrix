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
* $Id: page_account_manager_edit_fns.inc,v 1.6 2004/07/28 01:41:54 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_builder/page_asset_builder_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager_Edit_Fns extends Page_Asset_Builder_Edit_Fns
{

	/**
	* Messages used to provide info about the differnt types of create locations
	* @var array(string => string)
	*/
	var $location_messages = Array(
						'mandatory' => 'All new user accounts will be placed into these user groups',
						'optional'  => 'When creating a new account, the user will have the option of placing themselves into one or more of these user groups',
						'rules'     => 'These rules define where new user accounts should be created based on the values filled in by the user',
										);

	/**
	* An array of asset types that can be passed to asset_finder to define what types of create locations are allowed
	* @var array(string => string)
	*/
	var $location_types = Array('user_group' => 'D');


	/**
	* Constructor
	*
	*/
	function Page_Account_Manager_Edit_Fns()
	{
		$this->Page_Asset_Builder_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for choosing the user type that is created
	*
	* @param object Page_Account_Manager	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter		&$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			parent::paintAddCreateType($asset, $o, $prefix);
		} else {
			$user_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user');
		
			$options = Array('user' => 'User');
			foreach ($user_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_create_type', $options, false, array_keys($asset->attr('create_type')));
		}

	}//end paintAddCreateType()

	
	/**
	* Process the section for choosing the user type that is created
	*
	* This is overridden from Asset Builder because it makes sense for the account
	* manager to still be only allowed to create one type of user from the frontend interface.
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddCreateType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_create_type'])) {
			$type = trim($_POST[$prefix.'_create_type']);
			if (empty($type)) return false;
			$types = Array($type => true);
			$asset->setAttrValue('create_type', $types);
			return true;
		}
		return false;

	}//end processAddCreateType()
	

	/**
	* Returns true if the account manager uses email validation
	*
	* @return boolean
	* @access public
	*/
	function usingEmailValidation(&$asset)
	{
		return $asset->attr('use_email_validation');

	}//end usingEmailValidation()


}//end class

?>