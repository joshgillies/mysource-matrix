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
* $Id: core.js,v 1.26.2.1 2006/09/11 02:13:09 bcaldwell Exp $
*
*/

// This code is not generated by the script and is not affected by
// any plugins of HTMLArea. Thus, it has been placed in a separate
// include file for caching
//
// Modifications for PHP Plugin Based System
//           developed by Greg Sherwood for Squiz.Net.
//           http://www.squiz.net/
//           mailto:greg@squiz.net
//

/** Helper function: replace all TEXTAREA-s in the document with HTMLArea-s. */
HTMLArea.replaceAll = function() {
	var tas = document.getElementsByTagName("textarea");
	for (var i = tas.length; i > 0; (new HTMLArea(tas[--i])).generate());
};


// Create the status bar to tack onto the bottom of the editor
HTMLArea.prototype._createStatusBar = function() {
	var div = document.createElement("div");
	div.className = "htmlarea-statusBar";
	this._statusBar = div;
	div.appendChild(document.createTextNode(HTMLArea.I18N.msg["Path"] + ": "));

	// creates a holder for the path view
	span = document.createElement("span");
	span.className = "htmlarea-statusBarTree";
	this._statusBarTree = span;
	this._statusBar.appendChild(span);

	if (!this.config.statusBar) {
		// disable it...
		this._statusBar.style.display = "none";
	}
};



// Creates the HTMLArea object and replaces the textarea with it.
HTMLArea.prototype.generate = function () {
	var editor = this; // we'll need "this" in some nested functions
	// get the textarea
	var textarea = this._textArea;
	if (typeof textarea == "string") {
		// it's not element but ID
		this._textArea = textarea = document.getElementById(textarea);
	}
	this._ta_size = {
		w: textarea.offsetWidth,
		h: textarea.offsetHeight
	};

	// hide the textarea
	textarea.style.display = "none";

	// create the editor framework
	var htmlarea = document.createElement("div");
	htmlarea.id = "htmlarea";
	htmlarea.className = "htmlarea";
	this._htmlArea = htmlarea;

	// insert the editor before the textarea.
	textarea.parentNode.insertBefore(htmlarea, textarea);

	// retrieve the HTML on submit
	eval ('var otherOnSubmit_' + this._uniqueID + '= (textarea.form.onsubmit != null) ? textarea.form.onsubmit :  (function() {return true;});');
	eval ('textarea.form.onsubmit = function() { editor._formSubmit(); return otherOnSubmit_' + this._uniqueID + '(); };');
	// add a handler for the "back/forward" case -- on body.unload we save
	// the HTML content into the original textarea.
	eval ('var otherOnUnload_' + this._uniqueID + '= (window.onunload) ? window.onunload :  new Function;');
	eval ('window.onunload = function() { editor._textArea.value = editor.getHTML(); otherOnUnload_' + this._uniqueID + '(); };');

	// appends the toolbar
	this._htmlArea.appendChild(this._toolbar);

	// appends the status bar
	this._htmlArea.appendChild(this._statusBar);

	if (HTMLArea.is_gecko || editor.config.bodyType.toLowerCase() == 'iframe') {
		// create the IFRAME
		var iframe = document.createElement("iframe");
		htmlarea.appendChild(iframe);
		this._iframe = iframe;

		// resize the iframe after the window is loaded
		HTMLArea._addEvent(window, "load", function (event) {
			editor._resizeIframe(HTMLArea.is_ie ? window.event : event);
		});
	} else if (HTMLArea.is_ie && editor.config.bodyType.toLowerCase() == 'div') {
		var contentDiv = document.createElement("div");
		this._htmlArea.appendChild(contentDiv);
		this._doc = contentDiv;
	}

	// IMPORTANT: we have to allow Mozilla a short time to recognize the
	// new frame. Otherwise we get a stupid exception.
	function initIframe() {
		var doc = editor._iframe.contentWindow.document;
		if (!doc) {
			if (HTMLArea.is_gecko) {
				setTimeout(function () { editor._initIframe(); }, 10);
				return false;
			} else {
				alert(js_translate('unable_to_initialise_iframe'));
			}
		}
		if (HTMLArea.is_gecko) {
			// enable editable mode for Mozilla
			doc.designMode = "on";
		}
		editor._doc = doc;
		doc.open();
		var html = "<html>\n";
		html += "<head>\n";
		if (editor.config.styleSheet) { html += "<link rel=\"stylesheet\" href=\"" + editor.config.styleSheet + "\" type=\"text/css\">"; }
		html += "<style> body { " + editor.config.bodyStyle + " }\n";
		html += ".wysiwyg-noborders { border: 1px dashed #3366CC; }\n";
		html += "</style>\n";
		html += "</head>\n";
		html += "<body>\n";
		html += decodeURIComponent(editor._textArea.value);
		html += "</body>\n";
		html += "</html>";
		doc.write(html);
		doc.close();
		editor._docContent = doc.body;

		if (HTMLArea.is_ie) {
			// enable editable mode for IE.  For some reason this doesn't
			// work if done in the same place as for Gecko (above).
			doc.body.contentEditable = true;
		}

		editor.focusEditor();
		// intercept some events; for updating the toolbar & keyboard handlers
		HTMLArea._addEvents
			(doc, ["keydown", "keypress", "mousedown", "mouseup", "drag"],
			 function (event) {
				 return editor._editorEvent(HTMLArea.is_ie ? editor._iframe.contentWindow.event : event);
			 });

		editor._initialised = true;
		editor.updateToolbar();
		editor.focusEditor();

	};

	function initDiv() {

		if (editor.config.width == null) {
			editor._doc.style.width  = '100%';
		} else {
			editor._doc.style.width  = editor.config.width;
		}

		if (editor.config.width == null) {
			editor._doc.style.height = '100%';
		} else {
			editor._doc.style.height = editor.config.height;
		}


		editor._doc.innerHTML = decodeURIComponent(editor._textArea.value);
		if (HTMLArea.is_ie) { editor._doc.contentEditable = true; }
		// intercept some events; for updating the toolbar & keyboard handlers
		HTMLArea._addEvents
			(editor._doc, ["keydown", "keypress", "mousedown", "mouseup", "drag"],
			 function (event) {
				 return editor._editorEvent(event);
			 });
		editor._docContent = contentDiv;
		editor._doc = editor._docContent.document;
		editor.updateToolbar();
		editor.focusEditor();
		editor._initialised = true;
	};

	if (HTMLArea.is_gecko || editor.config.bodyType.toLowerCase() == 'iframe') {
		setTimeout(initIframe, HTMLArea.is_gecko ? 10 : 0);
	} else if (HTMLArea.is_ie && editor.config.bodyType.toLowerCase() == 'div') {
		setTimeout(initDiv, 0);
	}
};


// Switches editor mode; parameter can be "textmode" or "wysiwyg"
HTMLArea.prototype.setMode = function(mode, noFocus) {
	if (this._editMode == mode) return false;
	switch (mode) {
		case "textmode":
			var html = this.getHTML();
			if (HTMLArea.is_gecko) {
				var html = document.createTextNode(html);
				this._iframe.contentWindow.document.body.innerHTML = "";
				this._iframe.contentWindow.document.body.appendChild(html);
			} else if (HTMLArea.is_ie) {
				this._docContent.innerText = html;
			}
			if (this.config.statusBar) {
				this._statusBar.innerHTML = HTMLArea.I18N.msg["TEXT_MODE"];
			}
		break;
		case "wysiwyg":
			var html = this.getHTML();
			if (HTMLArea.is_gecko) {
				this._iframe.contentWindow.document.body.innerHTML = html;
			} else if (HTMLArea.is_ie) {
				this._docContent.innerHTML = html;
			}
			if (this.config.statusBar) {
				this._statusBar.innerHTML = '';
				this._statusBar.appendChild(document.createTextNode(HTMLArea.I18N.msg["Path"] + ": "));
				this._statusBar.appendChild(this._statusBarTree);
			}
		break;
		default:
			alert(js_translate('undefined_mode', '<' + mode + '>'));
		return false;
	}
	this._editMode = mode;
	if (!noFocus) { this.focusEditor(); }
};


// focuses the iframe window.  returns a reference to the editor document.
HTMLArea.prototype.focusEditor = function() {
	if (HTMLArea.is_gecko) {
		this._iframe.contentWindow.focus();
	} else if (HTMLArea.is_ie) {
		this._docContent.focus();
	}
	return this._docContent;
};


/**
 * Returns a node after which we can insert other nodes, in the current
 * selection.  The selection is removed.  It splits a text node, if needed.
 */
HTMLArea.prototype.insertNodeAtSelection = function(toBeInserted, range) {
	if (!HTMLArea.is_ie) {
		var sel = this._getSelection();
		if (range == null) {
			var range = this._createRange(sel);
		}
		// remove the current selection
		range.deleteContents();
		var node = range.startContainer;
		var pos = range.startOffset;
		switch (node.nodeType) {
			case 3: // Node.TEXT_NODE
				// we have to split it at the caret position.
				if (toBeInserted.nodeType == 3) {
					// do optimized insertion
					node.insertData(pos, toBeInserted.data);
					range.setEnd(node, pos + toBeInserted.length);
					range.setStart(node, pos + toBeInserted.length);
				} else {
					node = node.splitText(pos);
					node.parentNode.insertBefore(toBeInserted, node);
					range.setEnd(node, 0);
					range.setStart(node, 0);
				}
			break;
			case 1: // Node.ELEMENT_NODE
				range.insertNode(toBeInserted);
			break;
		}
		sel.addRange(range);
	} else {
		return null; // this function not yet used for IE <FIXME>
	}
};


/**
 * Call this function to insert HTML code at the current position.  It deletes
 * the selection, if any.
 */
HTMLArea.prototype.insertHTML = function(html, range) {
	if (range == null) {
		var sel = this._getSelection();
		var range = this._createRange(sel);
		var node = range.startContainer;
		if (!HTMLArea.is_ie) {
			if (node.tagName == 'HTML') {
				return false;
			}
		}
	}
	if (HTMLArea.is_ie) {
		range.pasteHTML(html);
	} else {
		// construct a new document fragment with the given HTML
		var fragment = this._doc.createDocumentFragment();
		var div = this._doc.createElement("div");
		div.innerHTML = html;
		while (div.firstChild) {
			// the following call also removes the node from div
			fragment.appendChild(div.firstChild);
		}
		// this also removes the selection
		var node = this.insertNodeAtSelection(fragment, range);
	}
};


// completely change the HTML inside
HTMLArea.prototype.setHTML = function(html) {
	switch (this._editMode) {
		case "textmode":
			if (HTMLArea.is_gecko) {
				var html = document.createTextNode(html);
				this._iframe.contentWindow.document.body.innerHTML = "";
				this._iframe.contentWindow.document.body.appendChild(html);
			} else if (HTMLArea.is_ie) {
				this._docContent.innerText = html;
			}
		break;
		case "wysiwyg":
			if (HTMLArea.is_gecko) {
				this._iframe.contentWindow.document.body.innerHTML = html;
			} else if (HTMLArea.is_ie) {
				this._docContent.innerHTML = html;
			}
		break;
		default:
			alert(js_translate('undefined_mode', '<' + mode + '>'));
	}
	return false;
};


/**
 *  Call this function to surround the existing HTML code in the selection with
 *  your tags.
 */
HTMLArea.prototype.surroundHTML = function(startTag, endTag) {
	var html = this.getSelectedHTML();
	// the following also deletes the selection
	this.insertHTML(startTag + html + endTag);
};


// Retrieve the selected block
HTMLArea.prototype.getSelectedHTML = function() {
	var sel = this._getSelection();
	var range = this._createRange(sel);
	var existing = null;
	if (HTMLArea.is_ie) {
		existing = range.htmlText;
	} else {
		existing = HTMLArea.getHTML(range.cloneContents(), false);
	}
	return existing;
};


// resize the iframe
HTMLArea.prototype._resizeIframe = function(ev) {
	// This function creates a new textarea of the
	// same width and height as the original, but appends
	// it to the body of the document.
	if (this.config.width != "auto" && this.config.height != "auto") {
		this._iframe.style.width = this.config.width;
		this._iframe.style.height = this.config.height;
		return;
	}

	var textarea = document.createElement("textarea");
	document.body.appendChild(textarea);

	// grab the attributes that determine size
	textarea.style.width = this._textArea.style.width;
	textarea.style.height = this._textArea.style.height;
	textarea.rows = this._textArea.rows;
	textarea.cols = this._textArea.cols;
	textarea.className = this._textArea.className;

	// get the width and height of the text area
	var width = textarea.offsetWidth;
	var height = textarea.offsetHeight;

	// hide the textarea
	textarea.style.display = "none";

	if (this.config.sizeIncludesToolbar) {
		// substract toolbar height
		height -= this._toolbar.offsetHeight;
	}

	// size the IFRAME according to user's prefs or initial textarea
	height = (this.config.height == "auto" ? (height + "px") : this.config.height);
	this._iframe.style.height = height + "px";
	width = (this.config.width == "auto" ? (width + "px") : this.config.width);
	this._iframe.style.width = width;
};