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
* $Id: insert_image.php,v 1.20.2.1 2005/04/20 23:53:07 gsherwood Exp $
*/

/**
* Insert Image Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

if (!isset($_GET['f_imageid'])) $_GET['f_imageid'] = 0;
?>

<html style="width: 630px; height: 480px;">
	<head>
		<title>Insert Image</title>

		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/javaExternalCall.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("matrixInsertImage");
			};

			/**
			* Add auto alternate text to selected image
			*
			* @return void
			*/
			function setAltText()
			{
				var path = document.getElementById("f_imageid[url]").value;
				while (path.indexOf("/") >= 0) path = path.substring(path.indexOf("/") + 1);
				ext = "";
				i = path.length;
				while (ext.charAt(0) != "." && i > 0) ext = path.substring(i--);
				path = path.substring(0, ++i);

				document.getElementById("f_alt").value = path;

			}//end setAltText()


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

				__dlg_close("matrixInsertImage", param);
				return false;
			};

			function onCancel() {
				__dlg_close("matrixInsertImage", null);
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
				width: 407px;
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

	<body onload="Init();" onUnload="asset_finder_onunload();">

		<div class="title">Insert Image</div>
		<form action="" method="get" name="main_form">
			<table width="100%">
				<tr>
					<td valign="top">
						<?php
							include_once(SQ_LIB_PATH.'/asset_map/asset_map.inc');
							$asset_map = new Asset_Map();
							$asset_map->embed_asset_map('simple', 200, 350);
						?>
					</td>
					<td valign="top">
						<table cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%">
									<fieldset>
									<legend><b>General</b></legend>
										<table style="width:100%">
											<tr>
												<td class="label" nowrap="nowrap">Image URL:</td>
												<td>
													<?php asset_finder('f_imageid', $_GET['f_imageid'], Array('image' => 'D'), '', false, 'setAltText'); ?>
												</td>
											</tr>
											<tr>
												<td class="label" nowrap="nowrap">Alternate text:</td>
												<td>
													<input type="text" name="alt" id="f_alt" style="width:100%" title="For browsers that don't support images" value="<?php echo $_REQUEST['f_alt']?>" />
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td valign="top" width="50%">
									<fieldset>
										<legend>Layout</legend>
										<table style="width:100%">
											<tr>
												<td class="label" width="30%">Alignment:</td>
												<td>
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
												</td>
											</tr>
											<tr>
												<td class="label">Border thickness:</td>
												<td>
													<input type="text" name="border" id="f_border" size="5" title="Leave empty for no border" value="<?php echo $_REQUEST['f_border']?>" />
												</td>
											</tr>
											<tr>
												<td class="label">Horizontal:</td>
												<td>
													<input type="text" name="horiz" id="f_horiz" size="5" title="Horizontal padding" value="<?php echo $_REQUEST['f_horiz']?>" />
												</td>
											</tr>
											<tr>
												<td class="label">Vertical:</td>
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
										<legend>Size</legend>
										<table style="width:100%">
											<tr>
												<td class="label" width="30%">Width:</td>
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
