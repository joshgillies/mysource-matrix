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
		$o->openField(translate('Discount Type'));

			$discount_types = $asset->getDiscountTypes();
			if ($write_access) {
				combo_box($prefix.'[discount_type]', $discount_types, FALSE, $rule_data['discount_type']);
			} else {
				echo $discount_types[$rule_data['discount_type']];
			}
			$o->note(translate('The discount type of this promotional rule. Do not use the percent symbol (%%) for the percentage discount type, e.g. use 50 for 50%% discount amount.'));

		$o->closeField();

		// Paint the new discount option if we have write access
		if ($write_access) {
			$o->openField(translate('New Discount Option'));

				echo '<b>'.translate('Asset Quantity').'</b>&nbsp;';

				text_box($prefix.'[new_asset_quantity]');
				echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
				echo '<b>'.translate('Discount Amount').'</b>&nbsp;';

				text_box($prefix.'[new_discount_amount]');
			$o->closeField();
		}

		// Paint the discount options
		$o->openField(translate('Discount Options'));

			if (empty($rule_data['discount_options'])) {
				echo translate('There are currently no discount options');

			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><?php echo translate('Asset Quantity'); ?></td>

						<td class="sq-backend-table-header"><?php echo translate('Discount Amount'); ?></td>

						<?php
						if ($write_access) {
							?>
						<td class="sq-backend-table-header"><?php echo translate('Delete?'); ?></td>

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
		$o->sectionNote(translate('This rule defines the discount amounts based on the number of assets/items being purchased. The asset quantity will be ordered by ascending automatically. It is supposed that the more assets being purchased, the more discount is given so that the total amount can be calculated correctly. For example, the asset quantity/discount amount pairs are 2/5, 3/9 and 10/50. If the currency unit is pound, it can be read as "buy 2 assets get 5 pounds discount (2.5 pounds discount for each asset), buy 3 get 9 pounds discount (3 pounds discount for each asset) and buy 10 get 50 pounds discount (5 pounds discount for each asset)". If there are 14 assets being purchased, they will be split to 10 + 3 + 1 instead of 10 + 2 + 2 because the rule for the more assets is taken into account first. So the discount amount for 14 assets are 50 + 9 = 59 pounds instead of 50 + 5 + 5 = 60 pounds. To have the correct calculation, you will need to add more thresholds like 4/10 (for 2+2) and 6/18 (for 3+3 instead of 2+2+2). But the better discount strategy should be "the asset quantity of the higher threshold is more than twice of the lower threshold". For example, you can use 2, 5 (greater than 2*2) and 11 (greater than 5*2) for the asset quantities. So 2/5, 5/15, and 11/55 are good thresholds.'));


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
