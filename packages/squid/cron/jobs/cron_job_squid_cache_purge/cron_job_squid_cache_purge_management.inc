<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge_management.inc,v 1.5 2012/08/30 00:59:22 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Squid_Cache_Purge_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Cron_Job_Squid_Cache_Purge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager installing this asset
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
											'description'	=> 'The ID of the asset which cache to be cleared',
										   ),
						'cascade'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Whether the purge action should include children of the selected asset',
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