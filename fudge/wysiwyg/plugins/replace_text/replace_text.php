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
* $Id: replace_text.php,v 1.1.28.1 2012/08/15 05:08:15 cupreti Exp $
*
*/

/**
* Insert HTML Tidy for the WYSIWYG
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @author	Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.28.1 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user')){
	exit;
}

if (!isset($_GET['name']))		  $_GET['name'] = '';

?>

<html style="width: 420px; height: 500px;">

	<head>
		<title>Replace Text</title>
		<?php
		// add required js translation files, as we are using asset finder
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
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>

		<script type="text/javascript">

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("ReplaceText");
				setTimeout('self.focus()',100);
			};

			function onOK() {
				var confirm_str = "WARNING!\nThe appearance of the content may be affected and the action cannot be undone.\nAre you sure you want to apply the replacement?";

				if (confirm(confirm_str)) {
					var rep_types = new Array();
					var selected_only = parseInt(document.getElementById("rep_type0").value);
					rep_types.push(selected_only);
					var i = 1;
					while (document.getElementById("rep_type"+i) != null)
					{
						rep_types.push(document.getElementById("rep_type"+i).checked);
						i++;
					}
					__dlg_close("ReplaceText", rep_types);
					return false;
				}
				return true;
			};

			function onCancel() {
				__dlg_close("ReplaceText", null);
				return false;
			};

			function checkAll(val) {
				document.getElementById("checkAllSpan").innerHTML = (val)?" Uncheck All":" Check All";
				var i = 0;
				while (document.getElementById("rep_type"+i) != null)
				{
					document.getElementById("rep_type"+i).checked = val;
					i++;
				}
			}
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
		</style>
	</head>

	<body onload="Init()">

		<div class="title">Replace Text</div>

		<form action="" method="get" name="Form1">
			<table border="0" width="100%">
				<tr>
					<td>
						<fieldset>
							<legend><b>Replacement types</b></legend>
							<table style="width:100%">
								<tr>
									<td>Selected text only?
										<select id="rep_type0" name="rep_type0">
											<option value="1">Yes</option>
											<option value="0">No</option>
										</select>
								</tr>
								<tr>
									<td>
										<input type="checkbox" name="rep_type" id="rep_type" onclick="checkAll(this.checked)"/><label for="rep_type"><span id="checkAllSpan"> Check All</span></label>
									</td>
								</tr>
							</table>
						</fieldset>
					</td>
				</tr>
				<tr>
					<td>
						<fieldset>
							<legend><b>Non-extreme options</b></legend>
							<table style="width:100%">
								<tr>
									<td>
										<input type="checkbox" name="rep_type1" id="rep_type1" checked="checked"/><label for="rep_type1"> Remove <b>&lt;font&gt;</b> tags</label><br/>
										<input type="checkbox" name="rep_type2" id="rep_type2" checked="checked"/><label for="rep_type2"> Remove double spaces</label><br/>
										<input type="checkbox" name="rep_type3" id="rep_type3" checked="checked"/><label for="rep_type3"> Remove <b>non-HTML</b> tags</label><br/>
										<input type="checkbox" name="rep_type4" id="rep_type4" checked="checked"/><label for="rep_type4"> Change Microsoft Word<sup>&#174;</sup>'s bullets</label><br/>
										<input type="checkbox" name="rep_type5" id="rep_type5" checked="checked"/><label for="rep_type5"> Remove soft hyphens (&amp;shy;)</label><br/>
									</td>
								</tr>
								<tr>
							</table>
						</fieldset>
					</td>
				</tr>
				<tr>
					<td>
					<fieldset>
							<legend><b>Extreme options</b></legend>
							<table style="width:100%">
								<tr>
									<td>
										<input type="checkbox" name="rep_type6" id="rep_type6" /><label for="rep_type6"> Remove <b>style</b> attribute</label><br/>
										<input type="checkbox" name="rep_type7" id="rep_type7" /><label for="rep_type7"> Remove <b>class</b> attribute</label><br/>
										<input type="checkbox" name="rep_type8" id="rep_type8" /><label for="rep_type8"> Remove <b>&lt;table&gt;</b> tags</label><br/>
										<input type="checkbox" name="rep_type9" id="rep_type9" /><label for="rep_type9"> Remove <b>&lt;span&gt;</b> tags</label><br/>
										<input type="checkbox" name="rep_type10" id="rep_type10" /><label for="rep_type10"> Remove all empty tags</label><br/>
										<input type="checkbox" name="rep_type11" id="rep_type11" /><label for="rep_type11"> Remove all tags' attributes (except HREF and SRC)</label><br/>
									</td>
								</tr>
							</table>
					</fieldset>
					</td>
				</tr>
			</table>

			<div style="text-align: right;">
			<hr />
			<button type="button" name="ok" onclick="if (!onOK()) return;">OK</button>
			&nbsp;
			<button type="button" name="cancel" onclick="window.close();">Cancel</button>
			</div>
		</form>
	</body>
</html>
