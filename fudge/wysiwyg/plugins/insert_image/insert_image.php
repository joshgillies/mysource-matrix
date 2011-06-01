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
* $Id: insert_image.php,v 1.10 2006/12/06 05:11:09 bcaldwell Exp $
*
*/

/**
* WYSIWYG Plugin - Insert Image Pop-up
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package Fudge
* @subpackage wysiwyg
*/
?>
<html>
	<head>
		<title>Insert Image</title>

		<script type="text/javascript" src="../../core/popup.js"></script>

		<script type="text/javascript">
			var preview_window = null;

			function Init() {
				__dlg_init("insertImage");
				document.getElementById("f_url").focus();
			};

			function onOK() {
				var required = {
					"f_url": "You must enter the URL",
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
				var fields = ["f_url", "f_alt", "f_align", "f_border",
							"f_horiz", "f_vert"];
				var param = new Object();
				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}
				if (preview_window) {
					preview_window.close();
				}
				__dlg_close("insertImage", param);
				return false;
			};

			function onCancel() {
				if (preview_window) {
					preview_window.close();
				}
				__dlg_close("insertImage", null);
				return false;
			};

			function onPreview() {
				alert(js_translate('preview_needs_rewritten'));
				var f_url = document.getElementById("f_url");
				var url = f_url.value;
				if (!url) {
					alert("You have to enter an URL first");
					f_url.focus();
					return false;
				}
				var img = new Image();
				img.src = url;
				var win = null;
				if (!document.all) {
					win = window.open("about:blank", "ha_imgpreview", "toolbar=no,menubar=no,personalbar=no,innerWidth=100,innerHeight=100,scrollbars=no,resizable=yes");
				} else {
					win = window.open("about:blank", "ha_imgpreview", "channelmode=no,directories=no,height=100,width=100,location=no,menubar=no,resizable=yes,scrollbars=no,toolbar=no");
				}
				preview_window = win;
				var doc = win.document;
				var body = doc.body;
				if (body) {
					body.innerHTML = "";
					body.style.padding = "0px";
					body.style.margin = "0px";
					var el = doc.createElement("img");
					el.src = url;

					var table = doc.createElement("table");
					body.appendChild(table);
					table.style.width = "100%";
					table.style.height = "100%";
					var tbody = doc.createElement("tbody");
					table.appendChild(tbody);
					var tr = doc.createElement("tr");
					tbody.appendChild(tr);
					var td = doc.createElement("td");
					tr.appendChild(td);
					td.style.textAlign = "center";

					td.appendChild(el);
					win.resizeTo(el.offsetWidth + 30, el.offsetHeight + 30);
				}
				win.focus();
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

	<body onload="Init()">
		<div class="title"><?php echo translate('insert_image'); ?></div>
		<form action="" method="get">
			<table border="0" width="100%" style="padding: 0px; margin: 0px">
				<tbody>
					<tr>
						<td style="width: 7em; text-align: right"><?php echo translate('image_url'); ?>:</td>
						<td>
							<input type="text" name="url" id="f_url" style="width:75%" title="Enter the image URL here" value="<?php echo $_REQUEST['f_url']?>" />
							<button name="preview" onclick="return onPreview();" title="Preview the image in a new window"><?php echo translate('preview'); ?></button>
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
				<legend><?php echo translate('layout'); ?></legend>

				<div class="space"></div>

				<div class="fl"><?php echo translate('alignment'); ?>:</div>
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

				<div class="fl"><?php echo translate('border_thickness'); ?>:</div>
				<input type="text" name="border" id="f_border" size="5"
				title="Leave empty for no border" value="<?php echo $_REQUEST['f_border']?>" />

				<div class="space"></div>
			</fieldset>

			<fieldset style="float:right; margin-right: 5px;">
				<legend><?php echo translate('spacing'); ?></legend>

				<div class="space"></div>

				<div class="fr"><?php echo translate('horizontal'); ?>:</div>
				<input type="text" name="horiz" id="f_horiz" size="5" title="Horizontal padding" value="<?php echo $_REQUEST['f_horiz']?>" />

				<p />

				<div class="fr"><?php echo translate('vertical'); ?>:</div>
				<input type="text" name="vert" id="f_vert" size="5" title="Vertical padding" value="<?php echo $_REQUEST['f_vert']?>" />

				<div class="space"></div>
			</fieldset>

			<div style="margin-top: 85px; text-align: right;">
			<hr />
			<button type="button" name="ok" onclick="return onOK();"><?php echo translate('ok'); ?></button>
			<button type="button" name="cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
			</div>
		</form>
	</body>
</html>
