<?php

require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Comparison_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Comparison_Edit_Fns extends Form_Question_Rule_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Comparison_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();

	} // end Form_Question_Rule_Type_Comparison_Edit_Fns()
	

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
		$contents = $asset->getOperators();
		if (empty($contents)) return false;

		// find out if we are some sort of select question
		$select = false;
		if ($asset->isSelection()) {
			$select = true;
			$my_question = &$asset->getMyQuestion();

		}

		// table tags are printed in caller (form_question_edit_fns::printRules())
		?>
		<tr>
			<td>
				<?php combo_box($prefix.'_operator', $contents, false, $asset->attr('operator')); ?>
			</td>
			<td>
			<?php
				if ($select) {
					combo_box($prefix.'_value', $my_question->getOptions(), false, $asset->attr('value'), 1, '', 30);
				} else {
					text_box($prefix.'_value', $asset->attr('value'));
				}
			?>
			</td>
			<td>
			  
			</td>
			<td>
				Delete? <?php check_box('delete_rule', $asset->id); ?>
			</td>
		</tr>
		<?php
		return true;
	
	} // end paintInlineBackend()


	/**
	* processes the section in the question to change the options of this rule
	* 
	* @param object Form				&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
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
