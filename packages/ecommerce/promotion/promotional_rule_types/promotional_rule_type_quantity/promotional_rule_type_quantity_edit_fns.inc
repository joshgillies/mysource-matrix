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
* $Id: promotional_rule_type_quantity_edit_fns.inc,v 1.5 2013/09/13 08:05:30 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_rule/promotional_rule_edit_fns.inc';

/**
* Promotional_Rule_Type_Quantity_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Type_Quantity_Edit_Fns extends Promotional_Rule_Edit_Fns
{
	
	
	/**
	 * Constructor
	 */
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	 * Paints the promotional rule's interface
	 *
	 * @param object	&$asset			The asset to paint
	 * @param object	&$o				The backend outputter
	 * @param string	$prefix			String prefix for this interface/asset
	 * @param array		$rule_data		The rule data
	 * @param boolean	$write_access	Whether or not the user has write access
	 *
	 * @return boolean
	 * @access public
	 */
	function paintRule(&$asset, &$o, $prefix, $rule_data, $write_access)
	{
		parent::paintRule($asset, $o, $prefix, $rule_data, $write_access);

		// Setup default values if there is no rule data
		if (is_null($rule_data)) {
			$rule_data = $asset->getDefaultRuleData();
		}
		
		// Paint the discount type selection
		$o->openField(translate('ecom_promotional_rule_type_quantity_discount_type'));
			$discount_types = $asset->getDiscountTypes();
			if ($write_access) {
				combo_box($prefix.'[discount_type]', $discount_types, FALSE, $rule_data['discount_type']);
			} else {
				echo $discount_types[$rule_data['discount_type']];
			}
			$o->note(translate('ecom_promotional_rule_type_quantity_discount_type_note'));
		$o->closeField();
		
		// Paint the new discount option if we have write access
		if ($write_access) {
			$o->openField(translate('ecom_promotional_rule_type_quantity_new_discount_option'));
				echo '<b>'.translate('ecom_promotional_rule_type_quantity_asset_quantity').'</b>&nbsp;';
				text_box($prefix.'[new_asset_quantity]');
				echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
				echo '<b>'.translate('ecom_promotional_rule_type_quantity_discount_amount').'</b>&nbsp;';
				text_box($prefix.'[new_discount_amount]');
			$o->closeField();
		}
		
		// Paint the discount options
		$o->openField(translate('ecom_promotional_rule_type_quantity_discount_options'));
			if (empty($rule_data['discount_options'])) {
				echo translate('ecom_promotional_rule_type_quantity_no_discount_options');
			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><?php echo translate('ecom_promotional_rule_type_quantity_asset_quantity'); ?></td>
						<td class="sq-backend-table-header"><?php echo translate('ecom_promotional_rule_type_quantity_discount_amount'); ?></td>
						<?php
						if ($write_access) {
							?>
						<td class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td>
							<?php
						}
						?>
					</tr>
					<?php
					$i = 0;
					foreach ($rule_data['discount_options'] as $asset_quantity => $discount_amount) {
						?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php
								if ($write_access) {
									text_box($prefix."[discount_options][$i][asset_quantity]", $asset_quantity);
								} else {
									echo $asset_quantity;
								}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								if ($write_access) {
									text_box($prefix."[discount_options][$i][discount_amount]", $discount_amount);
								} else {
									echo $discount_amount;
								}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
						<td class="sq-backend-table-cell">
							<?php
								check_box($prefix."[discount_options][$i][delete]");
							?>
						</td>
							<?php
						}
						?>
					</tr>
					<?php
						$i++;
					}
					?>
				</table>
				<?php
			}
		$o->closeField();
		
		// Add section note
		$o->sectionNote(translate('ecom_promotional_rule_type_quantity_section_note'));
		
		return $write_access;
		
	}//end paintRule()
	
	
	/**
	 * Process the promotional rule's interface
	 *
	 * @param object	&$asset		The asset to paint
	 * @param object	&$o			The backend outputter
	 * @param string	$prefix		String prefix for this interface/asset
	 * @param array		&$rule_data	The rule data
	 *
	 * @return boolean
	 * @access public
	 */
	function processRule(&$asset, &$o, $prefix, &$rule_data)
	{
		parent::processRule($asset, $o, $prefix, $rule_data);
		
		if (!isset($_POST[$prefix])) return FALSE;
		
		// Setup default values if there is no rule data
		if (is_null($rule_data)) {
			$rule_data = $asset->getDefaultRuleData();
		}
		
		// Save discount type
		$discount_type = array_get_index($_POST[$prefix], 'discount_type');
		if (array_key_exists($discount_type, $asset->getDiscountTypes())) {
			$rule_data['discount_type'] = $discount_type;
		}
		
		// Save the current discount options
		$discount_options = Array();
		$post_discount_options = array_get_index($_POST[$prefix], 'discount_options', Array());
		foreach ($post_discount_options as $i => $option) {
			// Do not save the option if its delete checkbox is selected
			if (isset($option['delete'])) continue;
			
			// Add the option to the option list if the inputs are valid
			$asset_quantity = array_get_index($option, 'asset_quantity');
			$discount_amount = array_get_index($option, 'discount_amount');
			if (is_numeric($asset_quantity) && is_numeric($discount_amount)) {
				$asset_quantity = (int) $asset_quantity;
				if ($asset_quantity >= 0) {
					$discount_options[$asset_quantity] = (float) $discount_amount;
				}
			}
		}
		
		// Add new discount option
		$new_asset_quantity = array_get_index($_POST[$prefix], 'new_asset_quantity');
		$new_discount_amount = array_get_index($_POST[$prefix], 'new_discount_amount');
		
		if (is_numeric($new_asset_quantity) && is_numeric($new_discount_amount)) {
			$new_asset_quantity = (int) $new_asset_quantity;
			if ($new_asset_quantity >= 0) {
				$discount_options[$new_asset_quantity] = (float) $new_discount_amount;
			}
		}
		
		// Sort the asset quantiy ascendingly
		if (!ksort($discount_options)) return FALSE;
		
		// Set the new discount options to rule data
		$rule_data['discount_options'] = $discount_options;
		
		return TRUE;
		
	}//end processRule()
	
	
}//end class

?>
