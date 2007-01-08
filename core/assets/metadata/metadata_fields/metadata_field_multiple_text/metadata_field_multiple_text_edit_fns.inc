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
* $Id: metadata_field_multiple_text_edit_fns.inc,v 1.2 2007/01/08 02:47:52 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Multiple_Text_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Multiple_Text_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Multiple_Text_Edit_Fns()
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
	* @param boolean	$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(&$asset, &$o, $value_str, $write_access, $enable_default=TRUE)
	{
		$prefix = $asset->getPrefix();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$value = '';
		$value_components = $asset->attr('value_components');

		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		if ($enable_default) {
			$o->addJsInclude(sq_web_path('data').'/asset_types/metadata_field_multiple_text/js/metadata_field_multiple_text.js');
			$is_default = FALSE;
			if (is_null($value_str)) {
				$value_str = $asset->getDefaultValue();
				Metadata_Field::decodeValueString($value_str, $value, $value_components);
				$is_default = TRUE;
			}
		}

		// if there is no value we need a blank array...not an array with one element (a blank string)
		if ($value == '') {
			$value = Array();
		} else {
			$value = explode('; ', $value);
		}

		$attribute = $asset->getOptionListAttribute();
		$attribute->value = implode($attribute->delimiter, $value);

		// If we're OK to edit, get our attribute, and paint it
		if ($write_access && $asset->attr('editable')) {
			if ($enable_default) {
				echo '<span id="'.$prefix.'_field">';
			}
			$attribute->paint($prefix, FALSE);
			if ($enable_default) echo '</span>';
		} else {
			$attribute->paint($prefix, TRUE);
		}

		if ($enable_default) {
			if ($write_access) {
				echo ' &nbsp; &nbsp; &nbsp; ';

				// Work out which options we have to set if we have to
				$i = 0;
				$default_keys = explode('; ', $asset->attr('default'));
				$default_vals_expr = '[\''.implode('\', \'', $default_keys).'\']';
				check_box($prefix.'_default', '1', $is_default, "handleDefaultClickMT(this, '$prefix', $default_vals_expr); ");
				label(translate('use_default'), $prefix.'_default');
				if ($is_default) {
					?>
					<script type="text/javascript"><!--
						setInputsEnabled(document.getElementById('<?php echo $prefix ?>_field'), false);
					// --></script>
					<?php
				}
			} else {
				if ($is_default) {
					echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
				}
			}
		}

		return TRUE;

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
		// nothing for us to do
		if (!$asset->attr('editable')) return TRUE;
		$prefix = $asset->getPrefix();

		$use_default = array_get_index($_POST, $prefix.'_default', '');

		if ($use_default) {
			$new_value_str = NULL;
			return TRUE;
		}

		// Default is not checked, so process ur select attribute
		$attribute = $asset->getOptionListAttribute();
		$attribute->process($prefix);

		// Get the string into proper form for the metadata field function
		$value = str_replace($attribute->delimiter, '; ', $attribute->value);
		$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());

		return TRUE;

	}//end processInlineValueInterface()


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
		if (is_null($value_str)) {
			$value_str = $asset->attr('default');
		}

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords);

	}//end generateMetadata()


	function paintDefault(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$attribute =& $asset->getOptionListAttribute();
		$attribute->paint($prefix, !$write_access);

		return $write_access;

	}//end paintDefault()


	function processDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$attribute =& $asset->getOptionListAttribute();
		$attribute->process($prefix);

		if ($attribute->processed) {
			$value = $attribute->value;
			$value_array = explode($attribute->delimiter, $value);
			$asset->setAttrValue('default', implode('; ', $value_array));
		} else {
			return FALSE;
		}

		return $asset->saveAttributes();

	}//end processDefault()


}//end class

?>
