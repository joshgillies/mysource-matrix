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
* $Id: search_log_report_management.inc,v 1.1 2005/04/13 04:38:46 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Management
*
* @see Asset_Management
*
* @author Robert Howard <rhoward@squiz.net>
* @author Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Log_Report_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_Log_Report_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['rollover_period'] = Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'parameters'	=> Array(
																'options'		=> Array(
																					'monthly'	=> 'Monthly',
																					'weekly'	=> 'Weekly',
																					'daily'		=> 'Daily',
																				   ),
																'multiple'		=> false,
																'allow_empty'	=> false,
															   ),
											'default'		=> 'weekly',
											'description'	=> 'The rollover period, determines how often the report pages are archived - weekly, monthly, or daily',
										 );

		$this->vars['row_count'] = Array(
									'added'			=> '0.1',
									'type'			=> 'int',
									'default'		=> 25,
									'description'	=> 'The number of rows to include in the generated report',
								   );

		$this->vars['generated_reports'] = Array(
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Contains a list of previously-generated reports, in the format:
																	Array(
																		"YYYY-mm-dd"	=> "YYYY-mm-dd (Rollover_Period)",
																	}
																Where Rollover_Period is $this->attr("rollover_period")',
										   );



	}//end constructor


}//end class

?>