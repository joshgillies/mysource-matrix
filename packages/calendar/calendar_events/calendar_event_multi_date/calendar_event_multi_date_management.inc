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
* $Id: calendar_event_multi_date_management.inc,v 1.1.2.1 2007/11/09 05:42:57 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Asset Management for Multi-Date Calendar Event
*
* @see Asset_Management
*
* @author  Greg Sherwoood <greg@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Multi_Date_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Multi_Date_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The full name of the calendar_event_multi_date',
											'update_fn'		=> '',
										   ),
						'short_date'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Summary of the dates',
											'is_admin'		=> FALSE,
										   ),
					  );


		$this->components = Array(
								'name'		=> Array(
												'description'		=> 'Name',
												'default_weighting'	=> '1',
											   ),
								'contents'	=> Array(
												'description'		=> 'Contents (HTML stripped)',
												'default_weighting'	=> '1',
											   ),
							);

	}//end constructor


	/**
	* Get the web-accessible files used by this asset type
	*
	* @return array
	* @access public
	*/
	function _getFileList()
	{
		return Array('js/dates_chooser.js');

	}//end _getFileList()


}//end class

?>
