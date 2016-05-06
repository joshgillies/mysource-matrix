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
*
*/
require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Content_Container_Template_Edit_Fns
*
* Purpose
*
*
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Container_Template_Edit_Fns extends Asset_Edit_Fns
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
		$root_links = $asset->getRootNodeLinks();
		$root_asset_ids = Array();
		foreach ($root_links as $root_link) {
			$root_asset_ids[$root_link['minorid']] = $root_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_root_nodes', array_keys($root_asset_ids));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($root_asset_ids));
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $assetid => $info) {
				echo '<li>'.get_asset_tag_line($assetid).'</li>';
			}
			?></ul><?php
		}
		return TRUE;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_root_nodes']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return FALSE;
		}
		$new_root_nodes = $_POST[$prefix.'_root_nodes'];
		foreach ($new_root_nodes as $new_root_node) {
			$new_root_node_ids[] = $new_root_node['assetid'];
		}

		// get existing root links
		$root_links = $asset->getRootNodeLinks();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = FALSE;
				}
			}
		}

		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'container_template_root')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $new_root->name, $new_root->id), E_USER_WARNING);
					break;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processRootNode()



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
	function paintMetadataSchema(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $asset->getMetadataSchemaLinks();
		$root_asset_ids = Array();
		foreach ($root_links as $root_link) {
			$root_asset_ids[$root_link['minorid']] = $root_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_metadata', array_keys($root_asset_ids), Array('metadata_schema' => 'D'));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($root_asset_ids));
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $assetid => $info) {
				echo '<li>'.get_asset_tag_line($assetid).'</li>';
			}
			?></ul><?php
		}
		$template_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', TRUE, 'major', 'container_template');
		$container_number = count($template_links);
		if($container_number > 1) {
		?>
			<div id="warning_message_for_content_template_metadata_schema" style="display: none;" class="sq-warning-message top-margin"><?php echo sprintf(translate('<strong>Warning:</strong> There are currently %s content containers that are using this template. Changing the Metadata Schema can cause unexpected loss of data if the Metadata is regenerated.'), $container_number);  ?></div>
			<script src="<?php echo sq_web_path('lib'); ?>/web/dfx/dfx.js"></script>
			<script type="text/javascript">
				(function() {
					dfxLoadedCallback = function() {
						// if asset finder assetid fields changed, show warning
						$assetidFields = dfxjQuery('[id ^=sq_asset_finder_<?php echo $prefix?>_metadata][id $=assetid]');
						$assetidFields.change(function() {
							dfxjQuery('#warning_message_for_content_template_metadata_schema').show();						
						});
						// if asset finder buttons clicked, show warning
						$buttonFields = dfxjQuery('input[id ^=sq_asset_finder_<?php echo $prefix?>_metadata][type =button]');
						$buttonFields.click(function() {
							dfxjQuery('#warning_message_for_content_template_metadata_schema').show();						
						});				
					}
				})();
			</script>
		<?php
		}

		return TRUE;

	}//end paintMetadataSchema()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processMetadataSchema(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_metadata']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return FALSE;
		}
		$new_root_nodes = $_POST[$prefix.'_metadata'];
		foreach ($new_root_nodes as $new_root_node) {
			$new_root_node_ids[] = $new_root_node['assetid'];
		}

		// get existing root links
		$root_links = $asset->getMetadataSchemaLinks();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;
		$deleted_schema = Array();
		$new_schema = Array();

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = FALSE;
				}
				else {
					$deleted_schema[] = $root_link['minorid'];
				}
			}
		}

		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				$changed = TRUE;
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'container_template_metadata')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $new_root->name, $new_root->id), E_USER_WARNING);
					break;
				}
				else {
					$new_schema[] = $new_root->id;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// now apply metadata schema to those containers
		if(!empty($new_schema) || !empty($deleted_schema)) {
			$schema_vars = Array();

			// get containers that have this templated applied
			$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'container_template');
			foreach ($containers as $container_link) {
				$container_id = $container_link['minorid'];
				foreach ($new_schema as $schemaid) {
					$schema_vars[] = Array(
									'assetids'			=> Array($container_id),
									'granted'			=> 1,
									'cascades'			=> TRUE,
									'schemaid'			=> $schemaid,
									'previous_access'	=> NULL,
								 );
				}
				foreach ($deleted_schema as $schemaid) {
					$schema_vars[] = Array(
									'assetids'			=> Array($container_id),
									'granted'			=> -1,
									'cascades'			=> TRUE,
									'schemaid'			=> $schemaid,
									'previous_access'	=> NULL,
									'delete_values'		=> FALSE,
								 );
				}
			}
			if(!empty($schema_vars)) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array('schema_changes' => $schema_vars);
				$hh->queueHipo('hipo_job_edit_metadata_schemas', $vars);
			}
		}

		return $ok;

	}//end processMetadataSchema()



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
	function paintPaintLayout(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $asset->getPaintLayoutLinks();
		$root_link = array_pop($root_links);
		$root_asset_id = isset($root_link['minorid']) ? $root_link['minorid'] : '';

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_container_template_paint_layout', $root_asset_id, Array('paint_layout_page' => 'D'));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($root_asset_id);
			foreach ($infos as $assetid => $info) {
				echo get_asset_tag_line($assetid);
				break;
			}
		}
		return TRUE;

	}//end paintPaintLayout()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processPaintLayout(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_container_template_paint_layout']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return FALSE;
		}
		$new_root_node = $_POST[$prefix.'_container_template_paint_layout'];
		$new_root_node_ids = Array();
		$new_root_nodes = Array();
		if(isset($new_root_node['assetid'])) {
			$new_root_node_ids[] = $new_root_node['assetid'];
			$new_root_nodes[] = $new_root_node;
		}

		// get existing root links
		$root_links = $asset->getPaintLayoutLinks();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = FALSE;
				}
			}
		}

		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'container_template_paint_layout')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $new_root->name, $new_root->id), E_USER_WARNING);
					break;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processPaintLayout()



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
	function paintSimpleEditLayout(&$asset, &$o, $prefix)
	{
		// get root links
		$root_links = $asset->getSimpleEditLayoutLinks();
		$root_link = array_pop($root_links);
		$root_asset_id = isset($root_link['minorid']) ? $root_link['minorid'] : '';

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_container_template_edit_layout', $root_asset_id, Array('layout' => 'D'));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($root_asset_id);
			foreach ($infos as $assetid => $info) {
				echo get_asset_tag_line($assetid);
				break;
			}
		}
		return TRUE;

	}//end paintSimpleEditLayout()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processSimpleEditLayout(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_container_template_edit_layout']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return FALSE;
		}
		$new_root_node = $_POST[$prefix.'_container_template_edit_layout'];
		$new_root_node_ids = Array();
		$new_root_nodes = Array();
		if(isset($new_root_node['assetid'])) {
			$new_root_node_ids[] = $new_root_node['assetid'];
			$new_root_nodes[] = $new_root_node;
		}

		// get existing root links
		$root_links = $asset->getSimpleEditLayoutLinks();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;

		// lets delete the root nodes that have been removed
		foreach ($root_links as $root_link) {
			if (!in_array($root_link['minorid'], $new_root_node_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
					$ok = FALSE;
				}
			}
		}

		// now, create the new node links
		foreach ($new_root_nodes as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($root_links as $root_link) {
				if ($root_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_root = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_root, SQ_LINK_NOTICE, 'container_template_edit_layout')) {
					$ok = FALSE;
					trigger_localised_error('CMS0016', sprintf(translate('Failed adding new root node "%1$s" [#%2$s]'), $new_root->name, $new_root->id), E_USER_WARNING);
					break;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processSimpleEditLayout()




	/**
	* Paint the usage of this template
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintUsage(Content_Container_Template $asset, Backend_Outputter $o, $prefix)
	{
		$page_size = 40;

		$template_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', TRUE, 'major', 'container_template');

		$o->openRaw();



		if (count($template_links) > 0) {

			$num_pages = ceil(count($template_links) / $page_size);
			$page_num = array_get_index($_POST, $prefix.'_usage_page', 1);
			$page_num = max($page_num, 1);
			$page_num = min($page_num, $num_pages);

			if ($num_pages == 1) {
				$page_tag = sprintf(translate('Page %1$s of %2$s'), $page_num, $num_pages);

			} else {
				$page_tag = sprintf(translate('Page <a style="color: #fff" href="#" onclick="return sq_pager_jump(\'%3$s\', sq_pager_prompt(1, %2$s));">%1$s</a> of %2$s'), $page_num, $num_pages, $prefix.'_usage_page');

			}
			$asset_count_tag = sprintf(translate('Displaying <strong>%1$s</strong> - <strong>%2$s</strong> of <strong>%3$s</strong> total assets'), ($page_num - 1) * $page_size + 1, min(count($template_links), $page_num * $page_size), count($template_links));



			$template_links = array_slice($template_links, ($page_num - 1) * $page_size, $page_size);

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
				<th class="sq-backend-table-header-header sq-pagination-links sq-no-side-border" style="text-align: left;"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></th>
				<th class="sq-backend-table-header-header sq-no-side-border" style="text-align: right;"><?php echo $asset_count_tag; ?></span></th>
			</tr>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-cell" colspan="2"><?php echo translate('Asset'); ?></th>

			</tr>
			</thead>
			<tbody>
			<?php

			foreach ($template_links as &$info) {
				?><tr class="sq-backend-table-row">
					<td class="sq-backend-table-cell" colspan="2">
					<?php
					$tag_line = get_asset_tag_line($info['minorid'], 'details');
					$parent_page = array_get_index($GLOBALS['SQ_SYSTEM']->am->getDependantParents($info['minorid'], '', TRUE, FALSE), 0);
					echo $tag_line;
					if($parent_page) {
						echo ' - ';
						echo get_asset_tag_line($parent_page, 'details');
					}
					?>
				</tr>
				<?php
			}//end foreach top tree
			?>

			</tbody>
			</table>
			<?php

		} else {
			$o->note(translate('<p>This template is not applied to any content container.</p>'));

		}

		$o->closeRaw();

	}//end paintUsage()


}//end class

?>
