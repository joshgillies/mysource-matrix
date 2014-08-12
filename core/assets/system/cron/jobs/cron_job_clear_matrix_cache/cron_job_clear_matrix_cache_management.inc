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
* $Id: cron_job_clear_matrix_cache_management.inc,v 1.3 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Clear_Matrix_Cache_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Cron_Job_Clear_Matrix_Cache_Management extends Asset_Management
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
						'type'			=> Array(
											'default'	=> 'one_off',
										   ),
						'ownerid'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'assetid',
											'default'		=> 0,
											'description'	=> 'The ID of the asset which Matrix cache to be cleared',
										   ),
						'level'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> 'single',
											'description'	=> 'Asset or, asset and children/dependent',
										   ),
						'type_codes'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Asset type to clear cache for',
										   ),
						'repopulate'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Whether we should repopulate the cache by requesting the URLs of the asset',
										   ),
					  );

	}//end constructor


}//end class

?>