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
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Condition_Simple_Edit_Mode_Management
*
* @see Asset_Management
*
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_In_Maintenance_Mode_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor


}//end class
?>