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
* $Id: metadata_field_text_edit_fns.inc,v 1.45 2013/10/13 23:51:55 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Text_Edit_Fns
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.45 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Text_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* @param Metadata_Field_Text	$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter		$o				the outputter class
	* @param string					$value_str		the current value for this field in the owning asset (NOT $asset)
	*												NULL means that there is no current value set
	* @param boolean				$write_access	whether to allow editing of this field
	*												(typically depends on write access to owner and editability of field)
	* @param boolean				$enable_default	Whether to show the 'use default' checkbox
	* @param string					$field_prefix	The field's input element's id value
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Text $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '', &$field_prefix='')
	{
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		$is_default = FALSE;

		if (is_null($value_str)) {
			$value_str = $asset->getDefaultValue();

			$is_default = TRUE;
		}

		$value = '';
		$value_components = $asset->attr('value_components');
		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		if ($write_access) {
			$extras = ($is_default && $enable_default) ? 'disabled="disabled"' : '';

			echo '<span class="sq-metadata-settings-wrapper">';
				if ($enable_default) {
					echo '<span class="sq-metadata-default-wrapper">';
					$default_string = str_replace("'", "\'", $asset->attr('default'));
					// get rid of javascript error 'unterminated string literal'. (bug 2386)
					$default_string = str_replace(Array("\r\n", "\r", "\n"), '\n', $default_string);

					$checkbox_extra = 'with(this.form.elements[\''.$prefix.'_value\']) { disabled = this.checked; if (this.checked) { value = \''.$default_string.'\'; } else {if(value==\'\') {value=\' \'; value = \'\';}}};';

					if (!empty($value_components)) {
						foreach ($value_components as $vc_name => $vc_value) {
							$default_value_comp = $asset->attr('value_components');
							$default_vc_value = $default_value_comp[$vc_name];
							$checkbox_extra .= 'with(this.form.elements[\''.$prefix.'_vc_value['.$vc_name.']\']) { disabled = this.checked; if (this.checked) { value = \''.$default_vc_value.'\'; } else {if(value==\'\') {value=\' \'; value = \'\';}}};';
						}
					}

					check_box($prefix.'_default', '1', $is_default, $checkbox_extra);
					label(translate('Use default'), $prefix.'_default');

					echo '</span>';
				}

				if ($print_cascade_values_option) {
					echo '<span class="sq-metadata-cascade-wrapper">';
						check_box($prefix.'_cascade_value', '1', FALSE);
						label(translate('Cascade value'), $prefix.'_cascade_value');

					echo '</span>';
				}
			echo '</span>';

			$field_prefix = $prefix.'_value';
			if ($asset->attr('multi_line')) {
				// if there is no character limit display the original text_area
				if (!$asset->attr('char_limit')) {
					text_area($field_prefix, $value, $asset->attr('box_width'), $asset->attr('box_height'), 0, $extras);
				} else {
					text_area($field_prefix, $value, $asset->attr('box_width'), $asset->attr('box_height'), 0, 'id="'.htmlspecialchars($prefix.'_value').'" onchange="var note=document.getElementById(\''.$prefix.'_value_note\'); note.innerHTML = \'\'; var tmp = document.getElementById(\''.htmlspecialchars($prefix.'_value').'\'); tmp.value = tmp.value.substr(0,'.$asset->attr('char_limit').');"  onkeydown="setTimeout(function() { var note=document.getElementById(\''.$prefix.'_value_note\');  var tmp = document.getElementById(\''.htmlspecialchars($prefix.'_value').'\'); if (tmp.value.length > '.$asset->attr('char_limit').') {note.innerHTML = \'&nbsp;\'+js_translate(\'max_characters_exceeded_by\', (tmp.value.length-'.$asset->attr('char_limit').'));} else {note.innerHTML = \'\'}   }, 50);" '.$extras);
				}
			} else {
				text_box($field_prefix, $value, $asset->attr('box_width'), $asset->attr('char_limit'), FALSE, $extras);
			}

			// the following text can be used to display the character limit
			// we suppose that char limit is only a positive attribute
			if ($asset->attr('char_limit')) {
				?>
					<span class="sq-char-limit sq-metadata-char-limit">
						<span class="sq-char-limit-text">Character Limit: <strong><?php echo $asset->attr('char_limit'); ?></strong> </span>
						<span class="sq-char-limit-error" id="<?php echo $prefix.'_value'; ?>_note"></span>
					</span>
				<?php
			}

		} else {
			echo '<span class="sq-metadata-settings-wrapper">';
				if ($is_default) {
					echo ' <em>('.translate('Using default value').')</em>';

				} else{

				}
			echo '</span>';
			// ensure JS is not executed in read only mode
			echo htmlspecialchars($value);
		}

		// now print value components
		if (!empty($value_components)) {
			echo '<br /><br />';
			$num_per_row = 3;
			$num_done = 0;
			?>
			<table class="sq-backend-table sq-value-components">
				<tr>
					<?php
					foreach ($value_components as $vc_name => $vc_value) {

						if ($num_done >= $num_per_row) {
							$num_done = 0;
							echo '</tr><tr>';
						}

						echo '<td width="150" valign="top" class="sq-backend-table-cell">';
						echo '<b>'.$vc_name.'</b>';
						echo '<br />';
						if ($write_access) {
							$extras = ($is_default && $enable_default) ? 'disabled="disabled"' : '';
							text_area($prefix.'_vc_value['.$vc_name.']', $vc_value, 30, 4, 0, $extras);
						} else {
							echo $vc_value;
						}
						echo '</td>';

						$num_done++;
					}
					if ($num_done && $num_done != $num_per_row) {
						for ($i = ($num_per_row - $num_done); $i > 0; $i--) {
							echo'<td width="150" class="sq-backend-table-cell">&nbsp;</td>';
						}
					}
					?>
				</tr>
			</table>
			<?php
		}//end if

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Text	$asset			the asset whose interface we are processing
	* @param string					&$new_value_str	a string by reference that we can store the new values in
	* @param bool     &$field_cascade_value Whether value updates to this field should cascade
	*                                       to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Text $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
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
			if (isset($_POST[$prefix.'_value'])) {
				$value = $_POST[$prefix.'_value'];
				$keyword = '%asset_metadata_'.$asset->name.'%';
				$value = trim(str_replace($keyword, '', $value));
				$value_components = $asset->attr('value_components');
				for (reset($value_components); NULL !== ($vc_name = key($value_components)); next($value_components)) {
					if (isset($_POST[$prefix.'_vc_value'][$vc_name])) {
						$clean_vc = trim(str_replace($keyword, '', $_POST[$prefix.'_vc_value'][$vc_name]));
						$value_components[$vc_name] = str_replace(Array('\n', '\r'), Array(' ', ''), $clean_vc);
					}
				}
				$new_value_str = Metadata_Field::encodeValueString(trim($value), $value_components);

				$filter_enabled = $GLOBALS['SQ_SYSTEM']->frontendFilterEnabled();
				if ($filter_enabled && !empty($new_value_str)) {
					$new_value_str = filter_content($new_value_str);
					if ($new_value_str === FALSE){
						trigger_localised_error('SYS0347', E_USER_WARNING, $asset->name, $asset->type());
						return FALSE;
					}
				}

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
	* Paint the default value
	*
	* Needs a custom paint/process now, because for a non-contextable field,
	* it needs to show the default/zero context's values instead of the current
	* context's...ie. we need to make a contextable attribute look
	* not contextable!
	*
	* @param Metadata_Field_Text	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDefault(Metadata_Field_Text $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$value_str = $asset->attr('default');
		} else {
			$value_str = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$value_str = array_get_index($value_str, $asset->id, '');
		}

		$value_components = Array();

		if ($write_access === TRUE) {
			text_area($prefix.'_default', $value_str, 30, 5);
		} else {
			echo $value_str;
		}

		return TRUE;

	}//end paintDefault()


	/**
	* Process the default value
	*
	* @param Metadata_Field_Text	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDefault(Metadata_Field_Text $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes') === FALSE) {
			return FALSE;
		}
		$is_contextable = (boolean)$asset->attr('is_contextable');
		$new_value = array_get_index($_POST, $prefix.'_default', '');
		$save_attrs = TRUE;

		if ($is_contextable === TRUE) {
			$value = $asset->attr('default');
		} else {
			$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$value = array_get_index($value, $asset->id, '');
		}

		if (strpos($new_value, '%asset_metadata_') !== FALSE) {
			trigger_localised_error('SYS0344', E_USER_WARNING);
			$new_value = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $new_value);
		}


		if ($value !== $new_value) {
			if ($is_contextable === TRUE) {
				// If the metadata value is contextable, just use the contextable
				// default
				$asset->setAttrValue('default', $new_value);
			} else {
				// Not contextable; in this case the default value authority goes
				// to the default context
				$GLOBALS['SQ_SYSTEM']->changeContext(0);

				$default_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
				$default_asset->setAttrValue('default', $new_value);
				$default_asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($default_asset);
				unset($default_asset);

				$GLOBALS['SQ_SYSTEM']->restoreContext();

				// No need to save attributes in THIS context.
				$save_attrs = FALSE;
			}
		} else {
			// Nothing to save!
			$save_attrs = FALSE;
		}

		return $save_attrs;

	}//end processDefault()


}//end class
?>
