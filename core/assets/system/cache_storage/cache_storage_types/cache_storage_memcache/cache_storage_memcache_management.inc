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
* $Id: cache_storage_memcache_management.inc,v 1.2 2009/09/14 06:49:26 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cache_Storage_Memcache_Management
*
* @see Asset_Management
*
* @author  James Hurst <james@riverratrecords.com>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Storage_Memcache_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
							/*
							'contents' => Array(
								'added'		=>	'0.0.1',
								'type'		=>	'text',
								'default'	=>	'',
							),
							'weblink' => Array(
								'added'		=>	'0.0.1',
								'type'		=>	'text',
								'default'	=>	'',
							),
							'title' => Array(
								'added'		=>	'0.0.1',
								'type'		=>	'text',
								'default'	=>	'',
							),
							*/
						);

	}//end constructor


}//end class
?>
