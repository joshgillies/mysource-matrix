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
* $Id: form_question_type_country_edit_fns.inc,v 1.9 2005/04/29 05:40:29 gsherwood Exp $
*
*/


require_once dirname(__FILE__).'/../form_question_type_select/form_question_type_select_edit_fns.inc';

/**
* Form_Question_Type_Country_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.9 $
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

	}//end constructor


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

		$options = $asset->getCompleteOptions();
		$selected_options = $asset->attr('available');
		//$selected_options = unserialize((count($asset->attr('available'))) ? $asset->attr('available') :  array_keys($options));


		if (!$asset->writeAccess('attributes')) {
			if (count($selected_options)) {
				// gather all the names of the countries available
				$sel_option_names = Array();
				foreach ($selected_options as $selected_option) {
					$sel_option_names[] = $options[$selected_option];
				}
				echo implode('<br />', $sel_option_names);
			} else {
				echo 'All countries';
			}
		} else {
			combo_box($prefix.'_available', $options, true, $selected_options);
		}

		return $asset->writeAccess('attributes');

	}//end paintAvailableCountries()


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

	}//end processAvailableCountries()


}//end class
?>
