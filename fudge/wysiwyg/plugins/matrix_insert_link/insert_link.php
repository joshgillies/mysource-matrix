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
* $Id: insert_link.php,v 1.62 2013/09/15 23:53:23 lwright Exp $
*
*/

/**
* Insert Link Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.62 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

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

if (!isset($_GET['assetid']))  {
	$_GET['assetid'] = 0;
} else {
	preg_match('/^([0-9A-Z:]*)/i', $_GET['assetid'], $matches);
	$_GET['assetid'] = isset($matches[1]) ? $matches[1] : $_GET['assetid'];
}
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
if (!isset($_GET['class_name'])) {
	$_GET['class_name'] = '';
}
if (!isset($_GET['id_name'])) {
	$_GET['id_name'] = '';
}
if (!isset($_GET['show_summary'])) {
	$_GET['show_summary'] = 0;
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

// we have the assetid lets try to get the asset
$option_selected = 'all_asset';
if (isset($_GET['assetid']) && $_GET['assetid']) {
	$asset_linked = $GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
	if (!is_null($asset_linked)) {
		if ($asset_linked instanceof Link) {
			$option_selected = 'link_manager';
		} else if ($asset_linked instanceof Page_Redirect) {
			$option_selected = 'redirect';
		}
	}
}
?><!DOCTYPE html>
<html>
	<head>
		<title>Insert Link</title>
		<meta http-equiv="content-type" content="text/html; charset=<?php echo SQ_CONF_DEFAULT_CHARACTER_SET;?>" />
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
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/web/dfx/dfx.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/asset_map.js' ?>"></script>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/asset_map/js/js_asset_map.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

		<script type="text/javascript">
			function getFocus() {
				setTimeout('self.focus()',100);
			};

			var new_window_bool_options = new Array('<?php echo implode("','", array_keys($new_window_bool_options))?>');

			function Init() {
				__dlg_init("matrixInsertLink");
				enable_new_window(document.main_form, <?php echo preg_replace('/[\'"\(\);\[\]{}<>=]+/', '', $_GET['new_window']); ?>);


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
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["url"];
				var param = new Object();
				var f = document.main_form;

				// check for the manual entering in the asset picker
				if ((form_element_value(f.url_link) == '') && (form_element_value(f.anchor) == '') &&
					(((f.elements["assetid[assetid]"].value != '') && (f.elements["assetid[assetid]"].value != 0)) ||
					 ((f.elements["page_redirect_assetid[assetid]"].value != '') && (f.elements["page_redirect_assetid[assetid]"].value != 0)) ||
					 ((f.elements["link_assetid[assetid]"].value != '') && (f.elements["link_assetid[assetid]"].value != 0))
					 )) {
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
				param["class_name"]  = form_element_value(f.class_name);
				param["id_name"]  = form_element_value(f.id_name);
				param["show_summary"]  = form_element_value(f.show_summary);
				param["assetid"] = f.elements["assetid[assetid]"].value;

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
					var assetid = '';
					if (f.elements["assetid[assetid]"].value != undefined && f.elements["assetid[assetid]"].value != '<?php echo $_GET['assetid']; ?>' && f.elements["assetid[assetid]"].value != '') {
						assetid = f.elements["assetid[assetid]"].value;
					} else if (f.elements["link_assetid[assetid]"].value != undefined && f.elements["link_assetid[assetid]"].value != '<?php echo $_GET['assetid']; ?>' && f.elements["link_assetid[assetid]"].value != '') {
						assetid = '%globals_asset_attribute_link_url:'+ f.elements["link_assetid[assetid]"].value +'%';
					} else if (f.elements["page_redirect_assetid[assetid]"].value != undefined && f.elements["page_redirect_assetid[assetid]"].value != '<?php echo $_GET['assetid']; ?>' && f.elements["page_redirect_assetid[assetid]"].value != '') {
						assetid = '%globals_asset_attribute_redirect_url:'+ f.elements["page_redirect_assetid[assetid]"].value +'%';
					}

					// Selected asset's type code
					var asset_type = '';
					if (f.elements["assetid[type_code]"].value != '') {
						asset_type = f.elements["assetid[type_code]"].value;
					} else if (f.elements["link_assetid[type_code]"].value != '') {
						asset_type = f.elements["link_assetid[type_code]"].value;
					} else if (f.elements["page_redirect_assetid[type_code]"].value != '') {
						asset_type = f.elements["page_redirect_assetid[type_code]"].value;
					}
					if (asset_type != '') {
						// Disable the irrelevant asset type option based on the currently selected asset
						asset_type = asset_type.toLowerCase();
						options = f.elements["asset_type_selector"].getElementsByTagName("option");
						for(var i=0; i<options.length; i++) {
							options[i].disabled = false;
							if (asset_type != 'link' && options[i].value == 'link_manager') {
								options[i].disabled = true;
							} else if (asset_type != 'page_redirect' && options[i].value == 'redirect') {
								options[i].disabled = true;
							}
						}
					}

					if (assetid != '') {
						// shadow asset
						if (assetid.search('globals_asset_attribute') != -1) {
							f.url_link.value = assetid;
						} else {
							if (assetid.search(/:/) != -1) {
								f.url_link.value = './?a=' + assetid + '$';
							} else {
								f.url_link.value = './?a=' + assetid;
							}
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

			function reset_type_selector()
			{
				// Enable all the asset selector options (default)
				options = document.main_form.elements["asset_type_selector"].getElementsByTagName("option");
				for(var i=0; i<options.length; i++) {
					options[i].disabled = false;
				}

				document.getElementById('assetid[type_code]').value = '';
				document.getElementById('link_assetid[type_code]').value = '';
				document.getElementById('page_redirect_assetid[type_code]').value = '';

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
				            <iframe src="insert_link_asset_map.php" name="sq_wysiwyg_popup_sidenav" frameborder="0" width="200" height="450" scrolling="no">
				            </iframe>
				        </div>
				    </td>
					<td valign="top">
							<table width="100%">
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
															<td class="lable"><?php echo translate('asset_type').':'; ?></td>
															<td colspan="3">
																<?php
																	$asset_selector = Array(
																						'all_assets'	=> 'Any Asset',
																						'link_manager'	=> 'Link Asset',
																						'redirect'		=> 'Redirect Page Asset',
																					  );

																	$object_type  = 'this.form.asset_type_selector.value';
																	$object_span1 = 'document.getElementById(\'asset_finder_all\')';
																	$object_span2 = 'document.getElementById(\'asset_finder_link_asset\')';
																	$object_span3 = 'document.getElementById(\'asset_finder_redirect_asset\')';
																	$js = 'onChange="chooseSourceType('.$object_type.', '.$object_span1.', '.$object_span2.', '.$object_span3.');"';
																	?>
																	<script type="text/javascript">
																		function chooseSourceType(type, span_all, span_link_manager, span_redirect_page) {
																			all_asset_picker = document.getElementById('assetid[assetid]');
																			all_asset_picker_id = document.getElementById('sq_asset_finder_assetid_assetid');
																			all_asset_picker_label = document.getElementById('sq_asset_finder_assetid_label');

																			link_asset_picker = document.getElementById('link_assetid[assetid]');
																			link_asset_picker_id = document.getElementById('sq_asset_finder_link_assetid_assetid');
																			link_asset_picker_label = document.getElementById('sq_asset_finder_link_assetid_label');

																			redirect_asset_picker = document.getElementById('page_redirect_assetid[assetid]');
																			redirect_asset_picker_id = document.getElementById('sq_asset_finder_page_redirect_assetid_assetid');
																			redirect_asset_picker_label = document.getElementById('sq_asset_finder_page_redirect_assetid_label');
																			var selected_assetid = '';
																			var selected_label = '';
																			if (all_asset_picker_id.value != '') {
																				selected_assetid = all_asset_picker_id.value;
																				selected_label = all_asset_picker_label.value;
																			} else if (link_asset_picker_id.value != '') {
																				selected_assetid = link_asset_picker_id.value;
																				selected_label = link_asset_picker_label.value;
																			} else if (redirect_asset_picker_id.value != '') {
																				selected_assetid = redirect_asset_picker_id.value;
																				selected_label = redirect_asset_picker_label.value;
																			}

																			if (type == 'all_assets') {
																				all_asset_picker.value=selected_assetid;
																				all_asset_picker.disabled=false;
																				all_asset_picker_id.value=selected_assetid;
																				all_asset_picker_id.disabled=false;
																				all_asset_picker_label.value=selected_label;
																				span_all.style.display='block';
																				span_all.disabled=false;
																				span_link_manager.style.display='none';
																				span_link_manager.disabled=true;
																				link_asset_picker.value='';
																				link_asset_picker.disabled=true;
																				link_asset_picker_id.value='';
																				link_asset_picker_id.disabled=true;
																				link_asset_picker_label.value='';
																				span_redirect_page.style.display='none';
																				span_redirect_page.disabled=true;
																				redirect_asset_picker.value='';
																				redirect_asset_picker.disabled=true;
																				redirect_asset_picker_id.value='';
																				redirect_asset_picker_id.disabled=true;
																				redirect_asset_picker_label.value='';

																			} else if (type == 'link_manager') {
																				all_asset_picker.value='';
																				all_asset_picker.disabled=true;
																				all_asset_picker_id.value='';
																				all_asset_picker_id.disabled=true;
																				all_asset_picker_label.value='';
																				span_all.style.display='none';
																				span_all.disabled=true;
																				span_link_manager.style.display='block';
																				span_link_manager.disabled=false;
																				link_asset_picker.value=selected_assetid;
																				link_asset_picker.disabled=false;
																				link_asset_picker_id.value=selected_assetid;
																				link_asset_picker_id.disabled=false;
																				link_asset_picker_label.value=selected_label;
																				span_redirect_page.style.display='none';
																				span_redirect_page.disabled=true;
																				redirect_asset_picker.value='';
																				redirect_asset_picker.disabled=true;
																				redirect_asset_picker_id.value='';
																				redirect_asset_picker_id.disabled=true;
																				redirect_asset_picker_label.value='';

																			} else {
																				all_asset_picker.value='';
																				all_asset_picker.disabled=true;
																				all_asset_picker_id.value='';
																				all_asset_picker_id.disabled=true;
																				all_asset_picker_label.value='';
																				span_all.style.display='none';
																				span_all.disabled=true;
																				span_link_manager.style.display='none';
																				span_link_manager.disabled=true;
																				link_asset_picker.value='';
																				link_asset_picker.disabled=true;
																				link_asset_picker_id.value='';
																				link_asset_picker_id.disabled=true;
																				link_asset_picker_label.value='';
																				span_redirect_page.style.display='block';
																				span_redirect_page.disabled=false;
																				redirect_asset_picker.value=selected_assetid;
																				redirect_asset_picker.disabled=false;
																				redirect_asset_picker_id.value=selected_assetid;
																				redirect_asset_picker_id.disabled=false;
																				redirect_asset_picker_label.value=selected_label;
																			}
																			setUrl();
																		}
																	</script>
																	<?php
																	combo_box('asset_type_selector', $asset_selector, FALSE, $option_selected, 0, $js);

																?>
															</td>
														</tr>
														<tr>
															<td class="label"><?php echo translate('select_asset'); ?>:</td>
															<td colspan="3">
																<span id="asset_finder_all" style="display:block">
																	<?php asset_finder('assetid', $_GET['assetid'], Array(), 'sq_wysiwyg_popup_sidenav', FALSE, 'setUrl'); ?>
																</span>
																<span id="asset_finder_link_asset" style="display:none">
																	<?php asset_finder('link_assetid', $_GET['assetid'], Array('link' => 'I'), 'sq_wysiwyg_popup_sidenav', FALSE, 'setUrl'); ?>
																</span>
																<span id="asset_finder_redirect_asset" style="display:none">
																	<?php asset_finder('page_redirect_assetid', $_GET['assetid'], Array('page_redirect' => 'I'), 'sq_wysiwyg_popup_sidenav', FALSE, 'setUrl'); ?>
																	<script type="text/javascript">
																		// Add event listener for Asset Selector's clear button
																		var all_clr_btn = document.getElementById('sq_asset_finder_assetid_clear_btn');
																		var link_clr_btn = document.getElementById('sq_asset_finder_link_assetid_clear_btn');
																		var page_redirect_clr_btn = document.getElementById('sq_asset_finder_page_redirect_assetid_clear_btn');
																		if (all_clr_btn.addEventListener) {
																			all_clr_btn.addEventListener('click', reset_type_selector, false);
																			link_clr_btn.addEventListener('click', reset_type_selector, false);
																			page_redirect_clr_btn.addEventListener('click', reset_type_selector, false);
																		} else if (all_clr_btn.attachEvent) {
																			// IE browser
																			all_clr_btn.attachEvent('onclick', reset_type_selector);
																			link_clr_btn.attachEvent('onclick', reset_type_selector);
																			page_redirect_clr_btn.attachEvent('onclick', reset_type_selector);

																		}
																</script>
																</span>
															</td>
														</tr>
														<tr>
															<td class="label"><?php echo translate('anchor_name'); ?>:</td>
															<td colspan="3"><?php text_box('anchor', $_GET['anchor'], 40, 0) ?></td>
														</tr>
													</table>
													</fieldset>
												</td>
								</tr>
								<tr>
												<td valign="top">
													<fieldset>
														<legend><b><?php echo translate('options'); ?></b></legend>
														<table>
															<tr>
																<td class="label"><?php echo translate('id_name_text'); ?>:</td>

																<td>																	
																	<?php text_box('id_name', $_GET['id_name'], 15); ?>		

																    <span class="label"><?php echo translate('class_name_text'); ?>:</span>
																    <?php text_box('class_name', $_GET['class_name'], 15); ?>
															
																	<span class="label"><?php echo translate('include_summary'); ?>:</span>
																	<?php check_box('show_summary', 1, $_GET['show_summary']); ?>
																</td>	
															</tr>
															<tr>	
																<td class="label"><?php echo translate('title'); ?>:</td>															
																<td colspan="2">																														
																	<?php text_box('link_title', $_GET['link_title'], 50); ?>
																</td>															
															</tr>
														</table>
													</fieldset>
												</td>
								</tr>
								<tr>
												<td valign="top" width="100%">
													<fieldset>
														<legend><b><?php echo translate('new_window_options'); ?></b></legend>
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
																				<?php echo translate('size'); ?> : <input type="text" value="<?php echo htmlspecialchars($_GET['new_window_options']['width']) ?>" size="3" name="width"> (w) x <input type="text" value="<?php echo htmlspecialchars($_GET['new_window_options']['height']) ?>" size="3" name="height"> (h)
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

								<div class="sq-popup-clean-fieldset">
									<div class="sq-popup-button-wrapper">
										<button type="button" name="cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
															    								
										<button type="button" name="ok" onclick="return onOK();" class="sq-btn-green"><?php echo translate('ok'); ?></button>
						
									</div>
								</div>			
					</td>
				<tr>
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

			function set_asset_finder_from_search(assetid, label, url, linkid, type_code) {
				document.cookie = 'lastSelectedAssetId=' + escape(assetid);
				reset_type_selector();
				var prefix = 'assetid';
				if (type_code == 'link' || type_code == 'page_redirect') {
					prefix = type_code + '_' + prefix;
				}
				
				dfx.getId(prefix + '[assetid]').value    = assetid;
				dfx.getId(prefix + '[url]').value        = url;
				dfx.getId(prefix + '[linkid]').value     = linkid;
				dfx.getId(prefix + '[type_code]').value  = type_code;
				dfx.getId('sq_asset_finder_' + prefix + '_label').value   = label;
				dfx.getId('sq_asset_finder_' + prefix + '_assetid').value = assetid;
				
				document.getElementById("new-message-popup").style.display = 'none';
				setUrl();
			}

		// --></script>
	</body>
</html>
