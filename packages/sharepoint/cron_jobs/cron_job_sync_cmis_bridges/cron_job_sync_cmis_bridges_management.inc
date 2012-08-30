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
* $Id: cron_job_sync_cmis_bridges_management.inc,v 1.3 2012/08/30 00:59:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Sync_CMIS_Bridges_Management
*
* @see Asset_Management
*
* @version $Revision: 1.3 $
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class Cron_Job_Sync_CMIS_Bridges_Management extends Asset_Management
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
						'type'					=> Array(
														'default'	=> 'repeating',
												   ),
						'when'					=> Array(
														'default'	=> 'DL=00:00',
												   ),
						'long'					=> Array(
												   		'default'	=> TRUE
												   ),											   					   
					  );

	}//end constructor


}//end class
?>
