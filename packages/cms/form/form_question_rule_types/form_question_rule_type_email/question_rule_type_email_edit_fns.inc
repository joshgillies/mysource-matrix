<?php

require_once dirname(__FILE__).'/../../question_rule/question_rule_edit_fns.inc';

/**
* Question_Rule_Type_Email_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Question_Rule_Type_Email_Edit_Fns extends Question_Rule_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Question_Rule_Type_Email_Edit_Fns()
	{
		$this->Question_Rule_Edit_Fns();
	}
	
	function paintInlineBackend($asset, $o, $prefix)
	{

		$contents = Array(
			1 => 'Must Be a Valid Email Address',
			2 => 'Must Not Be A Valid Email Address',
		);

		$default = ($asset->attr('operator')) ? $asset->attr('operator') : ''; 
		bam($default);
		?>
		<table>
			<tr>
				<td>
					Email Address Validation
				</td>
				<td>
					<?php combo_box($prefix.'_operator', $contents, false, $default); ?>
				<td>
			</tr>
		</table>
		<?
	}

	function processInlineBackend($asset, $o, $prefix)
	{
		if (isset($_POST[$prefix.'_operator'])) {
			$asset->setAttrValue('operator', $_POST[$prefix.'_operator']);
		}
	}


}//end class
?>
