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
* $Id: locking_method_redis_management.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Locking_Method_Redis_Management
*
* Purpose
*
*
* @author		Chris Smith <csmith@squiz.net>
* @version		$Revision: 1.2 $
* @package		MySource_Matrix_Packages
* @subpackage	__core__
*/
class Locking_Method_Redis_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager that will handle this asset
	*
	*/
	public function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end __construct()


}//end class

?>
