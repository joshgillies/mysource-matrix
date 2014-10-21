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
* $Id: metadata_field_wysiwyg_edit_fns.inc,v 1.10 2013/09/20 01:56:05 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';
require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';

/**
* Metadata_Field_WYSIWYG_Edit_Fns
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_WYSIWYG_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* @param Metadata_Field_Text	$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter		$o				the outputter class
	* @param string					$value_str		the current value for this field in the owning asset (NOT $asset)
	*												NULL means that there is no current value set
	* @param boolean				$write_access	whether to allow editing of this field
	*												(typically depends on write access to owner and editability of field)
	* @param boolean				$enable_default	Whether to show the 'use default' checkbox
	* @param string             	$type_code      The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_WYSIWYG $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		$wysiwyg = $asset->getAttribute('wysiwyg');

		$value = '';
		$value_components = $asset->attr('value_components');

		$is_default = FALSE;
		if (is_null($value_str)) {
			$value_str = $asset->attr('default_html');
			$is_default = TRUE;
		}

		if ($write_access) {
			echo '<div class="sq-metadata-settings-wrapper inline">';
			check_box($prefix.'_default', '1', $is_default, 'handleMetadataWYSIWYGDefaultClickFor'.$prefix.'()');
			label(translate('Use default'), $prefix.'_default');


			if ($print_cascade_values_option) {
				echo '&nbsp;&nbsp;&nbsp;&nbsp;';
				check_box($prefix.'_cascade_value', '1', FALSE);
				label(translate('Cascade value'), $prefix.'_cascade_value');

			}
			echo '</div>';
		} else {
			if ($is_default) {
				echo '<div class="sq-metadata-settings-wrapper inline">';
					echo ' <em>('.translate('Using default value').')</em>';

				echo '</div>';
			} else{

			}
		}

		Metadata_Field::decodeValueString($value_str, $value, $value_components);
		$wysiwyg->value = $value;
		$styles_common = '"';
		$wysiwyg_container_styles = $is_default ? $styles_common.'display:none"' : $styles_common.'display:block"';
		$wysiwyg_preview_class = $write_access ? '' : 'sq-metadata-wysiwyg-preview';
		echo '<div id="'.$prefix.'_wysiwyg_container'.'" style='.$wysiwyg_container_styles.' class="sq-metadata-wysiwyg-edit-wrapper '.$wysiwyg_preview_class.'">';
			$wysiwyg->paint($prefix, !$write_access);
		echo '</div>';
		$default_html_container_styles = $is_default ? $styles_common.'display:block"' : $styles_common.'display:none"';
		echo '<div id="'.$prefix.'_default_html_container'.'" style='.$default_html_container_styles.' class="sq-metadata-wysiwyg-preview">';
			echo $asset->attr('default_html');
		echo '</div>';
		?>
			<script type="text/javascript"><!--
			//<![CDATA[
				function handleMetadataWYSIWYGDefaultClickFor<?php echo $prefix; ?>() {
					var wysiwyg_container = document.getElementById('<?php echo $prefix.'_wysiwyg_container'; ?>');
					var default_html_container = document.getElementById('<?php echo $prefix.'_default_html_container'; ?>');
					if (wysiwyg_container.style.display == 'block') {
						wysiwyg_container.style.display = 'none';
						default_html_container.style.display = 'block';
					} else {
						wysiwyg_container.style.display = 'block';
						default_html_container.style.display = 'none';
					}
				}
			// ]]> -->
			</script>
		<?php

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Text	$asset					the asset whose interface we are processing
	* @param string					&$new_value_str			a string by reference that we can store the new values in
	* @param bool  					&$field_cascade_value	whether value updates to this field should cascade to children
	* @param string             	$type_code      		The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_WYSIWYG $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;
		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			// default is not checked, so process the attribute
			$wysiwyg = $asset->getAttribute('wysiwyg');
			$wysiwyg->process($prefix, NULL, FALSE);
			if ($wysiwyg->processed) {
				$new_value_str = Metadata_Field::encodeValueString(trim($wysiwyg->value), Array());

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
	* @param Metadata_Field_Select	$asset		the asset whose interface we are painting
	* @param string					$value_str	the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	* @param array					&$tag_value	the metadata tag value
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Field_WYSIWYG $asset, $value_str, &$tag_value, Array &$keywords)
	{
		// generate default value correctly
		if (is_null($value_str)) {
			$value_str = $asset->attr('default_html');
		}

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords);

	}//end generateMetadata()


	/**
	* Paint the default value
	*
	* Needs a custom paint/process now, because for a non-contextable field,
	* it needs to show the default/zero context's values instead of the current
	* context's...ie. we need to make a contextable attribute look
	* not contextable!
	*
	* @param Metadata_Field_Text	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDefault(Metadata_Field_WYSIWYG $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$value_str = $asset->attr('default_html');
		} else {
			$value_str = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default_html', 'metadata_field_wysiwyg', Array($asset->id), 0);
			$value_str = array_get_index($value_str, $asset->id, '');
		}

		$value = '';
		$value_components = Array();
		Metadata_Field::decodeValueString($value_str, $value, $value_components);

		$wysiwyg = $asset->getAttribute('default_html');
		$wysiwyg->_edit_params['height'] = '100';
		$wysiwyg->value = $value;
		$wysiwyg->paint($prefix, !$write_access);

		return TRUE;

	}//end paintDefault()


	/**
	* Process the default value
	*
	* @param Metadata_Field_Text	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDefault(Metadata_Field_WYSIWYG $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes') === FALSE) {
			return FALSE;
		}

		$wysiwyg = $asset->getAttribute('wysiwyg');
		$wysiwyg->process($prefix, NULL, FALSE);
		if ($wysiwyg->processed) {
			$new_value = Metadata_Field::encodeValueString(trim($wysiwyg->value), Array());
		} else {
			return FALSE;
		}

		$is_contextable = (boolean)$asset->attr('is_contextable');
		$save_attrs = TRUE;

		if ($is_contextable === TRUE) {
			$value = $asset->attr('default_html');
		} else {
			$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default_html', 'metadata_field_wysiwyg', Array($asset->id), 0);
			$value = array_get_index($value, $asset->id, '');
		}

		if (strpos($new_value, '%asset_metadata_') !== FALSE) {
			trigger_localised_error('SYS0344', translate('Cannot use asset_metadata_XXX keyword for metadata field value. Replacing asset_metadata_XXX keyword by metadata_field_XXX'), E_USER_WARNING);
			$new_value = preg_replace('/%asset_metadata_([^\s]*?)%/', '%metadata_field_$1%', $new_value);
		}

		if ($value !== $new_value) {
			if ($is_contextable === TRUE) {
				// If the metadata value is contextable, just use the contextable
				// default
				$asset->setAttrValue('default_html', $new_value);
			} else {
				// Not contextable; in this case the default value authority goes
				// to the default context
				$GLOBALS['SQ_SYSTEM']->changeContext(0);

				$default_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
				$default_asset->setAttrValue('default_html', $new_value);
				$default_asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($default_asset);
				unset($default_asset);

				$GLOBALS['SQ_SYSTEM']->restoreContext();

				// No need to save attributes in THIS context.
				$save_attrs = FALSE;
			}
		} else {
			// Nothing to save!
			$save_attrs = FALSE;
		}

		return $save_attrs;

	}//end processDefault()


}//end class
?>
