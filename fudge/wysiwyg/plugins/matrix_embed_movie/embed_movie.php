<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: embed_movie.php,v 1.5 2003/11/05 01:36:53 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

if (!isset($_GET['f_fileid'])) $_GET['f_fileid'] = 0;
?>

<html>
	<head>
		<title>Embed Movie</title>

		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">
			var parent_object = opener.editor_<?php echo $_REQUEST['editor_name']?>._object;
			
			window.opener.onFocus = function() { getFocus(); }
			parent_object.onFocus = function() { getFocus(); }

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("matrixEmbedMovie");
				var changeButton = document.getElementById('sq_asset_finder_f_fileid_change_btn');
				changeButton.click();
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["f_fileid", "f_width", "f_height"];
				var chk_fields = ["f_show_controls", "f_auto_start", "f_embed_loop"];
				var param = new Object();

				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}

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
				window.opener.focus();
				return false;
			};

			function onCancel() {
				__dlg_close("matrixEmbedMovie", null);
				window.opener.focus();
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

	<body onLoad="Init(); if (opener) opener.blockEvents('matrixEmbedMovie')" onUnload="if (opener) opener.unblockEvents(); asset_finder_onunload(); parent_object._tmp['disable_toolbar'] = false; parent_object.updateToolbar();">
		
		<div class="title">Embed Movie</div>
		
		<form action="" method="get" name="main_form">
			<table width="100%">
				<tr>
					<td>
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%">
									<fieldset>
									<legend><b>General</b></legend>
									<table style="width:100%">
										<tr>
											<td class="label">Movie URL:</td>
											<td>
											<?php asset_finder('f_fileid', $_GET['f_fileid'], Array('file' => 'I'), (($_GET['in_popup']) ? 'opener.opener.top' : 'opener.top'), 'getFocus'); ?>
											</td>
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
								<td valign="top" width="50%">
									<fieldset>
										<legend>Controls</legend>
										<table style="width:100%">
											<tr>
												<td class="label" colspan="2"><b>wmv, asf & asx only</b></td>
											</tr>
											<tr>
												<td class="label">Auto Start:</td>
												<td>
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
								<td>&nbsp;</td>
								<td valign="top" width="50%">
									<fieldset>
										<legend>Size</legend>
										<table style="width:100%">
											<tr>
												<td class="label">Width:</td>
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
