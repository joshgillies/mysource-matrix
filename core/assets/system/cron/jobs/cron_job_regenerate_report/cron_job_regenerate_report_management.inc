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
* $Id: cron_job_regenerate_report_management.inc,v 1.1 2006/10/16 05:14:29 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* CCron_Job_Regenerate_Report_Management
*
* @see Asset_Management
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Regenerate_Report_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Regenerate_Report_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type'	=> Array(
									'default'	=> 'repeating',
								   ),
						'when'	=> Array(
									'default'		=> 'TI='.time().',15,60',
									'description'	=> 'encoded when str for cron job, default is repeating time interval',
								   ),
						'asset'	=> Array(
									'type'			=> 'assetid',
									'default'		=> '',
									'description'	=> 'the report asset that we are working on',
								   ),
					  );

	}//end constructor


}//end class
?>
