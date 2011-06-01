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
* $Id: metadata_field_wysiwyg_edit_fns.inc,v 1.2 2009/03/22 21:51:03 akarelia Exp $
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
* @version $Revision: 1.2 $
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
	*
	* @return boolean
	* @access public
	*/
	public function paintValueInterface(Metadata_Field_WYSIWYG $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE)
	{
		$prefix = $asset->getPrefix();

		$wysiwyg = $asset->getAttribute('wysiwyg');

		$value = '';
		$value_components = $asset->attr('value_components');

		$is_default = FALSE;
		if (is_null($value_str)) {
			$value_str = $asset->attr('default_html');
			$is_default = TRUE;
		}

		Metadata_Field::decodeValueString($value_str, $value, $value_components);
		$wysiwyg->value = $value;
		$styles_common = '"float: left;';
		$wysiwyg_container_styles = $is_default ? $styles_common.'display:none"' : $styles_common.'display:block"';
		echo '<div id="'.$prefix.'_wysiwyg_container'.'" style='.$wysiwyg_container_styles.'>';
		$wysiwyg->paint($prefix, !$write_access);
		echo '</div>';
		$default_html_container_styles = $is_default ? $styles_common.'display:block"' : $styles_common.'display:none"';
		echo '<div id="'.$prefix.'_default_html_container'.'" style='.$default_html_container_styles.'>';
		echo $asset->attr('default_html');
		if (!$write_access) {
			echo ' <em style="color: #666">('.strtolower(translate('default')).')</em>';
		}
		echo '</div>';

		if ($write_access) {
			echo '<div style="float:left">';
			echo '&nbsp;&nbsp;&nbsp;&nbsp;';
			check_box($prefix.'_default', '1', $is_default, 'handleMetadataWYSIWYGDefaultClick()');
			label(translate('use_default'), $prefix.'_default');

			if ($print_cascade_values_option) {
				echo '&nbsp;&nbsp;&nbsp;&nbsp;';
				check_box($prefix.'_cascade_value', '1', FALSE);
				label(translate('cascade_value'), $prefix.'_cascade_value');
			}
			echo '</div>';
		}
		?>
			<script type="text/javascript"><!--
			//<![CDATA[
				function handleMetadataWYSIWYGDefaultClick() {
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
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Field_WYSIWYG $asset, &$new_value_str, &$field_cascade_value)
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;
		$prefix = $asset->getPrefix();
		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			// default is not checked, so process the attribute
			$wysiwyg = $asset->getAttribute('wysiwyg');
			$wysiwyg->process($prefix);
			if ($wysiwyg->processed) {
				$new_value_str = Metadata_Field::encodeValueString(trim($wysiwyg->value), Array());
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


}//end class
?>
