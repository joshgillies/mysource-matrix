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
* $Id: content_type_nest_content_edit_fns.inc,v 1.80 2013/07/31 06:22:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.80 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Edit_Fns extends Content_Type_Edit_Fns
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
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object	&$asset	the nest content content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('links') && $asset->writeAccess('attributes')) {
			$nested_asset = $this->_getNestedAsset($asset);

			if (!is_null($nested_asset)) {
				$paint_with = $asset->attr('paint_with');
				$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
				$paint_layout_id = (is_null($arbitrary_paint_layout)) ? 0 : $arbitrary_paint_layout->id;
				$asset_link = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($nested_asset->id);
				if (empty($asset_link)) {
					$asset_link = strip_url(current_url(FALSE, TRUE)).'/?a='.$nested_asset->id;
				}
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-cell-alt"><b><?php echo translate('cms_ct_nest_content_currently_nesting', $nested_asset->attr('name')) ?><?php require_once SQ_INCLUDE_PATH.'/general.inc'; echo ' : '.get_asset_tag_line($nested_asset->id);?></b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', $nested_asset->id, Array('asset' => 'D')); ?></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><b><a href="<?php echo $asset_link; ?>" id="sq_preview_new_window" target="_blank"><?php echo translate('cms_ct_nest_content_click_to_preview') ?></a></b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell-alt"><b>Dynamic Options:</b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell">
						<table>
							<colgroup>
								<col width="50%" />
								<col width="50%" />
							</colgroup>
							<thead>
								<tr>
									<th>Parameter</th>
									<th>GET Variable Name</th>
								</tr>
							</thead>
							<tbody>
								<tr>
									<td>Asset ID to nest the contents of:</td>
									<td><?php text_box($prefix.'_dynamic_nest_get', $asset->attr('dynamic_nest_get'), 30) ?></td>
								</tr>
							</tbody>
							</table>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell-alt"><b>Paint this asset:</b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell">
							<?php
							radio_button($prefix.'_paint_with', 'raw', ($paint_with == 'raw' ? 1 : 0));
							label('Raw (no Paint Layout)', $prefix.'_paint_with_raw');
							?><br/>
							<?php
							radio_button($prefix.'_paint_with', 'default', ($paint_with == 'default' ? 1 : 0));
							label('Using default Paint Layout (based on lookup rules)', $prefix.'_paint_with_default');
							?><br/>
							<?php
							radio_button($prefix.'_paint_with', 'arbitrary', ($paint_with == 'arbitrary' ? 1 : 0));
							label('Using an arbitrary Paint Layout: ', $prefix.'_paint_with_arbitrary');
							asset_finder($prefix.'_paint_with_layout', $paint_layout_id, Array('paint_layout_page' => 'D'));
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell-alt"><b>Send the following additional GET variables to the asset:</b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
							// get delete icon, with new lines replaced
							ob_start();
							sq_print_icon(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png', 16, 16, translate('delete'), translate('delete'), ' style="border:none"');
							$delete_icon = ob_get_clean();
							$delete_icon = str_replace("\n", ' ', $delete_icon);

							$additional_get_vars = $asset->attr('additional_get');
							if (empty($additional_get_vars)) {
								// max() won't work on an empty array
								$last_key = -1;
							} else {
								$last_key = max(array_keys($additional_get_vars));
							}
						?>
						<script type="text/javascript">
							if (typeof <?php echo $asset->type() ?> == 'undefined') {
								<?php echo $asset->type() ?> = {
									nextIndex: [],

									/**
									 * Adds a new row.
									 */
									addGetRow: function(prefix) {
										node = document.getElementById(prefix + '_additional_get_body');
										index = this.nextIndex[prefix];

										// create a new row
										tr = document.createElement('tr');
										tr.id = prefix + '_additional_get_row_' + index;

										// build the cells
										td1 = document.createElement('td');
										td1.innerHTML = '<?php text_box("' + prefix + '_additional_get[' + index + '][name]", '', 20) ?>';
										td2 = document.createElement('td');
										td2.innerHTML = '<?php text_box("' + prefix + '_additional_get[' + index + '][value]", '', 30) ?>'
										td3 = document.createElement('td');
										td3.innerHTML = '<a href="" onclick="<?php echo $asset->type() ?>.removeGetRow(\'' + prefix +'\', \'' + index + '\'); return false;"><?php echo $delete_icon; ?></a>';

										// Now put them all together
										tr.appendChild(td1);
										tr.appendChild(td2);
										tr.appendChild(td3);
										node.appendChild(tr);

										// Go to next index
										this.nextIndex[prefix]++;

										// Update display of "no variable" status
										this.updateNoVarStatus(prefix);
									},

									/*
									 * Removes a row.
									 */
									removeGetRow: function(prefix, index) {
									 	node = document.getElementById(prefix + '_additional_get_row_' + index);
									 	node.parentNode.removeChild(node);
										this.updateNoVarStatus(prefix);
									},

									/*
									 * Updates display of the "no variable" body.
									 */
									updateNoVarStatus: function(prefix) {
										node = document.getElementById(prefix + '_additional_get_body');
										var noVarBody = document.getElementById(prefix + '_additional_get_none');
										noVarBody.style.display = (node.getElementsByTagName('tr').length > 0) ? 'none' : 'block';
									}
								};
							}//end if

							// Set the first new index for newly created rows
							<?php echo $asset->type(); ?>.nextIndex['<?php echo $prefix; ?>'] = <?php echo ($last_key + 1); ?>;
						</script>
						<table>
							<colgroup>
								<col width="40%" />
								<col width="60%" />
								<col />
							</colgroup>
							<thead>
								<tr>
									<th>Variable Name</th>
									<th>Variable Value</th>
									<th><span style="white-space: nowrap"><?php echo translate('delete_question') ?></span></th>
								</tr>
							</thead>
							<tfoot>
								<tr>
									<td colspan="3"><a href="" onclick="<?php echo $asset->type() ?>.addGetRow('<?php echo $prefix ?>'); return false;" id="<?php echo $prefix ?>_additional_get_new_row"><?php echo translate('cms_ct_nest_content_additional_get_add'); ?></a></td>
								</tr>
								<tr>
									<td colspan="3">You can use keywords of the form <strong>%nested_*%</strong> (e.g %nested_asset_name%) to refer to any of the keywords of the asset being nested. <br/>You can also use global keywords (<strong>%globals_*%</strong>). These will be evaluated upon asset load.
									<br/>You can use keywords of the form <strong>%list_current_*%</strong> (e.g %list_current_asset_name%) to refer to the current asset being listed by listing page or search page.
									</td>
								</tr>
							</tfoot>
							<tbody id="<?php echo $prefix ?>_additional_get_none" style="display: <?php echo empty($additional_get_vars) ? 'table-row-group' : 'none' ?>">
								<tr>
									<td colspan="3"><em><?php echo translate('cms_ct_nest_content_additional_get_no_variables'); ?></em></td>
								</tr>
							</tbody>
							<tbody id="<?php echo $prefix ?>_additional_get_body">
							<?php
							foreach ($additional_get_vars as $key => $var_info) {
								?>
								<tr id="<?php echo $prefix; ?>_additional_get_row_<?php echo $key; ?>">
									<td><?php text_box($prefix.'_additional_get['.$key.'][name]', $var_info['name'], 20) ?></td>
									<td><?php text_box($prefix.'_additional_get['.$key.'][value]', $var_info['value'], 30) ?></td>
									<td><a href="" onclick="<?php echo $asset->type() ?>.removeGetRow('<?php echo $prefix; ?>','<?php echo $key; ?>'); return false;"><?php echo $delete_icon; ?></a></td>
								</tr>
								<?php
							}
							?>
							</tbody>
							</table>
						</td>
					</tr>
				</table>
				<?php
			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-cell-alt"><b><?php echo translate('cms_ct_nest_content_no_page_selected'); ?></b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', '', Array('asset' => 'D')); ?></td>
					</tr>
				</table>
				<?php
			}
		}//end if

		return TRUE;

	}//end paintBackend()


	/**
	* Prints a link to the asset if it is being viewed from an edit interface, otherwise
	* just calls printBody on the nested asset and lets it take care of things
	*
	* @param object		&$asset		the nest content content type object
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $editing=FALSE, $generating=FALSE)
	{
		// get the asset - we can't pass the type in because it could be anything
		// we also mute errors because we check later on
		$nested_asset = $this->_getNestedAsset($asset);
		if (is_null($nested_asset)) return FALSE;

		if ($generating) {
			$this->paintGenerated($asset, $nested_asset);
			return TRUE;
		}

		if ($editing) {
			// We are in the edit interface, so just print a link
			$asset_link = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($nested_asset->id);
			if (empty($asset_link)) {
				$asset_link = strip_url(current_url(FALSE, TRUE)).'/?a='.$nested_asset->id;
			}
			$asset_contents = '<strong>'.translate('cms_ct_nest_content_nested_content', get_asset_tag_line($nested_asset->id)).'</strong> <a href="'.$asset_link.'" id="sq_preview_new_window" target="_blank">'.translate('cms_ct_nest_content_click_to_preview').'</a>.';
		} else {
			$asset_contents = '';
			// This is the real deal, so we want to actually print the asset

			// Check for recursion
			// We use the cache key where available to distinguish differently-behaving
			// copies of the same asset (Bug Fix #1601)
			$asset_cache_key = $nested_asset->id;
			if (method_exists($nested_asset, '_getCacheKey')) {
				$asset_cache_key .= $nested_asset->_getCacheKey();
			}

			// Add GET vars to the cache key, as these will
			$additional_get_vars = $this->_processAdditionalGetVars($asset, $nested_asset);
			$asset_cache_key .= '-'.md5(serialize($additional_get_vars));

			if (isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key])) {
				unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);
				echo '<span class="sq-backend-warning"><b>[ ** '.translate('cms_ct_nest_content_recursion_warning').' ** ]<br/>'.translate('cms_ct_nest_content_recursion_body').'</b></span>';
				return TRUE;
			}

			// Now do the additional GET vars stuff
			$old_get = $_GET;
			$_GET = array_merge($_GET, $additional_get_vars);

			// Record that we have printed this asset
			// OKAY! so we havn't actually printed it yet, but we need to record that
			// we have or we might just fall endlessly into printBody functions and
			// never get back here again
			$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key] = 1;

			// Ask the asset to do its thing
			ob_start();
				switch ($asset->attr('paint_with')) {
					case 'raw':
						// Raw - just paint it
						$nested_asset->printAssetBody();
					break;

					case 'default':
						// Default Rules - use the "normal" method of determining
						// which paint layout gets used
						$nested_asset->printBodyWithPaintLayout();
					break;

					case 'arbitrary':
						// Get the arbitary paint layout; if there is none set,
						// paint it raw, otherwise paint it with the arbitrary layout
						$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
						if (empty($arbitrary_paint_layout)) {
							$nested_asset->printBody();
						} else {
							$nested_asset->printBodyWithPaintLayout($arbitrary_paint_layout->id);
						}//end else
					break;
				}//end switch

				$asset_contents = ob_get_contents();
			ob_end_clean();

			$keywords = extract_keywords($asset_contents);
			$replacements = Array();
			foreach ($keywords as $idx => $keyword) {
				if (strpos($keyword, 'nested_get_') === 0) {
					$sub_keyword = substr($keyword, 11);
					$replacements[$keyword] = array_get_index($_GET, $sub_keyword, '');
				}
			}

			replace_keywords($asset_contents, $replacements);

			$_GET = $old_get;

			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);
		}//end else

		echo $asset_contents;

		return TRUE;

	}//end paint()


	/**
	* Paints the content type as it should be cached
	*
	* @param object	&$asset			the table whose interface we are painting
	* @param object	&$nested_asset	the asset to nest
	*
	* @return void
	* @access public
	*/
	function paintGenerated(&$asset, &$nested_asset)
	{
		$additional_get_vars = $asset->attr('additional_get');
		$get_var_cache_key = md5(serialize($additional_get_vars));
		echo '<'.'?php'."\n";
		?>

		if (isset($_GET['<?php  echo $asset->attr('dynamic_nest_get');  ?>'])) {
			$nested_assetid = $_GET['<?php  echo $asset->attr('dynamic_nest_get');  ?>'];
			$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid,  '', TRUE);
			if ($nested_asset == null) {
				$nested_assetid = '<?php  echo $nested_asset->id;  ?>';
				$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid);
			}
		} else {
			$nested_assetid = '<?php  echo $nested_asset->id;  ?>';
			$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid);
		}
		$nesting_assetid = '<?php echo $asset->id; ?>';
		$temp_parent_array = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($nesting_assetid, '' , TRUE, FALSE);
        $parents_nesting_asset_id = $temp_parent_array[0];
        $parents_nesting_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parents_nesting_asset_id);

		$asset_cache_key = $nested_assetid.'-<?php echo $get_var_cache_key; ?>';
		if (method_exists($nested_asset, '_getCacheKey')) {
			$asset_cache_key .= $nested_asset->_getCacheKey();
		}

		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'])) $GLOBALS['SQ_NEST_CONTENT_PAINTED'] = Array();
		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key])) {

			$additional_get_vars = <?php
			// keyword delimiters need to be replaced temporarily
			echo str_replace('%', '&#37;', var_export($additional_get_vars, TRUE)); ?>;
			$processed_get_vars = Array();
			foreach ($additional_get_vars as $get_var) {
				$get_name = $get_var['name'];
				$get_value = str_replace('&#37;', '%', $get_var['value']);
				$keywords_list = extract_keywords($get_value);
				foreach ($keywords_list as $keyword) {
				    if(strpos($keyword, 'nested_') === 0) {
					$nested_asset_keyword = str_replace('nested_','',$keyword);
					$replacement = $nested_asset->getKeywordReplacement($nested_asset_keyword);
					if (!is_null($replacement)) {
						$get_value = str_replace('nested_','',$get_value);
						$get_value = str_replace('%'.$nested_asset_keyword.'%', $replacement, $get_value);
					}
				    }
				    else if (strpos($keyword, 'globals_') === 0) {
					replace_global_keywords($get_value);
				    }
				    else if (strpos($keyword, 'list_current_') === 0) {
					if(isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id']) && !empty($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'])) {
					    $asset_keyword = str_replace('list_current_','',$keyword);
					    $current_list_asset_id = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'];
					    $current_list_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_list_asset_id);
					    $replacement = $current_list_asset->getKeywordReplacement($asset_keyword);
					    if (!is_null($replacement)) {
						    $get_value = str_replace('list_current_','',$get_value);
						    $get_value = str_replace('%'.$asset_keyword.'%', $replacement, $get_value);
					    }
					}
				    }
				    else {
					// for all keywords replacement except globals and nested asset
					$replacement =  $parents_nesting_asset->getKeywordReplacement($keyword);
					if (!is_null($replacement)) {
						$get_value = str_replace('%'.$keyword.'%', $replacement, $get_value);
					}

				    }
				}

				preg_match('/([^\[]*)(.*)/', $get_name, $base_name);
				$array_bits = $base_name[2];
				if (empty($array_bits)) {
					$processed_get_vars[$base_name[1]] = $get_value;
				} else {
					$array_els = Array();

					if (!isset($processed_get_vars[$base_name[1]])) {
						$processed_get_vars[$base_name[1]] = Array();
					}

					$done = FALSE;
					while (!empty($array_bits)) {
						$found = preg_match('/\[([^\]]*)\](.*)/', $array_bits, $base_name);
						if (!$found) break;
						$array_bits = $base_name[2];
						$array_els[] = addslashes($base_name[1]);
					}

					$eval_str = '$processed_get_vars[\''.addslashes($base_name[1]).'\'][\''.implode('\'][\'', $array_els).'\'] = '.addslashes($get_value);
					eval($eval_str);
				}
			}
			$old_get[$asset_cache_key] = $_GET;
			$_GET = array_merge($_GET, $processed_get_vars);

			// $_REQUEST by default is supposed to contain the contnets of $_GET
			$old_request[$asset_cache_key] = $_REQUEST;
			$_REQUEST = array_merge($_REQUEST, $processed_get_vars);

			$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key] = 1;
			$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', TRUE);
			$asset_contents = '';
			if ($nested_asset->id && $nested_asset->readAccess()) {
				ob_start();
				<?php
				switch ($asset->attr('paint_with')) {
					case 'raw':
						?>
						$nested_asset->printAssetBody();
						<?php
					break;

					case 'default':
						?>
						$nested_asset->printBodyWithPaintLayout();
						<?php
					break;

					case 'arbitrary':
						$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
						if (empty($arbitrary_paint_layout)) {
							?>
							$nested_asset->printBody();
							<?php
						} else {
							?>
							$arbitrary_paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset(<?php echo $arbitrary_paint_layout->id ?>);
							$nested_asset->printBodyWithPaintLayout($arbitrary_paint_layout->id);
							<?php
						}
					break;
				}//end switch
				?>
				$asset_contents = ob_get_contents();
				ob_end_clean();

				$keywords = extract_keywords($asset_contents);
				$replacements = Array();
				foreach ($keywords as $idx => $keyword) {
					if (strpos($keyword, 'nested_get_') === 0) {
						$modifiers = NULL;
						$part_keyword = parse_keyword($keyword, $modifiers);
						$get_var = substr($part_keyword, 11);
						$value = array_get_index($_GET, $get_var, '');
						if ($part_keyword != $keyword) {
							apply_keyword_modifiers($value, $modifiers, Array('assetid' => $nested_asset->id));
						}

						$replacements[$keyword] = $value;
					}
				}
				replace_keywords($asset_contents, $replacements);
			}

			echo $asset_contents;
			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$asset_cache_key]);

			$_GET = $old_get[$asset_cache_key];
			$_REQUEST = $old_request[$asset_cache_key];
		} else {
			echo '<font color="red"><b>[ ** '.translate('cms_ct_nest_content_recursion_warning').' ** ]<br />'.translate('cms_ct_nest_content_recursion_body').'</b></font>';
		}

		<?php echo "\n".'?'.'>';

	}//end paintGenerated()


	/**
	* Process any Additional GET vars
	*
	* @param object	&$asset			the table whose interface we are painting
	* @param object	&$nested_asset	the asset to nest
	*
	* @return array
	* @access private
	*/
	function _processAdditionalGetVars(&$asset, &$nested_asset)
	{
		$processed_get_vars = Array();
		$additional_get_vars = $asset->attr('additional_get');
		if (!isset($additional_vars) || empty($additional_vars)) {
			return $processed_get_vars;
		}
		$temp_parent_array = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id, '' , TRUE, FALSE);
        $parents_nesting_asset_id = $temp_parent_array[0];
        $parents_nesting_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parents_nesting_asset_id);
		foreach ($additional_get_vars as $get_var) {
			$get_name = $get_var['name'];
			$get_value = $get_var['value'];
			$keywords_list = extract_keywords($get_value);
			foreach ($keywords_list as $keyword) {
			    if(strpos($keyword, 'nested_') === 0) {
				$nested_asset_keyword = str_replace('nested_','',$keyword);
				$replacement = $nested_asset->getKeywordReplacement($nested_asset_keyword);
				if (!is_null($replacement)) {
					$get_value = str_replace('nested_','',$get_value);
					$get_value = str_replace('%'.$nested_asset_keyword.'%', $replacement, $get_value);
				}
			    }
			    else if (strpos($keyword, 'globals_') === 0) {
				replace_global_keywords($get_value);
			    }
			    else if (strpos($keyword, 'list_current_') === 0) {
				if(isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id']) && !empty($_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'])) {
				    $asset_keyword = str_replace('list_current_','',$keyword);
				    $current_list_asset_id = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['list_current_asset_id'];
				    $current_list_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_list_asset_id);
				    $replacement = $current_list_asset->getKeywordReplacement($asset_keyword);
				    if (!is_null($replacement)) {
					    $get_value = str_replace('list_current_','',$get_value);
					    $get_value = str_replace('%'.$asset_keyword.'%', $replacement, $get_value);
				    }
				}
			    }
			    else {
				// for all keywords replacement except globals and nested asset
				$replacement =  $parents_nesting_asset->getKeywordReplacement($keyword);
				if (!is_null($replacement)) {
					$get_value = str_replace('%'.$keyword.'%', $replacement, $get_value);
				}

			    }
			}


			preg_match('/([^\[]*)(.*)/', $get_name, $base_name);
			$array_bits = $base_name[2];
			if (empty($array_bits)) {
				$processed_get_vars[$base_name[1]] = $get_value;
			} else {
				$array_els = Array();

				if (!isset($processed_get_vars[$base_name[1]])) {
					$processed_get_vars[$base_name[1]] = Array();
				}

				$done = FALSE;
				while (!empty($array_bits)) {
					$found = preg_match('/\[([^\]]*)\](.*)/', $array_bits, $base_name);
					if (!$found) break;
					$array_bits = $base_name[2];
					$array_els[] = addslashes($base_name[1]);
				}

				$eval_str = '$processed_get_vars[\''.addslashes($base_name[1]).'\'][\''.implode('\'][\'', $array_els).'\'] = '.addslashes($get_value);
				eval($eval_str);
			}//end else
		}//end foreach

		return $processed_get_vars;

	}//end _processAdditionalGetVars()


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the nest content content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		$ok = FALSE;

		$nested_asset = $this->_getNestedAsset($asset);
		$current_assetid = (!is_null($nested_asset)) ? $nested_asset->id : 0;

		$arbitrary_paint_layout = $this->_getArbitraryPaintLayout($asset);
		$current_apl_assetid = (!is_null($arbitrary_paint_layout)) ? $arbitrary_paint_layout->id : 0;

		$additional_get = array_get_index($_POST, $prefix.'_additional_get', Array());
		foreach ($additional_get as $var_index => $var_value) {
			$var_value['name'] = trim($var_value['name']);
			if (empty($var_value['name'])) {
				unset($additional_get[$var_index]);
			}
		}

		$dynamic_nest_get = (isset($_POST[$prefix.'_dynamic_nest_get'])) ? $_POST[$prefix.'_dynamic_nest_get'] : '';
		$assetid = (isset($_POST[$prefix.'_nested_assetid']['assetid'])) ? $_POST[$prefix.'_nested_assetid']['assetid'] : 0;
		$paint_with = array_get_index($_POST, $prefix.'_paint_with', 'raw');
		$paint_with_assetid = (isset($_POST[$prefix.'_paint_with_layout']['assetid'])) ? $_POST[$prefix.'_paint_with_layout']['assetid'] : 0;

		$to_update_links = FALSE;
		if ($paint_with == 'arbitrary') {
			if ($current_apl_assetid && $current_apl_assetid != $paint_with_assetid) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'paint_with_layout');
				if (!$asset->deleteLink($link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}//end if

				$to_update_links = TRUE;
			}//end if

			// create a NOTICE link for delete notification
			if ($paint_with_assetid) {
				// see if link exists first
				$link_exists = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $paint_with_assetid, SQ_LINK_NOTICE, 'paint_with_layout');
				if (empty($link_exists)) {
					$arbitrary_paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_with_assetid);
					if (!$asset->createLink($arbitrary_paint_layout, SQ_LINK_NOTICE, 'paint_with_layout')) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return FALSE;
					}//end if

					$to_update_links = TRUE;
				}//end if
			}//end if

			if ($to_update_links) $asset->linksUpdated();
		}//end if

		if (($dynamic_nest_get != $asset->attr('dynamic_nest_get')) || ($paint_with != $asset->attr('paint_with')) || ($additional_get != $asset->attr('additional_get')) || ($paint_with_assetid != $current_apl_assetid)) {
			$asset->setAttrValue('paint_with', $paint_with);
			$asset->setAttrValue('additional_get', $additional_get);
			$asset->setAttrValue('dynamic_nest_get', trim($dynamic_nest_get));

			if (!$asset->saveAttributes()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return FALSE;
			}

			$ok = TRUE;
		}

		if ($assetid) {

			if ($assetid != $current_assetid) {

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				if ($current_assetid) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
					if (!$asset->deleteLink($link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
				}

				// create a NOTICE link for delete notification
				$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$asset->createLink($nested_asset, SQ_LINK_NOTICE, 'nested_asset')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				// get the html contents of the nested asset so
				// we change the type of the cell later - other content
				// types can use the html

				// if we dont have the frontend asset set just buffer nothing
				if (SQ_IN_LIMBO && !isset($GLOBALS['SQ_SYSTEM']->frontend_asset)) {
					$html = '';
				} else {
					ob_start();
						$this->paint($asset, $prefix);
						$html = ob_get_contents();
					ob_end_clean();
				}

				$asset->setAttrValue('html', $html);

				// Bug Fix 1541
				if (!$current_assetid && $assetid) {
					// Normally you are not supposed to call linksUpdated() directly, however,
					// createAssetLink() does not call linksUpdated() to the major unless its dependant link.
					// let's manually update the paint layout bodycopy if there is any
					$asset->linksUpdated();
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$ok = TRUE;

				// Fix to Bug #5055	Nested content divs don't save properly across contexts
				// refresh the content_file.php for all the context here. This content_type(_nest_asset)
				// is different to any other content types. This is more so based on what asset is linked
				// with the link_value of 'nested_asset'. And since the Links are not contextable
				// we have to treat all the context w.r.t the changed asset
				$bodycopy_div_id = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($asset->id, 'bodycopy_container', FALSE));
				if (isset($bodycopy_div_id[0])) {
					$bodycopy_div = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_div_id[0]);
					$bodycopy_div_edit_fns = $bodycopy_div->getEditFns();
					$all_context = $GLOBALS['SQ_SYSTEM']->getAllContexts();
					foreach ($all_context as $context_id => $info) {
						$GLOBALS['SQ_SYSTEM']->changeContext($context_id);
							$bodycopy_div_edit_fns->generateContentFile($bodycopy_div);
						$GLOBALS['SQ_SYSTEM']->restoreContext();
					}
				}

			}//end if

		} else if ($current_assetid) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
			$ok = $asset->deleteLink($link['linkid']);
		}

		return $ok;

	}//end processBackend()


	/**
	* Get a reference to the asset the content type is nesting
	*
	* @param object	&$asset	the nest content content type object
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getNestedAsset(&$asset)
	{
		$nested_asset = NULL;

		if (isset($_GET[$asset->attr('dynamic_nest_get')])){
			$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_GET[$asset->attr('dynamic_nest_get')], '', TRUE);
		} else {
			$nested_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
			if (!empty($nested_link)) {
				$nested_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_link['minorid'], $nested_link['minor_type_code'], TRUE);
			}
		}

		return $nested_asset;

	}//end _getNestedAsset()


	/**
	* Get a reference to the asset the content type is nesting
	*
	* @param object	&$asset	the nest content content type object
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getArbitraryPaintLayout(&$asset)
	{
		$paint_layout = NULL;

		$paint_layout_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'paint_with_layout');
		if (!empty($paint_layout_link)) {
			$paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_layout_link['minorid'], $paint_layout_link['minor_type_code'], TRUE);
		}

		return $paint_layout;

	}//end _getArbitraryPaintLayout()


}//end class

?>
