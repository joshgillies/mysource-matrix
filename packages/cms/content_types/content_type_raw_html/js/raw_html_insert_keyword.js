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
* $Id: raw_html_insert_keyword.js,v 1.1 2005/02/11 04:58:25 tbarrett Exp $
*
*/

/**
* Insert the selected keyword from the select box into the html textarea with the given prefix
*
* @param string	prefix	The prefix of the select box and textarea to work with
*
* @return void
*/
function insertKeyword(prefix) 
{
	var keywordSelector = document.getElementById(prefix+'_keyword_inserter');
	var replacement = keywordSelector.value;
	if (replacement.length == 0) return;
	var myField = document.getElementById(prefix+'_html');
	insertText(replacement, myField);
	keywordSelector.selectedIndex = 0;

}//end insertKeyword()


/**
* Insert text into the supplied textarea at the cursor position
*
* @param string	text	The text to insert
* @param object	myField	The textarea to insert into
*
* @return void
*/
function insertText(text, myField)
{
	if (document.selection) {
		// IE
		myField.focus();
		var rng = document.selection.createRange();
		rng.colapse;
		rng.text = text;
	} else if (myField.selectionStart || myField.selectionStart == '0') {
		// Moz
		var startPos = myField.selectionStart;
		var endPos = myField.selectionEnd;
		myField.value = myField.value.substring(0, startPos)
						+ text
						+ myField.value.substring(endPos, myField.value.length);
		myField.focus();
		myField.selectionStart = startPos + text.length;
		myField.selectionEnd = startPos + text.length;
	} else {
		// Others
		myField.value += text;
	}

}//end insertText()
