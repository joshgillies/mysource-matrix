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
* $Id: page_upcoming_events_list_management.inc,v 1.2 2006/12/07 05:51:52 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Upcoming_Events_List_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Upcoming_Events_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Upcoming_Events_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'num_events'	=> Array(
											'type'			=> 'int',
											'default'		=> 3,
											'parameters'	=> Array(
																'allow_negative'	=> FALSE,
																'range_lower'		=> 1,
																'range_upper'		=> 255,
															   ),
										   ),
						'horizon'		=> Array(
											'type'			=> 'int',
											'default'		=> 180,
											'parameters'	=> Array(
																'allow_negative'	=> FALSE,
																'range_lower'		=> 1,
																'range_upper'		=> 800,
															   ),
										   ),
						'root_nodes'	=> Array(
											'type'		=> 'serialise',
											'default'	=> Array(),
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