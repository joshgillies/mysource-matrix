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
* $Id: preview.inc,v 1.7 2005/04/06 23:05:34 mmcintyre Exp $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Preview Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/


/**
* Paints the interface for previewing the asset
*
* @param object	Asset					&$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		&$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintPreview(&$owner, &$o, &$ei)
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
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
			$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parentid, '', true);
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
		$primary_url = '';
		$preview_urls = Array();
		$preview_designs = Array('' => 'Default Frontend Design');
		$has_default_design = false;

		$contains_diff_domain = false;
		$current_url = parse_url(current_url());

		foreach ($url_data as $url_info) {
			$protocol = ($url_info['https']) ? 'https://' : 'http://';
			$preview_urls[$protocol.$url_info['url']] = $protocol.$url_info['url'];
			if (empty($primary_url)) $primary_url = $protocol.$url_info['url'];

			$url_pieces = parse_url($protocol.$url_info['url']);
			if ($url_pieces['host'] != $current_url['host']) $contains_diff_domain = true;

			if (!empty($url_info['name'])) {
				$design_name = $url_info['name'];
				if (!preg_match('/^design::(system|user)::(.*)$/', $design_name, $matches)) continue;
				if ($matches[1] == 'user') {
					$preview_designs[$matches[2]] = ucwords(str_replace('_', ' ', $matches[2]));
				} else if ($design_name == 'design::system::frontend') {
					$has_default_design = true;
				}
			}
		}
		if (!$has_default_design) $preview_designs[''] = 'None';

		$o->openRaw();
			?>
			<script type="text/javascript">
				function set_preview_url()
				{
					var previewURL = document.getElementById('preview_url').value;
					var queryStringStarted = false;
					if (document.getElementById('use_cache').checked == false) {
						previewURL += '?no_cache=1';
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

			</script>
			<?php
		$o->closeRaw();

		$o->openSection('Preview Options');
			$o->openField('URL');
				if (count($preview_urls) > 1) {
					combo_box('preview_url', $preview_urls, false, $primary_url);
				} else {
					echo $primary_url;
					hidden_field('preview_url', $primary_url);
				}
			$o->closeField();
			$o->openField('Design');
				if (count($preview_designs) > 1) {
					combo_box('preview_design', $preview_designs, false, '');
				} else {
					echo ($has_default_design) ? 'Default Frontend Design' : 'None';
					hidden_field('preview_design', '');
				}
			$o->closeField();
			$o->openField('Use Cache');
				check_box('use_cache');
				echo '&nbsp;show the cached version of this asset';
			$o->closeField();
			$o->openRaw();
				?>
				<i>Press the Preview button after changing preview options.</i>
				<?php
				if ($contains_diff_domain && !$owner->effectiveUnrestricted()) {
					?>
					<div class="sq-backend-warning">
						<br>
						NOTE:<br>
						You may need be presented with a Login screen if you choose a URL from a domain different to <b><?php echo $current_url['host']; ?></b>.<br>
						Just enter your Username and Password to login as normal.
					</div>
					<?php
				}
			$o->closeRaw();
			$o->openRaw();
				?>
				<p style="text-align: right;">
					<?php normal_button('preview_button', 'Preview', "set_preview_url()"); ?>
				</p>
				<?php
			$o->closeRaw();
		$o->closeSection();

		$o->openSection('Preview');
			$o->openRaw('');
				?>
				<p class="sq-backend-field">
					Currently previewing "<?php echo $owner->name; ?>" [<a href="<?php echo $primary_url; ?>" id="sq_preview_new_window" target="_blank">show in new window</a>]
				</p>
				<iframe id="sq_preview_frame" src="<?php echo $primary_url; ?>?no_cache=1" style="width: 100%; height: 400px; border: 1px solid #C3C3C3;"></iframe>
				<?php
			$o->closeRaw();
		$o->closeSection();

	} else {

		// there is no preview URL available for this asset
		$o->openSection('No preview available');
			$o->openField('');
				echo 'This asset cannot be previewed or does not have a URL assigned to it.';
			$o->closeField();
		$o->closeSection();

	}//end if we have URLs

	return false;

}//end paintPreview()


/**
* Processes the interface for previewing the asset
*
* @param object	Asset					&$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		&$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processPreview(&$owner, &$o, &$ei)
{
	return false;

}//end processPreview()


?>