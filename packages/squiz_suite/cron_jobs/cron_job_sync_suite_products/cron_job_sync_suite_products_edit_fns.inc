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
* $Id: cron_job_sync_suite_products_edit_fns.inc,v 1.2 2012/08/30 00:59:25 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Sync_Suite_Products_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Cron_Job_Sync_Suite_Products_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Sync_Suite_Products_Edit_Fns()
	{
		parent::__construct();

	}//end constructor


}//end class

?>
