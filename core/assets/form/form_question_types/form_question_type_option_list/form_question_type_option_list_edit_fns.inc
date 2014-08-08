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
* $Id: form_question_type_option_list_edit_fns.inc,v 1.7 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.7 $
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
		$options = $asset->getOptions();
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

				echo '&nbsp;<label onclick="this.previousSibling.previousSibling.checked = (!this.previousSibling.previousSibling.checked || this.previousSibling.previousSibling.type==\'radio\')">'.$option.'</label>';
				echo '</td></tr>';
			}

		}//end if
		echo '</table>';

		return TRUE;

	}//end paintDefault()


}//end class

?>
