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
* $Id: cache_storage_redis_management.inc,v 1.1 2013/01/22 05:03:07 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cache_Storage_Redis_Management
*
* @see Asset_Management
*
* @author  James Hurst <james@riverratrecords.com>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Storage_Redis_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						);

	}//end constructor


}//end class
?>
