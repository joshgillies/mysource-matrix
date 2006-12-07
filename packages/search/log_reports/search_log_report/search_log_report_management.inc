<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: search_log_report_management.inc,v 1.4 2006/12/07 01:07:18 bcaldwell Exp $
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
* @version $Revision: 1.4 $
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
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
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