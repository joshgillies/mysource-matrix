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
* $Id: metadata_field_edit_fns.inc,v 1.67 2013/10/18 04:30:22 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Metadata_Field_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.67 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @return object
	* @access public
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for viewing keyword instructions
	*
	* @param Metadata_Field		$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintKeywordInstructions(Metadata_Field $asset, Backend_Outputter $o, $prefix)
	{
		ob_start();
			?><a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('lib')?>/web/popups/keywords.php?assetid=<?php echo $asset->id; ?>&all=1', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">click here </a></b><?php
			$replacements_link = ob_get_contents();
		ob_end_clean();
		$o->note(translate('metadata_keyword_replacements_usage', $replacements_link));

	}//end paintKeywordInstructions()


	/**
	* Processes the interface forview keyword instructions
	*
	* Note that keyword instructions are not processed so this function always
	* returns false
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processKeywordInstructions(Metadata_Field $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end processKeywordInstructions()


	/**
	* Paint the interface for editing value components
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintValueComponents(Metadata_Field $asset, Backend_Outputter $o, $prefix)
	{
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$is_default_context = ($contextid === 0) ? TRUE : FALSE;
		$is_contextable = $asset->attr('is_contextable');
		$can_change_keys = ($is_contextable === FALSE) || ($is_default_context = TRUE);

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access_perm = $asset->writeAccess();
		$write_access = $asset->writeAccess('attributes');

		if ($write_access_perm === TRUE) {
			if ($can_change_keys === TRUE) {
				if ($write_access === TRUE) {
					text_box($prefix.'_new_vc', '', 30);
				} else {
					// Would have permission if we had the asset locked
					echo '<i>'.translate('core_lock_asset_to_create_value_component').'</i>';
				}
			} else {
				// Can't add/delete components in an alternate context
				echo '<em>'.translate('core_cannot_create_value_component_in_alternate_context').'</em>';
			}
		} else {
			// Would not have permission to do it even if able to be locked
			echo '<i>'.translate('core_cannot_create_value_component').'</i>';
		}
		$o->closeField();

		// Get the value components for the current context (if contextable)
		// or the default context (if not)
		$default_vc = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('value_components', 'metadata_field', Array($asset->id), 0);
		$default_vc = unserialize($default_vc[$asset->id]);

		if ($can_change_keys === FALSE) {
			$current_vc = $asset->attr('value_components');
		} else {
			$current_vc = $default_vc;
		}

		if (!empty($current_vc)) {
			$o->openField(translate('current'));
			$num_per_row = 3;
			$num_done = 0;
			?>
			<table class="sq-backend-table sq-value-components">
				<tr>
					<?php
					foreach ($default_vc as $vc_name => $default_vc_value) {
						$vc_value = array_get_index($current_vc, $vc_name, $default_vc_value);

						if ($num_done >= $num_per_row) {
							$num_done = 0;
							echo '</tr><tr>';
						}

						echo '<td width="150" valign="top" class="sq-backend-table-cell">';
						if ($write_access === TRUE) {
							if ($can_change_keys === FALSE) {
								// If in alternate context, don't let the user edit the field name
								// but do send it to the process function, because it needs to be
								// saved to this context on commit
								echo '<b>'.$vc_name.'</b>';
								hidden_field($prefix.'_vc_name['.$vc_name.']', $vc_name);
							} else {
								text_box($prefix.'_vc_name['.$vc_name.']', $vc_name, 30);
							}
						} else {
							echo '<b>'.$vc_name.'</b>';
						}
						echo '<br />';
						if ($write_access) {
							text_area($prefix.'_vc_value['.$vc_name.']', $vc_value, 30, 4);
						} else {
							echo $vc_value;
						}
						if ($write_access) {
							if ($can_change_keys === TRUE) {
								echo '<br />';
								check_box($prefix.'_vc_delete['.$vc_name.']');
								label(translate('delete_question'), $prefix.'_vc_delete['.$vc_name.']');
							}
						}
						echo '</td>';

						$num_done++;
					}
					if ($num_done && $num_done != $num_per_row) {
						for ($i = ($num_per_row - $num_done); $i > 0; $i--) {
							echo'<td width="200" class="sq-backend-table-cell">&nbsp;</td>';
						}
					}
					?>
				</tr>
			</table>
			<?php
		}//end if

		return TRUE;

	}//end paintValueComponents()


	/**
	* Processes the interface for editing value components
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processValueComponents(Metadata_Field $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		$is_contextable = (boolean)$asset->attr('is_contextable');
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if (($is_contextable === FALSE) && ($contextid !== 0)) {
			$GLOBALS['SQ_SYSTEM']->changeContext(0);
			$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
		} else {
			$contexted_asset = $asset;
		}

		$current_vc = $contexted_asset->attr('value_components');
		$updated = FALSE;

		// edit the existing value components first
		foreach ($current_vc as $vc_name => $vc_value) {
			if (isset($_POST[$prefix.'_vc_delete'][$vc_name]) && $_POST[$prefix.'_vc_delete'][$vc_name]) {
				unset($current_vc[$vc_name]);
				$updated = TRUE;
				continue;
			}

			if (isset($_POST[$prefix.'_vc_name'][$vc_name])) {
				$new_vc_name  = trim($_POST[$prefix.'_vc_name'][$vc_name]);
			}
			if (isset($_POST[$prefix.'_vc_value'][$vc_name])) {
				$new_vc_value = trim($_POST[$prefix.'_vc_value'][$vc_name]);
			}
			if (empty($new_vc_name)) continue;

			// change the default value of a value component
			if ($vc_value != $new_vc_value) {
				$current_vc[$vc_name] = $new_vc_value;
				$updated = TRUE;
			}

			// change the name of a value component
			if ($vc_name != $new_vc_name) {
				$current_vc[$new_vc_name] = $current_vc[$vc_name];
				unset($current_vc[$vc_name]);
				$updated = TRUE;
			}
		}//end foreach

		// create a new value component if required
		if (isset($_POST[$prefix.'_new_vc']) && trim($_POST[$prefix.'_new_vc']) != '') {
			$new_vc = trim($_POST[$prefix.'_new_vc']);
			if (isset($current_vc[$new_vc])) {
				trigger_localised_error('CORE0206', E_USER_WARNING, $contexted_asset->name);
			} else if (preg_match('/^[1-9][0-9]+$/', $new_vc)) {
				// we are not allowing just numbers because we want to use array_merge()
				// and if there is only numbers it thinks the elements are in from an indexed array and it all goes bad.
				trigger_localised_error('CORE0207', E_USER_WARNING);
			} else {
				$current_vc[$new_vc] = '';
				$updated = TRUE;
			}
		}//end if

		if ($updated) {
			$contexted_asset->setAttrValue('value_components', $current_vc);
		}

		if (($is_contextable === FALSE) && ($contextid !== 0)) {
			// Commit the contexted asset; we don't need to commit the one
			// in the current context in this instance
			if ($updated) {
				$contexted_asset->saveAttributes();
				$updated = FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($contexted_asset);
			$GLOBALS['SQ_SYSTEM']->restoreContext();
		}

		return $updated;

	}//end processValueComponents()


	/**
	* Paint the field with the interface for filling in metadata values
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$value_str		the current value for this field in the asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param boolean	$write_access	does the current user have write access to the asset
	*									that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(Metadata_Field $asset, Backend_Outputter $o, $value_str, $write_access, $print_cascade_values_option = TRUE, $type_code=NULL)
	{
		$complete = $this->requiredFieldsComplete($asset, $value_str);
		// set field header to friendly_name if it is not empty
		$field_name = $asset->attr('friendly_name');
		if (!$field_name) $field_name = $asset->name;

		if ($asset->attr('required')) {
			$field_name .= ' <span class="sq-backend-warning">*</span>';
			if (!$complete && $write_access) {
				$field_name .= '<br /><span class="sq-backend-warning">'.translate('currently_empty').'</span>';
			}
		}

		// use description as the note for this field
		$o->openField($field_name, '');
		$writable = $write_access && $asset->attr('editable');
		$res = $this->paintValueInterface($asset, $o, $value_str, $writable, TRUE, $print_cascade_values_option, $type_code);
		if (!$write_access && $asset->attr('required') && !$complete) {
			echo '<b class="sq-backend-warning">['.strtoupper(translate('empty')).']</b>';
		}
		echo '<div class="sq-backend-smallprint sq-metadata-description">'.$asset->attr('description').'</div>';
		$o->closeField();
		return $res;

	}//end paintInlineValueInterface()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$value_str		the current value for this field in the asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param boolean	$write_access	does the current user have write access to the asset
	*									that we are setting metadata for (NOT THE SAME AS $asset)
	* @param boolean	$enable_default	Whether to show the 'use default' checkbox
	* @param string		$type_code      The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(Metadata_Field $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		return FALSE;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* Returns true if this field has been processed, false if not and it should be ignored
	*
	* @param Metadata_Field	$asset			the asset whose interface we are processing
	* @param string			&$new_value_str	a string by reference that we can store the new values in
	*										set to NULL to revert to default
	* @param bool     &$field_cascade_value Whether value updates to this field should cascade
	*                                       to children
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(Metadata_Field $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		return FALSE;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param Metadata_Field	$asset		the asset whose metadata we are checking
	* @param string			$value_str	the current value for this field in the asset (NOT $asset)
	*									NULL means that there is no current value set
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(Metadata_Field $asset, $value_str)
	{
		// if this is not a required field then of course it doesnt break the rule
		if (!$asset->attr('required')) return TRUE;

		if (is_null($value_str)) {
			$value = $asset->attr('default');
			$value_components = $asset->attr('value_components');
		} else {
			$value='';
			$value_components = $asset->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}

		if (trim($value) != '') return TRUE;

		// now check value components to see if they are complete
		foreach ($value_components as $vc_name => $vc_value) {
			if (trim($vc_value) != '') return TRUE;
		}

		return FALSE;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tag for this field
	*
	* @param Metadata_Field	$asset		the asset whose interface we are painting
	* @param string			$value_str	the current value for this field in the asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param array			&$tag_value	the metadata tag value
	* @param array			&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(Metadata_Field $asset, $value_str, &$tag_value, Array &$keywords)
	{
		// No name means we can't reference a value from the metadata array,
		// so we would be writing meta tags with PHP errors in them.
		if (trim($asset->name) === '') {
			return TRUE;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$frontend_option = $asset->attr('frontend');
		// if we aren't printing this on the frontend then don't print anything

		if (is_null($value_str)) {
			$value = $asset->attr('default');
			$value_components = $asset->attr('value_components');
			$value_str = Metadata_Field::encodeValueString($value, $value_components);
		} else {
			$value='';
			$value_components = $asset->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}

		$keywords = array_merge($keywords, retrieve_keywords_replacements($value_str, '.'));

		$empty_vc = TRUE;

		foreach ($value_components as $vc_name => $vc_value) {
			// if any value component is not empty then the whole thing is not empty
			if ($vc_value != '') $empty_vc = FALSE;
		}

		// if we are empty and it must be filled to be printed, then don't print anything
		if (($frontend_option == 'fill') && $value == '' && $empty_vc) {
			return TRUE;
		}

		$tag_value = $value_str;

		if ($frontend_option == 'hide') return TRUE;

		$name_text = ($asset->attr('field_name_text') == '') ? 'name' : $asset->attr('field_name_text');
		$value_text = ($asset->attr('value_name_text') == '') ? 'content' : $asset->attr('value_name_text');

		echo '<meta '.$name_text.'="'.$mm->escapeMetadata($asset->name).'"';
		if (trim($asset->attr('language')) != '') {
			echo ' lang="'.$mm->escapeMetadata($asset->attr('language')).'"';
		}
		if (trim($asset->attr('scheme'))   != '') {
			echo ' scheme="'.$mm->escapeMetadata($asset->attr('scheme')).'"';
		}

		echo ' '.$value_text.'="<?php echo $mm->escapeMetadata($metadata_values[\''.$mm->escapeMetadata(addslashes($asset->name), TRUE).'\']); foreach ($metadata_value_components[\''.$mm->escapeMetadata(addslashes($asset->name), TRUE).'\'] as $k => $v) {echo \'; \'.$k.\'=\'.$v; } ?>" />'."\n";


		return TRUE;

	}//end generateMetadata()


	/**
	* Show-if that returns TRUE if we can get away with the default edit
	* interface for a field
	*
	* @param Metadata_Field	$asset	The asset being painted/processed
	*
	* @return boolean
	*/
	public function canUsePlainDefault(Metadata_Field $asset)
	{
		return !($this->requiresCustomDefault($asset));

	}//end isContextableField()


	/**
	* Show-if that returns TRUE if we may need a custom default field
	*
	* We may require a custom default to replace the normal field if it is a
	* non-contextable field, and we are not in a default context. In this case
	* we should be redirected to a paint/process which will use the default
	* value of the default context.
	*
	* @param Metadata_Field	$asset	The asset being painted/processed
	*
	* @return boolean
	*/
	public function requiresCustomDefault(Metadata_Field $asset)
	{
		if (($asset->attr('is_contextable') === FALSE) && ($GLOBALS['SQ_SYSTEM']->getContextId() !== 0)) {
			return TRUE;
		} else {
			return FALSE;
		}

	}//end isNotContextableField()

}//end class

?>
