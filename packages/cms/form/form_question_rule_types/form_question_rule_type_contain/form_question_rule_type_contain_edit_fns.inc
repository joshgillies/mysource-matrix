<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_type_contain_edit_fns.inc,v 1.4 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Contain_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Contain_Edit_Fns extends Form_Question_Rule_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Contain_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();
	}
	
	function paintInlineBackend(&$asset, &$o, $prefix)
	{

		$contents = Array(
			1 => 'Must Contain ...',
			0 => 'Must Not Contain ...',
		);

		// table tags are printed in caller (form_question_edit_fns::printRules())
		?>
		<tr>
			<td>
				<?php combo_box($prefix.'_operator', $contents, false, $asset->attr('operator')); ?>
			</td>
			<td>
			   <?php text_box($prefix.'_value', $asset->attr('value')); ?>
			</td>
			<td>
				Delete? <?php check_box('delete_rule', $asset->id); ?>
			</td>
		</tr>
		<?php
	
	} // end paintInlineBackend()

	function processInlineBackend(&$asset, &$o, $prefix)
	{
		if ($this->processDelete()) return true;

		if (isset($_POST[$prefix.'_operator'])) {
			$asset->setAttrValue('operator', $_POST[$prefix.'_operator']);
		}
		if (isset($_POST[$prefix.'_value'])) {
			$asset->setAttrValue('value', $_POST[$prefix.'_value']);
		}
		return $asset->saveAttributes();

	}// end processInlineBackend()


}//end class
?>
