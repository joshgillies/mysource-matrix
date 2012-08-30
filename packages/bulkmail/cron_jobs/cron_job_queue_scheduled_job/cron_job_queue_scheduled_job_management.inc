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
* $Id: cron_job_queue_scheduled_job_management.inc,v 1.2 2012/08/30 00:57:13 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Queue_Scheduled_Job_Management
*
* Purpose
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __bulkmail__
*/
class Cron_Job_Queue_Scheduled_Job_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars	= Array(
						'type'					=> Array(
													'default'   => 'repeating',
		                                 		   ),
						'when'					=> Array(
		                                    		'default'   => 'ET=',
												   ),
					  );

	}//end constructor


}//end class

?>
