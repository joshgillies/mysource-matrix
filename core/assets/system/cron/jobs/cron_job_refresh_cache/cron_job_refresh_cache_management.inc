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
* $Id: cron_job_refresh_cache_management.inc,v 1.1 2007/02/26 05:02:57 skim Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Refresh_Cache_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Refresh_Cache_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Refresh_Cache_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type'					=> Array(
													'default'	=> 'repeating',
												   ),
						'max_execution_time'	=> Array(
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'the max execution time for each HTTP Request to URL',
												   ),
						'asset'					=> Array(
													'type'			=> 'assetid',
													'default'		=> '',
													'description'	=> 'the report asset that we are working on',
												   ),
					  );

	}//end constructor


}//end class
?>
