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
* $Id: calendar_event_recurring_management.inc,v 1.1 2004/07/28 01:27:23 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Recurring_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Calendar_Event_Recurring_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => '',
												'description' => 'The name of the event',
												),
							'description' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',  'default'   => '',
												'description' => 'Description of the event',
												),
							'start_date' => Array(
												'added'       => '0.0.1',
												'type'        => 'datetime',
												'default'     => '---------- --:--:--',
												'description' => 'The starting date and/or time of the event',
												),
							'end_date' => Array(
												'added'       => '0.0.1',
												'type'        => 'datetime',
												'default'     => '---------- --:--:--',
												'description' => 'The ending date and/or time of the event',
												),
							'frequency' => Array(
												'added'       => '0.0.1',
												'type'        => 'text',
												'default'     => '',
												'description' => 'The frequency of the recurrence',
												),
							'stop_date' => Array(
												'added'       => '0.0.1',
												'type'        => 'datetime',
												'default'     => '---------- --:--:--',
												'parameters'  => Array('allow_null' => true),
												'description' => 'No occurrences will occur after this date',
												),
							);

		$this->components = Array(
								'__name__' => Array(
									'default_weighting' => '1',
								),
								'name' => Array (
									'description'       => 'Event name',
									'default_weighting' => '1',
								),
								'description' => Array (
									'description'       => 'Event description',
									'default_weighting' => '1',
								),
								'start_date' => Array (
									'description'       => 'Start date and/or time of first occurrence',
									'default_weighting' => '1',
								),
								'end_date' => Array (
									'description'       => 'End date and/or time of first occurrence',
									'default_weighting' => '1',
								),
								'stop_date' => Array (
									'description'       => 'Stop date for recurrence',
									'default_weighting' => '1',
								),
							);

	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'js/frequency_chooser.js',
					 );

	}//end _getFileList()
	
	
}//end class

?>