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
* $Id: calendar_event_single_management.inc,v 1.6 2006/12/07 05:51:51 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Single_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Single_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',  'default'   => '',
											'description'	=> 'The name of the event',
										   ),
						'description'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'wysiwyg',  'default'   => '',
											'description'	=> 'Description of the event',
										   ),
						'start_date'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> TRUE,
																'show'			=> Array(
																					'd','m','y','h','i',
																				   ),
															   ),
											'description'	=> 'The starting date and/or time of the event',
											'update_fn'		=> '',
										   ),
						'end_date'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array('allow_null'	=> TRUE),
											'description'	=> 'The ending date and/or time of the event',
											'update_fn'		=> '',
										   ),
					  );

		$this->components = Array(
								'__name__'		=> Array(
													'default_weighting'	=> '1',
												   ),
								'name'			=> Array (
													'description'		=> 'Event name',
													'default_weighting'	=> '1',
												   ),
								'description'	=> Array (
													'description'		=> 'Event description',
													'default_weighting'	=> '1',
												   ),
								'start_date'	=> Array (
													'description'		=> 'Start date and/or time',
													'default_weighting'	=> '1',
												   ),
								'end_date'		=> Array (
													'description'		=> 'End date and/or time',
													'default_weighting'	=> '1',
												   ),
							);

	}//end constructor


}//end class

?>