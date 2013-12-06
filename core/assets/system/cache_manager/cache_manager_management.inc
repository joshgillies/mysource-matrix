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
* $Id: cache_manager_management.inc,v 1.21 2012/08/30 01:09:10 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cache_Manager_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.21 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Manager_Management extends Asset_Management
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

		$this->vars = Array(
						'enabled'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Turns caching on or off',
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'public_caching'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Turns caching on or off for not logged in users',
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'permission_caching'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Turns read/write/admin level caching on or off',
													'parameters'	=> Array('allow_empty' => FALSE),
												   ),
						'group_caching'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Turns group level caching on or off',
													'parameters'	=> Array('allow_empty' => FALSE),
												   ),
						'num_cache_dirs'		=> Array(
													'added'		=> '0.1',
													'type'		=> 'int',
													'default'	=> '20',
												   ),
						'expiry'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'int',
													'default'	=> '86400',
												   ),
						'type_codes'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'non_cachable_urls'		=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'browser_cache_expiry'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '',
													'allow_negative'	=> FALSE,
													'allow_empty'		=> TRUE,
										   ),
						'accelerator_cache_expiry'	=> Array(
													'added'			=> '0.4',
													'type'			=> 'int',
													'default'		=> '',
													'allow_negative'	=> FALSE,
													'allow_empty'		=> TRUE,
										   ),
						'cache_storage_type'    => Array(
													'added'     => '0.3',
													'type'      => 'text',
													'default'   => 'default',
													'description'	=> 'The method for storing cached data',
						),
						'root_nodes'			=> Array(
													'added'		=> '0.2',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						
						'non_cacheable_header_urls' => Array(
													'added'		    => '0.4',
													'type'		    => 'serialise',
													'default'	    => Array(),
													'description'   => 'Root URLs to send cacheable header regardless of other Matrix settings',
												   ),

						'user_cacheable_header_urls' => Array(
													'added'		    => '0.4',
													'type'		    => 'serialise',
													'default'	    => Array(),
													'description'   => 'Root URLs to send user cacheable header regardless of other Matrix settings',
												   ),
			
						'cacheale_header_protocol'	=> Array(
													'added'			=> '0.4',
													'type'			=> 'selection',
													'default'		=> 'http',
													'description'	=> 'Whether to send cacheable headers over http/https/both',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'http' 	=> 'HTTP only',
																							'https' => 'HTTPs only',
																							'both'	=> 'Both',
																							),
																		),
												   ),
						'cache_control_http'		=> Array(
													'added'			=> '0.4',
													'type'			=> 'selection',
													'default'		=> 'public',
													'description'	=> 'Cache control level (public/private) for http',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'public' => 'Public',
																							'private' => 'Private',
																							),
																		),
												   ),
						'cache_control_https'		=> Array(
													'added'			=> '0.4',
													'type'			=> 'selection',
													'default'		=> 'public',
													'description'	=> 'Cache control level (public/private) for https',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'public' => 'Public',
																							'private' => 'Private',
																							),
																		),

												   ),


					  );
					  
	}//end constructor


}//end class

?>
