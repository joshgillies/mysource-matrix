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
* $Id: metadata_field_thesaurus_edit_fns.inc,v 1.6.2.2 2006/02/21 03:16:57 sdanis Exp $
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
* @version $Revision: 1.6.2.2 $
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
	function paintValueInterface(&$asset, &$o, $value_str, $write_access)
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

		$communicator_url = $asset->getCommunicatorUrl();

		// thesaurus has to be active for successful operation
		if (!$asset->isThesaurusActive()) {
			echo '<span style="color: #FF0000">'.translate('lexicon_linked_thesaurus_inactive').'</span>';
		} else {
			$max_terms = $asset->attr('max_terms');
			if ($write_access && $max_terms > 0) {
				$th = $asset->getThesaurusRef();
				echo '<script type="text/javascript" src="'.$datapath.'/thesaurus_browser.js"></script>';
				$text_box_extra = ($asset->attr('allow_manual'))?'':'readonly="readonly"';
				if (!is_array($value)) $value = explode(',', $value);

				if (!is_array($value)) $value = Array($value);

				if (count($value) < $max_terms) {
					$value = array_pad($value, $max_terms, '');
				}
				echo '<table class="sq-backend-table">';
				echo '<tr><th>Value Input</th><th>Category Path</th></tr>';
				for ($i=0; $i < $max_terms; $i++) {

					$this_value = array_shift($value);
					$browser_prefix = $prefix.'_'.$i;

					echo '<tr '.(($i%2)?'class="alt"':'').'>';
					echo '<td>';
					text_box($browser_prefix.'_value', $this_value, 30, '', false, $text_box_extra);
					echo '
						<input type="button" value="Browse Thesaurus" onclick="'.$browser_prefix.'_browser.open(event)" />
						<input type="button" value="Clear" onclick="document.getElementById(\''.$browser_prefix.'_value\').value=\'\';" />
						<script type="text/javascript">
							var '.$browser_prefix.'_browser = new ThesaurusBrowser("'.$browser_prefix.'_BrowserContainer", "'.$browser_prefix.'_BrowserFrame", "'.$datapath.'/", "'.$datapath.'/images/", "'.$browser_prefix.'_browser", "'.$browser_prefix.'_Helper", "'.$communicator_url.'", "'.$browser_prefix.'_value", "'.$prefix.'");
						</script>
						</td>';
					echo '<td>';
					if (!empty($this_value)) {
						$entity = $th->getEntityByName($this_value);
						if (!empty($entity)) {

							$all_lineages = $th->getCategoryLineage($entity['id']);

							if (empty($all_lineages)) continue;

							foreach ($all_lineages as $lineage) {
								echo '<div>';
								foreach ($lineage as $element) {
									echo ' / ';
									echo $element['name'];
								}
								echo '</div>';
							}
						}
					}
					echo '</td>';
					echo '</tr>';
				}
				echo '</table>';
			} else {
				echo $value;
			}
			echo '<div style="color: #FF0000">'.array_get_index($asset->_tmp, 'error', '').'</div>';
		}

		return true;

	}//end paintValueInterface()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Schema		&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param array						$values			current metadata values set for this asset
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access) {
		$prefix = $asset->getPrefix();
		$direct = array_get_index($_REQUEST, $prefix.'_direct_connection', false);

		// if a direct connection is requested, pass the control
		// to the direct Communicator, otherwise take normal course of action
		if ($direct) {
			$asset->_directCommunicator();
			exit;	// need this since Matrix tries to print extras after this section is done
		}

		return parent::paintInlineValueInterface($asset, $o, $values, $write_access);

	}

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
		$max_terms = $asset->attr('max_terms');

		$value = Array();
		for ($i=0; $i < $max_terms; $i++) {
			$browser_prefix = $prefix.'_'.$i;
			$one_value = trim(array_get_index($_REQUEST, $browser_prefix.'_value', ''));
			if (empty($one_value)) continue;

			if ($asset->attr('editable')) {
				if ($asset->attr('force_thesaurus')) {
					// thesaurus has to be active for successful operation
					if (!$asset->isThesaurusActive()) return false;

					$th = $asset->getThesaurusRef();
					$term = $th->getEntityByName($one_value);
					if (empty($term)) {
						$asset->_tmp['error'] = translate('lexicon_term_not_found');
						return false;
					}
					$one_value = $term['name'];
				}
			}

			$value[] = $one_value;

		}

		$old_count = count($value);
		$value = array_unique($value);
		if ($old_count != count($value)) {
			$asset->_tmp['error'] = translate('lexicon_term_duplicate_removed');
		}
		$value = implode(',', $value);
		$new_value_str = Metadata_Field::encodeValueString($value, Array());

		return true;

	}//end processInlineValueInterface()


}//end class

?>
