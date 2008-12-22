<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_select_edit_fns.inc,v 1.5.4.1 2008/12/22 23:42:44 bpearson Exp $
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
* @version $Revision: 1.5.4.1 $
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
			hidden_field($prefix.'_old_options['.$key.']', $value, 'id='.$prefix.'_old_options['.$key.']');
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
					// neaten up any numeric keys passing through
					if (is_numeric($str_key)) {
						$str_key = (int) $str_key;
					}
					$new_default_vals[] = $old_options[$str_key];
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

		}//end if (isset($_POST[$prefix.'_default']))
		return TRUE;

	}//end processDefault()


	/**
	* Paints the options for this question
	*
	* @param object	&$asset	Form_Question, the asset whose interface we are painting
	* @param object	&$o		Backend_Outputter, the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSelOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		require_once(SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc');
		$option_list = new Asset_Attribute_Option_List(0, $asset->attr('options'));
		$option_list->paint($prefix.'_option_list', !$write_access, FALSE);

	}//end paintSelOptions()


	/**
	* Processes the options for this question
	*
	* @param object	&$asset	Form_Question, the asset whose interface we are painting
	* @param object	&$o		Backend_Outputter, the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSelOptions(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_option_list_options'])) {
			require_once(SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc');
			$option_list = new Asset_Attribute_Option_List(0, $asset->attr('options'));
			$option_list->process($prefix.'_option_list', FALSE);

			$asset->setAttrValue('options', $option_list->value);
			$asset->saveAttributes();
		}//end if

	}//end processSelOptions()


}//end class
?>
