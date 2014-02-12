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
* $Id: email_format.js,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

function emailFormatSwitchReadingMode(editor_name, html_label, text_label) {
	var textDiv = document.getElementById(editor_name + "_text_format");
	var htmlDiv = document.getElementById(editor_name + "_html_format");
	var formatSpan = document.getElementById(editor_name + "_format_mode");

	if (htmlDiv.style.display == "none") {

		htmlDiv.style.display = "";
		textDiv.style.display = "none";
		formatSpan.innerHTML = html_label; //'HTML Email Version';

	} else {

		textDiv.style.display = "";
		htmlDiv.style.display = "none";
		formatSpan.innerHTML = text_label; //'Text Email Version';
	}

}//end emailFormatSwitchReadingMode()

var initialisedEmailEditors = new Array();
function emailFormatSwitchEditingMode(editor_name, html_label, text_label, html_type, text_type) {
	
	var textDiv = document.getElementById(editor_name + "_text_body_div");
	var htmlDiv = document.getElementById(editor_name + "_html_body_div");
	var viperDiv = document.getElementById(editor_name + "_contents_div_viper");
	var formatSpan = document.getElementById(editor_name + "_format_mode");
	var typeStrong = document.getElementById(editor_name + "_content_type");

	if (htmlDiv.style.display == "none") {
		if(typeof Matrix_Viper === 'undefined')  {
		    var editor = eval('editor_' + editor_name);
		    var setDesignMode = true;

		    // initilise the wysiwg if this is the first time
		    // it is being shown - skip this otherwise
		    if (initialisedEmailEditors[editor._uniqueID] == null) {
			    initialisedEmailEditors[editor._uniqueID] = true;
			    editor.generate();
			    setDesignMode = false;
		    } else if (editor._initialised != true) {
			    return;
		    }


		    // if we are using an iframe for this editor, we set its designMode property if we need to
		    if (editor._iframe) {
			    editor._iframe.style.width = editor.config.width;
			    if (editor._iframe.contentWindow.document.designMode) {
				    editor._iframe.contentWindow.document.designMode = "on";
			    }
			    editor._iframe.style.height = editor.config.height;
		    }
		}
		else {
		    // init viper editor
		     //Matrix_Viper.viper.setEditableElement(viperDiv);
		}
		
		    textDiv.style.display = "none";
		    htmlDiv.style.display = "";
		    formatSpan.innerHTML = html_label; //'HTML Email Version';
		    typeStrong.innerHTML = html_type; //'WYSIWYG Content Container';


	} else {

		textDiv.style.display = "";
		htmlDiv.style.display = "none";
		formatSpan.innerHTML = text_label; //'Text Email Version';
		typeStrong.innerHTML = text_type; //'Raw HTML Content Container';
	}

}//end emailFormatSwitchEditingMode()