/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: edit.js,v 1.13 2003/11/11 03:56:09 gsherwood Exp $
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
			top.main.document.focus();
			submit_form();
		break;

		case "v" :
			// preview the asset on the frontend in a new window
			top.main.document.focus();
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