<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: preview.inc,v 1.19 2006/02/07 02:18:51 skim Exp $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Preview Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.19 $
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

		// set the primary preview url to the first system root url
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$primary_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($owner->id,$root_urls[0]);

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
				function set_preview_url()
				{
					var previewURL = document.getElementById('preview_url').value;
					var queryStringStarted = false;
					if (document.getElementById('use_cache').checked == false) {
						previewURL += '<?php echo ($add_nocache) ? '/'.SQ_CONF_NOCACHE_SUFFIX : ''; ?>';
						queryStringStarted = true;
					}
					if (document.getElementById('preview_design').value != '') {
						if (queryStringStarted == false) {
							previewURL += '?';
						} else {
							previewURL += '&';
						}
						previewURL += 'SQ_DESIGN_NAME=' + document.getElementById('preview_design').value;
					}
					document.getElementById('sq_preview_frame').src = previewURL;
					document.getElementById('sq_preview_new_window').href = previewURL;

				}//end set_preview_url()

				//Change the new window url depending on cache options
				function set_new_window_href(elt)
				{
					var href = document.getElementById('sq_preview_new_window').href;
					var nocache_suffix = '<?php echo ($add_nocache) ? '/'.SQ_CONF_NOCACHE_SUFFIX : ''; ?>';

					if (elt.checked == true) {
						href = href.replace(nocache_suffix, '');
					} else {
						href = href + nocache_suffix;
					}
					document.getElementById('sq_preview_new_window').href = href;
				}

			</script>
			<?php
		$o->closeRaw();

		$o->openSection(translate('preview_options'));
			$o->openField(translate('url'));
				if (count($preview_urls) > 1) {
					combo_box('preview_url', $preview_urls, FALSE, $primary_url);
				} else {
					echo $primary_url;
					hidden_field('preview_url', $primary_url);
				}
			$o->closeField();
			$o->openField(translate('design'));
				if (count($preview_designs) > 1) {
					combo_box('preview_design', $preview_designs, FALSE, '');
				} else {
					echo ($has_default_design) ? 'Default Frontend Design' : 'None';
					hidden_field('preview_design', '');
				}
			$o->closeField();
			$o->openField(translate('use_cache'));
				check_box('use_cache', '1', FALSE, 'set_new_window_href(this);');
				echo translate('show_cached_asset');
			$o->closeField();
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
					<?php normal_button('preview_button', translate('preview'), 'set_preview_url()'); ?>
				</p>
				<?php
			$o->closeRaw();
		$o->closeSection();

		$o->openSection(translate('preview'));
			$o->openRaw('');
				ob_start();
					?><a href="<?php echo $primary_url; ?>" id="sq_preview_new_window" target="_blank"><?php echo translate('show_in_new_window'); ?></a><?php
					$replacements_link = ob_get_contents();
				ob_get_clean();
				?>
				<p class="sq-backend-field">
					<?php echo translate('currently_previewing', $owner->name, $replacements_link); ?>
				</p>
				<iframe id="sq_preview_frame" src="<?php echo $primary_url; ?><?php echo ($add_nocache) ? '/'.SQ_CONF_NOCACHE_SUFFIX : ''; ?>" style="width: 100%; height: 400px; border: 1px solid #C3C3C3;"></iframe>
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