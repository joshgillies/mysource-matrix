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
* $Id: insert_anchor.php,v 1.11 2013/04/23 08:06:03 cupreti Exp $
*
*/

/**
* Insert Anchor Popup for the WYSIWYG
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

if (!isset($_GET['name']))		$_GET['name']	= "";

?>

<html style="width: 400px; height: 280px;">
	<head>
		<title>Insert Anchor</title>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

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
				__dlg_init("matrixInsertAnchor");
				setTimeout('self.focus()',100);
			};

			function onOK() {
				var confirm_str = "WARNING!\nYou are about to remove this anchor tag and can not be undone.\nAre you sure you want to remove the anchor?";

				// pass data back to the calling window
				var fields = ["name"];
				var param = new Object();
				var f = document.main_form;

				param["name"]	= form_element_value(f.name);
				param["remove"]	= form_element_value(f.remove);

				if (param["remove"] == "1") {
					if (confirm(confirm_str)) {
						__dlg_close("matrixInsertAnchor", param);
						return false;
					}
				} else {
					__dlg_close("matrixInsertAnchor", param);
					return false;
				}
				return true;
			};

			function onCancel() {
				__dlg_close("matrixInsertAnchor", null);
				return false;
			};

		</script>


	</head>

	<body onload="Javascript: Init();">
		<div class="sq-popup-heading-frame">
			<h1>Insert Anchor</h1>
		</div>

		<form action="" method="get" name="main_form" id="main-form">
			<table class="sq-fieldsets-table">
				<tr>
					<td valign="top" width="100%">
						<fieldset>
						<legend><b>General</b></legend>
						<table>
							<tr>
								<td class="label">Anchor Name:</td>
								<td colspan="3"><?php text_box('name', $_GET['name'], 40, 0);?>
								</td>
							</tr>
							<tr>
								<td colspan="4"><br /></td>
							</tr>
							<tr>
								<td class="label">Remove:</td>
								<td colspan="3"><?php check_box('remove', (empty($_GET['name']) ? '0' : '1'), FALSE, '', (empty($_GET['name']) ? 'disabled=true' : '')); ?>
								</td>
							</tr>
						</table>
						</fieldset>
					</td>
				</tr>
			</table>

			<div style="margin-top: 5px; margin-right: 5px; text-align: right;">
			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			&nbsp;
			<button type="button" name="ok" onclick="return onOK();" class="sq-btn-green">OK</button>
			</div>
		</form>
	</body>
</html>
