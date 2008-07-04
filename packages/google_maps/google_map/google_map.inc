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
* $Id: google_map.inc,v 1.1.1.1 2008/07/04 05:19:54 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';

/**
* Google Map Location
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1.1.1 $
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

        $bodycopies = Array ('Map Content', 'Popup Window');
        foreach ($bodycopies as $bodycopy) {
            $this->createBodycopy($bodycopy);
        }//end foreach

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

		$content = '';
		$func_name = 'get'.str_replace(' ','', $name).'BodycopyContent';
		$content = $this->$func_name();

		$bc_data = Array (
					'attributes'	=> Array (
										'content_type'	=> $content_type,
									   ),
					'content'		=> $content,
				   );

		$copy_link	= Array (
						'asset'			=> &$this,
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
    * printBody() this function print the frontend of the map
    *
    * @return void
    * @access public
    */
    function printBody()
    {
    	$api_key			= $this->attr('api_key');
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
		$custom_html_popup	= $this->attr('use_custom_popup_html');
		$enable_marking		= $this->attr('enable_adding_marker');
		$description		= 'defaultlocation';

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

    	if ($custom_html_popup) {	// If the customize popup window is enabled.
	    	$bodycopy_popup_window	=& $this->getBodycopy('Popup Window');
	        $keywords = $bodycopy_popup_window->getKeywords();
		}//end if

		if ($enable_marking) {
			$bodycopy_input_form	=& $this->getBodycopy('Input Form');
	        ob_start();
	        	$bodycopy_input_form->printBody();
	        $input_form_html = ob_get_contents();
	        ob_end_clean();
		}//end if

		$root_node_id	= $this->attr('root_assetid');
		$listing_root_nodes		= ($this->attr('listing_root_nodes'));

		$res = $this->getAssetList();

		$enable_listing	= $this->attr('enable_listing');
		$locations_info	= Array();


		if (!empty($res) && $enable_listing) {
			$node_info = Array();

			foreach ($res as $child_assetid => $asset_type)
			{
				$popup_html = '';
				$child_asset							= $GLOBALS['SQ_SYSTEM']->am->getAsset($child_assetid);

				if ($child_asset->type() == 'google_map_location') {
					$child_title		= $child_asset->attr('name');
					$child_latitude		= $child_asset->attr('latitude');
					$child_longitude	= $child_asset->attr('longitude');
					$child_icon_url		= $child_asset->attr('icon_url');
					$child_description	= $child_asset->attr('description');
				} else {
					$child_latitude		= $child_asset->getKeywordReplacement($node_info['latitude_keyword']);
					$child_longitude	= $child_asset->getKeywordReplacement($node_info['longitude_keyword']);
					$child_icon_url		= $child_asset->getKeywordReplacement($node_info['icon_url_keyword']);
					$child_description	= $child_asset->getKeywordReplacement($node_info['description_keyword']);
				}//end if

				$locations_info[$child_assetid]['title']= $child_title;
				$locations_info[$child_assetid]['lat']	= $child_latitude;
				$locations_info[$child_assetid]['lng']	= $child_longitude;
				$locations_info[$child_assetid]['icon']	= !empty($child_icon_url) ? $child_icon_url : $default_icon;
				$locations_info[$child_assetid]['desc']	= $child_description;

				if ($custom_html_popup) {
					$shadow_asset = FALSE;
					if (strpos($child_assetid, ':') !== FALSE) {
						continue;
					}//end if

	        		foreach ($keywords as $word) {
	        			$replacements[$word] = $child_asset->getKeywordReplacement($word);
					}//end foreach

					$bodycopy_popup_window->setKeywordReplacements($replacements);
			        ob_start();
			        	$bodycopy_popup_window->printBody();
			        $popup_html = ob_get_contents();
			        ob_end_clean();

					$locations_info[$child_assetid]['html']	= $popup_html;
				}//end if
			}//end foreach

		}//end if

		?>
			<script src="http://maps.google.com/maps?file=api&amp;v=2&amp;key=<?php echo $api_key; ?>" type="text/javascript"></script>
			<script type="text/javascript">
				<?php if (!empty($popup_html)) {
						$popup_html = preg_replace('/(\r\n|\r|\n)/s','', $popup_html);
				?>
				var popup_html		= '<?php echo $popup_html; ?>';
				<?php } ?>
				var centerLatitude	= <?php echo (float) $lat; ?>;
				var centerLongitude	= <?php echo (float) $lng; ?>;
				var desc			= '<?php echo $description; ?>';
				var zoomLevel		= <?php echo (int) $zoom_level; ?>;
				var map_type		= G_<?php echo strtoupper($map_type);?>_MAP;
				var icon_url		= '<?php echo $icon_url; ?>';
				var icon_width		= <?php echo $icon_width; ?>;
				var icon_height		= <?php echo $icon_height; ?>;

				// The MAP
				var map;
				var markerList = new Array();

			</script>
			<script language="JavaScript" src="<?php echo sq_web_path('data').'/asset_types/google_map/js/map_lib.js'?>"></script>
			<link href="<?php echo sq_web_path('data').'/asset_types/google_map/css/style.css';?>" rel="stylesheet" type="text/css" />

		<?php

		// Need to print here so we can have the "map" div, and add stuff to it.
		parent::printBody();

		if (!empty($locations_info)) {
			?>
				<script type="text/javascript">
				<?php
				if ($enable_marking) {
					$input_form_html = preg_replace('/(\r\n|\r|\n)/s','', $input_form_html);
					?>
					var lat;
					var lng;
					addInputFormListener('<?php echo $input_form_html; ?>');

			<?php
				}//end if
				foreach ($locations_info as $location_info) {
					$popup_content	= isset($location_info['html']) && !empty($location_info['html']) ? $location_info['html'] : $location_info['desc'];
					$html = str_replace('', '', addslashes($popup_content));
					$html = preg_replace('/(\r\n|\r|\n)/s','<br />', $html);
					$title = preg_replace('/\s/', '_', $location_info['title']);
					$title = preg_replace('/(\'|\.)/', '_', $title);
//preg_replace('/\s\s+/', ' ', $str);


					log_dump("TITLE:" . $title);
					?>
						addMarker('<?php echo $title ?>' ,<?php echo (float) $location_info['lat']; ?>, <?php echo (float) $location_info['lng']; ?>, '<?php echo $location_info['icon']; ?>', '<?php echo $html;?>');
					<?php
				}//end foreach
				if ($dynamic_lat && $dynamic_lng) {
					?>
					addMarker('center', <?php echo (float) $dynamic_lat; ?>, <?php echo (float) $dynamic_lng; ?>);
					<?php
				}//end if
			?>
				console.info(markerList);
				</script>
			<?php
		}//end if

		$this->addControls();

	}//end printBody();


	function addControls()
	{
		// Adding control and setting map types
		$map_control		= $this->attr('map_control');
		$map_type_control	= (boolean) $this->attr('map_type_control');
		$scale_control		= (boolean) $this->attr('scale_control');

		?>
			<script type="text/javascript">
				<?php
					if ($map_control == 'large') {?>
						map.addControl(new GLargeMapControl());
				<?php
					} else if ($map_control == 'small') {?>
						map.addControl(new GSmallMapControl());
				<?php
					} else if ($map_control == 'zoom') {?>
						map.addControl(new GSmallZoomControl());
				<?php
					}//end else if
					if ($scale_control) {?>
						map.addControl(new GScaleControl());
				<?php
					}
					if ($map_type_control) {
				?>
						map.addControl(new GMapTypeControl());
				<?php
					}
				?>

			</script>
		<?php

	}//end addControls()


	/**
	* Return the bodycopy linked to this page
	*
	* @return object
	* @access public
	*/
	function &getBodycopy($name)
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name, 'major');
		if (empty($link)) return $null;
		$bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* this function hold the html content for the map
	*
	* @return string
	* @access public
	*/
	function getMapContentBodycopyContent()
	{
		ob_start();?>
<div id="map" style="width:100%;height:100%;border:solid 1px;"></div>
<div id="toolbar">
	<ul id="options">
		<li><button style="background:#0099FF;color:white;font-weight:bold;font-size:10px;font-family:tahoma;" onClick="setMapType(G_NORMAL_MAP);">NORMAL</button></li>
		<li><button style="background:#0099FF;color:white;font-weight:bold;font-size:10px;font-family:tahoma;" onClick="setMapType(G_SATELLITE_MAP);">SATTELITE</button></li>
		<li><button style="background:#0099FF;color:white;font-weight:bold;font-size:10px;font-family:tahoma;" onClick="setMapType(G_HYBRID_MAP);">HYBRID</button></li>
		<li><button style="background:#0099FF;color:white;font-weight:bold;font-size:10px;font-family:tahoma;" onClick="setMapType(G_SKY_VISIBLE_MAP);">SKY VISIBLE</button></li>
		<li><button style="background:#0099FF;color:white;font-weight:bold;font-size:10px;font-family:tahoma;" onClick="setMapType(G_MARS_VISIBLE_MAP);">MARS VISIBLE MAP</button></li>
	</ul>
</div>
<div id="poweredBar">
	<span style="margin-top: 10px;padding-top:10px;">Powered By </span>
	<span style="font-weight:bold;">MYSOURCE MATRIX</span>
</div>
<script type="text/javascript">
	init();
	window.onunload = GUnload;
</script>
        <?php
        $content = ob_get_contents();
        ob_end_clean();

        return $content;

	}//end getMapContentBodycopyContent()


	/**
	* this function hold the html for the popup window
	*
	* @return string
	* @access public
	*/
	function getPopupWindowBodycopyContent()
	{
		ob_start();?>
<div style="background:#DDDDDD;width:100%;height:100%;text-align:center;vertical-align:top;">
    <span style="font-size:9px;font-family:tahoma;">
    	%asset_attribute_description%
    </span>
</div>
		<?php
		$content = ob_get_contents();
		ob_end_clean();
		return $content;

	}//end getPopupWindowBodycopyContent()


	function getInputFormBodycopyContent()
	{
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
	}//end getInputFormBodycopyContent()

	/**
	* printHead function
	*
	* @return void
	* @access public
	*/
	function printHead()
	{
		parent::printHead();

	}//end printHead()


    /**
    * Get a value of a dynamic variable
    *
    * @param string $variable_name  Name of variable
    *
    * @return string
    * @access public
    */
    function getDynamicVariableValue($variable_name)
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


}//end class
?>
