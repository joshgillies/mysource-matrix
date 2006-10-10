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
* $Id: form_question_type_option_list_edit_fns.inc,v 1.4 2006/10/10 05:53:09 emcdonald Exp $
*
*/


require_once dirname(__FILE__).'/../form_question_type_tickbox_list/form_question_type_tickbox_list_edit_fns.inc';


/**
* Form_Question_Type_Option_List_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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

	}//end constructor


	/**
	* paints a section to choose the default for this question
	*
	* @param object	&$asset	Form_Question, the asset whose interface we are painting
	* @param object	&$o		Backend_Outputter, the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		// default entry to have no default value
		$none = Array('none');
		$options = explode("\r\n", $asset->attr('options'));
		$options = array_merge($none, $options);

		echo "<table cellpadding=\"0\" cellspacing=\"0\">\n";
		$defaults = $asset->attr('default');

		if (!empty($options[1])) {

			foreach ($options as $ref => $option) {
				if (!trim($option)) continue;
				echo '<tr><td>';
				$name = $prefix.'default[]';

				if ($option == 'none') {
					$checked = (empty($defaults)) ? TRUE : FALSE;
				} else {
					$checked = (in_array($ref - 1, $defaults)) ? TRUE : FALSE;
				}

				if ($ref == 0) {
					radio_button($name, 'none', $checked);
				} else {
					radio_button($name, ($ref - 1), $checked);
				}

				echo '&nbsp;<span onclick="this.previousSibling.previousSibling.checked = (!this.previousSibling.previousSibling.checked || this.previousSibling.previousSibling.type==\'radio\')">'.$option.'</span>';
				echo '</td></tr>';
			}

		}//end if
		echo '</table>';

		return TRUE;

	}//end paintDefault()


}//end class

?>
