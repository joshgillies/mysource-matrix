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
* $Id: page_asset_builder_edit_fns.inc,v 1.12 2004/08/02 01:11:08 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Builder_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Messages used to provide info about the differnt types of create locations
	* @var array(string => string)
	*/
	var $location_messages = Array(
						'mandatory' => 'All new assets will be placed into these locations',
						'optional'  => 'When creating a new asset, the user will have the option of placing the asset into one or more of these locations',
						'rules'     => 'These rules define where new assets should be created based on the values filled in by the user',
										);

	/**
	* An array of asset types that can be passed to asset_finder to define what types of create locations are allowed
	* @var array(string => string)
	*/
	var $location_types = Array();


	/**
	* Constructor
	*
	*/
	function Page_Asset_Builder_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paint the current asset types that have been added
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		// do a check to see if any of the bodycopies have been removed manually
		$types = $asset->attr('create_type');
		if (empty($types)) {
			echo 'There are no Types Defined';
			return false;
		}

		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header">Asset Type</td>
			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header">Remove?</td>
				<?php
			}
			?>
		</tr>
		<?php
		foreach ($types as $type => $value) {
			?>
			<tr>
				<td class="sq-backend-table-cell">
					<?php
					$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type);
					echo $info['name'];
					?>
				</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-cell">
						<?php
						check_box($prefix.'_remove[]', $type, false);
						?>
					</td>
					<?php
				}
				?>
			</tr>
			<?php
		}//end foreach
		?>
		</table>
		<?php

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('create_type');

		// we are looking to see if any types have been removed
		if (isset($_POST[$prefix.'_remove'])) {
			foreach ($_POST[$prefix.'_remove'] as $type) {
				if (isset($types[$type])) unset($types[$type]);
			}
		}

		$asset->setAttrValue('create_type', $types);

		return true;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			if ($asset->attr('create_type') == '') {
				echo 'No asset type selected';
			} else {
				$is_multiple = (count($asset->attr('create_type')) > 1);
				if ($is_multiple) echo '<ul>';
				foreach(array_keys($asset->attr('create_type')) as $create_type) {
					$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($create_type);
					if ($is_multiple) echo '<li>';
					echo $info['name'];
					if ($is_multiple) echo '</li>';
				}
				if ($is_multiple) echo '</ul>';
			}
		} else {
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			foreach ($asset_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			asort($options);
			$options = array_reverse($options, true);
			$options[''] = '-- select asset type --';
			$options = array_reverse($options, true);
			combo_box($prefix.'_new_type', $options, false, '');
		}

	}//end paintAddCreateType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddCreateType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_type'])) {
			$type = trim($_POST[$prefix.'_new_type']);
			if (empty($type)) return false;
			$types = $asset->attr('create_type');
			if (!isset($types[$type])) {
				$types[$type] = true;
				$asset->setAttrValue('create_type', $types);
				return true;
			}
		}
		return false;

	}//end processAddCreateType()
	
	
	/**
	* Paint the interface for choosing the type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			if ($asset->attr('create_type') == '') {
				echo 'No asset type selected';
			} else {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->attr('create_type'));
				echo $info['name'];
			}
		} else {
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			$options = Array('' => '-- select asset type --');
			foreach ($asset_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_new_type', $options, false, $asset->attr('create_type'));
		}

	}//end paintCreateType()


	/**
	* Processes the interface for choosing the type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateType(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_type'])) return false;
		$create_type = trim($_POST[$prefix.'_create_type']);
		return $asset->setAttrValue('create_type', $create_type);

	}//end processCreateType()


	/**
	* Paint the interface for choosing the link type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLink(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if (!$asset->writeAccess('attributes')) {
			echo link_type_name($asset->attr('create_link_type'));
		} else {
			$link_options = get_constant_values('SQ_LINK_');
			$options = Array();
			foreach($link_options as $link_name => $bit_value) {
				$options[$bit_value] = link_type_name($bit_value);
			}
			combo_box($prefix.'_create_link_type', $options, false, $asset->attr('create_link_type'));
		}

	}//end paintCreateLink()


	/**
	* Processes the interface for choosing the link type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateLink(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_link_type'])) return false;
		$create_link_type = (int)$_POST[$prefix.'_create_link_type'];
		return $asset->setAttrValue('create_link_type', $create_link_type);

	}//end processCreateLink()


	/**
	* Paint the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLocation(&$asset, &$o, $prefix)
	{
		echo '<i><b>'.$this->location_messages['mandatory'].'</b></i><br/>';

		$write_access = $asset->writeAccess('attributes');
		$locations = $asset->attr('create_location');

		if (empty($locations)) {
			echo 'No create locations have been specified';
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">ID</td>
					<td class="sq-backend-table-header">Name</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($locations as $locationid => $null) {
					$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
					if (is_null($location)) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $location->id; ?></td>
						<td class="sq-backend-table-cell"><?php echo $location->name; ?></td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-cell"><?php check_box($prefix.'_delete_location['.$location->id.']'); ?></td><?php
						}
						?>
					</tr>
					<?php
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($location);
				}
				?>
			</table>
			<?php
		}

		if ($write_access) {
			echo '<br/><b>Add Location:</b> ';
			asset_finder($prefix.'_new_create_location', 0, $this->location_types);
		}

		return $write_access;

	}//end paintCreateLocation()


	/**
	* Processes the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateLocation(&$asset, &$o, $prefix)
	{
		$old_locations = $asset->attr('create_location');
		$new_locations = $old_locations;

		// add a new create location
		if (isset($_POST[$prefix.'_new_create_location'])) {
			$new_location =(int) $_POST[$prefix.'_new_create_location']['assetid'];
			if ($new_location && !isset($new_locations[$new_location])) {
				$new_locations[$new_location] = $new_location;
			}
		}

		// delete existing create locations
		if (isset($_POST[$prefix.'_delete_location'])) {
			foreach ($_POST[$prefix.'_delete_location'] as $locationid => $on) {
				if (isset($new_locations[$locationid])) unset($new_locations[$locationid]);
			}
		}

		if ($old_locations != $new_locations) return $asset->setAttrValue('create_location', $new_locations);
		return false;

	}//end processCreateLocation()


	/**
	* Paint the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLocationOptional(&$asset, &$o, $prefix)
	{
		echo '<i><b>'.$this->location_messages['optional'].'</b></i><br/>';

		$write_access = $asset->writeAccess('attributes');
		$locations = $asset->attr('create_location_optional');

		if (empty($locations)) {
			echo 'No create locations have been specified';
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">ID</td>
					<td class="sq-backend-table-header">Name</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($locations as $locationid => $null) {
					$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
					if (is_null($location)) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $location->id; ?></td>
						<td class="sq-backend-table-cell"><?php echo $location->name; ?></td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-cell"><?php check_box($prefix.'_delete_location_optional['.$location->id.']'); ?></td><?php
						}
						?>
					</tr>
					<?php
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($location);
				}
				?>
			</table>
			<?php
		}

		if ($write_access) {
			echo '<br/><b>Add Location:</b> ';
			asset_finder($prefix.'_new_create_location_optional', 0, $this->location_types);
		}

		return $write_access;

	}//end paintCreateLocationOptional()


	/**
	* Processes the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateLocationOptional(&$asset, &$o, $prefix)
	{
		$old_locations = $asset->attr('create_location_optional');
		$new_locations = $old_locations;

		// add a new create location
		if (isset($_POST[$prefix.'_new_create_location_optional'])) {
			$new_location =(int) $_POST[$prefix.'_new_create_location_optional']['assetid'];
			if ($new_location && !isset($new_locations[$new_location])) {
				$new_locations[$new_location] = $new_location;
			}
		}

		// delete existing create locations
		if (isset($_POST[$prefix.'_delete_location_optional'])) {
			foreach ($_POST[$prefix.'_delete_location_optional'] as $locationid => $on) {
				if (isset($new_locations[$locationid])) unset($new_locations[$locationid]);
			}
		}

		if ($old_locations != $new_locations) return $asset->setAttrValue('create_location_optional', $new_locations);
		return false;

	}//end processCreateLocationOptional()


	/**
	* Paint the interface for setting up create location rules
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLocationRules(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_rules = $asset->attr('create_location_rules');
		$current_assets = $asset->attr('create_type');
		
		if (empty($current_assets)) {
			echo 'You must specify the type of asset to create before create location rules can be defined';
			return false;
		}
		echo '<i><b>'.$this->location_messages['rules'].'</b></i><br/>';
		foreach(array_keys($asset->attr('create_type')) as $create_type) {

			$attributes = Array();
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type, Array('name', 'attributeid', 'type'));
			foreach ($attrs as $name => $data) $attributes[$name] = ucwords(str_replace('_', ' ', $name));
			
			$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($create_type, 'name');
			$o->openSection($type_name);
			$o->openRaw();
			
			if (empty($current_rules[$create_type])) $current_rules[$create_type] = Array();
			$type_current_rules = $current_rules[$create_type];
			
			$type_prefix = $prefix.'_'.$create_type;
			
			$attributes = array_reverse($attributes, true);
			$attributes[''] = '-- select attribute --';
			$attributes = array_reverse($attributes, true);

			if (!empty($type_current_rules)) ?><table class="sq-backend-table"><?php

			$i = -1;
			foreach ($type_current_rules as $i => $attr_rules) {

				foreach ($attr_rules as $attr => $rule_conds) {
					if ($attr == '_create_locations_') continue;
			
					$x = 0;
					foreach ($rule_conds as $cond => $cond_value) {
	
						if ($write_access) {
							require_once SQ_ATTRIBUTES_PATH.'/'.$attrs[$attr]['type'].'/'.$attrs[$attr]['type'].'.inc';
							$attr_class = 'Asset_Attribute_'.$attrs[$attr]['type'];
							$attribute = new $attr_class($attrs[$attr]['attributeid'], $cond_value);
						}
		
						?>
						<tr>
							<td class="sq-backend-table-cell">the value of attribute </td>
							<td class="sq-backend-table-cell">
								<?php
									if ($write_access) {
										combo_box($type_prefix.'_rules['.$i.']['.$attr.'][attrs]['.$x.']', $attributes, false, $attr);
									} else {
										echo $attributes[$attr];
									}
								?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
									$conditions = Array('equals'      => 'equals',
														'starts_with' => 'starts with',
														'ends_with'   => 'ends with'
														);
	
									if ($write_access) {
										combo_box($type_prefix.'_rules['.$i.']['.$attr.'][condition]['.$x.']', $conditions, false, $cond);
									} else {
										echo $conditions[$cond];
									}
								?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
									if ($write_access) {
										$attribute->paint($type_prefix.'_rules['.$i.']['.$attr.'][values]['.$x.']', false);
									} else {
										echo $cond_value;
									}
								?>
							</td>
						</tr>
						<?php
		
						$x++;
		
					}//end foreach
					
				}//end foreach
	
	
				if ($write_access) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><b>New Attribute</b></td>
						<td class="sq-backend-table-cell" colspan="3"><?php combo_box($type_prefix.'_rules['.$i.'][new_attr]', $attributes, false, ''); ?></td>
					</tr>
					<?php
				}
				
	
				$l = 1;
						
				$locations = $attr_rules['_create_locations_'];
				if (!$write_access) $location_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($locations));
				foreach ($locations as $location) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo ($l == 1) ? '<b>Location(s)</b>' : ''; ?></td>
						<td class="sq-backend-table-cell" colspan="3">
							<?php
								if ($write_access) {
									asset_finder($type_prefix.'_rule_location['.$i.']['.$l.']', $location, $this->location_types);
								} else {
									echo $location_info[$location]['name'].' [# '.$location.']';
								}
							?>
						</td>
					</tr>
					<?php
					$l++;
				}//end foreach
	
				if ($write_access) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><b>New Location</b></td>
						<td class="sq-backend-table-cell" colspan="3"><?php asset_finder($type_prefix.'_rule_location['.$i.']['.$l.']', 0, $this->location_types); ?></td>
					</tr>
					<?php
				}
				?>
				<tr>
					<td class="sq-backend-table-cell" colspan="4"><hr/></td>
				</tr>
				<?php
	
			}//end foreach
	
			if (!empty($current_rules)) { ?></table><?php }
	
			if ($write_access) {
				if ($i >=0) $i++;
				else $i = 0;
				echo '<b>New Rule:</b> ';
				$attributes[''] = '-- select attribute --';
				combo_box($type_prefix.'_rules['.$i.'][new_attr]', $attributes, false, '');
			}
			$o->closeRaw();
			$o->closeSection();
			?>

			<?php
		}//end foreach create types

		return true;

	}//end paintCreateLocationRules()


	/**
	* Processes the interface for setting up create location rules
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateLocationRules(&$asset, &$o, $prefix)
	{
		$rules = Array();
		foreach (array_keys($asset->attr('create_type')) as $create_type) {

			$type_prefix = $prefix.'_'.$create_type;

			foreach ($_POST[$type_prefix.'_rules'] as $i => $rule_data) {

				foreach ($rule_data as $attr => $rule) {
					
					if ($attr == '') continue;
					
					if ($attr == 'new_attr') {
						if (!empty($rule)) $rules[$create_type][$i][$rule]['equals'] = '';
						continue;
					}
		
					foreach ($rule['condition'] as $x => $condition) {
						if (empty($rule['attrs'][$x])) continue;
						$value = (isset($rule['values'][$x])) ? $rule['values'][$x] : '';
						$rules[$create_type][$i][$attr][$condition] = $value;
					}
				}

				if (isset($rules[$create_type][$i])) {
					$locations = Array();
					
					if (isset($_POST[$type_prefix.'_rule_location'][$i])) {
						foreach ($_POST[$type_prefix.'_rule_location'][$i] as $loc) {
							$locid = $loc['assetid'];
							if (!$locid) continue;
							$locations[] = $locid;
						}
					}
					
					$rules[$create_type][$i]['_create_locations_'] = array_unique($locations);
				}

			}//end foreach rules

		}//end foreach createt types

		return $asset->setAttrValue('create_location_rules', $rules);

	}//end processCreateLocationRules()


	/**
	* Paint the interface for choosing the status of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateStatus(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if (!$asset->writeAccess('attributes')) {
			echo get_status_description($asset->attr('create_status'));
		} else {
			$status_options = Array(SQ_STATUS_ARCHIVED => 'Archived',
									SQ_STATUS_UNDER_CONSTRUCTION => 'Under Construction',
									SQ_STATUS_LIVE => 'Live',
									);
			combo_box($prefix.'_create_status', $status_options, false, $asset->attr('create_status'));
		}

	}//end paintCreateStatus()


	/**
	* Processes the interface for choosing the status of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateStatus(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_status'])) return false;
		$create_status = (int)$_POST[$prefix.'_create_status'];
		return $asset->setAttrValue('create_status', $create_status);

	}//end processCreateStatus()


	/**
	* Paint the interface for choosing the status of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRequiredFields(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current = $asset->attr('required_fields');
		
		foreach(array_keys($asset->attr('create_type')) as $create_type) {
		
			$attributes = Array();
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type, Array('name'));
			foreach ($attrs as $name) $attributes[$name] = ucwords(str_replace('_', ' ', $name));
			
			if (!isset($current[$create_type])) $current[$create_type] = Array();	
			$type_prefix = $prefix.'_'.$create_type;
			
			$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($create_type, 'name');
			$o->openSection($type_name);
			$o->openRaw();
			
			if (empty($current[$create_type])) {
				echo 'No attributes are currently set to be required';
			} else {
				
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">Name</td>
						<td class="sq-backend-table-header">Error Text</td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-header">Delete</td><?php
						}
						?>
					</tr>
					<?php
					foreach ($current[$create_type] as $attr => $error_text) {
						?>
						<tr>
							<td class="sq-backend-table-cell"><?php echo $attributes[$attr]; ?></td>
							<td class="sq-backend-table-cell">
								<?php
								if ($write_access) {
									text_box($type_prefix.'_required_field['.$attr.']', $error_text, 50);
								} else {
									echo $error_text;
								}
								?>
							</td>
							<?php
							if ($write_access) {
								?>
								<td class="sq-backend-table-cell">
									<?php check_box($type_prefix.'_delete_required['.$attr.']'); ?>
								</td>
								<?php
							}
							?>
						</tr>
						<?php
					}
					?>
				</table>
				<?php
			}
	
			if ($write_access) {
				$attributes = array_reverse($attributes, true);
				$attributes[''] = '-- select attribute --';
				$attributes = array_reverse($attributes, true);
	
				echo '<br/><b>New required attribute ? </b>';
				combo_box($type_prefix.'_new_required', $attributes, false, '');
			}
			
			$o->closeRaw();
			$o->closeSection();
			
		}

		return $write_access;

	}//end paintRequiredFields()


	/**
	* Processes the interface for choosing the status of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processRequiredFields(&$asset, &$o, $prefix)
	{
		$current = $asset->attr('required_fields');
		$required_fields = Array();

		foreach (array_keys($asset->attr('create_type')) as $create_type) {
		
			if (!isset($current[$create_type])) $current[$create_type] = Array();
			$type_prefix = $prefix.'_'.$create_type;
			
			// construct new required fields array removing deleted fields and updating error text
			foreach ($current[$create_type] as $attr => $error_text) {
				if (!isset($_POST[$type_prefix.'_delete_required'][$attr]) || !$_POST[$type_prefix.'_delete_required'][$attr]) {
					$required_fields[$create_type][$attr] = (isset($_POST[$type_prefix.'_required_field'][$attr])) ? trim($_POST[$type_prefix.'_required_field'][$attr]) : $current[$create_type][$attr];
				}
			}
	
			// add new required field
			if (isset($_POST[$type_prefix.'_new_required']) && !empty($_POST[$type_prefix.'_new_required'])) {
				$new_required = $_POST[$type_prefix.'_new_required'];
				if (!in_array($new_required, $required_fields)) $required_fields[$create_type][$new_required] = '';
			}
		}
			
		return $asset->setAttrValue('required_fields', $required_fields);

	}//end processRequiredFields()


	/**
	* Returns true if the asset builder allows the user to select multiple create locations
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	function definesMultipleLocations(&$asset)
	{
		return (count($asset->attr('create_location_optional')) > 1);

	}//end definesMultipleLocations()


	/**
	* Returns true if the asset builder allows the user to create more than one asset type
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	function definesMultipleAssetTypes(&$asset)
	{
		return (count($asset->attr('create_type')) > 1);

	}//end definesMultipleLocations()


}//end class

?>
