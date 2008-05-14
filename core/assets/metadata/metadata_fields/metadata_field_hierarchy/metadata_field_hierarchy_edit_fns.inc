<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_hierarchy_edit_fns.inc,v 1.1.2.2 2008/05/14 06:18:47 bpearson Exp $
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
* @version $Revision: 1.1.2.2 $
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
	public function paintValueInterface(Metadata_Field_Hierarchy $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option=TRUE)
	{
		$prefix = $asset->getPrefix();
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
		// If we're OK to edit, get our hierarchy attribute, and paint it
		if ($write_access) {
			$hierarchy = $asset->getHierarchyAttribute();
			$hierarchy->value = $value;
			$hierarchy_options = $hierarchy->_params['options'];
			$hierarchy_sort = $hierarchy->_params['sort'];
			$hierarchy_structure = $hierarchy->_params['structure'];
			$hierarchy_level = $hierarchy->_params['level'];
			$hierarchy_depth = $hierarchy->_params['depth'];

			$hier_sort = (isset($hierarchy_sort) && $hierarchy_sort>=0) ? $hierarchy_sort : '0';
			$hier_structure = (isset($hierarchy_structure) && $hierarchy_structure) ? $hierarchy_structure : FALSE;
			$level = (isset($hierarchy_level) && $hierarchy_level>=0) ? $hierarchy_level : '1';
			$depth = (isset($hierarchy_depth) && $hierarchy_depth>=0) ? $hierarchy_depth : '1';

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
				$options = $this->_sortHierarchyOptions($options, $hier_sort, $hier_structure, $level, $depth);

				$hierarchy->_params['options'] = $options;
			}
			$hierarchy->paint($prefix, FALSE);
			if ($enable_default) echo '</span>';
		} else {
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

		if ($enable_default) {
			if ($write_access) {
				echo ' &nbsp; &nbsp; &nbsp; ';

				// Work out which options we have to set if we have to
				$i = 0;
				$default_keys = explode('; ', $asset->attr('default'));
				$default_selected = Array();

				$default_vals = Array();
				$non_default_vals = Array();
				foreach ($hierarchy_options as $key => $hierarchy_option) {
					if (in_array($key, $default_keys)) {
						$default_vals[] = "'".addslashes($key)."'";
					} else {
						$non_default_vals[] = "'".addslashes($key)."'";
					}
				}
				$default_vals_expr = '['.implode(', ', $default_vals).']';
				$non_default_vals_expr = '['.implode(', ', $non_default_vals).']';
				check_box($prefix.'_default', '1', $is_default, "handleDefaultClick(this, '$prefix', $default_vals_expr, $non_default_vals_expr); ");
				label(translate('use_default'), $prefix.'_default');
				$o->addJsInclude(sq_web_path('data').'/asset_types/metadata_field_hierarchy/js/metadata_field_hierarchy.js');
			} else {
				if ($is_default) {
					echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
				}
			}
		}

		if ($write_access && $print_cascade_values_option) {
			echo '&nbsp;&nbsp;&nbsp;&nbsp;';
			check_box($prefix.'_cascade_value', '1', FALSE);
			label(translate('cascade_value'), $prefix.'_cascade_value');
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
	public function processInlineValueInterface(Metadata_Field_Hierarchy $asset, &$new_value_str, &$field_cascade_value)
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;

		$prefix = $asset->getPrefix();
		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			// Default is not checked, so process our hierarchy attribute
			$hierarchy = $asset->getHierarchyAttribute();

			// Process the hierarchy and add the parents into the metadata field
			$value = (isset($_REQUEST[$prefix])) ? $_REQUEST[$prefix] : '';
			if (is_array($value)) implode('; ', $value);

			$hoptions = $hierarchy->_params['options'];
			$hlevel = $hierarchy->_params['level'];
			$hdepth = $hierarchy->_params['depth'];

			$hierarchy->value = $value;

			$new_value_str = $value;
			$processed=TRUE;

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
		$hierarchy_options = $asset->attr('hierarchy_options');
		$hierarchy_sort = $asset->attr('hier_sort');

		if (!empty($hierarchy_options)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('option_key'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('option_value'); ?></td>
					<?php
					if ($write_access) {
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
							if ($write_access) {
								text_box($prefix.'_options[key]['.$i.']', $option_key, 20);
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
						if ($write_access) {
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
		if ($write_access) {
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
		$lvl = $asset->attr('hier_level');
		$dpth = $asset->attr('hier_depth');

		if (!empty($hierarchy_options)) {
			// displaying default value if any values were actually saved in the array
			$o->openField(translate('default_value'));
				$default_hier = $asset->getHierarchyAttribute();
				if ($write_access) {
					if ($sort > 0) {
						$default_options = $this->_sortHierarchyOptions($hierarchy_options, $sort, $format, $lvl, $dpth);
						if (isset($default_options) && !is_null($default_options)) {
							$default_hier->_params['options'] = $default_options;
						}

					}
					$default_hier->paint($prefix.'_default', FALSE);
				} else {
					echo $default_hier->_params['options'][$default_hier->value];
				}
			$o->closeField();
		}

		// Being user friendly show a quick preview of what it looks like
		$o->openField(translate('preview'));
			if (!empty($hierarchy_options)) {
				// Show no setting or value
				$preview_hierarchy = $asset->getHierarchyAttribute();
				$preview_hierarchy->value = '';
				if ($sort > 0) {
					$preview_options = $this->_sortHierarchyOptions($hierarchy_options, $sort, $format, $lvl, $dpth);
					if (isset($preview_options) && !is_null($preview_options)) {
						$preview_hierarchy->_params['options'] = $preview_options;
					}

				}
				$preview_hierarchy->paint('some_name_not_used', FALSE);
			} else {
				echo translate('no_hierarchy_box_options');
			}//end if
		$o->closeField();

		return TRUE;

	}//end paintHierarchy()


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

		$hierarchy_options = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$option_changes = $_POST[$prefix.'_options'];
			foreach ($option_changes['key'] as $i => $key) {
				if (isset($option_changes['del'][$i])) continue;
				$hierarchy_options[trim($key)] = trim($option_changes['val'][$i]);
			}
		}

		// now get the new submitted values (if any) and add them to array
		if (strlen(trim($_POST[$prefix.'_new_key']))>0 && strlen(trim($_POST[$prefix.'_new_val']))>0) {
			$new_hierarchy_key  = trim($_POST[$prefix.'_new_key']);
			$new_hierarchy_val  = trim($_POST[$prefix.'_new_val']);
			$hierarchy_options[$new_hierarchy_key] = $new_hierarchy_val;
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
		if (!$asset->attr('multiple')) {
			if (!isset($hierarchy_options[$new_default])) {
				$new_default = '';
			}
		}
		$asset->setAttrValue('default', $new_default);

		return TRUE;

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
			$value_str = str_replace('; ', '\\; ', $asset->attr('default'));
		}

		$options = $asset->attr('hierarchy_options');
		$level = $asset->attr('hier_level');
		$depth = $asset->attr('hier_depth');

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords);

	}//end generateMetadata()


	/**
	* Sort the hierarchy field
	*
	* @param array		$hierarchy		An array containing the values to sort
	* @param int		$sort_by		The option to sort by (0=No Sort, 1=Sort by Key, 2=Sort by Value)
	* @param boolean	$show_parents	The option to show the parents
	* @param string		$level			The number of levels in the hierarchy
	* @param string		$depth			The depth of each level in the hierarchy
	* @param string		$pad			The character to show the hierarchy levels
	*
	* @return array|NULL
	* @access public
	*/
	public function _sortHierarchyOptions($hierarchy, $sort_by=0, $show_parents=FALSE, $level='1', $depth='1', $pad='-')
	{
		if (!is_array($hierarchy)) return NULL;
		if (empty($hierarchy)) return $hierarchy;

		switch ($sort_by) {
			case 1:
				// Sort by key ... Could use ksort() but if the keys are a series of numbers
				// ksort() sorts them numerically, whereas natsort() sorts them to a human like list
				$sort_by_key = array_keys($hierarchy);
				$aok = natsort($sort_by_key);
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
				$aok = sort($hierarchy);
			break;
		}

		if ($show_parents) {
			foreach ($hierarchy as $key => $value) {
				if (strlen($key) == $depth) {
					// The highest level
					$hierarchy[$key] = $value;
					continue;
				} else if (strlen($key) > ($level * $depth)) {
					// Whoa! This key exceeds the maximum it can go
					// Set it for the max anyhow
					$show_lvl = str_pad('', ($level*$depth), $pad);
					$hierarchy[$key] = $show_lvl.$value;
					continue;
				} else {
					// Other Levels
					$show_lvl = str_pad('', ((strlen($key))/$depth), $pad);
					$hierarchy[$key] = $show_lvl.$value;
					continue;
				}
			}
		}//end if

		return $hierarchy;

	}//end _sortHierarchyOptions()


	/**
	* Get the hierarchy tree
	*
	* @param string		$start_value	A string of the start value
	* @param array		$options		The all of the hierarchy options
	* @param int		$level			The number of levels in the hierarchy
	* @param int		$depth			The depth of each level in the hierarchy
	* @param string		$pad			The character to show the hierarchy levels
	*
	* @return array|NULL
	* @access public
	*/
	public function getHierarchyTree($start_value, $options, $level='1', $depth='1', $pad='; ')
	{
		$return_value = $start_value;
		$start_value = strtolower($start_value);
		$get_values = explode($pad, $start_value);
		if (is_array($get_values)) {
			$new_values = Array();
			foreach ($get_values as $value) {
				// Get the parents and add them
				$get_parents = $this->getParentHierarchy($value, $options, $level, $depth);
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
	* @param int		$level			The number of levels in the hierarchy
	* @param int		$depth			The depth of each level in the hierarchy
	*
	* @return array|NULL
	* @access public
	*/
	public function getParentHierarchy($value, $options=NULL, $level='1', $depth='1')
	{
		$return_val = NULL;

		if (!empty($value)) {
			if (is_null($options) || !is_array($options)) {
				return $return_val;
			}

			// This is the parent or no depth was specified
			if (strlen($value) == $depth) return $return_val;

			for ($i=1; $i<=$level; $i++) {
				$parent = '';
				$parent = substr($value, 0, ($i*$depth));
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


}//end class

?>
