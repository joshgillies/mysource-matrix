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
* $Id: default_delivery_method_edit_fns.inc,v 1.14 2013/06/20 07:27:02 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ecommerce/delivery_methods/ecommerce_delivery_method/ecommerce_delivery_method_edit_fns.inc';


/**
* Default_Delivery_Method_Edit_Fns
*
* Purpose
*
*
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Default_Delivery_Method_Edit_Fns extends Ecommerce_Delivery_Method_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Default_Delivery_Method_Edit_Fns()
	{
		parent::__construct();
	}//end constructor


	/**
	* Paints the available keywords
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAvailableKeywords(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$tmp_form =& $asset->getForm();
			$availableKeywords = $tmp_form->getQuestions();

			// print the response keyword
			echo '<style type="text/css">#keyword_table td {border:1px solid #B3B3B3;}  #keyword_table {border:1px solid #000}</style>';
			echo '<table id="keyword_table"><tr><td><strong>Keyword</strong></td><td><strong>Description</strong></td></tr>';
			foreach ($availableKeywords as $key => $value) {
				$key_parts = explode(':', $key);
				echo '<tr><td>%response_'.$key_parts[1].'%</td><td>print field value <strong>"'.$value['name'].'"</strong> </td></tr>';
			}
			echo '<tr><td>%customised_receipt%</td><td>print the customised receipt (if enabled via the checkout asset)</td></tr>';
			echo '<tr><td>%order_receipt%</td><td>print the order receipt</td></tr>';
			echo '<tr><td>%cart_summary%</td><td>print the cart summary</td></tr>';
			echo '<tr><td>%cart_summary_text%</td><td>print the cart summary in a text format</td></tr>';
			echo '<tr><td>%form_summary%</td><td>print the form summary</td></tr>';
			echo '<tr><td>%form_summary_text%</td><td>print the form summary in a text format</td></tr>';
			echo '<tr><td>%order_id%</td><td>print the order id</td></tr>';
			echo '<tr><td>%transaction_id%</td><td>print the unique transaction reference id</td></tr>';
			echo '<tr><td>%merchant_transaction_id%</td><td>print the unique merchant transaction reference id</td></tr>';
			echo '<tr><td>%negative_quantity_product_list%</td><td>print products which have negative quantities after purchasing</td></tr>';
			echo '<tr><td>%negative_quantity_message%</td><td>print negative quantity products message if there are any negative quantities</td></tr>';
			echo '</table>';

		}

		return $write_access;

	}//end paintAvailableKeywords()


	/**
	* Dummy function
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAvailableKeywords(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processAvailableKeywords()

	
	/**
	* Paint key-value pair pass through variable selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPassthroughVarQuestionLinks(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$write_access = $asset->writeAccess('attributes');

		$o->openRaw();

		echo '<table class="sq-backend-table">';
		echo '<tr>';
		echo '<th>Question Name</th>';
		echo '<th>Enabled?</th>';
		echo '<th>Key</th>';
		echo '<th>Value Options</th>';
		echo '</tr>';

		$form =& $asset->getForm();
		$all_form_questions = $form->getQuestions();
		$var_rules = $asset->attr('key_val_rules');

		foreach ($all_form_questions as $q_id => $question_data) {
			if (!$asset->_isAllowedPassThroughQuestionType($question_data['type_code'])) {
				continue;
			}

			$var_enabled = isset($var_rules[$q_id]);

			echo '<tr>';
			echo '<td>';
			echo $question_data['name'];
			echo '</td>';
			echo '<td>';
			$var_enabled_name = $prefix.'_var_enabled['.$q_id.']';
			check_box($var_enabled_name, $q_id, $var_enabled);
			echo '</td>';
			echo '<td>';
			if ($var_enabled) {
				$pass_through_key_name = $prefix.'_pass_through_key['.$q_id.']';
				text_box($pass_through_key_name, array_get_index($var_rules[$q_id], 'key', ''));
			} else {
				echo '<em style="color: grey;">None</em>';
			}
			echo '</td>';
			echo '<td>';
			if ($var_enabled) {
				$question = $am->getAsset($q_id);
				if (!($question instanceof Form_Question_Type_Country) && $question->isSelection()) {
					echo '<table class="sq-backend-table">';
					$options = $question->getOptions();
					$var_value = array_get_index($var_rules[$q_id], 'value', Array());
					$local_prefix = $prefix.'_pass_through_value['.$q_id.']';
					foreach ($options as $key => $option_text) {
						// if this option is not --leave empty--, print out the rule change table
						if (!($question instanceof Form_question_type_select) || !$question->isEmptyOption($key)){
							echo '<tr>';
							$rule_value = array_get_index($var_value, $key, '');
							echo '<td class="sq-backend-table-header" nowrap="nowrap">';
							echo $option_text;
							echo '</td>';
							echo '<td class="sq-backend-table-cell" width="100%">';
							text_box($local_prefix.'['.$key.']', $rule_value);
							echo '</td>';
							echo '</tr>';
						}
					}
					echo '</table>';
				} else {
					echo 'The value supplied by the user will be passed through to the Payment Gateway.';
				}

				$am->forgetAsset($question);
			} else {
				echo '<em style="color: grey;">Pass through variable is not enabled for this question</em>';
			}
			echo '</td>';
			echo '</tr>';
		}//end foreach

		echo '</table>';

		$o->closeRaw();

		return $write_access;

	}//end paintPassthroughVarQuestionLinks()


	/**
	* process key-value pair pass through variable selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPassthroughVarQuestionLinks(&$asset, &$o, $prefix)
	{
		// process the new pass through variable rule list
		$var_list = array_get_index($_REQUEST, $prefix.'_var_enabled', Array());
		
		$new_var_list = Array();
		if (!empty($var_list)) {
			$key_list = array_get_index($_REQUEST, $prefix.'_pass_through_key', Array());
			$value_list = array_get_index($_REQUEST, $prefix.'_pass_through_value', Array());
			foreach ($var_list as $id) {
				// add the empty var to the new list
				$new_var_list[$id] = Array(
											'key' 	=> array_get_index($key_list, $id, ''),
											'value'	=> array_get_index($value_list, $id, Array()));
			}
		}
		
		$asset->setAttrValue('key_val_rules', $new_var_list);
		
		return TRUE;

	}//end processPassthroughVarQuestionLinks()
	
	
	/**
	* Paint the interface for selecting the Payment Gateways
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPaymentGateways(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$gateways = $asset->attr('payment_gateways');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			multiple_asset_finder($prefix.'_gateways', $gateways, Array('payment_gateway' => 'D'));
		} else {
				echo '<ul>';
				foreach ($gateways as $index => $assetid) {
						echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				echo '</ul>';
		}

		return $write_access;

	}//end paintPaymentGateways()


	/**
	* Paint the interface for selecting the Payment Gateways
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPaymentGateways(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (empty($_POST[$prefix.'_gateways'])) return FALSE;

		$gateways = $_POST[$prefix.'_gateways'];
		$save_gateways = Array();
		foreach ($gateways as $index => $asset_info) {
			if (!empty($asset_info['assetid'])) {
				$save_gateways[] = $asset_info['assetid'];
			}//end if
		}//end foreach

		return $asset->setAttrValue('payment_gateways', $save_gateways);

	}//end processPaymentGateways()	
	

}//end class

?>
