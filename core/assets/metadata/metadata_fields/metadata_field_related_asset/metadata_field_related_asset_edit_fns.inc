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
* $Id: metadata_field_related_asset_edit_fns.inc,v 1.4 2012/09/26 03:48:02 cupreti Exp $
*
*/

require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Metadata_Field_Related_Asset_Edit_Fns
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Related_Asset_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* @param Metadata_Field_Related_Asset	$asset			the metadata field asset whose interface we are painting
	* @param Backend_Outputter				$o				the outputter class
	* @param string							$value_str		the current value for this field in the owning asset (NOT $asset)
	*														NULL means that there is no current value set
	* @param boolean						$write_access	whether to allow editing of this field
	*														(typically depends on write access to owner and editability of field)
	* @param boolean						$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$prefix = $asset->getPrefix();
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		
		$is_default = FALSE;
		$default_value = trim($asset->getDefaultValue());
		if (is_null($value_str)) {
			$is_default = TRUE;
			$value_str = $default_value;
		}
		$value_str = trim($value_str);

		if ($write_access) {
			$ra_styles = ($is_default && $enable_default) ? 'display:none' : 'display:block';
    	    echo '<div id="'.$prefix.'_ra_container'.'" style="float:left; '.$ra_styles.'">';
			asset_finder($prefix.'_value', $value_str);
			echo '</div>';

			$default_ra_styles = !($is_default && $enable_default) ? 'display:none' : 'display:block';
    	    echo '<div id="'.$prefix.'_default_ra_container'.'" style="float:left; '.$default_ra_styles.'">';
			echo !$default_value ? 'No default asset!' : get_asset_tag_line($default_value);
			echo '</div>';

			if ($enable_default) {
				echo ' &nbsp; &nbsp; &nbsp; ';
				$default_string = str_replace("'", "\'", $asset->attr('default'));
				// get rid of javascript error 'unterminated string literal'. (bug 2386)
				$default_string = str_replace(Array("\r\n", "\r", "\n"), '\n', $default_string);

				$checkbox_extra = 'with(this.form.elements[\''.$prefix.'_value\']) { disabled = this.checked; if (this.checked) { value = \''.$default_string.'\'; } else {if(value==\'\') {value=\' \'; value = \'\';}}};';

				check_box($prefix.'_default', '1', $is_default, 'handleMetadataRelatedAssetDefaultClickFor'.$prefix.'()');
				label(translate('use_default'), $prefix.'_default');
			}

			if ($print_cascade_values_option) {
				echo '&nbsp;&nbsp;&nbsp;&nbsp;';
				check_box($prefix.'_cascade_value', '1', FALSE);
				label(translate('cascade_value'), $prefix.'_cascade_value');
			}

			?>
				<script type="text/javascript"><!--
				//<![CDATA[
					function handleMetadataRelatedAssetDefaultClickFor<?php echo $prefix; ?>() {
						var ra_div = document.getElementById('<?php echo $prefix.'_ra_container'; ?>');
						var default_ra_div = document.getElementById('<?php echo $prefix.'_default_ra_container'; ?>');
	
						if (ra_div.style.display == 'block') {
							ra_div.style.display = 'none';
							default_ra_div.style.display = 'block';
						} else {
							ra_div.style.display = 'block';
							default_ra_div.style.display = 'none';
						}
					}
				// ]]> -->
				</script>
		<?php

		} else {
            echo !$value_str ? translate('no_asset_selected') : get_asset_tag_line($value_str);
			if ($is_default) {
				echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
			}
		}

		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Related_Asset	$asset					the asset whose interface we are processing
	* @param string							&$new_value_str			a string by reference that we can store the new values in
	* @param bool     						&$field_cascade_value 	Whether value updates to this field should cascade to children
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_Related_Asset $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;

		$prefix = $asset->getPrefix();
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			if (isset($_POST['sq_asset_finder_'.$prefix.'_value_assetid'])) {
				$value = $_POST['sq_asset_finder_'.$prefix.'_value_assetid'];
			} else {
				return $processed;
			}

			if ($value && !$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($value))) {
				trigger_localised_error('SYS0087', E_USER_WARNING, $value);
				return FALSE;
			}
			$new_value_str = $value;
			$processed = TRUE;
		}

		if (isset($_POST[$prefix.'_cascade_value'])) {
			$field_cascade_value = TRUE;
			$processed = TRUE;
		}

		return $processed;

	}//end processInlineValueInterface()


	/**
	* Paint the default value
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDefault(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');


		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$value = $asset->attr('default');
		} else {
			$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$value = array_get_index($value, $asset->id, '');
		}

	   	if (!$write_access) {
            if (!$value) {
                echo translate('no_asset_selected');
            } else {
                echo get_asset_tag_line($value);
            }
        } else {
    	    asset_finder($prefix.'_default', $value);
		}

		return TRUE;

	}//end paintDefault()


	/**
	* Process the default value
	*
	* @param Metadata_Field_Related_Asset	$asset	the asset whose interface we are painting
	* @param Backend_Outputter				$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDefault(Metadata_Field_Related_Asset $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes') === FALSE) {
			return FALSE;
		}
		$new_value = trim(array_get_index($_POST, 'sq_asset_finder_'.$prefix.'_default_assetid', ''));

		$is_contextable = (boolean)$asset->attr('is_contextable');
		if ($is_contextable === TRUE) {
			$value = $asset->attr('default');
		} else {
			$value = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$value = array_get_index($value, $asset->id, '');
		}

		$save_value = FALSE;
		if ($new_value != $value) {
			if ($new_value && !$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($new_value))) {
				trigger_localised_error('SYS0087', E_USER_WARNING, $new_value);
				return FALSE;
			}
			$save_value = TRUE;
			if ($is_contextable === TRUE) {
				// If the metadata value is contextable, just use the contextable default
				$asset->setAttrValue('default', $new_value);
				$save_value = TRUE;
			} else {
				// Not contextable; in this case the default value authority goes to the default context
				$GLOBALS['SQ_SYSTEM']->changeContext(0);

				$default_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
				$default_asset->setAttrValue('default', $new_value);
				$default_asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($default_asset);
				unset($default_asset);

				$GLOBALS['SQ_SYSTEM']->restoreContext();

				// No need to save attributes in THIS context.
				$save_value = FALSE;
			}
		}

		return $save_value;

	}//end processDefault()


}//end class
?>
