<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: form_ecommerce_edit_fns.inc,v 1.9 2006/12/07 05:26:29 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_email/form_email_edit_fns.inc';

require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Form_Ecommerce_Edit_Fns
*
* Purpose
*
*
* @author	Andrei Railean		<arailean@squiz.net>
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Form_Ecommerce_Edit_Fns extends Form_Email_Edit_Fns
{

	/**
	* Paint e-commerce rules selector
	*
	* @access public
	* @return boolean
	*/
	function paintEcommerceQuestionLinks(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('attributes');

		// obtain a list of all the questions in this form
		$all_questions = Array();

		$questions = $asset->attr('questions');
		if (!empty($questions)) {
			foreach ($questions as $q_id => $question) {
				// if the type is allowed, add it to the list
				if ($asset->_isAllowedQuestionType($question['question_type_code'])) {
					$id = $asset->id.':q'.$q_id;
					$all_questions[$id]['name'] = $question['attributes']['name'];
					// determine if this question is in our list
					if ($asset->_getEcommerceRule($id) !== false) {
						$all_questions[$id]['selected'] = true;
					} else {
						$all_questions[$id]['selected'] = false;
					}
				}
			}
		}

		// now get the questions inside each of the sections
		$sections = $asset->getSections();
		foreach ($sections as $section) {
			$questions = $section->attr('questions');
			if (!empty($questions)) {
				foreach ($questions as $q_id => $question) {
					// if the type is allowed, add it to the list
					if ($asset->_isAllowedQuestionType($question['question_type_code'])) {
						$id = $section->id.':q'.$q_id;
						$all_questions[$id]['name']  = $question['attributes']['name'];
						// determine if this question is in our list
						if ($asset->_getEcommerceRule($id) !== false) {
							$all_questions[$id]['selected'] = true;
						} else {
							$all_questions[$id]['selected'] = false;
						}
					}
				}
			}
		}

		$rule_list_name   = $prefix.'_LIST';
		$active_rule_name = $prefix.'_ACTIVE';

		$datapath = sq_web_path('data').'/asset_types/form_ecommerce/files';

		if ($write_access) {
			$to_output =
			'<script src="'.$datapath.'/lister.js"></script>'."\n".
			'	<table border="0" cellpadding="0" cellspacing="0">'."\n".
			'		<tr>'."\n".
			'			<td>'."\n".
			'				<select id="QuestionList" multiple="multiple" ondblclick="List.From2To();" style="width:200px;height:100px"></select>'."\n".
			'			</td>'."\n".
			'			<td>'."\n".
			'				<input type="button" value="&raquo;" onclick="List.From2To();" />'."\n".
			'				<br />'."\n".
			'				<input type="button" value="&laquo;" onclick="List.To2From();" />'."\n".
			'			</td>'."\n".
			'			<td>'."\n".
			'				<select multiple id="'.$active_rule_name.'" name="'.$active_rule_name.'" onblur="List.Clear();" ondblclick="List.To2From();" style="width:200px;height:100px"></select>'."\n".
			'				<select multiple id="'.$rule_list_name.'[]" name="'.$rule_list_name.'[]" style="visibility:hidden;position:absolute;left:0px;top:0px;"></select>'."\n".
			'			</td>'."\n".
			'			<td>'."\n".
			'				<input type="submit" value="'.translate('ecom_form_edit_selection').'" />'."\n".
			'			</td>'."\n".
			'		</tr>'."\n".
			'	</table>'."\n".
			'<input id="bull" type="hidden" value="&bull; " />'."\n".
			'<script>'."\n".
			'	var List = new Lister("QuestionList", "'.$active_rule_name.'", "'.$rule_list_name.'[]");'."\n";

			foreach ($all_questions as $qid => $qattr)
			{
				$to_output .= '	List.addOption("'.$qid.'","'.$qattr['name'].'",'.(($qattr['selected'])?'true':'false').');'."\n";
			}
			$to_output .= '	List.process();</script>'."\n";
		} else {
			$to_output = '';
			foreach ($all_questions as $qid => $qattr)
			{
				if ($qattr['selected']) $to_output .= $qattr['name']."<br />\n";
			}
		}

		echo $to_output;

		// select new rule to edit
		$rule_fields_name = $prefix.'_FIELDS';
		$rule_edited_name = $prefix.'_EDITED';

		$edited_rule_id   = array_get_index($_REQUEST, $rule_edited_name, false);
		$active_rule_id   = array_get_index($_REQUEST, $active_rule_name, $edited_rule_id);

		if ($active_rule_id && (($rule = $asset->_getEcommerceRule($active_rule_id)) !== false)) {
			$question = $am->getAsset($active_rule_id);
			$edit_form = '<div>';
			if ($question->isSelection()) {
				$edit_form .= '<h3>'.translate('ecom_form_currency_for_question', $question->attr('name')).'</h3>';
				$edit_form .= '<table class="sq-backend-table">';
				$options = $question->getOptions();
				foreach ($options as $key => $option_text) {
					$rule_value = array_get_index($rule, $key, 0);
					$edit_form .= '<tr>';
					$edit_form .= '<td class="sq-backend-table-header" nowrap="nowrap">'.$option_text.'</td>';
					$edit_form .= '<td class="sq-backend-table-cell" width="100%"><input type="text" name="'.$rule_fields_name.'['.$key.']" value="'.$rule_value.'" size="5" /></td>';
					$edit_form .= '</tr>';
				}
				$edit_form .= '</table>';
				$edit_form .= '<input type="hidden" name="'.$rule_edited_name.'" value="'.$active_rule_id.'" />';
			} else {
				$edit_form = translate('ecom_form_numeric_field');
			}
			$edit_form .= '</div>';

			echo $edit_form;
		}

		return $write_access;

	}//end paintEcommerceQuestionLinks()

	/**
	* Assign setted rules to form's questions
	*
	* @access public
	* @return boolean
	*/
	function processEcommerceQuestionLinks(&$asset, &$o, $prefix)
	{
		// process the new rule list
		$rule_list_name   = $prefix.'_LIST';
		$rule_list        = array_get_index($_REQUEST, $rule_list_name);

		$new_rule_list = Array();
		if (!empty($rule_list) && is_array($rule_list)) {
			foreach ($rule_list as $id) {
				$rule = $asset->_getEcommerceRule($id);
				// if new rule is in the current list, just copy it to the new one
				if ($rule) {
					$new_rule = $rule;
				// otherwise, create an empty rule
				} else {
					$new_rule = Array();
				}
				// add the rule to the new list
				$new_rule_list[$id] = $new_rule;
			}

		}

		// save the new rule list. this makes sure that unneeded rules are deleted
		// because they are not copied over to the new list
		$asset->setAttrValue('rules', $new_rule_list);

		// commit changes to the rule
		$rule_fields_name = $prefix.'_FIELDS';
		$rule_fields  = array_get_index($_REQUEST, $rule_fields_name);

		$rule_edited_name = $prefix.'_EDITED';
		$rule_id = array_get_index($_REQUEST, $rule_edited_name);

		$asset->_setEcommerceRule($rule_id, $rule_fields);

		return true;

	}//end processEcommerceQuestionLinks()


	 //temp draw-up just for future notice
	 function paintExport(&$asset, &$o, $prefix)
	 {
		 if (!$asset->writeAccess('attributes')) return false;
		 if (array_get_index($_POST, $prefix.'_export_switch', 0) == 1) $this->processExport($asset, $o, $prefix);

		 //paint option goes here
		 echo hidden_field($prefix.'_export_switch', '0');

		 echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;');

		 return true;

	 }//end paintExport()


	 function processExport(&$asset, &$o, $prefix)
	 {
		 if (!$_POST[$prefix.'_export_switch']) return true;

		 //begin and populate order object
		 header('Content-Type: application/xml');

		 // defining the file as attachment forces most browsers to offer it for download
		 header('Content-Disposition: attachment; filename=order-summary-'.$asset->id.'.xml;');

		 echo '<orders>'."\n";

		 //get each order
		 $children_id = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'order', false);

		 foreach ($children_id as $child_id => $assettype) {
			//retrieve asset with this id
			echo "\t".'<order id="'.$child_id.'">'."\n";

			//get this orders children
			$order_lines = $GLOBALS['SQ_SYSTEM']->am->getChildren($child_id, 'order_line', false);

			foreach ($order_lines as $line_id => $assettype) {

				$child =& $GLOBALS['SQ_SYSTEM']->am->getAsset($line_id);

				//extract name quantity price etc
				?>
		<order_line id="<?php echo $child_id ?>">
			<product id="<?php echo $child->attr('product_assetid'); ?>"></product>
			<quantity><?php echo $child->attr('quantity'); ?></quantity>
			<price><?php echo $child->attr('price'); ?></price>
		</order_line>
					<?php
			}

			echo "\t".'</order>'."\n";

		 }

		 echo '</orders>'."\n";
		 exit(0);

		 return true;

	 }//end processExport()


}//end class
?>
