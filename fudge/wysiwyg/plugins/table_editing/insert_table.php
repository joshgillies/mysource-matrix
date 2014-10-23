<!--
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: insert_table.html,v 1.11 2006/12/06 05:11:11 bcaldwell Exp $
*
*/
-->

<!DOCTYPE html>
<html style="height: 450px;">

	<head>
		<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"/>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<title><?php echo translate('Insert Table') ?></title>
		<link rel="stylesheet" type="text/css" href="../../../../__lib/web/css/edit.css" />
		<link rel="stylesheet" type="text/css" href="../../core/popup.css" />
		<script type="text/javascript" src="../../core/popup.js"></script>

		<script type="text/javascript">

			function Init() {
				__dlg_init("insertTable");
				document.getElementById("f_rows").focus();
			};

			function onOK() {
				var required = {
					"f_rows": "You must enter a number of rows",
					"f_cols": "You must enter a number of columns"
				};
				for (var i in required) {
					var el = document.getElementById(i);
					if (!el.value) {
						alert(required[i]);
						el.focus();
						return false;
					}
				}

				var fields = ["f_rows", "f_cols", "f_class",
							  "f_border", "f_width", "f_widthUnit",
							  "f_spacing", "f_padding", "f_summary"];
				var param = new Object();
				for (var i in fields) {
					var id = fields[i];
					var el = document.getElementById(id);
					param[id] = el.value;
				}

				// handles checkbox differently
				var el = document.getElementById("f_headerRow");
				param["f_headerRow"] = el.checked;
				var el = document.getElementById("f_headerCol");
				param["f_headerCol"] = el.checked;

				__dlg_close("insertTable", param);
				return false;
			};

			function onCancel() {
				__dlg_close("insertTable", null);
				return false;
			};

		</script>


	</head>

	<body onload="Init()">

		<div class="sq-popup-heading-frame title">
			<h1>Insert Table</h1>
		</div>

		<form action="" method="get" id="main-form">
			<table border="0" class="sq-fieldsets-table">
				<tr>
					<td>
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="50%">
									<fieldset>
									<legend><b>Dimensions</b></legend>
									<table style="width:100%">
										<tr>
											<td class="label">Rows:</td>
											<td><input type="text" id="f_rows" size="5" value="1" /></td>
										</tr>
										<tr>
											<td class="label">Cols:</td>
											<td><input type="text" id="f_cols" size="5" value="2" /></td>
										</tr>
									</table>
									</fieldset>
								</td>
								<td>&nbsp;</td>
								<td valign="top" width="50%">
									<fieldset>
									<legend><b>Table Headers</b></legend>
									<table  class="sq-popup-checkbox-list" class="width-100">
										<tr>
											<td class="label paddingr-1">First Row:</td>
											<td class="width-50"><input type="checkbox" id="f_headerRow" value="1"></td>
										</tr>
										<tr>
											<td class="label paddingr-1">First Column:</td>
											<td class="width-50"><input type="checkbox" id="f_headerCol" value="1"></td>
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
						<legend><b>Style Attributes</b></legend>
						<table width="100%">
							<tr>
								<td class="label">Class Name:</td>
								<td><input type="text" id="f_class" size="30" value="" /></td>
							</tr>
							<tr>
								<td class="label">Width:</td>
								<td>
									<input type="text" id="f_width" size="5" value="100" />
									<select id="f_widthUnit">
										<option value="%" selected="1">%</option>
										<option value="px">px</option>
									</select>
								</td>
							</tr>
							<tr>
								<td class="label">Border:</td>
								<td>
								<input type="text" id="f_border" size="5" value="1" />
								</td>
							</tr>
							<tr>
								<td class="label">Cell Spacing:</td>
								<td><input type="text" id="f_spacing" size="5" value="" /> px</td>
							</tr>
							<tr>
								<td class="label">Cell Padding:</td>
								<td><input type="text" id="f_padding" size="5" value="" /> px</td>
							</tr>
						</table>
						</fieldset>
					</td>
				</tr>

				<tr>
					<td>
						<fieldset>
						<legend><b>Optional Attributes</b></legend>
						<table width="100%">
							<tr>
								<td class="label" valign="top">Summary:</td>
								<td><textarea  id="f_summary" cols="20" rows="3"></textarea></td>
							</tr>
						</table>
						</fieldset>
					</td>
				</tr>

			</table>

			<div class="sq-popup-button-wrapper">	

			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			&nbsp;
			<button type="button" name="ok" onclick="return onOK();" class="sq-btn-green">OK</button>
			
			</div>

		</form>
	</body>
</html>