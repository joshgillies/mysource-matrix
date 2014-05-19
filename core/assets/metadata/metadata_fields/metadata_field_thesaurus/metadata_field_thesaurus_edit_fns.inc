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
* $Id: metadata_field_thesaurus_edit_fns.inc,v 1.24 2012/08/30 01:09:09 ewang Exp $
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
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Thesaurus_Edit_Fns extends Metadata_Field_Edit_Fns
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
	* Paints the root node selection box
	*
	* @param Metadata_Field_Thesaurus	$asset	asset being painted
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(Metadata_Field_Thesaurus $asset, Backend_Outputter $o, $prefix)
	{
		// get root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		$root_asset_ids = Array();
		foreach ($root_links as $root_link) {
			$root_asset_ids[$root_link['minorid']] = $root_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_root_nodes', array_keys($root_asset_ids));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($root_asset_ids));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('id') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('name') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('type') ?></td>
				</tr>
			<?php
			foreach ($infos as $assetid => $info) {
			?>

			<tr>
				<td class="sq-backend-table-cell"><?php echo $assetid ?></td>
				<td class="sq-backend-table-cell"><?php echo $info['name'] ?></td>
				<td class="sq-backend-table-cell"><?php echo ucwords(str_replace('_',' ', $info['type_code'])) ?></td>
			</tr>
			<?php
			}
			?>
			</table>
			<?php
		}
		return TRUE;

	}//end paintRootNode()


	/**
	* Paint the interface for filling in metadata values
	*
	* Returns true if a submitable interface has been printed
	*
	* @param Metadata_Field_Thesaurus	$asset			asset being painted
	* @param Backend_Outputter			$o				the outputter class
	* @param string						$value_str		the current value for this field in the owning asset (NOT $asset)
	*													NULL means that there is no current value set
	* @param boolean					$write_access	whether to allow editing of this field
	*													(typically depends on write access to owner and editability of field)
	* @param boolean					$enable_default	Whether to show the 'use default' checkbox
	*
	* @return boolean
	* @access public
	*/
	function paintValueInterface(Metadata_Field_Thesaurus $asset, Backend_Outputter $o, $value_str, $write_access, $enable_default=TRUE, $print_cascade_values_option = TRUE, $type_code=NULL, $suffix_to_prefix = '')
	{
		$thesaurus = $asset->getThesaurusRef();

		if ($thesaurus == FALSE) {
			echo translate('source_thesaurus_not_set');
			return;
		}//end if

		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$is_default = FALSE;
		$default = $asset->attr('default');
		if (is_null($value_str)) {
			$is_default = TRUE;
			$value = $default;
			$value_components = $asset->attr('value_components');
		} else {
			$value='';
			$value_components = $asset->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}


		if (empty($value)) {
			$values = Array();
		} else if (!is_array($value)) {
			$values = explode(',', $value);
		}

		if (!is_array($values)) $values = Array($values);

		$term_assetids = Array();

		foreach ($values as $term) {
			$term = str_replace('&#44;', ',', $term);
			$termid = $thesaurus->getTermIdByName($term);
			// Bug #3123
			// If the thesaurus field has changed, this produces errors on the backend
			// Only deal with valid terms
			if (!empty($termid)) {
				$term_assetids[$termid] = $thesaurus->id.':'.$termid;
			}
			$term_info[$termid] = $term;
		}

		if (isset($asset->_tmp['error'])) {
			echo '<ul>';
			foreach ($asset->_tmp['error'] as $message) {
				echo '<li class="sq-backend-warning">'.$message.'</li>';
			}
			echo '</ul>';
		}

		if ($write_access) {

			if ($enable_default) {

				?>
				<script type="text/javascript">
					//<![CDATA[
					function toggleManualInterface(div_1, div_2)
					{
						if (document.getElementById(div_1).style.display == 'block') {
							document.getElementById(div_1).style.display = 'none';
							document.getElementById(div_2).style.display = 'block';
						} else {
							document.getElementById(div_2).style.display = 'none';
							document.getElementById(div_1).style.display = 'block';
						}
					}
					//]]>
				</script>
				<?php

				// use-default-or-not chooser
				check_box($prefix.'_default', '1', $is_default, 'toggleManualInterface(\''.$prefix.'_interface\',\''.$prefix.'_defaults\')');
				label(translate('thesaurus_use_default_value'), $prefix.'_default');
				if ($is_default) {
					echo ' '.translate('thesaurus_default_values_note');
				}
				echo '<br />';

				// non-default-value chooser
				echo '<div id="'.$prefix.'_interface" style="padding: 5px 0px; display:'.($is_default ? 'none' : 'block').'">';
				multiple_asset_finder($prefix.'_thes_term', $term_assetids, Array('thesaurus_term'=>'D'));
				hidden_field($prefix.'_terms_submitted', 1);
				echo '<div style="margin-top: 3px">';
				echo translate('thesaurus_meta_thesaurus_note', get_asset_tag_line($thesaurus->id, 'details'));
				echo '</div>';
				echo '</div>';

				// default value display
				echo '<div id="'.$prefix.'_defaults" style="display:'.($is_default ? 'block' : 'none').'">';
				echo $default;
				echo '</div>';

			} else {
				multiple_asset_finder($prefix.'_thes_term', $term_assetids, Array('thesaurus_term'=>'D'));
				hidden_field($prefix.'_terms_submitted', 1);
			}

			if ($print_cascade_values_option) {
				check_box($prefix.'_cascade_value', '1', FALSE);
				label(translate('cascade_value'), $prefix.'_cascade_value');
			}

		} else {
			if (empty($values)) {
				echo translate('thesaurus_no_values');
			} else {
				echo translate($is_default ? 'thesaurus_using_default_values' : 'thesaurus_using_non_default_values');
				?>
				<table class="sq-backend-table" border="1" style="margin-left: 0px">
					<tr>
						<th style="width: 30%; white-space: none;"><?php echo translate('thesaurus_term'); ?></th>
						<th><?php echo translate('thesaurus_lineages'); ?></th>
					</tr>
				<?php
				foreach ($term_info as $term_id => $term_name) {
					// Bug #3123
					// If the thesaurus field has changed, this produces errors on the backend
					// Only deal with valid terms
					if (!empty($term_id)) {
					?>
					<tr>
						<td><?php echo get_asset_tag_line($term_assetids[$term_id], 'details') ?></td>
						<td>
							<?php
							$lineages = $thesaurus->getLineagesForTerm($term_id);
							if (!$lineages) continue;
							foreach ($lineages as $lineage) {
								foreach ($lineage as $ancestor) {
									echo ' / ';
									if (isset($ancestor['relation'])) {
										echo '('.$ancestor['relation'].') ';
									}
									echo $ancestor['term'];
								}
								echo '<br />';
							}
							?>
						</td>
					</tr>
					<?php
					}//end if
				}//end foreach
				?>
				</table>
				<?php
			}//end else
		}//end else
		return $write_access;

	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param Metadata_Field_Thesaurus	$asset			asset being painted
	* @param string						&$new_value_str	a string by reference that we can store the new values in
	* @param bool     &$field_cascade_value Whether value updates to this field should cascade
	*                                       to children
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(Metadata_Field_Thesaurus $asset, &$new_value_str, &$field_cascade_value, $type_code=NULL, $suffix_to_prefix='')
	{
		if (!$asset->attr('editable')) return FALSE; // nothing for us to do

		$processed = FALSE;

		$prefix = $asset->getPrefix().($type_code ? '_'.$type_code : '');
		if (!empty($suffix_to_prefix)) $prefix .= $suffix_to_prefix;

		$thesaurus = $asset->getThesaurusRef();
		$value = Array();

		if (!empty($_POST[$prefix.'_default'])) {
			// use default
			$new_value_str = NULL;
			$processed = TRUE;
		} else {
			if (!empty($_REQUEST[$prefix.'_terms_submitted'])) {
				$new_terms = array_get_index($_REQUEST, $prefix.'_thes_term', Array());
				foreach ($new_terms as $one_term) {
					if (empty($one_term['assetid'])) continue;
					$term_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($one_term['assetid']);
					$term_name = $term_asset->attr('name');
					if ($term_asset->_thesaurus->id != $thesaurus->id) {
						$asset->_tmp['error'][] = translate('thesaurus_meta_wrong_thesaurus', $term_name, $term_asset->_thesaurus->name, $term_asset->_thesaurus->id);
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($term_asset);
						continue;
					}
					$term_name = str_replace(',', '&#44;', $term_name);
					$value[] = $term_name;
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($term_asset);
				}

				$old_count = count($value);
				$value = array_unique($value);
				if ($old_count != count($value)) {
					$asset->_tmp['error'][] = translate('thesaurus_term_duplicate_removed');
				}

				$value = implode(',', $value);
				$new_value_str = Metadata_Field::encodeValueString($value, Array());

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
	* Paints the default value selection box
	*
	* @param Metadata_Field_Thesaurus	$asset	asset being painted
	* @param Backend_Outputter			$o		backend outputter
	* @param string						$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintDefault(Metadata_Field_Thesaurus $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ((boolean)$asset->attr('is_contextable') === TRUE) {
			$default = $asset->attr('default');
		} else {
			$default = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('default', 'metadata_field', Array($asset->id), 0);
			$default = $default[$asset->id];
		}

		if (!$write_access) {
			echo $default;
			return TRUE;
		}

		$thesaurus = $asset->getThesaurusRef();

		if (empty($default)) {
			$values = Array();
		} else if (!is_array($default)) {
			$values = explode(',', $default);
		}

		if (!is_array($values)) $values = Array($values);

		$term_assetids = Array();

		foreach ($values as $term) {
			$termid = $thesaurus->getTermIdByName($term);
			$term_assetids[] = $thesaurus->id.':'.$termid;
			$term_info[$termid] = $term;
		}

		multiple_asset_finder($prefix.'_default_term', $term_assetids, Array('thesaurus_term'=>'D'));

		return TRUE;

	}//end paintDefault()


	/**
	* Process the default value selection box
	*
	* @param Metadata_Field_Thesaurus	$asset	asset being painted
	* @param Backend_Outputter			$o		backend outputter
	* @param string						$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processDefault(Metadata_Field_Thesaurus $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$prefix = $asset->getPrefix();
		$thesaurus = $asset->getThesaurusRef();
		$is_contextable = (boolean)$asset->attr('is_contextable');
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$assetid = $asset->id;
		$value = Array();

		$new_terms = array_get_index($_REQUEST, $prefix.'_default_term', Array());
		foreach ($new_terms as $one_term) {
			if (empty($one_term['assetid'])) continue;
			$term_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($one_term['assetid']);
			$term_name = $term_asset->attr('name');
			if ($term_asset->_thesaurus->id != $thesaurus->id) {
				$asset->_tmp['error'][] = translate('thesaurus_meta_wrong_thesaurus', $term_name, $term_asset->_thesaurus->name, $term_asset->_thesaurus->id);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($term_asset);
				continue;
			}
			$value[] = $term_name;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($term_asset);
		}

		$old_count = count($value);
		$value = array_unique($value);
		if ($old_count != count($value)) {
			$asset->_tmp['error'][] = translate('thesaurus_term_duplicate_removed');
		}
		$value = implode(',', $value);

		$default = Metadata_Field::encodeValueString($value, Array());

		if ($is_contextable === FALSE) {
			if ($contextid !== 0) {
				$GLOBALS['SQ_SYSTEM']->changeContext(0);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			}
		}

		$asset->setAttrValue('default', $default);
		$asset->saveAttributes();

		if ($is_contextable === FALSE) {
			if ($contextid !== 0) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			}
		}

		// No need to save attrs again
		return FALSE;

	}//end processDefault()


}//end class

?>
