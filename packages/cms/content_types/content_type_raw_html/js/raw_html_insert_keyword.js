/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: raw_html_insert_keyword.js,v 1.4 2006/12/07 00:04:16 emcdonald Exp $
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
	var rememberScroll = myField.scrollTop;
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
	myField.scrollTop = rememberScroll;

}//end insertText()
