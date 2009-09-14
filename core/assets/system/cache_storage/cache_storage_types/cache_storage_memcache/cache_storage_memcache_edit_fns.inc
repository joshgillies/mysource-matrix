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
* $Id: cache_storage_memcache_edit_fns.inc,v 1.2 2009/09/14 06:49:26 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cache_storage/cache_storage/cache_storage_edit_fns.inc';

/**
* Cache_Storage_Memcache_Edit_Fns
*
* Empty.
*
* @author  James Hurst <james@riverratrecords.com>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Storage_Memcache_Edit_Fns extends Cache_Storage_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


}//end class
?>
