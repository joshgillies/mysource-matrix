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
* $Id: page_asset_builder_edit_fns.inc,v 1.6 2004/03/17 04:08:10 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';
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
			combo_box($prefix.'_create_type', $options, false, $asset->attr('create_type'));
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

	}//end processCreateLocation()


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
		echo '<i><b>'.$this->location_messages['rules'].'</b></i><br/>';

		$write_access = $asset->writeAccess('attributes');
		$current_rules = $asset->attr('create_location_rules');

		$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($asset->attr('create_type'), Array('name', 'attributeid', 'type'));
		foreach ($attrs as $name => $data) $attributes[$name] = ucwords(str_replace('_', ' ', $name));
		$attributes = array_reverse($attributes, true);
		$attributes[''] = '-- select attribute --';
		$attributes = array_reverse($attributes, true);

		if (!empty($current_rules)) ?><table class="sq-backend-table"><?php

		$i = 1;
		foreach ($current_rules as $rule_text => $locations) {
			if (empty($rule_text)) continue;
			// parse the rule text to work out how to display the interface
			$bits = explode(' && ', $rule_text);

			$x = 1;
			foreach ($bits as $bit) {
				list($attr, $value) = explode('==', $bit);
				$attr  = trim($attr, '$');
				$value = trim($value, " '");

				if ($write_access) {
					require_once SQ_ATTRIBUTES_PATH.'/'.$attrs[$attr]['type'].'/'.$attrs[$attr]['type'].'.inc';
					$attr_class = 'Asset_Attribute_'.$attrs[$attr]['type'];
					$attribute = new $attr_class($attrs[$attr]['attributeid'], $value);
				}

				?>
				<tr>
					<td class="sq-backend-table-cell">the value of attribute </td>
					<td class="sq-backend-table-cell">
						<?php
							if ($write_access) {
								combo_box($prefix.'_rules['.$i.'][attrs]['.$x.']', $attributes, false, $attr);
							} else {
								echo $attributes[$attr];
							}
						?>
					</td>
					<td class="sq-backend-table-cell"> is </td>
					<td class="sq-backend-table-cell">
						<?php
							if ($write_access) {
								$attribute->paint($prefix.'_rules['.$i.'][values]['.$x.']', false);
							} else {
								echo $value;
							}
						?>
					</td>
				</tr>
				<?php

				$x++;

			}//end foreach


			if ($write_access) {
				?>
				<tr>
					<td class="sq-backend-table-cell"><b>New Attribute</b></td>
					<td class="sq-backend-table-cell" colspan="3"><?php combo_box($prefix.'_rules['.$i.'][attrs]['.$x.']', $attributes, false, ''); ?></td>
				</tr>
				<?php
			}
			

			$l = 1;
			if (!$write_access) $location_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($locations));
			foreach ($locations as $location => $on) {
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo ($l == 1) ? '<b>Location(s)</b>' : ''; ?></td>
					<td class="sq-backend-table-cell" colspan="3">
						<?php
							if ($write_access) {
								asset_finder($prefix.'_rule_location['.$i.']['.$l.']', $location, $this->location_types);
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
					<td class="sq-backend-table-cell" colspan="3"><?php asset_finder($prefix.'_rule_location['.$i.']['.$l.']', 0, $this->location_types); ?></td>
				</tr>
				<?php
			}
			?>
			<tr>
				<td class="sq-backend-table-cell" colspan="4"><hr/></td>
			</tr>
			<?php

			$i++;

		}//end foreach

		if (!empty($current_rules)) ?></table><?php

		if ($write_access) {
			echo '<b>New Rule:</b> ';
			combo_box($prefix.'_rules['.$i.'][attrs][1]', $attributes, false, '');
		}

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
		foreach ($_POST[$prefix.'_rules'] as $i => $rule) {
			$rule_text = '';
			foreach ($rule['attrs'] as $x => $attr) {
				if ($attr == '') continue;
				$value = (isset($rule['values'][$x])) ? $rule['values'][$x] : '';
				$rule_text .= '$'.$attr.'$==\''.$value.'\' && ';
			}
			if (!empty($rule_text)) {
				$locations = Array();
				if (isset($_POST[$prefix.'_rule_location'][$i])) {
					foreach ($_POST[$prefix.'_rule_location'][$i] as $loc) {
						$locid = (int) $loc['assetid'];
						if (!$locid) continue;
						$locations[$locid] = $locid;
					}
				}
				$rules[substr($rule_text, 0, -4)] = $locations;
			}
		}

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
			$status_options = get_constant_values('SQ_STATUS_');
			$options = Array();
			foreach($status_options as $status_name => $bit_value) {
				$options[$bit_value] = get_status_description($bit_value);
			}
			combo_box($prefix.'_create_status', $options, false, $asset->attr('create_status'));
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
		$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($asset->attr('create_type'), Array('name'));
		foreach ($attrs as $name) $attributes[$name] = ucwords(str_replace('_', ' ', $name));
		$current = $asset->attr('required_fields');
		
		if (empty($current)) {
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
				foreach ($current as $attr => $error_text) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $attributes[$attr]; ?></td>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access) {
								text_box($prefix.'_required_field['.$attr.']', $error_text, 50);
							} else {
								echo $error_text;
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_delete_required['.$attr.']'); ?>
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
			combo_box($prefix.'_new_required', $attributes, false, '');
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

		// construct new required fields array removing deleted fields and updating error text
		foreach ($current as $attr => $error_text) {
			if (!isset($_POST[$prefix.'_delete_required'][$attr]) || !$_POST[$prefix.'_delete_required'][$attr]) {
				$required_fields[$attr] = (isset($_POST[$prefix.'_required_field'][$attr])) ? trim($_POST[$prefix.'_required_field'][$attr]) : $current[$attr];
			}
		}

		// add new required field
		if (isset($_POST[$prefix.'_new_required']) && !empty($_POST[$prefix.'_new_required'])) {
			$new_required = $_POST[$prefix.'_new_required'];
			if (!in_array($new_required, $required_fields)) $required_fields[$new_required] = '';
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


}//end class

?>
