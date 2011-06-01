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
* $Id: remap_manager_edit_fns.inc,v 1.29 2009/01/07 05:47:46 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Remap_Manager_Edit_Fns
*
* Edit functions for the remap manager.
*
* @see Asset_Edit_Fns
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Remap_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function __construct()
	{
		parent::__construct();

		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'all';

		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);

	}//end constructor


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param Remap_Manager		$asset	the remap manager
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function paintEditRemapURLs(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('all');

		$link_offset = array_get_index($_POST, $prefix.'_link_offset', 0);

		// check if we need to filter the url or remap url
		$url_filter = array_get_index($_POST, $prefix.'_URL_FILTER', '');
		$remap_url_filter = array_get_index($_POST, $prefix.'_REMAP_URL_FILTER', '');

		$remap_urls = Array();
		if ( (!empty($url_filter)) || (!empty($remap_url_filter))) {
			$bind_vars = Array(
					'url' 		=> (string) '%'.$url_filter.'%',
					'remap_url'	=> (string) '%'.$remap_url_filter.'%',
					);
			try {
				$remap_urls	= MatrixDAL::executeAll('core', 'getRemapUrlsFilter', $bind_vars);
			} catch (Exception $e) {
				throw new Exception("Unable to get remap urls due to the following database error: ".$e->getMessage());
			}

		} else {
			try {
				$remap_urls	= MatrixDAL::executeAll('core', 'getRemapUrls');
			} catch (Exception $e) {
				throw new Exception("Unable to get remap urls due to the following database error: ".$e->getMessage());
			}
		}

		$o->openField('Original URL Filter');
		text_box($prefix.'_URL_FILTER', $url_filter, 30);
		$o->note(translate('remap_manager_url_filter_note'));
		$o->closeField();
		$o->openField('Remapped URL Filter');
		text_box($prefix.'_REMAP_URL_FILTER', $remap_url_filter, 30);
		$o->note(translate('remap_manager_remap_url_filter_note'));
		$o->closeField();

		$o->openField('&nbsp;');

		if (empty($remap_urls)) {
			echo translate('no_url_remappings');
			return FALSE;
		}

		$num_per_page = 100;
		$total_assets = count($remap_urls);
		$remap_urls = array_slice($remap_urls, $link_offset, $num_per_page);
		$num_pages = ceil($total_assets / $num_per_page);

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header-header" colspan="5"><b><?php
					hidden_field($prefix.'_link_offset', $link_offset);
					if ($link_offset > 0) {
						?><a title="<?php echo translate('pagination_go_to_first') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
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
						?><a title="<?php echo translate('pagination_go_to_previous') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
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
						?><a title="<?php echo translate('pagination_go_to_next') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
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
						?><a title="<?php echo translate('pagination_go_to_last') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
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
				<td class="sq-backend-table-header"><?php echo translate('remap_url'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('expires'); ?></td>
				<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header">
							<?php
							label(translate('delete_question'), $prefix.'_select_all');
							check_box($prefix.'_select_all', '1', FALSE, "check_all(document.main_form, '{$prefix}_delete_remap', this.checked);");							
							?>
						</td>
						
						<?php
					}
				?>
				<td class="sq-backend-table-header">
					<?php echo translate('never_delete'); ?>
				</td>
			</tr>
			<?php
				foreach ($remap_urls as $url_info) {
					if (empty($url_info['expires'])) {
						$expires = translate('never');
					} else {
						$expires = $GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($url_info['expires']));
					}

					// any warning text to show for this remapped URL (any possible problems)
					$warning = '';

					// check to see if the URL is actually being used by a real asset within the system
					// because if it is, this remap will never be used
					if (strpos($url_info['url'], '://') !== FALSE) {
						list($protocol, $url) = explode('://', $url_info['url']);
						$url_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, TRUE, TRUE);
						if (!is_null($url_asset)) {
							$warning .= translate('url_currently_in_use', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($url_asset->type(), 'name'), translate('asset_format', $url_asset->name, $url_asset->id));
						}
					}
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $url_info['url']; ?><br/><span style="color: #FF0000;"><b><?php echo $warning; ?></b></span></td>
					<td class="sq-backend-table-cell"><?php echo $url_info['remap_url']; ?></td>
					<td class="sq-backend-table-cell"><?php echo $expires; ?></td>
					<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_delete_remap['.$url_info['url'].']');
								?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
								if ((isset($_POST[$prefix.'_never_delete_remap']) && array_key_exists($url_info['url'],$_POST[$prefix.'_never_delete_remap'])) || $url_info['never_delete']) {
									check_box($prefix.'_never_delete_remap['.$url_info['url'].']',1, TRUE);
									
								} else {
									check_box($prefix.'_never_delete_remap['.$url_info['url'].']');
								}
								?>
							</td>
							<?php
						} else {
							if ((isset($_POST[$prefix.'_never_delete_remap']) && array_key_exists($url_info['url'],$_POST[$prefix.'_never_delete_remap'])) || $url_info['never_delete']) {
								?>
									<td class="sq-backend-table-cell">
										<img src="<?php echo sq_web_path('lib'); ?>/web/images/tick.gif" width="12" height="12" />
									</td>
								<?php
							} else {
								?>
									<td class="sq-backend-table-cell">
										<img src="<?php echo sq_web_path('lib'); ?>/web/images/cross.gif" width="12" height="12" />
									</td>
								<?php
							}
						}
					?>
				</tr>
				<?php
				}//end foreach
			?>
		</table>
		<?php

		return $write_access;

	}//end paintEditRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param Remap_Manager		$asset	the remap manager
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function processEditRemapURLs(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('all')) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (isset($_POST[$prefix.'_never_delete_remap'])) {
			$urls_to_update = array_keys($_POST[$prefix.'_never_delete_remap']);
			try {
				$bind_vars = Array(
								'never_delete'		=> '0',
								'urls_to_update'	=> $urls_to_update,
								  );
				$bind_vars_add = Array(
									'never_delete'		=> '1',
									'urls_to_update'	=> $urls_to_update,
								  );
				MatrixDAL::executeQuery('remap_manager', 'removeNeverDeleteFromCurrentRemapURL', $bind_vars);
				MatrixDAL::executeQuery('remap_manager', 'addNeverDeleteToCurrentRemapURL', $bind_vars_add);
			} catch (DALException $e) {
				throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
			}
		} else {
			try {
				$bind_vars = Array('never_delete' => '0');
				MatrixDAL::executeQuery('remap_manager', 'updateAllRemapURL', $bind_vars);
			} catch (Exception $e) {
				throw new Exception("Unable to get remap urls due to the following database error: ".$e->getMessage());
			}			
		}

		if (isset($_POST[$prefix.'_delete_remap'])) {
			$urls = array_keys($_POST[$prefix.'_delete_remap']);
			if (isset($_POST[$prefix.'_never_delete_remap'])) {
				foreach ($_POST[$prefix.'_never_delete_remap'] as $url => $not_deletable) {
					foreach ($urls as  $index => $url_to_delete){
						if ($url == $url_to_delete && !$not_deletable == 0) {
							unset($urls[$index]);
						}
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
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param Remap_Manager		$asset	the remap manager
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function paintAddRemapURLs(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$o->note(translate('url_remap_explanation'));
		$o->closeField();

		$o->openField(translate('old_url'));
			text_box($prefix.'_add_remap_old_url', '', 50);
		$o->closeField();

		$o->openField(translate('new_url'));
			text_box($prefix.'_add_remap_new_url', '', 50);
		$o->closeField();
		
		$o->openField(translate('never_delete'));
			check_box($prefix.'_add_never_delete_option');
		$o->closeField();

		$o->openField(translate('expires'));
			text_box($prefix.'_add_remap_expires_days', '', 5);
			label(translate('days'), $prefix.'_add_remap_expires_days');
		$o->closeField();

		$o->openField(translate('csv_upload'));
			file_upload($prefix.'_add_remap_csv');

			// csv upload instructions
			$o->note(translate('csv_upload_instructions'));
			$o->note(translate('csv_upload_instructions_example_1').'<br />'.translate('csv_upload_instructions_example_2').'<br />'.translate('csv_upload_instructions_example_3'));
		$o->closeField();

	}//end paintAddRemapURLs()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param Remap_Manager		$asset	the remap manager
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function processAddRemapURLs(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('all')) return FALSE;

		// do the CSV upload first
		$file_info = get_file_upload_info($prefix.'_add_remap_csv');
		if (!empty($file_info)) {
			require_once SQ_FUDGE_PATH.'/csv/csv.inc';
			$csv = new CSV($file_info['tmp_name']);
			$csv->import();

			foreach ($csv->values as $line_value) {
				$old_url     = trim($line_value[0]);
				$new_url     = trim($line_value[1]);
				if (empty($old_url) || empty($new_url)) {
					return FALSE;
				}

				$expires = array_key_exists(2, $line_value) ? trim($line_value[2]) : '';

				if (empty($expires)) {
					$int_expires = NULL;
				} else {
					$int_expires = iso8601_ts($expires);
				}
				$never_delete = trim($line_value[3]);

				$asset->addRemapURL($old_url, $new_url, $int_expires, $never_delete);
			}
		}


		// now add a manually set remap if we have on to set
		if (!isset($_POST[$prefix.'_add_remap_old_url']) || !isset($_POST[$prefix.'_add_remap_new_url'])) {
			return FALSE;
		}
		$old_url = trim($_POST[$prefix.'_add_remap_old_url']);
		$new_url = trim($_POST[$prefix.'_add_remap_new_url']);
		$expires = NULL;
		if (isset($_POST[$prefix.'_add_never_delete_option'])) {
			$never_delete = trim($_POST[$prefix.'_add_never_delete_option']);
		} else {
			$never_delete = '0' ;
		}

		if (empty($old_url) || empty($new_url)) {
			return FALSE;
		}

		if (isset($_POST[$prefix.'_add_remap_expires_days'])) {
			$expires_days = (int)$_POST[$prefix.'_add_remap_expires_days'];
			if ($expires_days > 0) {
				$expires = strtotime('+ '.$expires_days.' days');
			}
		}

		$asset->addRemapURL($old_url, $new_url, $expires, $never_delete);

		return TRUE;

	}//end processAddRemapURLs()


	/**
	* Paints the root node selection box
	*
	* @param Remap_Manager		$asset	the remap manager
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function paintAddPageNotFoundRemap(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$link_to_404 = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, '404');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			if (!empty($link_to_404)) {
				echo get_asset_tag_line($link_to_404['minorid']);
			} else {
				echo '<em>'.translate('none').'</em>';
			}
		} else {
			asset_finder($prefix.'_404asset', (!empty($link_to_404)) ? $link_to_404['minorid'] : '');
		}

		return $write_access;

	}//end paintAddPageNotFoundRemap()


	/**
	* Processes the value input from root node selection box
	*
	* @param Remap_Manager		$asset	the remap manager
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	a unique prefix for variables
	*
	* @return void
	* @access public
	*/
	public function processAddPageNotFoundRemap(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_404asset']['assetid'])) {
			return FALSE;
		}
		$new_404_asset  = $_POST[$prefix.'_404asset']['assetid'];

		// getting existing root link
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, '404');

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_404_asset) {
			return FALSE;
		}

		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_404_asset) {
			$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_404_asset);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, '404')) {
					$ok = FALSE;
					trigger_localised_error('CORE0027', E_USER_WARNING, $root->name, $root->id);
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

	}//end processAddPageNotFoundRemap()


	/**
        * Paints the Automatic Remapping checkbox
        *
        * @param Remap_Manager		$asset	the asset to which we belong
        * @param Backend_Outputter	$o	the outputter class
        * @param string			$prefix	prefix for the form element
        *
        * @return boolean
        * @access public
        */
        public function paintAutomaticRemappingCheckbox(Remap_Manager $asset, Backend_Outputter $o, $prefix)
        {
                $write_access = $asset->writeAccess('attributes');
                $remap_upon_webpath_change = $asset->attr('remap_upon_webpath_change');

		$o->closeField();
		$o->openField(translate('remap_manager_automatically_add_remaps_question'));

                // paint checkbox
                if ($write_access) {
                        check_box($prefix.'_remap_upon_webpath_change', TRUE, $remap_upon_webpath_change);
                } else {
                        ?>
                                <img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $remap_upon_webpath_change ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
                        <?php
                }

		$o->note(translate('remap_manager_auto_add_remaps_note'));

		$o->closeField();

                return $write_access;

        }//end paintAutomaticRemappingCheckbox()


        /**
        * Process the Automatic Remapping checkbox
        *
        * @param Remap_Manager		$asset	the asset to which we belong
        * @param Backend_Outputter	$o	the outputter class
        * @param string			$prefix	prefix for the form element
        *
        * @return boolean
        * @access public
        */
        public function processAutomaticRemappingCheckbox(Remap_Manager $asset, Backend_Outputter $o, $prefix)
        {
                if (!$asset->writeAccess('attributes')) return FALSE;

                $remap_upon_webpath_change = isset($_POST[$prefix.'_remap_upon_webpath_change']);
                $asset->setAttrValue('remap_upon_webpath_change', $remap_upon_webpath_change);

                return TRUE;

        }//end processAutomaticRemappingCheckbox()


}//end class

?>
