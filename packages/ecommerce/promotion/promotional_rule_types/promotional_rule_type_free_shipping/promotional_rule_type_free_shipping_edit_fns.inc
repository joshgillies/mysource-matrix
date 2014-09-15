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
* $Id: promotional_rule_type_free_shipping_edit_fns.inc,v 1.2 2013/09/13 07:59:03 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_rule/promotional_rule_edit_fns.inc';

/**
* Promotional_Rule_Type_Quantity_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Type_Free_Shipping_Edit_Fns extends Promotional_Rule_Edit_Fns
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
		$o->openField(translate('ecom_promotional_rule_type_free_shipping_discount_type'));
			$discount_types = $asset->getDiscountTypes();
			if ($write_access) {
				combo_box($prefix.'[threshold_type]', $discount_types, FALSE, $rule_data['threshold_type']);
			} else {
				echo $discount_types[$rule_data['threshold_type']];

			}
		$o->closeField();

		// Paint the new discount option if we have write access
		$o->openField(translate('ecom_promotional_rule_type_free_shipping_threshold'));
			if ($write_access) {
				text_box($prefix.'[threshold_value]', $rule_data['threshold_value']);
			} else {
				echo $rule_data['threshold_value'];
			}

		$o->closeField();



		// Add section note
		$o->sectionNote(translate('ecom_promotional_rule_type_free_shipping_section_note'));

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

		// Save threshold type
		$threshold_type = array_get_index($_POST[$prefix], 'threshold_type', '');
		if (array_key_exists($threshold_type, $asset->getDiscountTypes())) {
			$rule_data['threshold_type'] = $threshold_type;
		}

		// Add threshold value
		$threshold_value = array_get_index($_POST[$prefix], 'threshold_value');
		if (is_numeric($threshold_value)) {
			$threshold_value = (int) $threshold_value;
			if ($threshold_value >= 0) {
				$rule_data['threshold_value'] = $threshold_value;
			}
		}

		return TRUE;

	}//end processRule()


}//end class

?>
