<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: fullscreen.php,v 1.8 2003/10/16 01:06:39 gsherwood Exp $
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
				if (document.all) {
					// IE
					newHeight = document.body.offsetHeight - editor_popup._toolbar.offsetHeight;
					if (newHeight < 0) { newHeight = 0; }
				} else {
					// Gecko
					newHeight = window.innerHeight - editor_popup._toolbar.offsetHeight;
				}
				editor_popup._textArea.style.height = editor_popup._iframe.style.height = newHeight + "px";
			}

			function init() {
				resize_editor();
				setTimeout(function() {
					editor_popup.setMode(parent_object._editMode);
					var html = parent_object.getHTML();
					html = editor_popup.make_absolute_urls(html);
					editor_popup.setHTML(html);

					// continuously update parent editor window
					setInterval(update_parent, 5000);

					// setup event handlers
					window.onresize = resize_editor;
				}, 333); // give it some time to meet the new frame
			}
		</script>
	</head>
	<body bgcolor="#CCCCCC" scroll="no" marginwidth="0" marginheight="0" leftmargin="0" topmargin="0" onload="init()" onUnload="update_parent()">
		<form style="margin: 0px; border: 1px solid; border-color: threedshadow threedhighlight threedhighlight threedshadow;">
			<?php
			$wysiwyg = new wysiwyg('popup', $_REQUEST['editor_web_path']);
			$wysiwyg->set_width('100%');
			$wysiwyg->set_height('100%');

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