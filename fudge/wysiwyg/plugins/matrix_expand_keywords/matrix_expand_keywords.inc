<?php
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
* $Id: matrix_expand_keywords.inc,v 1.19 2007/03/01 00:40:04 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* Matrix Keyword Expander
*
* Purpose
*     Using a specified language map asset, this plugin looks for
*	  keywords in the typed text and pops up a confirm dialog
*     to replace it with its full version as you type
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.19 $
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
			if ((key == 32) || (key == 13) || (key == 9) || (key == 0)) {
				expandKeywords(editor);
			}
		} else if (ev.type == "keydown") {
			var key = (HTMLArea.is_ie ? ev.keyCode : ev.charCode);
			if ((key == 0)) {
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
						if (range.startContainer.data.replace(new RegExp(/\s+$/), '') == '') {
							var contentNode = range.startContainer.parentNode.previousSibling.firstChild;
							range.selectNodeContents(contentNode);
							range.setEnd(range.endContainer, range.endContainer.data.length);
							range.setStart(range.startContainer, range.startContainer.data.length-oldWord.length);
						} else {
							range.setStart(range.startContainer, range.startOffset-oldWord.length-1);
						}
						// remove the current selection
						range.deleteContents();
						toBeInserted = document.createTextNode(newWord+' ');
						var node = range.startContainer;
						var pos = range.startOffset;
						node.insertData(pos, toBeInserted.data);
						range.setEnd(node, pos + newWord.length + 1);
						range.setStart(node, pos + newWord.length + 1);
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
				var contentNode = range.startContainer;
				var html = contentNode.nodeValue.substr(0, range.endOffset);
				html = html.replace(new RegExp(/\s+$/), ''); // strip trailing spaces
				if (html == '') {
					html = contentNode.parentNode.previousSibling.innerHTML;
					html = html.replace(new RegExp(/\s+$/), ''); // strip trailing spaces
				}
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