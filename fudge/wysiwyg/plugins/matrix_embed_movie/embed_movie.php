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
* $Id: embed_movie.php,v 1.11 2004/01/16 01:06:48 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

if (!isset($_GET['f_fileid'])) $_GET['f_fileid'] = 0;
?>

<html style="width: 600px; height: 480px;">
	<head>
		<title>Embed Movie</title>

		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/javaExternalCall.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">

			function Init() {
				__dlg_init("matrixEmbedMovie");
				var changeButton = document.getElementById('sq_asset_finder_f_fileid_change_btn');
				changeButton.click();
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["f_width", "f_height"];
				var chk_fields = ["f_show_controls", "f_auto_start", "f_embed_loop"];
				var param = new Object();

				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}
				// Because the id of the f_image field has array references in it, 
				// we can't get use getElementById, so do this...
				param["f_fileid"] = document.main_form.elements["f_fileid[assetid]"].value;

				for (var i in chk_fields) {
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
		</style>
	</head>

	<body onload="Javascript: Init();" onUnload="Javascript: asset_finder_onunload();">
		
		<div class="title">Embed Movie</div>
		
		<form action="" method="get" name="main_form">
			<table width="100%">
				<tr>
					<td valign="top">
						<?php
							include_once(SQ_LIB_PATH.'/asset_map/asset_map.inc');
							$asset_map = new Asset_Map();
							$asset_map->embed_asset_map('simple');
						?>
					</td>
					<td valign="top">
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%">
									<fieldset>
										<legend><b>General</b></legend>
										<table style="width:100%">
											<tr>
												<td class="label">Movie URL:</td>
												<td>
													<?php asset_finder('f_fileid', $_GET['f_fileid'], Array('file' => 'I'), ''); ?>
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="50%">
									<fieldset>
										<legend>Controls</legend>
										<table style="width:100%">
											<tr>
												<td class="label" colspan="2"><b>wmv, asf & asx only</b></td>
											</tr>
											<tr>
												<td class="label">Auto Start:</td>
												<td width="50%">
													<input type="checkbox" name="auto_start" id="f_auto_start" value="1" <?php echo ($_REQUEST['f_auto_start'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<td class="label">Loop:</td>
												<td>
													<input type="checkbox" name="embed_loop" id="f_embed_loop" value="1" <?php echo ($_REQUEST['f_embed_loop'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
											<tr>
												<td class="label" colspan="2"><b>mov, wmv, asf & asx only</b></td>
											</tr>
											<tr>
												<td class="label">Show Controls:</td>
												<td>
													<input type="checkbox" name="show_controls" id="f_show_controls" value="1" <?php echo ($_REQUEST['f_show_controls'] == '1') ? 'checked' : ''?> />
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="50%">
									<fieldset>
										<legend>Size</legend>
										<table style="width:100%">
											<tr>
												<td class="label" width="50%">Width:</td>
												<td>
													<input type="text" name="width" id="f_width" size="5" title="Width" value="<?php echo $_REQUEST['f_width']?>" />
												</td>
											</tr>
											<tr>
												<td class="label">Height:</td>
												<td>
													<input type="text" name="height" id="f_height" size="5" title="Height" value="<?php echo $_REQUEST['f_height']?>" />
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
			<button type="button" name="ok" onclick="return onOK();">OK</button>
			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			</div>
		</form>
	</body>
</html>