<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_country_edit_fns.inc,v 1.4 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
				echo translate('All countries');

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
