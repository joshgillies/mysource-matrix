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
* $Id: edit.js,v 1.16 2003/11/26 00:51:13 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

// if the browser is IE, regiter the onkeydown event
if(document.all) { document.onkeydown = sq_process_key_down; }


// Execute keyboard shortcuts for IE browsers
function sq_process_key_down() {

	var key;
	
	// was the ALT key pressed?
	if(!event.altKey) return true;

	// okay, ALT was pressed - but what other key was pressed?
	key = String.fromCharCode(event.keyCode);
	key = key.toLowerCase();
	
	switch (key) {
		case "s" :
			// emulate pressing of the commit button
			if (top.main) { top.main.document.focus(); }
			submit_form();
		break;

		case "v" :
			// preview the asset on the frontend in a new window
			if (top.main) { top.main.document.focus(); }
			if (document.main_form.sq_preview_url) {
				preview_popup = window.open(document.main_form.sq_preview_url.value, 'preview', '');
			}
		break;
	}//end switch

}//end sq_process_key_down()


// redirect the user to another page with a friendly message
// and a manual click they can click if something goes wrong
function sq_redirect(url) {

	document.write("<html>");
	document.write("	<head>");
	document.write("		<style type=\"text/css\">");
	document.write("			body {");
	document.write("				font-size:			12px;");
	document.write("				font-family:		Arial, Verdana Helvetica, sans-serif;");
	document.write("				color:				#000000;");
	document.write("				background-color:	#FFFFFF;");
	document.write("			}");
	document.write("		</style>");
	document.write("	</head>");
	document.write("	<body>");
	document.write("		Please wait while you are redirected. If you are not redirected, please click <a href=\"" + url + "\" title=\"Click here to manually redirect\">here</a>");
	document.write("	</body>");
	document.write("</html>");
	window.location = url;

}//end sq_redirect()


// This function switches the view between wysiwyg and
// page content for a wysiwyg editing field on the page
var initialisedEditors = new Array();
function switchEditingMode(contentDivID, editDivID, editor) {
	var contentDiv = document.getElementById(contentDivID); // div with page contents
	var editDiv = document.getElementById(editDivID);       // div with wysiwyg

	if (editDiv.style.display == "none") { // the edit div is hidden
		var setDesignMode = true;

		// initilise the wysiwg if this is the first time
		// it is being shown - skip this otherwise
		if (initialisedEditors[editor._uniqueID] == null) {
			initialisedEditors[editor._uniqueID] = true;
			editor.generate();
			//editor.updateToolbar(false);
			setDesignMode = false;
		} else if (editor._initialised != true) {
			return;
		}

		editDiv.style.display = ""; // show the wysiwyg

		// if we are using an iframe for this editor, we resize it
		// and set its designMode property if we need to
		if (editor._iframe) {
			editor._iframe.style.width = "100%";
			if (setDesignMode && HTMLArea.is_gecko) { editor._iframe.contentWindow.document.designMode = "on"; }
			var iframeHeight = contentDiv.offsetHeight - editor._toolbar.offsetHeight - editor._statusBar.offsetHeight;
			if (iframeHeight < editor.config.height) { iframeHeight = editor.config.height; }
			editor._iframe.style.height = iframeHeight + 'px';
		}
		contentDiv.style.display = "none"; // hide the contents
	} else if (editor._initialised == true) {
		// the content div is hidden and the
		// wysiwyg editor has been initialised
		contentDiv.innerHTML = editor.getHTML();
		contentDiv.style.display = "";  // show the contents
		editDiv.style.display = "none"; // hide the wysiwyg

		if (editor._iframe) {
			var editCell = document.getElementById(editor._uniqueID + "_cell");
			if (editCell) { editCell.style.height = "100%"; }
		}
	}
}