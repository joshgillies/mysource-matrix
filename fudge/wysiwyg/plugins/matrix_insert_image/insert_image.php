<?php
require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

if (!isset($_GET['f_imageid'])) $_GET['f_imageid'] = 0;
?>

<html style="width: 398; height: 218">
	<head>
		<title>Insert Image</title>

		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">
			
			window.opener.top.main.document.body.onclick = function() {
				setTimeout('self.focus()',100);
			}

			function getFocus() {
				setTimeout('self.focus()',100);
			}

			function Init() {
				__dlg_init();
			};

			function onOK() {
				var required = {
					//"f_url": "You must enter the URL",
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
				var fields = ["f_imageid", "f_alt", "f_align", "f_border",
							"f_horiz", "f_vert"];
				var param = new Object();
				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}
				__dlg_close(param);
				return false;
			};

			function onCancel() {
				__dlg_close(null);
				return false;
			};
		</script>

		<style type="text/css">
			html, body {
				background: ButtonFace;
				color: ButtonText;
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

	<body onLoad="Init(); if (opener) opener.blockEvents()" onUnload="if (opener) opener.unblockEvents(); asset_finder_onunload()">
		<div class="title">Insert Image</div>
		<form action="" method="get" name="main_form">
			<table border="0" width="100%" style="padding: 0px; margin: 0px">
				<tbody>
					<tr>
						<td style="width: 7em; text-align: right">Image URL:</td>
						<td>
							<?php asset_finder('f_imageid', $_GET['f_imageid'], Array('image' => 'D'), 'window.opener.top', 'getFocus'); ?>
						</td>
					</tr>
					<tr>
						<td style="width: 7em; text-align: right">Alternate text:</td>
						<td><input type="text" name="alt" id="f_alt" style="width:100%" title="For browsers that don't support images" value="<?php echo $_REQUEST['f_alt']?>" /></td>
					</tr>
				</tbody>
			</table>

			<p />

			<fieldset style="float: left; margin-left: 5px;">
				<legend>Layout</legend>

				<div class="space"></div>

				<div class="fl">Alignment:</div>
				<select size="1" name="align" id="f_align" title="Positioning of this image">
					<?php
					if (!isset($_REQUEST['f_align'])) $_REQUEST['f_align'] = 'baseline';
					$options_array = Array(	'' => 'Not set',
											'left' => 'Left',
											'right' => 'Right',
											'texttop' => 'Texttop',
											'absmiddle' => 'Absmiddle',
											'baseline' => 'Baseline',
											'absbottom' => 'Absbottom',
											'bottom' => 'Bottom',
											'middle' => 'Middle',
											'top' => 'Top'
										  );
					foreach ($options_array as $value => $text) {
						?><option value="<?php echo $value?>" <?php echo ($_REQUEST['f_align'] == $value) ? 'selected="1"' : ''?>><?php echo $text?></option><?php
					}
					?>
				</select>

				<p />

				<div class="fl">Border thickness:</div>
				<input type="text" name="border" id="f_border" size="5"
				title="Leave empty for no border" value="<?php echo $_REQUEST['f_border']?>" />

				<div class="space"></div>
			</fieldset>

			<fieldset style="float:right; margin-right: 5px;">
				<legend>Spacing</legend>

				<div class="space"></div>

				<div class="fr">Horizontal:</div>
				<input type="text" name="horiz" id="f_horiz" size="5" title="Horizontal padding" value="<?php echo $_REQUEST['f_horiz']?>" />

				<p />

				<div class="fr">Vertical:</div>
				<input type="text" name="vert" id="f_vert" size="5" title="Vertical padding" value="<?php echo $_REQUEST['f_vert']?>" />

				<div class="space"></div>
			</fieldset>

			<div style="margin-top: 85px; text-align: right;">
			<hr />
			<button type="button" name="ok" onclick="return onOK();">OK</button>
			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			</div>
		</form>
	</body>
</html>
