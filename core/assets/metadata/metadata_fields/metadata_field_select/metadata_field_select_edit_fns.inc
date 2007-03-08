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
* $Id: metadata_field_select_edit_fns.inc,v 1.36 2007/03/08 04:00:38 tbarrett Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Select_Edit_Fns
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.36 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Select_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Select_Edit_Fns()
	{
		$this->Metadata_Field_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for filling in metadata values
	*
	* Returns true if a submitable interface has been printed
	*
	* @param object		&$asset			the metadata field asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$value_str		the current value for this field in the owning asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param boolean	$write_access	whether to allow editing of this field
	*									(typically depends on write access to owner and editability of field)
	* @param boolean	$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(&$asset, &$o, $value_str, $write_access, $enable_default=TRUE)
	{
		$prefix = $asset->getPrefix();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$value = '';
		$value_components = $asset->attr('value_components');

		$select_options = $asset->attr('select_options');
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
		// If we're OK to edit, get our selection attribute, and paint it
		if ($write_access) {
			$selection = $asset->getSelectionAttribute();
			$selection->value = $value;
			if ($enable_default) {
				$selection->_edit_params['extras'] = 'onclick="document.getElementById(\''.$prefix.'_default\').checked = false;"';
				if ($is_default) {
					$selection->_edit_params['extras'] .= ' disabled="disabled"';
				}
				echo '<span id="'.$prefix.'_field">';
			}
			$selection->paint($prefix, FALSE);
			if ($enable_default) echo '</span>';
		} else {
			if (!((is_array($value) && empty($value)) || (is_string($value) && $value == ''))) {
				if (!is_array($value)) $value = Array($value);
				$value_output = '';
				foreach ($value as $option) {
					if (isset($select_options[$option])) {
						$value_output .= $select_options[$option].'; ';
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
				foreach ($select_options as $key => $select_option) {
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
				$o->addJsInclude(sq_web_path('data').'/asset_types/metadata_field_select/js/metadata_field_select.js');
			} else {
				if ($is_default) {
					echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
				}
			}
		}

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object	&$asset			the asset whose interface we are processing
	* @param string	&$new_value_str	a string by reference that we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_value_str)
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$prefix = $asset->getPrefix();
		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			return TRUE;
		} else {
			// Default is not checked, so process our select attribute
			$selection = $asset->getSelectionAttribute();
			$selection->process($prefix);
			if ($selection->processed) {
				// Get the string into proper form for the metadata field function
				$value = str_replace('|', '; ', $selection->value);
				$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());
				return TRUE;
			} else {
				// nothing was submitted
				return FALSE;
			}
		}

	}//end processInlineValueInterface()


	/**
	* Paint the interface for dropdown selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSelection(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$select_options = $asset->attr('select_options');

		if (!empty($select_options)) {
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
				foreach ($select_options as $option_key => $option_value) {
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
				}//end foreach select_options
				?>
			</table>
			<?php

		} else {
			echo translate('no_select_box_options');
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

		if (!empty($select_options)) {
			// displaying default value if any values were actually saved in the array
			$o->openField(translate('default_value'));
				$selection = $asset->getSelectionAttribute();
				$selection->paint($prefix.'_default', !$write_access);
			$o->closeField();
		}

		return TRUE;

	}//end paintSelection()


	/**
	* Processes the interface for dropdown selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSelection(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$select_options = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$option_changes = $_POST[$prefix.'_options'];
			foreach ($option_changes['key'] as $i => $key) {
				if (isset($option_changes['del'][$i])) continue;
				$select_options[trim($key)] = trim($option_changes['val'][$i]);
			}
		}

		// now get the new submitted values (if any) and add them to array
		if (!empty($_POST[$prefix.'_new_key']) && !empty($_POST[$prefix.'_new_val'])) {
			$new_select_key  = trim($_POST[$prefix.'_new_key']);
			$new_select_val  = trim($_POST[$prefix.'_new_val']);
			$select_options[$new_select_key] = $new_select_val;
		}

		// updating asset attribute value
		$asset->setAttrValue('select_options', $select_options);

		// saving default value
		$new_default = '';
		if (!empty($select_options) && isset($_POST[$prefix.'_default'])) {
			$new_default = $_POST[$prefix.'_default'];
			if (is_array($new_default)) {
				$new_default = implode('; ', $new_default);
			}
		}
		if (!$asset->attr('multiple')) {
			if (!isset($select_options[$new_default])) {
				$new_default = '';
			}
		}
		$asset->setAttrValue('default', $new_default);

		return TRUE;

	}//end processSelection()


	/**
	* Paint the interface for setting selection edit params
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEditParams(&$asset, &$o, $prefix)
	{
		$selection = $asset->getSelectionAttribute();
		$selection->paintEditParams($prefix.'_style', $asset->writeAccess('attributes'));

	}//end paintEditParams()


	/**
	* Processes the interface for setting selection edit params
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processEditParams(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$selection = $asset->getSelectionAttribute();
		$edit_params = $selection->processEditParams($prefix.'_style');
		return $asset->setAttrValue('edit_params', $edit_params);

	}//end processEditParams()


	/**
	* Output the complete metadata tag for this field
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	* @param array	&$tag_value	the metadata tag value
	* @param array	&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $value_str, &$tag_value, &$keywords)
	{
		if (is_null($value_str)) {
			$value_str = str_replace('; ', '\\; ', $asset->attr('default'));
		}
		$value_str = $asset->getValueFromKey($value_str);

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords);

	}//end generateMetadata()


}//end class

?>
