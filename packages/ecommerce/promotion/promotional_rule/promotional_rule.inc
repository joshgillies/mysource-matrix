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
* $Id: promotional_rule.inc,v 1.3 2013/09/13 07:57:53 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Promotional_Rule
*
* This is the base class for all the promotional rule assets
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule extends Asset
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
	 * Evaluate the asset data based on the rule data
	 * 
	 * This method needs to be overridden by child assets
	 * 
	 * @param array &$asset_data	The asset data
	 * @param array &$rule_data		The rule data
	 * 
	 * @return boolean
	 * @access public
	 */
	public function evaluate(&$asset_data, $rule_data)
	{
		// Check the conditions applied the rule
		$success = TRUE;
		$restrictions = array_get_index($rule_data, 'restrictions', Array());
		if (!empty($restrictions)) {
			$restriction_type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('promotional_rule_restriction');
			foreach($restrictions as $restriction) {
				$restriction_type_code = $restriction['restriction_type_code'];
				if (in_array($restriction_type_code, $restriction_type_codes)) {
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($restriction_type_code);
					$restriction_asset = new $restriction_type_code();
					if (!$restriction_asset->validate($asset_data, $restriction['restrcition_data'])) {
						// If any condition fails, the rule will not apply
						$success = FALSE;
						break;
					}
				}// end if
			}//end foreach
		}//end if

		return $success;
		
	}//end evaluate()
	
	
}//end class

?>
