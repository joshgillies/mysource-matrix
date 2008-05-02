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
* $Id: asset_edit_fns.inc,v 1.77 2008/05/02 04:51:19 bpearson Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.77 $
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
				text_box($prefix.'_paths[]', $path, 25);
			} else {
				echo $path;
			}
			echo '<br/>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 25);

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

		$link_offset = array_get_index($_POST, $prefix.'_link_offset', 0);

		$num_per_page = 100;
		$total_assets = count($remap_urls);
		$remap_urls = array_slice($remap_urls, $link_offset, $num_per_page);
		$num_pages = ceil($total_assets / $num_per_page);

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header-header" colspan="4"><b><?php
					hidden_field($prefix.'_link_offset', $link_offset);
					if ($link_offset > 0) {
						?><a title="<?php echo translate('pagination_go_to_first') ?>" style="text-decoration: none; color: #fff" href="#" onclick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" style="color: #333"><?php
					}
					?>
					&lt;&lt;
					<?php
					if ($link_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if ($link_offset > 0) {
						?><a title="<?php echo translate('pagination_go_to_previous') ?>" style="text-decoration: none; color: #fff" href="#" onclick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" style="color: #333"><?php
					}
					?>
					&lt;
					<?php
					if ($link_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp; &nbsp; <?php echo translate('page_number', round(($link_offset + $num_per_page) / $num_per_page), ceil($total_assets / $num_per_page)); ?> &nbsp; &nbsp;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('pagination_go_to_next') ?>" style="text-decoration: none; color: #fff" href="#" onclick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" style="color: #333"><?php
					}
					?>
					&gt;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('pagination_go_to_last') ?>" style="text-decoration: none; color: #fff" href="#" onclick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" style="color: #333"><?php
					}
					?>
					&gt;&gt;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?></b>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('url'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('expires'); ?></td>
				<?php
					if ($admin_access) {
						?>
						<td class="sq-backend-table-header">
							<?php
							check_box($prefix.'_select_all', '1', FALSE, "check_all(document.main_form, '{$prefix}_delete_remap', this.checked);");
							label(translate('delete_question'), $prefix.'_select_all');
							?>
						</td>
						<?php
					}
				?>
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
					<td class="sq-backend-table-cell"><?php echo $expires; ?></td>
					<?php
						if ($admin_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_delete_remap['.$url_info['url'].']');
								?>
							</td>
							<?php
						}
					?>
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
					echo 'http://'.$data['url'], '<br/>';
				}
				if ($data['https']) {
					echo 'https://'.$data['url'], '<br/>';
				}
			}
		}

		if ($asset->force_secure) {
			echo '<br>';
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
		$remap_manager->addRemapURL($old_url, $new_url, $expires);

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

				$o->note(translate('web_paths_auto_add_remaps_note'));
			$o->closeField();
                }


                return $write_access;

        }//end paintAutomaticRemapSetting()


}//end class
?>
