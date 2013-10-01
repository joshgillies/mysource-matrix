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
* $Id: preview.inc,v 1.40.4.4 2013/10/01 01:34:52 cupreti Exp $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Preview Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.40.4.4 $
* @package MySource_Matrix
*/


/**
* Paints the interface for previewing the asset
*
* @param object	&$owner	Asset:the asset whose interface we are painting
* @param object	&$o		Backend_Outputter: the outputter class
* @param object	&$ei	Asset_Edit_Interface: the edit interface
* @param array	$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintPreview(&$owner, &$o, &$ei, $keywords=NULL)
{
	// turning off error reporting for get size function
	// `since a wrong url will display all unwanted error messgae...
	error_reporting(0);
	$q = $_GET['calc'];
	if (strlen($q) > 0) {
		printSize($owner);
		exit;
	}
	error_reporting(6143); // restore the error reporting
	$db = MatrixDAL::getDb();
	$sql = 'SELECT l.url, l.http, l.https, lv.name
			FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
			LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv
			ON ((l.url = lv.url)
			OR (l.url || \'/\' = lv.url)
			OR ((l.url LIKE lv.url || \'/%\')
			AND (l.url NOT LIKE lv.url || \'%/$\')))';

	$where = 'l.assetid = :ownerid
			AND lv.name LIKE :pattern';
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');

	$full_sql = $sql.' '.$where.' ORDER BY l.url';
	$query = MatrixDAL::preparePdoQuery($full_sql);

	try {
		MatrixDAL::bindValueToPdo($query, 'ownerid', $owner->id);
		MatrixDAL::bindValueToPdo($query, 'pattern', 'design::%');
		$url_data = MatrixDAL::executePdoAll($query);
	} catch (Exception $e) {
		throw new Exception('Unable to get the design name and url for assetid #'.$owner->id.' due to the following database error:'.$e->getMessage());
	}//end try catch

	$dependant_parent_id = '';
	if (empty($url_data)) {
		// there is no direct URL for this asset
		// so look at the dependant parents to try and find one
		$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($owner->id);
		foreach ($parents as $parentid) {
			$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid, '', TRUE);
			if (is_null($parent)) continue;

			// Recycle query from before
			try {
				MatrixDAL::bindValueToPdo($query, 'ownerid', $parent->id);
				MatrixDAL::bindValueToPdo($query, 'pattern', 'design::%');
				$parent_urls = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get the design name and url for assetid #'.$owner->id.' due to the following database error:'.$e->getMessage());
			}//end try catch

			// Pick the first dependant parent that has URL
			if (empty($dependant_parent_id) && !empty($parent_urls)) {
				$dependant_parent_id = $parent->id;
			}

			$url_data = array_merge($url_data, $parent_urls);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
		}
	}

	if (empty($url_data)) {
		// there is no URL with lookup values for this asset
		// so just do a straight call for lookups
		$url_data = $owner->getLookups();
	}


	if (!empty($url_data)) {
		$preview_urls = Array();
		$preview_designs = Array('system::frontend' => 'Default Frontend Design');
		$has_default_design = FALSE;

		$contains_diff_domain = FALSE;
		$current_url = parse_url(current_url());

		// set the primary preview url to the current root url
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$current_root_url = $root_urls[0];
		foreach ($root_urls as $root_url) {
			if (preg_match('/'.$current_url['host'].'/i', $root_url)) {
				$current_root_url = $root_url;
				break;
			}
		}

		// If we got the URL data from the dependant parents
		if (!empty($dependant_parent_id)) {
			$primary_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($dependant_parent_id, $current_root_url);
		} else {
			$primary_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($owner->id, $current_root_url);
		}

		// we can't preview primary url from other domain in frame
		$open_in_new_window = FALSE;
		if(SQ_CONF_ENFORCE_FRAME_SAMEORIGIN) {
		    $primary_url_data = parse_url($primary_url);
		    $primary_url_no_protocol = array_get_index($primary_url_data, 'host',  '').array_get_index($primary_url_data, 'path', '');
		     if(strpos($primary_url_no_protocol, $current_root_url) !== 0) {
			$open_in_new_window = TRUE; 
		     }
		}
		foreach ($url_data as $url_info) {
			$protocol_list = Array();


			
			if ($url_info['https']) {
				$protocol_list['https'] = 'https://';
			}
			if ($url_info['http']) {
				$protocol_list['http'] = 'http://';
			}

			foreach ($protocol_list as $protocol) {
				$preview_urls[$protocol.$url_info['url']] = $protocol.$url_info['url'];
				// we can't preview pages in other root domain in frame
				if(SQ_CONF_ENFORCE_FRAME_SAMEORIGIN) {
				    if(strpos($url_info['url'], $current_root_url) !== 0) {
					$preview_urls[$protocol.$url_info['url']] .= ' (open in new window)';
				    }
				}

				if (empty($primary_url)) {
					$primary_url = $protocol.$url_info['url'];
				}

				$url_pieces = parse_url($protocol.$url_info['url']);
				if ($url_pieces['host'] != $current_url['host']) {
					$contains_diff_domain = TRUE;
				}

				if (!empty($url_info['name'])) {
					$design_name = $url_info['name'];
					if (!preg_match('/^design::(system|user)::(.*)$/', $design_name, $matches)) {
						continue;
					}
					if ($matches[1] == 'user') {
						$preview_designs[$matches[2]] = ucwords(str_replace('_', ' ', $matches[2]));
					} else if ($design_name == 'design::system::frontend') {
						$has_default_design = TRUE;
					}
				}
			}//end foreach protocol
		}//end foreach url

		if (!$has_default_design) {
			$preview_designs[''] = 'None';
		}

		$add_nocache = TRUE;
		$root_url_array = $GLOBALS['SQ_SYSTEM']->am->getRootURL(strip_url($owner->getURL(), TRUE));
		$root_url = trim($root_url_array['url']);
		
		if (strpos($primary_url, '/__data/') !== FALSE || (SQ_CONF_STATIC_ROOT_URL && ($root_url == SQ_CONF_STATIC_ROOT_URL || strpos($root_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE || strpos($primary_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE))) {
			$add_nocache = FALSE;
		}
		//Bug #4708 openRaw and closeRaw incompatible in LIMBO mode with buffering OFF as they behave as sections (creating tables).
		if (!((!$o->_buffering) && SQ_IN_LIMBO)) $o->openRaw();
		
			?>
			<script type="text/javascript">

				var preview_url = '<?php echo str_replace("'", "\'", $primary_url); ?>';
				var xmlHttp = null;
				var url_calculate;
				var open_new_window = <?php echo $open_in_new_window ? 'true' : 'false';  ?>

				if(open_new_window) {
				    window.open(preview_url);
				}
				
				function update_preview_url()
				{
					var url = document.getElementById('preview_url').value;
					var url_key = document.getElementById('preview_url').options[document.getElementById('preview_url').selectedIndex].text;
					
					if(/open\sin\snew\swindow/.test(url_key) ) {
					    open_new_window = true;
					}
					else {
					    open_new_window = false;
					}
					var query_args = [];

					var uc = document.getElementById('use_cache');
					if (uc != null) {
						if (!uc.checked) {
							url += '<?php echo ($add_nocache) ? '/'.SQ_CONF_NOCACHE_SUFFIX : ''; ?>';
						}
					}

					var sd = document.getElementById('show_diff');
					if (sd != null) {
						if (sd.checked) {
							query_args.push('SQ_ACTION=diff');
						}
					}

					var pd = document.getElementById('preview_design');
					if ((pd != null) && (pd.value != '')) {
						query_args.push('SQ_DESIGN_NAME='+pd.value);
					}

					var pc = document.getElementById('preview_context');
					if ((pc != null) && (pc.value != '-sep-') && (pc.value != '-default-')) {
						query_args.push('SQ_CONTEXT_NAME='+pc.value);
					}

					if (query_args.length != 0) {
						var query_str = '';
						for (var i=0; i < query_args.length; i++) {
							query_str += '&'+query_args[i];
						}
						url += '?'+query_str.substr(1);
					}

					document.getElementById('sq_preview_new_window').href = url;
					preview_url = url;
				}

				function do_preview()
				{
					if(open_new_window) {
					    window.open(preview_url);
					}
					else {
					    document.getElementById('sq_preview_frame').src = preview_url;
					}
				}

				function get_size() {
					try
					{
						xmlHttp=new XMLHttpRequest();
					}
					catch(e)
					{
						try
						{
							xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
						}
						catch(e)
						{
							alert ("Your browser does not support XMLHTTP!");
							return;
						}
					}
					document.getElementById("size_button").value = "Calculating ....";
					document.getElementById("size_button").disabled = true;
					if (preview_url.match(/SQ_DESIGN_NAME=/)) {
						var design_name = preview_url.split("?SQ_DESIGN_NAME=");
						if(design_name[0].match(/_nocache/)){
							url_calculate = design_name[0].split("_nocache");
						}
						url_calculate=url_calculate+"/_admin/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=<?php echo $owner->id ?>&asset_ei_screen=preview";
					url_calculate = url_calculate+"&calc=calculate";
					url_calculate = url_calculate+"&sid="+Math.random();
					} else {
						if(preview_url.match(/_nocache/)){
							url_calculate = preview_url.split("_nocache");
						}
						url_calculate=preview_url+"/_admin/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=<?php echo $owner->id ?>&asset_ei_screen=preview";
						url_calculate = url_calculate+"&calc=calculate";
						url_calculate = url_calculate+"&sid="+Math.random();
					}


					if (preview_url.match(/SQ_DESIGN_NAME=/)) {
						url_calculate = url_calculate+"&SQ_DESIGN_NAME="+design_name[1];
					}
					xmlHttp.onreadystatechange = stateChanged;
					xmlHttp.open("GET",url_calculate,true);
					xmlHttp.send(null);
				}

				function stateChanged()
				{
					if (xmlHttp.readyState==4)
					{
						document.getElementById("size_elements").innerHTML=xmlHttp.responseText;
						document.getElementById("size_title").innerHTML = "Size";
						document.getElementById("size_button").value = "Calculate Size";
						document.getElementById("size_button").disabled = false;
					}
				}

			</script>
		<?php
		//Bug #4708 openRaw and closeRaw incompatible in LIMBO mode with buffering OFF as they behave as sections (creating tables).
		if (!((!$o->_buffering) && SQ_IN_LIMBO)) $o->closeRaw();

		$o->openSection(translate('preview_options'));
			$o->openField(translate('url'));
				if (count($preview_urls) > 1) {
					combo_box('preview_url', $preview_urls, FALSE, $primary_url, 1, 'onchange="update_preview_url()"');
				} else {
					echo $primary_url;
					hidden_field('preview_url', $primary_url);
				}
			$o->closeField();

			$o->openField(translate('design'));
				if (count($preview_designs) > 1) {
					combo_box('preview_design', $preview_designs, FALSE, '', 1, 'onchange="update_preview_url()"');
				} else {
					echo ($has_default_design) ? 'Default Frontend Design' : 'None';
				}
			$o->closeField();

			$o->openField(translate('context'));
				$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				$current_context = $GLOBALS['SQ_SYSTEM']->getContextId();
				if (count($contexts) > 1) {
					$context_list = Array(
										'-default-' => translate('use_normal_context_for_this_url'),
										'-sep-'		=> '----------------------',
									);
					foreach ($contexts as $context_item_id => $context_item) {
						if ((int)$current_context === (int)$context_item_id) {
							$context_name = $context_item['name'];
						}

						$context_list[rawurlencode($context_item['name'])] = $context_item['name'];
					}
					combo_box('preview_context', $context_list, FALSE, Array('-default-'), 1, 'onchange="update_preview_url()"');
				}
			$o->closeField();


			$o->openField(translate('use_cache'));
				check_box('use_cache', '1', FALSE, 'if (this.checked) { document.getElementById(\'show_diff\').checked = false; } update_preview_url()');
				label(translate('show_cached_asset'), 'use_cache');
			$o->closeField();

			if ($owner->status & SQ_SC_STATUS_SAFE_EDITING) {
				$o->openField(translate('show_diff'));
					check_box('show_diff', '1', FALSE, 'if (this.checked) { document.getElementById(\'use_cache\').checked = false; } update_preview_url()');
					label(translate('show_safe_edit_difference'), 'show_diff');
				$o->closeField();
			}
			$o->openField('');
				if($contains_diff_domain) {
				    if(SQ_CONF_ENFORCE_FRAME_SAMEORIGIN) {
					echo translate('different_domain_preview_new_window');
				    }
				    else if (!$owner->effectiveUnrestricted()) {
					echo translate('different_domain_login', $current_url['host']);
				    }
				}
			$o->closeField();
		
			$o->openField('<span id="size_title"></span>');
			?>
				<span id="size_elements"></span>
				<p style="text-align: right;">
					<?php normal_button('preview_button', translate('preview'), 'do_preview()'); echo ' ';
					normal_button('size_button', translate('calculate'), 'get_size()');?>
				</p>
				<?php
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('preview'));
			$o->openRaw('');
				if ($add_nocache) {
					$primary_url .= '/'.SQ_CONF_NOCACHE_SUFFIX;
				}
				ob_start();
					?><a href="<?php echo $primary_url; ?>" id="sq_preview_new_window" target="_blank"><?php echo translate('show_in_new_window'); ?></a><?php
					$replacements_link = ob_get_contents();
				ob_end_clean();
				?>
				<p class="sq-backend-field">
					<?php echo translate('currently_previewing', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $replacements_link); ?>
				</p>
				<iframe name="sq_preview_frame" id="sq_preview_frame" src="<?php echo $primary_url; ?>" style="width: 100%; height: 400px; border: 1px solid #C3C3C3;"></iframe>
				<?php
			$o->closeRaw();
		$o->closeSection();

	} else {

		// there is no preview URL available for this asset
		$o->openSection(translate('no_preview_available'));
			$o->openField('');
				echo translate('asset_cannot_preview');
			$o->closeField();
		$o->closeSection();

	}//end if we have URLs

	return FALSE;

}//end paintPreview()


/**
* Processes the interface for previewing the asset
*
* @param object	&$owner	Asset: the asset whose interface we are painting
* @param object	&$o		Backend_Outputter: the outputter class
* @param object	&$ei	Asset_Edit_Interface: the edit interface
*
* @return boolean
* @access public
*/
function processPreview(&$owner, &$o, &$ei)
{
	return FALSE;

}//end processPreview()


/**
* Processes the size of the Asset we are previewing
*
* @param object	&$owner	Asset: the asset whose interface we are painting
*
* @return void
* @access public
*/
function printSize(&$owner)
{
	$asset_here = $GLOBALS['SQ_SYSTEM']->am->getAsset($owner->id,'',TRUE);
	// If its a file asset just get the size and display it
	if ($owner instanceof File) {
		$existing = $asset_here->getExistingFile();
		echo '<b>'.easy_filesize($existing['size']).'</b>';
		return;
	}
	ob_start();
		$asset_here->printFrontend();
		$file_content = ob_get_contents();
	ob_end_clean();
	$file_size = mb_strlen($file_content);
	ob_start();
		$asset_here->printBody();
		$body_content = ob_get_contents();
	ob_end_clean();
	$body_size = mb_strlen($body_content);
	$array_img = Array();
	$array_img_2 = Array();
	$array_css = Array();
	$array_js = Array();
	$total_img_size = 0;
	$total_js_size = 0;
	$total_css_size = 0;
	$total_other_size = 0;
	preg_match_all('/(url\(\"?([^\")]+))/ie', $file_content, $array_img);
	preg_match_all('/\<(img|input|embed).*\>/iU', $file_content, $matches);
	preg_match_all('/(?:src|background)[\s]?=[\s]?"?([^"]+)"?/i', implode('', $matches[0]), $array_img_embd);

	preg_match_all('/<script[^>]+.*[\s]*(src|background)[\040]*=[\040]*\"?([^\"\' >]+)/ie', $file_content, $array_js);
	preg_match_all('/<link[^>]+href[\040]*=[\040]*[\"|\'|\\\\]*([^\'|\"|>|\040]*(.*)\.css)[\"|\'|>|\040|\\\\]* /ie',$file_content, $array_css);
	$array_img = $array_img[2];
	$array_img = array_unique(array_merge($array_img,$array_img_embd[1]));
	$array_js  = array_unique($array_js[2]);
	$array_css = array_unique($array_css[1]);
	$array_types = Array(
					'img'	=> Array(
								0	=> $system_url_img = NULL,
								1	=> $array_img,
							   ),
					'css'	=> Array(
								0	=> $system_url_css = NULL,
								1	=> $array_css,
							   ),
					'js'	=> Array(
								0	=> $system_url_js = NULL,
								1	=> $array_js,
							   ),
				   );

	foreach ($array_types as $key => $type_asset) {
		foreach ($type_asset[1] as $key_value => $url) {
			$url = str_replace("'",'',$url);
			$basename = explode('/',$url);
			$protocol = explode(':',$url);
			$db = MatrixDAL::getDb();
			$sql = 'SELECT
							assetid
					FROM
							sq_ast_path
					WHERE
							path = :url';
			try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'url', $basename[sizeof($basename)-1]);
					$result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
					throw new Exception('Unable to get URLs for asset '.$path.' due to database error: '.$e->getMessage());
			}
			if (!($result)) {
				$type_asset[0] = $type_asset[0].$url.'~';
			} else {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($result[0][0]);
				if ($asset && $key == 'img') {
					$total_img_size += $asset->attr('size');
				} else if ($asset && $key == 'css') {
					$total_css_size += filesize($asset->data_path.'/'.$asset->name);
				} else if ($asset && $key == 'js') {
					$total_js_size += filesize($asset->data_path.'/'.$asset->name);
				} else if ($asset) {
					$total_other_size += filesize($asset->data_path.'/'.$asset->name);
				}
			}
		}//end foreach
		// get default times so that we can restore them later
		$default_execution_time = ini_get('max_execution_time');
		$default_socket_time = ini_get('default_socket_timeout');
		$type_asset[0] = explode('~',$type_asset[0]);
		if (sizeof($type_asset[0]) != 1) {
			foreach ($type_asset[0] as $key => $url) {
				$url = str_replace("'",'',$url);
				if (strpos($url, '__data') !== FALSE && $url != '') {
					$parts = explode('__data',$url);
					$url = SQ_DATA_PATH.'/public'.$parts[1];
					$total_js_size_here = filesize($url);
					$total_js_size += $total_js_size_here;
				} else {
					if (strpos($url, '__lib') !== FALSE && $url != '') {
						$parts = explode('__lib',$url);
						$url = SQ_INCLUDE_PATH.'/../lib'.$parts[1];
						$total_js_size_here = filesize($url);
						$total_js_size += $total_js_size_here;
					} else if ($url != '') {
						// set execution time to 5 second so that script doesnt take too long to fetch the headers
						set_time_limit(5);
						ini_set('default_socket_timeout', 5);
						$headers = get_headers($url,1);
						if ($key == 'img') {
							$total_img_size += $headers['Content-Length'];
						} else if ($key == 'css') {
							$total_css_size += $headers['Content-Length'];
						} else if ($key == 'js') {
							$total_js_size += $headers['Content-Length'];
						} else {
							$total_other_size += $headers['Content-Length'];
						}
						if (strpos($headers[0], '404') !== FALSE || !($headers)) {
							$element_unknown = $element_unknown.$url.'~';
						}
						set_time_limit($default_execution_time);
						ini_set('default_socket_timeout', $default_socket_time);
					}
				}
			}//end foreach
		}//end if
	}//end foreach
	$output = '<table width=40% cellspacing =\'0\'>
				<tr><td></td><td>'.sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL('page_standard'),15,15,'',NULL,'').'</td><td>HTML content</td><td align=right>'.number_format(($file_size/1024),2).'</td><td>KB</td></tr>
				<tr><td></td><td>'.sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL('bodycopy'),15,15,'',NULL,'').'</td><td>Body content</td><td align=right>'.number_format(($body_size/1024),2).'</td><td>KB</td></tr>
				<tr><td></td><td>'.sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL('design_css'),15,15,'',NULL,'').'</td><td>CSS ('.sizeof($array_css).')</td><td align=right>'.number_format((($total_css_size)/1024),2).'</td><td>KB</td></tr>
				<tr><td></td><td>'.sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL('text_file'),15,15,'',NULL,'').'</td><td>Javascript ('.sizeof($array_js).')</td><td align=right>'.number_format((($total_js_size)/1024),2).'</td><td>KB</td></tr>
				<tr><td></td><td>'.sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL('image'),15,15,'',NULL,'').'</td><td>Media ('.sizeof($array_img).')</td><td align=right>'.number_format(($total_img_size/1024),2).'</td><td>KB</td></tr>';

	if ($total_other_size > 0) {
		$output .= '<tr><td></td><td>'.sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL('file'),15,15,'',NULL,'').'</td><td>Other Elements</td><td align=right>'.number_format(($total_other_size/1024),2).'</td><td>KB</td></tr>
		<tr><td></td><td>'.sq_get_icon(sq_web_path('lib').'/web/images/blank.gif',15,15,'',NULL,'').'</td><td style=\'border-top: thin solid#000000\'><b>Total<b></td><td align=right style=\'border-top: thin solid#000000\'><b>'.number_format((($file_size+$total_img_size+$total_js_size+$total_css_size)/1024),2).'</b></td><td style=\'border-top: thin solid#000000\'><b>KB</b></td></tr></table>';
	} else {
		$output .= '<tr><td></td><td>'.sq_get_icon(sq_web_path('lib').'/web/images/blank.gif',15,15,'',NULL,'').'</td><td style=\'border-top: thin solid#000000\'><b>Total</b></td><td align=right style=\'border-top: thin solid#000000\'><b>'.number_format((($file_size+$total_img_size+$total_js_size+$total_css_size)/1024),2).'</b></td><td style=\'border-top: thin solid#000000\'><b>KB</b></td></tr></table>';
	}

	echo $output;

	$element_unknown = explode('~',$element_unknown);
	if (sizeof($element_unknown)!= 1) {
		echo '<br><span class="sq-backend-warning ">Could not get data for</span>';
		foreach ($element_unknown as $key => $unknown_url) {
			if ($unknown_url != '') {
				echo '<br><span class="sq-backend-warning ">'.$unknown_url.'</span>';
			}
		}
	}

}//end printSize()


?>
