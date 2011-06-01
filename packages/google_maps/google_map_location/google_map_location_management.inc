<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: google_map_location_management.inc,v 1.1.1.1 2008/07/04 05:19:54 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Google_Map_Location_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage google_maps
*/

class Google_Map_Location_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Google_Map_Location_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars	= Array (
						'name'=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The name of the location',
										'is_admin'		=> FALSE,
										'parameters'	=> Array (
															'allow_empty'	=> FALSE,
															   ),
										   ),
						'short_name'=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The name of the location',
										'is_admin'		=> FALSE,
										'parameters'	=> Array (
															'allow_empty'	=> FALSE,
															   ),
										   ),
						'longitude'=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The longitude of the location',
										'is_admin'		=> FALSE,
										'parameters'	=> Array (
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'latitude'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The latitude of the location',
										'is_admin'		=> FALSE,
										'parameters'	=> Array (
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'description'=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The description of the location',
										'is_admin'		=> FALSE,
										'parameters'	=> Array (
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'icon_url'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The URL for this location icon',
										'is_admin'		=> FALSE,
										'parameters'	=> Array (
															'allow_empty'	=> TRUE,
															   ),
										   ),
					  );

	}//end constructor


}//end class
?>
