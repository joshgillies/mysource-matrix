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
* $Id: metadata_field_multiple_text_edit_fns.inc,v 1.14 2013/09/20 01:56:05 lwright Exp $
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
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Multiple_Text_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* @param Metadata_Field_Multiple_Text	&$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter				&$o				the outputter class
	* @param string							$value_str		the current value for this field in the owning asset (NOT $asset)
	*														NULL means that there is no current value set
	* @param boolean						$write_access	whether to allow editing of this field
	*														(typically depends on write access to owner and editability of field)
	* @param boolean						$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Multiple_Text $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$value = '';
		$value_components = $asset->attr('value_components');

		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		$is_default = FALSE;
		if ($enable_default) {
			$o->addJsInclude(sq_web_path('data').'/asset_types/metadata_field_multiple_text/js/metadata_field_multiple_text.js');
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
		if ($write_access) {

			if($enable_default || $print_cascade_values_option) echo '<span class="sq-metadata-settings-wrapper">';
				if ($enable_default) {
					echo '<span class="sq-metadata-default-wrapper">';
						// Work out which options we have to set if we have to
						$i = 0;
						$default_keys = str_replace('\'', '\\\'', $asset->attr('default'));
						$default_keys = explode('; ', $default_keys);
						$default_vals_expr = '[\''.implode('\', \'', $default_keys).'\']';
						check_box($prefix.'_default', '1', $is_default, "handleDefaultClickMT(this, '$prefix', $default_vals_expr); ");
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
				
			if($enable_default || $print_cascade_values_option) echo '</span>';

			echo '<div class="sq-metadata-multitext-wrapper">';
				if ($enable_default) { echo '<span id="'.$prefix.'_field">';}
						$attribute->paint($prefix, FALSE);
					echo '</span>';
				if ($enable_default && $is_default) {
					echo '</span>';
					?>
					<script type="text/javascript"><!--
						setInputsEnabled(document.getElementById('<?php echo $prefix ?>_field'), false);
					// --></script>
					<?php
				}
			echo '</div>';

		} else {
			echo '<span class="sq-metadata-settings-wrapper">';
				if ($is_default) {
					echo ' <em>('.translate('Using default value').')</em>';

				} else{

				}
			echo '</span>';
			echo '<div class="sq-metadata-multitext-wrapper">';
				$attribute->paint($prefix, TRUE);
			echo '</div>';
		}

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Multiple_Text	$asset			the asset whose interface we are processing
	* @param string							&$new_value_str	a string by reference that we can store the new values in
	* @param bool     &$field_cascade_value Whether value updates to this field should cascade
	*                                       to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Multiple_Text $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		// nothing for us to do
		if (!$asset->attr('editable')) return FALSE;

		$processed = FALSE;

		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			// not default
			$attribute = $asset->getOptionListAttribute();
			$attribute->process($prefix);
			if ($attribute->processed) {
				// Get the string into proper form for the metadata field function
				$value = str_replace($attribute->delimiter, '; ', $attribute->value);
				$keyword = '%asset_metadata_'.$asset->name.'%';
				$value = trim(str_replace($keyword, '', $value));
				$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());

				$filter_enabled = $GLOBALS['SQ_SYSTEM']->frontendFilterEnabled();
				if ($filter_enabled && !empty($new_value_str)) {
					$new_value_str = filter_content($new_value_str);
					if ($new_value_str === FALSE){
						trigger_localised_error('SYS0347', sprintf(translate('Unable to filter content of Metadata Field "%1$s" of Type "%2$s"'), $asset->name, $asset->type()), E_USER_WARNING);
						return FALSE;
					}
				}

				$processed = TRUE;
			}
		}

		if (isset($_POST[$prefix.'_cascade_value'])) {
			$field_cascade_value = TRUE;
			$processed = TRUE;
		}

		return $processed;

	}//end processInlineValueInterface()


	/**
	* Output the complete metadata tag for this field
	*
	* @param Metadata_Field_Multiple_Text	$asset		the asset whose interface we are painting
	* @param string							$value_str	the current value for this field in the asset (NOT $asset)
	*													NULL means that there is no current value set
	* @param array							&$tag_value	the metadata tag value
	* @param array							&$keywords	an array we can put keywords in that need replacing
	* @param boolean        				$content_file   Whether the generated metadata will be included in a metadata content file
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Field_Multiple_Text $asset, $value_str, &$tag_value, Array &$keywords, $content_file=TRUE)
	{
		if (is_null($value_str)) {
			$value_str = $asset->attr('default');
		}

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords, $content_file);

	}//end generateMetadata()


	/**
	* Paint the interface for choosing default value
	*
	* @param Metadata_Field_Multiple_Text	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter object
	* @param string							$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function paintDefault(Metadata_Field_Multiple_Text $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$attribute = $asset->getOptionListAttribute();
		$attribute->paint($prefix, !$write_access);

		return $write_access;

	}//end paintDefault()


	/**
	* Process the interface for choosing default value
	*
	* @param Metadata_Field_Multiple_Text	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter object
	* @param string							$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	public function processDefault(Metadata_Field_Multiple_Text $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$is_contextable = (boolean)$asset->attr('is_contextable');
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		// change to default asset if we have to
		if ($is_contextable === FALSE) {
			if ($contextid !== 0) {
				$GLOBALS['SQ_SYSTEM']->changeContext(0);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			}
		}

		$attribute = $asset->getOptionListAttribute();
		$attribute->process($prefix);

		if ($attribute->processed) {
			$value = $attribute->value;

			if (strpos($value, '%asset_metadata_') !== FALSE) {
				trigger_localised_error('SYS0344', translate('Cannot use asset_metadata_XXX keyword for metadata field value. Replacing asset_metadata_XXX keyword by metadata_field_XXX'), E_USER_WARNING);
				$value = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $value);
			}
			$value_array = explode($attribute->delimiter, $value);
			$asset->setAttrValue('default', implode('; ', $value_array));
			$asset->saveAttributes();
		}

		// restore previous assets
		if ($is_contextable === FALSE) {
			if ($contextid !== 0) {
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			}
		}

		// No need to save attributes in THIS context
		return FALSE;

	}//end processDefault()


}//end class

?>
