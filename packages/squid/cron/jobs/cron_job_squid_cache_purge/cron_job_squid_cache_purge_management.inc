<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge_management.inc,v 1.2 2006/12/07 02:54:14 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Squid_Cache_Purge_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Cron_Job_Squid_Cache_Purge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Squid_Cache_Purge_Management(&$pm)
	{
		$this->Asset_Management($pm);

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