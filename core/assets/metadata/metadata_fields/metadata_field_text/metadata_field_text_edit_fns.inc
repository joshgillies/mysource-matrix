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
* $Id: metadata_field_text_edit_fns.inc,v 1.16 2005/12/16 05:43:27 lwright Exp $
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
* @version $Revision: 1.16 $
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
		$this->Metadata_Field_Edit_Fns();

	}//end constructor


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
		$prefix = $asset->getPrefix();

		$is_default = false;

		if (is_null($value_str)) {
			$value_str = $asset->getDefaultValue();
			$is_default = true;
		}

		$value = '';
		$value_components = $asset->attr('value_components');
		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		// set field header to friendly_name if it is not empty
		$field_name = $asset->attr('friendly_name');
		if (!$field_name) $field_name = $asset->name;
		if ($asset->attr('editable')) {
			if ($write_access) {
				if ($asset->attr('multi_line')) {
					text_area($prefix.'_value', $value, 45, 7, 0, 'onkeydown="document.getElementById(\''.$prefix.'_default\').checked = false;"');
				} else {
					text_box($prefix.'_value', $value, 30, '', false, 'onkeydown="document.getElementById(\''.$prefix.'_default\').checked = false;"');
				}
				echo ' '.strtolower(translate('or')).' ';
				check_box($prefix.'_default', '1', $is_default);
				echo ' '.strtolower(translate('use_default'));
				echo ' <em style="color: #666">('.$asset->attr('default').')</em>';
			} else {
					echo $value;
					if ($is_default) {
						echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
					}
			}

			// now print value components
			if (!empty($value_components)) {
				echo '<br /><br />';
				$num_per_row = 3;
				$num_done = 0;
				?>
				<table class="sq-backend-table">
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
								text_area($prefix.'_vc_value['.$vc_name.']', $vc_value, 30, 4);
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
		}
		return true;

	}//end paintValueInterface()


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
	function paintInlineValueInterface(&$asset, &$o, $value_str, $write_access)
	{
		$prefix = $asset->getPrefix();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$is_default = false;

		if (is_null($value_str)) {
			$value_str = $asset->getDefaultValue();
			$is_default = true;
		}

		$value = '';
		$value_components = $asset->attr('value_components');
		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		// set field header to friendly_name if it is not empty
		$field_name = $asset->attr('friendly_name');
		if (!$field_name) $field_name = $asset->name;

		if ($asset->attr('required')) {
			$field_name .= ' <span style="color: #FF0000">*</span>';
		}

		// use description as the note for this field
		$o->openField($field_name, '', $asset->attr('description'));
			if ($asset->attr('editable')) {
				if ($write_access) {
					if ($asset->attr('multi_line')) {
						text_area($prefix.'_value', $value, 45, 7, 0, ($is_default ? 'disabled="disabled" ' : '').'onkeydown="document.getElementById(\''.$prefix.'_default\').checked = false;"');
					} else {
						text_box($prefix.'_value', $value, 30, '', false, ($is_default ? 'disabled="disabled" ' : '').'onkeydown="document.getElementById(\''.$prefix.'_default\').checked = false;"');
					}
					echo ' &nbsp; &nbsp; &nbsp; ';
					check_box($prefix.'_default', '1', $is_default, 'with(this.form.elements[\''.$prefix.'_value\']) { disabled = this.checked; if (this.checked) { value = \''.addslashes($asset->getDefaultValue()).'\'; } }');
					echo ' '.translate('use_default');
					//echo ' <em style="color: #666">('.$asset->attr('default').')</em>';
				} else {
					echo $value;
					if ($is_default) {
						echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
					}
				}

				// now print value components
				if (!empty($value_components)) {
					echo '<br /><br />';
					$num_per_row = 3;
					$num_done = 0;
					?>
					<table class="sq-backend-table">
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
									text_area($prefix.'_vc_value['.$vc_name.']', $vc_value, 30, 4);
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

			} else {
				echo $value;

				// now print value components
				if (!empty($value_components)) {
					if (trim($value != '')) echo '<br />';
					?>
					<table border="0" cellspacing="1" cellpadding="1">
					<?php
					foreach ($value_components as $vc_name => $vc_value) {
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

				}//endif

			}//endif
		$o->closeField();

		return true;

	}//end paintInlineValueInterface()


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
		// nothing for us to do
		if (!$asset->attr('editable')) return true;

		$prefix = $asset->getPrefix();

		$value = array_get_index($_POST, $prefix.'_value', '');
		$value_components = $asset->attr('value_components');
		$use_default = array_get_index($_POST, $prefix.'_default', '');

		if ($use_default) {
			$new_value_str = null;
		} else {
			for (reset($value_components); null !== ($vc_name = key($value_components)); next($value_components)) {
				if (isset($_POST[$prefix.'_vc_value'][$vc_name])) {
					$value_components[$vc_name] = str_replace(Array('\n', '\r'), Array(' ', ''), trim($_POST[$prefix.'_vc_value'][$vc_name]));
				}
			}

			$new_value_str = Metadata_Field::encodeValueString(trim($value), $value_components);
		}

		return true;

	}//end processInlineValueInterface()


}//end class

?>
