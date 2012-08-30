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
* $Id: calendar_event_modification_management.inc,v 1.10 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.10 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Modification_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Modification_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The modified name of the event',
										   ),
						'description'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'wysiwyg',
											'default'		=> '',
											'description'	=> 'The modified description of the event',
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
											'parameters'	=> Array(
																'allow_null'	=> TRUE,
															   ),
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