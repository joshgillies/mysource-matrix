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
* $Id: promotional_rule_restriction_compare_value_management.inc,v 1.1 2013/09/13 08:10:06 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Promotional_Rule_Restriction_Compare_Value_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Rule_Restriction_Compare_Value_Management extends Asset_Management
{
	
	
	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);
		
	}//end constructor
	
	
}//end class

?>