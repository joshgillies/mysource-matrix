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
* $Id: spell_checker_popup.php,v 1.15 2013/09/11 03:30:50 ewang Exp $
*
*/

/**
* Spell Checker Popup for the WYSIWYG
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.15 $
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
<!DOCTYPE html>
<html style="height: 400px">
	<head>
		<title>Spell Checker</title>
		<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"/>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />
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

	
	</head>

	<body onLoad="Init(); initDocument(); if (opener) opener.blockEvents('spellChecker')" onUnload="if (opener) opener.unblockEvents(); parent_object._tmp['disable_toolbar'] = false; parent_object.updateToolbar();">
		<form style="display: none;" action="spell_checker.php" method="post" target="framecontent" accept-charset="utf-8">
			<input type="hidden" name="content" id="f_content"/>
			<input type="hidden" name="dictionary" id="f_dictionary"/>
			<input type="hidden" name="init" id="f_init" value="1"/>
			<input type="hidden" name="token" id="f_token" value="<?php echo (get_unique_token()); ?>"/>
		</form>
		<div id="spellchecker">
			<table cellspacing="0" cellpadding="0" width="100%">
				<tr>
					<td colspan="2">
						<table width="100%" cellpadding="0" cellspacing="0">
							<tr>
								<td class="sq-popup-heading-frame title">						
										<h1 id="status"><?php echo translate('please_wait'); ?>...</h1>							
								</td>
								<td>
									<!--hiding the dictionary chooser for now -->
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
					<td valign="top" class="sq-popup-asset-map">
						<fieldset>
							<legend><b><?php echo translate('original_word'); ?></b></legend>
							<div id="v_currentWord" style="text-align: center"><?php echo translate('please_wait'); ?>...</div>
						</fieldset>
						<fieldset>
							<legend><b><?php echo translate('replace_with'); ?></b></legend>
							<div>
								<input type="text" id="v_replacement" style="width: 192px;" /><br />
								<div nowrap>
									<button class="button" id="b_replace"><?php echo translate('replace'); ?></button>
									<button class="button" id="b_replall"><?php echo translate('replace_all'); ?></button><br />
									<button class="button" id="b_ignore"><?php echo translate('ignore'); ?></button>
									<button class="button" id="b_ignall"><?php echo translate('ignore_all'); ?></button>
								</div>
							</div>
						</fieldset>

						<fieldset>
							<legend><b><?php echo translate('suggestions'); ?></b></legend>
							<div >
								<select size="11" style="width: 200px;" id="v_suggestions"></select>
							</div>
						</fieldset>
				
					</td>
					<td>
						<table height="100%" width="96%">
							<tr>
								<td>
									<fieldset>
										<legend><b>Content</b></legend>
										<iframe src="about:blank" width="100%" height="300" id="i_framecontent" name="framecontent" class="f_content"></iframe>
									</fieldset>
								</td>
							</tr>
							<tr>
								<td>
									<div class="sq-popup-button-wrapper">												
										<button type="button" id="b_cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
										&nbsp;
										<button type="button" class="sq-btn-green" id="b_ok" onclick="return onOK();"><?php echo translate('ok'); ?></button>							
								    </div>
								</td>
							</tr>
						</table>						
					</td>
				</tr>
			</table>
		</div>
	</body>
</html>
