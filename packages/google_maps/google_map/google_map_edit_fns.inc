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
* $Id: google_map_edit_fns.inc,v 1.8 2012/10/22 23:33:54 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';

/**
* Google_Map_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage google_maps
*/
class Google_Map_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;
	}//end constructor


    /**
    * paint data mapping
    *
    * @param object $asset
    * @param object $o
    * @param string $prefix
    */
    function paintDataMapping(&$asset, &$o, $prefix)
    {
        $write_access = $asset->writeAccess('attributes');
        $var_list = $asset->attr('dynamic_vars');

        $o->openSection('Data Mappings');
        $o->openRaw();
        $map = $asset->getAttribute('parameter_map');

        $parameter_names = $map->getParameters();

        foreach ($var_list as $var_name => $default) {
            $map->setParameter($var_name, 'Variable: '.$var_name);
        }

        $map->paint($prefix.'_parameter_map', !$write_access);
        $o->closeRaw();

        return $write_access;

    }//end paintDataMapping()


    /**
    * process data mapping
    *
    * @param object $asset
    * @param object $o
    * @param string $prefix
    */
    function processDataMapping(&$asset, &$o, $prefix)
    {
        $write_access = $asset->writeAccess('attributes');
        if (!$write_access) return FALSE;
        $var_name_pattern = '/^[a-zA-Z_\-0-9\.]+$/';
        $var_list = ($asset->attr('dynamic_vars'));

        $map = $asset->getAttribute('parameter_map');

        $map->process($prefix.'_parameter_map');

        $asset->setAttrValue('parameter_map', unserialize($map->value));

        return $write_access;

    }//end processDataMapping()


    /**
    * Paint the interface for creating tables in the bodycopy
    *
    * @param object &$asset the asset whose interface we are painting
    * @param object &$o     the Backend Outputter object
    * @param string $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    function redirectToBodycopy(&$asset, &$o, $prefix)
    {
        // get the bodycopy attached to us
        $bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', 'page_contents', '1', '1');
        $bodycopy = NULL;
        if (!empty($bodycopy_link)) {
            $bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');
        }
        if (is_null($bodycopy)) {
            trigger_localised_error('CORE0016', sprintf(translate('Unable to locate the bodycopy for standard page "%s"'), $asset->name), E_USER_ERROR);
            return FALSE;
        }

        // redirect to the bodycopy's edit contents screen
        $o->setRedirect($bodycopy->getBackendHref('contents'));

        // fudge the navigation history a little so when they click on the link to ger back to the
        // page it doesnt take them to the edit contents screen and then redirect them again
        $_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

        return FALSE;

    }//end redirectToBodycopy()


    /**
    * paint default icon size
    *
    * @return boolean
    * @access public
    */
    function paintDefaultIconSize(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');

		$default_icon_size	= $asset->attr('default_icon_size');
		$default_width		= $asset->attr('default_icon_width');
		$default_height		= $asset->attr('default_icon_height');

		$o->openField('Default Icon Size');
				if ($wa) {
					check_box($prefix.'_use_default', '1', $default_icon_size, '
						if (this.checked==1) {
							document.getElementById("'.$prefix.'_default_icon_width").disabled=true;
							document.getElementById("'.$prefix.'_default_icon_height").disabled=true;
						} else {
							document.getElementById("'.$prefix.'_default_icon_width").disabled=false;
							document.getElementById("'.$prefix.'_default_icon_height").disabled=false;
						}//end else
							');
					echo ' Use Default Size<br /><br />';
					$extra = $default_icon_size ? 'disabled=true' : '';
					text_box($prefix.'_default_icon_width', $default_width, 5, '', FALSE, $extra);echo ' px';
					$o->note('Icon Width');
					text_box($prefix.'_default_icon_height', $default_height, 5, '', FALSE, $extra);echo ' px';
					$o->note('Icon Height');
				} else {
					echo $default_width.' px';
					$o->note('Icon Width');
					echo $default_height.' px';
					$o->note('Icon Height');
				}//end else

		$o->closeField();

		return $wa;

    }//end paintDefaultIconSize()


    /**
    * process default icon size
    *
    * @return boolean
    * @access public
    */
    function processDefaultIconSize(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');
		$default_width	= $asset->attr('default_icon_width');
		$default_height	= $asset->attr('default_icon_height');
		$default_size	= $asset->attr('default_icon_size');

		$attr_change = FALSE;
		$new_default_size	= isset($_POST[$prefix.'_use_default']) ? $_POST[$prefix.'_use_default'] : FALSE;
		if (!is_null($new_default_size) && $new_default_size != $default_size) {
			$attr_change = TRUE;
			$asset->setAttrValue('default_icon_size', $new_default_size);
		}

		if ($new_default_size != 1) {
			$new_width	= isset($_POST[$prefix.'_default_icon_width']) ? $_POST[$prefix.'_default_icon_width'] : '';
			$new_height	= isset($_POST[$prefix.'_default_icon_height']) ? $_POST[$prefix.'_default_icon_height'] : '';
			if (!empty($new_width) && $new_width != $default_width) {
				$asset->setAttrValue('default_icon_width', $new_width);
				$attr_change = TRUE;
			}//end if
			if (!empty($new_height) && $new_height != $default_height) {
				$asset->setAttrValue('default_icon_height', $new_height);
				$attr_change = TRUE;
			}//end if
		}//end if

		if ($attr_change) $asset->saveAttributes();

		return FALSE;

    }//end processDefaultIconSize()


    /**
    * paint default lat lng setting
    *
    * @return boolean
    * @access public
    */
    function paintDefaultLatLng(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');

		$api_key			= $asset->attr('api_key');
		$default_latitude	= $asset->attr('default_latitude');
		$default_longitude	= $asset->attr('default_longitude');
		$default_zoom_level	= $asset->attr('default_zoom_level');

		$o->openField('Default Map Center');
				if ($wa) {
					text_box($prefix.'_default_latitude', $default_latitude, 20, '', FALSE, 'onFocus="this.style.color = \'black\';"');
					if (!empty($api_key)) {?>
					&nbsp;
					<a href="javascript:switch_display();" style="text-decoration:none;border:none;">
						<img src="<?php echo sq_web_path('data').'/asset_types/google_map/images/globe.png'?>" border="0" title="Pick From Map" style="position:relative;top:5px;" />
					</a>
					<?php }
					$o->note('The default Latitude of the map');
					text_box($prefix.'_default_longitude', $default_longitude, 20, '', FALSE, 'onFocus="this.style.color = \'black\';"');
					if (!empty($api_key)) {?>
					&nbsp;
					<a href="javascript:switch_display();">
						<img src="<?php echo sq_web_path('data').'/asset_types/google_map/images/globe.png'?>" border="0" title="Pick From Map" style="position:relative;top:5px;" />
					</a>
					<?php }
					$o->note('The default Longitude of the map');
					text_box($prefix.'_default_zoom_level', $default_zoom_level, 20, '', FALSE, 'onFocus="this.style.color = \'black\';"');
					if (!empty($api_key)) {?>
					&nbsp;
					<a href="javascript:switch_display();">
						<img src="<?php echo sq_web_path('data').'/asset_types/google_map/images/globe.png'?>" border="0" title="Pick From Map" style="position:relative;top:5px;" />
					</a>
					<?php }
					$o->note('The default Zoom Level of the map');

				} else {
					echo $default_latitude;
					$o->note('The default Latitude of the map');
					echo $default_longitude;
					$o->note('The default Longitude of the map');
					echo $default_zoom_level;
					$o->note('The default Zoom Level of the map');
				}//end else

		$o->closeField();

		if (!empty($api_key) && $wa) {
			$o->openRaw();
					?>
						<div id="map" style="width:100%;height:250px;border:1px solid;display:none;">
						</div>
						<br /><br />
						<script src="<?php echo (current_protocol()); ?>://maps.googleapis.com/maps/api/js?key=<?php echo $api_key; ?>&amp;sensor=false" type="text/javascript"></script>
						<script type="text/javascript">
						var loaded = false;
						function init_picker() {
							var map;
							 var mapOptions = {
							 center: new google.maps.LatLng(<?php echo $default_latitude; ?>, <?php echo $default_longitude; ?>),
							 zoom: <?php echo $default_zoom_level; ?>,
							 mapTypeId: google.maps.MapTypeId.ROADMAP
							};
							map = new google.maps.Map(document.getElementById("map"), mapOptions);
							google.maps.event.addListener(map, 'click', function(event) {
						        var lng  = event.latLng.lng();
						        var lat  = event.latLng.lat()
						        document.getElementById('<?php echo $prefix; ?>_default_latitude').style.color = 'red';
								document.getElementById('<?php echo $prefix; ?>_default_latitude').value = lat;

								document.getElementById('<?php echo $prefix; ?>_default_longitude').style.color = 'red';
								document.getElementById('<?php echo $prefix; ?>_default_longitude').value = lng;
							 }//end function
							)
							google.maps.event.addListener(map, 'zoom_changed', function() {
								var zoomLevel = map.getZoom();
								document.getElementById('<?php echo $prefix; ?>_default_zoom_level').style.color = 'red';
								document.getElementById('<?php echo $prefix; ?>_default_zoom_level').value = zoomLevel;
							    }//end function
							)//end addListener
						}//end init_picker()

						function switch_display()
						{
							var new_display_val = 'block';
							var display_val = document.getElementById('map').style.display;
							if (display_val != 'none') {
								new_display_val = 'none';
							}//end if
							document.getElementById('map').style.display = new_display_val;
							if (loaded == false) {
								init_picker();
								loaded = true;
							}//end if
						}//end switch_display()
	    				</script>

					<?php
			$o->closeRaw();
		}//end if
    }//end paintDefaultLatLng()


    /**
    * process default lat lng setting
    *
    * @return boolean
    * @access public
    */
    function processDefaultLatLng(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');
		$default_latitude	= $asset->attr('default_latitude');
		$default_longitude	= $asset->attr('default_longitude');
		$default_zoom_level	= $asset->attr('default_zoom_level');

		$attr_change = FALSE;
		$new_default_latitude	= isset($_POST[$prefix.'_default_latitude']) ? $_POST[$prefix.'_default_latitude'] : '';
		if (!is_null($new_default_latitude) && $new_default_latitude != $default_latitude) {
			$attr_change = TRUE;
			$asset->setAttrValue('default_latitude', $new_default_latitude);
		}//end if

    	$new_default_longitude	= isset($_POST[$prefix.'_default_longitude']) ? $_POST[$prefix.'_default_longitude'] : '';
		if (!is_null($new_default_longitude) && $new_default_longitude != $default_longitude) {
			$attr_change = TRUE;
			$asset->setAttrValue('default_longitude', $new_default_longitude);
		}//end if

    	$new_default_zoom_level	= isset($_POST[$prefix.'_default_zoom_level']) ? $_POST[$prefix.'_default_zoom_level'] : '';
		if (!is_null($new_default_zoom_level) && $new_default_zoom_level != $default_zoom_level) {
			$attr_change = TRUE;
			$asset->setAttrValue('default_zoom_level', $new_default_zoom_level);
		}//end if

		if ($attr_change) $asset->saveAttributes();

		return FALSE;

    }//end paintDefaultLatLng()


    /**
    * paint default marker icon
    *
    * @return boolean
    * @access public
    */
    function paintDefaultMarkerIcon(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');
		$use_google_marker_icon	= $asset->attr('use_google_marker_icon');
		$default_marker_icon	= $asset->attr('default_marker_icon');

		$o->openField('Default Marker Icon URL');
				if ($wa) {
					check_box($prefix.'_use_default_marker_icon', '1', $use_google_marker_icon, '
						if (this.checked==1) {
							document.getElementById("'.$prefix.'_default_marker_icon_url").disabled=true;
						} else {
							document.getElementById("'.$prefix.'_default_marker_icon_url").disabled=false;
						}//end else
							');
					echo ' Use Google Default Marker<br /><br />';
					$extra = $use_google_marker_icon ? 'disabled=true' : '';
					text_box($prefix.'_default_marker_icon_url', $default_marker_icon, empty($default_marker_icon) ? 25 : strlen($default_marker_icon), '', FALSE, $extra);;
					$o->note('The URL to default icon for markers on the map');
				} else {
					echo $default_marker_icon;
					$o->note('The URL to default icon for markers on the map');

				}//end else

		$o->closeField();

		return $wa;

    }//end paintDefaultMarkerIcon()


    /**
    * process default marker icon
    *
    * @return boolean
    * @access public
    */
    function processDefaultMarkerIcon(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');
		$default_marker_icon	= $asset->attr('default_marker_icon');
		$use_google_marker_icon	= $asset->attr('use_google_marker_icon');

		$attr_change = FALSE;
		$new_use_google_marker_icon	= isset($_POST[$prefix.'_use_default_marker_icon']) ? $_POST[$prefix.'_use_default_marker_icon'] : FALSE;
		if (!is_null($new_use_google_marker_icon) && $new_use_google_marker_icon != $use_google_marker_icon) {
			$attr_change = TRUE;
			$asset->setAttrValue('use_google_marker_icon', $new_use_google_marker_icon);
		}
		if ($new_use_google_marker_icon != 1) {
			$new_default_marker_icon	= isset($_POST[$prefix.'_default_marker_icon_url']) ? $_POST[$prefix.'_default_marker_icon_url'] : '';
			if (!is_null($new_default_marker_icon) && $new_default_marker_icon != $default_marker_icon) {
				$asset->setAttrValue('default_marker_icon', $new_default_marker_icon);
				$attr_change = TRUE;
			}//end if
		}//end if

		if ($attr_change) $asset->saveAttributes();

		return FALSE;

    }//end processDefaultMarkerIcon()


    /**
    * paint listing option
    *
    * @param object $asset
    * @param object $o
    * @param string $prefix
    *
    * @return unknown
    * @access public
    */
	function paintListingOption(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');

		$enable_listing			= $asset->attr('enable_listing');
		$listing_root_nodes		= $asset->attr('listing_root_nodes');
		if (!is_array($listing_root_nodes))
			$listing_root_nodes	= unserialize($listing_root_nodes);

		bam($listing_root_nodes);
		$default_listing_asset	= $asset->attr('default_listing_asset');
		$extra_custom	= $enable_listing ? '' : ' disabled="disabled"';
		$extra_fields	= $enable_listing&&$default_listing_asset ? '' : ' disabled="disabled"';

		if (!$enable_listing) {
			echo '<span style="color:red;">Listing Feature is not enabled.</span>';
		}//end if

		if (!empty($listing_root_nodes)) {
			$o->openSection('Listing Nodes');
			$o->openRaw();
			?>

				<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 0px">
					<table style="width:90%;padding-left: 30px;">
						<tr>
							<td class="sq-backend-table-cell" style="width:118px;padding-left:15px">Type Name</td>
							<td class="sq-backend-table-cell">Listing Root Node Options</td>
							<?php if ($wa) { ?>
								<td class="sq-backend-table-cell"><?php echo translate('Delete?'); ?>

							<?php }//end if ?>

						<?php
							foreach ($listing_root_nodes as $node_info) {
								if (is_string($node_info)) $node_info = unserialize($node_info);
						?>
						<tr>
							<td class="sq-backend-table-cell" style="width:118px;padding-left:15px;">
								<strong><?php echo $node_info['type_name'] ?></strong>
								<br />
							</td>
							<td class="sq-backend-table-cell">
							<?php
								if ($wa) {
									asset_finder($prefix.'[existing_nodes]', $node_info['root_assetid'], Array('page_asset_listing'));
								} else {
									echo get_asset_tag_line($node_info['root_assetid']);
								}
							?><br />
							<?php	if (isset($node_info['icon_url'])) { ?>
								<br /><span>Default Icon URL: <b><?php echo $node_info['icon_url'] ?></b></span>
							<?php }
									if (isset($node_info['use_google_location_asset'])) {
										$use_default = !empty($node_info['use_google_location_asset']);

										?>
								<br /><span>Use Default Google Location Asset: <b><?php echo $use_default ? 'Yes' : 'No' ;  ?></b></span><br />
							<?php
										if (isset($node_info['latitude_keyword'])) {
										?>
											<br /><span>Latitude Keyword: <b><?php echo $node_info['latitude_keyword'];  ?></b></span>
										<?php
										}
										if (isset($node_info['longitude_keyword'])) {
										?>
											<br /><span>Longitude Keyword: <b><?php echo $node_info['longitude_keyword'];  ?></b></span>
										<?php
										}
										if (isset($node_info['description_keyword'])) {
										?>
											<br /><span>Description Keyword: <b><?php echo $node_info['description_keyword'];  ?></b></span>
										<?php
										}
										if (isset($node_info['icon_url_keyword'])) {
										?>
											<br /><span>Icon URL Keyword: <b><?php echo $node_info['icon_url_keyword'];  ?></b></span>
										<?php
										}
									}
							?>
							</td>
							<?php if ($wa) {?>
									<td class="sq-backend-table-cell"><?php check_box($prefix.'[delete]['.$node_info['root_assetid'].']', $node_info['root_assetid'], FALSE); ?></td>
							<?php } ?>
						</tr>
						<?php }//end foreach
						?>
					</table>
				</div>
			<?php
			$o->closeRaw();
			$o->closeSection();
		}//end if


		if ($wa) {

			$o->openSection('Add New Listing Node');
			?>
			<?php
			$o->openField('New Root Node');
				 asset_finder($prefix.'_new_root_node', '', Array('page_asset_listing' => 'D'));
				$o->note('Select an Asset Listing to use');
			$o->closeField();

			$o->openField('Type Name');
				text_box($prefix.'_new_type_name', '', 25);
				$o->note('The Type Name associate with the listed assets');
			$o->closeField();

			$o->openField('Icon URL');
				text_box($prefix.'_new_icon_url', '', 25);
				$o->note('The Icon URL for these listed assets');
			$o->closeField();

			$o->openField('Use Default Google Location Asset');
				check_box($prefix.'_new_use_google_location_asset', '1', TRUE, 'display_keywords();');
				$o->note('');
			$o->closeField();

			$o->openRaw();
			?>
				<script type="text/javascript">
					function display_keywords()
					{
						var new_display_val = 'block';
						var display_val = document.getElementById('keyword_div').style.display;
						if (display_val != 'none') {
							new_display_val = 'none';
						}//end if
						document.getElementById('keyword_div').style.display = new_display_val;
					}//end switch_display()

    			</script>
						<div id="keyword_div" style="width:100%;height:auto;border:0px solid;display:none;">
						<table style="width:auto;padding-left: 0px;">
							<tr>
								<td width="150" style="font-weight:bold;padding-left:44px;">Location Name Keyword</td>
								<td><?php	text_box($prefix.'_new_location_name_keyword', '', 25);	?></td>
							</tr>
							<tr>
								<td width="150" style="font-weight:bold;padding-left:44px;">Latitude Keyword</td>
								<td><?php	text_box($prefix.'_new_latitude_keyword', '', 25);	?></td>
							</tr>
							<tr>
								<td style="font-weight:bold;padding-left:44px;">Longitude Keyword</td>
								<td><?php	text_box($prefix.'_new_longitude_keyword', '', 25); ?></td>
							</tr>
							<tr>
								<td style="font-weight:bold;padding-left:44px;">Description Keyword</td>
								<td><?php	text_box($prefix.'_new_description_keyword', '', 25); ?></td>
							</tr>
							<tr>
								<td style="font-weight:bold;padding-left:44px;">Icon URL Keyword</td>
								<td><?php	text_box($prefix.'_new_icon_url_keyword', '', 25); ?></td>
							</tr>
						</table>
						<p style="padding-left:44px;">Use Keywords to specify custom fields. (Example: %asset_metadata_fieldname%)</p>
						<table style="width:auto;padding-left: 0px;">
							<tr>
								<td width="150" style="font-weight:bold;padding-left:44px;"><br />Custom Popup Layout<br /></td>
								<td><?php	check_box($prefix.'_new_use_asset_listing_bodycopy', '1', TRUE);	?><br /><br /></td>
							</tr>
						</table>
						</div>
			<?php
			$o->closeRaw();

			$o->closeSection();
		}//end if
		?>
		<?php

		return $wa;

    }//end paintListingOption()


    /**
    * process listing function
    *
    * @param object $asset
    * @param object $o
    * @param string $prefix
    *
    * @return boolean
    * @access public
    */
	function processListingOption(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');

		$enable_listing			= $asset->attr('enable_listing');
		$listing_root_nodes		= ($asset->attr('listing_root_nodes'));
		$listing_root_nodes		= unserialize($listing_root_nodes);

		$default_listing_asset	= $asset->attr('default_listing_asset');

		$new_root_node					= isset($_POST[$prefix.'_new_root_node']) ? $_POST[$prefix.'_new_root_node'] : '';
		$new_type_name					= isset($_POST[$prefix.'_new_type_name']) ? $_POST[$prefix.'_new_type_name'] : '';
		$new_icon_url					= isset($_POST[$prefix.'_new_icon_url']) ? $_POST[$prefix.'_new_icon_url'] : '';
		$new_use_google_location_asset	= isset($_POST[$prefix.'_new_use_google_location_asset']) ? $_POST[$prefix.'_new_use_google_location_asset'] : '';

		$new_location_name_keyword		= isset($_POST[$prefix.'_new_location_name_keyword']) ? $_POST[$prefix.'_new_location_name_keyword'] : '';
		$new_latitude_keyword			= isset($_POST[$prefix.'_new_latitude_keyword']) ? $_POST[$prefix.'_new_latitude_keyword'] : '';
		$new_longitude_keyword			= isset($_POST[$prefix.'_new_longitude_keyword']) ? $_POST[$prefix.'_new_longitude_keyword'] : '';
		$new_description_node			= isset($_POST[$prefix.'_new_description_keyword']) ? $_POST[$prefix.'_new_description_keyword'] : '';
		$new_icon_url_node				= isset($_POST[$prefix.'_new_icon_url_keyword']) ? $_POST[$prefix.'_new_icon_url_keyword'] : '';

		$delete_list					= isset($_POST[$prefix]['delete']) ? $_POST[$prefix]['delete'] : Array();

		$attr_change = FALSE;
		$new_root_node_options = Array();

		if (!empty($new_root_node) && !empty($new_root_node['assetid']) && $wa) {
			$new_root_node_options	= Array (
										'root_assetid'	=> $new_root_node['assetid'],
									  );

			if (!empty($new_type_name)) {
				$new_root_node_options['type_name']	= $new_type_name;
			}//end if
			if (!empty($new_icon_url))	{
				$new_root_node_options['icon_url']	= $new_icon_url;
			}//end if
			if (!is_null($new_use_google_location_asset))	{
				$new_root_node_options['use_google_location_asset']	= $new_use_google_location_asset;
			}//end if

			if (($new_use_google_location_asset == 0)) {
				if (!empty($new_location_name_keyword)) {
					$new_root_node_options['location_name_keyword']	=	$new_location_name_keyword;
				}//end if
				if (!empty($new_latitude_keyword)) {
					$new_root_node_options['latitude_keyword']		=	$new_latitude_keyword;
				}//end if
				if (!empty($new_latitude_keyword)) {
					$new_root_node_options['latitude_keyword']		=	$new_latitude_keyword;
				}//end if
				if (!empty($new_longitude_keyword)) {
					$new_root_node_options['longitude_keyword']		=	$new_longitude_keyword;
				}//end if
				if (!empty($new_description_node)) {
					$new_root_node_options['description_keyword']	=	$new_description_node;
				}//end if
				if (!empty($new_icon_url_node)) {
					$new_root_node_options['icon_url_keyword']		=	$new_icon_url_node;
				}//end if
			}//end if

			$listing_root_nodes[$new_root_node['assetid']] = $new_root_node_options;
			$asset->setAttrValue('listing_root_nodes', $listing_root_nodes);
			$attr_change = TRUE;
		}//end if

		if (!empty($delete_list)) {
			foreach ($delete_list as $node_id => $id_node) { // They are the same thing
				if (isset($listing_root_nodes[$node_id])) {
					unset($listing_root_nodes[$node_id]);
					$asset->setAttrValue('listing_root_nodes', $listing_root_nodes);
					$attr_change = TRUE;
				}//end if
			}//end foreach
		}//end if

		if ($attr_change)	$asset->saveAttributes();

		return TRUE;

    }//end paintListingOption()


    /**
    * paint Asset Types
    *
    * @param object $asset
    * @param object $o
    * @param string $prefix
    *
    * @return boolean
    * @access public
    */
    function paintAssetTypes(&$asset, &$o, $prefix)
    {
    	$wa = $asset->writeAccess('attributes');
		parent::paintAssetTypes($asset, $o, $prefix);
		$types = $asset->attr('types');
		$keywords_list	= $asset->attr('asset_types_keywords');

		$o->openRaw();
		foreach ($types as $type_code => $inherit) {
			if ($type_code !== 'google_map_location') {

					?>
					<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 0px 30px;margin-left:75px;">
						<?php if (!isset($keywords_list[$type_code])) echo 'Type code '.ucwords(str_replace('_',' ', $type_code)) .' is not of type Google Map Location, Mapping of keywords is required.'; ?>
						<p class="sq-backend-section-subheading"><?php echo 'Keyword mapping for '.ucwords(str_replace('_',' ', $type_code)); ?></p>

						<div id="<?php echo $prefix ?>_connector_div" style="padding: 0px 0px 2em 30px;">
							<table class="sq-backend-table" style="width:100%">
								<tr>
									<td width="100"><p></p>Location Name keyword</td>
									<td><p><?php
												if (!$wa) {
													echo array_get_index($keywords_list[$type_code], 'name_keyword', '');
												} else {
													text_box($prefix.'_'.$type_code.'_name_keyword', array_get_index($keywords_list[$type_code], 'name_keyword', ''), 50);
												}//end if
										   ?></p></td>
								</tr>
								<tr>
									<td width="100"><p></p>Latitude keyword</td>
									<td><p><?php
												if (!$wa) {
													echo array_get_index($keywords_list[$type_code], 'latitude_keyword', '');
												} else {
													text_box($prefix.'_'.$type_code.'_latitude_keyword', array_get_index($keywords_list[$type_code], 'latitude_keyword', ''), 50);
												}//end if
										   ?></p></td>
								</tr>
								<tr>
									<td width="100"><p></p>Longitude keyword</td>
									<td><p><?php
												if (!$wa) {
													echo array_get_index($keywords_list[$type_code], 'longitude_keyword', '');
												} else {
													text_box($prefix.'_'.$type_code.'_longitude_keyword', array_get_index($keywords_list[$type_code], 'longitude_keyword', ''), 50);
												}//end else
										   ?></p></td>
								</tr>
								<tr>
									<td width="100"><p></p>Description keyword</td>
									<td><p><?php
												if (!$wa) {
													echo array_get_index($keywords_list[$type_code], 'description_keyword', '');
												} else {
													text_box($prefix.'_'.$type_code.'_description_keyword', array_get_index($keywords_list[$type_code], 'description_keyword', ''), 50);
												}//end else
										   ?></p></td>
								</tr>
								<tr>
									<td width="100"><p></p>Icon URL keyword</td>
									<td><p><?php
												if (!$wa) {
													echo array_get_index($keywords_list[$type_code], 'icon_url_keyword', '');
												} else {
													text_box($prefix.'_'.$type_code.'_icon_url_keyword', array_get_index($keywords_list[$type_code], 'icon_url_keyword', ''), 50);
												}
										   ?></p></td>
								</tr>
								<tr>
									<td width="100"><p></p>Address Keyword</td>
									<td><p><?php
												if (!$wa) {
													echo array_get_index($keywords_list[$type_code], 'address_keyword', '');
												} else {
													text_box($prefix.'_'.$type_code.'_address_keyword', array_get_index($keywords_list[$type_code], 'address_keyword', ''), 50);
												}
										   ?></p></td>
								</tr>
							</table>
						</div>
					</div>
					<?php

			}//end if
		}//end foreach
		?><br /><br /><?php
		$o->closeRaw();

    }//end paintAssetTypes()


	/**
    * process listing function
    *
    * @param object $asset
    * @param object $o
    * @param string $prefix
    *
    * @return boolean
    * @access public
    */
    function processAssetTypes(&$asset, &$o, $prefix)
    {
		parent::processAssetTypes($asset, $o, $prefix);

		// In this process function we have to capture all the values given to each of the asset types.
		$types = $asset->attr('types');
		$keywords_list	= $asset->attr('asset_types_keywords');

		foreach ($types as $type_code => $inherit) {
			if ($type_code !== 'google_map_location') {
				// Now we check for everything in POST
				$keywords_list[$type_code]['name_keyword']			= array_get_index($_POST, $prefix.'_'.$type_code.'_name_keyword', NULL);
				$keywords_list[$type_code]['latitude_keyword']		= array_get_index($_POST, $prefix.'_'.$type_code.'_latitude_keyword', NULL);
				$keywords_list[$type_code]['longitude_keyword']		= array_get_index($_POST, $prefix.'_'.$type_code.'_longitude_keyword', NULL);
				$keywords_list[$type_code]['description_keyword']	= array_get_index($_POST, $prefix.'_'.$type_code.'_description_keyword', NULL);
				$keywords_list[$type_code]['icon_url_keyword']		= array_get_index($_POST, $prefix.'_'.$type_code.'_icon_url_keyword', NULL);
				$keywords_list[$type_code]['address_keyword']		= array_get_index($_POST, $prefix.'_'.$type_code.'_address_keyword', NULL);
			}//end if
		}//end foreach
		$asset->setAttrValue('asset_types_keywords', $keywords_list);

    }//end processAssetTypes()


    /**
    * This function paint the map code
    *
    * @return void
    * @access public
    */
    function paintMapInitialisation(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');

		$o->openField('Map Initialisation');
		$map_code	= $asset->attr('map_code');
		if ($wa) {
			text_area($prefix.'_map_code', $map_code, 100, 10);
		} else {
			echo nl2br(htmlentities($map_code));
		}//end else
		$o->closeField();

		$street_view	= $asset->attr('enable_street_view');
		$extra			= 'disabled=disabled';

		$o->openField('Standalone Street View Initialisation');
		$street_view_code	= $asset->attr('street_view_code');
		if ($wa) {
			text_area($prefix.'_street_view_code', $street_view_code, 100, 10, FALSE, $street_view ? '' : $extra);
		} else {
			echo nl2br(htmlentities($street_view_code));
		}//end else
		$o->closeField();

		return $wa;

    }//end paintMapInitialisation()


    /**
    * This function paint the map code
    *
    * @return void
    * @access public
    */
    function processMapInitialisation(&$asset, &$o, $prefix)
    {
		$wa = $asset->writeAccess('attributes');
		if (isset($_POST[$prefix.'_map_code'])) {
			$new_map_code	= $_POST[$prefix.'_map_code'];
			$map_code	= $asset->attr('map_code');
			if ($new_map_code != $map_code) {
				$asset->setAttrValue('map_code', $new_map_code);
			}//end if
		}//end if

		if (isset($_POST[$prefix.'_street_view_code'])) {
			$new_street_view_code	= $_POST[$prefix.'_street_view_code'];
			$street_view_code	= $asset->attr('street_view_code');
			if ($new_street_view_code != $street_view_code) {
				$asset->setAttrValue('street_view_code', $new_street_view_code);
			}//end if
		}//end if

		return TRUE;

    }//end processMapInitialisation()


}//end class
?>
