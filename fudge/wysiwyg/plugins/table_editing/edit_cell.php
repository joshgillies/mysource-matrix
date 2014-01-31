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
* $Id: edit_cell.php,v 1.14 2013/04/23 08:09:35 cupreti Exp $
*
*/

/**
* Cell Edit Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix
*/
require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once dirname(__FILE__).'/../../wysiwyg_plugin.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

$wysiwyg = null;
$plugin = new wysiwyg_plugin($wysiwyg);
$_GET['f_bgcolor'] = preg_replace('/[^a-zA-Z_0-9 ]+/', '', $_GET['f_bgcolor']);
$_GET['f_color'] = preg_replace('/[^a-zA-Z_0-9 ]+/', '', $_GET['f_color']);
$_GET['f_borderColor'] = preg_replace('/[^a-zA-Z_0-9 ]+/', '', $_GET['f_borderColor']);
?>

<html style="width:380px; height:270px;">

	<head>
		<title>Edit Cell Properties</title>
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
				__dlg_init("editCellProperties");
			};

			function onOK() {
				var fields = ["f_width", "f_height", "f_widthUnit",
							  "f_heightUnit", "f_align", "f_valign",
							  "f_bgcolor", "f_color", "f_borderColor",
							  "f_borderWidth", "f_borderStyle"];
				var params = new Object();
				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					params[id] = el.value;
				}
				__dlg_close("editCellProperties", params);
				return false;
			};

			function onCancel() {
				__dlg_close("editCellProperties", null);
				return false;
			};

		</script>

		<style type="text/css">

		</style>

	</head>

	<body onLoad="Init();">

		<div class="title">Cell Properties</div>

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
											<input type="text" id="f_width" size="5" value="<?php echo htmlspecialchars($_GET['f_width']); ?>" />
											<select id="f_widthUnit">
												<option value="px" <?php echo ($_GET['f_widthUnit'] == 'px') ? 'selected' : ''; ?>>px</option>
												<option value="%"  <?php echo ($_GET['f_widthUnit'] == '%')  ? 'selected' : ''; ?>>%</option>
											</select>
											</td>
										</tr>
										<tr>
											<td class="label">Height:</td>
											<td>
											<input type="text" id="f_height" size="5" value="<?php echo htmlspecialchars($_GET['f_height']); ?>" />
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
									<legend><b>Alignment</b></legend>
									<table style="width:100%">
										<tr>
											<td class="label">Horiz:</td>
											<td>
											<select id="f_align">
												<option value="">none</option>
												<option value="left"   <?php echo ($_GET['f_align'] == 'left')   ? 'selected' : ''; ?>>left</option>
												<option value="center" <?php echo ($_GET['f_align'] == 'center') ? 'selected' : ''; ?>>center</option>
												<option value="rigth"  <?php echo ($_GET['f_align'] == 'right')  ? 'selected' : ''; ?>>right</option>
											</select>
											</td>
										</tr>
										<tr>
											<td class="label">Vert:</td>
											<td>
											<select id="f_valign">
												<option value="">none</option>
												<option value="top"    <?php echo ($_GET['f_valign'] == 'top')    ? 'selected' : ''; ?>>top</option>
												<option value="middle" <?php echo ($_GET['f_valign'] == 'middle') ? 'selected' : ''; ?>>middle</option>
												<option value="bottom" <?php echo ($_GET['f_valign'] == 'bottom') ? 'selected' : ''; ?>>bottom</option>
											</select>
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
					<td width="100%">
						<fieldset>
						<legend><b>Cell Styles / Colours</b></legend>
							<table width="100%">
								<tr>
									<td class="label">Background:</td>
									<td width="100%">
									<input type="hidden" id="f_bgcolor" value="<?php echo htmlspecialchars($_GET['f_bgcolor']); ?>">
									<span class="buttonColor" id="f_bgcolor_button"><span class="buttonColor-chooser" id="f_bgcolor_chooser" style="background-color: #<?php echo htmlspecialchars($_GET['f_bgcolor']); ?>" onClick="Javascript: colorPopup('f_bgcolor');" onMouseOver="Javascript: colorButton('f_bgcolor', 'buttonColor-hilite');" onMouseOut="Javascript: colorButton('f_bgcolor', 'buttonColor');"></span><span title="Unset Colour" class="buttonColor-nocolor" id="f_bgcolor_unset" style="background-color: #<?php echo htmlspecialchars($_GET['f_bgcolor']); ?>" onClick="Javascript: nullColor('f_bgcolor');" onMouseOver="Javascript: colorButton('f_bgcolor', 'buttonColor-hilite'); this.className = 'buttonColor-nocolor-hilite';" onMouseOut="Javascript: colorButton('f_bgcolor', 'buttonColor'); this.className = 'buttonColor-nocolor';">&#x00d7;</span></span>
									</td>
								</tr>
								<tr>
									<td class="label">Foreground:</td>
									<td width="100%">
									<input type="hidden" id="f_color" value="<?php echo htmlspecialchars($_GET['f_color']); ?>">
									<span class="buttonColor" id="f_color_button"><span class="buttonColor-chooser" id="f_color_chooser" style="background-color: #<?php echo htmlspecialchars($_GET['f_color']); ?>" onClick="Javascript: colorPopup('f_color');" onMouseOver="Javascript: colorButton('f_color', 'buttonColor-hilite');" onMouseOut="Javascript: colorButton('f_color', 'buttonColor');"></span><span title="Unset Colour" class="buttonColor-nocolor" id="f_color_unset" style="background-color: #<?php echo htmlspecialchars($_GET['f_color']); ?>" onClick="Javascript: nullColor('f_color');" onMouseOver="Javascript: colorButton('f_color', 'buttonColor-hilite'); this.className = 'buttonColor-nocolor-hilite';" onMouseOut="Javascript: colorButton('f_color', 'buttonColor'); this.className = 'buttonColor-nocolor';">&#x00d7;</span></span>
									</td>
								</tr>
								<tr>
									<td class="label" valign="top">Border:</td>
									<td width="100%" valign="top">
									<input type="hidden" id="f_borderColor" value="<?php echo htmlspecialchars($_GET['f_borderColor']); ?>">
									<span class="buttonColor" id="f_borderColor_button"><span class="buttonColor-chooser" id="f_borderColor_chooser" style="background-color: #<?php echo htmlspecialchars($_GET['f_borderColor']); ?>" onClick="Javascript: colorPopup('f_borderColor');" onMouseOver="Javascript: colorButton('f_borderColor', 'buttonColor-hilite');" onMouseOut="Javascript: colorButton('f_borderColor', 'buttonColor');"></span><span title="Unset Colour" class="buttonColor-nocolor" id="f_borderColor_unset" style="background-color: #<?php echo htmlspecialchars($_GET['f_borderColor']); ?>" onClick="Javascript: nullColor('f_borderColor');" onMouseOver="Javascript: colorButton('f_borderColor', 'buttonColor-hilite'); this.className = 'buttonColor-nocolor-hilite';" onMouseOut="Javascript: colorButton('f_borderColor', 'buttonColor'); this.className = 'buttonColor-nocolor';">&#x00d7;</span></span>
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
									<input type="text" id="f_borderWidth" size="5" value="<?php echo htmlspecialchars($_GET['f_borderWidth']); ?>" /> px
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
