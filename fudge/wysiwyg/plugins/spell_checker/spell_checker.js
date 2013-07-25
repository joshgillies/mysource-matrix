/**
* Copyright (c) 2002 - interactivetools.com, inc.
* Portions Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: spell_checker.js,v 1.9 2013/07/25 23:25:17 lwright Exp $
*
*/


var is_ie = window.opener.HTMLArea.is_ie;
var frame = null;
var currentElement = null;
var wrongWords = null;
var modified = false;
var allWords = {};


function makeCleanDoc(leaveFixed) {
	for (var i in wrongWords) {
		var el = wrongWords[i];
		if (!(leaveFixed && /HA-spellcheck-fixed/.test(el.className))) {
			el.parentNode.insertBefore(el.firstChild, el);
			el.parentNode.removeChild(el.nextSibling);
			el.parentNode.removeChild(el);
		} else {
			el.className = "HA-spellcheck-fixed";
			el.parentNode.removeChild(el.nextSibling);
		}
	}
	return frame.contentWindow.document.body.innerHTML;
};


/**
* changes the current dictionary to another one
*/
function recheckClicked() {
	document.getElementById("status").innerHTML = js_translate('changing_dictionary', document.getElementById("f_dictionary").value);
	var field = document.getElementById("f_content");
	field.value = makeCleanDoc(true);
	field.form.submit();
};


/**
* replaces a word in the iframe with the one selected by the user
*/
function replaceWord(el) {
	var replacement = document.getElementById("v_replacement").value;
	modified = (el.innerHTML != replacement);
	if (el) {
		el.className = el.className.replace(/\s*HA-spellcheck-(hover|fixed)\s*/g, " ");
	}
	el.className += " HA-spellcheck-fixed";
	el.__msh_fixed = true;
	if (!modified) {
		return false;
	}
	el.innerHTML = replacement;
};


/**
* capsture the replace button on a click
*/
function replaceClicked() {
	replaceWord(currentElement);
	var start = currentElement.__msh_id;
	var index = start;
	do {
		++index;
		if (index == wrongWords.length) {
			index = 0;
		}
	} while ((index != start) && wrongWords[index].__msh_fixed);
	if (index == start) {
		index = 0;
		alert(js_translate('spellcheck_complete'));
	}
	wrongWords[index].onclick();
	return false;
};


/**
* capture the replace all button on a click
*/
function replaceAllClicked() {
	var replacement = document.getElementById("v_replacement").value;
	var ok = true;
	var spans = allWords[currentElement.__msh_origWord];
	if (spans.length == 0) {
		alert(js_translate('impossible_condition_occured'));
	} else if (spans.length == 1) {
		replaceClicked();
		return false;
	}

	if (ok) {
		for (var i in spans) {
			if (spans[i] != currentElement) {
				replaceWord(spans[i]);
			}
		}
		// replace current element the last, so that we jump to the next word ;-)
		replaceClicked();
	}
	return false;
};


/**
* capture the ignore button on a click
*/
function ignoreClicked() {
	document.getElementById("v_replacement").value = currentElement.__msh_origWord;
	replaceClicked();
	return false;
};


/**
* capture the ignore all button on a click
*/
function ignoreAllClicked() {
	document.getElementById("v_replacement").value = currentElement.__msh_origWord;
	replaceAllClicked();
	return false;
};


/**
* initialises all the event handlers, gets the content from the wysiwyg
*/
function initDocument() {
	modified = false;
	frame = document.getElementById("i_framecontent");
	var field = document.getElementById("f_content");

	var html = parent_object.getHTML();

	field.value = html;
	field.form.submit();
	document.getElementById("f_init").value = "0";

	// assign some global event handlers

	var select = document.getElementById("v_suggestions");
	select.onchange = function() {
		document.getElementById("v_replacement").value = this.value;
	};
	if (is_ie) {
		select.attachEvent("ondblclick", replaceClicked);
	} else {
		select.addEventListener("dblclick", replaceClicked, true);
	}

	document.getElementById("b_replace").onclick = replaceClicked;
	document.getElementById("b_replall").onclick = replaceAllClicked;
	document.getElementById("b_ignore").onclick  = ignoreClicked;
	document.getElementById("b_ignall").onclick  = ignoreAllClicked;
	document.getElementById("b_recheck").onclick = recheckClicked;

	select = document.getElementById("v_dictionaries");
	select.onchange = function() {
		document.getElementById("f_dictionary").value = this.value;
	};
};


/**
* capture when a word is clicked
*/
function wordClicked() {
	if (currentElement) {
		var a = allWords[currentElement.__msh_origWord];
		currentElement.className = currentElement.className.replace(/\s*HA-spellcheck-current\s*/g, " ");
		for (var i in a) {
			var el = a[i];
			if (el != currentElement) {
				el.className = el.className.replace(/\s*HA-spellcheck-same\s*/g, " ");
			}
		}
	}
	currentElement = this;
	this.className += " HA-spellcheck-current";
	var a = allWords[currentElement.__msh_origWord];
	for (var i in a) {
		var el = a[i];
		if (el != currentElement) {
			el.className += " HA-spellcheck-same";
		}
	}
	document.getElementById("b_replall").disabled = (a.length <= 1);
	document.getElementById("b_ignall").disabled = (a.length <= 1);
	var txt;
	if (a.length == 1) {
		txt = a.length + ' ' + js_translate('occurrence');
	} else {
		txt = a.length + ' ' + js_translate('occurrences');
	}

	document.getElementById("status").innerHTML = '&nbsp;' + js_translate('found_occurrences_of_word', txt, '<b>' + currentElement.__msh_origWord + '</b>');
	var select = document.getElementById("v_suggestions");
	for (var i = select.length; --i >= 0;) {
		select.remove(i);
	}

	var suggestions = new Array();
	if (this.nextSibling.firstChild) {
		suggestions = this.nextSibling.firstChild.data.split(/,/);
		if (suggestions.length) {
			for (var i = 0; i < suggestions.length; ++i) {
				var txt = suggestions[i];
				var option = document.createElement("option");
				option.value = txt;
				option.appendChild(document.createTextNode(txt));
				select.appendChild(option);
			}
		}
	}

	document.getElementById("v_currentWord").innerHTML = this.__msh_origWord;
	if (suggestions.length > 0) {
		select.selectedIndex = 0;
		select.onchange();
	} else {
		document.getElementById("v_replacement").value = this.innerHTML;
	}
	return false;
};


/**
* capture when a word has the mouse over it
*/
function wordMouseOver() {
	this.className += " HA-spellcheck-hover";
};


/**
* Capture when a word has the mouse out of it
*/
function wordMouseOut() {
	this.className = this.className.replace(/\s*HA-spellcheck-hover\s*/g, " ");
};


/**
* Gets called when the spell parser has finished checking the spelling
*/
function finishedSpellChecking() {
	// initialization of global variables
	currentElement = null;
	wrongWords = null;
	allWords = {};

	document.getElementById("status").innerHTML = '&nbsp;'+ js_translate('spell_checker') + ' ';
	var doc = frame.contentWindow.document;
	var spans = doc.getElementsByTagName("span");
	var sps = [];
	var id = 0;

	for (var i = 0; i < spans.length; ++i) {
		var el = spans[i];
		if (/HA-spellcheck-error/.test(el.className)) {
			sps.push(el);
			el.onclick = wordClicked;
			el.onmouseover = wordMouseOver;
			el.onmouseout = wordMouseOut;
			el.__msh_id = id++;
			var txt = (el.__msh_origWord = el.firstChild.data);
			el.__msh_fixed = false;

			if (typeof allWords[txt] == "undefined") {
				allWords[txt] = [el];
			} else {
				allWords[txt].push(el);
			}
		}
	}

	wrongWords = sps;
	if (sps.length == 0) {
		if (!modified) {
			alert(js_translate('no_spelling_errors'));
		} else {
			alert(js_translate('no_errors'));
		}
		return false;
	}

	(currentElement = sps[0]).onclick();

	var as = doc.getElementsByTagName("a");
	for (var i = as.length; --i >= 0;) {
		var a = as[i];
		a.onclick = function() {
			// disable hrefs because we are spell checking
			// they can click links in the editor later
			return false;
		};
	}

	var dicts = doc.getElementById("HA-spellcheck-dictionaries");
	if (dicts) {
		dicts.parentNode.removeChild(dicts);
		dicts = dicts.innerHTML.split(/,/);
		var select = document.getElementById("v_dictionaries");
		for (var i = select.length; --i >= 0;) {
			select.remove(i);
		}
		for (var i = 0; i < dicts.length; ++i) {
			var txt = dicts[i];
			var option = document.createElement("option");
			option.value = txt;
			option.appendChild(document.createTextNode(txt));
			select.appendChild(option);
		}
	}
};
