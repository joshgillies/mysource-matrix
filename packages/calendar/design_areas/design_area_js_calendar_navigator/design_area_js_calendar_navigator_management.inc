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
* $Id: design_area_js_calendar_navigator_management.inc,v 1.1 2004/09/10 04:23:40 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_JS_Calendar_Navigator_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Version$ - 1.0
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
			'target_assetid' => Array(
				'added'			=> '0.1',
				'type'			=> 'assetid',
				'default'		=> '',
				'description'   => 'Asset which will be controlled by this javascript calendar. This asset must supply 2 functions: getViewRequestName() and getDateRequestName(), which are the names of request variables that the target asset uses to determine what action to take. If asset is not supplied, this JavaScript calendar can be used standalone, with limited functionality.',
			),

			'element_id'=> Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> 'js_calendar_navigator',
				'description'	=> 'Element ID on HTML page'
			),

			'style_assetid' => Array(
				'added'			=> '0.1',
				'type'			=> 'assetid',
				'default'		=> '',
				'description'   => 'Optional CSS',
			),


		);

	}//end constructor


}//end class
?>
