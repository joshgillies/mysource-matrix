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
* $Id: spell_checker_popup.php,v 1.4 2003/11/26 00:51:23 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

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

		<script type="text/javascript">
			var parent_object = opener.editor_<?php echo $_REQUEST['editor_name']?>._object;

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
		</form>
		<table class="major_table" cellspacing="0" cellpadding="0">
			<tr>
				<td class="status_div" colspan="2">
					<table width="100%" cellpadding="0" cellspacing="0">
						<tr>
							<td class="status_div">
								<span id="status" class="status">&nbsp;Please Wait...</span>
							</td>
							<td class="status_div">
								<!-- hiding the dictionary chooser for now -->
								<span style="float: right; display: none">
									<span class="status">Dictionary</span>
									<select id="v_dictionaries" style="width: 10em"></select>
									<button class="button2" id="b_recheck">Re-check</button>
								</span>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td valign="top" class="controls" nowrap>
					<div class="sectitle">Original word</div>
					<div class="secbody" id="v_currentWord" style="text-align: center">Please Wait...</div>
					<div class="sectitle">Replace with</div>
					<div class="secbody">
						<input type="text" id="v_replacement" style="width: 94%; margin-left: 3%; align: center" /><br />
						<div style="text-align: center; margin-top: 2px;" nowrap>
							<button class="button" id="b_replace">Replace</button>
							<button class="button" id="b_replall">Replace all</button><br />
							<button class="button" id="b_ignore">Ignore</button>
							<button class="button" id="b_ignall">Ignore all</button>
						</div>
					</div>
					<div class="sectitle">Suggestions</div>
					<div class="secbody">
						<select size="11" style="width: 94%; margin-left: 3%;" id="v_suggestions"></select>
					</div>
					<div valign="top" class="secbody" align="center" nowrap>
						&nbsp;<button class="button" id="b_ok" onclick="return onOK();">OK</button>
						<button class="button" id="b_cancel" onclick="return onCancel();">Cancel</button>
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
