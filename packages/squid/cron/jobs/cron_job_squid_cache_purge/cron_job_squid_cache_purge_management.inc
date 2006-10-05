<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge_management.inc,v 1.1.1.1 2006/10/05 05:37:23 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Squid_Cache_Purge_Management
*
* @see Asset_Management
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.1.1.1 $
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