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
* $Id: form_question_type_select_edit_fns.inc,v 1.1.4.1 2007/03/15 03:34:30 skim Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';


/**
* Form_Question_Type_Select_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.1.4.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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

	}//end constructor


	/**
	* paints the default answer selected by the user
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$options = $asset->getOptions();
		foreach ($options as $key => $value) {
			hidden_field($prefix.'_old_options[]', $value, 'id='.$prefix.'_old_options['.$key.']');
		}
		$multiple = ($asset->attr('multiple')) ? TRUE : FALSE;
		return combo_box($prefix.'_default', $options, $multiple, $asset->attr('default'));

	}//end paintDefault()


	/**
	* processes the default answer selected by the user
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefault(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_default'])) {

			$default = $_POST[$prefix.'_default'];
			if (!is_array($default)) $default = Array($default);

			$old_options = array_get_index($_POST, $prefix.'_old_options', Array());

			if (!empty($old_options)) {
				$new_options = $asset->getOptions();
				$new_values = Array();
				foreach ($default as $str_key) {
					$new_default_vals[] = $old_options[(int) $str_key];
				}

				$new_default = Array();
				foreach ($new_default_vals as $val) {
					if (($idx = array_search($val, $new_options)) !== FALSE) {
						$new_default[] = $idx;
					}
				}
				if (!empty($new_default)) {
					$default = $new_default;
				}
			}

			if (!$asset->setAttrValue('default', $default)) {
				return FALSE;
			}

		}
		return TRUE;

	}//end processDefault()


}//end class
?>
