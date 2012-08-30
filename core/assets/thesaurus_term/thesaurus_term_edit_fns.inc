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
* $Id: thesaurus_term_edit_fns.inc,v 1.14 2012/08/30 01:09:16 ewang Exp $
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
* @version $Revision: 1.14 $
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
	* interface for editing name
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintName(&$asset, &$o, $prefix)
	{
		$write = $asset->writeAccess('attributes');

		// Paint the name
		if ($write) {
			check_box($prefix.'_rename', '1');
			label(translate('rename_thesaurus_term_sys_wide'), $prefix.'_rename');
			echo '<input type="hidden" name="'.$prefix.'_old_name" value="'.$asset->attr('name').'" />';
		}

		return TRUE;

	}//end paintName()


	/**
	* Process interface for editing name
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processName(&$asset, &$o, $prefix)
	{
		$old_name = array_get_index($_POST, $prefix.'_old_name', '');
		$name = $asset->attr('name');
		$rename = array_get_index($_POST, $prefix.'_rename', 0);

		// Save the name
		if (!empty($name)) {
			$asset->setAttrValue('name', $name);
			if (!$asset->saveAttributes()) {
				$asset->setAttrValue('name', $old_name);
				return FALSE;
			} else {
				// Attributes saved, if the user wants to rename metadata... do it!
				if (!empty($rename) && !empty($asset->_thesaurus->id)) {
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$vars = Array('thesaurusid' => $asset->_thesaurus->id, 'terms' => Array($old_name => $name));
					$hh->queueHipo('hipo_job_rename_thesaurus_term', $vars);
				}//end if
			}//end if
		}//end if

		return TRUE;

	}//end processName()


	/**
	* interface for editing relations
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
			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);
			if ($parent_asset->type() == 'thesaurus') {
				echo translate('thesaurus_creating_top_level_term');
			} else {
				$relations = (property_exists($parent_asset, '_thesaurus')) ? $parent_asset->_thesaurus->getActiveRelations() : NULL;
				?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header"><strong><?php echo translate('parent'); ?></strong></th>
						<th class="sq-backend-table-header"><strong><?php echo translate('relation'); ?></strong></th>
					</tr>
					<tr>
						<td><?php echo get_asset_tag_line($parent_assetid); ?></td>
						<td><?php
							if (empty($relations)) {
								echo 'Because the thesaurus has no relations, this term will not be linked to the parent';
							} else {
								combo_box($prefix.'_new_relation[existing]', $relations, FALSE, $parent_asset->_thesaurus->attr('default_rel'));
							}
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
			if (empty($relations)) {
				echo '<tr><td colspan="2">There are no relations defined in this thesaurus, you cannot set or view the relations to parent terms.</td></tr>';
			} else {

				foreach ($parents as $parent) {
					$parent_rel = array_get_index($parent, 'relid');

					if (is_null($parent['termid'])) continue;
					echo '<tr>';
						echo '<td>';
							echo get_asset_tag_line($thesaurus->id.':'.$parent['termid']);
						echo '</td>';
						echo '<td>';
						if ($writeable) {
							combo_box($prefix.'_relations['.$parent['linkid'].']', $relations, FALSE, $parent_rel);
						} else {
							echo array_get_index($relations, $parent_rel);
						}
						echo '</td>';
					echo '</tr>';
				}
			}

			echo '</table>';

		}//end else

		$o->closeRaw();
		return TRUE;

	}//end paintRelations()


	/**
	* Process interface for editing relations
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
			foreach ($relations as $link_id => $relid) {
				$thesaurus->updateLinkRelation($link_id, $relid);
			}
		}

		return TRUE;

	}//end processRelations()


	/**
	* interface for editing relations
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintTermInfo(&$asset, &$o, $prefix)
	{
		// cannot display anything while creating a new asset
		if ($asset->id == 0) {
			return;
		}

		$o->openRaw();

		echo '<table>';
		$notes = $asset->getTermNotes();
		foreach ($notes as $note_name => $note_array) {
			echo '<tr><td valign="top" align="right">'.$note_name.':</td><td valign="top">';
			foreach ($note_array as $note) {
				echo nl2br($note).'<br />';
			}
			echo '</td></tr>';
		}

		$bridge_id = $asset->_thesaurus->id;
		$child_rel = $asset->_thesaurus->getChildRelationsForTerm($asset->_termid);
		foreach ($child_rel as $relid => $relname) {
			echo '<tr><td valign="top" align="right">'.$relname.':</td><td valign="top">';
			$rel_terms = $asset->_thesaurus->getChildTerms($asset->_termid, $relid);
			foreach ($rel_terms as $term) {
				$term_assetid = $bridge_id.':'.$term['termid'];
				$href_array = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($term_assetid => 'details'));
				$term_backendhref = $href_array[$term_assetid];
				echo '<a href="'.$term_backendhref.'">'.$term['term'].'</a>';
				echo '<br />';
			}
			echo '</td></tr>';
		}

		echo '</table>';

		$o->closeRaw();
		return TRUE;

	}//end paintTermInfo()


}//end class
?>
