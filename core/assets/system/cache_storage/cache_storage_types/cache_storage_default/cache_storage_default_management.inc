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
* $Id: cache_storage_default_management.inc,v 1.2.2.2 2009/09/21 03:47:58 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cache_Storage_Default_Management
*
* @see Asset_Management
*
* @author  James Hurst <james@riverratrecords.com>
* @version $Revision: 1.2.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Storage_Default_Management extends Asset_Management
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
