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
* $Id: metadata_field_related_asset_edit_fns.inc,v 1.4 2012/09/26 03:48:02 cupreti Exp $
*
*/

require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Metadata_Field_Related_Asset_Edit_Fns
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Related_Asset_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* @param Metadata_Field_Related_Asset	$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter				$o				the outputter class
	* @param string							$value_str		the current value for this field in the owning asset (NOT $asset)
	*														NULL means that there is no current value set
	* @param boolean						$write_access	whether to allow editing of this field
	*														(typically depends on write access to owner and editability of field)
	* @param boolean						$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		$is_default = FALSE;
		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$default_value = $asset->attr('default');
		} else {
			$default_value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$default_value = array_get_index($value, $asset->id, '');
		}

		if (is_null($value_str)) {
			$is_default = TRUE;
			$value_str = $default_value;
		}
		$related_assetids = json_decode($value_str, TRUE);
		if (!is_array($related_assetids)) {
			$related_assetids = Array($value_str);
		}

		$related_assets_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($related_assetids, Array(), TRUE, 'type_code');

		// Filter out the invalid asset ids entries preserving the order
		// Also get the asset's type code list in the same order
		$related_asset_types = Array();
		if (!empty($related_assetids)) {
			foreach($related_assetids as $index => $related_assetid) {
				if (!isset($related_assets_info[$related_assetid])) {
					unset($related_assetids[$index]);
				} else {
					$related_asset_types[] = $related_assets_info[$related_assetid];
				}
			}//end foreach
		}//end if

		if ($write_access) {
			if($enable_default || $print_cascade_values_option) echo '<span class="sq-metadata-settings-wrapper">';
				if ($enable_default) {
					echo '<span class="sq-metadata-default-wrapper">';
						$default_string = str_replace("'", "\'", $asset->attr('default'));
						// get rid of javascript error 'unterminated string literal'. (bug 2386)
						$default_string = str_replace(Array("\r\n", "\r", "\n"), '\n', $default_string);

						$checkbox_extra = 'with(this.form.elements[\''.$prefix.'_value\']) { disabled = this.checked; if (this.checked) { value = \''.$default_string.'\'; } else {if(value==\'\') {value=\' \'; value = \'\';}}};';

						check_box($prefix.'_default', '1', $is_default, 'handleMetadataRelatedAssetDefaultClickFor'.$prefix.'()');
						label(translate('Use default'), $prefix.'_default');

					echo '</span>';
				}

				$allow_cascading = $asset->attr('allow_cascading'); 
				if ($print_cascade_values_option && $allow_cascading) {
					echo '<span class="sq-metadata-cascade-wrapper">';
						check_box($prefix.'_cascade_value', '1', FALSE);
						label(translate('Cascade value'), $prefix.'_cascade_value');

					echo '</span>';
				}
				
			if ($enable_default || $print_cascade_values_option) echo '</span>';
			echo '<div>';
			$ra_styles = ($is_default && $enable_default) ? 'display:none' : 'display:block';
			$asset_types = Array();
			$restrict_asset_types = $asset->attr('restrict_asset_types');
			foreach($restrict_asset_types as $asset_type => $inherit) {
				$asset_types[$asset_type] = $inherit ? 'D' : 'I';
			}
			echo '<div id="'.$prefix.'_ra_container'.'" style="float:left; '.$ra_styles.'" data-restricted-types="'.get_type_codes_string($asset_types).'" data-asset-types="'.implode('|', $related_asset_types).'" data-max-selections="'.$asset->attr('max_selections').'">';
			multiple_asset_finder($prefix.'_value', $related_assetids, $asset_types, 'sq_sidenav', FALSE, 'null', Array('clear'), FALSE, $asset->attr('max_selections'));
			echo '</div>';

			$default_ra_styles = !($is_default && $enable_default) ? 'display:none' : 'display:block';
			echo '<div id="'.$prefix.'_default_ra_container'.'" style="float:left; '.$default_ra_styles.'">';
			if (empty($default_value)) {
				echo '<em>'.translate('No default asset set.').'</em>';
			} else {
				echo '<div>';
				$default_related_assetids = json_decode($default_value, TRUE);
				if (!is_array($default_related_assetids)) {
					$default_related_assetids = Array($default_value);
				}
				$this->_printAssetTagLine($default_related_assetids);
				echo '</div>';
			}
			echo '</div>';
			?>
				<script type="text/javascript"><!--
				//<![CDATA[
					function handleMetadataRelatedAssetDefaultClickFor<?php echo $prefix; ?>() {
						var ra_div = document.getElementById('<?php echo $prefix.'_ra_container'; ?>');
						var default_ra_div = document.getElementById('<?php echo $prefix.'_default_ra_container'; ?>');

						if (ra_div.style.display == 'block') {
							ra_div.style.display = 'none';
							default_ra_div.style.display = 'block';
						} else {
							ra_div.style.display = 'block';
							default_ra_div.style.display = 'none';
						}
					}
				// ]]> -->
				</script>
		<?php
		
		} else {
			echo '<span class="sq-metadata-settings-wrapper">';
				if ($is_default) {
					echo ' <em>('.translate('Using default value').')</em>';

				} else{

				}
			echo '</span>';
			if (empty($related_assetids)) {
	            echo translate('No asset selected');
			} else {
				$this->_printAssetTagLine($related_assetids);
			}
		}
		echo '</div>';
		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Related_Asset	$asset					the asset whose interface we are processing
	* @param string							&$new_value_str			a string by reference that we can store the new values in
	* @param bool     						&$field_cascade_value 	Whether value updates to this field should cascade to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Related_Asset $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
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
			$new_value = array_get_index($_POST, $prefix.'_value', Array());
			$new_assetids = Array();
			foreach($new_value as $asset_info) {
				if (!empty($asset_info['assetid'])) {
					$new_assetids[] = is_numeric($asset_info['assetid']) ? (int) $asset_info['assetid'] : $asset_info['assetid'];
				}
			}
			$assets_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($new_assetids, Array(), TRUE, 'type_code');
			foreach(array_diff($new_assetids, array_keys($assets_info)) as $invalid_assetid) {
				trigger_localised_error('SYS0087', sprintf(translate('Asset #%s does not exist'), $invalid_assetid), E_USER_WARNING);
			}

			$restricted_asset_types = $asset->attr('restrict_asset_types');
			// We use original array instead of '$asset_info' to preserve the input asset ids order
			foreach($new_assetids as $index => $new_assetid) {
				if (empty($assets_info[$new_assetid])) {
					unset($new_assetids[$index]);
					continue;
				}

				if (!$this->_typeCodeMatch($assets_info[$new_assetid], $restricted_asset_types)) {
					 unset($new_assetids[$index]);
				}
			}
			// With single selection setting, metadata value is stored as plain string and multiple as json string
			if ($asset->attr('max_selections') <= 1) {
				$new_value_str = !empty($new_assetids) ? array_pop($new_assetids) : '';
			} else {
				$new_value_str = json_encode(array_unique($new_assetids));
			}
			$processed = TRUE;
		}

		if (isset($_POST[$prefix.'_cascade_value'])) {
			$field_cascade_value = TRUE;
			$processed = TRUE;
		}

		return $processed;

	}//end processInlineValueInterface()


	/**
	* Paint the "Restrict Asset Types" field
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintRestrictAssetTypes(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$types = $asset->attr('restrict_asset_types');
		if ($write_access) {
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_restrict_asset_types', TRUE, $display_values, NULL, FALSE, TRUE);
		} else {
			if (empty($types)) {
				 echo translate('No asset types restriction set.');
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
				<table class="sq-backend-table">
					<tr>
						<th><?php echo translate('Type') ?></th>
						<th><?php echo translate('Inherit?') ?></th>
					</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.png';
					$inherit_alt = $inherit ? translate('Yes') : translate('No');
					?>
					<tr>
						<td><?php echo get_asset_type_icon($type).$type_names[$type]; ?></td>
						<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
					</tr>
					<?php
				}//end foreach
				echo '</table>';
			}//end else
		}//end else

		return TRUE;

	}//end paintRestrictAssetTypes()


	/**
	* Process the "Restrict Asset Types" field
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processRestrictAssetTypes(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$success = FALSE;
		if (isset($_POST[$prefix.'_restrict_asset_types'])) {
			$existing_types = $asset->attr('restrict_asset_types');

			$submitted_types = Array();
			foreach ($_POST[$prefix.'_restrict_asset_types']['type_code'] as $i => $type_code) {
				if (!empty($type_code)) {
					$submitted_types[$type_code] = $_POST[$prefix.'_restrict_asset_types']['inherit'][$i];
				}
			}//enf foreach

			$success = $asset->setAttrValue('restrict_asset_types', $submitted_types);
		}//end if

		return $success;

	}//end processRestrictAssetTypes()


	/**
	* Paint the default value
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDefault(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$value = $asset->attr('default');
		} else {
			$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$value = array_get_index($value, $asset->id, '');
		}

		$assetids = Array();
		if (!empty($value)) {
			$assetids = json_decode($value, TRUE);
			if (!is_array($assetids)) {
				$assetids = Array($value);
			}
		}

		if (!$write_access) {
			if (empty($assetids)) {
				echo translate('No asset selected');
			} else {
				$this->_printAssetTagLine($assetids);
			}
		} else {

			$asset_types = Array();
			$restrict_asset_types = $asset->attr('restrict_asset_types');
			foreach($restrict_asset_types as $asset_type => $inherit) {
				$asset_types[$asset_type] = $inherit ? 'D' : 'I';
			}
			multiple_asset_finder($prefix.'_default', $assetids, $asset_types, 'sq_sidenav', FALSE, 'null', Array('clear'), FALSE, $asset->attr('max_selections'));
		}

		return TRUE;

	}//end paintDefault()


	/**
	* Process the default value
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDefault(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes') === FALSE) {
			return FALSE;
		}
		$new_value = array_get_index($_POST, $prefix.'_default', Array());
		$new_assetids = Array();
		foreach($new_value as $asset_info) {
			if (!empty($asset_info['assetid'])) {
				$new_assetids[] = is_numeric($asset_info['assetid']) ? (int) $asset_info['assetid'] : $asset_info['assetid'];
			}
		}

		$max_selections = $asset->attr('max_selections');
		if (isset($_POST[$asset->getPrefix().'_max_selections'])) {
			$max_selections = $_POST[$asset->getPrefix().'_max_selections'];
		}
		// With single selection setting, metadata value is stored as plain string and multiple as json string
		if ($max_selections <= 1) {
			$new_value = !empty($new_assetids) ? $new_assetids[0] : '';
		} else {
			$new_value = json_encode($new_assetids);
		}

		$is_contextable = (boolean)$asset->attr('is_contextable');
		if ($is_contextable === TRUE) {
			$value = $asset->attr('default');
		} else {
			$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$value = array_get_index($value, $asset->id, '');
		}

		$save_value = FALSE;
		if ($new_value != $value) {
			// Validate specified asset ids
			$assets_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($new_assetids, Array(), TRUE, 'type_code');
			foreach(array_diff($new_assetids, array_keys($assets_info)) as $invalid_assetid) {
				trigger_localised_error('SYS0087', sprintf(translate('Asset #%s does not exist'), $invalid_assetid), E_USER_WARNING);
			}

			$restricted_asset_types = $asset->attr('restrict_asset_types');
			// We use original array instead of '$assets_info' to preserve the input asset ids order
			foreach($new_assetids as $index => $new_assetid) {
				if (empty($assets_info[$new_assetid])) {
					unset($new_assetids[$index]);
					continue;
				}

				// Check type code restriction
				if (!$this->_typeCodeMatch($assets_info[$new_assetid], $restricted_asset_types)) {
					 unset($new_assetids[$index]);
				}
			}

			if ($max_selections <= 1) {
				$new_value = !empty($new_assetids) ? $new_assetids[0] : '';
			} else {
				$new_value = json_encode(array_unique($new_assetids));
			}
			$save_value = TRUE;
			if ($is_contextable === TRUE) {
				// If the metadata value is contextable, just use the contextable default
				$asset->setAttrValue('default', $new_value);
				$save_value = TRUE;
			} else {
				// Not contextable; in this case the default value authority goes to the default context
				$GLOBALS['SQ_SYSTEM']->changeContext(0);

				$default_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
				$default_asset->setAttrValue('default', $new_value);
				$default_asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($default_asset);
				unset($default_asset);

				$GLOBALS['SQ_SYSTEM']->restoreContext();

				// No need to save attributes in THIS context.
				$save_value = FALSE;
			}
		}

		return $save_value;

	}//end processDefault()


	/**
	* Paint the "Max Selection" field
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintMaxSelection(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$value = $asset->attr('max_selections');
	   	if (!$write_access) {
			echo $value;
        } else {
			$extra_js = 'onkeyup="validate_numeric_text_field(this, true);" onblur="validate_numeric_range(this, false);" onchange="validate_numeric_text_field(this, true);"';
			text_box($asset->getPrefix().'_max_selections', $value, 10, '', FALSE, $extra_js);
		}

		return TRUE;

	}//end paintMaxSelection()


	/**
	* Process the "Max Selection" field
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processMaxSelection(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$success = FALSE;
		if ($write_access) {
			$new_value = array_get_index($_POST, $asset->getPrefix().'_max_selections', '1');
			if ($new_value < 1 || !preg_match('/[0-9]+/', $new_value)) {
				$new_value = 1;
			}
			if ($new_value != $asset->attr('max_selections')) {
				$success = $asset->setAttrValue('max_selections', $new_value);
			}
		}

		return $success;

	}//end processMaxSelection()


	/**
	* Paint the asset ids list
	*
	* @param array $assetids
	*
	* @return void
	* @access private
	*/
	private function _printAssetTagLine($assetids)
	{
		$multiple = count($assetids) > 1;
		echo $multiple ? '<ul style="margin: 1px 15px">' : '';
		foreach($assetids as $assetid) {
			echo $multiple ? '<li>' : '';
			echo get_asset_tag_line($assetid);
			echo $multiple ? '</li>' : '';
		}
		echo $multiple ? '</ul>' : '';

	}//end _printAssetTagLine()


	/**
	* Returns TRUE if the given type code satisfies the given type code restriction setting
	*
	* @param string $asset_type
	* @param array  $restriction
	*
	* @return boolean
	* @access private
	*/
	private function _typeCodeMatch($asset_type, $restriction)
	{
		$type_code_match = TRUE;
		// Check type code restriction
		if (!empty($restriction)) {
			$type_code_match = FALSE;
			foreach ($restriction as $type_code_restricted => $inherit) {
				if ($type_code_restricted == $asset_type) {
					$type_code_match = TRUE;
					break;
				}
				if ($inherit) {
					$asset_type_with_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($asset_type);
					foreach($asset_type_with_parents as $type_code) {
						if ($type_code == $type_code_restricted) {
							$type_code_match = TRUE;
							break 2;
						}//end if
					}//end foreach
				}//end if
			}//end foreach
		}

		return $type_code_match;

	}//end _typeCodeMatch()

}//end class
?>
