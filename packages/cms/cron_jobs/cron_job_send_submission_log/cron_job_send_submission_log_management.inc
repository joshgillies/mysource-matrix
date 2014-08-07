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
* $Id: cron_job_send_submission_log_management.inc,v 1.2 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Status_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __cms__
*/
class Cron_Job_Send_Submission_Log_Management extends Asset_Management
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
						'type'				=> Array(
												'default'	=> 'repeating',
											   ),
						'display_name'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'parameters'	=> Array(														
														'allow_empty'	=> FALSE,
													   ),
												'description'	=> 'Name of the cron job set by the user',
											),											   
						'first_run'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Cron jobs very initial run time set by the user',
											),
						'previous_run'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Last time the job actually ran',
											),											
						'submissions_range' => Array(											   
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'last_run',
												'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'last_run'	=> 'The last run',
																			'date'	=> 'The given "Start" date',
																		   ),
													   ),
												'description'   => 'Pick the form submissions based on this option',
											),											   
						'submissions_from_date'	=> Array(
												'added'         => '0.1',
												'type'          => 'datetime',
												'default'       => '---------- --:--:--',
												'parameters'    => Array(
																	'allow_null'    => TRUE,
																	'show'          => Array('d','m','y','h','i'),
																	),
												'description'   => 'For very first run, look into submission logs since this date',
												'is_admin'      => TRUE,
											),
						'logs_format'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'csv',
												'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'csv'	=> 'Comma seperated values (CSV)',
																			'csv_nhr'	=> 'Comma seperated values (CSV) - No header row',
																			'xml'	=> 'XML format',																			
																		   ),
													   ),
												'description'	=> 'Send submission logs in this format',
											),
						'csv_delimiter'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> ',',
												'parameters'	=> Array(														
														'allow_empty'	=> FALSE,
													   ),
												'description'	=> 'Delimiter to use for submission log in CSV format',
											),
						'email_format'		=> Array(
												'added'     => '0.1',
												'type'      => 'email_format',
												'default'   => Array(),
												'description'	=> 'Email format to use when sending the submission logs',
											),                                          
				);

	}//end constructor

}//end class
?>
