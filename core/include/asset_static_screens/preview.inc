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
* $Id: preview.inc,v 1.1 2005/02/04 04:18:52 gsherwood Exp $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Preview Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.1 $
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
	$start_sql = 'SELECT l.url, l.http, l.https, d.name FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_design d ON l.url = d.url';
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($owner->id));
	$url_data = $db->getAll($start_sql.' '.$where.' ORDER BY l.url');
	assert_valid_db_result($url_data);

	if (empty($url_data)) {
		// there is no direct URL for this assets, so
		// look at the dependant parents to try and find one
		$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($owner->id);
		foreach ($parents as $parentid) {
			$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parentid, '', true);
			if (is_null($parent)) continue;
			
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($parent->id));
			$parent_urls = $db->getAll($start_sql.' '.$where.' ORDER BY l.url');
			assert_valid_db_result($parent_urls);
			
			$url_data = array_merge($url_data, $parent_urls);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
		}
	}

	if (!empty($url_data)) {
		$primary_url = '';
		$preview_urls = Array();
		$preview_designs = Array('' => 'Default Frontend Design');
		$has_default_design = false;

		foreach ($url_data as $url_info) {
			$protocol = ($url_info['https']) ? 'https://' : 'http://';
			$preview_urls[$protocol.$url_info['url']] = $protocol.$url_info['url'];
			if (empty($primary_url)) $primary_url = $protocol.$url_info['url'];
			
			$design_name = $url_info['name'];
			if (substr($design_name, 0, 11) == 'user_design') {
				$design_name = substr($design_name, 13);
				$preview_designs[$design_name] = ucwords(str_replace('_', ' ', $design_name));
			} else if ($design_name == 'system_design::frontend') {
				$has_default_design = true;
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

				}//end set_preview_url()

				function open_preview_window()
				{
					var previewURL = document.getElementById('sq_preview_frame').src;
					var win = window.open(previewURL, 'sq_preview_window');
					win.focus();

				}//end open_preview_window()
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
				echo '<i>Press the Preview button after changing preview options.</i>';
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
					Currently previewing "<?php echo $owner->name; ?>" [<a href="#" onClick="open_preview_window(); return false;">show in new window</a>]
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