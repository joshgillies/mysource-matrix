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
* $Id: form_question_rule_type_found_in_edit_fns.inc,v 1.8 2003/11/26 00:51:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Found_In_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Found_In_Edit_Fns extends Form_Question_Rule_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Found_In_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();
	
	} // end Form_Question_Rule_Type_Found_In_Edit_Fns()
	

	/**
	* paints a section in the question to change the options of this rule
	* 
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function paintInlineBackend(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$contents = Array(
			1 => 'Text Must Be Found In...',
			0 => 'Text Must Not Be Found In...',
		);

		// table tags are printed in caller (form_question_edit_fns::printRules())
		?>
		<tr>
			<td>
				<?php combo_box($prefix.'_operator', $contents, false, $asset->attr('operator')); ?>
			</td>
			<td>
			<?php
				$q = &$asset->getMyQuestion();
				if ($q->isSelection()) {
					$asset->getOptionDropdownList();
					echo '&nbsp;Or';
				} else {
					text_box($prefix.'_value', $asset->attr('value'));
					echo '&nbsp;Or';
				}
			?>
			</td>
			<td>
				Delete? <?php check_box('delete_rule[]', $asset->id); ?>
			</td>
		</tr>
		<tr>
		<td>&nbsp;</td>
		  <td>
			  <?php $asset->printAssetFinder($asset->attr('comparison_question_id')) ?>
		  </td>
		</tr>
		<?php

		return true;
	
	} // end paintInlineBackend()

}//end class
?>
