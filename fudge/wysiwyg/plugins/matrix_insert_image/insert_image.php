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
* $Id: insert_image.php,v 1.64 2013/09/15 23:53:23 lwright Exp $
*
*/

/**
* Insert Image Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.64 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

$url_protocol_options = Array(
							''			=> '',
							'http://'	=> 'http://',
							'https://'	=> 'https://',
						);

if (!isset($_GET['f_imageid'])) $_GET['f_imageid'] = 0;
?><!DOCTYPE html>

<html>
	<head>
		<title>Insert Image</title>
		<meta http-equiv="content-type" content="text/html; charset=<?php echo SQ_CONF_DEFAULT_CHARACTER_SET;?>" />

		<?php
		//add required js translation files, as we are using asset finder
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
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/JsHttpConnector.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/tooltip.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/web/dfx/dfx.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/asset_map.js' ?>"></script>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/asset_map/js/js_asset_map.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

		<script type="text/javascript">

			var is_post_ie11 = Boolean(navigator.userAgent.match(/Trident\/\d{1,}[\.\d]*;/)) && navigator.appName=="Netscape";

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("matrixInsertImage");
				if (document.getElementById("f_imageid[assetid]").value != 0) {
					newImg(document.getElementById('image_container'), '<?php echo sq_web_path('root_url'); ?>' + '/?a=' + document.getElementById("f_imageid[assetid]").value, document.getElementById('f_width').value, document.getElementById('f_height').value);
				}
			};

			function onOK() {
				document.getElementById('main-form').action = "";
				document.getElementById('main-form').method = "get";
				if (navigator.appName == "Microsoft Internet Explorer" || is_post_ie11) {
					// Hack for IE, Files don't get uploaded unless this is set a very special way
					// ie. don't set it here !?!
				} else {
					document.getElementById('main-form').enctype = "";
				}
				document.getElementById('main-form').target = "";
				var required = {
					"f_alt": "Please enter the alternate text"
				};
				for (var i in required) {
					var el = document.getElementById(i);
					if (!el.value) {
						alert(required[i]);
						el.focus();
						return false;
					}
				}
				// pass data back to the calling window
				var fields = ["f_alt", "f_title", "f_align", "f_border", "f_image_class", "f_image_id",
							"f_horiz", "f_vert", "f_width", "f_height"];
				var param = new Object();
				for (var i = 0; i < fields.length; i++) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}

				// Because the id of the f_image field has array references in it,
				// we can't get use getElementById, so do this...
				param["f_imageid"] = document.main_form.elements["f_imageid[assetid]"].value;

				// Optional Long Description
				if (document.main_form.elements["f_longdesc[assetid]"].value != "" && document.main_form.elements["f_longdesc[assetid]"].value != "0") {
					param["f_longdesc"] = document.main_form.elements["f_longdesc[assetid]"].value;
				} else {
					param["f_longdesc"] = document.main_form.elements["f_longdesc_protocol"].value + document.main_form.elements["f_longdesc_link"].value;
				}

				__dlg_close("matrixInsertImage", param);
				return false;
			};

			function onCancel() {
				__dlg_close("matrixInsertImage", null);
				return false;
			};

			function populateImageInfo(responseText) {
				var imageInfo = var_unserialise(responseText);

				// generate the alt text from the file name if they have not set it
				// for the image asset
				if (imageInfo['alt'] == '') {
					var path = imageInfo['name'];
					ext = "";
					i = path.length;
					while (ext.charAt(0) != "." && i > 0) ext = path.substring(i--);
					imageInfo['alt'] = path.substring(0, ++i);
				}
				document.getElementById("f_alt").value		= imageInfo['alt'];
				document.getElementById("f_width").value	= imageInfo['width'];
				document.getElementById("f_height").value	= imageInfo['height'];
				newImg(document.getElementById('image_container'), '<?php echo sq_web_path('root_url'); ?>' + '/?a=' + document.getElementById("f_imageid[assetid]").value, imageInfo['width'], imageInfo['height']);
			};

			function setImageInfo() {
				// put a random no in the url to overcome any caching
				var assetid = document.getElementById("f_imageid[assetid]").value;

				var url = '<?php echo sq_web_path('root_url').'/'.SQ_CONF_LIMBO_SUFFIX; ?>/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=' + escape(assetid) + '&asset_ei_screen=image_info&ignore_frames=1&t=' + Math.random() * 1000;

				JsHttpConnector.submitRequest(url, populateImageInfo);
			};

			function doStatus(img) {
				//on Image Load
			}

			function newImg(div, url, width, height) {
				var limit = 160.0;
				var scalar = 1.0;
				// convert width and height into a float format
				width = parseFloat(width);
				height = parseFloat(height);
				var img = document.getElementById('preview_image');
				img.height = 0;
				img.width = 0;
				img.src = url;

				img.onload = function() { doStatus()};

				if( width > limit || height > limit) {
					if (width > height) {
						scalar = limit / width;
					} else {
						scalar = limit / height;
					}
				}

				img.width = parseInt(Math.ceil(width * scalar));
				img.height = parseInt(Math.ceil(height * scalar));

				div.appendChild(img);

			}


			function setImagePreview() {
				if (document.getElementById("f_imageid[assetid]").value == "" || document.getElementById("f_imageid[assetid]").value == 0) {
					return;
				}
				newImg(document.getElementById('image_container'), '<?php echo sq_web_path('root_url'); ?>' + '/?a=' + document.getElementById("f_imageid[assetid]").value);
			}

			function toggleCreateImage() {
				changeButton = document.getElementById('show_create_button');
				changeDivElements = ["show_create_image_label1", "show_create_image1", "show_create_image_label2", "show_create_image2", "show_create_image_submit"];
					//document.getElementById('show_create_image');
				errorDiv = document.getElementById('show_upload_error');
				if (changeButton.style.display == "block") {
					for (var i = 0; i < changeDivElements.length; i++) {
						var changeDiv = document.getElementById(changeDivElements[i]);
						changeDiv.style.visibility = "visible";
						changeDiv.style.display = "block";
					}
					changeButton.style.visibility = "hidden";
					changeButton.style.display = "none";
				} else {
					for (var i = 0; i < changeDivElements.length; i++) {
						var changeDiv = document.getElementById(changeDivElements[i]);
						changeDiv.style.visibility = "hidden";
						changeDiv.style.display = "none";
					}
					errorDiv.style.visibility = "hidden";
					errorDiv.style.display = "none";
					changeButton.style.visibility = "visible";
					changeButton.style.display = "block";
				}
			}

			function submitCreateImage() {
				document.getElementById('main-form').action = "upload_image.php";
				document.getElementById('main-form').method = "post";
				document.getElementById('main-form').setAttribute("enctype", "multipart/form-data");
				document.getElementById('main-form').target = "create_image_frame";
			}

		</script>

		<?php define('SQ_PAINTED_SIMPLE_ASSET_MAP', TRUE); ?>
	</head>

	<body onload="Init();" onUnload="asset_finder_onunload();">
		<form action="" method="get" name="main_form" id="main-form">
	    	<?php
			// insert nonce secuirty token.
			if( $GLOBALS['SQ_SYSTEM']->user && !($GLOBALS['SQ_SYSTEM']->user instanceof Public_User))
			hidden_field('token', get_unique_token());
			?>
			<table class="sq-fieldsets-table">
				<tr>
				    <td valign="top" class="sq-popup-asset-map">
				        <div id="asset_map">
				            <iframe src="insert_image_asset_map.php" name="sq_wysiwyg_popup_sidenav" frameborder="0" width="200" height="600px" scrolling="no">
				            </iframe>
				        </div>
				    </td>
					<td valign="top">
						<table cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%" colspan=2>
								<fieldset>
									<legend><b><?php echo translate('general'); ?></b></legend>
										<table>
											<tr>
												<td class="label" nowrap="nowrap"><?php echo translate('image_url'); ?>:</td>
												<td>
													<?php asset_finder('f_imageid', (assert_valid_assetid($_GET['f_imageid'], '', TRUE, FALSE) ? $_GET['f_imageid'] : 0), Array('image' => 'D', 'image_variety' => 'D'), 'sq_wysiwyg_popup_sidenav', false, 'setImageInfo'); ?>
												</td>
											</tr>
											<tr>
												<td class="label" nowrap="nowrap"><?php echo translate('alternate_text'); ?>:</td>
												<td>
													<input type="text" size="70" name="alt" id="f_alt" title="For browsers that don't support images" value="<?php echo htmlspecialchars($_REQUEST['f_alt']) ?>" />
												</td>
											</tr>
											<tr>
												<td class="label" nowrap="nowrap"><?php echo translate('title_text'); ?>:</td>
												<td>
													<input type="text" size="70" name="title" id="f_title" title="Your Image Title" value="<?php echo htmlspecialchars($_REQUEST['f_title']) ?>" />
												</td>
											</tr>
											<tr>
												<td></td>
												<td>
												<div id="show_create_button" style="display: block;">
													<input type="button" name="show" value="Create Image" onclick="toggleCreateImage();" />
												</div>
												</td>												
											</tr>
											<tr>
												<td colspan="2" class="sq-popup-no-padding sq-popup-hidden-cell">
													<div id="show_upload_error" class="hidden">
													</div>
												</td>
											</tr>
											<tr>
												<td class="sq-popup-no-padding sq-popup-hidden-cell"><div id="show_create_image_label1" class="hidden">Create an image:</div></td>
												<td class="sq-popup-no-padding sq-popup-hidden-cell"><div id="show_create_image1" class="hidden"><input name="create_image_upload" id="create_image_upload" type="file" /></div></td>
											</tr>
											<tr>
												<td class="sq-popup-no-padding sq-popup-hidden-cell"><div id="show_create_image_label2" class="hidden">Create under:</div></td>
												<td class="sq-popup-no-padding sq-popup-hidden-cell"><div id="show_create_image2" class="hidden"><?php asset_finder('f_create_root_node', '0', Array(), 'sq_wysiwyg_popup_sidenav'); ?></div></td>
											</tr>
											<tr>
												<td class="sq-popup-no-padding sq-popup-hidden-cell"></td>
												<td class="sq-popup-no-padding sq-popup-hidden-cell"><div id="show_create_image_submit" class="hidden"><input type="submit" name="create_image_submit" value="Create &amp; Use Image" onclick="submitCreateImage();" /><input type="button" name="cancel_create_image" value="Cancel" onclick="toggleCreateImage();" /></div></td>
											    </td>
											</tr>
											</div>
											<tr>
												<td colspan="2">
													<div id="create_image_frame_div" class="hidden">
														<iframe id="create_image_frame" name="create_image_frame" src=""></iframe>
													</div>
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="100%" colspan=2>
								<fieldset>
									<legend><b><?php echo translate('optional_attributes'); ?></b></legend>
									<table >
										<tr>
											<td valign="top" class="label" nowrap="nowrap"><?php echo translate('longdesc_text'); ?>:</td>
											<td>
												<?php
												if (!empty($_GET['f_longdesc'])) {
													if (preg_match("/^\d+$/", $_GET['f_longdesc']) && $_GET['f_longdesc'] != "0") {?>
														Enter URL manually<br />
														<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, '', 'style="font-family: courier new; font-size: 11px;"'); ?>
														<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', '', 40, 0)?>
														<br />or choose a Standard Page asset<br />
														<?php asset_finder('f_longdesc', $_GET['f_longdesc'], Array('page_standard' => 'D'), 'sq_wysiwyg_popup_sidenav'); ?>
														<br />If you enter URL manually, current asset in the asset finder must be cleared .<br />
													<?php
													} else {
														$matches = Array();
														if (preg_match_all('/^(http:\/\/|https:\/\/){1}(.*)$/', $_GET['f_longdesc'], $matches) === FALSE ||
															empty($matches[0])) {?>
															Enter URL manually<br />
															<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, FALSE, '', 'style="font-family: courier new; font-size: 11px;"'); ?>
															<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', $_GET['f_longdesc'], 40, 0)?>
															<br />or choose a Standard Page asset<br />
															<?php asset_finder('f_longdesc', '0', Array('page_standard' => 'D'), 'sq_wysiwyg_popup_sidenav'); ?>
														<?php
														} else {?>
															Enter URL manually<br />
															<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, FALSE, $matches[1][0], 'style="font-family: courier new; font-size: 11px;"'); ?>
															<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', $matches[2][0], 40, 0)?>
															<br />or choose a Standard Page asset<br />
															<?php asset_finder('f_longdesc', '0', Array('page_standard' => 'D'), 'sq_wysiwyg_popup_sidenav'); ?>
														<?php
														}

													}
												} else {?>
													Enter URL manually<br />
													<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, '', 'style="font-family: courier new; font-size: 11px;"'); ?>
													<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', '', 40, 0)?>
													<br />or choose a Standard Page asset<br />
													<?php asset_finder('f_longdesc', '0', Array('page_standard' => 'D'), 'sq_wysiwyg_popup_sidenav'); ?>
												<?php
												}?>
											<br />If you enter URL manually, current asset in the asset finder must be cleared.<br />
											</td>
										</tr>
									</table>
								</fieldset>
							</tr>
							<tr>
								<td width="50%" rowspan="2">
									<fieldset class="prev">
									<legend><b><?php echo translate('preview'); ?></b></legend>
										<table class="preview" >
											<tr>
												<td id="image_container" align="center" valign="center" height="160px" width="340px">
													<img id="preview_image" src="<?php echo sq_web_path('fudge') ?>/wysiwyg/images/blank.gif" width="0" height="0">
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
								<td valign="top" width="50%">
									<fieldset>
									<legend><b><?php echo translate('layout'); ?></b></legend>
									<table>
										<tr>
											<td class="label" width="30%"><?php echo translate('alignment'); ?>:</td>
											<td>
												<select size="1" name="align" id="f_align" title="<?php echo translate('positioning_of_this_image'); ?>">
													<?php
													if (!isset($_REQUEST['f_align'])) {
														$_REQUEST['f_align'] = 'baseline';
													}
													$options_array = Array(
																		''			=> 'Not set',
																		'left'		=> translate('left'),
																		'right'		=> translate('right'),
																		'texttop'	=> translate('texttop'),
																		'absmiddle'	=> translate('absmiddle'),
																		'baseline'	=> translate('baseline'),
																		'absbottom'	=> translate('absbottom'),
																		'bottom'	=> translate('bottom'),
																		'middle'	=> translate('middle'),
																		'top'		=> translate('top'),
																	 );
													foreach ($options_array as $value => $text) {
														?><option value="<?php echo $value?>" <?php echo ($_REQUEST['f_align'] == $value) ? 'selected="1"' : ''?>><?php echo $text?></option><?php
													}
													?>
												</select>
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('border_thickness'); ?>:</td>
											<td>
												<input type="text" name="border" id="f_border" size="5" title="Leave empty for no border" value="<?php echo htmlspecialchars($_REQUEST['f_border']) ?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('horizontal'); ?>:</td>
											<td>
												<input type="text" name="horiz" id="f_horiz" size="5" title="Horizontal padding" value="<?php echo htmlspecialchars($_REQUEST['f_horiz']) ?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('vertical'); ?>:</td>
											<td>
												<input type="text" name="vert" id="f_vert" size="5" title="Vertical padding" value="<?php echo htmlspecialchars($_REQUEST['f_vert']) ?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('name'); ?>:</td>
											<td>
												<input type="text" name="image_id" id="f_image_id" size="20" title="Name" value="<?php echo htmlspecialchars($_REQUEST['f_image_id']) ?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('class'); ?>:</td>
											<td>
												<input type="text" name="image_class" id="f_image_class" size="20" title="Class" value="<?php echo htmlspecialchars($_REQUEST['f_image_class']) ?>" />
											</td>
										</tr>
									</table>
								</fieldset>
								</td>
							</tr>
							<tr>
					
								<td valign="top" width="50%">
									<fieldset>
									<legend><b><?php echo translate('size'); ?></b></legend>
									<table>
										<tr>
											<?php
											$disable_resize = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE');
											?>
											<td class="label" width="30%"><?php echo translate('width'); ?>:</td>
											<td>
												<?php
												switch ($disable_resize) {
													case 'yes':
														echo htmlspecialchars($_REQUEST['f_width']).'&nbsp;px';
														?>
														<input type="hidden" name="width" id="f_width" size="5" title="Width" value="<?php echo htmlspecialchars($_REQUEST['f_width']) ?>" />
														<?php
													break;
													case 'no':
														?>
														<input type="text" name="width" id="f_width" size="5" title="Width" value="<?php echo htmlspecialchars($_REQUEST['f_width']) ?>" />&nbsp;px
														<?php
													break;
												}
												?>
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('height'); ?>:</td>
											<td>
												<?php
												switch ($disable_resize) {
													case 'yes':
														echo htmlspecialchars($_REQUEST['f_height']).'&nbsp;px';
														?>
														<input type="hidden" name="height" id="f_height" size="5" title="Height" value="<?php echo htmlspecialchars($_REQUEST['f_height']) ?>" />
														<?php
													break;
													case 'no':
														?>
														<input type="text" name="height" id="f_height" size="5" title="Height" value="<?php echo htmlspecialchars($_REQUEST['f_height']) ?>" />&nbsp;px
														<?php
													break;
												}
												?>
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

			function set_asset_finder_from_search(assetid, label, url, linkid, filename, alt, width, height) {
				document.cookie = 'lastSelectedAssetId=' + escape(assetid);

				var prefix = 'f_imageid';
				dfx.getId(prefix + '[assetid]').value    = assetid;
				dfx.getId(prefix + '[url]').value        = url;
				dfx.getId(prefix + '[linkid]').value     = linkid;
				dfx.getId('sq_asset_finder_' + prefix + '_label').value   = label;
				dfx.getId('sq_asset_finder_' + prefix + '_assetid').value = assetid;
				
				document.getElementById("new-message-popup").style.display = 'none';
				document.getElementById("f_alt").value = alt;

				var image_info = {
				    name: filename,
				    alt: alt,
				    width: width,
				    height: height
				};

				image_info_ser = var_serialise(image_info);
				populateImageInfo(image_info_ser);
				//setUrl('', './?a=' + assetid);
			}

		// --></script>
	</body>
</html>
