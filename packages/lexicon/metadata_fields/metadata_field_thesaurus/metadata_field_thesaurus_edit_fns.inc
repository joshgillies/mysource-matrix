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
* $Id: metadata_field_thesaurus_edit_fns.inc,v 1.4 2005/04/27 06:55:16 lwright Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Thesaurus_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Metadata_Field_Thesaurus_Edit_Fns extends Metadata_Field_Edit_Fns
{
	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Field		&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param string						$value_str		the current value for this field in the asset (NOT $asset)
	*													NULL means that there is no current value set
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $value_str, $write_access)
	{
		$prefix = $asset->getPrefix();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if (is_null($value_str)) {
			$value = $asset->attr('default');
			$value_components = $asset->attr('value_components');
		} else {
			$value='';
			$value_components = $asset->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}

		$datapath = sq_web_path('data').'/asset_types/metadata_field_thesaurus/files';

		$field_name = $asset->name;
		if ($asset->attr('required')) $field_name .= ' <span style="color: #FF0000">*</span>';

		$communicator_url = $asset->getCommunicatorUrl();
		$o->openField($field_name);

		// thesaurus has to be active for successful operation
		if (!$asset->isThesaurusActive()) {
			echo '<span style="color: #FF0000">'.translate('lexicon_linked_thesaurus_inactive').'</span>';
		} else {
			if ($write_access) {
				$text_box_extra = ($asset->attr('allow_manual'))?'':'readonly="readonly"';

				text_box($prefix.'_value', $value, 30, '', false, $text_box_extra);
				echo '<input type="button" style="width:20px; height:20px; background: url('.$datapath.'/images/thesaurus_button.gif); border: none" onclick="'.$prefix.'_browser.open(event)" />';
				echo '<script type="text/javascript" src="'.$datapath.'/thesaurus_browser.js"></script>';
				echo '<script type="text/javascript">';
				echo '	var '.$prefix.'_browser = new ThesaurusBrowser("'.$prefix.'_BrowserContainer", "'.$prefix.'_BrowserFrame", "'.$datapath.'/", "'.$datapath.'/images/", "'.$prefix.'_browser", "'.$prefix.'_Helper", "'.$communicator_url.'", "'.$prefix.'_value", "'.$prefix.'");';
				echo '</script>';

			} else {
				echo $value;
			}
			echo '<span style="color: #FF0000">'.array_get_index($asset->_tmp, 'error', '').'</span>';
		}
		$o->closeField();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Field	&$asset			the asset whose interface we are painting
	* @param string					&$new_value_str		a string by reference that we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_value_str)
	{
		$prefix = $asset->getPrefix();
		if ($asset->attr('editable')) {
			if (!$asset->attr('force_thesaurus')) {
				$value = trim(array_get_index($_POST, $prefix.'_value', ''));
			} else {
				// thesaurus has to be active for successful operation
				if (!$asset->isThesaurusActive()) {
					return false;
				}

				if (isset($_POST[$prefix.'_value'])) {
					$value = trim(array_get_index($_POST, $prefix.'_value', ''));
					$th = $asset->getThesaurusRef();
					$term = $th->getEntityByName($value);
					if (empty($term) && !empty($value)) {
						$asset->_tmp['error'] = translate('lexicon_term_not_found');
						return false;
					}
					$value = $term['name'];
				}
			}
		}

		$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());

		return true;

	}//end processInlineValueInterface()


}//end class

?>
