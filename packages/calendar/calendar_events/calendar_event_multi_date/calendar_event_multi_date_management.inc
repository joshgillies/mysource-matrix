<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_multi_date_management.inc,v 1.3.6.1 2009/08/20 01:10:20 lwright Exp $
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
* @version $Revision: 1.3.6.1 $
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
											'is_contextable'	=> TRUE,
										   ),
						'short_date'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Summary of the dates',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
					  );


		$this->components = Array(
								'name'		=> Array(
												'description'		=> 'Name',
												'default_weighting'	=> '1',
											   ),
								'contents'	=> Array (
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
