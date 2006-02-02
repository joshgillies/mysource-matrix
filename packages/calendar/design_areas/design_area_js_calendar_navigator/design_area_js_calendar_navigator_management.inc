<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: design_area_js_calendar_navigator_management.inc,v 1.5 2006/02/02 04:10:57 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_JS_Calendar_Navigator_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
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
