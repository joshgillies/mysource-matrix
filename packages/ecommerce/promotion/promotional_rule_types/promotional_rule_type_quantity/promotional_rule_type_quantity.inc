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
* $Id: promotional_rule_type_quantity.inc,v 1.3 2013/09/09 08:34:53 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_rule/promotional_rule.inc';

/**
* Promotional_Rule_Type_Quantity
*
* The promotional rule for multiple buy. Notice that the word "mutiple" here is for multiple of the same asset/product only
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Type_Quantity extends Promotional_Rule
{
	
	
	/**
	 * Constructor
	 * 
	 * @param int $assetid	the asset id to be loaded
	 */
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor
	
	
	/**
	 * Return the discount types supported by this quantity promotional rule
	 * 
	 * @return array
	 * @access public
	 */
	public function getDiscountTypes()
	{
		return Array(
						'fixed_amount'	=> 'Fixed Amount',
						'percentage'	=> 'Percentage',
					);
		
	}//end getDiscountTypes()
	
	
	/**
	 * Return the default discount type
	 * 
	 * @return string
	 * @access public
	 */
	public function getDefaultDiscountType()
	{
		return 'fixed_amount';
		
	}//end getDefaultDiscountType()
	
	
	/**
	 * Return the default rule data.
	 * 
	 * The structure of the rule data is 
	 * 		Array(
	 * 				'discount_type'		=> fixed amount or percentage,
	 * 				'discount_options'	=> Array(asset quantity => discount amount),
	 * 			 )
	 * 
	 * @return array
	 * @access public
	 */
	public function getDefaultRuleData()
	{
		return Array(
						'discount_type' => $this->getDefaultDiscountType(),
						'discount_options' => Array(),
					);
		
	}//end getDefaultRuleData()
	
	
	/**
	 * Evaluate the asset data based on the rule data
	 * 
	 * @param array &$asset_data	The asset data
	 * @param array &$rule_data		The rule data
	 * 
	 * @return void
	 * @access public
	 */
	public function evaluate(&$asset_data, $rule_data)
	{
		$price = $asset_data['price'];
		$remaining_quantity = $asset_data['quantity'];
		$discount = 0;
		
		$discount_options = $rule_data['discount_options'];
		$fixed_amount = $rule_data['discount_type'] == 'fixed_amount' ? TRUE : FALSE;
		
		// '0' quantity option means its flat discount regardless of the quantity
		// Rest of the quantity rules will be ignored
		if (isset($discount_options[0])) {
			if ($fixed_amount) {
				$discount = $discount_options[0];
			} else {
				$discount = $price * $discount_options[0]/100;
			}

			$asset_data['discount'] += $discount;
			return;
		}

		$discount_options = array_reverse($discount_options, TRUE);
		foreach ($discount_options as $asset_quantity => $discount_amount) {
			if ($remaining_quantity >= $asset_quantity) {
				$quotient = (int) ($remaining_quantity / $asset_quantity);
				$remaining_quantity = $remaining_quantity % $asset_quantity;
				if ($fixed_amount) {
					$discount += $quotient * $discount_amount;
				} else {
					$discount += $quotient * ($asset_quantity * $price * $discount_amount / 100);
				}
				
				if ($remaining_quantity == 0) break;
			}
		}
		
		$asset_data['discount'] += $discount;
		
	}//end evaluate()
	
	
}//end class

?>
