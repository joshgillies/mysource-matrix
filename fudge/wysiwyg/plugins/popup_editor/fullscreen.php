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
* $Id: fullscreen.php,v 1.11 2003/11/26 00:51:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* WYSIWYG Full Screen Editor
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/

include_once('../../wysiwyg.inc');
?>

<html>
	<head>
		<title>Popup Editor</title>
		<script type="text/javascript">
			var parent_object = opener.editor_<?php echo $_REQUEST['editor_name']?>._object;

			function update_parent() {
				parent_object.setHTML(editor_popup.getHTML());
			}

			function resize_editor() {
				var newHeight;
				var newWidth;
				if (document.all) {
					// IE
					newHeight = document.body.offsetHeight - editor_popup._toolbar.offsetHeight;
					if (newHeight < 0) { newHeight = 0; }
					newWidth = document.body.offsetWidth;
					if (newWidth < 0) { newWidth = 0; }
				} else {
					// Gecko
					newHeight = window.innerHeight - editor_popup._toolbar.offsetHeight;
					newWidth  = window.innerWidth;
				}
				editor_popup._textArea.style.height = editor_popup._iframe.style.height = newHeight + "px";
				editor_popup._textArea.style.width = editor_popup._iframe.style.width = newWidth + "px";
			}

			function init() {
				setTimeout(function() {
					editor_popup._inPopup = true;
					editor_popup.setMode(parent_object._editMode);
					var html = parent_object.getHTML();
					html = editor_popup.make_absolute_urls(html);
					editor_popup.setHTML(html);

					// continuously update parent editor window
					setInterval(update_parent, 5000);

					// setup event handlers
					window.onresize = resize_editor;
					resize_editor();
				}, 333); // give it some time to meet the new frame
			}
		</script>
	</head>
	<body bgcolor="#CCCCCC" scroll="no" marginwidth="0" marginheight="0" leftmargin="0" topmargin="0" onload="init()" onUnload="update_parent()">
		<form style="margin: 0px; border: 1px solid; border-color: threedshadow threedhighlight threedhighlight threedshadow;">
			<?php
			$wysiwyg = new wysiwyg('popup', $_REQUEST['editor_web_path']);
			$wysiwyg->set_width('500');
			$wysiwyg->set_height('300');
			$wysiwyg->set_body_type('iframe');
			$wysiwyg->set_show_status_bar(false);

			$wysiwyg->add_relative_href_check('http[s]?://'.$_SERVER['HTTP_HOST'].str_replace('fullscreen.php', '', $_SERVER['PHP_SELF']).'(\?a=[0-9]+)', './$1');
			$rhc = unserialize(rawurldecode($_REQUEST['rhc']));
			foreach ($rhc as $find => $replace) $wysiwyg->add_relative_href_check(str_replace('\\\\\\', '\\', $find), str_replace('\\\\\\', '\\', $replace));

			$auc = unserialize(rawurldecode($_REQUEST['auc']));
			foreach ($rhc as $find => $replace) $wysiwyg->add_absolute_url_check(str_replace('\\\\\\', '\\', $find), str_replace('\\\\\\', '\\', $replace));
			$wysiwyg->add_absolute_url_check('\./\?(a=[0-9]+)', 'http://'.$_SERVER['HTTP_HOST'].$_REQUEST['editor_php_self'].'?$1');

			foreach (explode('|',$_REQUEST['editor_plugins']) as $name) {
				if (trim($name) == '') continue;
				$wysiwyg->add_plugin($name);
			}
			$wysiwyg->paint();
			?>
		</form>
	</body>
</html>