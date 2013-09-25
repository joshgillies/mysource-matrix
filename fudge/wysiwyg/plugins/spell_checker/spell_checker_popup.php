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
* $Id: spell_checker_popup.php,v 1.14.4.1 2013/09/11 03:34:32 ewang Exp $
*
*/

/**
* Spell Checker Popup for the WYSIWYG
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.14.4.1 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

require_once dirname(__FILE__).'/../../wysiwyg_plugin.inc';
$wysiwyg = null;
$plugin = new wysiwyg_plugin($wysiwyg);
?>

<!--
  htmlArea v3.0 - Copyright (c) 2003 interactivetools.com, inc.
  This notice MUST stay intact for use (see license.txt).

  A free WYSIWYG editor replacement for <textarea> fields.
  For full source code and docs, visit http://www.interactivetools.com/

  Version 3.0 developed by Mihai Bazon for InteractiveTools.
	http://students.infoiasi.ro/~mishoo

  Modifications for PHP Plugin Based System
	developed by Greg Sherwood for Squiz.Net.
	http://www.squiz.net/
	greg@squiz.net

  Spell Checker Modifications for PHP Plugin Based System
	developed by Marc McIntyre for Squiz.Net.
	http://www.squiz.net/
	mmcintyre@squiz.net
-->

<html style="width: 600px; height: 400px">
	<head>
		<title>Spell Checker</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<script type="text/javascript" src="../../core/popup.js"></script>
		<?php
			//add required js translation files
			list($lang, $country, $variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

			$include_list[] = sq_web_path('lib').'/js/general.js';
			$include_list[] = sq_web_path('lib').'/js/translation.js';

			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'.js')) {
				$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$lang.'.js';
			}

			if (!empty($country)) {
				if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'.js')) {
					$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'.js';
				}

				if (!empty($variant)) {
					if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js')) {
						$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js';
					}
				}
			}


			foreach($include_list as $link) {
				?><script type="text/javascript" src="<?php echo $link; ?>"></script><?php
			}
		?>

		<script type="text/javascript">
			var parent_object = opener.editor_<?php echo preg_replace('/[\'"\(\);\[\]{}<>=]+/', '', $_REQUEST['editor_name']); ?>._object;

			window.opener.onFocus = function() { getFocus(); }
			parent_object.onFocus = function() { getFocus(); }

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("spellChecker");
			};

			function onOK() {
				var param = new Object();
				param["html"] = makeCleanDoc(false);
				__dlg_close("spellChecker", param);
				return false;
			};

			function onCancel() {
				__dlg_close("spellChecker", null);
				return false;
			};
		</script>

		<script type="text/javascript" src="<?php echo $_SERVER['PHP_SELF'].'/../../'.$plugin->get_popup_href('spell_checker.js', 'spell_checker')?>"></script>

		<style type="text/css">
			html, body {
				font-family: Verdana, Arial, Helvetica, san-serif;
				font-size: xx-small;
				font-weight: normal;
				text-decoration: none;
				background-color: #402F48;
				color: #FFFFFF;
				margin: 1px 1px;
			}

			a:link, a:visited {
				color: #FFFFFF; text-decoration: none;
			}

			a:hover {
				color: #B7A9BD; text-decoration: underline;
			}

			table {
				background-color: #402F48; color: ButtonText;
				font-family: tahoma,verdana,sans-serif; font-size: 11px;
			}

			iframe {
				background-color:#402F48;
				color: #FFFFFF;
				font-family: tahoma,verdana,sans-serif; font-size: 11px;
			}

			.controls .sectitle {
				color: #FFFFFF;
				font-weight: bold; padding: 2px 4px;
				margin:0px auto;
				text-align:left;

			}

			.controls .secbody {
				margin-bottom: 0px;
			}

			button, select {
				font-family: tahoma,verdana,sans-serif; font-size: 11px;
			}

			button {
				width: 6em; padding: 0px;
			}

			input, select {
				font-family: fixed,"andale mono",monospace;
			}


			#v_currentWord {
				color: #A7A1AA; font-weight: bold; font-size: 120%;
			}

			#statusbar {
				padding: 0px 0px 0px 5px;
			}

			#status {
				font-weight: bold;
			}

			.button2{
				font-size: 7pt;
				font-family: Verdana,Arial,Helvetica;
				color: #FFFFFF;
				width: 70px;
				background: #725B7D;
				border-style: solid;
				border-width: 1;
				border-color: #402F48;
				font-weight: bold;
				margin: 2px 2px;
			}
			.button{
				margin: 2px 2px;
				font-size: 7pt;
				font-family: Verdana,Arial,Helvetica;
				color: #FFFFFF;
				width: 70px;
				background: #725B7D;
				border-style: solid;
				border-width: 1;
				border-color: #7D7582;
				font-weight: bold;
			}

			.status_div {
				background-color: #7D7582;
				padding: 2px 2px;
			}

			.major_table {
				height: 100%;
				width: 100%;
				border: solid 2px;
				border-color: #7D7582;
				padding: 0px 0px 0px 0px;
			}

			.status {
				color: #FFFFFF;
			}
		</style>
	</head>

	<body onLoad="Init(); initDocument(); if (opener) opener.blockEvents('spellChecker')" onUnload="if (opener) opener.unblockEvents(); parent_object._tmp['disable_toolbar'] = false; parent_object.updateToolbar();">
		<form style="display: none;" action="spell_checker.php" method="post" target="framecontent" accept-charset="utf-8">
			<input type="hidden" name="content" id="f_content"/>
			<input type="hidden" name="dictionary" id="f_dictionary"/>
			<input type="hidden" name="init" id="f_init" value="1"/>
			<input type="hidden" name="token" id="f_token" value="<?php echo (get_unique_token()); ?>"/>
		</form>
		<table class="major_table" cellspacing="0" cellpadding="0">
			<tr>
				<td class="status_div" colspan="2">
					<table width="100%" cellpadding="0" cellspacing="0">
						<tr>
							<td class="status_div">
								<span id="status" class="status">&nbsp;<?php echo translate('please_wait'); ?>...</span>
							</td>
							<td class="status_div">
								<!-- hiding the dictionary chooser for now -->
								<span style="float: right; display: none">
									<span class="status"><?php echo translate('dictionary'); ?></span>
									<select id="v_dictionaries" style="width: 10em"></select>
									<button class="button2" id="b_recheck"><?php echo translate('re-check'); ?></button>
								</span>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td valign="top" class="controls" nowrap>
					<div class="sectitle"><?php echo translate('original_word'); ?></div>
					<div class="secbody" id="v_currentWord" style="text-align: center"><?php echo translate('please_wait'); ?>...</div>
					<div class="sectitle"><?php echo translate('replace_with'); ?></div>
					<div class="secbody">
						<input type="text" id="v_replacement" style="width: 94%; margin-left: 3%; align: center" /><br />
						<div style="text-align: center; margin-top: 2px;" nowrap>
							<button class="button" id="b_replace"><?php echo translate('replace'); ?></button>
							<button class="button" id="b_replall"><?php echo translate('replace_all'); ?></button><br />
							<button class="button" id="b_ignore"><?php echo translate('ignore'); ?></button>
							<button class="button" id="b_ignall"><?php echo translate('ignore_all'); ?></button>
						</div>
					</div>
					<div class="sectitle"><?php echo translate('suggestions'); ?></div>
					<div class="secbody">
						<select size="11" style="width: 94%; margin-left: 3%;" id="v_suggestions"></select>
					</div>
					<div valign="top" class="secbody" align="center" nowrap>
						&nbsp;<button class="button" id="b_ok" onclick="return onOK();"><?php echo translate('ok'); ?></button>
						<button class="button" id="b_cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
					</div>
				</td>
				<td width="100%">
					<table width="100%" height="100%">
						<tr>
							<td>
								<iframe src="about:blank" width="100%" height="100%" id="i_framecontent" name="framecontent" class="f_content"></iframe>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
	</body>
</html>
