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
* $Id: embed_movie.php,v 1.37 2013/09/15 23:53:23 lwright Exp $
*
*/

/**
* Embed Movie Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.37 $
* @package MySource_Matrix
*/


require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

if (!isset($_GET['f_fileid'])) $_GET['f_fileid'] = 0;
?><!DOCTYPE html>

<html>
	<head>
		<title>Embed Movie</title>

		<?php
		//add required js translation files, as we are using asset finder
		$include_list = Array(sq_web_path('lib').'/js/translation.js');

		$locales = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

		foreach ($locales as $locale) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$locale.'.js')) {
				$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$locale.'.js';
			}
		}

		$url_protocol_options = Array(
									''			=> '',
									'http://'	=> 'http://',
									'https://'	=> 'https://',
									'ftp://'	=> 'ftp://',
									'rtsp://'	=> 'rtsp://',
								);

		foreach ($include_list as $link) {
			?><script type="text/javascript" src="<?php echo $link; ?>"></script>
		<?php
		}
		?>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/javaExternalCall.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/tooltip.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/web/dfx/dfx.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/asset_map.js' ?>"></script>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/asset_map/js/js_asset_map.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

		<script type="text/javascript">

			function Init() {
				__dlg_init("matrixEmbedMovie");
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["f_width", "f_height"];
				var chk_fields = ["f_show_controls", "f_auto_start", "f_embed_loop"];
				var param = new Object();

				for (var i = 0; i < fields.length; i++) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}
				if (document.getElementById('url_link').value && document.getElementById('url_link').value.substring(0, 5) != './?a=') {
					if (!document.getElementById('url_protocol').value) {
						alert("Please specify the protocol for external url");
						return false;
					}
					param['use_external'] = true;
					param["external_url"] = document.getElementById('url_protocol').value + document.getElementById('url_link').value;
				} else {
					param['use_external'] = false;
					param["f_fileid"] = document.getElementById('assetid[assetid]').value;
				}
				for (var i = 0; i < chk_fields.length; i++) {
					var id = chk_fields[i];
					var el = document.getElementById(id);
					if (el.checked) {
						param[id] = "1";
					} else {
						param[id] = "0";
					}
				}
				__dlg_close("matrixEmbedMovie", param);

				return false;
			};

			function onCancel() {
				__dlg_close("matrixEmbedMovie", null);

				return false;
			};
		</script>


		<?php define('SQ_PAINTED_SIMPLE_ASSET_MAP', TRUE); ?>
	</head>

	<body onload="Init();" onUnload="asset_finder_onunload();">
		<form action="" method="get" name="main_form" id="main-form">
			<table class="sq-fieldsets-table">
				<tr>
				    <td valign="top" class="sq-popup-asset-map">
				        <div id="asset_map">
				            <iframe src="embed_movie_asset_map.php" name="sq_wysiwyg_popup_sidenav" frameborder="0" width="200" height="450" scrolling="no">
				            </iframe>
				        </div>
				    </td>
					<td valign="top">
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" colspan="2">
									<script type="text/javascript">
										function setUrl(protocol, link) {
											var f = document.main_form;

											if (protocol != null) highlight_combo_value(f.url_protocol, protocol);
											if (link     != null) {
												f.url_link.value = link;
											} else {
												var assetid = f.elements["assetid[assetid]"].value;

												if (assetid != '') {
													// shadow asset
													if (assetid.search(/:/) != -1) {
														f.url_link.value = './?a=' + assetid + '$';
													} else {
														f.url_link.value = './?a=' + assetid;
													}
													highlight_combo_value(f.url_protocol, '');
												}
											}
											setTimeout('self.focus()',100);
										};
									</script>
									<fieldset>
										<legend><b><?php echo translate('general'); ?></b></legend>
										<table width="100%" cellspacing="0" cellpadding="0">
											<tr>
												<td valign="top" width="100%">
													<table style="width:100%">
														<tr>
															<td class="label"><?php echo translate('protocol'); ?>:</td>
															<td><?php  combo_box('url_protocol',$url_protocol_options , false,$_REQUEST['f_fileprotocol'],0, ''); ?></td>
															<td class="label"><?php echo translate('link'); ?>:</td>
															<td><?php text_box('url_link', $_REQUEST['f_fileurl'], 40, 0)?></td>
														</tr>
														<tr>
															<td class="label"><?php echo translate('select_asset'); ?>:</td>
															<td colspan="3"><?php asset_finder('assetid', '', Array('file' => 'D'), 'sq_wysiwyg_popup_sidenav', false, 'setUrl'); ?></td>
														</tr>
													</table>
												</td>
											</tr>
										</table>

									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="50%">
									<fieldset>
										<legend><b><?php echo translate('controls'); ?></b></legend>
										<table style="width:100%">
											<tr>
												<td class="label" colspan="2"><b><?php echo translate('wmv-asf-asx_only'); ?></b></td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('auto_start'); ?>:</td>
												<td width="50%">
													<input type="checkbox" name="auto_start" id="f_auto_start" value="1" <?php echo ($_REQUEST['f_auto_start'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('loop'); ?>:</td>
												<td>
													<input type="checkbox" name="embed_loop" id="f_embed_loop" value="1" <?php echo ($_REQUEST['f_embed_loop'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<td class="label" colspan="2"><b><?php echo translate('mov-wmv-asf-asx_only'); ?></b></td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('show_controls'); ?>:</td>
												<td>
													<input type="checkbox" name="show_controls" id="f_show_controls" value="1" <?php echo ($_REQUEST['f_show_controls'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
								<td valign="top" width="50%">
									<fieldset>
										<legend><b><?php echo translate('size'); ?></b></legend>
										<table style="width:100%">
											<tr>
												<td class="label" width="50%"><?php echo translate('width'); ?>:</td>
												<td>
												<input type="text" name="width" id="f_width" size="5" title="Width" value="<?php echo empty($_REQUEST['f_width']) ? '100' : htmlspecialchars($_REQUEST['f_width']) ?>" />
												</td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('height'); ?>:</td>
												<td>
												<input type="text" name="height" id="f_height" size="5" title="Height" value="<?php echo empty($_REQUEST['f_height']) ? '100' : htmlspecialchars($_REQUEST['f_height']) ?>" />
												</td>
											</tr>									
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td>
										<button type="button" name="cancel" onclick="return onCancel();" class="sq-popup-btn-cancel"><?php echo translate('cancel'); ?></button>	
								</td>
								<td>										
									<div class="sq-popup-button-wrapper">																		    								
										<button type="button" name="ok" onclick="return onOK();" class="sq-btn-green"><?php echo translate('ok'); ?></button>	
									</div>								
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>


		</form>

		<!-- Search results -->
		<div id="new-message-popup" class="sq-new-message-popup-wrapper">
			<div id="new-message-popup-titlebar">
				<div id="new-message-popup-close"><a href="#" onclick="document.getElementById('new-message-popup').style.display = 'none'; return false;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/cancel.png"></a></div>
				<span id="new-message-popup-title">Searched for ''</span>
			</div>
			<div id="new-message-popup-details"></div>
		</div>
		<div id="search-wait-popup"><div id="search-wait-popup-titlebar"><div id="search-wait-popup-close"><a href="#" onclick="document.getElementById('search-wait-popup').style.display = 'none'; return false;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/cancel.png"></a></div><span id="search-wait-popup-title">Search in Progress</span></div>
			<div id="search-wait-popup-details">Your search is being processed, please wait...</div>
		</div> 


		<script type="text/javascript"><!--
			var current = 1;
			var results_per_page = <?php echo $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_PAGE_SIZE') !== FALSE ? $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_PAGE_SIZE') : 5; ?>;
			var total_results = 0;

			function jump_to_search_results(page) {
				// Show the correct page
				document.getElementById("search-result-page-" + current).style.display = 'none';
				document.getElementById("search-result-page-" + page).style.display = 'block';

				// Update page start and end markers
				document.getElementById("sq-search-results-page-start").innerHTML = (results_per_page * (page - 1)) + 1;
				document.getElementById("sq-search-results-page-end").innerHTML = Math.min(total_results, results_per_page * page);

				for (i = ((page - 1) * results_per_page) + 1; i <= Math.min(total_results, page * results_per_page); i++) {
					// collapse the new page when page is switched, so they're
					// back to just the tag lines
					document.getElementById("search-result-" + i + "-expand-link").innerHTML = '+';
					document.getElementById("search-result-" + i + "-detail").style.display = 'none';
				}

				current = page;
			}

			function set_asset_finder_from_search(assetid, label, url, linkid) {
				document.cookie = 'lastSelectedAssetId=' + escape(assetid);

				var prefix = 'assetid';
				dfx.getId(prefix + '[assetid]').value    = assetid;
				dfx.getId(prefix + '[url]').value        = url;
				dfx.getId(prefix + '[linkid]').value     = linkid;
				dfx.getId('sq_asset_finder_' + prefix + '_label').value   = label;
				dfx.getId('sq_asset_finder_' + prefix + '_assetid').value = assetid;
				
				document.getElementById("new-message-popup").style.display = 'none';
				setUrl('', './?a=' + assetid);
			}

		// --></script>
	</body>
</html>
