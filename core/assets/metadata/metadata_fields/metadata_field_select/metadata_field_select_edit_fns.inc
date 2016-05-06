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
* $Id: metadata_field_select_edit_fns.inc,v 1.54 2013/07/25 23:23:48 lwright Exp $
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
* @version $Revision: 1.54 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Select_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* @param Metadata_Field_Select	$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter		$o				the outputter class
	* @param string					$value_str		the current value for this field in the owning asset (NOT $asset)
	*												NULL means that there is no current value set
	* @param boolean				$write_access	whether to allow editing of this field
	*												(typically depends on write access to owner and editability of field)
	* @param boolean				$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Select $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$value = '';
		$value_components = $asset->attr('value_components');

		$select_options = $asset->getContextedSelectOptions();
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

			if($enable_default || $print_cascade_values_option) echo '<span class="sq-metadata-settings-wrapper">';
				if ($enable_default) {
					echo '<span class="sq-metadata-default-wrapper">';
						// Work out which options we have to set if we have to
						$i = 0;
						$default_keys = explode('; ', $asset->attr('default'));
						$default_selected = Array();

						$default_vals = Array();
						$non_default_vals = Array();
						foreach ($select_options as $key => $select_option) {
							if (in_array($key, $default_keys)) {
								$default_vals[] = "'".addslashes(str_replace(' ', '_', $key))."'";
							} else {
								$non_default_vals[] = "'".addslashes(str_replace(' ', '_', $key))."'";
							}
						}
						$default_vals_expr = '['.implode(', ', $default_vals).']';
						$non_default_vals_expr = '['.implode(', ', $non_default_vals).']';
						check_box($prefix.'_default', '1', $is_default, "handleDefaultClick(this, '$prefix', $default_vals_expr, $non_default_vals_expr); ");
						label(translate('Use default'), $prefix.'_default');

						$o->addJsInclude(sq_web_path('data').'/asset_types/metadata_field_select/js/metadata_field_select.js');
					echo '</span>';
				}

   				$allow_cascading = $asset->attr('allow_cascading');
				if ($write_access && $print_cascade_values_option && $allow_cascading) {
					echo '<span class="sq-metadata-cascade-wrapper">';
						check_box($prefix.'_cascade_value', '1', FALSE);
						label(translate('Cascade value'), $prefix.'_cascade_value');

					echo '</span>';
				}
			if($enable_default || $print_cascade_values_option) echo '</span>';

			$selection = $asset->getSelectionAttribute();
			$selection->value = $value;
			if ($enable_default) {
				$selection->_edit_params['extras'] = 'onclick="document.getElementById(\''.$prefix.'_default\').checked = false;"';
				if ($is_default) {
					$selection->_edit_params['extras'] .= ' disabled="disabled"';
				}
				echo '<span id="'.$prefix.'_field" class="sq-metadata-select-list-wrapper">';
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
			} else {
				echo '<span class="sq-metadata-settings-wrapper">';
					if ($is_default) {
						echo ' <em>('.translate('Using default value').')</em>';

					} else{

					}
				echo '</span>';
			}
		}

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Select	$asset			the asset whose interface we are processing
	* @param string					&$new_value_str	a string by reference that we can store the new values in
	* @param bool     &$field_cascade_value Whether value updates to this field should cascade
	*                                       to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Select $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
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
			// Default is not checked, so process our select attribute
			$selection = $asset->getSelectionAttribute();
			$selection->process($prefix);
			if ($selection->processed) {
				// Get the string into proper form for the metadata field function
				$value = str_replace('|', '; ', $selection->value);
				$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());
				$processed = TRUE;
			}
		}

		if (isset($_POST[$prefix.'_cascade_value'])) {
			$field_cascade_value = TRUE;
			$processed = TRUE;
		}

		return $processed;

	}//end processInlineValueInterface()


	/**
	* Paint the interface for dropdown selection box
	*
	* @param Metadata_Field_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintSelection(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$is_contextable = (boolean)$asset->attr('is_contextable');

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$is_default_context = ($contextid === 0) ? TRUE : FALSE;

		if ($is_contextable === TRUE) {
			$select_options = $asset->getContextedSelectOptions();
		} else {
			$select_options = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('select_options', 'metadata_field_select', Array($asset->id), 0);
			$select_options = unserialize($select_options[$asset->id]);
		}

		?>
		<table class="sq-backend-table<?php echo $write_access ? ' sq-sortable-table' : ''?>" id="<?php echo $prefix.'_table'; ?>">
			<thead>
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('Option Key'); ?></th>

					<th class="sq-backend-table-header"><?php echo translate('Option Value'); ?></th>

					<?php
					if (($write_access  === TRUE) && (($is_default_context === TRUE) || ($is_contextable === FALSE))) {
						?><th class="sq-backend-table-header" style="width:80px; text-align:center;"><?php echo translate('Delete?'); ?></th><?php

					}
					?>
				</tr>
			</thead>
			<tbody class="sq-metadata-select-options">
				<?php
				if (!is_array($select_options)) {
					$select_options = Array();
				}
				$i = 0;
				// Base this on the default values because then any new
				// option not already in a modified context can be set to the
				// default context's value for it

				foreach ($select_options as $option_key => $option_value) {
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
							<td class="sq-backend-table-cell" style="text-align: center">
								<?php check_box($prefix.'_options[del]['.$i.']', $option_key); ?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
					$i++;
				}//end foreach select_options
				if ($write_access && ($is_default_context === TRUE || $is_contextable === FALSE)) {
					$i++;
				?>
					<tr>
						<td class="sq-backend-table-cell"><?php text_box($prefix.'_options[key]['.$i.']'); ?></td>
						<td class="sq-backend-table-cell" align="left"><?php text_box($prefix.'_options[val]['.$i.']'); ?></td>
						<td class="sq-backend-table-cell"></td>
					</tr>
				<?php
				}
				?>
			</tbody>
			<?php
			if ($write_access && ($is_default_context === TRUE || $is_contextable === FALSE)) {
			?>
			<tfoot>
					<tr>
						<td class="sq-backend-table-cell" colspan="3"><?php
							normal_button($prefix.'_options_add_new', translate('Add Option'));
						?>
						</td>
					</tr>
			</tfoot>
			<?php
				}
			?>
		</table>
		<script src="<?php echo sq_web_path('lib'); ?>/web/dfx/dfx.js"></script>
		<script>
			(function() {
				dfxLoadedCallback = function() {
					//for drag n drop sorting
					dfx.tableSort('#<?php echo $prefix.'_table'  ?>', '.sq-sortable-table', 'all');
					
					// Click event for 'Add Option' button
					var optionIndex = <?php echo $i; ?>;
					var $addNewOptionButton = dfxjQuery('#<?php echo $prefix.'_options_add_new'; ?>');
						$addNewOptionButton.click(function(event) {
							optionIndex++;
							var keyFieldId = '<?php echo $prefix; ?>_options[key]['+optionIndex+']';
							var valueFieldId = '<?php echo $prefix; ?>_options[val]['+optionIndex+']';
							var newOptionRowHtml = ''+
								'<tr title="<?php echo translate('Drag to Reorder'); ?>">'+
									'<td class="sq-backend-table-cell"><input type="text" class="sq-form-field" id="'+keyFieldId+'" name="'+keyFieldId+'"></td>'+
									'<td class="sq-backend-table-cell"><input type="text" class="sq-form-field" id="'+valueFieldId+'" name="'+valueFieldId+'"></td>'+
									'<td class="sq-backend-table-cell"></td>'+
								'</tr>';
							dfxjQuery('#<?php echo $prefix.'_table'; ?>.sq-sortable-table').find('tbody').append(newOptionRowHtml);
					});

				}//end dfxLoadedCallback()
			})();
		</script>
		<?php

		if (!empty($select_options)) {
			// displaying default value if any values were actually saved in the array
			$o->openField(translate('Default Value'));

				$selection = $asset->getSelectionAttribute();
				$selection->paint($prefix.'_default', !$write_access);
			$o->closeField();
		}

		return TRUE;

	}//end paintSelection()


	/**
	* Processes the interface for dropdown selection box
	*
	* @param Metadata_Field_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processSelection(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
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

		$select_options = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$option_changes = $_POST[$prefix.'_options'];
			foreach ($option_changes['key'] as $i => $key) {
				$key = trim($key);
				$value = trim($option_changes['val'][$i]);
				if (!empty($key) && !isset($option_changes['del'][$i])) {
					if (strpos($key, '%asset_metadata_') !== FALSE) {
						trigger_localised_error('SYS0344', translate('Cannot use asset_metadata_XXX keyword for metadata field value. Replacing asset_metadata_XXX keyword by metadata_field_XXX'), E_USER_WARNING);
						$key = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $key);
					}
					if (strpos($value, '%asset_metadata_') !== FALSE) {
						trigger_localised_error('SYS0344', translate('Cannot use asset_metadata_XXX keyword for metadata field value. Replacing asset_metadata_XXX keyword by metadata_field_XXX'), E_USER_WARNING);
						$value = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $value);
					}
					$select_options[$key] = $value;
				}
			}//end foreach
		}//end if

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
		
		if (!$asset->attr('multiple') && !$asset->attr('allow_empty')) {
			if(!empty($select_options)) {
				reset($select_options);
				if(!isset($_POST[$prefix.'_default'])){
					$new_default = key($select_options); 
				}else{
					if(array_key_exists($_POST[$prefix.'_default'], $select_options) === FALSE) {
						$new_default = key($select_options);
					}
				}
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

	}//end processSelection()


	/**
	* Paint the interface for setting selection edit params
	*
	* @param Metadata_Field_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintEditParams(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
	{
		$selection = $asset->getSelectionAttribute();
		$selection->paintEditParams($prefix.'_style', $asset->writeAccess('attributes'));

	}//end paintEditParams()


	/**
	* Processes the interface for setting selection edit params
	*
	* @param Metadata_Field_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processEditParams(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$selection = $asset->getSelectionAttribute();
		$edit_params = $selection->processEditParams($prefix.'_style');
		return $asset->setAttrValue('edit_params', $edit_params);

	}//end processEditParams()


	/**
	* Output the complete metadata tag for this field
	*
	* @param Metadata_Field_Select	$asset		the asset whose interface we are painting
	* @param string					$value_str	the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	* @param array					&$tag_value	the metadata tag value
	* @param array					&$keywords	an array we can put keywords in that need replacing
	* @param boolean        		$content_file   Whether the generated metadata will be included in a metadata content file
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Field_Select $asset, $value_str, &$tag_value, Array &$keywords, $content_file)
	{
		if (is_null($value_str)) {
			$value_str = Metadata_Field::encodeValueString($asset->attr('default'), Array());
		}

		// If we are showing the value, then go fetch it
		if ($asset->attr('visible_part') === 'value') {
			$value_str = $asset->getValueFromKey($value_str);
		}

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords, $content_file);

	}//end generateMetadata()


	/**
	* Print the interface for uploading a CSV file containing a mass list of options
	*
	* @param Form_Question_Type_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	public function paintOptionUpload(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if ($write_access) file_upload($prefix.'_option_upload');

		return $write_access;

	}//end paintOptionUpload()


	/**
	* Print the interface for the csv header checkbox
	*
	* @param Form_Question_Type_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	public function paintOptionUploadHeader(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if ($write_access) check_box($prefix.'_csv_header', '1');

		return $write_access;

	}//end paintOptionUploadHeader()


	/**
	* Print the interface for uploading a CSV file containing a mass list of options
	*
	* @param Form_Question_Type_Select	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	public function processOptionUpload(Metadata_Field_Select $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_FILES[$prefix.'_option_upload'])) {
			$file = $_FILES[$prefix.'_option_upload'];

			$header = FALSE;
			if (isset($_POST[$prefix.'_csv_header'])) {
				$header = TRUE;
			}

			if ($file['error'] === UPLOAD_ERR_OK) {
				$asset->importOptionsFromCSV($file['tmp_name'], $header);
			}
		}

	}//end processOptionUpload()


}//end class

?>
