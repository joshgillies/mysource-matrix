<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_select_edit_fns.inc,v 1.6 2003/10/10 06:49:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_Select_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Select_Edit_Fns extends Form_Question_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Select_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();

	} // end Form_Question_Type_Select_Edit_Fns()


	/**
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function paintDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$options = $asset->getOptions();
		$multiple = ($asset->attr('multiple')) ? true : false;
		
		return combo_box($prefix.'_default', $options, $multiple, $asset->attr('default'));

	} // end paintDefault()


	/**
	* processes the default answer selected by the user
	*
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function processDefault(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_default'])) {
			$default = $_POST[$prefix.'_default'];
			if (!is_array($default)) {
				$default = Array($default);
			}
			if (!$asset->setAttrValue('default', $default)) {
				return false;
			}
		}
		return true;
	
	} // end processDefault()


}//end class
?>
