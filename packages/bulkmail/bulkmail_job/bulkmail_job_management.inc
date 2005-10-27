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
* $Id: bulkmail_job_management.inc
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
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bulkmail_Job_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
											   ),
						'details_setting'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(
																		'use_post_office_header'	=> true,
																	   ),
													'description'	=> 'Whether to use the post office settings for various details.',
											   ),
						'current_state'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
											   ),
						'last_processed'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'Timestamp of when the last recipient is processed.',
											   ),
						'parent_process_pid'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'The id of the parent process that last processed this asset.',
											   ),
						'child_process_pids'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'The history of all the ids of the child processes that processed this asset.',
											   ),
					  );

	}//end constructor

}//end class
?>