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
* $Id: htmlarea-lang-en.js,v 1.8 2012/08/30 00:56:51 ewang Exp $
*
*/

// I18N constants

// LANG: "en", ENCODING: UTF-8 | ISO-8859-1
// Author: Mihai Bazon, <mishoo@infoiasi.ro>

// FOR TRANSLATORS:
//
//   1. PLEASE PUT YOUR CONTACT INFO IN THE ABOVE LINE
//      (at least a valid email address)
//
//   2. PLEASE TRY TO USE UTF-8 FOR ENCODING;
//      (if this is not possible, please include a comment
//       that states what encoding is necessary.)

HTMLArea.I18N = {

	// the following should be the filename without .js extension
	// it will be used for automatically load plugin language.
	lang: "en",

	tooltips: {
		bold:           js_translate("Bold"),
		italic:         js_translate("Italic"),
		underline:      js_translate("Underline"),
		strikethrough:  js_translate("Strikethrough"),
		subscript:      js_translate("Subscript"),
		superscript:    js_translate("Superscript"),
		justifyleft:    js_translate("Justify Left"),
		justifycenter:  js_translate("Justify Center"),
		justifyright:   js_translate("Justify Right"),
		justifyfull:    js_translate("Justify Full"),
		orderedlist:    js_translate("Ordered List"),
		unorderedlist:  js_translate("Bulleted List"),
		outdent:        js_translate("Decrease Indent"),
		indent:         js_translate("Increase Indent"),
		forecolor:      js_translate("Font Color"),
		hilitecolor:    js_translate("Background Color"),
		horizontalrule: js_translate("Horizontal Rule"),
		createlink:     js_translate("Insert Web Link"),
		insertimage:    js_translate("Insert Image"),
		inserttable:    js_translate("Insert Table"),
		htmlmode:       js_translate("Toggle HTML Source"),
		popupeditor:    js_translate("Enlarge Editor"),
		about:          js_translate("About this editor"),
		showhelp:       js_translate("Help using editor"),
		textindicator:  js_translate("Current style"),
		undo:           js_translate("Undoes your last action"),
		redo:           js_translate("Redoes your last action"),
		cut:            js_translate("Cut selection"),
		copy:           js_translate("Copy selection"),
		paste:          js_translate("Paste from clipboard")
	},

	buttons: {
		"ok":           js_translate("OK"),
		"cancel":       js_translate("Cancel")
	},

	msg: {
		"Path":         js_translate("Path"),
		"TEXT_MODE":    js_translate("You are in TEXT MODE.  Use the [<>] button to switch back to WYSIWYG.")
	}
};
