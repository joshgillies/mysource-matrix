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
* $Id: promotional_rule.inc,v 1.1 2010/08/19 10:35:40 ata Exp $
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
* @version $Revision: 1.1 $
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
	 * @return void
	 * @access public
	 */
	public function evaluate(&$asset_data, $rule_data)
	{
		
	}//end evaluate()
	
	
}//end class

?>