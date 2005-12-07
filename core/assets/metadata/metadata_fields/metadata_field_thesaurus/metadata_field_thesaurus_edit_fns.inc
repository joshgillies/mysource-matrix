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
* $Id: metadata_field_thesaurus_edit_fns.inc,v 1.2 2005/12/07 00:51:18 arailean Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Thesaurus_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'root');
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
		return true;

	}//end paintRootNode()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$value_str		the current value for this field in the asset (NOT $asset) NULL means that there is no current value set
	* @param boolean	$write_access	does the current user have write access to the asset that we are setting metadata for (NOT THE SAME AS $asset)
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

		$thesaurus =& $asset->getThesaurusRef();

		if (empty($value)) {
			$values = Array();
		} else if (!is_array($value)) {
			$values = explode(',', $value);
		}

		if (!is_array($values)) $values = Array($values);

		$term_assetids = Array();

		foreach ($values as $term) {
			$termid = $thesaurus->getTermIdByName($term);
			$term_assetids[] = $thesaurus->id.':'.$termid;
			$term_info[$termid] = $term;
		}

		echo '<p>'.translate('thesaurus_meta_thesaurus_note').get_asset_tag_line($thesaurus->id, 'details').'</p>';

		if (isset($asset->_tmp['error'])) {
			echo '<ul>';
			foreach ($asset->_tmp['error'] as $message) {
				echo '<li class="sq-backend-warning">'.$message.'</li>';
			}
			echo '</ul>';
		}

		if ($write_access) {
			multiple_asset_finder($prefix.'_thes_term', $term_assetids, Array('thesaurus_term'=>'D'));
		}

		if (!empty($values)) {
			$o->openSection(translate('thesaurus_term_lineages'));
			foreach ($term_info as $term_id => $term_name) {
				$lineages = $thesaurus->getLineagesForTerm($term_id);
				if (!$lineages) continue;
				$o->openField($term_name);
				foreach ($lineages as $lineage) {
					foreach ($lineage as $ancestor) {
						echo ' / ';
						if (isset($ancestor['relation'])) {
							echo '('.$ancestor['relation'].') ';
						}
						echo $ancestor['term'];
					}
					echo '<br>';
				}
			}
		}
		return true;
	}//end paintValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object	&$asset			the asset whose interface we are painting
	* @param string	&$new_value_str	a string by reference that we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_value_str)
	{
		$prefix = $asset->getPrefix();
		$thesaurus =& $asset->getThesaurusRef();
		$value = Array();

		$new_terms = array_get_index($_REQUEST, $prefix.'_thes_term', Array());
		foreach ($new_terms as $one_term) {
			if (empty($one_term['assetid'])) continue;
			$term_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($one_term['assetid']);
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
		$new_value_str = Metadata_Field::encodeValueString($value, Array());

		return true;

	}//end processInlineValueInterface()


}//end class

?>
