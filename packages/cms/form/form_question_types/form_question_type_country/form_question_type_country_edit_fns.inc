<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_country_edit_fns.inc,v 1.2 2003/10/10 06:49:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form_question_type_select/form_question_type_select_edit_fns.inc';

/**
* Form_Question_Type_Country_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Country_Edit_Fns extends Form_Question_Type_Select_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Country_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();
	
	} // end Form_Question_Type_Country_Edit_Fns()


	/**
	* print the availiable countries
	*
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function paintAvailableCountries(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$options = $asset->getCompleteOptions();
		$selected_options = (count($asset->attr('available'))) ? $asset->attr('available') :  array_keys($options);
		
		return combo_box($prefix.'_available', $options, true, $selected_options);
	
	} // end printAvailableCountries()


	/**
	* processes the availiable countries
	*
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function processAvailableCountries(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_available'])) {
			$available = $_POST[$prefix.'_available'];
			if (!is_array($available)) {
				$available = Array($available);
			}
			if (!$asset->setAttrValue('available', $available)) {
				return false;
			}
		}
		return true;
	
	} // end processAvailableCountries()

}//end class
?>
