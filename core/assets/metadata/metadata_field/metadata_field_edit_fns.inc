<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Metadata_Field_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Metadata_Field_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paint the interface for viewing keyword instructions
	*
	* @param object Metadata_Field		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintKeywordInstructions(&$asset, &$o, $prefix)
	{
		?>
		<b>You can use keyword replacements in place of values for this metadata field.<br/>
		These values can be used in the main <i>Default</i> value of the field as well as in the values of any <i>Value Components</i> the field has defined.<br/>For a complete list of available keyword replacements <a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">click here </a></b><?php

	}//end paintKeywordInstructions()


	/**
	* Processes the interface forview keyword instructions
	*
	* Note that keyword instructions are not processed so this function always
	* returns false
	*
	* @param object Metadata_Field		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean (false always)
	* @access public
	*/
	function processKeywordInstructions(&$asset, &$o, $prefix)
	{
		return false;

	}//end processKeywordInstructions()


	/**
	* Paint the interface for editing value components
	*
	* @param object Metadata_Field		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintValueComponents(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess();

		if ($write_access) {
			text_box($prefix.'_new_vc', '', 30);
		} else {
			echo '<i>You do not have permission to create a new value component</i>';
		}
		$o->closeField();

		$current_vc = $asset->attr('value_components');
		if (!empty($current_vc)) {
			$o->openField('Current');
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
						if ($write_access) text_box($prefix.'_vc_name['.$vc_name.']', $vc_name, 30);
						else echo '<b>'.$vc_name.'</b>';
						echo '<br />';
						if ($write_access) text_area($prefix.'_vc_value['.$vc_name.']', $vc_value, 30, 4);
						else echo $vc_value;
						if ($write_access) {
							echo '<br />';
							check_box($prefix.'_vc_delete['.$vc_name.']');
							echo ' Delete ?';
						}
						echo '</td>';

						$num_done++;
					}
					if ($num_done && $num_done != $num_per_row) {
						for ($i = ($num_per_row - $num_done); $i > 0; $i--) echo'<td width="200" class="sq-backend-table-cell-alt">&nbsp;</td>';
					}
					?>
				</tr>
			</table>
			<?php
		}

		return true;

	}//end paintValueComponents()


	/**
	* Processes the interface for editing value components
	*
	* @param object Metadata_Field		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processValueComponents(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;

		$current_vc = $asset->attr('value_components');
		$updated = false;

		// edit the existing value components first
		foreach ($current_vc as $vc_name => $vc_value) {
			if (isset($_POST[$prefix.'_vc_delete'][$vc_name]) && $_POST[$prefix.'_vc_delete'][$vc_name]) {
				unset($current_vc[$vc_name]);
				$updated = true;
				continue;
			}

			if (isset($_POST[$prefix.'_vc_name'][$vc_name]))  $new_vc_name  = trim($_POST[$prefix.'_vc_name'][$vc_name]);
			if (isset($_POST[$prefix.'_vc_value'][$vc_name])) $new_vc_value = trim($_POST[$prefix.'_vc_value'][$vc_name]);
			if (empty($new_vc_name)) continue;
			
			// change the default value of a value component
			if ($vc_value != $new_vc_value) {
				$current_vc[$vc_name] = $new_vc_value;
				$updated = true;
			}

			// change the name of a value component
			if ($vc_name != $new_vc_name) {
				$current_vc[$new_vc_name] = $current_vc[$vc_name];
				unset($current_vc[$vc_name]);
				$updated = true;
			}
		}//end foreach

		// create a new value component if required
		if (isset($_POST[$prefix.'_new_vc']) && trim($_POST[$prefix.'_new_vc']) != '') {
			$new_vc = trim($_POST[$prefix.'_new_vc']);
			if (isset($current_vc[$new_vc])) {
				trigger_error('Value component "'.$new_vc.'" already exists for field "'.$asset->name.'"', E_USER_WARNING);
			} else {
				$current_vc[$new_vc] = '';
				$updated = true;
			}
		}//end if

		if ($updated) {
			$asset->setAttrValue('value_components', $current_vc);
			return true;
		}

		return false;

	}//end processValueComponents()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Field		&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param array						$values			current metadata values set for this asset
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access)
	{
		$prefix = $asset->getPrefix();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$field_name = $asset->name;
		if ($asset->attr('required')) $field_name .= ' <span style="color: #FF0000">*</span>';

		$o->openField($field_name);
			if (isset($values['value'])) $default = $values['value'];
			else $default = $asset->attr('default');

			if ($asset->attr('editable')) {
				if ($write_access) {
					if ($asset->attr('multi_line')) text_area($prefix.'_value', $default, 30, 4);
					else text_box($prefix.'_value', $default, 30);
				} else {
					echo $default;
				}
				
				// now print value components
				$current_vc = $asset->attr('value_components');
				if (!empty($current_vc)) {
					echo '<br /><br />';
					$num_per_row = 3;
					$num_done = 0;
					?>
					<table class="sq-backend-table">
						<tr>
							<?php
							foreach ($current_vc as $vc_name => $vc_value) {
								if (isset($values['value_components'][$vc_name])) {
									$vc_value = $values['value_components'][$vc_name];
								}

								if ($num_done >= $num_per_row) {
									$num_done = 0;
									echo '</tr><tr>';
								}

								echo '<td width="150" valign="top" class="sq-backend-table-cell">';
								echo '<b>'.$vc_name.'</b>';
								echo '<br />';
								if ($write_access) text_area($prefix.'_vc_value['.$vc_name.']', $vc_value, 30, 4);
								else echo $vc_value;
								echo '</td>';

								$num_done++;
							}
							if ($num_done && $num_done != $num_per_row) {
								for ($i = ($num_per_row - $num_done); $i > 0; $i--) echo'<td width="200" class="sq-backend-table-cell-alt">&nbsp;</td>';
							}
							?>
						</tr>
					</table>
					<?php
				}
			} else {
				echo $default;

				// now print value components
				$current_vc = $asset->attr('value_components');
				if (!empty($current_vc)) {
					if (trim($default != '')) echo '<br />';
					?>
					<table border="0" cellspacing="1" cellpadding="1">
					<?php
					foreach ($current_vc as $vc_name => $vc_value) {
						if (isset($values['value_components'][$vc_name])) {
							$vc_value = $values['value_components'][$vc_name];
						}
						?>
						<tr>
							<td valign="top" align="right" class="sq-backend-data"><b><?php echo $vc_name; ?></b></td>
							<td valign="top" align="left" class="sq-backend-data"><?php echo $vc_value; ?></td>
						</tr>
						<?php
					}
					?>
					</table>
					<?php
				}
			}
		$o->closeField();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Field	&$asset			the asset whose interface we are painting
	* @param array					&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$prefix = $asset->getPrefix();

		$default = $asset->attr('default');
		if ($asset->attr('editable')) {
			if (isset($_POST[$prefix.'_value'])) {
				$new_values[$asset->name]['value'] = trim($_POST[$prefix.'_value']);
			}

			$current_vc = $asset->attr('value_components');
			if (!empty($current_vc)) {
				$new_values[$asset->name]['value_components'] = Array();
				foreach ($current_vc as $vc_name => $vc_value) {
					if (isset($_POST[$prefix.'_vc_value'][$vc_name])) {
						$new_values[$asset->name]['value_components'][$vc_name] = trim($_POST[$prefix.'_vc_value'][$vc_name]);
					}
				}
			}
		}

		return true;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object Metadata_Field	&$asset		the asset whose metadata we are checking
	* @param array					$values		the current metadata values
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $values)
	{
		// if this is not a required field then of course it doesnt break the rule
		if (!$asset->attr('required')) return true;

		// check for the main value being complete
		if (isset($values['value'])) $default = $values['value'];
		else $default = $asset->attr('default');
		$default = trim($default);
		if (!empty($default)) return true;

		// now check value components to see if they are complete
		$current_vc = $asset->attr('value_components');
		if (!empty($current_vc)) {
			foreach ($current_vc as $vc_name => $vc_value) {
				if (isset($values['value_components'][$vc_name])) {
					$vc_value = $values['value_components'][$vc_name];
				}
				$vc_value = trim($vc_value);
				if (!empty($vc_value)) return true;
			}
		}

		return false;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tag for this field
	*
	* @param object Metadata_Field	&$asset		the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $values, &$keywords)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		if (isset($values['value'])) $default = $values['value'];
		else $default = $asset->attr('default');

		$keywords = array_merge($keywords, retrieve_keywords_replacements($default));

		echo '<meta NAME="'.$asset->name.'"';
		if (trim($asset->attr('language')) != '') echo ' LANG="'.$asset->attr('language').'"';
		if (trim($asset->attr('scheme')) != '') echo ' SCHEME="'.$asset->attr('scheme').'"';

		$value_string = '';
		$current_vc = $asset->attr('value_components');
		if (!empty($current_vc)) {
			foreach ($current_vc as $vc_name => $vc_value) {
				if (isset($values['value_components'][$vc_name])) {
					$vc_value = $values['value_components'][$vc_name];
				}
				$keywords = array_merge($keywords, retrieve_keywords_replacements($vc_value));
				$value_string .= $vc_name.'='.$vc_value.'; ';
			}
		}

		echo ' CONTENT="';
		if (trim($default) != '') {
			echo trim($default);
			if (trim($value_string) != '') echo '; ';
		}
		if (trim($value_string) != '') echo trim($value_string);
		echo "\">\n";

		return true;

	}//end generateMetadata()

}//end class

?>