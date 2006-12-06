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
* $Id: cron_job_generate_log_reports_management.inc,v 1.2 2006/12/06 04:24:01 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Generate_Log_Reports_Management
*
* @see Asset_Management
*
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Generate_Log_Reports_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Generate_Log_Reports_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type'	=> Array(
									'default'	=> 'repeating',
								   ),
						'when'	=> Array(
									'default'	=> 'ET=',
								   ),
					  );

	}//end constructor


}//end class
?>
