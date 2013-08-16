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
* $Id: google_map.inc,v 1.33 2013/08/16 06:49:17 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';

/**
* Google Map Location
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage google_maps
*/
class Google_Map extends Page_Asset_Listing
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
		$this->_ser_attrs	= TRUE;
		$this->api_key		= '';

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Standard pages create a bodycopy asset when they are created
	*
	* @param array  &$link  information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link, $args=Array())
	{
		if (!parent::_createAdditional($link)) return FALSE;

		$this->createBodycopy('Popup Window');

		return TRUE;

	}//end _createAdditional()


	/**
	* This function create the bodycopy for the map to be customized
	*
	* @return unknown
	* @access public
	*/
	function createBodycopy($name)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$bodycopy->setAttrValue('name', $name);
		$content_type = 'content_type_raw_html';

		$content = $this->_getDefaultBodycopyContent(str_replace(' ','_', strtolower($name)));

		$bc_data = Array (
					'attributes'	=> Array (
										'content_type'	=> $content_type,
									   ),
					'content'		=> $content,
				   );

		$copy_link	= Array (
						'asset'			=> $this,
						'link_type'		=> SQ_LINK_TYPE_2,
						'is_dependant'	=> 1,
						'is_exclusive'	=> 1,
						'value'			=> $name,
					  );

		if (!$bodycopy->create($copy_link, $bc_data)) {
			return FALSE;
		}//end if

	}//end createBodycopy()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{

		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* This function return the cache key of the map based on root nodes
	*
	* @return string
	* @access public
	*/
	public function getCacheKey()
	{
		$root_nodes	= $this->getRootNodes();
		$hash_str	= '';
		foreach ($root_nodes as $root_assetid) {
			$hash_str	.= $root_assetid;
		}//end
		$hash	= md5($hash_str);

		return $hash;
	}//end getCacheKey()


	/**
	* Returns an array of the result of query execution
	*
	* @param string $cache_key  the cache key for this entry
	*
	* @return array
	* @access public
	*/
	public function getCachedResultSet($cache_key='')
	{
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$result = $cm->loadFromCache($this->id, $this->type(), $cache_key, FALSE);

		// empty string means that the result was not serialised
		// which means cache manager does not have an entry for this cache_key
		if ($result === FALSE) return FALSE;

		$result = unserialize($result);
		return $result;

	}//end getCachedResultSet()


	/**
	* this function cache the result
	*
	* @return void
	* @access private
	*/
	private function cacheResultSet(Array $result, $cache_key='')
	{
		$cm	= $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$r	= $cm->saveToCache($this->id, $this->type(), $cache_key, serialize($result), FALSE);

	}//end cacheResultSet()


	/**
	* printBody() this function print the frontend of the map
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		$api_key			= $this->attr('api_key');
		$this->api_key		= $api_key;
		$lat				= $this->attr('default_latitude');
		$lng				= $this->attr('default_longitude');
		$zoom_level			= $this->attr('default_zoom_level');
		$map_type			= $this->attr('default_map_type');
		$use_google_marker	= $this->attr('use_google_marker_icon');
		$icon_url			= '';
		if (!$use_google_marker)
			$icon_url		= $this->attr('default_marker_icon');
		$icon_width			= $this->attr('default_icon_width');
		$icon_height		= $this->attr('default_icon_height');
		$default_icon_size	= $this->attr('default_icon_size');
		if ($default_icon_size) {
			$icon_height	= 25;
			$icon_width		= 25;
		}//end if

		$description		= 'defaultlocation';
		$keywords_list		= $this->attr('asset_types_keywords');
		$street_view		= $this->attr('enable_street_view');
		$create_location	= $this->attr('enable_create_location');

		$dynamic_lat		= $this->getDynamicVariableValue('latitude');
		$dynamic_lng		= $this->getDynamicVariableValue('longitude');
		$dynamic_desc		= $this->getDynamicVariableValue('description');
		$dynamic_zoom_level	= $this->getDynamicVariableValue('zoom_level');
		$dynamic_map_type	= $this->getDynamicVariableValue('map_type');

		if ($dynamic_lat && $dynamic_lng) {
			$lat			= $dynamic_lat;
			$lng			= $dynamic_lng;
		}//end if

		if (!empty($dynamic_zoom_level))	$zoom_level 	= $dynamic_zoom_level;
		if (!empty($dynamic_desc)) 			$description	= $dynamic_desc;
		if (!empty($dynamic_map_type)) 		$map_type		= $dynamic_map_type;

		$cache_key				= $this->getCacheKey();
		$cached_result			= $this->getCachedResultSet($cache_key);

		$bodycopy_popup_window	= $this->getBodycopy('Popup Window');
		$keywords				= $bodycopy_popup_window->getKeywords();

		$root_node_id			= $this->attr('root_assetid');

		$res					= $this->getAssetList();

        $exclude_assets = $this->attr('exclude_assets');

        // get children of exclude root nodes, add them to the exclude list
        $exclude_root_nodes = $this->attr('exclude_root_nodes');
        foreach($exclude_root_nodes as $key => $value) {
            $exclude_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($value);
            $exclude_assets += array_keys($exclude_children);
        }//end foreach

        if (!empty($exclude_assets)) {
            foreach ($exclude_assets as $index => $assetid) {
                if (isset($res[$assetid])) {
                    unset($res[$assetid]);
                }//end if
            }//end foreach
        }//end if


		$res = $this->convertProxyAssetTypes($res);
		$this->filterAssetList($res);

		$root_nodes				= $this->getRootNodes();

		$assets_parent			= Array();

		$toggle_root_nodes		= $this->attr('toggle_root_nodes');

		// Only query the tree when we are not using grouping, but toggling is turned on
		if ($toggle_root_nodes) {
			foreach ($root_nodes as $root_assetid) {
				$children = $this->classifyChildren($root_assetid, array_keys($res));
				if (!empty($children)) {
					foreach ($children as $child_info) {
						$assets_parent[$child_info['minorid']][]	= $root_assetid;
					}//end foreach
				}//end if
			}//end foreach
		}//end if

		$locations_info		= Array();
		$address_keyword	= FALSE;
		$listing_ldap_user	= FALSE;

		if (!empty($res)) {
			$node_info = Array();

			foreach ($res as $child_assetid => $asset_type)
			{
				$popup_html = '';
				$child_asset			= $GLOBALS['SQ_SYSTEM']->am->getAsset($child_assetid);
				$child_asset_type		= $child_asset->type();

				$child_address = NULL;
				if ($child_asset->type() == 'google_map_location') {
					$child_title		= $child_asset->attr('name');
					$child_latitude		= $child_asset->attr('latitude');
					$child_longitude	= $child_asset->attr('longitude');
					$child_icon_url		= $child_asset->attr('icon_url');
					$child_description	= $child_asset->attr('description');
				} else {
					$child_title		= $child_asset->getKeywordReplacement(str_replace('%', '', $keywords_list[$child_asset_type]['name_keyword']));
					if (is_array($child_title)) {
						$child_title	= implode(' ', $child_title);
					}//end if

					$child_latitude		= $child_asset->getKeywordReplacement(str_replace('%', '', $keywords_list[$child_asset_type]['latitude_keyword']));
					$child_longitude	= $child_asset->getKeywordReplacement(str_replace('%', '', $keywords_list[$child_asset_type]['longitude_keyword']));
					$child_icon_url		= $this->replaceKeywordCombination($keywords_list[$child_asset_type]['icon_url_keyword'], $child_asset);
					$child_description	= $child_asset->getKeywordReplacement(str_replace('%', '', $keywords_list[$child_asset_type]['description_keyword']));
					$address_keyword	= array_get_index($keywords_list[$child_asset_type], 'address_keyword', NULL);
					if ($address_keyword) {
						$child_address	= $this->replaceKeywordCombination($address_keyword, $child_asset);
					}//end if
				}//end if

				$loc_index	= $child_assetid;
				$ldap_user = ($child_asset->type() instanceof Ldap_User || $child_asset->type() instanceof Ldap_User_Group);

				// Override if ldap user
				if ($address_keyword && !is_int($child_assetid) && $ldap_user) {
					$loc_index	= $child_asset->getKeywordReplacement('asset_attribute_uid');
				}//end if

				$locations_info[$loc_index]['title']= $child_title;
				$locations_info[$loc_index]['lat']	= $child_latitude;
				$locations_info[$loc_index]['lng']	= $child_longitude;
				$locations_info[$loc_index]['icon']	= !empty($child_icon_url) ? $child_icon_url : $icon_url;
				$locations_info[$loc_index]['desc']	= $child_description;
				if ($address_keyword) {
					$locations_info[$loc_index]['addr']	= $child_address;

					// This is special case when we use LDAP, but what if we dont use LDAP
					$index	= $child_asset->id;
					$asset_types_list = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('ldap_user'));
					$asset_types_list[] = 'ldap_user';
					$listing_ldap_user	= in_array($child_asset_type, $asset_types_list);
					if ($listing_ldap_user) {
						$index	= $child_asset->getKeywordReplacement('asset_attribute_uid');
					}//end if
					$locations_info[$loc_index]['index']	= $index;

					// Now if we use address, we will check
					if (empty($child_latitude) || empty($child_longitude)) {
						// Check the cache
						if (!empty($cached_result) && isset($cached_result[$index])) {
							// If in cache
							if (isset($locations_info[$loc_index]['lat'])) {
								$locations_info[$loc_index]['lat']	= $cached_result[$index]['lat'];
							}//end if
							if (isset($locations_info[$loc_index]['lng'])) {
								$locations_info[$loc_index]['lng']	= $cached_result[$index]['lng'];
							}//end if
						} else {
							// We are calling the Geocoding service
							$result	= $this->getLatLngFromAddress($child_address);
							$cached_result[$index]	= $result;
							if ($result !== FALSE) {
								$locations_info[$loc_index]['lat']	= $cached_result[$index]['lat'];
								$locations_info[$loc_index]['lng']	= $cached_result[$index]['lng'];
							}//end if
							$this->cacheResultSet($cached_result, $cache_key);
						}//end else
					}//end if

				}//end if

				$replacements	= Array();
				foreach ($keywords as $word) {
					$asset_keyword	= $this->getMappedKeyword($child_asset_type, $word);
					$replacements[$word] = $child_asset->getKeywordReplacement($asset_keyword);
				}//end foreach

				$bodycopy_popup_window->setKeywordReplacements($replacements);
				ob_start();
					$bodycopy_popup_window->printBody();
				$popup_html = ob_get_contents();
				ob_end_clean();

				$locations_info[$loc_index]['html']	= $popup_html;

			}//end foreach

		}//end if

		?>
			<script src="<?php echo (current_protocol()); ?>://maps.googleapis.com/maps/api/js?key=<?php echo $api_key; ?>&amp;sensor=false" type="text/javascript"></script>
			<script type="text/javascript">
			//<![CDATA[
				<?php if (!empty($popup_html)) {
						$popup_html = preg_replace('/(\r\n|\r|\n)/s','', $popup_html);
				?>
				var popup_html		= '<?php echo preg_replace("/'/", "\'", $popup_html); ?>';
				<?php } ?>
				var centerLatitude	= <?php echo (float) $lat; ?>;
				var centerLongitude	= <?php echo (float) $lng; ?>;
				var desc			= '<?php echo $description; ?>';
				var zoomLevel		= <?php echo (int) $zoom_level; ?>;
				var map_type		= google.maps.MapTypeId.<?php echo $map_type; ?>;
				var icon_url		= '<?php echo $icon_url; ?>';
				var icon_width		= <?php echo $icon_width; ?>;
				var icon_height		= <?php echo $icon_height; ?>;

				// The MAP
				var map;
				var myPano;				// The street view object
				var overlay				= null;
				var geocoder			= null;
				var allMarkerList		= {};
				var allDisplayingMarker	= {};
				var markerTypes			= {};
				var markerRootNodes		= {};
				var addressList			= [];
				var cache_home_url		= '<?php echo $this->getUrl(); ?>';
				var	cache_key			= '<?php echo $cache_key; ?>';

				var current_marker		= null;
				var current_listener	= null;
				var current_listener_name = 'navigate_tool';


			<?php
				$geocoder	= $this->attr('enable_geocoder');
				if ($geocoder) {
				?>
					geocoder = new google.maps.Geocoder();
				<?php
				}//end if


			?>
			//]]>
			</script>
			<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/google_map/js/map_lib.js'?>"></script>
		

		<?php

		// Need to print here so we can have the "map" div, and add stuff to it.
		parent::printBody();

		if (!empty($locations_info)) {
			?>
				<script type="text/javascript">
				//<![CDATA[
					var lat;
					var lng;

					var newMarkers		= [];
					var newPolylines	= [];
					// Now we are listing new markers, but we have to store them in a central array.

			<?php


				$asset_types	= $this->attr('types');
				foreach ($asset_types as $type_code	=> $inherit) {
					?>
					markerTypes['<?php echo $type_code; ?>'] = { toggle:true };
					<?php
				}//end foreach

				if ($toggle_root_nodes) {
					foreach ($root_nodes as $root_assetid) {
						?>
						markerRootNodes['<?php echo $root_assetid; ?>']	= { toggle:true };
						<?php
					}//end foreach
				}//end if

				foreach ($locations_info as $child_assetid => $location_info) {

					if ( empty($location_info['lat']) || empty($location_info['lng'])) {
						// We can't map this address so just continue
						continue;
					} else {

						$popup_content	= isset($location_info['html']) && !empty($location_info['html']) ? $location_info['html'] : $location_info['desc'];
						$html = str_replace('', '', preg_replace("/'/", "\'", $popup_content));
						$html = preg_replace('/(\r\n|\r|\n)/s','<br />', $html);
						$title = preg_replace('/\s/', '_', $location_info['title']);
						$title = preg_replace('/(\'|\.)/', '_', $title);
						$location_lat	= (float) $location_info['lat'];
						$location_lng	= (float) $location_info['lng'];
						if (isset($cached_result[$child_assetid])) {
							$location_lat	= $cached_result[$child_assetid]['lat'];
							$location_lng	= $cached_result[$child_assetid]['lng'];
						}//end if

						?>
						var marker = addMarker('<?php echo $title ?>' ,<?php echo $location_lat; ?>, <?php echo $location_lng; ?>, '<?php echo $location_info['icon']; ?>', '<?php echo $html;?>', <?php echo $street_view ? 'true' : 'false'; ?>);
						<?php
							if ($toggle_root_nodes && !$listing_ldap_user) {
								if (isset($assets_parent[$child_assetid])) {
									//We can't have marker toggling while listing ldap users  (use Geocoder to find addresses)
									// Array structure does not allow so
							?>
									markerRootNodes['<?php echo $assets_parent[$child_assetid][0]; ?>']['<?php echo $child_assetid;?>'] = marker;
						<?php
								}//end if
							}//end if ?>

							allMarkerList['<?php echo $child_assetid; ?>'] = marker;
						<?php
							if (!$listing_ldap_user) {
								// We can't have marker toggling while listing ldap users (use Geocoder to find addresses),
								// Array structure does not allow so
						?>
								markerTypes['<?php echo $res[$child_assetid][0]['type_code']; ?>']['<?php echo $child_assetid;?>'] = marker;
					<?php
							}//end if
					}//end if

				}//end foreach
				if ($dynamic_lat && $dynamic_lng) {
					?>
					var marker = addMarker('center', <?php echo (float) $dynamic_lat; ?>, <?php echo (float) $dynamic_lng; ?>);
					<?php
				}//end if
			?>
			//]]>
				</script>
			<?php
			if ($create_location) {
				$this->getGoogleMapLocationAssetAttributeId();
			}//end if

			if ($street_view) {
				$this->enableStreetView();
			}//end if
		}//end if


	}//end printBody();


	/**
	* this function replace all the keyword inside for you
	*
	* @return string
	* @access public
	*/
	function replaceKeywordCombination($combination, $asset)
	{
		$multiple_keywords	= extract_keywords($combination);
		$replacements	= Array();
		foreach ($multiple_keywords as $keyword) {
			$replacements[$keyword]		= $asset->getKeywordReplacement($keyword);
		}//end foreach

		replace_keywords($combination, $replacements);

		return $combination;

	}//end replaceKeywordCombination();


	/**
	* Return the bodycopy linked to this page
	*
	* @param string	$name	The name of the bodycopy
	*
	* @return object
	* @access public
	*/
	function getBodycopy($name)
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name, 'major');
		if (empty($link)) return $null;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* Get a value of a dynamic variable
	*
	* @param string $variable_name  Name of variable
	*
	* @return string
	* @access public
	*/
	public function getDynamicVariableValue($variable_name)
	{
		$dynamic_vars = $this->attr('dynamic_vars');
		if (!isset($dynamic_vars[$variable_name])) return '';

		$parameter_map = $this->getAttribute('parameter_map');
		$value = $parameter_map->getParameterValue($variable_name);

		if (empty($value)) {
			$value = array_get_index($dynamic_vars, $variable_name, '');
		}//end if

		return $value;

	}//end getDynamicVariableValue()


	/**
	* return the keywords for content type
	*
	* @return array
	* @access public
	*/
	function _getContentsKeywords()
	{

		$keywords							= parent::_getContentsKeywords();
		$keywords['location_name']			= translate('google_map_name_keyword');
		$keywords['location_latitude']		= translate('google_map_latitude_keyword');
		$keywords['location_longitude']		= translate('google_map_longitude_keyword');
		$keywords['location_description']	= translate('google_map_description_keyword');
		$keywords['location_icon_url']		= translate('google_map_icon_url_keyword');
		$keywords['google_map']				= translate('google_map_initialisation');

		$keywords['street_view']			= translate('google_map_street_view');
		$keywords['toggle_street_view_window']	= translate('google_map_toggle_street_view_window');
		$keywords['toggle_new_address_list']= translate('google_map_toggle_new_address_list');
		$keywords['toggle_tool_bar']		= translate('google_map_toggle_tool_bar');
		$keywords['tool_bar']				= translate('google_map_tool_bar');
		$keywords['toggle_address_finder']	= translate('google_map_toggle_address_finder');

		$keywords['toggle_asset_types']		= translate('google_map_toggle_asset_types');
		$keywords['toggle_root_nodes']		= translate('google_map_toggle_root_nodes');

		$keywords['address_finder']			= translate('google_map_address_finder');
		$keywords['new_address_list']		= translate('google_map_new_address_list');
		$keywords['asset_builder']			= translate('google_map_asset_builder');

		$keywords['get_closest_location']	= translate('google_map_get_closest_location');
		$keywords['clear_new_markers']		= translate('google_map_clear_new_markers');
		$keywords['map_type_menu']			= translate('google_map_map_type_menu');

		return $keywords;

	}//end _getContentsKeywords()


	/**
	* function getKeywordReplacements
	*
	* @param string $keywords
	*
	* @return string
	* @access public
	*/
	public function getKeywordReplacement($keyword)
	{
		if ($keyword == 'google_map') {
			$map_code	= $this->attr('map_code');
			return $map_code;
		} else if ($keyword == 'street_view') {
			$street_view_code	= $this->attr('street_view_code');
			return $street_view_code;
		} else if ($keyword == 'address_finder') {
			$street_view_enabled =  $this->attr('enable_street_view');
			$street_view_enabled = $street_view_enabled ? 'true' : 'false';
			return  '<div id="address_finder" class="address_finder_div">
						<form action="#" onsubmit="findLocationFromAddress(this.address.value, null, null, '.$street_view_enabled.'); return false">
						  <p>
							<input type="text" style="width:400px;" name="address" value="" />
							<input type="submit" value="Find Location" />
						  </p>
						</form>
					</div>';
		} else if ($keyword == 'new_address_list') {
			return '<div id="new_address_list" class="new_address_list">
					</div>';
		} else if ($keyword == 'map_type_menu') {
			$map_menu = '<ul id="map_menu" class="map_type_menu" style="list-style:none;">
					<li><button onClick="setMapType(google.maps.MapTypeId.ROADMAP);">ROADMAP</button></li>
					<li><button onClick="setMapType(google.maps.MapTypeId.SATELLITE);">SATELLITE</button></li>
					<li><button onClick="setMapType(google.maps.MapTypeId.TERRAIN);">TERRAIN</button></li>
					<li><button onClick="setMapType(google.maps.MapTypeId.HYBRID);">HYBRID</button></li>';
			$map_menu .= '</ul>';
			return $map_menu;
		} else if ($keyword == 'clear_new_markers') {
			return '<a href="javascript:clearNewMarker(newMarkers);clearNewMarker(newPolylines);">Clear New Markers</a>';
		} else if ($keyword == 'get_closest_location') {
			return '<a href="javascript:getClosestLocation();">Get Closest Location</a>';
		} else if ($keyword == 'asset_builder') {
			return '<div id="asset_builder" class="asset_builder_div">NEST YOUR ASSET BUILDER HERE</div>';
		} else if ($keyword == 'toggle_street_view_window') {
			return '<a href="javascript:toggleDiv(\'street_view\');" id="toggle_street_view">Toggle Street View</a>';
		} else if ($keyword == 'toggle_new_address_list') {
			return '<a href="javascript:toggleDiv(\'new_address_list\');" id="toggle_new_address_list">Toggle New Address List</a>';
		} else if ($keyword == 'toggle_address_finder') {
			return '<a href="javascript:toggleDiv(\'address_finder\');" id="toggle_address_finder">Toggle Address Finder</a>';
		} else if ($keyword == 'toggle_tool_bar') {
			return '<a href="javascript:toggleDiv(\'tool_bar_list\');" id="toggle_tool_bar">Toggle Tool Bar</a>';
		} else if ($keyword == 'tool_bar') {
			return '
			<ul id="tool_bar_list" class="tool_bar" style="list-style:none;">
				<li><a href="javascript:addListeners(\'navigate\', this.id);" style="color:red;" title="Navigate Map" >
					<img src="'.sq_web_path('data').'/asset_types/google_map/images/map.png'.'" style="border-color:red;border:2px solid;" id="navigate_tool" name="navigate_tool"/>
				</a>
				</li>
				<li>
				<a href="javascript:addListeners(\'add_marker\', this.id);" style="color:white;" title="Add Marker">
					<img src="'.sq_web_path('data').'/asset_types/google_map/images/arrowdown.png'.'" style="border-color:white;border:2px solid;" id="add_marker_tool" name="add_marker_tool"/>
				</a>
				</li>
				<li>
				<a href="javascript:addListeners(\'street_view\', this.id);" style="color:white;" title="Street View">
					<img src="'.sq_web_path('data').'/asset_types/google_map/images/binocular.png'.'" style="border-color:white;border:2px solid;" id="street_view_tool" name="street_view_tool"/>
					</a>
				</li>
			</ul>';
		} else if ($keyword == 'toggle_asset_types') {
				$asset_types	= $this->attr('types');
				$result			= '';
				foreach ($asset_types as $type	=> $inherit) {
					$result		.= '<a href="javascript:toggleDisplay(markerTypes, \''.$type.'\')">Toggle '.ucwords(str_replace('_', ' ', $type)).'</a><br />';
				}//end foreach
				return $result;
		} else if ($keyword == 'toggle_root_nodes') {
				$root_nodes		= $this->getRootNodes();
				$result			= '';
				foreach ($root_nodes	as $root_assetid) {
					$root_asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($root_assetid);
					$result		.= '<a href="javascript:toggleDisplay(markerRootNodes, \''.$root_assetid.'\')">Toggle '.$root_asset->attr('name').'</a><br />';
				}//end foreach
				return $result;
		} else {
			parent::getKeywordReplacement($keyword);
		}//end if

	}//end getKeywordReplacement()


	/**
	* this function return the mapped keyword for each asset type
	*
	* @param string $type_code
	* @param string $keyword
	*
	* @return mixed string|boolean
	* @access public
	*/
	public function getMappedKeyword($type_code, $keyword)
	{
		$keywords_list	= $this->attr('asset_types_keywords');
		if (isset($keywords_list[$type_code])) {
			switch ($keyword) {
				case 'location_name':
					return str_replace('%', '', $keywords_list[$type_code]['name_keyword']);
					break;
				case 'location_latitude':
					return str_replace('%', '', $keywords_list[$type_code]['latitude_keyword']);
					break;
				case 'location_longitude':
					return str_replace('%', '', $keywords_list[$type_code]['longitude_keyword']);
					break;
				case 'location_description':
					return str_replace('%', '', $keywords_list[$type_code]['description_keyword']);
					break;
				case 'location_icon_url':
					return str_replace('%', '', $keywords_list[$type_code]['icon_url_keyword']);
					break;
				default:
					return $keyword;
					break;
			}//end switch
		} else if ($type_code == 'google_map_location') {
			switch ($keyword) {
				case 'location_name':
					return 'asset_attribute_name';
					break;
				case 'location_latitude':
					return 'asset_attribute_latitude';
					break;
				case 'location_longitude':
					return 'asset_attribute_longitude';
					break;
				case 'location_description':
					return 'asset_attribute_description';
					break;
				case 'location_icon_url':
					return 'asset_attribute_icon_url';
					break;
				default:
					return $keyword;
					break;
			}//end switch
		} else {
			return FALSE;
		}//end else
	}//end getMappedKeyword()


	/**
	* Get the default content for a bodycopy
	*
	* @param string	$bodycopy_code	The code name for the bodycopy
	*
	* @return string
	* @access private
	*/
	function _getDefaultBodycopyContent($bodycopy_code)
	{
		switch ($bodycopy_code) {
			case 'page_contents':
				return '%google_map%
						%asset_listing%';
			case 'default_format':
				return '<a href="javascript:getInfo(\'%asset_assetid%\');">%asset_name%</a>';
			case 'popup_window':
				return '%location_name% - %location_description%';
			case 'input_form':
				ob_start();?>
		<fieldset style="width:200px;color:black;background:#DDDDDD;">
		<legend style="font-weight:bold;">New Marker</legend>
		<label for="found">Location</label>
		<input type="text" id="found" style="width:100%;"/>
		<input type="hidden" id="depth" value="" style="width:100%;"/>
		<label for="depth">Who drowned here?</label>
		<input type="text" id="icon" style="width:100%;"/>
		<input type="hidden" id="longitude" value="' , lng , '"/>
		<input type="hidden" id="latitude" value="' , lat , '"/><br />
		<input type="submit" value="Add Location" style="margin-left:20px;"/>
		</fieldset><?php
				$content = ob_get_contents();
				ob_end_clean();

				return $content;
		}//end switch

		parent::_getDefaultBodycopyContent($bodycopy_code);

	}//end _getDefaultBodycopyContent()


	/**
	* find the children of each rootnode
	*
	* @param string $root_id
	* @param string $assetids
	*
	* @return array
	* @access public
	*/
	public function classifyChildren($root_id, $assetids)
	{
		$root_id = MatrixDAL::quote($root_id);
		foreach ($assetids as $index => $id) {
		    $assetids[$index] = MatrixDAL::quote($id);
		}
		$assetids = implode(', ', $assetids);
		$sql		= 'SELECT minorid FROM sq_ast_lnk WHERE majorid = '.$root_id.' AND minorid IN ('.$assetids.')';

		$query		= MatrixDAL::preparePdoQuery($sql);
		$children	= MatrixDAL::executePdoAssoc($query);

		if (empty($children)) {
			$root_asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($root_id);
			if (implements_interface($root_asset, 'bridge')) {
				$children	= $root_asset->getChildren($root_id);
				$children_ids	= Array();
				foreach ($children as $assetid => $type_code_info) {
					$children_ids[]	= Array ('minorid'	=> $assetid);
				}//end if
				$children	= $children_ids;
			}//end if
		}//end if

		return $children;

	}//end find_children()


	/**
	* This function add the google map location attribute ID to the JS to be used to create new location
	*
	* @return void
	* @access public
	*/
	public function getGoogleMapLocationAssetAttributeId()
	{
		$sql		= 'select name, attrid from sq_ast_attr where type_code = \'google_map_location\' and name IN (\'name\', \'description\', \'latitude\', \'longitude\')';
		$query		= MatrixDAL::preparePdoQuery($sql);
		$result		= MatrixDAL::executePdoGroupedAssoc($query);

		if (!empty($result)) {
			?>
				<script type="text/javascript">
				//<![CDATA[
					var FormEle	= {};
					<?php
						foreach ($result as $attr_name	=> $attr_id_info) {
							// Giving a shot here, how would I know whether it is google_map_location_"0" or "1"? It's better be the asset builder only built for the map
							?>
							FormEle.<?php echo $attr_name; ?> = '<?php echo 'google_map_location_0_'.$attr_id_info[0]['attrid']; ?>';
							<?php
						}//end foreach
					?>
				//]]>
				</script>
			<?php

		}//end if

	}//end getGoogleMapLocationAttributeId()


	/**
	* This function add the Street View object to the map
	*
	* @return void
	* @access public
	*/
	public function enableStreetView()
	{
		?>
		<script type="text/javascript">
		//<![CDATA[
			var center = new  google.maps.LatLng(centerLatitude, centerLongitude);
			panoramaOptions = { position:center };
			myPano = new google.maps.StreetViewPanorama(document.getElementById("street_view"), panoramaOptions);
			map.setStreetView(myPano);
		//]]>
		</script>
		<?php
	}//end enableStreetView()


	/**
	* This function get Lat Lng from Google via XML then cache it
	*
	* @return array
	* @access public
	*/
	public function getLatLngFromAddress($address)
	{
		if (empty($this->api_key)) {
			$this->api_key	= $this->attr('api_key');
		}//end if
		$base_url		= "http://maps.googleapis.com/maps/api/geocode/xml";
		$request_url	= $base_url."?address=" . urlencode($address).'&sensor=false';

		$geocode_pending = TRUE;

		$lat	= NULL;
		$lng	= NULL;
		$steps  = 0;
		while ($geocode_pending) {
			$delay	= 0;

			$options = array(
					    'RETURNTRANSFER' => 1,
					   );

			$details  = fetch_url($request_url, $options);
			$maps_xml = $details['response'];

			$xml	= simplexml_load_string($maps_xml) or die("url not loading");
			
			$status = (string) $xml->status;

			if ($status == "OK") {
				$geocode_pending = false;
				$result = $xml->result;
				
				$coordinates = $result[0]->geometry->location;
				
				$lat =  (string) $coordinates->lat;
				$lng =  (string) $coordinates->lng;
				
			} else {
				// GeoCoder returns this status on 2 conditions: 1. sending addresses too fast 2. exceeding 15000 addresses per 24Hr
				// slow down, and bail if steps is too great (to prevent an infinite loop, by exceeding 15000 addresses) 
				if ($steps < 100) {
					$delay += 100000;
					$steps++;
				} else {
					$geocode_pending = FALSE;
				}//end if
			}
			usleep($delay);
		}//end while

		if (!empty($lat) && !empty($lng)) {
			$result	= Array (
					'lat'	=> $lat,
					'lng'	=> $lng,
				   );
			return $result;
		} else {
			return FALSE;
		}//end else

	}//end getLatLngFromAddress()



}//end class
?>
