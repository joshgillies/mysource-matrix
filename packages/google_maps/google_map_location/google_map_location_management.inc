<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: google_map_location_management.inc,v 1.4 2012/08/30 00:58:46 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Google_Map_Location_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.4 $
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
										'is_contextable'	=> TRUE,
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
										'is_contextable'	=> TRUE,
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
										'is_contextable'	=> TRUE,
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
