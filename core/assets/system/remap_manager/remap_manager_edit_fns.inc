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
* $Id: remap_manager_edit_fns.inc,v 1.46 2013/09/06 02:57:35 ewang Exp $
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
* @version $Revision: 1.46 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Remap_Manager_Edit_Fns extends Asset_Edit_Fns
{

	/**
	 * Set the number of results to show per page.
	 * This is used to work out paging and also when fetching
	 * the remaps to display (or for csv download).
	 *
	 * csv download increases this substantially so we do
	 * less trips to the db.
	 */
	private $num_per_page = 100;


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

	}//end __construct()


	/**
	 * _fetchRemapUrls
	 * Fetches the urls from the db.
	 * This is used both when displaying the remaps and also when
	 * downloading them so we don't fetch all remaps at once, and instead
	 * chunk the results based on $this->num_per_page.
	 *
	 * @param string	$url_filter			This can be used to limit the results to a particular url
	 * @param string	$remap_url_filter	Used to limit results to a particular remap url
	 * @param int		$page_num			Which "page" to return results for.
	 * 										Used by pagination and also csv download
	 * @param boolean	$return_count		Whether to return the count of the number of remaps.
	 *										Enabled by default. Disabled for csv download by subsequent calls
	 *										(So we only get the total number of remaps once)
	 *
	 * @return mixed	If $return_count is set to true (default), then the return array will be multi-dimensional:
	 *               ('total_remaps' => X, 'remap_rows' => array(remap_row, remap_row, remap_row)
	 *               If $return_count is false, then it only returns the remap_rows and the array is not multi
	 *               ie. $return = array(remap_row, remap_row, remap_row)
	 */
	private function _fetchRemapUrls($url_filter='', $remap_url_filter='', $page_num=1, $return_count=TRUE)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$offset = $this->num_per_page * ($page_num - 1);

		$remap_urls = Array();
		if ( (!empty($url_filter)) || (!empty($remap_url_filter))) {
			$bind_vars = Array(
						'url' 		=> (string) '%'.$url_filter.'%',
						'remap_url'	=> (string) '%'.$remap_url_filter.'%',
					);

			$where = ' WHERE url LIKE :url AND remap_url LIKE :remap_url';

		} else {
			$bind_vars = Array();

			$where = '';
		}

		$count_query = 'SELECT COUNT(url) FROM sq_ast_lookup_remap '.$where;

		/**
		 * we need the order by here so we get consistent results when we do paging
		 * url is the primary key so that's the best thing to use.
		*/
		$query = 'SELECT url, remap_url, expires, never_delete, auto_remap FROM sq_ast_lookup_remap '.$where.' ORDER BY url';

		$query = db_extras_modify_limit_clause($query, MatrixDAL::getDbType(), $this->num_per_page, $offset);

		try {
			if ($return_count) {
				$count_result = MatrixDAL::preparePdoQuery($count_query);
			}
			$result = MatrixDAL::preparePdoQuery($query);

			if (!empty($bind_vars)) {
				foreach ($bind_vars as $bind_var => $bind_value) {
					if ($return_count) {
						MatrixDAL::bindValueToPdo($count_result, $bind_var, $bind_value);
					}
					MatrixDAL::bindValueToPdo($result, $bind_var, $bind_value);
				}
			}
			if ($return_count) {
				$total_assets = MatrixDAL::executePdoOne($count_result);
			}

			$remap_urls = MatrixDAL::executePdoAll($result);
		} catch (Exception $e) {
			throw new Exception('Unable to get remap urls due to the following database error: '.$e->getMessage());
		}

		if ($return_count) {
			return Array(
					'total_remaps'	=> $total_assets,
					'remap_rows'	=> $remap_urls,
				);
		}
		return $remap_urls;
	}// end _fetchRemapUrls()


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
		
		// check if we need to filter the url or remap url
		$url_filter = array_get_index($_POST, $prefix.'_URL_FILTER', '');
		$remap_url_filter = array_get_index($_POST, $prefix.'_REMAP_URL_FILTER', '');

		if ( (!empty($url_filter)) || (!empty($remap_url_filter))) {
			$remap_entries_tag = 'remap_entries_filtered';
			$no_entries_tag = 'no_url_remappings_filtered';
		} else {
			$remap_entries_tag = 'remap_entries';
			$no_entries_tag = 'no_url_remappings';
		}

		$page_num = array_get_index($_POST, $prefix.'_remap_urls_page', 1);
		$page_num = max($page_num, 1);

		$remap_info = $this->_fetchRemapUrls($url_filter, $remap_url_filter, $page_num, TRUE);
		$total_assets = $remap_info['total_remaps'];
		$remap_urls = $remap_info['remap_rows'];

		$o->openField(translate('purge_all_remaps'));
		combo_box($prefix.'_purge_all_remaps', Array(0 => translate('no'), 1 => translate('yes')), FALSE, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm(js_translate('confirm_purge_all_remaps'))) { this.value = 0; } } \"". ($write_access ? '' : "disabled"));
		$o->note(translate('purge_all_remaps_note'));
		$o->closeField();

		
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
			echo translate($no_entries_tag);
			return FALSE;
		}

		$num_pages = ceil($total_assets / $this->num_per_page);

		if ($num_pages == 1) {
			$page_tag = translate('page_number', $page_num, $num_pages);
		} else {
			$page_tag = translate('page_number_with_pager', $page_num, $num_pages, $prefix.'_remap_urls_page');
		}
		$asset_count_tag = translate('item_range', ($page_num - 1) * $this->num_per_page + 1, $total_assets, $total_assets, strtolower(translate($remap_entries_tag)));

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
				<td class="sq-backend-table-header-header"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></td>
				<td class="sq-backend-table-header-header" colspan="<?php echo ($write_access === TRUE) ? 5 : 4 ?>" style="text-align: right; font-weight: normal"><?php echo $asset_count_tag; ?></span></td>
			</tr>
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('url'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('remap_url'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('expires'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('type'); ?></td>
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
					<?php echo translate('never_delete');
					
					if ($write_access) {						
						check_box($prefix.'never_delete_select_all', '1', FALSE, "check_all(document.main_form, '{$prefix}_never_delete_remap', this.checked);");						
					}
				?>					
				</td>
				
			</tr>
			<?php
				foreach ($remap_urls as $url_info) {
					if (empty($url_info['expires'])) {
						$expires = translate('never');
					} else {
						$expires = $GLOBALS['SQ_SYSTEM']->datetime(iso8601_ts($url_info['expires']));
					}

					

					// To let the process function know the remaps on the current page
					hidden_field($prefix.'_current_page_remaps['.$url_info['url'].']', $page_num);

					$warning = '';
					// check to see if the URL is actually being used by a real asset within the system
					// because if it is, this remap will never be used
					$url_asset = null;
					// ?a=xx sytle url
					if(strpos($url_info['url'], '//?a=')) {
					    preg_match('/\?a=([0-9:]+)/', $url_info['url'], $matches);
					    if(isset($matches[1])) {
						$url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($matches[1]);
						if(!empty($url))
						    $url_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[1]);						    
					    }
					}
					else {
					    // normal url
					    $old_url_parts = parse_url($url_info['url']);
					    $protocol = array_get_index($old_url_parts, 'scheme', NULL);
					    if(!empty($protocol)) {
						$url = array_get_index($old_url_parts, 'host', '').array_get_index($old_url_parts, 'path', '');
						$url_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, TRUE, TRUE);
					    }
					}
					 if(!empty($url_asset))
						$warning .= translate('url_currently_in_use', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($url_asset->type(), 'name'), translate('asset_format', $url_asset->name, $url_asset->id));
					 
					
					// check to see if the new URL is a valid asset URL
					$warning_remap = '';
					$new_url_parts = parse_url($url_info['remap_url']);
					$protocol = array_get_index($new_url_parts, 'scheme', NULL);
					if(!empty($protocol)) {
					    $url = array_get_index($new_url_parts, 'host', '').array_get_index($new_url_parts, 'path', '');
					    //  does it matche with root matrix url
					    $root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
					    $is_matrix_url = FALSE;
					    foreach($root_urls as $root_url) {
						if(strpos($url, $root_url) === 0)
							$is_matrix_url = TRUE;
					    }
					   // if it's not a Matrix URL, no need to show warning
					    if($is_matrix_url) {
						 // if it's one of special url, no need to show warning
						if(strpos($url, '__data') === FALSE && strpos($url, '__lib') === FALSE && strpos($url, '__fudge') === FALSE) {
						    $url_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, TRUE, TRUE);
						    $count_query = 'SELECT count(*) FROM sq_ast_lookup_remap WHERE url = :url ';
						    $count_result = MatrixDAL::preparePdoQuery($count_query);
						    MatrixDAL::bindValueToPdo($count_result, 'url', $url_info['remap_url']);
						    $remap_url = MatrixDAL::executePdoOne($count_result);
						    
						    // if the new url is not an valid asset url, neither another redirect url, show warning
						    if(empty($url_asset) && empty($remap_url))
							$warning_remap = 'Invalid remap URL';
						    }
					    }
					}
					
					// check if the remap has expired
					$warning_expire = '';
					if(isset($url_info['expires']) && !empty($url_info['expires'])) {
					    if(strtotime($url_info['expires']) < time())
						$warning_expire = 'Remap expired';	
					}
					
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php echo $url_info['url']; ?><br/><span style="color: #FF0000;"><b><?php echo $warning; ?></b></span></td>
					<td class="sq-backend-table-cell"><?php echo $url_info['remap_url']; ?><br/><span style="color: #FF0000;"><b><?php echo $warning_remap; ?></b></span></td>
					<td class="sq-backend-table-cell"><?php echo $expires; ?><br/><span style="color: #FF0000;"><b><?php echo $warning_expire; ?></b></span></td>
					<td class="sq-backend-table-cell"><?php echo ((int)$url_info['auto_remap'] === 1) ? translate('automatic') : (($url_info['auto_remap'] === NULL) ? translate('unknown') : translate('manual')); ?></td>
					<?php
						if ($write_access) {
						$never_delete_remap = (isset($_POST[$prefix.'_never_delete_remap']) && array_key_exists($url_info['url'],$_POST[$prefix.'_never_delete_remap'])) || $url_info['never_delete'];
							?>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_delete_remap['.$url_info['url'].']', 1, FALSE, '', $never_delete_remap ? 'disabled' : '');
								?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
								check_box($prefix.'_never_delete_remap['.$url_info['url'].']', 1, $never_delete_remap);
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
		
		// Only update the remaps in current page
		$urls_to_check = (isset($_POST[$prefix.'_never_delete_remap'])) ? array_keys($_POST[$prefix.'_never_delete_remap']) : array();
		$current_page_remaps  = array_get_index($_POST, $prefix.'_current_page_remaps', Array());
		$urls_in_current_page = array_keys($current_page_remaps);
		$urls_to_uncheck = array_diff($urls_in_current_page, $urls_to_check);

		try {
			$bind_vars = Array(
							'never_delete'		=> '0',
							'urls_to_update'	=> $urls_to_uncheck,
						);
			$bind_vars_add = Array(
							'never_delete'		=> '1',
							'urls_to_update'	=> $urls_to_check,
						);

			if (!empty($urls_to_uncheck)) {
				MatrixDAL::executeQuery('remap_manager', 'addNeverDeleteToCurrentRemapURL', $bind_vars);
			}
			if (!empty($urls_to_check)) {
				MatrixDAL::executeQuery('remap_manager', 'addNeverDeleteToCurrentRemapURL', $bind_vars_add);
			}
		} catch (DALException $e) {
			throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
		}

		// Purge all remaps with "Never Delete" option set to "No"
		if (isset($_POST[$prefix.'_purge_all_remaps']) && $_POST[$prefix.'_purge_all_remaps']) {
			try {
				$bind_vars = Array('never_delete' => '0');
				MatrixDAL::executeQuery('remap_manager', 'deleteRemapsByNeverDeleteOption', $bind_vars);
			} catch (DALException $e) {
				throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
			}
		}		
		
		if (isset($_POST[$prefix.'_delete_remap'])) {
			$urls = array_keys($_POST[$prefix.'_delete_remap']);
			if (isset($_POST[$prefix.'_never_delete_remap'])) {
				foreach ($_POST[$prefix.'_never_delete_remap'] as $url => $not_deletable) {
					foreach ($urls as  $index => $url_to_delete) {
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
	* @return void
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

		$o->openField(translate('csv_upload_header'));
			check_box($prefix.'_csv_header');
			$o->note(translate('csv_upload_header_example'));
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
			$csv_start = 1;
			if (isset($_POST[$prefix.'_csv_header'])) {
				$csv_start = 2;
			}
			$csv->import($csv_start);

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

				// if the value is not set or isnt what we expect
				// set never_delete option to the default behaviour
				// configured on the remap_manager details screen
				if (!isset($line_value[3]) || trim($line_value[3]) == '') {
					$never_delete = $asset->attr('never_delete_remap_default');
				} else {
					$never_delete = trim($line_value[3]);
				}

				$asset->addRemapURL($old_url, $new_url, $int_expires, $never_delete, FALSE);
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

		$asset->addRemapURL($old_url, $new_url, $expires, $never_delete, FALSE);

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
	* @param string				$prefix	prefix for the form element
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
	
	
	/**
	* Paints the Never Delete Remap checkbox
	*
	* @param Remap_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o	the outputter class
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNeverDeleteRemapCheckbox(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
			$write_access = $asset->writeAccess('attributes');
			$never_delete_remap_default = $asset->attr('never_delete_remap_default');

	$o->closeField();
	$o->openField(translate('remap_manager_never_delete_remap_default_question'));

			// paint checkbox
			if ($write_access) {
					check_box($prefix.'_never_delete_remap_default', TRUE, $never_delete_remap_default);
			} else {
					?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $never_delete_remap_default ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
					<?php
			}

	$o->note(translate('remap_manager_never_delete_remap_default_note'));

	$o->closeField();

			return $write_access;

	}//end paintNeverDeleteRemapCheckbox()


	/**
	* Process the Never Delete Remap checkbox
	*
	* @param Remap_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o	the outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNeverDeleteRemapCheckbox(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
			if (!$asset->writeAccess('attributes')) return FALSE;

			$never_delete_remap_default = isset($_POST[$prefix.'_never_delete_remap_default']);
			$asset->setAttrValue('never_delete_remap_default', $never_delete_remap_default);

			return TRUE;

	}//end processNeverDeleteRemapCheckbox()


	/**
	* Paint interface for the exporting of remap entries to a CSV file
	*
	* @param Remap_Manager		$asset	The asset that we are painting
	* @param Backend_Outputter	$o		The outputter to inject the output into
	* @param string				$prefix Prefix for form elements
	*
	* @return boolean
	*/
	public function paintExportEntriesToCSV(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess();

		if ($write_access === TRUE) {
			$this->processExportEntriesToCSV($asset, $o, $prefix);

			normal_button($prefix.'_export_to_csv', translate('download_file'), 'this.form.'.$prefix.'_download_file.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_download_file.value = 0; return true;');
			hidden_field($prefix.'_download_file', 0);
		}

		return FALSE;

	}//end paintExportEntriesToCSV()


	/**
	* Process interface for the exporting of remap entries to a CSV file
	*
	* (...and actually do the exporting if we're asked to do it!)
	*
	* @param Remap_Manager		$asset	The asset that we are painting
	* @param Backend_Outputter	$o		The outputter to inject the output into
	* @param string				$prefix Prefix for form elements
	*
	* @return boolean
	*/
	public function processExportEntriesToCSV(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess() === FALSE) {
			return FALSE;
		}

		$download_file = (int)array_get_index($_POST, $prefix.'_download_file', 0);

		if ($download_file === 1) {
			// check if we need to filter the url or remap url
			$url_filter = array_get_index($_POST, $prefix.'_URL_FILTER', '');
			$remap_url_filter = array_get_index($_POST, $prefix.'_REMAP_URL_FILTER', '');

			$page = 1;

			// since we're going to be downloading this file, lets set a bigger per_page size
			$this->num_per_page = 10000;

			$remap_urls = $this->_fetchRemapUrls($url_filter, $remap_url_filter, $page, FALSE);

			if (empty($remap_urls)) {
				// Cannot export remap entries to CSV, there are no entries to export
				trigger_localised_error('CORE0287', E_USER_WARNING);
			} else {

				require_once SQ_FUDGE_PATH.'/csv/csv.inc';

				$csv = new CSV();
				$csv->setFilename('remap_entries.csv');
				$csv->setDeliminator(',');

				$headers = Array('Original URL', 'Remapped URL', 'Expiry', 'Never Delete', 'Automatic');
				$csv->setFieldHeaders($headers);

				$header_printed = FALSE;
				while (!empty($remap_urls)) {
					$values = Array();
					foreach ($remap_urls as $remap_url) {
						$value_row = Array(
								$remap_url['url'],
								$remap_url['remap_url'],
								$remap_url['expires'],
							);
						if ($remap_url['never_delete'] !== NULL) {
							$value_row[] = (boolean)($remap_url['never_delete']) ? 1 : 0;
						} else {
							$value_row[] = NULL;
						}
						if ($remap_url['auto_remap'] !== NULL) {
							$value_row[] = (boolean)($remap_url['auto_remap']) ? 1 : 0;
						} else {
							$value_row[] = NULL;
						}
						$values[] = $value_row;
					}

					$csv->setValues($values);

					if (!$header_printed) {
						$csv->export(FALSE);
						$header_printed = TRUE;
					} else {
						$csv->exportValues(FALSE);
					}
					$page++;
					// see if we have more remaps to get,
					// eventually this will be an empty array.
					$remap_urls = $this->_fetchRemapUrls($url_filter, $remap_url_filter, $page, FALSE);
				}

				// We don't want the rest of the interface printed, so exit.
				exit(0);

			}//end else if there are any remaps (or not)
		}//end if download is 1

		return FALSE;

	}//end processExportEntriesToCSV()


	/**
	* Paints the Preserve URL Paramaters checkbox
	*
	* @param Remap_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o	the outputter class
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintPreserveUrlParametersCheckbox(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
			$write_access = $asset->writeAccess('attributes');
			$preserve_url_parameters = $asset->attr('preserve_url_parameters');



			// paint checkbox
			if ($write_access) {
					check_box($prefix.'_preserve_url_parameters', TRUE, $preserve_url_parameters);
			} else {
					?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $preserve_url_parameters ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
					<?php
			}

	
			return $write_access;

	}//end paintPreserveUrlParametersCheckbox()


	/**
	* Process the Preserve URL Paramaters checkbox
	*
	* @param Remap_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o	the outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processPreserveUrlParametersCheckbox(Remap_Manager $asset, Backend_Outputter $o, $prefix)
	{
			if (!$asset->writeAccess('attributes')) return FALSE;

			$preserve_url_parameters = isset($_POST[$prefix.'_preserve_url_parameters']);
			$asset->setAttrValue('preserve_url_parameters', $preserve_url_parameters);

			return TRUE;

	}//end processPreserveUrlParametersCheckbox()

	
}//end class

?>
