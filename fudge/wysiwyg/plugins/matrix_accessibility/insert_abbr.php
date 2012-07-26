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
* $Id: insert_abbr.php,v 1.9.26.1 2012/07/26 05:21:13 ewang Exp $
*
*/

/**
* Insert Abbreviation Popup for the WYSIWYG
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.9.26.1 $
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

		<?php
		//add required js translation files, as we are using asset finder
		$include_list = Array(sq_web_path('lib').'/js/translation.js');

		$locales = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

		foreach($locales as $locale) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$locale.'.js')) {
				$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$locale.'.js';
			}
		}

		foreach($include_list as $link) {
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
				document.write('<tr>');
				document.write('<td class="label"><?php echo translate('abbreviation'); ?>:</td>');
				document.write('<td colspan="3"><?php text_box('abbr', preg_replace('/[\']+/', '' , trim($_GET['abbr'])), 40, 0);?></td>');
				document.write('</tr>');
				document.write('<tr>');
				document.write('    <td class="label"><?php echo translate('definition'); ?>:</td>');
				document.write('    <td colspan="3"><?php text_box('title', preg_replace('/[\']+/', '' , trim($_GET['title'])), 40, 0);?></td>');
				document.write('</tr>');
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
		<div class="title"><?php echo translate('insert_abbreviation'); ?></div>
		<form action="" method="get" name="main_form">
			<table width="100%" >
				<tr>
					<td valign="top" width="100%">
						<fieldset>
						<legend><b><?php echo translate('general'); ?></b></legend>
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
					document.write('<button type="button" name="ok" onclick="return onOK();"><?php echo translate('ok'); ?></button>');
				</script>
				<button type="button" name="cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
			</div>
		</form>
	</body>
</html>
