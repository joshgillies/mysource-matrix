<?php
require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

if (!isset($_GET['f_fileid'])) $_GET['f_fileid'] = 0;
?>

<html>
	<head>
		<title>Insert Image</title>

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
				return false;
			};

			function onCancel() {
				__dlg_close("matrixEmbedMovie", null);
				return false;
			};
		</script>

		<style type="text/css">
			html, body {
				background: #F0F0F0;
				color: #000000;
				font: 11px Tahoma,Verdana,sans-serif;
				margin: 0px;
				padding: 0px;
			}
			body { padding: 5px; }
			table {
				font: 11px Tahoma,Verdana,sans-serif;
			}
			form p {
				margin-top: 5px;
				margin-bottom: 5px;
			}
			.fl { width: 9em; float: left; padding: 2px 5px; text-align: right; }
			.fr { width: 6em; float: left; padding: 2px 5px; text-align: right; }
			fieldset { padding: 0px 10px 5px 5px; }
			select, input, button { font: 11px Tahoma,Verdana,sans-serif; }
			button { width: 70px; }
			.space { padding: 2px; }

			.title { background: #ddf; color: #000; font-weight: bold; font-size: 120%; padding: 3px 10px; margin-bottom: 10px;
			border-bottom: 1px solid black; letter-spacing: 2px;
			}
			form { padding: 0px; margin: 0px; }
		</style>
	</head>

	<body onLoad="Init(); if (opener) opener.blockEvents('matrixEmbedMovie')" onUnload="if (opener) opener.unblockEvents(); asset_finder_onunload(); parent_object._tmp['disable_toolbar'] = false; parent_object.updateToolbar();">
		<div class="title">Embed Movie</div>
		<form action="" method="get" name="main_form">
			<table border="0" width="100%" style="padding: 0px; margin: 0px">
				<tbody>
					<tr>
						<td style="width: 7em; text-align: right">Image URL:</td>
						<td>
							<?php asset_finder('f_fileid', $_GET['f_fileid'], Array('file' => 'I'), 'window.opener.top', 'getFocus'); ?>
						</td>
					</tr>
				</tbody>
			</table>
			<p />

			<fieldset style="float:left; margin-left: 5px;">
				<legend>Controls</legend>

				<div class="space"></div>

				<div class="space" align="left"><b>wmv, asf & asx only</b></div>
				<div class="fl">Auto Start:</div>
				<input type="checkbox" name="auto_start" id="f_auto_start" value="1" <?php echo ($_REQUEST['f_auto_start'] == '1') ? 'checked' : ''?> />

				<p />

				<div class="fl">Loop:</div>
				<input type="checkbox" name="embed_loop" id="f_embed_loop" value="1" <?php echo ($_REQUEST['f_embed_loop'] == '1') ? 'checked' : ''?> />

				<p />

				<div class="space" align="left"><b>mov, wmv, asf & asx only</b></div>
				<div class="fl">Show Controls:</div>
				<input type="checkbox" name="show_controls" id="f_show_controls" value="1" <?php echo ($_REQUEST['f_show_controls'] == '1') ? 'checked' : ''?> />

				<p />

				<div class="space"></div>
			</fieldset>

			<fieldset style="float:right; margin-right: 5px;">
				<legend>Embedded Size</legend>

				<div class="space"></div>

				<div class="fr">Width:</div>
				<input type="text" name="width" id="f_width" size="5" title="Width" value="<?php echo $_REQUEST['f_width']?>" />

				<p />

				<div class="fr">Height:</div>
				<input type="text" name="height" id="f_height" size="5" title="Height" value="<?php echo $_REQUEST['f_height']?>" />

				<div class="space"></div>
			</fieldset>

			<div style="margin-top: 145px; text-align: right;">
			<hr />
			<button type="button" name="ok" onclick="return onOK();">OK</button>
			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			</div>
		</form>
	</body>
</html>
