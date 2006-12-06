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
* $Id: thesaurus_term_edit_fns.inc,v 1.7 2006/12/05 23:38:38 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Thesaurus_Term_Edit_Fns
*
* Purpose
*	Container of a thesaurus term
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus_Term_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Thesaurus_Term_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		// disable all static screens
		$this->static_screens = Array();

		// enable just the details
		$this->static_screens['details']['name'] = translate('details');
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* TODO: function comment: paintRelations
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintRelations(&$asset, &$o, $prefix)
	{

		// Special case for when the asset is being created
		$o->openRaw();
		if ($asset->id == 0) {
			$parent_assetid = array_get_index($_REQUEST, 'parent_assetid');
			$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);
			if ($parent_asset->type() == 'thesaurus') {
				echo translate('thesaurus_creating_top_level_term');
			} else {
				$relations = $parent_asset->_thesaurus->getActiveRelations();
				?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header"><strong><?php echo translate('parent'); ?></strong></th>
						<th class="sq-backend-table-header"><strong><?php echo translate('relation'); ?></strong></th>
					</tr>
					<tr>
						<td><?php echo get_asset_tag_line($parent_assetid); ?></td>
						<td><?php
							combo_box($prefix.'_new_relation[existing]', $relations, FALSE, NULL);
							echo translate('thesaurus_or_create_new');
							echo text_box($prefix.'_new_relation[new]', NULL, $size='30', $max='255');
						?></td>
					</tr>
				</table>
				<?php
			}
		} else {
			$thesaurus = $asset->_thesaurus;
			$relations = $thesaurus->getActiveRelations();

			$parents = $asset->_thesaurus->getParentTerms($asset->_termid);

			$writeable = $asset->writeAccess('all');

			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><strong><?php echo translate('parent'); ?></strong></th>
					<th class="sq-backend-table-header"><strong><?php echo translate('relation'); ?></strong></th>
				</tr>
			<?php
			foreach ($parents as $parent) {
				if (empty($parent['termid'])) continue;
				echo '<tr>';
					echo '<td>';
						echo get_asset_tag_line($thesaurus->id.':'.$parent['termid']);
					echo '</td>';
					echo '<td>';
					if ($writeable) {
						combo_box($prefix.'_relations['.$parent['linkid'].'][existing]', $relations, FALSE, $parent['relation']);
						echo translate('thesaurus_or_create_new');
						text_box($prefix.'_relations['.$parent['linkid'].'][new]', NULL, $size='30', $max='255');
					} else {
						echo array_get_index($parent, 'relation');
					}
					echo '</td>';
				echo '</tr>';
			}
			echo '</table>';
		}//end else

		$o->closeRaw();
		return TRUE;

	}//end paintRelations()


	/**
	* TODO: function comment: processRelations
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processRelations(&$asset, &$o, $prefix)
	{
		if ($asset->id == 0) {
			$new_rel = array_get_index($_REQUEST, $prefix.'_new_relation');
			if (isset($new_rel['new'])) {
				$relation = trim($new_rel['new']);
			}

			if (empty($relation)) {
				$relation = array_get_index($new_rel, 'existing');
			}

			$asset->_tmp['relation'] = $relation;
		} else {
			$thesaurus =& $asset->_thesaurus;
			$relations = array_get_index($_REQUEST, $prefix.'_relations', Array());
			foreach ($relations as $rel_id => $rel_types) {

				if (isset($rel_types['new'])) {
					$relation = trim($rel_types['new']);
				}

				if (empty($relation)) {
					$relation = array_get_index($rel_types, 'existing');
				}

				$thesaurus->renameLinkRelation($rel_id, $relation);
			}
		}

		return TRUE;

	}//end processRelations()


}//end class
?>
