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
* $Id: form_question_type_country_edit_fns.inc,v 1.4 2003/11/26 00:51:28 gsherwood Exp $
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
