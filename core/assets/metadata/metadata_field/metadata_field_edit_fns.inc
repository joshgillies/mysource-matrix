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
* $Id: metadata_field_edit_fns.inc,v 1.39 2006/01/26 22:34:04 lwright Exp $
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
* @version $Revision: 1.39 $
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
	function Metadata_Field_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for viewing keyword instructions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintKeywordInstructions(&$asset, &$o, $prefix)
	{
		ob_start();
			?><a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">click here </a></b><?php
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
	function processKeywordInstructions(&$asset, &$o, $prefix)
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
	function paintValueComponents(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			text_box($prefix.'_new_vc', '', 30);
		} else {
			echo '<i>'.translate('core_cannot_create_value_component').'</i>';
		}
		$o->closeField();

		$current_vc = $asset->attr('value_components');
		if (!empty($current_vc)) {
			$o->openField(translate('current'));
			$num_per_row = 3;
			$num_done = 0;
			?>
			<table class="sq-backend-table">
				<tr>
					<?php
					foreach ($current_vc as $vc_name => $vc_value) {
						if ($num_done >= $num_per_row) {
							$num_done = 0;
							echo '</tr><tr>';
						}

						echo '<td width="150" valign="top" class="sq-backend-table-cell">';
						if ($write_access) {
							text_box($prefix.'_vc_name['.$vc_name.']', $vc_name, 30);
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
							echo '<br />';
							check_box($prefix.'_vc_delete['.$vc_name.']');
							label(translate('delete_question'), $prefix.'_vc_delete['.$vc_name.']');
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
	function processValueComponents(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$current_vc = $asset->attr('value_components');
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
				trigger_localised_error('CORE0206', E_USER_WARNING, $asset->name);
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
			$asset->setAttrValue('value_components', $current_vc);
			return TRUE;
		}

		return FALSE;

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
	function paintInlineValueInterface(&$asset, &$o, $value_str, $write_access)
	{
		// set field header to friendly_name if it is not empty
		$field_name = $asset->attr('friendly_name');
		if (!$field_name) $field_name = $asset->name;

		if ($asset->attr('required')) {
			$field_name .= ' <span style="color: #FF0000">*</span>';
		}

		// use description as the note for this field
		$o->openField($field_name, '', $asset->attr('description'));
		$res = $this->paintValueInterface($asset, $o, $value_str, $write_access);
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
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(&$asset, &$o, $value_str, $write_access)
	{
		return FALSE;

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
		return FALSE;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object	&$asset		the asset whose metadata we are checking
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $value_str)
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
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$frontend_option = $asset->attr('frontend');
		// if we aren't printing this on the frontend then don't print anything
		if ($frontend_option == 'hide') return TRUE;

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

		echo '<meta name="'.$asset->name.'"';
		if (trim($asset->attr('language')) != '') {
			echo ' lang="'.$asset->attr('language').'"';
		}
		if (trim($asset->attr('scheme'))   != '') {
			echo ' scheme="'.$asset->attr('scheme').'"';
		}

		echo ' content="<?php echo $metadata_values[\''.$asset->name.'\']; ?>" />'."\n";

		$tag_value = $value_str;

		return TRUE;

	}//end generateMetadata()


}//end class

?>
