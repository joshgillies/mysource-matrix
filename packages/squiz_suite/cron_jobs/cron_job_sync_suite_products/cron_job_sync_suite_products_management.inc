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
* $Id: cron_job_sync_suite_products_management.inc,v 1.3 2012/08/30 00:59:25 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Sync_Suite_Products_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>	
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Cron_Job_Sync_Suite_Products_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type'	=> Array(
									'default'	=> 'repeating',
								   ),
						'when'	=> Array(
									'default'	=> 'HR=00',
								   ),
					  );

	}//end constructor


}//end class
?>
