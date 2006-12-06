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
* $Id: form_question_type_tickbox_list_edit_fns.inc,v 1.4 2006/12/06 00:35:32 emcdonald Exp $
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
* @version $Revision: 1.4 $
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
		if (!$asset->writeAccess('attributes')) return false;

		$options = explode("\r\n", $asset->attr('options'));
		$multiple = (get_class($asset) == 'form_question_type_tickbox_list') ? true : false;
		$defaults = $asset->attr('default');

		foreach ($options as $ref => $option) {
			if (!trim($option)) continue;

			$type = ($multiple) ? 'check_box' : 'radio_button';
			$name = $prefix.'default[]';

			$checked = (in_array($ref, $defaults)) ? true : false;
			$type($name, $ref, $checked, '', 'id="'.$prefix.'default'.$ref.'"');
			label($option, $prefix.'default'.$ref);
			echo '<br />';
		}

		return true;

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
				return false;
			}
		// if nothing has been submitted, then nothing has been selected, so blank all defaults out
		} else {
			$asset->setAttrValue('default', Array());
		}
		$asset->saveAttributes();

		return true;

	}//end processDefault()


}//end class
?>
