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
* $Id: metadata_schema_edit_fns.inc,v 1.59 2012/10/03 07:53:26 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Metadata_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.59 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint the interface for creating a new metadata section
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintNewSection(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$o->openField(translate('Section Name'));

			text_box($prefix.'_new_section', '', 30);
		$o->closeField();
		return TRUE;

	}//end paintNewSection()


	/**
	* Processes the interface for creating a new metadata section
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processNewSection(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		if (isset($_POST[$prefix.'_new_section']) && trim($_POST[$prefix.'_new_section'])) {
			$new_section_name = trim($_POST[$prefix.'_new_section']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('metadata_section');
			$new_section = new Metadata_Section();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_section->setAttrValue('name', $new_section_name);
			if (!$new_section->create($schema_link)) return FALSE;
		}

		return TRUE;

	}//end processNewSection()


	/**
	* Paint the interface for regenerating metadata files
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintRegenerate(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {
			$o->openField(translate('Regenerate metadata'));

				check_box($prefix.'_regenerate');
				label(translate('Regenerate metadata files'), $prefix.'_regenerate');

				$o->note(translate('Check the box below to regenerate the metadata for all assets that have currently applied this schema.'));

			$o->closeField();
		} else {
			$o->note(translate('Permission Denied to regenerate the metadata'));

		}

		return TRUE;

	}//end paintRegenerate()


	/**
	* Processes the interface for regenerating metadata files
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processRegenerate(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('')) return FALSE;

		if (isset($_POST[$prefix.'_regenerate']) && $_POST[$prefix.'_regenerate']) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaids' => Array($asset->id,));
			$hh->queueHipo('hipo_job_regenerate_metadata', $vars);
		}

		return TRUE;

	}//end processRegenerate()


	/**
	* Paint the interface for editing metadata sections
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintSections(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('links');

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			$section_access = $section->writeAccess('links');
			if (is_null($section)) continue;

			$o->openSection($section->name);
				if ($write_access && $section_access) {
					$o->openField(translate('Delete?'));

						check_box($prefix.'_delete_sections['.$link_data['linkid'].']');
						label(translate('Delete this section and all its fields?'), $prefix.'_delete_sections['.$link_data['linkid'].']');

					$o->closeField();
				}
				$edit_fns = $section->getEditFns();
				if (!$edit_fns->paintInlineOptionInterface($section, $o, $write_access)) {
					return FALSE;
				}
			$o->closeSection();
		}

		return TRUE;

	}//end paintSections()


	/**
	* Processes the interface for editing metadata sections
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processSections(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;
		$updated = FALSE;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if ($edit_fns->processInlineOptionInterface($section, $o)) {
				$updated = TRUE;
			}
		}

		// delete any sections that need deleting
		if (isset($_POST[$prefix.'_delete_sections'])) {
			$delete_sections = $_POST[$prefix.'_delete_sections'];
			foreach ($delete_sections as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = TRUE;
			}
		}

		return $updated;

	}//end processSections()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param Metadata_Schema	$asset			the asset whose interface we are painting
	* @param Backend_Outputter	$o				the outputter class
	* @param string				$values			the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	* @param boolean			$write_access	does the current user have write access to the asset
	*											that we are setting metadata for (NOT THE SAME AS $asset)
	* @param boolean		$print_cascade_values_option	print the Metadata Field checkbox option to 'Cascade Values'
	* @param string				$type_code		The type_code of the asset that the metadata is being printed
	* @param boolean		$type_code_prefix	Whether to use $type_code as part of the metadata field id's prefix
	*
	* @return boolean
	* @access public
	*/
	public function paintInlineValueInterface(Metadata_Schema $asset, Backend_Outputter $o, $values, $write_access, $print_cascade_values_option = TRUE, $type_code=NULL, $type_code_prefix=FALSE)
	{
		$o->openSection($asset->name);
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		$res = FALSE;
		foreach ($current_links as $link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if ($edit_fns->paintInlineValueInterface($section, $o, $values, $write_access, $print_cascade_values_option, $type_code, $type_code_prefix)) {
				$res = TRUE;
			}
		}
		$o->closeSection();

		return $res;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* Returns true if at least one section processed something
	*
	* @param Metadata_Schema	$asset			the asset whose interface we are painting
	* @param array				&$new_values	an array we can store the new values in
	* @param string				$type_code		The type_code of the asset that the metadata is being printed
	* @param boolean			$type_code_prefix	Whether to use $type_code as part of the metadata field id's prefix
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Schema $asset, Array &$new_values, Array &$schema_cascade_values_asset_ids, $type_code=NULL, $type_code_prefix=FALSE)
	{
		$res = FALSE;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section_cascade_values_asset_ids = Array();
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if ($edit_fns->processInlineValueInterface($section, $new_values, $section_cascade_values_asset_ids, $type_code, $type_code_prefix)) {
				$res = TRUE;

				foreach($section_cascade_values_asset_ids as $asset_id => $val) {
					$schema_cascade_values_asset_ids[$asset_id] = 1;
				}
			}
		}

		return $res;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param Metadata_Schema	$asset		the asset whose metadata we are checking
	* @param array				$values		the current metadata values
	* @param string				$type_code	The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	public function requiredFieldsComplete(Metadata_Schema $asset, Array $values, $type_code=NULL)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($section, $values, $type_code)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this schema
	*
	* @param Metadata_Schema	$asset			the asset whose interface we are painting
	* @param array				&$values		the current metadata values
	* @param array				&$tag_values	the metadata tag values
	* @param array				&$keywords		an array we can put keywords in that need replacing
	* @param string				$type_code		The type_code of the asset that the metadata is being printed
	* @param boolean			$content_file   Whether the generated metadata will be included in a metadata content file
	* @param array              &$mfield_types  Type of the metadata fields
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Schema $asset, Array &$values, Array &$tag_values, Array &$keywords, $type_code=NULL, $content_file=TRUE, &$mfield_types=Array())
	{
		$frontend_option = $asset->attr('frontend');

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// Buffer the contents of any META tags
		ob_start();

		echo '<!-- '.$mm->escapeMetadata($asset->name).' //-->'."\n\n";

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', TRUE, 'major', NULL, TRUE);
		$valid_sections = Array();
		foreach ($current_links as $link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$valid_sections[] = $section->name;

			$edit_fns = $section->getEditFns();

			if (!$edit_fns->generateMetadata($section, $values, $tag_values, $keywords, $type_code, $content_file, $mfield_types)) {
				ob_end_clean();
				return FALSE;
			}

		}

		// If printing on front-end, print the META tags, otherwise discard them
		$contents = ob_get_clean();
		if ($frontend_option) echo $contents;

		return TRUE;

	}//end generateMetadata()


	//--        METADATA SCHEMA USAGE        --//


	public function paintUsage(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		$page_size = 10;

		$o->openRaw();

		$schema_treeids_sql = 'SELECT
								treeid, assetid, granted, cascades
								FROM (sq_ast_lnk_tree ta
									JOIN sq_ast_lnk la ON ta.linkid = la.linkid
										JOIN sq_ast_mdata ma ON la.minorid = ma.assetid)
								WHERE ma.schemaid = :schemaid
								AND NOT EXISTS (
									SELECT * FROM (sq_ast_lnk_tree tb
													JOIN sq_ast_lnk lb ON tb.linkid = lb.linkid
														JOIN sq_ast_mdata mb ON lb.minorid = mb.assetid)
									WHERE mb.schemaid = ma.schemaid
									AND length(ta.treeid) - LENGTH(tb.treeid) = :tree_size AND SUBSTR(ta.treeid, 1, LENGTH(ta.treeid) - :tree_size_1) = tb.treeid
									AND ma.granted = mb.granted
									AND ma.cascades = mb.cascades
								) ORDER BY length(treeid), treeid';

		$query = MatrixDAL::preparePdoQuery($schema_treeids_sql);
		MatrixDAL::bindValueToPdo($query, 'schemaid', $asset->id);
		MatrixDAL::bindValueToPdo($query, 'tree_size', SQ_CONF_ASSET_TREE_SIZE);
		MatrixDAL::bindValueToPdo($query, 'tree_size_1', SQ_CONF_ASSET_TREE_SIZE);
		$treeids = MatrixDAL::executePdoAssoc($query);

		if (count($treeids) > 0) {
			$tree_tops = $this->_getTreeTops($asset, $treeids);

			$num_pages = ceil(count($tree_tops) / $page_size);
			$page_num = array_get_index($_POST, $prefix.'_usage_page', 1);
			$page_num = max($page_num, 1);
			$page_num = min($page_num, $num_pages);

			if ($num_pages == 1) {
				$page_tag = sprintf(translate('Page %1$s of %2$s'), $page_num, $num_pages);

			} else {
				$page_tag = sprintf(translate('Page <a style="color: #fff" href="#" onclick="return sq_pager_jump(\'%3$s\', sq_pager_prompt(1, %2$s));">%1$s</a> of %2$s'), $page_num, $num_pages, $prefix.'_usage_page');

			}
			$asset_count_tag = sprintf(translate('Displaying <strong>%1$s</strong> - <strong>%2$s</strong> of <strong>%3$s</strong> total trees'), ($page_num - 1) * $page_size + 1, min(count($tree_tops), $page_num * $page_size), count($tree_tops));



			$tree_tops = array_slice($tree_tops, ($page_num - 1) * $page_size, $page_size);

			$links = Array(
						'first'		=> '&lt;&lt;',
						'previous'	=> '&lt;',
						'page'		=> $page_tag,
						'next'		=> '&gt;',
						'last'		=> '&gt;&gt;',
					);

			if ($page_num > 1) {
				$links['first'] = '<a title="'.translate('Go to the first page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', 1)">'.$links['first'].'</a>';

				$links['previous'] = '<a title="'.translate('Go to the previous page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';

			} else {
				$links['first'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['first'].'</span>';

				$links['previous'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['previous'].'</span>';

			}

			if ($page_num < $num_pages) {
				$links['last'] = '<a title="'.translate('Go to the last page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.$num_pages.')">'.$links['last'].'</a>';

				$links['next'] = '<a title="'.translate('Go to the next page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num + 1).')">'.$links['next'].'</a>';

			} else {
				$links['last'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['last'].'</span>';

				$links['next'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['next'].'</span>';

			}

			hidden_field($prefix.'_usage_page', $page_num); ?>
			<table class="sq-backend-table">
			<thead>
			<tr class="sq-backend-table-row">
				<td class="sq-backend-table-header-header"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></td>
				<td class="sq-backend-table-header-header" colspan="2" style="text-align: right; font-weight: normal"><?php echo $asset_count_tag; ?></span></td>
			</tr>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-cell"><?php echo translate('Asset'); ?></th>

				<th style="width: 15%" class="sq-backend-table-cell"><?php echo translate('Granted?') ?></th>

				<th style="width: 15ex" class="sq-backend-table-cell"><?php echo translate('Cascade?') ?></th>

			</tr>
			</thead>
			<tbody>
			<?php

			foreach ($tree_tops as $tt_assetid => &$tree_top_info) {
				$root_tree = $tree_top_info;
				unset($root_tree['sub_trees']);
				$root_trees = array_merge(Array($root_tree), $tree_top_info['sub_trees']);

				foreach ($root_trees as $root_assetid => &$tree_info) {

					?><tr class="sq-backend-table-row">
						<td class="sq-backend-table-cell">
						<?php

						$tag_line = get_asset_tag_line($tree_info['assetid'], 'details');

						echo ($tree_info['depth'] === 0 ? '' : '<span style="padding-left: '.($tree_info['depth']*5).'ex">');
						echo sprintf(translate('%1$s'), $tag_line);

						echo ($tree_info['depth'] === 0 ? '' : '</span>');

						$cascades_str = translate('No');
						if ($tree_info['cascades'] === '1') {
						    $cascades_str = translate('Yes');
						}

						?></td>
						<td class="sq-backend-table-cell">
							<strong><span style="color: <?php echo (($tree_info['granted'] == '0') ? 'red' : 'green'); ?>"><?php echo (($tree_info['granted'] == '0') ? translate('Denied') : translate('Applied')) ?></span></strong>


						</td>
						<td class="sq-backend-table-cell">
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($tree_info['cascades'] === '1' ? 'tick' : 'cross'); ?>.png" width="15" height="15" style="vertical-align: middle;" alt="<?php echo $cascades_str; ?>" />
						</td>
					</tr>
					<?php
				}//end foreach entry

				unset($tree_info);

			}//end foreach top tree
			?>

			</tbody>
			</table>
			<?php

			$o->note(translate('<p>This screen shows which assets have this schema either applied or denied.</p>

	<p>Where the applying or denying of the schema is cascaded unaffected through a tree of assets, only the top asset of that tree will be displayed. However, if any children underneath the asset do anything different to its parent - such as denying the schema where its parent had applied (or vice-versa), or it no longer cascades to new assets - a child entry in the table is created.</p>'));


		} else {
			$o->note(translate('<p>This schema is not applied to any assets.</p>'));

		}

		$o->closeRaw();

		return FALSE;

	}//end paintUsage()


	/**
	* Get the "tree tops" in the supplied list of assets
	*
	* An asset is a "tree top" in a set of assets if none of its ancestors
	* in the asset tree are in the set with it.  Conversely if an asset's parent
	* or grandparent etc is in the set, then that asset is not a tree top.
	*
	* Returns an array indexed by treeid, where each value is another array
	* containing assetid and number of children of that assetid found in
	* the set.
	*
	* @param array	$assetids	The assetids we are analysing
	*
	* @return array
	* @access public
	*/
	public function _getTreeTops(Metadata_Schema $asset, Array $treeids)
	{
		$used_assetids = Array();

		$all_treeids = Array();
		foreach ($treeids as $treeid) {
			$all_treeids[$treeid['treeid']] = $treeid;
		}
		unset($treeids);

		$res = Array();
		foreach ($all_treeids as $tree_info) {
			$treeid = $tree_info['treeid'];
			$treeid_copy = $treeid;
			$top_parent  = $treeid;
			$depth       = 0;
			$used_sub_tree = FALSE;

			while (empty($treeid_copy) === FALSE) {
				if (isset($res[$treeid_copy]) === TRUE) {
					if ($used_sub_tree === FALSE) {
						$used_sub_tree = TRUE;
					}
				}

				if (($used_sub_tree === TRUE) && (array_key_exists($treeid_copy, $all_treeids) === TRUE)) {
					$top_parent = $treeid_copy;
					$depth++;
				}

				$treeid_copy = substr($treeid_copy, 0, -SQ_CONF_ASSET_TREE_SIZE);
			}

			$res[$treeid] = $tree_info;
			$res[$treeid]['top_parent'] = $top_parent;
			$res[$treeid]['depth'] = $depth;
			$res[$treeid]['sub_trees'] = Array();

			// Have we seen this asset before? If so, let it go, but mark it
			// to be dumped afterwards so we only have one of each asset
			if (array_search($tree_info['assetid'], $used_assetids) === FALSE) {
				$res[$treeid]['dupe'] = FALSE;
				$used_assetids[] = $tree_info['assetid'];
			} else {
				$res[$treeid]['dupe'] = TRUE;
			}
		}

		foreach ($res as $key => &$res_item) {
			if ($res_item['dupe'] === TRUE) {
				unset($res[$key]);
			} else {
				if ($res_item['depth'] > 0) {
					$res[$res_item['top_parent']]['sub_trees'][$res_item['treeid']] =& $res_item;
					unset($res[$key]);
				}
			}
		}

		// Sort each sub-tree top-to-bottom, to ensure correct tree order
		foreach ($res as $key => &$res_item) {
			uksort($res_item['sub_trees'], create_function('$key1,$key2', 'return strcmp($key1, $key2);'));
		}

		unset($res_item);

		return $res;

	}//end _getTreeTops()


}//end class

?>
