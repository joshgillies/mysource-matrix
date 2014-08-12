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
* $Id: report_asset_statuses_management.inc,v 1.5 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Asset_Statuses_Management
*
* @see Asset_Management
*
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Report_Asset_Statuses_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$this->vars = Array(
						'search_status'							=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'selection',
																	'default'		=> SQ_STATUS_LIVE,
																	'description'	=> 'The statuses we are interested in',
																	'parameters'	=> Array(
																						'options'		=> get_status_names(),
																						'multiple'		=> TRUE,
																						'allow_empty'	=> FALSE,
																					   ),
																   ),
						'status_change_comparator_more_than'	=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'boolean',
																	'default'		=> 0,
																	'description'	=> 'Are we interested in status changes before or after the time specified?',
																	'parameters'	=> Array(
																						'allow_empty'	=> FALSE,
																					   ),
																   ),
						'status_change_period'					=> Array(
																	'added'			=> '0.1',
																	'type'			=> 'duration',
																	'default'		=> 0,
																	'description'	=> 'Number of seconds in the past that we want to compare status change time to',
																   ),
					  );

	}//end constructor


}//end class

?>
