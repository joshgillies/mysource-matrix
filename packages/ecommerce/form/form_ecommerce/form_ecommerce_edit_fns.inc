<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_ecommerce_edit_fns.inc,v 1.17 2008/11/25 00:48:04 mbrydon Exp $
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
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Form_Ecommerce_Edit_Fns extends Form_Email_Edit_Fns
{


	/**
	* Paint the bodycopy selection interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopyList(&$asset, &$o, $prefix)
	{
		$o->openRaw();

		$bodycopies = $asset->_bodycopies;
		foreach ($bodycopies as $one_bodycopy) {
			$bodycopy_asset = $asset->getBodycopy($one_bodycopy);
			echo '<div style="padding:1ex;">';
			echo get_asset_tag_line($bodycopy_asset->id, 'contents');
			echo '</div>';
		}

		$o->closeRaw();

	}//end paintBodycopyList()


	/**
	* Process the bodycopy selection interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseBodycopy(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processUseBodycopy()


	/**
	* Paint e-commerce rules selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEcommerceQuestionLinks(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('attributes');

		$o->openRaw();

		echo '<table class="sq-backend-table">';
		echo '<tr>';
		echo '<th>Question Name</th>';
		echo '<th>Ecom Enabled?</th>';
		echo '<th>Is Taxable?</th>';
		echo '<th>Options</th>';
		echo '</tr>';

		$all_form_questions = $asset->getAllQuestions();
		$taxable_rules = $asset->attr('taxable_rules');

		foreach ($all_form_questions as $q_id => $question_data) {
			if (!$asset->_isAllowedQuestionType($question_data['question_type_code'])) {
				continue;
			}

			$rule = $asset->_getEcommerceRule($q_id);
			$ecom_enabled = ($rule !== FALSE);

			echo '<tr>';
			echo '<td>';
			echo $question_data['attributes']['name'];
			echo '</td>';
			echo '<td>';
			$ecom_enabled_prefix = $prefix.'_ecom_enabled['.$q_id.']';
			check_box($ecom_enabled_prefix, $q_id, $ecom_enabled);
			echo '</td>';
			echo '<td>';
			$taxable = array_get_index($taxable_rules, $q_id, FALSE);
			$taxable_prefix = $prefix.'_taxable['.$q_id.']';
			check_box($taxable_prefix, $q_id, $taxable);
			echo '</td>';
			echo '<td>';
			if ($ecom_enabled) {
				$question = $am->getAsset($q_id);
				if ($question->isSelection()) {
					echo '<table class="sq-backend-table">';
					$options = $question->getOptions();
					$local_prefix = $prefix.'_ecom_rule['.$q_id.']';
					foreach ($options as $key => $option_text) {
						// if this option is not --leave empty--, print out the rule change table
						if (!($question instanceof Form_question_type_select) || !$question->isEmptyOption($key)){
							echo '<tr>';
							$rule_value = array_get_index($rule, $key, 0);
							echo '<td class="sq-backend-table-header" nowrap="nowrap">';
							echo $option_text;
							echo '</td>';
							echo '<td class="sq-backend-table-cell" width="100%">';
							text_box($local_prefix.'['.$key.']', $rule_value, 5);
							echo '</td>';
							echo '</tr>';
						}
					}
					echo '</table>';
				} else {
					echo translate('ecom_form_numeric_field');
				}

				$am->forgetAsset($question);
			} else {
				echo '<em style="color: grey;">E-commerce support for this question is not enabled</em>';
			}
			echo '</td>';
			echo '</tr>';
		}//end foreach

		echo '</table>';

		$o->closeRaw();

		return $write_access;

	}//end paintEcommerceQuestionLinks()


	/**
	* process e-commerce rules selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processEcommerceQuestionLinks(&$asset, &$o, $prefix)
	{
		// process the new rule list
		$rule_list_name   = $prefix.'_ecom_enabled';
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

		$asset->setAttrValue('rules', $new_rule_list);

		$rule_settings = array_get_index($_REQUEST, $prefix.'_ecom_rule', Array());

		foreach ($rule_settings as $q_id => $rule) {
			$asset->_setEcommerceRule($q_id, $rule);
		}

		$taxable = array_get_index($_REQUEST, $prefix.'_taxable', Array());
		$asset->setAttrValue('taxable_rules', $taxable);

		return TRUE;

	}//end processEcommerceQuestionLinks()


	/**
	* Paint export interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExport(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (array_get_index($_POST, $prefix.'_export_switch', 0) == 1) {
			$this->processExport($asset, $o, $prefix);
		}

		// paint option goes here
		echo hidden_field($prefix.'_export_switch', '0');

		echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;');

		return TRUE;

	}//end paintExport()


	/**
	* Process export interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExport(&$asset, &$o, $prefix)
	{
		if (!$_POST[$prefix.'_export_switch']) return TRUE;

		// begin and populate order object
		header('Content-Type: application/xml');

		// defining the file as attachment forces most browsers to offer it for download
		header('Content-Disposition: attachment; filename=order-summary-'.$asset->id.'.xml;');

		echo '<orders>'."\n";

		// get each order
		$children_id = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'order', FALSE);

		foreach ($children_id as $child_id => $assettype) {
			// retrieve asset with this id
			echo "\t".'<order id="'.$child_id.'">'."\n";

			// get this orders children
			$order_lines = $GLOBALS['SQ_SYSTEM']->am->getChildren($child_id, 'order_line', FALSE);

			foreach ($order_lines as $line_id => $assettype) {

				$child =& $GLOBALS['SQ_SYSTEM']->am->getAsset($line_id);

				// extract name quantity price etc
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

		return TRUE;

	}//end processExport()


	/**
	* Paint log file info interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLogFileInfo(&$asset, &$o, $prefix)
	{
		$log_file = $asset->_getLogFilePath();
		if (is_readable($log_file)) {
			$last_modified = filemtime($log_file);
			$file_size = filesize($log_file);
			echo 'Filename: <strong>'.$log_file.'</strong>';
			echo '<br />';
			echo 'Last Modified: <strong>'.date('r', $last_modified).'</strong>';
			echo '<br />';
			echo 'Size: <strong>'.$file_size.'</strong>B';
		} else {
			echo 'Log file does not exist';
			echo '<br />';
			echo 'Filename: '.$log_file;
		}

	}//end paintLogFileInfo()


}//end class
?>
