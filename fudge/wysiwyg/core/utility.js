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
* $Id: utility.js,v 1.20.2.1 2006/01/17 00:29:55 skim Exp $
*
*/

// This code is not generated by the script and is not affected by
// any plugins of HTMLArea. Thus, it has been placed in a separate
// include file for caching
//
// Modifications for PHP Plugin Based System
//           developed by Greg Sherwood for Squiz.Net.
//           http://www.squiz.net/
//           greg@squiz.net
//

// browser identification
HTMLArea.agt = navigator.userAgent.toLowerCase();
HTMLArea.is_ie     = ((HTMLArea.agt.indexOf("msie") != -1) && (HTMLArea.agt.indexOf("opera") == -1));
HTMLArea.is_opera  = (HTMLArea.agt.indexOf("opera") != -1);
HTMLArea.is_mac    = (HTMLArea.agt.indexOf("mac") != -1);
HTMLArea.is_mac_ie = (HTMLArea.is_ie && HTMLArea.is_mac);
HTMLArea.is_win_ie = (HTMLArea.is_ie && !HTMLArea.is_mac);
HTMLArea.is_gecko  = (navigator.product == "Gecko");


// variable used to pass the object to the popup editor window.
HTMLArea._object = null;


// FIXME!!! this should return false for IE < 5.5
HTMLArea.checkSupportedBrowser = function() {
	if (HTMLArea.is_gecko) {
		if (navigator.productSub < 20021201) {
			alert(js_translate('mozilla13_alpha_not_supported'));
			return false;
		}
		if (navigator.productSub < 20030210) {
			alert(js_translate('mozilla13_beta_not_supported'));
		}
	}
	return HTMLArea.is_gecko || HTMLArea.is_ie;
};


// selection & ranges

// returns the current selection object
HTMLArea.prototype._getSelection = function() {
	if (HTMLArea.is_ie) {
		if (!this._doc) return null;
		return this._doc.selection;
	} else {
		if (!this._iframe) return null;
		return this._iframe.contentWindow.getSelection();
	}
};


// returns a range for the current selection
HTMLArea.prototype._createRange = function(sel) {
	if (HTMLArea.is_ie) {
		return sel.createRange();
	} else {
		this.focusEditor();
		if (typeof sel != "undefined") {
			return sel.getRangeAt(0);
		} else {
			return this._doc.createRange();
		}
	}
};


// returns a range for the current selection
HTMLArea.prototype._createTextRange = function(sel) {
	if (HTMLArea.is_ie) {
		return this._doc.body.createTextRange();
	} else {
		this.focusEditor();
		if (sel) {
			return sel.getRangeAt(0);
		} else {
			return this._doc.body.createTextRange();
		}
	}
};


// Returns the deepest node that contains both endpoints of the selection.
HTMLArea.prototype.getParentElement = function() {
	var sel = this._getSelection();
	if (sel == null) return [];
	var range = this._createRange(sel);
	if (HTMLArea.is_ie) {
		return range.parentElement ? range.parentElement() : this._docContent;
	} else {
		var p = range.commonAncestorContainer;
		while (p.nodeType == 3) {
			p = p.parentNode;
		}
		return p;
	}
};


// Returns an array with all the ancestor nodes of the selection.
HTMLArea.prototype.getAllAncestors = function() {
	var p = this.getParentElement();
	var a = [];
	while (p && (p.nodeType == 1) && (p.tagName.toLowerCase() != 'body')) {
		a.push(p);
		p = p.parentNode;
	}
	a.push(this._docContent);
	return a;
};


// retrieves the closest element having the specified tagName in the list of
// ancestors of the current selection/caret.
HTMLArea.prototype.getClosest = function(tagName) {
	var ancestors = this.getAllAncestors();
	var ret = null;
	tagName = ("" + tagName).toLowerCase();
	for (var i in ancestors) {
		var el = ancestors[i];
		if (el && el.tagName && el.tagName.toLowerCase() == tagName) {
			ret = el;
			break;
		}
	}
	return ret;
};


// Selects the contents inside the given node
HTMLArea.prototype.selectNodeContents = function(node, pos) {
	this.focusEditor();
	this.forceRedraw();
	var range;
	var collapsed = (typeof pos != "undefined");
	if (HTMLArea.is_ie) {
		range = this._doc.body.createTextRange();
		range.moveToElementText(node);
		(collapsed) && range.collapse(pos);
		range.select();
	} else {
		var sel = this._getSelection();
		range = this._doc.createRange();
		range.selectNodeContents(node);
		(collapsed) && range.collapse(pos);
		sel.removeAllRanges();
		sel.addRange(range);
	}
};


HTMLArea.prototype.forceRedraw = function() {
	this._doc.body.style.visibility = "hidden";
	this._doc.body.style.visibility = "visible";
};


// event handling

HTMLArea._addEvent = function(el, evname, func) {
	if (HTMLArea.is_ie) {
		el.attachEvent("on" + evname, func);
	} else {
		el.addEventListener(evname, func, true);
	}
};


HTMLArea._addEvents = function(el, evs, func) {
	for (var i in evs) {
		HTMLArea._addEvent(el, evs[i], func);
	}
};


HTMLArea._removeEvent = function(el, evname, func) {
	if (HTMLArea.is_ie) {
		el.detachEvent("on" + evname, func);
	} else {
		el.removeEventListener(evname, func, true);
	}
};


HTMLArea._removeEvents = function(el, evs, func) {
	for (var i in evs) {
		HTMLArea._removeEvent(el, evs[i], func);
	}
};


HTMLArea._stopEvent = function(ev) {
	if (HTMLArea.is_ie) {
		ev.cancelBubble = true;
		ev.returnValue = false;
	} else {
		ev.preventDefault();
		ev.stopPropagation();
	}
};


HTMLArea._removeClass = function(el, className) {
	if (!(el && el.className)) {
		return;
	}
	var cls = el.className.split(" ");
	var ar = new Array();
	for (var i = cls.length; i > 0;) {
		if (cls[--i] != className) {
			ar[ar.length] = cls[i];
		}
	}
	el.className = ar.join(" ");
};


HTMLArea._addClass = function(el, className) {
	// remove the class first, if already there
	HTMLArea._removeClass(el, className);
	el.className += " " + className;
};


HTMLArea._hasClass = function(el, className) {
	if (!(el && el.className)) {
		return false;
	}
	var cls = el.className.split(" ");
	for (var i = cls.length; i > 0;) {
		if (cls[--i] == className) {
			return true;
		}
	}
	return false;
};


HTMLArea._isBlockElement = function(el) {
	var blockTags = " body form textarea fieldset ul ol dl li div " +
		"p h1 h2 h3 h4 h5 h6 quote pre table thead " +
		"tbody tfoot tr td iframe address ";
	return (blockTags.indexOf(" " + el.tagName.toLowerCase() + " ") != -1);
};


HTMLArea._needsClosingTag = function(el) {
	var closingTags = " script style div span tr td tbody table em strong font a ";
	return (closingTags.indexOf(" " + el.tagName.toLowerCase() + " ") != -1);
};


// performs HTML encoding of some given string
HTMLArea.htmlEncode = function(str) {
	// we don't need regexp for that, but.. so be it for now.
	str = str.replace(/&/ig, "&amp;");
	str = str.replace(/</ig, "&lt;");
	str = str.replace(/>/ig, "&gt;");
	str = str.replace(/\x22/ig, "&quot;");
	// \x22 means '"' -- we use hex reprezentation so that we don't disturb
	// JS compressors (well, at least mine fails.. ;)
	return str;
};


// Retrieves the HTML code from the given node.  This is a replacement for
// getting innerHTML, using standard DOM calls.
HTMLArea.getHTML = function(root, outputRoot) {
	var html = "";
	switch (root.nodeType) {
		case 1: // Node.ELEMENT_NODE
		case 11: // Node.DOCUMENT_FRAGMENT_NODE
			var closed;
			var i;
			if (outputRoot) {
				closed = (!(root.hasChildNodes() || HTMLArea._needsClosingTag(root)));
				html = "<" + root.tagName.toLowerCase();
				var attrs = root.attributes;
				for (i = 0; i < attrs.length; ++i) {
					var a = attrs.item(i);
					if (!a.specified) {
						continue;
					}
					var name = a.name.toLowerCase();
					if (name.substr(0, 4) == "_moz") {
						// Mozilla reports some special tags
						// here; we don't need them.
						continue;
					}
					var value;
					if (name != 'style') {
						// IE5.5 reports 25 when cellSpacing is
						// 1; other values might be doomed too.
						// For this reason we extract the
						// values directly from the root node.
						// I'm starting to HATE JavaScript
						// development.  Browser differences
						// suck.
						if (typeof root[a.nodeName] != "undefined") {
							value = root[a.nodeName];
						} else {
							value = a.nodeValue;
						}
					} else { // IE fails to put style in attributes list
						value = root.style.cssText.toLowerCase();
					}
					if (/_moz/.test(value)) {
						// Mozilla reports some special tags
						// here; we don't need them.
						continue;
					}
					html += " " + name + '="' + value + '"';
				}
				html += closed ? " />" : ">";
			}
			for (i = root.firstChild; i; i = i.nextSibling) {
				html += HTMLArea.getHTML(i, true);
			}
			if (outputRoot && !closed) {
				html += "</" + root.tagName.toLowerCase() + ">";
			}
		break;
			case 3: // Node.TEXT_NODE
			html = HTMLArea.htmlEncode(root.data);
		break;
			case 8: // Node.COMMENT_NODE
			html = "<!--" + root.data + "-->";
		break;		// skip comments, for now.
	}
	return html;
};


// creates a rgb-style color from a number
HTMLArea._makeColor = function(v) {
	if (typeof v != "number") {
		// already in rgb (hopefully); IE doesn't get here.
		return v;
	}
	// IE sends number; convert to rgb.
	var r = v & 0xFF;
	var g = (v >> 8) & 0xFF;
	var b = (v >> 16) & 0xFF;
	return "rgb(" + r + "," + g + "," + b + ")";
};


// returns hexadecimal color representation from a number or a rgb-style color.
HTMLArea._colorToRgb = function(v) {
	// if v is null, they've probably selected text
	// with more than one color or something, so return
	// null here because other tests will fail
	if (!v) return null;

	// returns the hex representation of one byte (2 digits)
	function hex(d) {
		return (d < 16) ? ("0" + d.toString(16)) : d.toString(16);
	};

	if (typeof v == "number") {
		// we're talking to IE here
		var r = v & 0xFF;
		var g = (v >> 8) & 0xFF;
		var b = (v >> 16) & 0xFF;
		return "#" + hex(r) + hex(g) + hex(b);
	}

	if (v.substr(0, 3) == "rgb") {
		// in rgb(...) form -- Mozilla
		var re = /rgb\s*\(\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)/;
		if (v.match(re)) {
			var r = parseInt(RegExp.$1);
			var g = parseInt(RegExp.$2);
			var b = parseInt(RegExp.$3);
			return "#" + hex(r) + hex(g) + hex(b);
		}
		// doesn't match RE?!  maybe uses percentages or float numbers
		// -- FIXME: not yet implemented.
		return null;
	}

	if (v[0] == "#") {
		// already hex rgb (hopefully :D )
		return v;
	}

	// if everything else fails ;)
	return null;
};


HTMLArea.getLength = function(value) {
	var len = parseInt(value);
	if (isNaN(len)) {
		len = "";
	}
	return len;
};


// Applies the style found in "params" to the given element.
HTMLArea.processStyle = function(params, element) {
	var style = element.style;
	for (var i in params) {
		var val = params[i];
		switch (i) {
			case "f_bgcolor":
				style.backgroundColor = val;
			break;

			case "f_color":
				style.color = val;
			break;

			case "f_backgroundImage":
				if (/\S/.test(val)) {
					style.backgroundImage = "url(" + val + ")";
				} else {
					style.backgroundImage = "none";
				}
			break;

			case "f_borderWidth":
				val = parseInt(val);
				if (isNaN(val)) { val = 0; }
				style.borderWidth = val;
			break;

			case "f_borderStyle":
				style.borderStyle = val;
			break;

			case "f_borderColor":
				style.borderColor = val;
			break;

			case "f_borderCollapse":
				style.borderCollapse = val ? "collapse" : "";
			break;

			case "f_width":
				if (/\S/.test(val)) {
					style.width = val + params["f_widthUnit"];;
				} else {
					style.width = "";
				}
			break;

			case "f_height":
				if (/\S/.test(val)) {
					style.height = val + params["f_heightUnit"];;
				} else {
					style.height = "";
				}
			break;

			case "f_textAlign":
				if (val == "char") {
					var ch = params["f_st_textAlignChar"];
					if (ch == '"') {
						ch = '\\"';
					}
					style.textAlign = '"' + ch + '"';
				} else {
					style.textAlign = val;
				}
			break;

			case "f_verticalAlign":
				style.verticalAlign = val;
			break;

			case "f_float":
				style.cssFloat = val;
			break;
		}
	}
};


// receives an URL to the popup dialog and a function that receives one value;
// this function will get called after the dialog is closed, with the return
// value of the dialog.
HTMLArea.prototype._popupDialog = function(code, url, width, height, modal, action, args) {
	if (modal == true) {
		openModalDialog(code, this.pluginURL(url), width, height, action, args)
	} else {
		openDialog(code, this.pluginURL(url), width, height, action, args);
	}
};


// paths
HTMLArea.prototype.imgURL = function(file) {
	return this.config.editorURL + this.config.imgURL + file;
};


HTMLArea.prototype.pluginURL = function(file) {
	return this.config.editorURL + this.config.pluginURL + file;
};