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
* $Id: asset_edit_fns.inc,v 1.91 2013/07/25 23:23:50 lwright Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/conditions_list.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.91 $
* @package MySource_Matrix
*/
class Asset_Edit_Fns extends MySource_Object
{

	/**
	* An array of tab code names and their display names
	* @var array
	*/
	var $static_screens = Array(
							'details'			=> Array(
													'name'			=> 'Details',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'menu',
												   ),
							'permissions'		=> Array(
													'name'			=> 'Permissions',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'permissions',
												   ),
							'workflow'			=> Array(
													'name'			=> 'Workflow',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'workflow',
												   ),
							'roles'				=> Array(
													'name'			=> 'Roles',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'roles',
												   ),
							'metadataSchemas'	=> Array(
													'name'			=> 'Metadata Schemas',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'metadata',
												   ),
							'metadata'			=> Array(
													'name'			=> 'Metadata',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'metadata',
												   ),
							'dependants'		=> Array(
													'name'			=> 'Dependants',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'linking'			=> Array(
													'name'			=> 'Linking',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'links',
												   ),
							'settings'			=> Array(
													'name'			=> 'Settings',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'all',
												   ),
							'history'			=> Array(
													'name'			=> 'History',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'logs'				=> Array(
													'name'			=> 'Logs',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'preview'			=> Array(
													'name'			=> 'Preview',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'none',
												   ),
							'lookupValues'		=> Array(
													'name'			=> 'Lookup Settings',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'lookupValues',
												   ),
							'tagging'			=> Array(
													'name'			=> 'Tagging',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'menu',
												   ),
							'layouts'			=> Array(
													'name'			=> 'Layouts',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'all',
												   ),
						  );


	/**
	* Constructor
	*
	*/
	function Asset_Edit_Fns()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Paint the interface for the asset paths
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintPaths(&$asset, &$o, $prefix)
	{

		$wa = $asset->writeAccess('lookups');

		$paths = $asset->getWebPaths();

		foreach ($paths as $path) {
			if ($wa) {
				text_box($prefix.'_paths[]', $path, 50, '', FALSE, 'style="margin-bottom:10px;"');
			} else {
				echo $path;
			}
			echo '<br>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 50);

		return $wa;

	}//end paintPaths()


	/**
	* Paint the interface for the asset remaps
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintEditRemapURLs(&$asset, &$o, $prefix)
	{
		$admin_access = $asset->adminAccess('lookups');

		$remap_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
		$remap_urls = $remap_manager->getRemapURLs($asset);

		if (empty($remap_urls)) {
			echo translate('no_associated_remaps');
			return FALSE;
		}

		$num_per_page = 100;
		$total_assets = count($remap_urls);
		$num_pages = ceil($total_assets / $num_per_page);

		$page_num = (int) array_get_index($_POST, $prefix.'_remap_urls_page', 1);
		$page_num = max($page_num, 1);
		$page_num = min($page_num, $num_pages);

		if ($num_pages == 1) {
			$page_tag = translate('page_number', $page_num, $num_pages);
		} else {
			$page_tag = translate('page_number_with_pager', $page_num, $num_pages, $prefix.'_remap_urls_page');
		}
		$asset_count_tag = translate('item_range', ($page_num - 1) * $num_per_page + 1, min($total_assets, $page_num * $num_per_page), $total_assets, strtolower(translate('remap_entries')));

		$remap_urls = array_slice($remap_urls, ($page_num - 1) * $num_per_page, $num_per_page);

		$links = Array(
					'first'		=> '&lt;&lt;',
					'previous'	=> '&lt;',
					'page'		=> $page_tag,
					'next'		=> '&gt;',
					'last'		=> '&gt;&gt;',
				);

		if ($page_num > 1) {
			$links['first'] = '<a title="'.translate('pagination_go_to_first').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_remap_urls_page\', 1)">'.$links['first'].'</a>';
			$links['previous'] = '<a title="'.translate('pagination_go_to_previous').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_remap_urls_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';
		} else {
			$links['first'] = '<span title="'.translate('pagination_cannot_go_further_back').'." style="color: #333">'.$links['first'].'</span>';
			$links['previous'] = '<span title="'.translate('pagination_cannot_go_further_back').'." style="color: #333">'.$links['previous'].'</span>';
		}

		if ($page_num < $num_pages) {
			$links['last'] = '<a title="'.translate('pagination_go_to_last').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_remap_urls_page\', '.$num_pages.')">'.$links['last'].'</a>';
			$links['next'] = '<a title="'.translate('pagination_go_to_next').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_remap_urls_page\', '.($page_num + 1).')">'.$links['next'].'</a>';
		} else {
			$links['last'] = '<span title="'.translate('pagination_cannot_go_further_forward').'." style="color: #333">'.$links['last'].'</span>';
			$links['next'] = '<span title="'.translate('pagination_cannot_go_further_forward').'." style="color: #333">'.$links['next'].'</span>';
		}

		hidden_field($prefix.'_remap_urls_page', $page_num); ?>

		<table class="sq-backend-table">
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-header-header"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></th>
				<th class="sq-backend-table-header-header" colspan="<?php echo ($admin_access === TRUE) ? 5 : 4 ?>" style="text-align: right; font-weight: normal"><?php echo $asset_count_tag; ?></span></th>
			</tr>
			<tr>
				<th class="sq-backend-table-header"><?php echo translate('url'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('remap_url'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('expires'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('type'); ?></th>
				<?php
					if ($admin_access) {
						?>
						<th class="sq-backend-table-header">
							<?php
							check_box($prefix.'_select_all', '1', FALSE, "check_all(document.main_form, '{$prefix}_delete_remap', this.checked);");
							label(translate('delete_question'), $prefix.'_select_all');
							?>
						</th>
						<?php
					}
				?>
				<th class="sq-backend-table-header"><?php echo translate('never_delete'); ?></th>
			</tr>
			<?php
				foreach ($remap_urls as $url_info) {
					if (empty($url_info['expires'])) {
						$expires = translate('never');
					} else {
						$expires = $GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($url_info['expires']));
					}
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $url_info['url']; ?></td>
					<td class="sq-backend-table-cell"><?php echo $url_info['remap_url']; ?></td>
					<td class="sq-backend-table-cell"><?php echo $expires; ?></td>
					<td class="sq-backend-table-cell"><?php echo ((int)$url_info['auto_remap'] === 1) ? translate('automatic') : (($url_info['auto_remap'] === NULL) ? translate('unknown') : translate('manual')); ?></td>
					<?php
						if ($admin_access) {
							?>
							<td class="sq-backend-table-cell" align="center">
								<?php
								check_box($prefix.'_delete_remap['.$url_info['url'].']', 1, FALSE, '', $url_info['never_delete'] ? 'disabled' : '');
								?>
							</td>
							<?php
						}
					?>
					
					<td class="sq-backend-table-cell" align="center"><img src="<?php echo sq_web_path('lib')."/web/images/".($url_info['never_delete'] ? "tick.gif" : "cross.gif"); ?>" width="15" height="15" /></td>
				</tr>
				<?php
				}//end foreach
			?>
		</table>

		<?php

		return $admin_access;

	}//end paintEditRemapURLs()


	/**
	* Processes the Paths posted from the paintPaths fn above
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPaths(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_paths'])) return FALSE;

		// Add remaps only if they are to be automatically added when the web path is changed. The default behaviour
		// is to add remaps. This functionality is controlled by a global switch in the Remap Manager and can be overridden
		// on the Web Paths screen (per asset) on a one-shot basis when changing web paths

		$auto_add_remaps = isset($_POST[$prefix.'_remap_upon_webpath_change']);

		if (!$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']), $auto_add_remaps)) {
			return FALSE;
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);

		return TRUE;

	}//end processPaths()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processEditRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('lookups')) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (isset($_POST[$prefix.'_delete_remap'])) {
			$urls = array_keys($_POST[$prefix.'_delete_remap']);

			$remap_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
			$remap_urls = $remap_manager->getRemapURLs($asset);

			foreach ($remap_urls as $url_info) {
				foreach ($urls as $index => $url_to_delete) {
					if ($url_info['url'] === $url_to_delete && $url_info['never_delete']) {
						unset($urls[$index]);
					}
				}
			}

			try {
				$bind_vars = Array('urls' => $urls);
				MatrixDAL::executeQuery('remap_manager', 'deleteRemapsByURL', $bind_vars);
			} catch (DALException $e) {
				throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processEditRemapURLs()


	/**
	* Paint the interface for the asset paths
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintURLs(&$asset, &$o, $prefix)
	{
		$urls = $asset->getLookups();
		if (empty($urls)) {
			echo translate('no_associated_urls');
		} else {
			usort($urls, create_function('$a, $b', 'return strcmp($a[\'url\'], $b[\'url\']);'));
			foreach ($urls as $data) {
				if ($data['http']) {
					echo $data['url'].'<br/>';
				}
				if ($data['https']) {
					echo $data['url'].'<br/>';
				}
			}
			/*echo '<ul class="sq-remove-brs" style="margin-top: 0;">';
			foreach ($urls as $data) {
				if ($data['http']) {
					echo '<li><a href="http://'.$data['url'], '" target="_blank">http://'.$data['url'], '</a></li><br/>';
				}
				if ($data['https']) {
					echo '<li><a href="https://'.$data['url'], '" target="_blank">https://'.$data['url'], '</a></li><br/>';
				}
			}
			echo '</ul>';*/
		}

		if ($asset->force_secure === '1') {
			$o->note(translate('force_secure_set_on_asset'));
		}

	}//end paintURLs()


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('lookups')) return FALSE;
		
		$urls = $asset->getURLs();
		if (empty($urls)) {
			$o->openField(translate('url_remappings'));
				label(translate('url_remap_no_associated_asset_urls'));
			$o->closeField();

			return FALSE;
		}

		foreach ($urls as $url) {
			if ($url['http'] == 1) {
				$contents['http://'.$url['url']] = 'http://'.$url['url'];
			}
			if ($url['https'] == 1) {
				$contents['https://'.$url['url']] = 'https://'.$url['url'];
			}
		}

		$remap_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
		$never_delete = ($remap_manager->attr('never_delete_remap_default') == FALSE) ? 0 : 1;

		$o->openField(translate('old_url'));
			text_box($prefix.'_add_remap_old_url', '', 50);
		$o->closeField();

		$o->openField(translate('new_url'));
			combo_box($prefix.'_add_remap_new_url', $contents);
			$o->note(translate('url_remap_explanation'));
		$o->closeField();

		$o->openField(translate('expires'));
			text_box($prefix.'_add_remap_expires_days', '', 5);
			label(translate('days'), $prefix.'_add_remap_expires_days');
		$o->closeField();

		$o->openField(translate('never_delete'));
			check_box($prefix.'_add_remap_never_delete', TRUE, $never_delete);
		$o->closeField();

	}//end paintAddRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param object	&$asset	the remap manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAddRemapURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('lookups')) return FALSE;

		// now add a manually set remap if we have on to set
		if (!isset($_POST[$prefix.'_add_remap_old_url']) || !isset($_POST[$prefix.'_add_remap_new_url'])) {
			return FALSE;
		}
		$old_url = trim($_POST[$prefix.'_add_remap_old_url']);
		$new_url = trim($_POST[$prefix.'_add_remap_new_url']);
		$expires = NULL;

		if (empty($old_url)) return FALSE;

		if (isset($_POST[$prefix.'_add_remap_expires_days'])) {
			$expires_days = (int)$_POST[$prefix.'_add_remap_expires_days'];
			if ($expires_days > 0) {
				$expires = strtotime('+ '.$expires_days.' days');
			}
		}

		$remap_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
		$never_delete = isset($_POST[$prefix.'_add_remap_never_delete']) ? 1 : 0;
		$remap_manager->addRemapURL($old_url, $new_url, $expires, $never_delete, FALSE);

		return TRUE;

	}//end processAddRemapURLs()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int	&$asset	the asset to check workflow completness for
	*
	* @return boolean
	* @access public
	*/
	function isWorkflowComplete(&$asset)
	{
		return $GLOBALS['SQ_SYSTEM']->getWorkflowManager()->isWorkflowComplete($asset->id);

	}//end isWorkflowComplete()


	/**
	* Paint a generic interface for editing assets that are notice linked to this asset
	*
	* Returns true if editable interface printed
	*
	* @param object		&$asset		the asset whose interface we are painting
	* @param array		$link_value	the current link
	* @param array		$type_codes	type_codes to pass to asset_finder()
	* @param string		$field_name	field_name to pass to asset_finder()
	* @param boolean	$multiple	Whether to allow multiple selections
	*
	* @return boolean
	* @access public
	*/
	function paintNoticeLinkedAsset(&$asset, $link_value, $type_codes=Array(), $field_name='', $multiple=FALSE)
	{
		if ($multiple) {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', $link_value);
		} else {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);
			$links = empty($link) ? Array() : Array($link);
		}

		if (empty($field_name)) {
			$field_name = $asset->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));
		}

		if (!$asset->writeAccess('links')) {
			if (empty($links)) {
				echo '<i>'.translate('none').'</i>';
			} else {
				foreach ($links as $link) {
					echo get_asset_tag_line($link['minorid']);
				}
			}
			return FALSE;
		} else {
			if ($multiple) {
				$current = Array();
				foreach ($links as $link) {
					$current[] = $link['minorid'];
				}
				multiple_asset_finder($field_name, $current, $type_codes);
			} else {
				$current = empty($links) ? 0 : $links[0]['minorid'];
				asset_finder($field_name, $current, $type_codes);
			}
			return TRUE;
		}

	}//end paintNoticeLinkedAsset()


	/**
	* Process a generic interface for editing assets that are notice linked to this asset
	*
	* @param object		&$asset		the asset whose interface we are painting
	* @param string		$link_value	the value to put on the link
	* @param string		$field_name	the name used for the asset finder
	* @param boolean	$multiple	Whether to allow multiple selections
	*
	* @return boolean
	* @access public
	*/
	function processNoticeLinkedAsset(&$asset, $link_value, $field_name='', $multiple=FALSE)
	{
		if (empty($field_name)) {
			$field_name = $asset->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));
		}
		if (!isset($_POST[$field_name])) return FALSE;

		if ($multiple) {

			$submitted_minorids = Array();
			foreach ($_POST[$field_name] as $elt) {
				if (!empty($elt['assetid'])) {
					$submitted_minorids[] = $elt['assetid'];
				}
			}
			$submitted_minorids = array_unique($submitted_minorids);

			$existing_minorids = Array();
			$to_delete = Array();
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', $link_value);
			foreach ($links as $link) {
				if (in_array($link['minorid'], $submitted_minorids)) {
					$existing_minorids[] = $link['minorid'];
				} else {
					$to_delete[] = $link['linkid'];
				}
			}
			$to_add = array_diff($submitted_minorids, $existing_minorids);
			$to_delete = array_unique($to_delete);

			if (empty($to_add) && empty($to_delete)) {
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$success = TRUE;

			foreach ($to_add as $schemaid) {
				$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
				if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $schema, SQ_LINK_NOTICE, $link_value)) {
					$success = FALSE;
					break;
				}
			}

			foreach ($to_delete as $linkid) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid)) {
					$success = FALSE;
					break;
				}
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		} else {

			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, $link_value);

			// if there has been no change, then there is nothing to do
			if (!empty($link) && $link['minorid'] == $_POST[$field_name]['assetid']) {
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$success = TRUE;

			// if there is a current link, we need to delete it
			if (!empty($link)) {
				$success = (bool) $asset->deleteLink($link['linkid']);
			}

			if ($success && !empty($_POST[$field_name]['assetid'])) {
				$selected_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$field_name]['assetid']);
				if (!is_null($selected_asset)) {
					$success = (bool) $asset->createLink($selected_asset, SQ_LINK_NOTICE, $link_value);
				}
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}
		return $success;

	}//end processNoticeLinkedAsset()


	/**
	* Return true if the search manager asset type is installed
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function isSearchInstalled(&$asset, $prefix)
	{
		return $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');

	}//end isSearchInstalled()


	/**
	* Return true if this asset is being created
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function isBeingCreated(&$asset)
	{
		return ($asset->id == 0);

	}//end isBeingCreated()


	/**
	* Return true if this asset is being created
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function isNotBeingCreated(&$asset)
	{
		return !$this->isBeingCreated($asset);

	}//end isNotBeingCreated()


	/**
	* Paints the Automatic Remapping checkbox
	*
	* @param Remap_Manager          $asset  the asset to which we belong
	* @param Backend_Outputter      $o      the outputter class
	* @param string                 $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAutomaticRemapSetting(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('lookups');

		if ($write_access) {
			$o->closeField();

			$o->openField(translate('web_paths_auto_add_remaps'));
				$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
				$remap_upon_webpath_change = $rm->attr('remap_upon_webpath_change');

				// paint checkbox
				check_box($prefix.'_remap_upon_webpath_change', TRUE, $remap_upon_webpath_change);
				label(translate('web_paths_auto_add_remaps_label'), $prefix.'_remap_upon_webpath_change');

				$o->note(translate('web_paths_auto_add_remaps_note'));
			$o->closeField();
		}
		
		return $write_access;

	}//end paintAutomaticRemapSetting()

		
	/**
	* Paint the interface for the choosing link type when creating assets
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLink(&$asset, &$o, $prefix)
	{
		$editable_link_types = Array(
							SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
							SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
						   );
						   
		if (isset($_REQUEST['parent_assetid'])){
			$restricted_sites = $GLOBALS['SQ_SYSTEM']->getUserPrefs('site', 'SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT');
			$parent_sites = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($_REQUEST['parent_assetid'], 'site'));
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($_REQUEST['parent_assetid'], 'site');
			if (!empty($asset_info)) $parent_sites[] = $_REQUEST['parent_assetid'];

			if (array_intersect($restricted_sites, $parent_sites) != Array()) {
				unset($editable_link_types[SQ_LINK_TYPE_1]);
				$_REQUEST['link_type'] = SQ_LINK_TYPE_2;
			}
		}
		
		$o->closeField();
		$o->openField(translate('link_type'));
		combo_box('link_type', $editable_link_types, FALSE, isset($_REQUEST['link_type']) ? $_REQUEST['link_type'] : SQ_LINK_TYPE_1);
		$o->closeField();
		return TRUE;

	}//end paintCreateLink()

	
	
	/**
	* Paints adding a new user condition
	*
	* @param Paint_Layout_Bodycopy	$asset	the asset to which we belong
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintConditions(&$asset, &$o, $prefix)
	{
		$keywords = Array();
		
		// 1. asset content
		$keywords[] = 'asset_contents';

		// 2. asset keywords (includes attributes)
		$asset_keywords = $asset->getAvailableKeywords();
		$keywords = array_merge($keywords, array_keys($asset_keywords));


		// lineage
		$keywords[] = 'asset_lineage';
		$keywords[] = 'asset_lineage_linked';

		// 3. asset metadata
		$keywords[] = 'asset_metadata_';
			
		$write_access = $asset->writeAccess('attributes');
		$condition_list = new Conditions_List($asset, 'available_conditions');
		return $condition_list->paintConditionsList($asset, $o, $prefix, $write_access, $keywords);

	}//end paintConditions()


	/**
	* Processses adding a new user condition
	*
	* @param Paint_Layout_Bodycopy	$asset	the asset to which we belong
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function processConditions(&$asset, &$o, $prefix)
	{
		$allowed_keywords = Array();
		$write_access = $asset->writeAccess('attributes');
		$condition_list = new Conditions_List($asset, 'available_conditions');
		return $condition_list->processConditionsList($asset, $o, $prefix, $write_access, $allowed_keywords);

	}//end processConditions()

	
}//end class
?>
