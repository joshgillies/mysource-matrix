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
* $Id: google_map_management.inc,v 1.8.4.1 2013/10/22 05:19:04 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Google_Map_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.8.4.1 $
* @package MySource_Matrix_Packages
* @subpackage google_maps
*/

class Google_Map_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Google_Map_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars	= Array (
						'api_key'				=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The API Key provided by Google',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> TRUE,
																   ),
												   ),
						'map_code'				=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '<div id="map" style="width:500px;height:500px;border:solid 0px;"></div>
<script type="text/javascript">
	gmap_init();
</script>',
													'description'	=> 'Map code',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> TRUE,
																   ),
												   ),
						'street_view_code'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '<div id="street_view" style="width:500px; height:500px"></div>',
													'description'	=> 'Map code',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> TRUE,
																   ),
												   ),
                        'map_control'		=> Array(
													'added'         => '0.0.1',
													'type'          => 'selection',
													'default'       => 'none',
													'description'   => '',
													'parameters'    => Array(
																		'options'       => Array(
												   											'none'			=> 'None',
																							'large'			=> 'Large Control',
																							'small'			=> 'Small Control',
												   											'zoom'			=> 'Zoom Control',
												   										   ),
		                                                               	'multiple'      => FALSE,
		                                                               	'allow_empty'   => FALSE,
																	   ),

		                                           ),
						'map_type_control'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable the Map Type Control of the map',
													'is_admin'		=> '1',
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'scale_control'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable the Scale Control of the map',
													'is_admin'		=> '1',
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'default_icon_size'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'TRUE',
													'description'	=> 'Use default google icon size',
													'is_admin'		=> '1',
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'use_google_marker_icon'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'TRUE',
													'description'	=> 'Use default google marker icon',
													'is_admin'		=> '1',
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'default_marker_icon'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The default icon for markers on the map',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> TRUE,
																	   ),
												   ),
						'default_icon_width'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '25',
													'description'	=> 'The default width of the icon',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'default_icon_height'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '25',
													'description'	=> 'The default height of the icon',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'default_latitude'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '0',
													'description'	=> 'The default latitude of the map',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'default_longitude'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '0',
													'description'	=> 'The default longitude of the map',
													'is_admin'		=> '1',
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'default_zoom_level'	=> Array (
														'added'			=> '0.1',
														'type'			=> 'int',
														'default'		=> '10',
														'description'	=> 'The default zoom level of the map',
														'is_admin'		=> FALSE,
														'parameters'	=> Array (
																			'allow_empty'	=> FALSE,
																	   ),
												   ),
                        'default_map_type'		=> Array(
													'added'         => '0.0.1',
													'type'          => 'selection',
													'default'       => 'ROADMAP',
													'description'   => '',
													'parameters'    => Array(
																		'options'       => Array(
																							'ROADMAP'			=> 'Road Map',
																							'SATELLITE'			=> 'Satellite Map',
												   											'HYBRID'			=> 'Hybrid Map',
												   											'TERRAIN'			=> 'Terrain Map',
																						   ),
		                                                               	'multiple'      => FALSE,
		                                                               	'allow_empty'   => FALSE,
																	   ),

		                                           ),
						'root_assetid'			=> Array(
		                                            'added'         => '0.1',
		                                            'type'          => 'assetid',
		                                            'default'       => '',
		                                            'description'   => 'Root node where user information is stored under.',
												   ),
                        'parameter_map'			=> Array (
                                                    'added'         => '0.1',
                                                    'type'          => 'parameter_map',
                                                    'default'       => Array(),
	'parameters'	=> Array(
							'parameters'	=> Array(
												'root_node'			=> 'Replacement Root node for the listing (must be a child of the static root node)',
												'asset_selection'	=> 'Array of specific asset IDs or selection group (specified assets must be children of the static related node)',
											   ),
						   ),
                                                   ),
                        'dynamic_vars'			=> Array(
                                                    'added'     => '0.1',
                                                    'type'      => 'serialise',
                                                    'default'   => Array (
                                                                    'latitude'           	=> '',
                                                   					'longitude'				=> '',
																	'description'			=> '',
                                                   					'zoom_level'			=> '',
                                                   					'map_type'				=> '',
                                                                   ),
                                                   ),
						'enable_listing'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable the Listing capability',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'use_custom_popup_html'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable custom HTML popup',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'enable_geocoder'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Use of Geocoding service',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																   ),
												   ),
						'asset_types_keywords'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array (),
													'description'	=> 'Store all the associate',
													'is_admin'		=> FALSE,
												   ),
						'toggle_root_nodes'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable toogling marker of different rootnodes.',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'latitude_keyword'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The latitude keyword to be used for the map',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'longitude_keyword'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The longitude keyword to be used for the map',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'description_keyword'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The description keyword to be used for the map',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'enable_street_view'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable Google Street View capability',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'enable_create_location'	=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'FALSE',
													'description'	=> 'Enable or Disable creating of new Google Map Location',
													'is_admin'		=> FALSE,
													'parameters'	=> Array (
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
					  );

	}//end constructor


    /**
    * Gets a list of all files that the bodycopy wants web accessable
    *
    * @return array
    * @access private
    */
    function _getFileList()
    {
        return Array(
                'css/style.css',
				'js/map_lib.js',
        		'images/star_icon.png',
        		'images/globe.png',
        		'images/arrowdown.png',
        		'images/glasses.png',
				'images/binocular.png',
        		'images/map.png',
			   );

	}//end _getFileList()


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
