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
* $Id: form_question_type_tickbox_list_edit_fns.inc,v 1.7 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';


/**
* Form_Question_Type_Tickbox_List_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Tickbox_List_Edit_Fns extends Form_Question_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Tickbox_List_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();

	}//end constructor


	/**
	* paints a section to choose the default for this question
	*
	* @param object Form_Question		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$options = $asset->getOptions();
		$multiple = (get_class($asset) == 'Form_Question_Type_Tickbox_List') ? TRUE : FALSE;
		$defaults = $asset->attr('default');

		foreach ($options as $ref => $option) {
			if (!trim($option)) continue;

			$type = ($multiple) ? 'check_box' : 'radio_button';
			$name = $prefix.'default[]';

			$checked = (in_array($ref, $defaults)) ? TRUE : FALSE;
			$type($name, $ref, $checked, '', 'id="'.$prefix.'default'.$ref.'"');
			label($option, $prefix.'default'.$ref);
			echo '<br />';
		}

		return TRUE;

	}//end paintDefault()


	/**
	* processes the section to choose the default for this question
	*
	* @param object Form_Question		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefault(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'default']) && $_POST[$prefix.'default'][0] == 'none') {
			$asset->setAttrValue('default', Array());
		}
		else if (isset($_POST[$prefix.'default'])) {
			if (!$asset->setAttrValue('default', $_POST[$prefix.'default'])) {
				return FALSE;
			}
		// if nothing has been submitted, then nothing has been selected, so blank all defaults out
		} else {
			$asset->setAttrValue('default', Array());
		}
		$asset->saveAttributes();

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
	function paintOptions(&$asset, &$o, $prefix)
	{

		$write_access = $asset->writeAccess('attributes');
		if ($write_access) {
			?>
			<label class="sq-option-list-header" for="<?php echo $prefix; ?>_option_list_options[0]"><?php echo translate('Label'); ?></label>

			<?php
		}
		require_once(SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc');
		$option_list = new Asset_Attribute_Option_List(0, $asset->attr('options'));
		$option_list->paint($prefix.'_option_list', !$write_access, FALSE);

	}//end paintOptions()


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
	function processOptions(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_option_list_options'])) {
			require_once(SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc');
			$option_list = new Asset_Attribute_Option_List(0, $asset->attr('options'));
			$option_list->process($prefix.'_option_list', FALSE);

			$asset->setAttrValue('options', $option_list->value);
			$asset->saveAttributes();
		}//end if

	}//end processOptions()


}//end class
?>
