<?php
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
* $Id: matrix_expand_keywords.inc,v 1.12.2.2 2006/04/13 05:25:05 tbarrett Exp $
*
*/


/**
* Matrix Keyword Expander
*
* Purpose
*     Using a specified language map asset, this plugin looks for
*	  keywords in the typed text and pops up a confirm dialog
*     to replace it with its full version as you type
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.12.2.2 $
* @package Fudge
* @subpackage wysiwyg
*/
class matrix_expand_keywords  extends wysiwyg_plugin
{

	/**
	* Prints an JS the plugin wants executed when a key is pressed in a WYSIWYG area
	* This code is in a function - do not use script tags
	* You can reference the variables 'editor' and 'key' ( = the character pressed)
	* from within this code
	*
	* @return void
	* @access public
	*/
	function print_plugin_event_handler()
	{
		?>
		if ((ev.type == "keypress")) {
			var key = (HTMLArea.is_ie ? ev.keyCode : ev.charCode);
			window.status = key;
			if ((key == 32) || (key == 13) || (key == 9) || (key == 0)) {
				expandKeywords(editor);
			}
		}
		<?php

	}//end print_plugin_event_handler()

	/**
	* Paints any JS generic functions needed by this plugin
	* These functions can be used by all WYSIWYGs on the page
	* This code is printed outside of script tags - print script tags yourself
	*
	* @return void
	* @access public
	*/
	function paint_generic()
	{
		if (!$GLOBALS['SQ_SYSTEM']->am->installed('thesaurus')) {
			?>
			<script type="text/javascript"> function expandKeywords() {} </script>
			<?php
			return;
		}
		$thesaurusid = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg','SQ_THESAURUS');
		if (!$thesaurusid) {
			?>
			<script type="text/javascript"> function expandKeywords() {} </script>
			<?php
			return;
		}

		$thesaurus = &$GLOBALS['SQ_SYSTEM']->am->getAsset($thesaurusid, '', true);
		if (!$thesaurus->id) {
			?>
			<script type="text/javascript"> function expandKeywords() {} </script>
			<?php
			return;
		}

		$abbrevs = $thesaurus->getAbbreviations();
		if (!is_array($abbrevs) || empty($abbrevs)) {
			?>
			<script type="text/javascript"> function expandKeywords() {} </script>
			<?php
			return;
		}

		?>
		<script type="text/javascript">
		var abbrevs = Array();

		<?php
		foreach ($abbrevs as $abbrev => $word) {
			echo "        abbrevs['$abbrev'] = '$word';\n";
		}
		?>

		var stripTagsRE = /(<\S[^>]*>)|(\W)/g; // yes, for some reason this cannot have quotes around it

		HTMLArea.prototype.replaceLastWord = function(oldWord, newWord) {
			switch (this._editMode) {
				case "wysiwyg":
					if (HTMLArea.is_gecko) {
						var range = this._createRange(this._getSelection());
						range.setStart(range.startContainer, range.startOffset-oldWord.length);
						// remove the current selection
						range.deleteContents();
						toBeInserted = document.createTextNode(newWord);
						var node = range.startContainer;
						var pos = range.startOffset;
						node.insertData(pos, toBeInserted.data);
						range.setEnd(node, pos + newWord.length);
						range.setStart(node, pos + newWord.length);
						sel.addRange(range);
					} else if (HTMLArea.is_ie) {
						var doc = this._docContent;
						while (doc.nodeName != 'BODY') {
							doc = doc.parentNode;
						}
						doc = doc.parentNode.parentNode;
						var rng = doc.selection.createRange();
						rng.moveStart('character', -1 * oldWord.length);
						rng.text = newWord;
					}
				break;
				case "textmode":
					// code me!
				break;
			}
		}

		function expandKeywords(editor)
		{
			var lastWord = getLastWord(editor);
			if (((newWord = abbrevs[lastWord]) != null) && (typeof newWord == 'string')) {
				if (confirm(newWord + '?')) {
					editor.replaceLastWord(lastWord, newWord);
				} else {
					delete abbrevs[lastWord];
				}
			}
		}

		function getLastWord(editor)
		{
			if (HTMLArea.is_gecko) {
				var range = editor._createRange(editor._getSelection());
				var html = range.startContainer.nodeValue.substr(0, range.endOffset);
				html = html.replace(new RegExp(/\s+$/), ''); // strip trailing spaces
				return html.substring(html.lastIndexOf(" ")+1);
			} else {
				var doc = editor._docContent;
				while (doc.nodeName != 'BODY') {
					doc = doc.parentNode;
				}
				doc = doc.parentNode.parentNode;
				var rng = doc.selection.createRange();
				while ((rng.text.charAt(0) != ' ') && (rng.htmlText.charAt(0) != '>') && (rng.text.charCodeAt(0) != 13)) {
					if (rng.moveStart('character', -1) == 0) break;
				}
				while ((rng.text.charAt(0) == ' ') || (rng.htmlText.charAt(0) == '>')) {
					rng.moveStart('character', 1);
				}
				return rng.text.replace(new RegExp(/^\s+/), '');
			}
		}


		</script>
		<?php

	}//end paint_generic()


}//end class

?>