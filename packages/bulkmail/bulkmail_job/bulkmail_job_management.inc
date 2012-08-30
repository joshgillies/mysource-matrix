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
* $Id: bulkmail_job_management.inc,v 1.13 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bulkmail_Job_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars = Array(
						'name'						=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'use_post_office_header'	=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'description'	=> 'Whether to use the post office header details.',
													   ),
						'content_text_only'			=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'When TRUE, the bulkmail will be text only.',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'content_body_filter'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'content_body_filter_match'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'content_body_filter_text'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'text',
														'default'		=> '',
													   ),
						'user_subscriptions_status'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														'description'	=> 'Determines if the user subscription is activated or not.',
													   ),
						'subscriptions'				=> Array(
														'added'			=> '0.2',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'Data about the user subscriptions.',
													   ),
						'scheduled_time'			=> Array(
														'added'			=> '0.2',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'Data about the job scheduling.',
													   ),
						'scheduling_status'			=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														'description'	=> 'Determines if the job scheduling is activated or not.',
													   ),
						'last_scheduled_run'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'Registers the last scheduled run performed by cron job',
													   ),
					  );

	}//end constructor


}//end class
?>
