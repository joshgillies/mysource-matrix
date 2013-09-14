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
* $Id: promotional_condition_compare_value.inc,v 1.1 2013/09/14 01:43:20 cupreti Exp $
*
*/


require_once dirname(__FILE__).'/../../promotional_condition/promotional_condition.inc';

/**
* Promotional_Condition_Compare_Value
*
* If the value "soruce" value "compares" with "compare" value,
* the condition will be successful
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Condition_Compare_Value extends Promotional_Condition
{
	
	
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor
	

	/**
	 * Return the discount types supported by this promotional condition
	 * 
	 * @return array
	 * @access public
	 */
	public function getComparisionTypes()
	{
		return Array(					
						'string_exact' 		=> 'String - Exact match',
						'string_begins' 	=> 'String - Begins with',
						'string_ends' 		=> 'String - Ends with',
						'string_contains' 	=> 'String - Contains',
						'numeric_equals' 	=> 'Numeric - Equals',
						'numeric_lt' 		=> 'Numeric - Less than',
						'numeric_gt' 		=> 'Numeric - Greater than',
						'numeric_lte' 		=> 'Numeric - Less than or equals',
						'numeric_gte' 		=> 'Numeric - Greater than or equals',
						'regex_m' 			=> 'Regular expression - Match',
						'regex_nm' 			=> 'Regular expression - Not match',
					);
		
	}//end getDiscountTypes()	
	
	
	/**
	 * Return the default condition data.
	 * 
	 * The structure of the condition data is 
	 * 		Array(
	 * 				'source_field'		=> source value for comparision
	 * 				'compare_field'		=> value to compare source value to
	 *				'compare_type'		=> comparision type (regex, string match, numeric)
	 * 			 )
	 * 
	 * @return array
	 * @access public
	 */
	public function getDefaultConditionData()
	{
		return Array(
						'source_value'		=> '',
						'compare_value'		=> '',
						'comparison_type'	=> 'string_exact',
						'case_sensitive'	=> FALSE,
					);
		
	}//end getDefaultRuleData()
	
	
	/**
	 * Evaluate the asset data based on the condition data
	 * 
	 * This method needs to be overridden by child assets
	 * 
	 * @param array $asset_data			The asset data
	 * @param array $condition_data		The condition data
	 * 
	 * @return boolean
	 * @access public
	 */
	public function validate($asset_data, $condition_data)
	{
		$source_value = array_get_index($condition_data, 'source_value', '');
		$compare_value = array_get_index($condition_data, 'compare_value', '');
		// Alteast one value must be non-empty for the comparison
		if (empty($source_value) && empty($source_value)) {
			return FALSE;
		}		
		
		replace_global_keywords($source_value);
		replace_global_keywords($compare_value);

		$comparison_type = array_get_index($condition_data, 'comparison_type', '');
		if (empty($comparison_type) || !in_array($comparison_type, array_keys($this->getComparisionTypes()))) {
			return FALSE;
		}
		
		if (!array_get_index($condition_data, 'case_sensitive', FALSE)) {
			$source_value = strtolower($source_value);
			$compare_value = strtolower($compare_value);
		}
		
		$ret = FALSE;
		switch ($comparison_type) {
			case 'string_exact':
				$ret = $source_value == $compare_value;
			break;
			
			case 'string_begins':
				$ret = !empty($compare_value) && strpos($source_value, $compare_value) === 0;
			break;
			
			case 'string_ends':				
				$ret = !empty($compare_value) && strpos($source_value, $compare_value) === strlen($source_value)-strlen($compare_value);
			break;
			
			case 'string_contains':
				$ret = !empty($compare_value) && strpos($source_value, $compare_value) !== FALSE;
			break;			
			
			case 'numeric_equals':
				$ret = $source_value == $compare_value;
			break;
			
			case 'numeric_lt':
				$ret = $source_value < $compare_value;
			break;
			
			case 'numeric_gt':
				$ret = $source_value > $compare_value;
			break;
			
			case 'numeric_lte':
				$ret = $source_value <= $compare_value;
			break;
			
			case 'numeric_gte':
				$ret = $source_value >= $compare_value;
			break;
			
			case 'regex_m':
				$ret = !empty($compare_value) && @preg_match('/'.str_replace('/', '\/',$compare_value).'/', $source_value);
			break;
			
			case 'regex_nm':
				if (!empty($compare_value)) {
					$ret = @preg_match('/'.str_replace('/', '\/',$compare_value).'/', $source_value);
					if ($ret !== FALSE) {
						$ret = !$ret;
					}				
				}
			break;
			
		}//end switch		
		
		return $ret;
		
	}//end validate()
	
	
}//end class

?>
