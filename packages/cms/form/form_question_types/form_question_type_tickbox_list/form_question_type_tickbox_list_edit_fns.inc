<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_tickbox_list_edit_fns.inc,v 1.4 2003/10/13 05:22:45 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_Tickbox_List_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
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
	
	} // end Form_Question_Type_Tickbox_List_Edit_Fns()


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
		$multiple = ($asset->attr('multiple')) ? true : false;
		
		echo "<table cellpadding=\"0\" cellspacing=\"0\">\n";
		
		$defaults = $asset->attr('default');

		foreach ($options as $ref => $option) {
			if (!trim($option)) continue;

			echo '<tr><td>';
			$val  = explode(":", $option);
			$type = ($multiple) ? 'check_box' : 'radio_button';
			$name = $prefix.'default[]';
			
			$title = (count($val) > 1) ? $val[1] : $val[0];
			$checked = (in_array($ref, $defaults)) ? true : false;
			$type($name, $ref, $checked);
			
			echo '&nbsp;'.$title;
			echo '</td></tr>';
		}
		echo '</table>';

		return true;
	
	} // end paintDefault()

	
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
		if (isset($_POST[$prefix.'default'])) {
			if (!$asset->setAttrValue('default', $_POST[$prefix.'default'])) {
				return false;
			}
		// if nothing has been submitted, then nothing has been selected, so blank all defaults out
		} else {
			$asset->setAttrValue('default', Array());
		}
		$asset->saveAttributes();
		
		return true;
	
	}// end processDefault()


}//end class
?>
