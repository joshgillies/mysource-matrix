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
* $Id: funnelback_search_log_report_management.inc,v 1.2 2012/08/30 00:58:43 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Funnelback_Search_Log_Report_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>	
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Log_Report_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Funnelback_Search_Log_Report_Management(&$pm)
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
