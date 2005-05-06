<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_builder_edit_fns.inc,v 1.30 2005/05/06 07:01:11 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Page_Asset_Builder_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.30 $
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
								'fixed'			=> 'All assets created will be linked under the following locations',
								'selectable'	=> 'When creating a new asset, the user will asked to choose a location from each of the sets below',
								'conditional'	=> 'New assets will be linked under additional locations according to the rules below',
								'additional'	=> 'If you specify some data sources to get additional location data from, and the assetids supplied by those data sources at runtime satisfy the conditions below, the created asset will be linked under those locations as well.  Lists of locations can be supplied in the format assetid;assetid;assetid or assetid-linktype;assetid-linktype;',
								'dynamic'		=> 'When creating a new asset, the frontend user will be asked to chose a location (or multiple locations), which is generated dynamically according to the assets currently available under the root folder.',
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


//--        DETAILS SCREEN        --//


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
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('create_type');
		if (empty($types)) {
			echo '<p style="color: red">No asset types have been chosen</p>';
			return false;
		}
		
		if ($asset->writeAccess('attributes')) {
			asset_type_chooser($prefix.'_create_types', true, array_keys($types));
		} else {
			$type_infos = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(null, true);
			foreach ($types as $type => $value) {
				echo $type_infos[$type]['name'].'<br />';
			}
		}

	}//end paintAssetTypes()


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
	function processAssetTypes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_create_types'])) {
			$types = Array();
			foreach ($_POST[$prefix.'_create_types'] as $type) {
				if (!empty($type)) $types[$type] = 1;
			}
			$asset->setAttrValue('create_type', $types);
			return true;
		}
		return false;

	}//end processAssetTypes()


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
			$status_options = Array(
								SQ_STATUS_ARCHIVED				=> 'Archived',
								SQ_STATUS_UNDER_CONSTRUCTION	=> 'Under Construction',
								SQ_STATUS_LIVE					=> 'Live',
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
		if (!isset($_POST[$prefix.'_create_status'])) {
			return false;
		}
		$create_status = (int)$_POST[$prefix.'_create_status'];
		return $asset->setAttrValue('create_status', $create_status);

	}//end processCreateStatus()


//--        CREATE LOCATIONS SCREEN         --//


	/**
	* Paint the interface for choosing the locations to which all new assets will be linked
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFixedCreateLocations(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$link_type_names = Array(SQ_LINK_TYPE_1=>'', SQ_LINK_TYPE_2=>'', SQ_LINK_TYPE_3=>'', SQ_LINK_NOTICE=>'');
		foreach ($link_type_names as $link_type => $null) {
			$link_type_names[$link_type] = link_type_name($link_type);
		}

		echo '<p><i><b>'.$this->location_messages['fixed'].'</b></i></p>';

		$write_access = $asset->writeAccess('attributes');
		$locations = $asset->attr('create_location');

		if (empty($locations)) {
			echo 'No fixed create locations have been specified';
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">ID</td>
					<td class="sq-backend-table-header">Name</td>
					<td class="sq-backend-table-header">Link Type</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete</td><?php
					}
					?>
				</tr>
				<?php

				foreach ($locations as $locationid => $link_type) {
					$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
					if (is_null($location)) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $location->id; ?></td>
						<td class="sq-backend-table-cell"><?php echo $location->name; ?></td>
						<td class="sq-backend-table-cell"><?php echo $link_type_names[$link_type]; ?>
						</td>
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
			echo '<p><b>Add Location:</b>&nbsp;&nbsp;&nbsp; ';
			asset_finder($prefix.'_new_create_location', 0, $this->location_types);
			echo '&nbsp;&nbsp;&nbsp;Link Type: ';
			combo_box($prefix.'_new_link_type', $link_type_names, false, '');
			echo '</p>';
		}

		return $write_access;

	}//end paintFixedCreateLocations()


	/**
	* Processes the interface for choosing the locations to which all new assets will be linked
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processFixedCreateLocations(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$old_locations = $asset->attr('create_location');
		$new_locations = $old_locations;

		// add a new create location
		if (isset($_POST[$prefix.'_new_create_location'])) {
			$new_location =(int) $_POST[$prefix.'_new_create_location']['assetid'];
			if ($new_location && !isset($new_locations[$new_location])) {
				$link_type_name = link_type_name((int)$_POST[$prefix.'_new_link_type']);
				if (!empty($link_type_name)) {
					$new_locations[$new_location] = (int)$_POST[$prefix.'_new_link_type'];
				}
			}
		}

		// delete existing create locations
		if (isset($_POST[$prefix.'_delete_location'])) {
			foreach ($_POST[$prefix.'_delete_location'] as $locationid => $on) {
				if (isset($new_locations[$locationid])) {
					unset($new_locations[$locationid]);
				}
			}
		}

		if ($old_locations != $new_locations) {
			return $asset->setAttrValue('create_location', $new_locations);
		}
		return false;

	}//end processFixedCreateLocations()


	/**
	* Paint the interface for a single set of selectable create locations
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	* @param string						$name	the name of the selectable locations set
	* @param array						&$data	details of the selectable locations set
	*
	* @return void
	* @access public
	*/
	function _paintSelectableLocationSet(&$asset, &$o, $prefix, $name, &$data)
	{
		$write_access = $asset->writeAccess('attributes');
		$code_name = str_replace(' ', '_', strtolower($name));
		?>
		<table class="sq-backend-table">
			<tr>
				<td colspan="2" class="sq-backend-table-header">
					<?php echo $name; ?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Location Options:</b>
				</td>
				<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							multiple_asset_finder($prefix.'_'.$code_name.'_location_options', array_keys($data['locations']), $this->location_types);
						} else {
							echo '<ul style="margin: 1px 15px;">';
							foreach ($data['locations'] as $assetid => $name) {
								echo '<li>'.$name.' (ID #'.$assetid.')</li>';
							}
							echo '</ul>';
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Link Type:</b>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					$link_type_names = Array(SQ_LINK_TYPE_1=>'', SQ_LINK_TYPE_2=>'', SQ_LINK_TYPE_3=>'', SQ_LINK_NOTICE=>'');
					foreach ($link_type_names as $link_type => $null) {
						$link_type_names[$link_type] = link_type_name($link_type);
					}
					if ($write_access) {
						combo_box($prefix.'_'.$code_name.'_link_type', $link_type_names, false, $data['link_type']);
					} else {
						echo $link_type_names[$data['link_type']];
					}
				?>
				</td>
			</tr>

			<tr>
				<td class="sq-backend-table-cell">
					<b>Input style</b>
				</td>

				<td class="sq-backend-table-cell">
				<?php if ($write_access) {	?>
					<select name="<?php echo $prefix.'_'.$code_name.'_input_style'; ?>">
						<option value="dropdown" <?php echo $data['input_style']=='dropdown' ? 'selected="selected"' : ''; ?>>Dropdown</option>
						<option value="radio" <?php echo $data['input_style']=='radio' ? 'selected="selected"' : ''; ?>>Radio Buttons</option>
					</select>
				<?php } else {
						echo $data['input_style'];
					  }
				?>
				</td>

			</tr>

			<tr>
				<td class="sq-backend-table-cell" >
					Rules:
				</td>
				<td class="sq-backend-table-cell">
					<table border="0" cellpadding="0" cellspacing="0" style="width: 100%">
						<tr>
							<td style="width: 48%">
								<?php if ($write_access) { ?>
									<input type="checkbox" name="<?php echo $prefix.'_'.$code_name.'_allow_empty'; ?>" id="<?php echo $prefix.'_'.$code_name.'_allow_empty'; ?>" value="1"  class="sq-form-field"  <?php echo $data['allow_empty'] ? 'checked="checked"' : ''; ?> />
								<?php } else { ?>
									<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $data['allow_empty'] ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
								<?php } ?>
								Allow No Selection &nbsp;&nbsp;&nbsp;&nbsp;
							</td>

							<td style="width: 48%;">
								<?php if ($write_access) { ?>
									<input type="checkbox" name="<?php echo $prefix.'_'.$code_name.'_allow_multiple'; ?>" id="<?php echo $prefix.'_'.$code_name.'_allow_multiple'; ?>" value="1"  class="sq-form-field"  <?php echo $data['allow_multiple'] ? 'checked="checked"' : ''; ?> />
								<?php } else { ?>
									<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $data['allow_multiple'] ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
								<?php } ?>
								Allow Multiple Selections &nbsp;&nbsp;&nbsp;&nbsp;
							</td>

							<td style="width: 4%; text-align: right">
							<?php if ($write_access) { ?>
								<input type="hidden" name="<?php echo $prefix.'_'.$code_name.'_delete' ?>" id="<?php echo $prefix.'_'.$code_name.'_delete' ?>" value="0"  class="sq-form-field" /><input type="button" value="Delete Set" onclick="if (confirm('Are you sure you want to delete this set of selectable create locations?')) { document.getElementById('<?php echo $prefix.'_'.$code_name.'_delete' ?>').value=1; form.submit(); }" />
							<?php } ?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<br />
		<?php

	}//end _paintSelectableLocationSet()


	/**
	* Paint the interface for choosing sets of create locations for the frontend user to choose from
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSelectableLocationSets(&$asset, &$o, $prefix)
	{
		echo '<p><i><b>'.$this->location_messages['selectable'].'</b></i></p>';

		$write_access = $asset->writeAccess('attributes');
		if (is_array($sets = $asset->attr('create_location_optional')) && (!empty($sets))) {
			foreach ($sets as $name => $data) {
				$this->_paintSelectableLocationSet($asset, $o, $prefix, $name, $data);
			}
		} else {
			echo 'No sets of selectable locations have been specified';
		}

		if ($write_access) {
			echo '<p><b>Add New Set: </b> ';
			text_box($prefix.'_new_set_name', '');
			echo '</p>';
		}

		return $write_access;

	}//end paintSelectableLocationSets()


	/**
	* Processes the interface for choosing sets of create locations for the frontend user to choose from
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSelectableLocationSets(&$asset, &$o, $prefix)
	{

		$sets = $asset->attr('create_location_optional');

		// see if there is a set to delete altogether
		foreach (array_keys($sets) as $name) {
			$code_name = str_replace(' ', '_', strtolower($name));
			if ($_POST[$prefix.'_'.$code_name.'_delete']) {
				unset($sets[$name]);
				$asset->setAttrValue('create_location_optional', $sets);
				$asset->saveAttributes();

				return true; // when deleting a set, nothing else should be done.
			}
		}

		// update the details for existing sets
		foreach ($sets as $name => $details) {
			$code_name = str_replace(' ', '_', strtolower($name));

			// location options
			if (is_array($new_options = array_values($_POST[$prefix.'_'.$code_name.'_location_options']))) {
				$sets[$name]['locations'] = Array();
				foreach ($new_options as $option) {
					if (($new_loc_id = $option['assetid']) && (!in_array($new_loc_id, array_keys($sets[$name]['locations'])))) {
						$sets[$name]['locations'][$new_loc_id] = ''; // we'll get the name in a sec
					}
				}
			}

			// get the names
			$asset_infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($sets[$name]['locations']));
			foreach ($asset_infos as $assetid => $asset_info) {
				$sets[$name]['locations'][$assetid] = $asset_info['name'];
			}

			// update the other options
			$sets[$name]['allow_empty'] = (int)isset($_POST[$prefix.'_'.$code_name.'_allow_empty']);
			$sets[$name]['allow_multiple'] = (int)isset($_POST[$prefix.'_'.$code_name.'_allow_multiple']);
			if (in_array(($input_style = $_POST[$prefix.'_'.$code_name.'_input_style']), Array('dropdown', 'radio'))) {
				$sets[$name]['input_style'] = $input_style;
			}
			if (in_array(($link_type = $_POST[$prefix.'_'.$code_name.'_link_type']), Array(SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE))) {
				$sets[$name]['link_type'] = $link_type;
			}
		}


		// see if there is a new set to create
		if ($_POST[$prefix.'_new_set_name']) {
			$sets[$_POST[$prefix.'_new_set_name']] = Array(
														'locations'			=> Array(),
														'allow_empty'		=> 0,
														'allow_multiple'	=> 0,
														'input_style'		=> 'dropdown',
														'link_type'			=> SQ_LINK_TYPE_1,
													 );
		}

		$asset->setAttrValue('create_location_optional', $sets);
		$asset->saveAttributes();
		$asset->_updated();

		return true;

	}//end processSelectableLocationSets()


	/**
	* Paint the interface for setting up rules that determine create locations
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintConditionalCreateLocations(&$asset, &$o, $prefix)
	{
		$current_assets = $asset->attr('create_type');
		if (empty($current_assets)) {
			echo 'You must specify the type of asset to create before create location rules can be defined';
			return false;
		}
		echo '<p><i><b>'.$this->location_messages['conditional'].'</b></i></p>';

		$write_access = $asset->writeAccess('attributes');
		$current_rules = $asset->attr('create_location_rules');

		$link_type_names = Array(SQ_LINK_TYPE_1 => '', SQ_LINK_TYPE_2 => '', SQ_LINK_TYPE_3 => '', SQ_LINK_NOTICE => '');
		foreach ($link_type_names as $lt => $null) {
			$link_type_names[$lt] = link_type_name($lt);
		}

		$comparators = Array(
						'equals'		=> 'equals',
						'starts_with'	=> 'starts with',
						'ends_with'		=> 'ends with',
						'contains'		=> 'contains',
					   );

		$create_types = $asset->attr('create_type');
		// print the rules for each create type
		foreach ($create_types as $create_type => $null) {
			echo '<p><b>Rules for Asset Type '.$create_type.'</b></p>';

			$attributes = Array();
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type, Array('name', 'attrid', 'type'));
			foreach ($attrs as $name => $data) {
				$attributes[$name] = ucwords(str_replace('_', ' ', $name));
			}
			$attributes = Array(''=>'--Choose Attribute--') + $attributes;

			$i = 1;
			// print out the current rules, if we have them
			if ((isset($current_rules[$create_type])) && (is_array($rules = $current_rules[$create_type])) && (!empty($rules))) {
				foreach ($rules as $rule) {
					$parent_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($rule['locations']);
					?>
					<table class="sq-backend-table" style="width: 90%; margin-left: 2em; margin-top: 1ex; "><tr><td class="sq-backend-table-header">
					<b>Rule <?php echo $i; ?></b>
					</td></tr>
					<tr><td class="sq-backend-table-cell">

					If all these conditions are true...<br />
					<ul>
					<?php
					for ($j = 0; $j < count($rule['attributes']); $j++) {
						if ($write_access) {
							echo '<li>';
							combo_box($prefix.'_rules['.$create_type.']['.$i.'][attributes]['.$j.']', $attributes, false, $rule['attributes'][$j]);
							echo '&nbsp;';
							combo_box($prefix.'_rules['.$create_type.']['.$i.'][comparators]['.$j.']', $comparators, false, $rule['comparators'][$j]);
							echo '&nbsp;';
							text_box($prefix.'_rules['.$create_type.']['.$i.'][values]['.$j.']', $rule['values'][$j]);
							echo '</li>';
						} else {
							echo '<li><i>'.$attributes[$rule['attributes'][$j]].'</i> '.$comparators[$rule['comparators'][$j]].' "'.$rule['values'][$j].'"</li>';
						}
					}
					if ($write_access) {
						// allow them to add a new condition
						echo '<li>';
						combo_box($prefix.'_rules['.$create_type.']['.$i.'][attributes]['.$j.']', $attributes, false, '');
						echo '&nbsp;';
						combo_box($prefix.'_rules['.$create_type.']['.$i.'][comparators]['.$j.']', $comparators, false, '');
						echo '&nbsp;';
						text_box($prefix.'_rules['.$create_type.']['.$i.'][values]['.$j.']', '');
						echo '</li>';
					}
					?>
					</ul>

					...link the asset to all these locations:<br />
					<ul style="margin-bottom: 3px;">
					<?php
					$j = 0;
					while ($j < count($rule['locations'])) {
						if ($write_access) {
							echo '<li>';
							asset_finder($prefix.'_rules['.$create_type.']['.$i.'][locations]['.$j.']', $rule['locations'][$j], $this->location_types);
							echo ' Link Type: ';
							combo_box($prefix.'_rules['.$create_type.']['.$i.'][link_types]['.$j.']', $link_type_names, false, $rule['link_types'][$j]);
							echo '</li>';
						} else {
							echo '<li>'.$parent_names[$rule['locations'][$j]]['name'].' (ID '.$rule['locations'][$j].') ('.$link_type_names[$rule['link_types'][$j]].')</li>';
						}
						$j++;
					}
					if ($write_access) {
						// allow them to add a location
						echo '<li>';
						asset_finder($prefix.'_rules['.$create_type.']['.$i.'][locations]['.$j.']', 0, $this->location_types);
						echo ' Link Type: ';
						combo_box($prefix.'_rules['.$create_type.']['.$i.'][link_types]['.$j.']', $link_type_names, false, '');
						echo '</li>';
					}
					?>
					</ul>
					<?php if ($write_access) { ?>
						<input type="hidden" name="<?php echo $prefix.'_rules['.$create_type.']['.$i.'][delete]'; ?>" id="<?php echo $prefix.'_rules_'.$create_type.'_'.$i.'_delete'; ?>" value="0" />
						<input style="display: inline; float: right; clear: none; margin: 0px; " type="button" onclick="if (confirm('Are you sure you want to delete this rule?')) { document.getElementById('<?php echo $prefix.'_rules_'.$create_type.'_'.$i.'_delete'; ?>').value=1; form.submit(); }" value="Delete Rule" />
					<?php } ?>
					</td></tr>
					</table>

					<?php
					$i++;
				}
			} else {
				?><span style="margin-left: 2em; ">(no rules defined)<br /></span><?php
			}

			// allow the user to add a rule
			if ($write_access) {
				?>
				<table class="sq-backend-table" style="width: 90%; margin-left: 2em; margin-top: 1ex; ">
					<tr>
						<td class="sq-backend-table-header"><a id="<?php echo $prefix.'_'.$create_type; ?>_new_rule_link" href="#" onclick="document.getElementById('<?php echo $prefix.'_'.$create_type; ?>_new_rule_link').style.display='none'; document.getElementById('<?php echo $prefix.'_'.$create_type; ?>_new_rule_header').style.display='inline'; document.getElementById('<?php echo $prefix.'_'.$create_type; ?>_new_rule_body').style.display='block'; return false;"><b>New Rule</b></a><span style="display: none" id="<?php echo $prefix.'_'.$create_type; ?>_new_rule_header" style="display: none" ><b>New Rule</b></span></td>
					</tr>

					<tr>
						<td id="<?php echo $prefix.'_'.$create_type; ?>_new_rule_body" class="sq-backend-table-cell" style="display: none" >
							If all these conditions are true...<br />
							<ul>
				<?php
				for ($j = 0; $j < 3; $j++) {
					echo '<li>';
					combo_box($prefix.'_rules['.$create_type.']['.$i.'][attributes]['.$j.']', $attributes, false, '');
					echo '&nbsp;';
					combo_box($prefix.'_rules['.$create_type.']['.$i.'][comparators]['.$j.']', $comparators, false, '');
					echo '&nbsp;';
					text_box($prefix.'_rules['.$create_type.']['.$i.'][values]['.$j.']', '');
				}
				?>
							</ul>

							Link the asset to all these locations:<br />
							<ul>
								<li>
				<?php
					asset_finder($prefix.'_rules['.$create_type.']['.$i.'][locations][0]', 0, $this->location_types); echo ' Link Type: ';
					combo_box($prefix.'_rules['.$create_type.']['.$i.'][link_types][0]', $link_type_names, false, '');
				?>
								</li>
							</ul>
						</td>
					</tr>
				</table>
				<?php
			}

		}

		return true;

	}//end paintConditionalCreateLocations()


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
	function processConditionalCreateLocations(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rules'])) return false;
		$rules = $_POST[$prefix.'_rules'];

		// remove empty elements from our array
		foreach ($rules as $create_type => $type_rules) {
			foreach ($type_rules as $i => $rule_data) {
				foreach ($rule_data['attributes'] as $index => $attribute) {
					if ((empty($attribute)) || empty($rule_data['values'][$index])) {
						unset($rules[$create_type][$i]['attributes'][$index]);
						unset($rules[$create_type][$i]['comparators'][$index]);
						unset($rules[$create_type][$i]['values'][$index]);
					} else {
						// make it lowercase for comparison later
						$rules[$create_type][$i]['values'][$index] = strtolower($rules[$create_type][$i]['values'][$index]);
					}
				}
				foreach ($rule_data['locations'] as $index => $location) {
					if (empty($location['assetid'])) {
						unset($rules[$create_type][$i]['locations'][$index]);
						unset($rules[$create_type][$i]['link_types'][$index]);
					} else {
						$rules[$create_type][$i]['locations'][$index] = $location['assetid'];
					}
				}
				// remove the rule altogether if it's totally empty or the delete button was clicked
				if ((isset($rule_data['delete']) && $rule_data['delete']) || (strlen(implode('', $rule_data['attributes'])) == 0)) {
					unset($rules[$create_type][$i]);
				} else {
					unset($rules[$create_type][$i]['delete']);
				}

			}//end foreach ($type_rules)
		}//end foreach (create_type)

		return $asset->setAttrValue('create_location_rules', $rules);

	}//end processConditionalCreateLocations()


	/**
	* Paints info about additional create locations
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAdditionalLocationsInfo(&$asset, &$o, $prefix)
	{
		echo '<p><i><b>'.$this->location_messages['additional'].'</b></i></p>';
		return false;

	}//end paintAdditionalLocationsInfo()


	/**
	* Nothing
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAdditionalLocationsInfo(&$asset, &$o, $prefix)
	{
		return true;

	}//end processAdditionalLocationsInfo()


	/**
	* Paints the interface for saying where additional create locations can come from
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAdditionalLocationParents(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'_additional_location_parents', $asset->attr('additional_location_parents'), $this->location_types);
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset->attr('additional_location_parents'));
			echo '<ul style="margin: 1px 15px;">';
			foreach ($infos as $assetid => $info) {
				echo '<li>'.$info['name'].' (ID #'.$assetid.')</li>';
			}
			echo '</ul>';
		}
		return true;

	}//end paintAdditionalLocationParents()


	/**
	* Processes the interface for saying where additional create locations can come from
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAdditionalLocationParents(&$asset, &$o, $prefix)
	{
		$new_parents = Array();
		if (isset($_POST[$prefix.'_additional_location_parents']) && is_array($_POST[$prefix.'_additional_location_parents'])) {
			foreach ($_POST[$prefix.'_additional_location_parents'] as $parent) {
				if ($parent['assetid']) {
					$new_parents[] = $parent['assetid'];
				}
			}
		}
		$asset->setAttrValue('additional_location_parents', $new_parents);

	}//end processAdditionalLocationParents()


	/**
	* Paints the interface for saying what types of asset additional locations can be
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAdditionalLocationTypes(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo '<ul style="margin: 1px 15px;">';
			foreach ($asset->attr('additional_location_types') as $type) {
				echo '<li>'.ucwords(str_replace('_', ' ', $type)).'</li>';
			}
			echo '</ul>';
		} else {
			asset_type_chooser($prefix.'_additional_location_types', true, $asset->attr('additional_location_types'));
		}
		return true;

	}//end paintAdditionalLocationTypes()


	/**
	* Processes the interface for saying what types of asset additional locations can be
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAdditionalLocationTypes(&$asset, &$o, $prefix)
	{
		$new_types = Array();
		if (isset($_POST[$prefix.'_additional_location_types']) && is_array($_POST[$prefix.'_additional_location_types'])) {
			foreach ($_POST[$prefix.'_additional_location_types'] as $type) {
				if ($type) $new_types[] = $type;
			}
		}

		return $asset->setAttrValue('additional_location_types', array_unique($new_types));

	}//end processAdditionalLocationTypes()


	/**
	* Paint the interface for choosing sets of dynamic create locations for the frontend user to choose from
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDynamicCreateLocations(&$asset, &$o, $prefix)
	{
		echo '<p><i><b>'.$this->location_messages['dynamic'].'</b></i></p>';

		$write_access = $asset->writeAccess('attributes');
		if (is_array($sets = $asset->attr('create_location_dynamic')) && (!empty($sets))) {
			foreach ($sets as $name => $data) {
				$this->_paintDynamicCreateLocationSet($asset, $o, $prefix, $name, $data);
			}
		} else {
			echo 'No sets of dynamic locations have been specified';
		}

		if ($write_access) {
			echo '<p><b>Add New Set: </b> ';
			text_box($prefix.'_new_dynamic_set_name', '');
			echo '</p>';
		}

		return $write_access;

		$o->openRaw();
		$array = Array();
		$this->_paintDynamicCreateLocation($asset, $o, $prefix, 'My Sample Set', $array);
		$o->closeRaw();

		$o->openRaw();
		?>
			<ul>
				<li>Main Options:
					<ul>
						<li>Root node</li>
						<li>Depth</li>
						<li>Asset types</li>
					</ul>
				</li>
				<li>Style / Interface Options:
					<ul>
						<li>Selection style</li>
						<li>Link type</li>
						<li>Allow no selection</li>
						<li>Allow multiple selection</li>
					</ul>
				</li>
			</ul>
		<?php
		$o->closeRaw();
		return true;

	}//end paintDynamicCreateLocations()


	/**
	* Handles the painting of a Dynamic Create Locations set box
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	* @param string						$name	the name of the selectable locations set
	* @param array						&$data	details of the selectable locations set
	*
	* @access private
	* @return boolean
	*/
	function _paintDynamicCreateLocationSet(&$asset, &$o, $prefix, $name, &$data)
	{
		$write_access = $asset->writeAccess('attributes');
		$code_name    = 'dynamic_location_'.str_replace(' ', '_', strtolower($name));
		?>
		<table class="sq-backend-table">
			<tr>
				<td colspan="2" class="sq-backend-table-header">
					<?php echo $name; ?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Root Node:</b>
				</td>
				<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							asset_finder($prefix.'_'.$code_name.'_root', array_get_index($data, 'root', 0), $this->location_types);
						} else {
							$rootid = array_get_index($data, 'root', 0);
							if ($rootid > 0 && $GLOBALS['SQ_SYSTEM']->am->assetExists($rootid)) {
								$root_name = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($rootid), Array(), false, 'name');
								echo '<p>'.$root_name[$rootid].' (ID #'.$rootid.')</p>';
							} else {
								echo '<p>None selected</p>';
							}
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Depth:</b>
				</td>
				<td class="sq-backend-table-cell">
					<?php
						if ($write_access) {
							int_text_box($prefix.'_'.$code_name.'_depth', array_get_index($data, 'depth', 0), false, 3);
							echo ' Levels (0 is unlimited depth)';
						} else {
							$levels = array_get_index($data, 'depth', 0);
							if ($levels == 0) $levels = 'Infinite';
							echo '<p>'.$levels.' Levels</p>';
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Restrict to these Asset Types:</b>
				</td>
				<td class="sq-backend-table-cell">
					<?php
						if (isset($data['types'])) {
							$selected_asset_types = $data['types'];
						} else {
							$selected_asset_types = Array(
														'type_code'	=> Array(),
														'inherit'	=> Array(),
													);
						}

						if ($write_access) {
							asset_type_chooser($prefix.'_'.$code_name.'_types', true, $selected_asset_types, null, false, true);
						} else {
							foreach ($selected_asset_types['type_code'] as $index => $type) {
								if (empty($type)) {
									unset($selected_asset_types['type_code'][$index]);
								} else {
									$selected_asset_types['type_code'][$index] = &$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
								}
							}
							echo implode(",<br />\n", $selected_asset_types['type_code']);
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Include links of the following <br/>type in the locations list:</b>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					// the link types that dynamic create locations concern themselves with
					$link_types = Array(
									SQ_LINK_TYPE_1	=> 'TYPE_1',
									SQ_LINK_TYPE_2	=> 'TYPE_2',
								  );
					if ($write_access) {
						foreach ($link_types as $link_type => $link_name) {
							if (isset($data['filter_link_type'][$link_type])) {
								$selected = true;
							} else {
								$selected = false;
							}
							check_box($prefix.'_'.$code_name.'_filter_link_type['.$link_type.']', '1', $selected);
							?>&nbsp;<?php echo $link_name; ?><br />
							<?php
						}
					} else {
						$display_link_types = Array();
						foreach (array_get_index($data, 'filter_link_type', Array()) as $filter_link_type => $filter_link_type_data) {
							if (isset($link_types[$filter_link_type])) {
								$display_link_types[] = $link_types[$filter_link_type];
							}
						}
						if (empty($display_link_types)) {
							echo 'No exclusions.';
						} else {
							echo implode(",<br />\n", $display_link_types);
						}
					}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<b>Link Type of Created Asset:</b>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					$link_type_names = get_link_type_names();
					$selected_link_type = array_get_index($data, 'link_type', SQ_LINK_TYPE_1);
					if ($write_access) {
						combo_box($prefix.'_'.$code_name.'_link_type', $link_type_names, false, $selected_link_type);
					} else {
						echo $link_type_names[$data['link_type']];
					}
				?>
				</td>
			</tr>

			<tr>
				<td class="sq-backend-table-cell">
					<b>Input style</b>
				</td>

				<td class="sq-backend-table-cell">
				<?php
				$input_style        = array_get_index($data, 'input_style', 'None Selected');
				$display_structured = array_get_index($data, 'display_structured', 0);
				$options = Array(
							'dropdown'	=> 'Dropdown',
							'radio'		=> 'Radio Buttons',
						   );
				if ($write_access) {
					?><select name="<?php echo $prefix.'_'.$code_name.'_input_style'; ?>"><?php
					foreach ($options as $opt_value => $opt_text) {
						echo '<option value="'.$opt_value.'" '.($input_style == $opt_value ? 'selected="selected"' : '').'>'.$opt_text.'</option>'."\n";
					}
					?></select>&nbsp;<?php

					// display structured?
					check_box($prefix.'_'.$code_name.'_display_structured', '1', ($display_structured == 1 ? true : false));
					echo "&nbsp;Display Assets as a Tree\n";

				} else {
					echo '<p>'.$options[$input_style].' ('.($display_structured ? 'Display Assets as a Tree' : 'Display Assets as a Normal List').')</p>';
				}
				?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" >
					Rules:
				</td>
				<td class="sq-backend-table-cell">
					<table border="0" cellpadding="0" cellspacing="0" style="width: 100%">
						<tr>
							<td style="width: 48%">
								<?php
								$selected_allow_empty = array_get_index($data, 'allow_empty', false);
								if ($write_access) {
									?>
									<input type="checkbox" name="<?php echo $prefix.'_'.$code_name.'_allow_empty'; ?>" id="<?php echo $prefix.'_'.$code_name.'_allow_empty'; ?>" value="1"  class="sq-form-field"  <?php echo $selected_allow_empty ? 'checked="checked"' : ''; ?> />
									<?php
								} else {
									?>
									<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $selected_allow_empty ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
									<?php
								}
								?> Allow No Selection &nbsp;&nbsp;&nbsp;&nbsp;
							</td>

							<td style="width: 48%;">
								<?php
								$selected_allow_multiple = array_get_index($data, 'allow_multiple', false);
								if ($write_access) {
									?>
									<input type="checkbox" name="<?php echo $prefix.'_'.$code_name.'_allow_multiple'; ?>" id="<?php echo $prefix.'_'.$code_name.'_allow_multiple'; ?>" value="1"  class="sq-form-field"  <?php echo $selected_allow_multiple ? 'checked="checked"' : ''; ?> />
									<?php
								} else {
									?>
									<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $selected_allow_multiple ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
									<?php
								}
								?> Allow Multiple Selections &nbsp;&nbsp;&nbsp;&nbsp;
							</td>

							<td style="width: 4%; text-align: right">
							<?php if ($write_access) { ?>
								<input type="hidden" name="<?php echo $prefix.'_'.$code_name.'_delete' ?>" id="<?php echo $prefix.'_'.$code_name.'_delete' ?>" value="0"  class="sq-form-field" /><input type="button" value="Delete Set" onclick="if (confirm('Are you sure you want to delete this set of dynamic create locations?')) { document.getElementById('<?php echo $prefix.'_'.$code_name.'_delete' ?>').value=1; form.submit(); }" />
							<?php } ?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<br />
		<?php

	}//end _paintDynamicCreateLocationSet()


	/**
	* Processes the interface for choosing sets of dynamic create locations for the frontend user to choose from
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDynamicCreateLocations(&$asset, &$o, $prefix)
	{
		/** Settings structure:
		* $sets = Array(
		*			'set_name' => Array(
		*							'root'			=> location_id, 	// from asset_finder
		*							'types'			=> Array(
		*												0 => 'page',
		*											   ),
		*							'depth'			=> 0, 				// default to infinite levels
		*							'input_style'	=> 'dropdown',		// or 'radio', 'checkbox', 'multiselect'
		*							'allow_empty'	=> false, 			// boolean
		*							'link_type'		=> SQ_LINK_TYPE_1,
		*						  )
		*		  )
		*/

		$sets = $asset->attr('create_location_dynamic');

		// see if there is a set to delete altogether
		foreach (array_keys($sets) as $name) {
			$code_name = 'dynamic_location_'.str_replace(' ', '_', strtolower($name));

			if ($_POST[$prefix.'_'.$code_name.'_delete']) {
				unset($sets[$name]);
				$asset->setAttrValue('create_location_dynamic', $sets);
				$asset->saveAttributes();

				// when deleting a set, nothing else should be done.
				return true;
			}
		}

		// update the details for existing sets
		foreach ($sets as $name => $details) {
			$code_name = 'dynamic_location_'.str_replace(' ', '_', strtolower($name));

			// root
			if (isset($_POST[$prefix.'_'.$code_name.'_root']['assetid'])) {
				$sets[$name]['root'] = $_POST[$prefix.'_'.$code_name.'_root']['assetid'];
			}

			// depth
			if (isset($_POST[$prefix.'_'.$code_name.'_depth'])) {
				$sets[$name]['depth'] = $_POST[$prefix.'_'.$code_name.'_depth'];
			}

			// backend display
			if (isset($_POST[$prefix.'_'.$code_name.'_types'])) {
				$sets[$name]['types'] = $_POST[$prefix.'_'.$code_name.'_types'];
			}

			// filter link_type
			if (isset($_POST[$prefix.'_'.$code_name.'_filter_link_type'])) {
				$sets[$name]['filter_link_type'] = $_POST[$prefix.'_'.$code_name.'_filter_link_type'];
			} else {
				$sets[$name]['filter_link_type'] = Array();
			}

			// input style
			if (in_array(($input_style = $_POST[$prefix.'_'.$code_name.'_input_style']), Array('dropdown', 'radio'))) {
				$sets[$name]['input_style'] = $input_style;
			}

			// display structured?
			if (isset($_POST[$prefix.'_'.$code_name.'_display_structured'])) {
				$sets[$name]['display_structured'] = $_POST[$prefix.'_'.$code_name.'_display_structured'];
			} else {
				$sets[$name]['display_structured'] = 0;
			}

			// link_type
			if (isset($_POST[$prefix.'_'.$code_name.'_link_type'])) {
				$link_type_list = get_link_type_names();
				if (isset($link_type_list[$_POST[$prefix.'_'.$code_name.'_link_type']])) {
					$sets[$name]['link_type'] = (int) $_POST[$prefix.'_'.$code_name.'_link_type'];
				}
			}

			// update the other options
			$sets[$name]['allow_empty']    = (int) isset($_POST[$prefix.'_'.$code_name.'_allow_empty']);
			$sets[$name]['allow_multiple'] = (int) isset($_POST[$prefix.'_'.$code_name.'_allow_multiple']);
		}


		// see if there is a new set to create
		if ($_POST[$prefix.'_new_dynamic_set_name']) {
			$sets[$_POST[$prefix.'_new_dynamic_set_name']] = Array(
																'root'				=> 0,
																'depth'				=> 0,
																'allow_empty'		=> 0,
																'allow_multiple'	=> 0,
																'input_style'		=> 'dropdown',
																'link_type'			=> SQ_LINK_TYPE_1,
																'filter_link_type'	=> Array(
																						SQ_LINK_TYPE_1	=> '1',
																						SQ_LINK_TYPE_2	=> '1',
																					   ),
																'types'				=> Array(
																						'type_code'	=> Array(),
																						'inherit'	=> Array(),
																					   ),
																'diplay_structured'	=> 0,
															 );
		}

		$asset->setAttrValue('create_location_dynamic', $sets);
		$asset->saveAttributes();
		$asset->_updated();

		return true;

	}//end processDynamicCreateLocations()


//--         REQUIRED FIELDS SCREEN          --//


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

		foreach (array_keys($asset->attr('create_type')) as $create_type) {

			$attributes = Array();
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type, Array('name'));
			foreach ($attrs as $name) {
				$attributes[$name] = ucwords(str_replace('_', ' ', $name));
			}

			if (!isset($current[$create_type])) {
				$current[$create_type] = Array();
			}
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

				$new_required = Array('' => '-- select attribute --');
				foreach ($attributes as $attr => $name) {
					if (isset($current[$create_type][$attr])) continue;
					$new_required[$attr] = $name;
				}

				echo '<br/><b>New required attribute ? </b>';
				combo_box($type_prefix.'_new_required', $new_required, false, '');
			}

			$o->closeRaw();
			$o->closeSection();

		}//end for

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

			if (!isset($current[$create_type])) {
				$current[$create_type] = Array();
			}
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
				if (!isset($required_fields[$create_type][$new_required])) {
					$required_fields[$create_type][$new_required] = '';
				}
			}
		}

		return $asset->setAttrValue('required_fields', $required_fields);

	}//end processRequiredFields()


//--         OTHER STUFF          --//


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

	}//end definesMultipleAssetTypes()


}//end class

?>
