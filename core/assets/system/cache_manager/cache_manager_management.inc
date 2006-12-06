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
* $Id: cache_manager_management.inc,v 1.10 2006/12/06 04:23:57 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cache_Manager_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.10 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Cache_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

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
					  );

	}//end constructor


}//end class

?>
