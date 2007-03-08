<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_text_edit_fns.inc,v 1.26 2007/03/08 04:00:38 tbarrett Exp $
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
* @version $Revision: 1.26 $
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
	* Returns true if a submitable interface has been printed
	*
	* @param object		&$asset			the metadata field asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$value_str		the current value for this field in the owning asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param boolean	$write_access	whether to allow editing of this field
	*									(typically depends on write access to owner and editability of field)
	* @param boolean	$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(&$asset, &$o, $value_str, $write_access, $enable_default=TRUE)
	{
		$prefix = $asset->getPrefix();

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
			if ($asset->attr('multi_line')) {
				text_area($prefix.'_value', $value, 45, 7, 0, $extras);
			} else {
				text_box($prefix.'_value', $value, 30, '', FALSE, $extras);
			}
			if ($enable_default) {
				echo ' &nbsp; &nbsp; &nbsp; ';
				check_box($prefix.'_default', '1', $is_default, 'with(this.form.elements[\''.$prefix.'_value\']) { disabled = this.checked; if (this.checked) { value = \''.str_replace("'", "\'", $asset->attr('default')).'\'; } }');
				label(translate('use_default'), $prefix.'_default');
			}
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

		return $write_access;

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
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$prefix = $asset->getPrefix();
		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			return TRUE;
		} else {
			if (isset($_POST[$prefix.'_value'])) {
				$value = $_POST[$prefix.'_value'];
				$value_components = $asset->attr('value_components');
				for (reset($value_components); NULL !== ($vc_name = key($value_components)); next($value_components)) {
					if (isset($_POST[$prefix.'_vc_value'][$vc_name])) {
						$value_components[$vc_name] = str_replace(Array('\n', '\r'), Array(' ', ''), trim($_POST[$prefix.'_vc_value'][$vc_name]));
					}
				}
				$new_value_str = Metadata_Field::encodeValueString(trim($value), $value_components);
				return TRUE;
			} else {
				// nothing submitted
				return FALSE;
			}
		}

	}//end processInlineValueInterface()


}//end class
?>