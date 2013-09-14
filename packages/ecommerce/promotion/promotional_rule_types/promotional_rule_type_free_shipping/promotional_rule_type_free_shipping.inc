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
* $Id: promotional_rule_type_free_shipping.inc,v 1.3 2013/09/14 01:43:20 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_rule/promotional_rule.inc';

/**
* Promotional_Rule_Type_Free_Shipping
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Type_Free_Shipping extends Promotional_Rule
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
	 * Return the discount types supported by this promotional rule
	 * 
	 * @return array
	 * @access public
	 */
	public function getDiscountTypes()
	{
		return Array(
						'amount'	=> 'Amount',
						'quantity'	=> 'Quantity',
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
		return 'amount';
		
	}//end getDefaultDiscountType()
	

	/**
	 * Return the default "threshold" value
	 * 
	 * @return string
	 * @access public
	 */
	public function getDefaultThresholdValue()
	{
		return '0';
		
	}//end getDefaultThresholdValue()
	
	
	/**
	 * Return the default rule data.
	 * 
	 * The structure of the rule data is 
	 * 		Array(
	 * 				'threshold_type'	=> threshold type i.e. amount or quantity,
	 * 				'threshold_value'	=> amount/quanitity threshold value,
	 * 			 )
	 * 
	 * @return array
	 * @access public
	 */
	public function getDefaultRuleData()
	{
		return Array(
						'threshold_type' => $this->getDefaultDiscountType(),
						'threshold_value' => $this->getDefaultThresholdValue(),
					);
		
	}//end getDefaultRuleData()
		
	
	
	/**
	 * Evaluate the asset data based on the rule data
	 * 
	 * @param array &$asset_data	The asset data
	 * @param array $rule_data		The rule data
	 * 
	 * @return boolean
	 * @access public
	 */
	public function evaluate(&$asset_data, $rule_data)
	{
		$price = array_get_index($asset_data, 'price', 0);
		$quanity = array_get_index($asset_data, 'quantity', 0);

		// Minimun cost required for free shipping
		$threshold_value = array_get_index($rule_data, 'threshold_value', 0);
		if ($threshold_value >= 0) {
			$comp_value = $rule_data['threshold_type'] == 'amount' ? $price : $quanity;
			if ($comp_value >= $threshold_value) {
				$asset_data['free_shipping'] = TRUE;
			}
		}
		
		return TRUE;
		
	}//end evaluate()
	
}//end class

?>
