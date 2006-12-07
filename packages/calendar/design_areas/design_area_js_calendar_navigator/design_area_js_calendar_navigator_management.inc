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
* $Id: design_area_js_calendar_navigator_management.inc,v 1.6 2006/12/07 05:51:51 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_JS_Calendar_Navigator_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Design_Area_JS_Calendar_Navigator_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_JS_Calendar_Navigator_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'target_assetid'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Asset which will be controlled by this javascript calendar. If asset is not supplied, this JavaScript calendar can be used standalone, with limited functionality.',
											   ),
						'element_id'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> 'js_calendar_navigator',
												'description'	=> 'Element ID on HTML page',
											   ),
						'style_assetid'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Optional CSS',
											   ),
					  );

	}//end constructor


}//end class
?>
