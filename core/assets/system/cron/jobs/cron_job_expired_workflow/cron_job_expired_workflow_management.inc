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
* $Id: cron_job_expired_workflow_management.inc,v 1.5 2006/12/06 04:24:00 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Expired_WorkFlow_Management
*
* @see Asset_Management
*
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Expired_WorkFlow_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Expired_WorkFlow_Management(&$pm)
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
