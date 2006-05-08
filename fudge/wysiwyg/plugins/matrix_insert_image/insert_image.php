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
* $Id: insert_image.php,v 1.41 2006/05/08 00:10:49 lwright Exp $
*
*/

/**
* Insert Image Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.41 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

$url_protocol_options = Array(
							''			=> '',
							'http://'	=> 'http://',
							'https://'	=> 'https://',
						);

if (!isset($_GET['f_imageid'])) $_GET['f_imageid'] = 0;
?>

<html style="width: 740px; height: 580px;">
	<head>
		<title>Insert Image</title>

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

		<script type="text/javascript">

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
				var fields = ["f_alt", "f_align", "f_border",
							"f_horiz", "f_vert", "f_width", "f_height"];
				var param = new Object();
				for (var i in fields) {
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
				var url = '<?php echo sq_web_path('root_url').'/'.SQ_CONF_BACKEND_SUFFIX; ?>/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=' + escape(assetid) + '&asset_ei_screen=image_info&ignore_frames=1&t=' + Math.random() * 1000;
				JsHttpConnector.submitRequest(url, populateImageInfo);
			};

			function doStatus(img) {
				//on Image Load
			}

		function newImg(div, url, width, height) {
			var limit = 160;
			var scalar = 1;
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

			img.width = width * scalar;
			img.height = height * scalar;

			div.appendChild(img);

		}


			function setImagePreview() {
				if (document.getElementById("f_imageid[assetid]").value == "" || document.getElementById("f_imageid[assetid]").value == 0) {
					return;
				}
				newImg(document.getElementById('image_container'), '<?php echo sq_web_path('root_url'); ?>' + '/?a=' + document.getElementById("f_imageid[assetid]").value);
			}

		</script>

		<style type="text/css">
			html, body {
				background: #FCFCFC;
				color: #000000;
				font: 11px Tahoma,Verdana,sans-serif;
				margin: 0px;
				padding: 0px;
				padding: 5px;
			}

			table {
				font: 11px Tahoma,Verdana,sans-serif;
			}

			/* main popup title */
			.title {
				background: #402F48;
				color: #FFFFFF;
				font-weight: bold;
				font-size: 120%;
				padding: 3px 10px;
				margin-bottom: 10px;
				border-bottom: 1px solid black;
				letter-spacing: 4px;
			}

			/* fieldset styles */
			fieldset {
				padding: 0px 10px 5px 5px;
				border-color: #725B7D;
			}

			.prev {
				padding: 0px 10px 5px 5px;
				border-color: #725B7D;
				margin-right: 3px;
				height: 200px;
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

			.preview {
				height: 120px;
				width: 160px;
			}

			.buttonColor-nocolor, .buttonColor-nocolor-hilite { padding: 0px; }
			.buttonColor-nocolor-hilite { background: #402F48; color: #FFFFFF; }
		</style>
	</head>

	<body onload="Init();" onUnload="asset_finder_onunload();">

		<div class="title"><?php echo translate('insert_image'); ?></div>
		<form action="" method="get" name="main_form">
			<table width="100%">
				<tr>
					<td valign="top">
						<?php
							include_once(SQ_LIB_PATH.'/asset_map/asset_map.inc');
							$asset_map = new Asset_Map();
							$asset_map->embedAssetMap('simple', 200, 350);
						?>
					</td>
					<td valign="top">
						<table cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%" colspan=2>
								<fieldset>
									<legend><b><?php echo translate('general'); ?></b></legend>
										<table style="width:100%">
											<tr>
												<td class="label" nowrap="nowrap"><?php echo translate('image_url'); ?>:</td>
												<td>
													<?php asset_finder('f_imageid', (assert_valid_assetid($_GET['f_imageid'], '', TRUE, FALSE) ? $_GET['f_imageid'] : 0), Array('image' => 'D', 'image_variety' => 'D'), '', false, 'setImageInfo'); ?>
												</td>
											</tr>
											<tr>
												<td class="label" nowrap="nowrap"><?php echo translate('alternate_text'); ?>:</td>
												<td>
													<input type="text" name="alt" id="f_alt" style="width:100%" title="For browsers that don't support images" value="<?php echo $_REQUEST['f_alt']?>" />
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
									<table style="width:100%">
										<tr>
											<td valign="top" class="label" nowrap="nowrap"><?php echo translate('longdesc_text'); ?>:</td>
											<td>
												<?php
												if (!empty($_GET['f_longdesc'])) {
													if (preg_match("/^\d+$/", $_GET['f_longdesc']) && $_GET['f_longdesc'] != "0") {?>
														Enter URL manually<br />
														<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, '', 'style="font-family: courier new; font-size: 11px;"'); ?>
														<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', '', 40, 0)?>
														<br /><br />or choose a Standard Page asset<br />
														<?php asset_finder('f_longdesc', $_GET['f_longdesc'], Array('page_standard' => 'D'), ''); ?>
														<br /><br />If you enter URL manually, current asset in the asset finder must be cleared .<br />
													<?php
													} else {
														$matches = Array();
														if (preg_match_all('/^(http:\/\/|https:\/\/){1}(.*)$/', $_GET['f_longdesc'], $matches) === FALSE ||
															empty($matches[0])) {?>
															Enter URL manually<br />
															<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, FALSE, '', 'style="font-family: courier new; font-size: 11px;"'); ?>
															<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', $_GET['f_longdesc'], 40, 0)?>
															<br /><br />or choose a Standard Page asset<br />
															<?php asset_finder('f_longdesc', '0', Array('page_standard' => 'D'), ''); ?>
															<br /><br />If you enter URL manually, current asset in the asset finder must be cleared .<br />
														<?php
														} else {?>
															Enter URL manually<br />
															<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, FALSE, $matches[1][0], 'style="font-family: courier new; font-size: 11px;"'); ?>
															<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', $matches[2][0], 40, 0)?>
															<br /><br />or choose a Standard Page asset<br />
															<?php asset_finder('f_longdesc', '0', Array('page_standard' => 'D'), ''); ?>
															<br /><br />If you enter URL manually, current asset in the asset finder must be cleared .<br />
														<?php
														}

													}
												} else {?>
													Enter URL manually<br />
													<?php echo translate('protocol'); ?>&nbsp;<?php combo_box('f_longdesc_protocol', $url_protocol_options, '', 'style="font-family: courier new; font-size: 11px;"'); ?>
													<?php echo translate('link'); ?>&nbsp;<?php text_box('f_longdesc_link', '', 40, 0)?>
													<br /><br />or choose a Standard Page asset<br />
													<?php asset_finder('f_longdesc', '0', Array('page_standard' => 'D'), ''); ?>
													<br /><br />If you enter URL manually, current asset in the asset finder must be cleared .<br />
												<?php
												}?>
											</td>
										</tr>
									</table>
								</fieldset>
							</tr>
							<tr>
								<td valign="center" align="center" rowspan=2 width="50%">
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
												<input type="text" name="border" id="f_border" size="5" title="Leave empty for no border" value="<?php echo $_REQUEST['f_border']?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('horizontal'); ?>:</td>
											<td>
												<input type="text" name="horiz" id="f_horiz" size="5" title="Horizontal padding" value="<?php echo $_REQUEST['f_horiz']?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('vertical'); ?>:</td>
											<td>
												<input type="text" name="vert" id="f_vert" size="5" title="Vertical padding" value="<?php echo $_REQUEST['f_vert']?>" />
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
									<table style="width:100%">
										<tr>
											<td class="label" width="30%"><?php echo translate('width'); ?>:</td>
											<td>
												<input type="text" name="width" id="f_width" size="5" title="Width" value="<?php echo $_REQUEST['f_width']?>" />
											</td>
										</tr>
										<tr>
											<td class="label"><?php echo translate('height'); ?>:</td>
											<td>
												<input type="text" name="height" id="f_height" size="5" title="Height" value="<?php echo $_REQUEST['f_height']?>" />
											</td>
										</tr>
									</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td colspan=2>
									<div style="margin-top: 5px; text-align: right;">
										<hr />
										<button type="button" name="ok" onclick="return onOK();"><?php echo translate('ok'); ?></button>
										<button type="button" name="cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</form>
	</body>
</html>
