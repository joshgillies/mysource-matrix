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
* $Id: metadata_field_text_edit_fns.inc,v 1.1.2.1 2004/02/18 14:36:28 brobertson Exp $
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
class Metadata_Field_Text_Edit_Fns extends Metadata_Field_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Metadata_Field_Text_Edit_Fns()
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


}//end class

?>
