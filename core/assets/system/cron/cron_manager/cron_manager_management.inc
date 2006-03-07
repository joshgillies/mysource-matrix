<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: cron_manager_management.inc,v 1.17 2006/03/07 06:01:24 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Manager_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'epoch'						=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'The GMT time of the "first" run that the Cron Manager made (or will make) was at this time',
													   ),
						'refresh_time'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'duration',
														'default'		=> 900,
														'description'	=> 'The number of seconds between cron runs, whenever you change this update the "epoch" to the first time that this change will be felt',
													   ),
						'last_run'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'The last time that the cron manager was run',
													   ),
						'running'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'Are we currently running ?',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'run_check'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'The number of times that a cron run has been started and stopped because the running flag is set true, this get\'s reset in run() after each pass over what jobs have to be done',
													   ),
						'warn_after_num_run_checks'	=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 3,
														'description'	=> 'When run_check hits this number a warning will be sent out to announce the possible un-removed locking',
													   ),
						'num_failed_attempts'		=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 3,
														'description'	=> 'The number of times that a cron job can fail and request to have another attempt at running',
													   ),
						'exclude_times'				=> Array(
														'added'			=> '0.2',
														'type'			=> 'serialise',
														'default'		=> Array(
																			0	=> Array(
																					'from'	=> 0,
																					'to'	=> 0,
																					'days'	=> Array(),
																				   ),
																		   ),
														'description'	=> 'Cronjob Exlude times',
													   ),
					  );

	}//end constructor


}//end class
?>
