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
* $Id: preview.inc,v 1.23 2006/12/05 05:36:15 bcaldwell Exp $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Preview Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix
*/


/**
* Paints the interface for previewing the asset
*
* @param object	&$owner	Asset:the asset whose interface we are painting
* @param object	&$o		Backend_Outputter: the outputter class
* @param object	&$ei	Asset_Edit_Interface: the edit interface
*
* @return boolean
* @access public
*/
function paintPreview(&$owner, &$o, &$ei)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$sql = 'SELECT l.url, l.http, l.https, lv.name
			FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
					LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url';

	$where = 'assetid = '.$db->quote($owner->id).'
			  AND name LIKE '.$db->quote('design::%');
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');

	$url_data = $db->getAll($sql.' '.$where.' ORDER BY l.url');
	assert_valid_db_result($url_data);

	if (empty($url_data)) {
		// there is no direct URL for this assets, so
		// look at the dependant parents to try and find one
		$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($owner->id);
		foreach ($parents as $parentid) {
			$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid, '', TRUE);
			if (is_null($parent)) continue;

			$where = 'assetid = '.$db->quote($parent->id).'
					  AND name LIKE '.$db->quote('design::%');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$parent_urls = $db->getAll($sql.' '.$where.' ORDER BY l.url');
			assert_valid_db_result($parent_urls);

			$url_data = array_merge($url_data, $parent_urls);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
		}
	}

	if (empty($url_data)) {
		// there is no URL's with lookup values for this asset
		// so just do a straight call for lookups
		$url_data = $owner->getLookups();
	}


	if (!empty($url_data)) {
		$preview_urls = Array();
		$preview_designs = Array('' => 'Default Frontend Design');
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

		$primary_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($owner->id, $current_root_url);

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
		if (strpos($primary_url, '/__data/') !== FALSE) {
			$add_nocache = FALSE;
		}

		$o->openRaw();
			?>
			<script type="text/javascript">

				var preview_url = '<?php echo str_replace("'", "\'", $primary_url); ?>';

				function update_preview_url()
				{
					var url = document.getElementById('preview_url').value;
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
					document.getElementById('sq_preview_frame').src = preview_url;
				}

			</script>
			<?php
		$o->closeRaw();

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
			$o->openField(translate('use_cache'));
				check_box('use_cache', '1', FALSE, 'if (this.checked) { document.getElementById(\'show_diff\').checked = false; } update_preview_url()');
				label(translate('show_cached_asset'), 'use_cache');
			$o->closeField();
			if ($owner->status & SQ_SC_STATUS_SAFE_EDITING) {
				$o->openField(translate('show_diff'));
					check_box('show_diff', '1', FALSE, 'if (this.checked) 				{ document.getElementById(\'use_cache\').checked = false; } update_preview_url()');
					label(translate('show_safe_edit_difference'), 'show_diff');
				$o->closeField();
			}
			$o->openRaw();
				?>
				<i><?php echo translate('preview_change_options'); ?></i>
				<?php
				if ($contains_diff_domain && !$owner->effectiveUnrestricted()) {
					echo translate('different_domain_login', $current_url['host']);
				}
			$o->closeRaw();
			$o->openRaw();
				?>
				<p style="text-align: right;">
					<?php normal_button('preview_button', translate('preview'), 'do_preview()'); ?>
				</p>
				<?php
			$o->closeRaw();
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
					<?php echo translate('currently_previewing', $owner->name, $replacements_link); ?>
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


?>