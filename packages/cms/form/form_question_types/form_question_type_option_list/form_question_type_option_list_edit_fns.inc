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
* $Id: form_question_type_option_list_edit_fns.inc,v 1.3 2004/10/15 01:26:58 dbaranovskiy Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form_question_type_tickbox_list/form_question_type_tickbox_list_edit_fns.inc';

/**
* Form_Question_Type_Option_List_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Option_List_Edit_Fns extends Form_Question_Type_Tickbox_List_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Option_List_Edit_Fns()
	{
		$this->Form_Question_Type_Tickbox_List_Edit_Fns();
	
	}//end Form_Question_Type_Option_List_Edit_Fns()


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
		
		echo "<table cellpadding=\"0\" cellspacing=\"0\">\n";
		
		$defaults = $asset->attr('default');

		foreach ($options as $ref => $option) {
			if (!trim($option)) continue;

			echo '<tr><td>';
			$val  = explode(":", $option);
			$name = $prefix.'default[]';
			
			$title = (count($val) > 1) ? $val[1] : $val[0];
			$checked = (in_array($ref, $defaults)) ? true : false;
			radio_button($name, $ref, $checked);
			
			echo '&nbsp;<span onclick="this.previousSibling.previousSibling.checked = (!this.previousSibling.previousSibling.checked || this.previousSibling.previousSibling.type==\'radio\')">'.$title.'</span>';
			echo '</td></tr>';
		}
		echo '</table>';

		return true;
	
	}//end paintDefault()


}//end class
?>
