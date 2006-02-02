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
* $Id: page_calendar_restricted_management.inc,v 1.3 2006/02/02 03:55:37 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Restricted_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Restricted_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Calendar_Restricted_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'start_date'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> FALSE,
															   ),
											'description'	=> 'The starting date and/or time of the event',
											'update_fn'		=> '',
										   ),
						'end_date'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array('allow_null' => FALSE),
											'description'	=> 'The ending date and/or time of the event',
											'update_fn'		=> '',
										   ),
						'days_per_page'	=> Array(
											'type'			=> 'int',
											'default'		=> 1,
											'parameters'	=> Array('allow_negative' => FALSE),
										   ),
						'next_text'		=> Array(
											'type'		=> 'text',
											'default'	=> 'Next %n% Days >>',
										   ),
						'prev_text'		=> Array(
											'type'		=> 'text',
											'default'	=> '<< Previous %n% days',
										   ),
					  );

		$this->components = Array(
								'short_name'	=> Array(
													'description'		=> 'Short Name',
													'default_weighting'	=> '1',
												   ),
							);

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>