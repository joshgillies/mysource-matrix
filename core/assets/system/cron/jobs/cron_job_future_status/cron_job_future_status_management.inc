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
* $Id: cron_job_future_status_management.inc,v 1.10 2006/12/06 04:24:01 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Status_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Status_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Status_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type'				=> Array(
												'default'	=> 'one_off',
											   ),
						'status'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'The status to change the asset to',
											   ),
						'dependants_only'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'If the status change should cascade to all children',
											   ),
					  );

	}//end constructor


}//end class
?>
