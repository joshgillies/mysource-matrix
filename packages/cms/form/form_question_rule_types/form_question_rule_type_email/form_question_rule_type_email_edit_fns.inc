<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_rule_type_email_edit_fns.inc,v 1.6 2003/10/14 22:54:06 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule_edit_fns.inc';

/**
* Form_Question_Rule_Type_Email_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Email_Edit_Fns extends Form_Question_Rule_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Type_Email_Edit_Fns()
	{
		$this->Form_Question_Rule_Edit_Fns();
	}
	

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
			'1' => 'Must Be a Valid Email Address',
			'0' => 'Must Not Be A Valid Email Address',
		);

		?>
		<tr>
			<td>
				<?php combo_box($prefix.'_operator', $contents, false, $asset->attr('operator')); ?>
			</td>
			<td>
				&nbsp;
			</td>
			<td>
				Delete? <?php check_box('delete_rule[]', $asset->id); ?>
			</td>
		</tr>
		<?php
	
	} // end paintInlineBackend()


}//end class
?>
