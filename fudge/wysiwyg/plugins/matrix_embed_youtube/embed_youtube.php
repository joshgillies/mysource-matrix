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
* $Id: embed_youtube.php,v 1.5 2013/04/23 08:07:28 cupreti Exp $
*
*/

/**
* Embed YouTube Popup for the WYSIWYG
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/


require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

if (!isset($_GET['f_fileid'])) $_GET['f_fileid'] = 0;
?>
<!DOCTYPE html>
<html style="height: 600px;">
	<head>
		<title>Embed YouTube</title>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
	    <link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

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
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/tooltip.js' ?>"></script>

		<script type="text/javascript">

			function Init() {
				__dlg_init("matrixEmbedYouTube");
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["f_width", "f_height", "f_colour1", "f_colour2"];
				var chk_fields = ["f_auto_start", "f_loop", "f_full_screen", "f_egm", "f_rel", "f_show_border", "f_enable_js"];
				var param = new Object();

				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}
				param["f_vid"] = document.getElementById('video_id').value;
				param["f_video_url"] = document.getElementById('video_url').value;
				for (var i in chk_fields) {
					var id = chk_fields[i];
					var el = document.getElementById(id);
					if (el.checked) {
						param[id] = "1";
					} else {
						param[id] = "0";
					}
				}
				__dlg_close("matrixEmbedYouTube", param);

				return false;
			};

			function onCancel() {
				__dlg_close("matrixEmbedYouTube", null);

				return false;
			};
		</script>


	</head>

	<body onload="Javascript: Init();">
		<form action="" method="get" name="main_form" id="main-form">
			<table class="sq-fieldsets-table">
				<tr>
					<td valign="top">
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" colspan="2">
									<fieldset>
										<legend><b><?php echo translate('General'); ?></b></legend>
										<table width="100%" cellspacing="0" cellpadding="0">
											<tr>
												<td valign="top" width="100%">
													<table>
														<tr>
															<td class="label">Video ID:</td>
															<td><?php text_box('video_id', (!empty($_REQUEST['f_vid']))?$_REQUEST['f_vid']:'', 20, 0)?></td>
														</tr>
														<tr>								
															<td class="label">Or <?php echo translate('URL'); ?>:</td>
															<td><?php text_box('video_url', (!empty($_REQUEST['f_video_url']))?$_REQUEST['f_video_url']:'', 75, 0)?></td>
														</tr>
													</table>
												</td>
											</tr>
										</table>

									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="50%" rowspan="2">
									<fieldset>
										<legend><b><?php echo translate('Controls'); ?></b></legend>
										<table style="width:100%">
											<tr>
												<!-- autoplay -->
												<td class="label"><?php echo translate('Auto Start'); ?>:</td>
												<td width="50%">
													<input type="checkbox" name="auto_start" id="f_auto_start" value="1" <?php echo ($_REQUEST['f_auto_start'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<!-- loop -->
												<td class="label"><?php echo translate('Loop'); ?>:</td>
												<td>
													<input type="checkbox" name="loop" id="f_loop" value="1" <?php echo ($_REQUEST['f_loop'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<!-- full screen -->
												<td class="label">Full Screen:</td>
												<td>
													<input type="checkbox" name="full_screen" id="f_full_screen" value="1" <?php echo ($_REQUEST['f_full_screen'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<!-- related -->
												<td class="label">Show Related Videos:</td>
												<td>
													<input type="checkbox" name="rel" id="f_rel" value="1" <?php echo ($_REQUEST['f_rel'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<!-- genius bar -->
												<td class="label">Enable Genius Bar:</td>
												<td>
													<input type="checkbox" name="egm" id="f_egm" value="1" <?php echo ($_REQUEST['f_egm'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<!-- enable js api -->
												<td class="label">Enable Javascript API:</td>
												<td>
													<input type="checkbox" name="enable_js" id="f_enable_js" value="1" <?php echo ($_REQUEST['f_enable_js'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
								<td valign="top" width="50%">
									<fieldset>
										<legend><b><?php echo translate('Size'); ?></b></legend>
										<table style="width:100%">
											<tr>
												<td class="label" width="50%"><?php echo translate('Width'); ?>:</td>
												<td>
												<input type="text" name="width" id="f_width" size="5" title="Width" value="<?php echo empty($_REQUEST['f_width']) ? '480' : htmlspecialchars($_REQUEST['f_width']) ?>" />
												</td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('Height'); ?>:</td>
												<td>
												<input type="text" name="height" id="f_height" size="5" title="Height" value="<?php echo empty($_REQUEST['f_height']) ? '385' : htmlspecialchars($_REQUEST['f_height']) ?>" />
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="50%">
									<fieldset>
										<legend><b><?php echo translate('Style'); ?></b></legend>
										<table style="width:100%">
											<tr>
												<td class="label" width="50%"><?php echo translate('Border'); ?>:</td>
												<td>
												<input type="checkbox" name="show_border" id="f_show_border" size="5" title="Border" value="1" <?php echo ($_REQUEST['f_show_border'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('Primary Border Colour'); ?>:</td>
												<td>
												<input type="text" name="colour1" id="f_colour1" size="8" title="Colour 1" value="<?php echo empty($_REQUEST['f_colour1']) ? '' : htmlspecialchars($_REQUEST['f_colour1']) ?>" />
												</td>
											</tr>
											<tr>
												<td class="label"><?php echo translate('Secondary Border Colour'); ?>:</td>
												<td>
												<input type="text" name="colour2" id="f_colour2" size="8" title="Colour 2" value="<?php echo empty($_REQUEST['f_colour2']) ? '' : htmlspecialchars($_REQUEST['f_colour2']) ?>" />
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td>
									<button type="button" name="cancel" onclick="return onCancel();" class="sq-popup-btn-cancel"><?php echo translate('Cancel'); ?></button>	
								</td>
								<td>										
									<div class="sq-popup-button-wrapper">																		    								
										<button type="button" name="ok" onclick="return onOK();" class="sq-btn-green"><?php echo translate('OK'); ?></button>	
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
