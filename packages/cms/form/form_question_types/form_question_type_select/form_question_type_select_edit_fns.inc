<?php

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
	}

	function paintDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

		$options = explode("\r\n", $asset->attr('options'));
		$multiple = ($asset->attr('multiple')) ? true : false;
		combo_box($prefix.'_default', $options, $multiple, $asset->attr('default'));

	}

	function processDefault(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_default'])) {
			if (!$asset->setAttrValue('default', $_POST[$prefix.'_default'])) {
				return false;
			}
		}
		return true;
	}


}//end class
?>
