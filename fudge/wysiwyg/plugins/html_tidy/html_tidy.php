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
* $Id: html_tidy.php,v 1.2 2004/11/05 02:47:42 dbaranovskiy Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Insert HTML Tidy for the WYSIWYG
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (!isset($_GET['name']))		  $_GET['name'] = "";

?>

<html style="width: 380px; height: 400px;">

	<head>
		<title>Replace Text</title>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("ReplaceText");
				setTimeout('self.focus()',100);
			};

			function onOK() {
				var rep_types = new Array();
				var i = 0;
				while (document.getElementById("rep_type"+i) != null)
				{
					rep_types.push(document.getElementById("rep_type"+i).checked);
					i++;
				}
				__dlg_close("ReplaceText", rep_types);
				return false;
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
									<td class="label" style="border-bottom:solid 1px #725B7D">
										<input type="checkbox" name="rep_type" onclick="checkAll(this.checked)"/><span id="checkAllSpan"> Check All</span>
									</td>
								</tr>
								<tr>
									<td class="label">
										<input type="checkbox" name="rep_type0" id="rep_type0" /> Remove <b>&lt;font&gt;</b> tags<br/>
										<input type="checkbox" name="rep_type1" id="rep_type1" /> Remove <b>style</b> attribute<br/>
										<input type="checkbox" name="rep_type2" id="rep_type2" /> Remove <b>class</b> attribute<br/>
										<input type="checkbox" name="rep_type3" id="rep_type3" /> Remove <b>&lt;table&gt;</b> tags<br/>
										<input type="checkbox" name="rep_type4" id="rep_type4" /> Remove <b>&lt;span&gt;</b> tags<br/>
										<input type="checkbox" name="rep_type5" id="rep_type5" /> Remove <b>non-HTML</b> tags<br/>
										<input type="checkbox" name="rep_type6" id="rep_type6" /> Remove double spaces<br/>
										<input type="checkbox" name="rep_type7" id="rep_type7" /> Remove all empty tags<br/>
										<input type="checkbox" name="rep_type8" id="rep_type8" /> Remove all tag's atributes<br/>
										<input type="checkbox" name="rep_type9" id="rep_type9" /> Change Microsoft Word<sup>&#174;</sup>'s bullets<br/>
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
			<button type="button" name="cancel" onclick="window.close();">Cancel</button>
			</div>
		</form>
	</body>
</html>
