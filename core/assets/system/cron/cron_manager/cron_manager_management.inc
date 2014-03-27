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
* $Id: cron_manager_management.inc,v 1.23 2012/11/08 06:16:11 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Manager_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Manager_Management extends Asset_Management
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
						'epoch'						=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'The GMT time of the "first" run that the Scheduled Jobs Manager made (or will make) was at this time.',
													   ),
						'refresh_time'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'duration',
														'default'		=> 900,
														'description'	=> 'The number of seconds between Scheduled Job runs, whenever you change this update the "epoch" to the first time that this change will be felt.',
													   ),
						'last_run'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'The last time that the Scheduled Jobs Manager was run.',
													   ),
						'running'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'Are we currently running?',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'run_check'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'The number of times that a Scheduled Job has been started and stopped because the running flag is set true, this get\'s reset in run() after each pass over what jobs have to be done.',
													   ),
						'warn_after_num_run_checks'	=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 3,
														'description'	=> 'When run_check hits this number a warning will be sent out to announce the possible un-removed locking.',
													   ),
						'num_failed_attempts'		=> Array(
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> 3,
														'description'	=> 'The number of times that a Scheduled Job can fail and request to have another attempt at running.',
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
														'description'	=> 'Scheduled Job Exlude times',
													   ),
						'current_job'				=> Array(
														'added'			=> '0.3',
														'type'			=> 'text',
														'default'		=> NULL,
														'description'	=> 'Currently Executing Job',
													   ),
						'dead_lock_email_to'		=> Array(
														'added'			=> '0.3',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'Users to send the Scheduled Jobs deadlock email to.',
													   ),
						'max_years'					=> Array(
														'added'			=> '0.3',
														'type'			=> 'int',
														'default'		=> 10,
														'description'	=> 'Number of years in the future to display in the dropdown that be set when setting up a Scheduled Job.',
													),



					  );

	}//end constructor


}//end class
?>
