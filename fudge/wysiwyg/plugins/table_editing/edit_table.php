<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: edit_table.php,v 1.3 2003/09/26 05:26:38 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once dirname(__FILE__).'/../../wysiwyg_plugin.inc';
$wysiwyg = null;
$plugin = new wysiwyg_plugin($wysiwyg);
?>

<html style="width:360px; height:380px;">

	<head>
		<title>Edit Table Properties</title>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="../../core/dialog.js"></script>

		<script type="text/javascript">
			function colorPopup(id) {
				var field = document.getElementById(id);
				var span = document.getElementById(id + "_chooser");
				var color = field.value;

				var strPage = "<?php echo $_SERVER['PHP_SELF'].'/../../'.$plugin->get_popup_href('select_color.html', 'select_color')?>";

				openModalDialog("selectColor", strPage, 238, 163, function(color) {
					if (color) {
						span.style.backgroundColor = "#" + color;
						field.value = color;
					}
				}, color);
			};

			function nullColor(id) {
				var field = document.getElementById(id);
				var span = document.getElementById(id + "_chooser");
				span.style.backgroundColor = "";
				field.value = "";
			};

			function colorButton(id, classname) {
				var btn = document.getElementById(id + "_button");
				btn.className = classname;
			};

			function Init() {
				__dlg_init("editTableProperties");
			};

			function onOK() {
				var fields = ["f_width", "f_height", "f_widthUnit",
							  "f_heightUnit", "f_spacing", "f_padding",
							  "f_borders", "f_frames", "f_rules",
							  "f_bgcolor", "f_color", "f_borderColor",
							  "f_borderWidth", "f_borderStyle"];
				var params = new Object();
				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					params[id] = el.value;
				}
				__dlg_close("editTableProperties", params);
				return false;
			};

			function onCancel() {
				__dlg_close("editTableProperties", null);
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

	<body onLoad="Init();">

		<div class="title">Table Properties</div>

		<form action="" method="get">
			<table width="100%">
				<tr>
					<td>
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="50%">
									<fieldset>
									<legend><b>Dimensions</b></legend>
									<table style="width:100%">
										<tr>
											<td class="label">Width:</td>
											<td>
											<input type="text" id="f_width" size="5" value="<?php echo $_GET['f_width']; ?>" />
											<select id="f_widthUnit">
												<option value="px" <?php echo ($_GET['f_widthUnit'] == 'px') ? 'selected' : ''; ?>>px</option>
												<option value="%"  <?php echo ($_GET['f_widthUnit'] == '%')  ? 'selected' : ''; ?>>%</option>
											</select>
											</td>
										</tr>
										<tr>
											<td class="label">Height:</td>
											<td>
											<input type="text" id="f_height" size="5" value="<?php echo $_GET['f_height']; ?>" />
											<select id="f_heightUnit">
												<option value="px" <?php echo ($_GET['f_heightUnit'] == 'px') ? 'selected' : ''; ?>>px</option>
												<option value="%"  <?php echo ($_GET['f_heightUnit'] == '%')  ? 'selected' : ''; ?>>%</option>
											</select>
											</td>
										</tr>
									</table>
									</fieldset>
								</td>
								<td>&nbsp;</td>
								<td valign="top" width="50%">
									<fieldset>
									<legend><b>Spacing and Padding</b></legend>
									<table style="width:100%">
										<tr>
											<td class="label">Spacing:</td>
											<td><input type="text" id="f_spacing" size="5" value="<?php echo $_GET['f_spacing']; ?>" /> px</td>
										</tr>
										<tr>
											<td class="label">Padding:</td>
											<td><input type="text" id="f_padding" size="5" value="<?php echo $_GET['f_padding']; ?>" /> px</td>
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
						<fieldset>
						<legend><b>Frames and Borders</b></legend>
						<table width="100%">
							<tr>
								<td class="label">Borders:</td>
								<td><input id="f_borders" type="text" size="5" value="<?php echo $_GET['f_borders']; ?>" /> px</td>
							</tr>
							<tr>
								<td class="label">Frames:</td>
								<td>
								<select id="f_frames">
									<option value=""></option>
									<option value="void"   <?php echo ($_GET['f_frames'] == 'void')   ? 'selected' : ''; ?>>No Sides</option>
									<option value="above"  <?php echo ($_GET['f_frames'] == 'above')  ? 'selected' : ''; ?>>The top side only</option>
									<option value="below"  <?php echo ($_GET['f_frames'] == 'below')  ? 'selected' : ''; ?>>The bottom side only</option>
									<option value="hsides" <?php echo ($_GET['f_frames'] == 'hsides') ? 'selected' : ''; ?>>The top and bottom sides only</option>
									<option value="vsides" <?php echo ($_GET['f_frames'] == 'vsides') ? 'selected' : ''; ?>>The right and left sides only</option>
									<option value="lhs"    <?php echo ($_GET['f_frames'] == 'lhs')    ? 'selected' : ''; ?>>The left-hand side only</option>
									<option value="rhs"    <?php echo ($_GET['f_frames'] == 'rhs')    ? 'selected' : ''; ?>>The right-hand side only</option>
									<option value="box"    <?php echo ($_GET['f_frames'] == 'box')    ? 'selected' : ''; ?>>All four sides</option>
								</select>
								</td>
							</tr>
							<tr>
								<td class="label">Rules:</td>
								<td>
								<select id="f_rules">
									<option value=""></option>
									<option value="none" <?php echo ($_GET['f_rules'] == 'none') ? 'selected' : ''; ?>>No rules</option>
									<option value="rows" <?php echo ($_GET['f_rules'] == 'rows') ? 'selected' : ''; ?>>Rules will appear between rows only</option>
									<option value="cols" <?php echo ($_GET['f_rules'] == 'cols') ? 'selected' : ''; ?>>Rules will appear between columns only</option>
									<option value="all"  <?php echo ($_GET['f_rules'] == 'all')  ? 'selected' : ''; ?>>Rules will appear between all rows and columns</option>
								</select>
								</td>
							</tr>
						</table>
						</fieldset>
					</td>
				</tr>
				<tr>
					<td width="100%">
						<fieldset>
						<legend><b>Table Styles / Colours</b></legend>
							<table width="100%">
								<tr>
									<td class="label">Background:</td>
									<td width="100%">
									<input type="hidden" id="f_bgcolor" value="<?php echo $_GET['f_bgcolor']; ?>">
									<span class="buttonColor" id="f_bgcolor_button"><span class="buttonColor-chooser" id="f_bgcolor_chooser" style="background-color: #<?php echo $_GET['f_bgcolor']; ?>" onClick="Javascript: colorPopup('f_bgcolor');" onMouseOver="Javascript: colorButton('f_bgcolor', 'buttonColor-hilite');" onMouseOut="Javascript: colorButton('f_bgcolor', 'buttonColor');"></span><span title="Unset Colour" class="buttonColor-nocolor" id="f_bgcolor_unset" style="background-color: #<?php echo $_GET['f_bgcolor']; ?>" onClick="Javascript: nullColor('f_bgcolor');" onMouseOver="Javascript: colorButton('f_bgcolor', 'buttonColor-hilite'); this.className = 'buttonColor-nocolor-hilite';" onMouseOut="Javascript: colorButton('f_bgcolor', 'buttonColor'); this.className = 'buttonColor-nocolor';">&#x00d7;</span></span>
									</td>
								</tr>
								<tr>
									<td class="label">Foreground:</td>
									<td width="100%">
									<input type="hidden" id="f_color" value="<?php echo $_GET['f_color']; ?>">
									<span class="buttonColor" id="f_color_button"><span class="buttonColor-chooser" id="f_color_chooser" style="background-color: #<?php echo $_GET['f_color']; ?>" onClick="Javascript: colorPopup('f_color');" onMouseOver="Javascript: colorButton('f_color', 'buttonColor-hilite');" onMouseOut="Javascript: colorButton('f_color', 'buttonColor');"></span><span title="Unset Colour" class="buttonColor-nocolor" id="f_color_unset" style="background-color: #<?php echo $_GET['f_color']; ?>" onClick="Javascript: nullColor('f_color');" onMouseOver="Javascript: colorButton('f_color', 'buttonColor-hilite'); this.className = 'buttonColor-nocolor-hilite';" onMouseOut="Javascript: colorButton('f_color', 'buttonColor'); this.className = 'buttonColor-nocolor';">&#x00d7;</span></span>
									</td>
								</tr>
								<tr>
									<td class="label" valign="top">Border:</td>
									<td width="100%" valign="top">
									<input type="hidden" id="f_borderColor" value="<?php echo $_GET['f_borderColor']; ?>">
									<span class="buttonColor" id="f_borderColor_button"><span class="buttonColor-chooser" id="f_borderColor_chooser" style="background-color: #<?php echo $_GET['f_borderColor']; ?>" onClick="Javascript: colorPopup('f_borderColor');" onMouseOver="Javascript: colorButton('f_borderColor', 'buttonColor-hilite');" onMouseOut="Javascript: colorButton('f_borderColor', 'buttonColor');"></span><span title="Unset Colour" class="buttonColor-nocolor" id="f_borderColor_unset" style="background-color: #<?php echo $_GET['f_borderColor']; ?>" onClick="Javascript: nullColor('f_borderColor');" onMouseOver="Javascript: colorButton('f_borderColor', 'buttonColor-hilite'); this.className = 'buttonColor-nocolor-hilite';" onMouseOut="Javascript: colorButton('f_borderColor', 'buttonColor'); this.className = 'buttonColor-nocolor';">&#x00d7;</span></span>
									&nbsp;
									<select id="f_borderStyle">
										<option value="none"   <?php echo ($_GET['f_borderStyle'] == 'none')   ? 'selected' : ''; ?>>None</option>
										<option value="dotted" <?php echo ($_GET['f_borderStyle'] == 'dotted') ? 'selected' : ''; ?>>Dotted</option>
										<option value="dashed" <?php echo ($_GET['f_borderStyle'] == 'dashed') ? 'selected' : ''; ?>>Dashed</option>
										<option value="solid"  <?php echo ($_GET['f_borderStyle'] == 'solid')  ? 'selected' : ''; ?>>Solid</option>
										<option value="double" <?php echo ($_GET['f_borderStyle'] == 'double') ? 'selected' : ''; ?>>Double</option>
										<option value="groove" <?php echo ($_GET['f_borderStyle'] == 'groove') ? 'selected' : ''; ?>>Grooved</option>
										<option value="ridge"  <?php echo ($_GET['f_borderStyle'] == 'ridge')  ? 'selected' : ''; ?>>Ridged</option>
										<option value="inset"  <?php echo ($_GET['f_borderStyle'] == 'inset')  ? 'selected' : ''; ?>>Inset</option>
										<option value="outset" <?php echo ($_GET['f_borderStyle'] == 'outset') ? 'selected' : ''; ?>>Outset</option>
									</select>
									&nbsp;
									<input type="text" id="f_borderWidth" size="5" value="<?php echo $_GET['f_borderWidth']; ?>" /> px
									</td>
								</tr>
							</table>
						</fieldset>
					</td>
				</tr>
			</table>

			<div style="text-align: right;">
			<hr />
			<button type="button" name="ok" onclick="return onOK();">OK</button>
			&nbsp;
			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			</div>
		</form>
	</body>
</html>
