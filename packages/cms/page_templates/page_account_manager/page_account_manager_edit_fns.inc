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
* $Id: page_account_manager_edit_fns.inc,v 1.1 2003/11/27 02:48:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';
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
class Page_Account_Manager_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Standard_Edit_Fns()
	{
		$this->Page_Edit_Fns();

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
	function paintCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->attr('create_type'));
			echo $info['name'];
		} else {
			$user_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user');
		
			$options = Array('user' => 'User');
			foreach ($user_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_create_type', $options, false, $asset->attr('create_type'));
		}

	}//end paintCreateType()


	/**
	* Processes the interface for choosing the user type that is created
	*
	* @param object Page_Account_Manager	$asset	the asset to which we belong
	* @param object	Backend_Outputter		$o		the outputter class
	* @param string							$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateType(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_type'])) return false;
		$create_type = trim($_POST[$prefix.'_create_type']);
		return $asset->setAttrValue('create_type', $create_type);

	}//end processCreateType()


}//end class

?>