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
* $Id: promotional_rule_restriction_edit_fns.inc,v 1.1 2013/09/13 08:06:44 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Promotional_Rule_Restriction_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Restriction_Edit_Fns extends Asset_Edit_Fns
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
	function paintRuleRestriction(&$asset, &$o, $prefix, $rule_data, $write_access)
	{		
		
	}//end paintRuleRestriction()
	
	
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
	function processRuleRestriction(&$asset, &$o, $prefix, &$rule_data)
	{
		
	}//end processRuleRestriction()
	
	
}//end class

?>