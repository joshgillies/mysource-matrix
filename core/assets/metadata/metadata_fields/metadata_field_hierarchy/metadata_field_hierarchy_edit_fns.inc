<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_hierarchy_edit_fns.inc,v 1.26 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Hierarchy_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.26 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Hierarchy_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for filling in metadata values
	*
	* Returns true if a submitable interface has been printed
	*
	* @param Metadata_Field_Hierarchy	$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter			$o				the outputter class
	* @param string						$value_str		the current value for this field in the owning asset (NOT $asset)
	*													NULL means that there is no current value set
	* @param boolean					$write_access	whether to allow editing of this field
	*													(typically depends on write access to owner and edit of a field)
	* @param boolean					$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option=TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$value = '';
		$value_components = $asset->attr('value_components');

		$is_default = FALSE;
		if (is_null($value_str)) {
			$value_str = $asset->getDefaultValue();
			$is_default = TRUE;
		}

		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		if ($asset->attr('multiple')) {
			// if there is no value we need a blank array...not an array with one element (a blank string)
			if ($value == '') {
				$value = Array();
			} else {
				$value = explode('; ', $value);
			}
		}

		if ($enable_default) {
			echo '<span id="'.$prefix.'_field">';
		}

		$display_type = $asset->attr('display_type');
		$hierarchy = $asset->getHierarchyAttribute();
		$hierarchy_options = $hierarchy->_params['options'];
		if ($write_access) {

			echo '<span class="sq-metadata-settings-wrapper">';
				if ($enable_default) {
					echo '<span class="sq-metadata-default-wrapper">';
					// Work out which options we have to set if we have to
					$i = 0;
					$default_keys = explode('; ', $asset->attr('default'));
					$default_selected = Array();

					$default_key_values = Array();
					$default_values = Array();
					$non_default_keys = Array();
					foreach ($hierarchy_options as $key => $hierarchy_option) {
						if (in_array($key, $default_keys)) {
							$default_key_values[] = "'".addslashes($key)."'";
							$default_values[] = "'".addslashes($hierarchy_option)."'";
						} else {
							$non_default_keys[] = "'".addslashes($key)."'";
						}
					}
					$default_keys_expr = '['.implode(', ', $default_key_values).']';
					$default_values_expr = '['.implode(', ', $default_values).']';
					$non_default_keys_expr = '['.implode(', ', $non_default_keys).']';
					check_box($prefix.'_default', '1', $is_default, "handleDefaultClickHierarchy(this, '$prefix', $default_keys_expr, $default_values_expr, $non_default_keys_expr, $display_type); ");
					label(translate('use_default'), $prefix.'_default');
					$o->addJsInclude(sq_web_path('data').'/asset_types/metadata_field_hierarchy/js/metadata_field_hierarchy.js');
					echo '</span>';
				}

				if ($print_cascade_values_option) {
					echo '<span class="sq-metadata-cascade-wrapper">';
						check_box($prefix.'_cascade_value', '1', FALSE);
						label(translate('cascade_value'), $prefix.'_cascade_value');
					echo '</span>';
				}
			echo '</span>';

			if (!$display_type) {
				// flat view
				$hierarchy->value = $value;
				$hierarchy_sort = $hierarchy->_params['sort'];
				$hierarchy_structure = $hierarchy->_params['structure'];

				$hier_sort = (isset($hierarchy_sort) && $hierarchy_sort>=0) ? $hierarchy_sort : '0';
				$hier_structure = (isset($hierarchy_structure) && $hierarchy_structure) ? $hierarchy_structure : FALSE;

				if ($enable_default) {
					$hierarchy->_edit_params['extras'] = 'onclick="document.getElementById(\''.$prefix.'_default\').checked = false;"';
					if ($is_default) {
						$hierarchy->_edit_params['extras'] .= ' disabled="disabled"';
					}
					echo '<span id="'.$prefix.'_field">';
				}

				// Sort the hierarchy if selected
				if ($hier_sort) {
					$options = $hierarchy->_params['options'];
					$options = $this->_sortHierarchyOptions($options, $hier_sort, $hier_structure);

					$hierarchy->_params['options'] = $options;
				}
				$hierarchy->paint($prefix, FALSE);
			} else {
				// drill-down view
				$level_titles = $asset->attr('level_titles');
				$hierarchy_options = $this->_sortHierarchyOptions($hierarchy_options, 1, FALSE);
				if (!is_array($value)) {
					$value = explode('; ', $value);
				}

				// work out whether our fields are disabled or not based on whether we are currently showing the 'use default' checkbox
				if (!$enable_default) {
					$show_default_box = FALSE;
				} else {
					if ($is_default) {
						$show_default_box = TRUE;
					} else {
						$show_default_box = FALSE;
					}
				}
				$this->paintDrillDownView($hierarchy_options, $value, $level_titles, $show_default_box, explode('; ', $asset->attr('default')), $prefix);
			}
		} else {
			echo '<span class="sq-metadata-settings-wrapper">';
				if ($is_default) {
					echo ' <em>('.translate('using_default_value').')</em>';
				} else{

				}	
			echo '</span>';
			
			if (!((is_array($value) && empty($value)) || (is_string($value) && $value == ''))) {
				if (!is_array($value)) $value = Array($value);
				$value_output = '';
				foreach ($value as $option) {
					if (isset($hierarchy_options[$option])) {
						$value_output .= $hierarchy_options[$option].'; ';
					}
				}
				echo trim($value_output, '; ');
			}
		}

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Hierarchy	$asset					the asset whose interface we are processing
	* @param string						&$new_value_str			a string by reference that we can store the new values in
	* @param bool     					&$field_cascade_value	Whether value updates to this field should cascade
	*                                       					to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Hierarchy $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;
		
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			// Default is not checked, so process our hierarchy attribute
			$hierarchy = $asset->getHierarchyAttribute();

			if (isset($_REQUEST[$prefix])) {
				$value = $_REQUEST[$prefix];
			} else {
				return $processed;
			}

			if (is_array($value)) $value = implode('; ', $value);
			$value = Metadata_Field::encodeValueString(trim($value), Array());
			$hoptions = $hierarchy->_params['options'];

			$hierarchy->value = $value;

			$new_value_str = $value;
			$processed = TRUE;

		}//end if

		if (isset($_POST[$prefix.'_cascade_value'])) {
			$field_cascade_value = TRUE;
			$processed = TRUE;
		}

		return $processed;

	}//end processInlineValueInterface()


	/**
	* Paint the interface for hierarchy
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintHierarchy(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$hierarchy_options = $asset->getContextedHierarchyOptions();
		$hierarchy_sort = $asset->attr('hier_sort');

		$is_contextable = (boolean)$asset->attr('is_contextable');

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$is_default_context = ($contextid === 0) ? TRUE : FALSE;

		if ($is_contextable === TRUE) {
			$hierarchy_options = $asset->getContextedHierarchyOptions();
		} else {
			$hierarchy_options = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('hierarchy_options', 'metadata_field_hierarchy', Array($asset->id), 0);
			$hierarchy_options = unserialize($hierarchy_options[$asset->id]);
		}

		if (!empty($hierarchy_options)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('option_key'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('option_value'); ?></td>
					<?php
					if (($write_access  === TRUE) && ($is_default_context === TRUE)) {
						?><td class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td><?php
					}
					?>
				</tr>
				<?php
				$i = 0;
				foreach ($hierarchy_options as $option_key => $option_value) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access  === TRUE) {
								if (($is_default_context === TRUE) || ($is_contextable === FALSE)) {
									text_box($prefix.'_options[key]['.$i.']', $option_key, 20);
								} else {
									echo $option_key;
									hidden_field($prefix.'_options[key]['.$i.']', $option_key);
								}
							} else {
								echo $option_key;
							}
							?>
						</td>
						<td class="sq-backend-table-cell" align="left">
							<?php
							if ($write_access) {
								text_box($prefix.'_options[val]['.$i.']', $option_value, 20);
							} else {
								echo $option_value;
							}
							?>
						</td>
						<?php
						if (($write_access  === TRUE) && (($is_default_context === TRUE) || ($is_contextable === FALSE))) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_options[del]['.$i.']', $option_key); ?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
					$i++;
				}//end foreach
				?>
			</table>
			<?php

		} else {
			echo translate('no_hierarchy_box_options');
		}//end if empty

		// now adding two fields for new entry (key => value)
		if (($write_access  === TRUE) && (($is_default_context === TRUE) || ($is_contextable === FALSE))) {
			$o->openField(translate('new_option'));
				echo '<b>'.translate('key').'</b> ';
				text_box($prefix.'_new_key','');
				echo ' <b>'.translate('value').'</b> ';
				text_box($prefix.'_new_val','');
			$o->closeField();
		}

		// Some values we need
		$sort = $asset->attr('hier_sort');
		$format = $asset->attr('hier_structure');

		if (!empty($hierarchy_options)) {
			// displaying default value if any values were actually saved in the array
			$o->openField(translate('default_value'));
				$default_hier = $asset->getHierarchyAttribute();
				$default_hier->paint($prefix.'_default', !$write_access);
			$o->closeField();
		}

		// Being user friendly show a quick preview of what it looks like
		$o->openField(translate('preview'));
			if (!empty($hierarchy_options)) {

				$display_type = $asset->attr('display_type');
				if (!$display_type) {

					// perform any sorting and indentation
					if ($sort > 0) {
						$preview_options = $this->_sortHierarchyOptions($hierarchy_options, $sort, $format);
						if (isset($preview_options) && !is_null($preview_options)) {
							foreach ($preview_options as $key => $value) {
								$preview_options[$key] = substr($value, 0, 100);
							}
						}
					} else {
						$preview_options = $hierarchy_options;
					}

					// Only preview don't show full length
					if (isset($preview_options) && !is_null($preview_options)) {
						foreach ($preview_options as $key => $value) {
							$preview_options[$key] = substr($value, 0, 100);
						}
					}

					// flat view
					$preview_hierarchy = $asset->getHierarchyAttribute();
					$preview_hierarchy->_params['options'] = $preview_options;
					$preview_hierarchy->value = '';
					$preview_hierarchy->paint('some_name_not_used', FALSE);
				} else {
					$preview_options = $this->_sortHierarchyOptions($hierarchy_options, 1, FALSE);
					$level_titles = $asset->attr('level_titles');
					$this->paintDrillDownView($preview_options, Array(), $level_titles, FALSE, Array(), $prefix);
				}
			} else {
				echo translate('no_hierarchy_box_options');
			}//end if
		$o->closeField();

		return TRUE;

	}//end paintHierarchy()


	/**
	* Recursively paint the drill-down view of the hierarchy
	*
	* @param array		$options			The array options
	* @param array		$receptacle_keys	The keys of the receptacle box
	* @param array		$level_titles		The titles for each level
	* @param boolean	$is_default			Whether the 'use default' button is selected
	* @param array		$default_keys		The default keys
	* @param string		$prefix				Prefix of the asset
	*
	* @return boolean
	* @access public
	*/
	public function paintDrillDownView($options, $receptacle_keys, $level_titles, $is_default, $default_keys, $prefix)
	{
		echo '<table width="100px">';

		$levels = Array();
		$receptacle_options = Array();
		foreach ($options as $key => $value) {
			// hierarchy may not be linear so calculate how many select boxes we will need this way
			if (!in_array(strlen($key), $levels)) {
				$levels[] = strlen($key);
			}

			// get formatted receptacle contents
			if ($is_default) {
				// default values
				if (in_array($key, $default_keys)) {
					$receptacle_options[$key] = $key.'. '.$value;
				}
			} else {
				foreach ($receptacle_keys as $receptacle_key) {
					if ($receptacle_key === $key) {
						$receptacle_options[$key] = $key.'. '.$value;
					}
				}
			}
		}

		$the_options = Array();
		$count_levels = count($levels);
		$field_name = '';
		$receptacle_field_name = $prefix;
		for ($i=0; $i<$count_levels; $i++) {
			$title = isset($level_titles[$levels[$i]]) ? $level_titles[$levels[$i]] : '';
			echo '<tr><td>';
			echo empty($title) ? '' : $title;
			echo '</td><td>';

			foreach ($options as $key => $value) {
				$field_name = $prefix.'_stationary_drill_'.$i;

				// print each level's combo boxes and filter at the same time
				if (strlen($key) === $levels[$i]) {
					// first level is easy
					if ($i === 0) {
						$the_options[$key] = $value;
					} else {
						// fetch the children for the previous list
						// the currently selected value is the first in the array
						$keys = array_keys($the_options);

						// used for the initial painting of the drill-down
						if (isset($keys[0])) {
							$the_options = $this->_getImmediateChildren($options, $keys[0]);
						} else {
							// hierarchy may not be linear so deal with it
							// look for the next level if this is the case
							foreach ($options as $search_key => $search_value) {
								// the options must be sorted for this to work
								if (strlen($search_key) === $levels[$i-1]) {
									$found_key = $search_key;
									break;
								}
							}

							$the_options = $this->_getImmediateChildren($options, $found_key);
						}

						break;
					}

				}
			}

			// deepest level box allows multiple selections
			// also give the deepest level box a maximum height
			$extras = $is_default ? 'disabled="disabled"; onChange="swapOptions_for_'.$prefix.'(this)";' : 'onChange="swapOptions_for_'.$prefix.'(this)";';
			echo combo_box($field_name, $the_options, ($i === $count_levels-1) ? TRUE : FALSE, '', ($i === $count_levels-1) ? 5 : '', $extras);

			// print the 'add' button
			// $field_name contains the name of the last box
			$add_on_click = 'addToReceptacle_for_'.$prefix.'(\''.$field_name.'\')';
			// new column for 'add' buttons
			echo '</td><td valign="top">';
			echo normal_button($prefix.'_drill_down_button', translate('add'), $add_on_click);

			echo '</td></tr>';
		}

		// print the receptacle box
		echo '<tr><td></td><td>';
		$extras = $is_default ? 'disabled="disabled"; style="width:300px";' : 'style="width:300px";';
		echo combo_box($receptacle_field_name, $receptacle_options, TRUE, '', 5, $extras);

		// print the remove buttons
		echo '<br />';
		$remove_on_click = 'removeFromReceptacle_for_'.$prefix.'()';
		echo normal_button($prefix.'_drill_down_button', translate('remove'), $remove_on_click);
		$remove_all_on_click = 'removeAllFromReceptacle_for_'.$prefix.'()';
		echo normal_button($prefix.'_drill_down_button', translate('remove_all'), $remove_all_on_click);

		// close the receptacle row
		echo '</td></tr>';
		echo '</table>';

		echo $this->_printJSObjects($options, $prefix);

		?>
			<script type="text/javascript">
			var prefix_for_<?php echo $prefix ?> = '<?php echo $prefix; ?>';
			// select all options in the receptacle box otherwise on commit we will lose our values
			var receptacleName_for_<?php echo $prefix ?> = '<?php echo $receptacle_field_name; ?>';
			var receptacle_for_<?php echo $prefix ?> = document.getElementById(receptacleName_for_<?php echo $prefix ?>);
			selectAllReceptacleItems_for_<?php echo $prefix ?>(receptacle_for_<?php echo $prefix ?>);

			function swapOptions_for_<?php echo $prefix ?>(select)
			{
				// the values to replace with
				var replaceWith = data_for_<?php echo $prefix ?>[select.value];

				// get the next box which is the box we will populate
				var stationaryDrillNum = select.name.match(/.+_stationary_drill_(.+)/);
				stationaryDrillNum = parseInt(stationaryDrillNum[1]) + 1;
				var nextBoxName = prefix_for_<?php echo $prefix ?> + '_stationary_drill_' + stationaryDrillNum;
				var nextBox = document.getElementById(nextBoxName);

				// does it exist or are we at the end of the line?
				if (!isset(nextBox)) {
					return;
				}

				// truncate entries from the next box
				nextBox.options.length = 0;

				// populate the next box with the correct entries
				var i = 0;
				for (key in replaceWith) {
					nextBox.options[i] = new Option(replaceWith[key], key);
					i++;
				}

				// recurse until there are no more boxes to populate
				if (nextBox) {
					swapOptions_for_<?php echo $prefix ?>(nextBox);
				}

			}

			function addToReceptacle_for_<?php echo $prefix ?>(selectName_for_<?php echo $prefix ?>)
			{
				// get the box which holds the values we want to add
				var select = document.getElementById(selectName_for_<?php echo $prefix ?>);

				var options = select.options;
				for (var i=0; i<options.length; i++) {
					if (options[i].selected) {

						// we have the selected options, now add them to the receptacle if not currently present
						var found = false;
						for (var j=0; j<receptacle_for_<?php echo $prefix ?>.options.length; j++) {
							if (receptacle_for_<?php echo $prefix ?>.options[j].value == options[i].value) {
								found = true;
							}
						}

						if (!found) {
							// didn't find it, so append it to the receptacle
							receptacle_for_<?php echo $prefix ?>.options[receptacle_for_<?php echo $prefix ?>.options.length] = new Option(options[i].value + '. ' + options[i].text, options[i].value);
						}
					}
				}

				selectAllReceptacleItems_for_<?php echo $prefix ?>(receptacle_for_<?php echo $prefix ?>);
			}

			function removeFromReceptacle_for_<?php echo $prefix ?>()
			{
				// remove all selected items from the receptacle
				var receptacle_for_<?php echo $prefix ?> = document.getElementById(receptacleName_for_<?php echo $prefix ?>);
				var options = receptacle_for_<?php echo $prefix ?>.options;
				for (var i=0; i<options.length; i++) {
					if (options[i].selected) {
						receptacle_for_<?php echo $prefix ?>.remove(i);
					}
				}

				selectAllReceptacleItems_for_<?php echo $prefix ?>(receptacle_for_<?php echo $prefix ?>);
			}

			function removeAllFromReceptacle_for_<?php echo $prefix ?>()
			{
				// remove all items from the receptacle
				var receptacle_for_<?php echo $prefix ?> = document.getElementById(receptacleName_for_<?php echo $prefix ?>);
				receptacle_for_<?php echo $prefix ?>.options.length = 0;
			}

			function isset(v)
			{
				if (typeof v !== 'undefined' && v !== null) {
					return true;
				}

				return false;
			}

			function selectAllReceptacleItems_for_<?php echo $prefix ?>(receptacle_for_<?php echo $prefix ?>)
			{
				var options = receptacle_for_<?php echo $prefix ?>.options;
				for (var i=0; i<options.length; i++) {
					if (!options[i].selected) {
						options[i].selected = true;
					}
				}
			}

			</script>

		<?php

	}//end paintDrillDownView()


	/**
	* Outputs JS objects needed for the drill-down view
	*
	* @param array	$options	The hierarchy options
	* @param string	$prefix		The prefix of the asset
	*
	* @return void
	* @access private
	*/
	function _printJSObjects($options, $prefix)
	{
		echo '<script type="text/javascript">';
		echo 'var data_for_'.$prefix.' = {};';
		foreach ($options as $key => $value) {
			$children = $this->_getImmediateChildren($options, $key);

			if (!empty($children)) {
				echo 'var options_for_'.$prefix.' = {};';
				// create a JS object holding the values of the children
				foreach ($children as $child_key => $child_value) {
					echo 'options_for_'.$prefix.'[\''.$child_key.'\'] = \''.$child_value.'\';';
				}
				?>
					data_for_<?php echo $prefix ?>.<?php echo $key; ?> = options_for_<?php echo $prefix ?>;
				<?php
			}
		}
		echo '</script>';

	}//end _printJSObjects()


	/**
	* Gets the immediate children of the passed parent
	*
	* @param array	$options		The hierarchy options
	* @param string	$parent_string	The parent string
	* @param int	$target_length	The length of the next child of the passed parent
	*
	* @return array
	* @access private
	*/
	function _getImmediateChildren($options, $parent_string, $target_length=NULL)
	{
		if (is_null($target_length)) {
			// a child is by definition, 1 char longer
			$target_length = strlen($parent_string) + 1;
		}

		$children = Array();
		foreach ($options as $key => $value) {
			// ensure we are getting children of this value
			if ((strlen($key) === $target_length) && (strpos($key, $parent_string) === 0)) {
				$children[$key] = $value;
			}
		}

		if (empty($children)) {
			// look for a deeper level because the hierarchy may not be linear
			$levels = Array();
			foreach ($options as $key => $value) {
				if (strlen($key) > $target_length) {
					if (!in_array(strlen($key), $levels)) {
						$levels[] = strlen($key);
					}
				}
			}

			// sort so we are sure the first index is the next level
			sort($levels);
			if (isset($levels[0])) {
				$target_length = $levels[0];
				// do our search again
				foreach ($options as $key => $value) {
					// ensure we are getting children of this value
					if ((strlen($key) === $target_length) && (strpos($key, $parent_string) === 0)) {
						$children[$key] = $value;
					}
				}
			}
		}

		return $children;

	}//end _getImmediateChildren()


	/**
	* Processes the interface for hierarchy field
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processHierarchy(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		$is_contextable = (boolean)$asset->attr('is_contextable');

		if ($is_contextable === FALSE) {
			// If we are not contextable, we might as well be in the default
			// context... so change to it, and reload our asset
			$GLOBALS['SQ_SYSTEM']->changeContext(0);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$hierarchy_options = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$option_changes = $_POST[$prefix.'_options'];
			foreach ($option_changes['key'] as $i => $key) {
				if (isset($option_changes['del'][$i])) continue;
				$hierarchy_options[trim($key)] = trim($option_changes['val'][$i]);
			}
		}

		// now get the new submitted values (if any) and add them to array
		if (array_key_exists($prefix.'_new_key', $_POST) === TRUE) {
			if (strlen(trim($_POST[$prefix.'_new_key']))>0 && strlen(trim($_POST[$prefix.'_new_val']))>0) {
				$new_hierarchy_key  = trim($_POST[$prefix.'_new_key']);
				$new_hierarchy_val  = trim($_POST[$prefix.'_new_val']);

				if (strpos($new_hierarchy_key, '%asset_metadata_') !== FALSE) {
					trigger_localised_error('SYS0344', E_USER_WARNING);
					$new_hierarchy_key = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $new_hierarchy_key);
				}
				if (strpos($new_hierarchy_val, '%asset_metadata_') !== FALSE) {
					trigger_localised_error('SYS0344', E_USER_WARNING);
					$new_hierarchy_val = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $new_hierarchy_val);
				}

				$hierarchy_options[$new_hierarchy_key] = $new_hierarchy_val;
			}
		}

		// updating asset attribute value
		$asset->setAttrValue('hierarchy_options', $hierarchy_options);

		// saving default value
		$new_default = '';
		if (!empty($hierarchy_options) && isset($_POST[$prefix.'_default'])) {
			$new_default = $_POST[$prefix.'_default'];
			if (is_array($new_default)) {
				$new_default = implode('; ', $new_default);
			}
		}
		$asset->setAttrValue('default', $new_default);
		if ($asset->saveAttributes() === TRUE) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		if ($is_contextable === FALSE) {
			// If we are not contextable, we might as well be in the default
			// context... so change to it, and reload our asset
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
		}

		// Don't require our own saving of attributes
		return FALSE;

	}//end processHierarchy()


	/**
	* Paint the interface for setting hierarchy edit params
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintEditParams(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		$hierarchy = $asset->getHierarchyAttribute();
		$hierarchy->paintEditParams($prefix.'_style', $asset->writeAccess('attributes'));

	}//end paintEditParams()


	/**
	* Processes the interface for setting hierarchy edit params
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processEditParams(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$hierarchy = $asset->getHierarchyAttribute();
		$edit_params = $hierarchy->processEditParams($prefix.'_style');
		return $asset->setAttrValue('edit_params', $edit_params);

	}//end processEditParams()


	/**
	* Output the complete metadata tag for this field
	*
	* @param Metadata_Field_Hierarchy	$asset		the asset whose interface we are painting
	* @param string						$value_str	the current value for this field in the asset (NOT $asset)
	*												NULL means that there is no current value set
	* @param array						&$tag_value	the metadata tag value
	* @param array						&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Field_Hierarchy $asset, $value_str, &$tag_value, Array &$keywords)
	{
		if (is_null($value_str)) {
			$value_str = Metadata_Field::encodeValueString($asset->attr('default'), Array());
		}

		// If we are showing the value, then go fetch it
		if ($asset->attr('visible_part') === 'value') {
			$value_str = $asset->getValueFromKey($value_str);
		}

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords);

	}//end generateMetadata()


	/**
	* Sort the hierarchy field
	*
	* @param array		$hierarchy		An array containing the values to sort
	* @param int		$sort_by		The option to sort by (0=No Sort, 1=Sort by Key, 2=Sort by Value)
	* @param boolean	$show_parents	The option to show the parents
	* @param string		$pad			The character to show the hierarchy levels
	*
	* @return array|NULL
	* @access public
	*/
	public function _sortHierarchyOptions($hierarchy, $sort_by=0, $show_parents=FALSE, $pad='-')
	{
		if (!is_array($hierarchy)) return NULL;
		if (empty($hierarchy)) return $hierarchy;

		switch ($sort_by) {
			case 1:
				// Using usort to get a more structured list
				$sort_by_key = array_keys($hierarchy);
				$aok = usort($sort_by_key, 'strcmp');
				$sort_hier = Array();
				foreach ($sort_by_key as $key) {
					$sort_hier[$key] = $hierarchy[$key];
				}

				// Reset hierarchy and reset it to $sort_hier
				$hierarchy = Array();
				$hierarchy = $sort_hier;
			break;
			case 2:
				// Sort by value
				$aok = asort($hierarchy);
			break;
		}

		if ($show_parents) {
			foreach ($hierarchy as $key => $value) {
				$show_lvl = str_pad('', (strlen($key)-1), $pad);
				$hierarchy[$key] = $show_lvl.$value;
			}
		}//end if

		return $hierarchy;

	}//end _sortHierarchyOptions()


	/**
	* Get the hierarchy tree
	*
	* @param string		$start_value	A string of the start value
	* @param array		$options		The all of the hierarchy options
	* @param string		$pad			The character to show the hierarchy levels
	*
	* @return array|NULL
	* @access public
	*/
	public function getHierarchyTree($start_value, $options, $pad='; ')
	{
		$return_value = $start_value;
		$start_value = strtolower($start_value);
		$get_values = explode($pad, $start_value);
		if (is_array($get_values)) {
			$new_values = Array();
			foreach ($get_values as $value) {
				// Get the parents and add them
				$get_parents = $this->getParentHierarchy($value, $options);
				if (is_array($get_parents) && !is_null($get_parents)) {
					foreach ($get_parents as $parent) {
						if (!in_array($parent, $get_values)) {
							$new_values[] = $parent;
						}
					}
				}
				// Get the children and add them
				$get_childs = $this->getChildrenHierarchy($value, $options);
				if (is_array($get_childs) && !is_null($get_childs)) {
					foreach ($get_childs as $child) {
						if (!in_array($child, $get_values)) {
							$new_values[] = $child;
						}
					}
				}
			}
			// Add the old values to the end
			foreach ($get_values as $value) {
				$new_values[] = $value;
			}

			// Sort them out
			natsort($new_values);

			// Remove duplicate entries
			$new_values = array_unique($new_values);

			// Prep the value to return
			$return_value = implode($pad, $new_values);
		}//end if

		return $return_value;

	}//end getHierarchyTree()


	/**
	* Get the parents in the hierarchy tree of this value
	*
	* @param string		$value			A string of the start value
	* @param array		$options		The all of the hierarchy options
	*
	* @return array|NULL
	* @access public
	*/
	public function getParentHierarchy($value, $options=NULL)
	{
		$return_val = NULL;

		if (!empty($value)) {
			if (is_null($options) || !is_array($options)) {
				return $return_val;
			}

			// This is the parent or no depth was specified
			if (strlen($value) == 1) return $return_val;

			for ($i=0; $i<strlen($value); $i++) {
				$parent = '';
				$parent = substr($value, 0, $i);
				if (array_key_exists(strtolower($parent), array_change_key_case($options, CASE_LOWER))) {
					$return_val[] = $parent;
				}
			}

		}

		return $return_val;

	}//end getParentHierarchy()


	/**
	* Get the children in the hierarchy tree of this value
	*
	* @param string		$value			A string of the start value
	* @param array		$options		The all of the hierarchy options
	*
	* @return array|NULL
	* @access public
	*/
	public function getChildrenHierarchy($value, $options=NULL)
	{
		$return_val = NULL;

		if (!empty($value)) {
			if (is_null($options) || !is_array($options)) {
				return $return_val;
			}

			foreach ($options as $opt_key => $opt_value) {
				if (preg_match('/^'.strtolower($value).'/i', strtolower($opt_key))) {
					$return_val[] = strtolower($opt_key);
				}
			}
		}

		return $return_val;

	}//end getChildrenHierarchy()


	/**
	* Show-if conditional: display flat view options if selected
	*
	* @param object &$asset asset being painted
	*
	* @return boolean
	* @access public
	*/
	function isFlatView(&$asset)
	{
		$attr = $asset->attr('display_type');
		return empty($attr) ? TRUE : FALSE;

	}//end isFlatView()


	/**
	* Show-if conditional: display drill-down view options if selected
	*
	* @param object &$asset asset being painted
	*
	* @return boolean
	* @access public
	*/
	function isDrillDownView(&$asset)
	{
		$attr = $asset->attr('display_type');
		return empty($attr) ? FALSE : TRUE;

	}//end isDrillDownView()


	/**
	* Paint the interface for setting level titles
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintLevelTitles(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$hierarchy = $asset->getHierarchyAttribute();
		$hierarchy_options = $hierarchy->_params['options'];
		unset($hierarchy);
		$level_titles = $asset->attr('level_titles');

		$levels = Array();
		foreach ($hierarchy_options as $key => $value) {
			// hierarchy may not be linear so calculate how many select boxes we will need this way
			if (!in_array(strlen($key), $levels)) {
				$levels[] = strlen($key);
			}
		}

		echo '<table>';
		$i = 1;
		foreach ($levels as $id) {
			echo '<tr>';
			echo '<td>'.translate('level').' '.$i.':</td><td>';
			if ($wa) {
				echo text_box($prefix.'_level_titles['.$id.']', isset($level_titles[$id]) ? htmlspecialchars_decode($level_titles[$id]) : '').'</td>';
			} else {
				echo isset($level_titles[$id]) ? $level_titles[$id] : '';
			}
			echo '</tr>';
			$i++;
		}
		echo '</table>';

		return $wa;

	}//end paintLevelTitles()


	/**
	* Processes the interface for setting level titles
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processLevelTitles(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			if (isset($_POST[$prefix.'_level_titles'])) {
				foreach ($_POST[$prefix.'_level_titles'] as $key => $value) {
					$_POST[$prefix.'_level_titles'][$key] = htmlspecialchars($value);
				}
				$asset->setAttrValue('level_titles', $_POST[$prefix.'_level_titles']);
			}
		}

		return $wa;

	}//end processLevelTitles()
	
	
	/**
	* Paint the interface for setting Display Type
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDisplayType(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$display_type = $asset->attr('display_type');
		$options = Array(
					'0'	=> 'Flat View',
					'1'	=> 'Drill-Down View',
				   );
		if ($wa) {
			echo combo_box($prefix.'_display_type', $options, FALSE, $display_type);
		} else {
			echo $options[$display_type];
		}

		return $wa;

	}//end paintDisplayType()


	/**
	* Processes the interface for setting Display Type
	*
	* @param Metadata_Field_Hierarchy	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDisplayType(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			if (isset($_POST[$prefix.'_display_type'])) {
				$asset->setAttrValue('display_type', $_POST[$prefix.'_display_type']);
				if ($_POST[$prefix.'_display_type'] == '1') $asset->setAttrValue('multiple', TRUE);
				$asset->saveAttributes();
			}
		}

		return $wa;

	}//end processDisplayType()
	
	
}//end class

?>
