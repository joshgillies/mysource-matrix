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
* $Id: metadata_field_multiple_text_edit_fns.inc,v 1.13 2012/11/07 02:28:38 akarelia Exp $
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
* @version $Revision: 1.13 $
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
		$prefix = $asset->getPrefix();
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
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
		if ($write_access) {
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
				$default_keys = str_replace('\'', '\\\'', $asset->attr('default'));
				$default_keys = explode('; ', $default_keys);
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

		if ($write_access && $print_cascade_values_option) {
			echo '&nbsp;&nbsp;&nbsp;&nbsp;';
			check_box($prefix.'_cascade_value', '1', FALSE);
			label(translate('cascade_value'), $prefix.'_cascade_value');
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

		$prefix = $asset->getPrefix();
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

				if(!SQ_IN_BACKEND && !SQ_IN_LIMBO && !SQ_IN_LOGIN && !SQ_IN_CRON && !SQ_PHP_CLI) {
					$filter_enabled = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_FILTER_FRONT_END_INPUT');
					if(isset($GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] ) && $GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION']) {
						$filter_enabled = FALSE;
					} else if (isset($GLOBALS['SQ_CLONE_COMPONENTS'] ) && $GLOBALS['SQ_CLONE_COMPONENTS']) {
						$filter_enabled = FALSE;
					}
					
					if ($filter_enabled && !empty($new_value_str)) {
						$new_value_str = filter_content($new_value_str);
						if ($new_value_str === FALSE){
							trigger_localised_error('SYS0347', E_USER_WARNING, $asset->name, $asset->type());
							return FALSE;
						}
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
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Field_Multiple_Text $asset, $value_str, &$tag_value, Array &$keywords)
	{
		if (is_null($value_str)) {
			$value_str = $asset->attr('default');
		}

		return parent::generateMetadata($asset, $value_str, $tag_value, $keywords);

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
