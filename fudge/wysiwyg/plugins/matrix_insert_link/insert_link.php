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
* $Id: insert_link.php,v 1.43 2008/11/28 02:45:53 bpearson Exp $
*
*/

/**
* Insert Link Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.43 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

// URL protocol options
$pref = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_LINK_TYPES');

$url_protocol_options = Array('' => '');
$url_protocol_combo_box = Array('' => '');
foreach ($pref as $pref_el) {
	$url_protocol_options[$pref_el['type']]   = $pref_el['template'];
	$url_protocol_combo_box[$pref_el['type']] = $pref_el['type'];
}

$new_window_bool_options = Array(
							'toolbar'		=> 'Show Tool Bar',
							'menubar'		=> 'Show Menu Bars',
							'location'		=> 'Show Location Bar',
							'status'		=> 'Show Status Bar',
							'scrollbars'	=> 'Show Scroll Bars',
							'resizable'		=> 'Allow Resizing',
						   );

if (!isset($_GET['assetid']))  $_GET['assetid'] = 0;
if (!isset($_GET['url']))      $_GET['url'] = 0;
if (!isset($_GET['protocol'])) $_GET['protocol'] = '';
if (!isset($_GET['status_text'])) {
	$_GET['status_text'] = '';
}
if (!isset($_GET['link_title'])) {
	$_GET['link_title'] = '';
}
if (!isset($_GET['target']))     $_GET['target'] = '';
if (!isset($_GET['new_window'])) {
	$_GET['new_window'] = 0;
}

if (strpos($_GET['assetid'], '#') !== FALSE) {
	list($_GET['assetid'], $_GET['anchor']) = explode('#', $_GET['assetid']);
}

// If we have an anchor, it will have been stuck in the URL, so break it away
if (strpos($_GET['url'], '#') !== FALSE) {
	list($_GET['url'], $_GET['anchor']) = explode('#', $_GET['url']);
} else {
	$_GET['anchor'] = '';
}

if (!isset($_GET['new_window'])) {
	foreach ($new_window_bool_options as $option => $option_text) {
		$_GET['new_window_options'][$options] = 0;
	}
} else {
	$_GET['new_window_options'] = var_unserialise($_GET['new_window_options']);
}

?>

<html style="width: 750px; height: 488px; ">
	<head>
		<title>Insert Link</title>
		<?php
		// add required js translation files, as we are using asset finder
		$include_list = Array(sq_web_path('lib').'/js/translation.js');

		$locales = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

		foreach ($locales as $locale) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$locale.'.js')) {
				$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$locale.'.js';
			}
		}

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

		<script type="text/javascript">
			function getFocus() {
				setTimeout('self.focus()',100);
			};

			var new_window_bool_options = new Array('<?php echo implode("','", array_keys($new_window_bool_options))?>');

			function Init() {
				__dlg_init("matrixInsertLink");
				enable_new_window(document.main_form, <?php echo $_GET['new_window']?>);


				var patterns = {<?php
					$url_protocol_options_sorted = $url_protocol_options;
					uasort($url_protocol_options_sorted, create_function('$a,$b', 'return strlen($b) - strlen($a);'));

					foreach ($url_protocol_options_sorted as $label => $pattern) {
						$pattern_array[] = '\''.addslashes($label).'\': \''.str_replace('%%link%%', '([^#]*)', addslashes($pattern)).'\'';
					}

					echo implode($pattern_array, ', ');
				?>};

				// loop through each pattern
				for (label in patterns) {
					e = patterns[label];
					// Add hash
					e = '^' + e + '(#(.*))?$';
					var re = new RegExp(e, '');
					// we need to make sure that the url does'nt have any single quote
					var results = re.exec('<?php echo str_replace("'", '%27', $_GET['url']); ?>');
					if (results) {
						break;
					}
				}

				if (results) {
					setUrl(label, results[1]);
				} else {
					// we need to make sure that the url does not have any single quote
					setUrl('', '<?php echo str_replace("'", '%27', $_GET['url']); ?>');
				}
				//var e = '^(.+:\/\/?)?([^#]*)(#(.*))?$';
				//var re = new RegExp(e, '');
				//var results = re.exec('<?php echo $_GET['url']?>');
				//setUrl(results[1], results[2]);
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["url"];
				var param = new Object();
				var f = document.main_form;

				// check for the manual entering in the asset picker
				if ((form_element_value(f.url_link) == '') && (form_element_value(f.anchor) == '') && (f.elements["assetid[assetid]"].value != '') && (f.elements["assetid[assetid]"].value != 0)) {
					setUrl();
				}

				// check if there is just an anchor in there
				if ((form_element_value(f.url_link) == '') && (form_element_value(f.anchor) != '')) {
				  param["url"]         = '#' + form_element_value(f.anchor);
				} else {
					var patterns = {<?php
					foreach ($url_protocol_options as $label => $pattern) {
						$pattern_array[] = '\''.addslashes($label).'\': \''.addslashes($pattern).'\'';
					}

					echo implode($pattern_array, ', ');
				?>};
					if (form_element_value(f.url_protocol) == '') {
						param["url"] = form_element_value(f.url_link)
					} else {
						param["url"] = patterns[form_element_value(f.url_protocol)].replace('%%link%%', form_element_value(f.url_link));
					}
					param["url"] += (form_element_value(f.anchor) == '' ? '' : '#' + form_element_value(f.anchor));
				}
				param["status_text"] = form_element_value(f.status_text);
				param["link_title"] = form_element_value(f.link_title);
				param["target"]      = form_element_value(f.target);
				param["new_window"]  = form_element_value(f.new_window);

				param["new_window_options"] = new Object();
				param["new_window_options"]["width"]  = form_element_value(f.width);
				param["new_window_options"]["height"] = form_element_value(f.height);
				for (var i=0; i < new_window_bool_options.length; i++) {
					param["new_window_options"][new_window_bool_options[i]] = (f.elements[new_window_bool_options[i]].checked) ? 1 : 0;
				}

				__dlg_close("matrixInsertLink", param);
				return false;
			};

			function onCancel() {
				__dlg_close("matrixInsertLink", null);
				return false;
			};

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

			function enable_new_window(f, enable) {
				var bg_colour = '#' + ((enable == 2) ? 'ffffff' : 'c0c0c0');

				// make sure that the new window box says what it's supposed to
				highlight_combo_value(f.new_window, enable);

				f.target.disabled  = (enable != 0);

				var disable = (enable != 2);
				f.width.disabled  = disable;
				f.height.disabled = disable;
				f.width.style.backgroundColor  = bg_colour;
				f.height.style.backgroundColor = bg_colour;
				for (var i=0; i < new_window_bool_options.length; i++) {
					f.elements[new_window_bool_options[i]].disabled = disable;
				}
			};

		</script>

		<style type="text/css">
			html, body {
				background: #FCFCFC;
				color: #000000;
				font: 11px Tahoma,Verdana,sans-serif;
				margin: 0px;
				padding: 0px;
			}

			table {
				font: 11px Tahoma,Verdana,sans-serif;
			}

			form#main-form {
				padding: 5px;
				clear: right;
			}

			#quick-search {
				font: 11px Tahoma,Verdana,sans-serif;
				letter-spacing: 0;
				float: right;
				padding-right: 12px;
			}

			#quick-search #quick-search-for {
				font: 11px Arial,Verdana,sans-serif;
				border: 1px solid black;
				padding: 1px 3px;
			}

			#quick-search #quick-search-for-label {
				font: 11px Arial,Verdana,sans-serif;
				color: #999;
			}


			/* main popup title */
			.title {
				background: #402F48;
				color: #FFFFFF;
				font-weight: bold;
				font-size: 120%;
				padding: 6px 10px;
				margin-bottom: 10px;
				border-bottom: 1px solid black;
				letter-spacing: 4px;
			}

			/* fieldset styles */
			fieldset {
				padding: 0px 10px 5px 5px;
				border-color: #725B7D;
			}

			.fl { width: 9em; float: left; padding: 2px 5px; text-align: right; }
			.fr { width: 7em; float: left; padding: 2px 5px; text-align: right; }

			/* form and form fields */
			form { padding: 0px; margin: 0px; }

			select, input, button {
				font: 11px Tahoma,Verdana,sans-serif;
			}

			button {
				width: 70px;
			}

			/* colour picker button styles */
			.buttonColor, .buttonColor-hilite {
				cursor: default;
				border: 1px solid;
				border-color: #9E86AA #725B7D #725B7D #9E86AA;
			}

			.buttonColor-hilite {
				border-color: #402F48;
			}

			.buttonColor-chooser, .buttonColor-nocolor, .buttonColor-nocolor-hilite {
				height: 0.6em;
				border: 1px solid;
				padding: 0px 1em;
				border-color: ButtonShadow ButtonHighlight ButtonHighlight ButtonShadow;
			}

			.buttonColor-nocolor, .buttonColor-nocolor-hilite { padding: 0px; }
			.buttonColor-nocolor-hilite { background: #402F48; color: #FFFFFF; }

			/* Popup styles (for backend search feature) */

			#new-message-popup, #search-wait-popup {
				position: absolute;
				right: 10px;
				top: 0;
				width: 300px;
				background-color: white;
				border: 2px solid black;
				font: normal 10px Arial,Verdana,sans-serif;
				display: none;
			}

			#new-message-popup-titlebar, #search-wait-popup-titlebar {
				font-weight: bold;
				padding: 5px;
			}

			#new-message-popup-close, #search-wait-popup-close {
				float: right;
			}

			#new-message-popup-close a, #search-wait-popup-close a {
				color: black;
				text-decoration: none;
			}

			#new-message-popup-details, #search-wait-popup-details {
				padding: 5px;
			}

			div.search-result {
				padding: 0;
				margin: 5px;
			}

			div.search-result-blurb {
				padding: 0;
				margin: 5px;
				font-weight: bold;
			}

			div.search-result-pager {
				padding: 0;
				margin: 5px;
				text-align: center;
			}

			div.search-result-detail {
				padding: 0;
				padding-left: 15px;
				margin: 5px;
				display: none;
			}

			a.search-result-expand-link {
				text-decoration:	none;
				top:				0px;
				left:				0px;
				height:				10px;
				font-size:			14px;
				margin-top:			0px;
				font-weight: 		bold;
				text-decoration:	none;
				color:				#33B9E6;
			}

			.search-result-expand-div {
				float:				left;
				width:				22px;
				font-weight: 		bold;
				background-color:	white;
				white-space:		nowrap;
			}

			.search-result-entry {
				margin-top:		5px;
				text-indent:	-38px;
				padding-left:	50px;
			}

			.sq-backend-search-failed-table {
				border:				2px solid #594165;
				border-collapse:	collapse;
				background-color:	#ECECEC;
			}

			.sq-backend-search-failed-heading, .sq-backend-search-failed-body {
				color:				#342939;
				background-color:	#ececec;
				font-family:		Arial, Verdana, Helvetica, sans-serif;
				font-size:			10px;
				vertical-align:		top;
				padding:			5px;
				text-decoration:	none;
				font-weight:		bold;
			}

			.sq-backend-search-failed-body {
				color:				#342939;
				font-weight:		normal;
			}

			.sq-backend-search-results-table {
				border:				2px solid #594165;
				border-collapse:	collapse;
				background-color:	#ECECEC;
			}

			.sq-backend-search-results-heading, .sq-backend-search-results-body {
				color:				#342939;
				background-color:	#FFFFFF;
				font-family:		Arial, Verdana, Helvetica, sans-serif;
				font-size:			10px;
				vertical-align:		top;
				padding:			5px;
				text-decoration:	none;
				font-weight:		bold;
			}

			.sq-backend-search-results-heading {
				background-color:	#F0F0E6;
			}

			.sq-backend-search-results-highlight {
				background-color:	yellow;
			}

			.sq-backend-search-results-body {
				color:				#342939;
				font-weight:		normal;
			}
		</style>
	</head>

	<body onload="Javascript: Init();" onUnload="Javascript: asset_finder_onunload();">
		<form action="" method="get" name="main_form" id="main-form">
			<table>
				<tr>
					<td valign="top">
						<?php
							include_once(SQ_LIB_PATH.'/asset_map/asset_map.inc');
							$asset_map =& new Asset_Map();
							$asset_map->embedAssetMap('simple', 200, 350);
						?>
					</td>
					<td valign="top">
							<table width="100%">
								<tr>
									<td>
										<table width="100%" cellspacing="0" cellpadding="0">
											<tr>
												<td valign="top" width="100%">
													<fieldset>
													<legend><b><?php echo translate('general'); ?></b></legend>
													<table style="width:100%">
														<tr>
															<td class="label"><?php echo translate('protocol'); ?>:</td>
															<td><?php
															combo_box('url_protocol', $url_protocol_combo_box, $_GET['protocol'], 'style="font-family: courier new; font-size: 11px;"'); ?></td>
															<td class="label"><?php echo translate('link'); ?>:</td>
															<td><?php text_box('url_link', $_GET['url'], 40, 0)?></td>
														</tr>
														<tr>
															<td class="label"><?php echo translate('select_asset'); ?>:</td>
															<td colspan="3"><?php asset_finder('assetid', $_GET['assetid'], Array(), '', FALSE, 'setUrl'); ?></td>
														</tr>
														<tr>
															<td class="label"><?php echo translate('anchor_name'); ?>:</td>
															<td colspan="3"><?php text_box('anchor', $_GET['anchor'], 40, 0) ?></td>
														</tr>
													</table>
													</fieldset>
												</td>
											</tr>
										</table>
									</td>
								</tr>
								<tr>
									<td>
										<table width="100%" cellspacing="0" cellpadding="0">
											<tr>
												<td valign="top" width="100%">
													<fieldset>
														<legend><?php echo translate('options'); ?></legend>
														<table style="width:100%">
															<tr>
																<td class="label"><?php echo translate('status_bar_text'); ?>:</td>
																<td><?php text_box('status_text', $_GET['status_text'], 50); ?></td>
															</tr>
															<tr>
																<td class="label"><?php echo translate('title'); ?>:</td>
																<td><?php text_box('link_title', $_GET['link_title'], 50); ?></td>
															</tr>
														</table>
													</fieldset>
												</td>
											</tr>
										</table>
									</td>
								</tr>
								<tr>
									<td>
										<table width="100%" cellspacing="0" cellpadding="0">
											<tr>
												<td valign="top" width="100%">
													<fieldset>
														<legend><?php echo translate('new_window_options'); ?></legend>
														<table style="width:100%">
															<tr>
																<td class="label" valign="top"><?php echo translate('target'); ?>:</td>
																<td><?php text_box('target', $_GET['target']); ?></td>
															</tr>
															<tr>
																<td class="label" rowspan="2" valign="top"><?php echo translate('new_window'); ?>:</td>
																<td><?php combo_box('new_window', Array('0' => translate('no'), '1' => translate('yes'), '2' => translate('advanced')), FALSE, $_GET['new_window'], 1, 'onChange="javascript: enable_new_window(this.form, form_element_value(this));"'); ?></td>
															</tr>
															<tr>
																<td>
																	<table border="0" cellspacing="0" cellpadding="0">
																		<tr>
																		<?php
																			$count = 0;
																			foreach ($new_window_bool_options as $var => $name) {
																				$count++;
																			?>
																					<td width="33%">
																						<input type="checkbox" value="1" name="<?php echo $var?>" <?php echo ($_GET['new_window_options'][$var]) ? 'checked' : '';?>>
																						<?php echo $name?>
																					</td>
																			<?php
																				if ($count % 2 == 0) echo '</tr><tr>';
																			}
																		?>
																		</tr>
																		<tr>
																			<td colspan="3">
																				<?php echo translate('size'); ?> : <input type="text" value="<?php echo $_GET['new_window_options']['width']?>" size="3" name="width"> (w) x <input type="text" value="<?php echo $_GET['new_window_options']['height']?>" size="3" name="height"> (h)
																			</td>
																		</tr>
																	</table>
																</td>
															</tr>
														</table>
													</fieldset>
												</td>
											</tr>
										</table>
									</td>
								</tr>
							</table>

						<div style="margin-top: 5px; text-align: right;">
						<hr />
						<button type="button" name="ok" onclick="return onOK();"><?php echo translate('ok'); ?></button>
						<button type="button" name="cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
						</div>
					</td>
				<tr>
			</table>
		</form>

		<!-- Search results -->
		<div id="new-message-popup"><div id="new-message-popup-titlebar"><div id="new-message-popup-close">[ <a href="#" onclick="document.getElementById('new-message-popup').style.display = 'none'; return false;">x</a> ]</div><span id="new-message-popup-title">Searched for ''</span></div>
			<div id="new-message-popup-details"></div>
		</div>
		<div id="search-wait-popup"><div id="search-wait-popup-titlebar"><div id="search-wait-popup-close">[ <a href="#" onclick="document.getElementById('search-wait-popup').style.display = 'none'; return false;">x</a> ]</div><span id="search-wait-popup-title">Search in Progress</span></div>
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

				ASSET_FINDER_OBJ.set_hidden_field('assetid[assetid]', assetid);
				ASSET_FINDER_OBJ.set_hidden_field('assetid[url]', url);
				ASSET_FINDER_OBJ.set_hidden_field('assetid[linkid]', linkid);
				ASSET_FINDER_OBJ.set_text_field('sq_asset_finder_assetid_label', (assetid == 0) ? '' : label + ' (Id : #' + assetid + ')');

				document.getElementById("new-message-popup").style.display = 'none';
				setUrl('', './?a=' + assetid);
			}

		// --></script>
	</body>
</html>
