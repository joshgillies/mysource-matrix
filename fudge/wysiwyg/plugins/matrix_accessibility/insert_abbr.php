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
* $Id: insert_abbr.php,v 1.4 2004/09/09 03:34:17 amiller Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Insert Abbreviation Popup for the WYSIWYG
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (!isset($_GET['title']))		  $_GET['title'] = "";

?>

<html style="width: 400px; height: 200px;">
	<head>
		<title>Insert Abbreviation</title>

		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">
			var can_insert = true;
			if (navigator.appName == "Microsoft Internet Explorer") { can_insert = false; }

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("matrixInsertAbbr");
				setTimeout('self.focus()',100);
			};

			function onOK() {
				// pass data back to the calling window 
				var fields = ["title"];
				var param = new Object();
				var f = document.main_form;

				param["title"] = form_element_value(f.title);
				param["abbr"]  = form_element_value(f.abbr);
				
				__dlg_close("matrixInsertAbbr", param);
				return false;
			};

			function onCancel() {
				__dlg_close("matrixInsertAbbr", null);
				return false;
			};
			
			function buildForm() {
				if (can_insert) {
					document.write('<tr>');
					document.write('<td class="label">Abbreviation:</td>');
					document.write('<td colspan="3"><?php text_box('abbr', trim($_GET['abbr']), 40, 0);?></td>');
					document.write('</tr>');
					document.write('<tr>');
					document.write('    <td class="label">Definition:</td>');
					document.write('    <td colspan="3"><?php text_box('title', trim($_GET['title']), 40, 0);?></td>');
					document.write('</tr>');
				} else {
					document.write('<tr>');
					document.write('    <td colspan="4">Internet Explorer does not support automatically');
					document.write('    inserting the Abbreviation tag. You may insert this tag manually in Source View mode.</td>');
					document.write('</tr>');
				}
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

	<body onload="Javascript: Init();">
		<div class="title">Insert Abbreviation</div>
		<form action="" method="get" name="main_form">
			<table width="100%" >
				<tr>
					<td valign="top" width="100%">
						<fieldset>
						<legend><b>General</b></legend>
						<table style="width:100%">
							<script type="text/javascript"> 
								buildForm(); 
							</script>
						</table>
						</fieldset>
					</td>
				</tr>
			</table> 
	
			<div style="margin-top: 5px; margin-right: 5px; text-align: right;">
				<hr />
				<script type="text/javascript" language="javascript">
				if (can_insert) {
					document.write('<button type="button" name="ok" onclick="return onOK();">OK</button>');
				};
				</script>
				<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			</div>
		</form>
	</body>
</html>
