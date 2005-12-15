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
* $Id: form_question_type_option_list_edit_fns.inc,v 1.7.2.2 2005/12/15 22:24:47 lwright Exp $
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
* @version $Revision: 1.7.2.2 $
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
					$checked = (empty($defaults)) ? true : false;
				} else {
					$checked = (in_array($ref - 1, $defaults)) ? true : false;
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

		return true;

	}//end paintDefault()


}//end class
?>
