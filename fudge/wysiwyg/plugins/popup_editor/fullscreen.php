<?php
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

			function _CloseOnEsc() {
				if (event.keyCode == 27) { window.close(); return; }
			}

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
					editor_popup.setHTML(parent_object.getHTML());

					// continuously update parent editor window
					setInterval(update_parent, 1000);

					// setup event handlers
					document.body.onkeypress = _CloseOnEsc;
					window.onresize = resize_editor;
				}, 333); // give it some time to meet the new frame
			}
		</script>
	</head>
	<body bgcolor="#CCCCCC" scroll="no" marginwidth="0" marginheight="0" leftmargin="0" topmargin="0" onload="init()">
		<form style="margin: 0px; border: 1px solid; border-color: threedshadow threedhighlight threedhighlight threedshadow;">
			<?php
			$wysiwyg = new wysiwyg('popup', $_REQUEST['editor_web_path']);
			$wysiwyg->set_width('100%');
			$wysiwyg->set_height('100%');
			foreach (explode('|',$_REQUEST['editor_plugins']) as $name) {
				if (trim($name) == '') continue;
				$wysiwyg->add_plugin($name);
			}
			$wysiwyg->paint();
			?>
		</form>
	</body>
</html>