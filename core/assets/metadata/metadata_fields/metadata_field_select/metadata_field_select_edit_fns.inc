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
* $Id: metadata_field_select_edit_fns.inc,v 1.8 2004/10/19 01:05:12 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Text_Edit_Fns
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 1.0
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
		Metadata_Field_Edit_Fns::Metadata_Field_Edit_Fns(); // calling super

	}//end constructor


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

			$select_options = $asset->attr('select_options');

			if ($asset->attr('multiple')) $default = explode('; ', $default);
			if ($asset->attr('editable')) {
				combo_box($prefix.'_value', $select_options, $asset->attr('multiple'), $default);
			} else {
				if (!is_array($default)) $default = Array($default);
				$default_string = '';
				foreach ($default as $option) $default_string .= $select_options[$option].'; ';
				echo trim($default_string, '; ');
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
			$value = array_get_index($_POST, $prefix.'_value', '');
			if (is_array($value)) $value = implode('; ', $value);
			$new_values[$asset->name]['value'] = trim($value);
		}

		return true;

	}//end processInlineValueInterface()


	/**
	* Paint the interface for dropdown selection box
	*
	* @param object Metadata_Field_Selection	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSelection(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$select_options = $asset->attr('select_options');
		$default = $asset->attr('default');

		if (!empty($select_options)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">Option Key</td>
					<td class="sq-backend-table-header">Option Value</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete ?</td><?php
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
			echo 'There are currently no select box options';
		}//end if empty

		// now adding two fields for new entry (key => value)
		if ($write_access) {
			$o->openField('New Option');
				echo '<b>key</b> ';
				text_box($prefix.'_new_key','');
				echo ' <b>value</b> ';
				text_box($prefix.'_new_val','');
			$o->closeField();
		}
		
		if (!empty($select_options)) {
			// displaying default value if any values were actually saved in the array
			$o->openField('Default Value');
				$selection = $asset->getSelectionAttribute();
				$selection->paint($prefix.'_default', !$write_access);
			$o->closeField();
		}

		return true;

	}//end paintSelection()


	/**
	* Processes the interface for dropdown selection box
	*
	* @param object Metadata_Field_Selection	$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSelection(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;
		
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
			if (is_array($new_default)) $new_default = implode('; ', $new_default);
		}
		if (!$asset->attr('multiple')) {
			if (!isset($select_options[$new_default])) $new_default = '';
		}
		$asset->setAttrValue('default', $new_default);

		return true;

	}//end processSelection()
	
	
	/**
	* Paint the interface for setting selection edit params
	*
	* @param object Metadata_Field_Selection	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEditParams(&$asset, &$o, $prefix)
	{
		$selection = $asset->getSelectionAttribute();
		$selection->_edit_params = $asset->attr('edit_params');
		$selection->paintEditParams($prefix.'_style', $asset->writeAccess('attributes'));

	}//end paintEditParams()


	/**
	* Processes the interface for setting selection edit params
	*
	* @param object Metadata_Field_Selection	&$asset	the asset to which we belong
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditParams(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$selection = $asset->getSelectionAttribute();
		$edit_params = $selection->processEditParams($prefix.'_style');
		return $asset->setAttrValue('edit_params', $edit_params);

	}//end processEditParams()


}//end class

?>
