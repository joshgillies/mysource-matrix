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
* $Id: report_asset_statuses_management.inc,v 1.2 2005/02/16 08:58:56 brobertson Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Asset_Statuses_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Report_Asset_Statuses_Management(&$pm)
	{
		$this->Asset_Management($pm);

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$this->vars = Array(
							'search_status' => Array(
								'added'       => '0.1',
								'type'        => 'selection',
								'default'     => SQ_SC_STATUS_ALL,
								'description' => 'The status we are interested in',
								'parameters'  => Array(
									'options'  => Array(SQ_SC_STATUS_ALL => 'Any Status') + get_status_names(),
									'multiple' => false,
									'allow_empty' => false,
								),
							),
							'status_change_comparator_more_than' => Array(
								'added'       => '0.1',
								'type'        => 'boolean',
								'default'     => 0,
								'description' => 'Are we interested in status changes before or after the time specified?',
								'parameters'  => Array(
									'allow_empty' => false,
								),
							),
							'status_change_period' => Array(
								'added'         => '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'Number of minutes in the past that we want to compare status change time to',
							),
		);

	}//end constructor


}//end class

?>