<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_datetime_management.inc,v 1.4 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_DateTime_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_DateTime_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Design_Area_DateTime_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'format' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> 'h:ia  d/m/Y',
								'description'   => 'Format of the datetime field, see PHP\'s date fn for format specifications <a href="http://au2.php.net/date">http://au2.php.net/date</a>'
							),
							'datetime' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'default'		=> 'now',
								'parameters'	=> Array(
									'options' => Array(
											'now'		=> 'When the page is loaded',
											'updated'	=> 'When the page was last modified',
									),
								),
								'description'	=> 'The value to be used',
							),
						);

	}//end constructor

}//end class
?>
