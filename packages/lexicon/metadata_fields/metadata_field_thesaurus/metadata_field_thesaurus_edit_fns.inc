<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_thesaurus_edit_fns.inc,v 1.1.2.1 2005/05/02 00:35:50 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.1.2.1 $
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

		$datapath = sq_web_path('data').'/asset_types/metadata_field_thesaurus/files';

		$field_name = $asset->name;
		if ($asset->attr('required')) $field_name .= ' <span style="color: #FF0000">*</span>';

		$value = array_get_index($values, 'value');

		$communicator_url = $asset->getCommunicatorUrl();
		$o->openField($field_name);

		// thesaurus has to be active for successful operation
		if (!$asset->isThesaurusActive()) {
			echo '<span style="color: #FF0000">Linked Thesaurus is <strong>INACTIVE</strong>. This value cannot be set!</span>';
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
	* @param array					&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$prefix = $asset->getPrefix();
		if ($asset->attr('editable')) {
			if (!$asset->attr('force_thesaurus')) {
				$new_values[$asset->name]['value'] = trim($_POST[$prefix.'_value']);
			} else {
				// thesaurus has to be active for successful operation
				if (!$asset->isThesaurusActive()) {
					return false;
				}

				if (isset($_POST[$prefix.'_value'])) {
					$value = trim($_POST[$prefix.'_value']);
					$th = $asset->getThesaurusRef();
					$term = $th->getEntityByName($value);
					if (empty($term) && !empty($value)) {
						$asset->_tmp['error'] = 'Term not found in thesaurus';
						return false;
					}
					$new_values[$asset->name]['value'] = $term['name'];
				}
			}
		}

		return true;

	}//end processInlineValueInterface()


}//end class

?>
