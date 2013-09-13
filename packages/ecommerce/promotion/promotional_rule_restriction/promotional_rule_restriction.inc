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
* $Id: promotional_rule_restriction.inc,v 1.1 2013/09/13 08:06:44 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Promotional_Rule_Restriction
*
* This is the base class for all the promotional rule restriction assets
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Restriction extends Asset
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
	 * @param array $asset_data		The asset data
	 * @param array $restrcition_data	The rule data
	 * 
	 * @return boolean
	 * @access public
	 */
	public function validate($asset_data, $rule_data)
	{
		return TRUE;
		
	}//end validate()
	
	
}//end class

?>