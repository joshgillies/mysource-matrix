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
* $Id: metadata_field_select_edit_fns.inc,v 1.2.2.4 2004/03/26 16:41:50 brobertson Exp $
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

			if ($asset->attr('editable')) {
				if ($write_access) {	
					combo_box($prefix.'_value', $select_options, false, $default);
				} else {
					echo $select_options[$default];
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

				foreach ($select_options as $option_key => $option_value) {

				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $option_key; ?></td>
					<td class="sq-backend-table-cell" align="left"><?php echo $option_value; ?></td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-cell"><?php check_box($prefix.'_options[]', $option_key); ?></td><?php
					}
					?>
				</tr>
				<?php

				} // end foreach select_options

				?></table><?php

			// displaying default value if any values were actually saved in the array
			$o->openField('Default Value');
			if ($write_access) {
				combo_box($prefix.'_default', $select_options, false, $default);
			} else {
				echo $select_options[$default];
			}
			$o->closeField();

		} else {
			echo 'There are currently no select box options';
		} // end if empty

		// now adding two fields for new entry (key => value)
		if ($write_access) {
			$o->openField('Option Key');
				text_box($prefix.'_select_key','');
			$o->closeField();

			$o->openField('Option Value');
				text_box($prefix.'_select_val','');
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

		$select_options = $asset->attr('select_options');
		
		$update = false;

		// 1). Deleting checked items

		if (isset($_POST[$prefix.'_options'])) {
			foreach ($_POST[$prefix.'_options'] as $option_key) {
				unset($select_options[$option_key]);
			}
			$update = true;
		}

		// 2). now get the new submitted values (if any) and add them to array

		if (!empty($_POST[$prefix.'_select_key']) && !empty($_POST[$prefix.'_select_val'])) {
			$new_select_key  = trim($_POST[$prefix.'_select_key']);
			$new_select_val  = trim($_POST[$prefix.'_select_val']);

			$select_options[$new_select_key] = $new_select_val;
			$update = true;
		}
			
		// 3). Updating asset attribute value
		if ($update) {
			$asset->setAttrValue('select_options', $select_options);
		} 

		// 4). Saving default value anyway
		$new_default = trim($_POST[$prefix.'_default']);
		$asset->setAttrValue('default', $new_default); // saving anyway

		return true;

	}//end processSelection()


}//end class

?>
