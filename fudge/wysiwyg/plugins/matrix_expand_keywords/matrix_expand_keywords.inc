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
* $Id: matrix_expand_keywords.inc,v 1.2 2004/10/27 00:42:06 tbarrett Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.2 $
* @package Fudge
* @subpackage wysiwyg
*/
class matrix_expand_keywords  extends wysiwyg_plugin {

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
			if ((key == 32) || (key == 13) || (key == 9)) {
				expandKeywords(editor);
				/*
				editor.insertHTML('__WORD__ ');
				var html = editor.getHTML();
				print_alert(html.indexOf('__WORD__'));
				var upToNow = html.substring(0, html.indexOf('__WORD__')).replace(stripTagsRE, " ");;
				print_alert(upToNow.substring(upToNow.lastIndexOf(' ')));
				editor.setHTML(html.replace('__WORD__', ''));
				*/
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
		?>
		<script type="text/javascript">
		var stripTagsRE = /<\S[^>]*>/g; // yes, for some reason this cannot have quotes around it


		HTMLArea.prototype.replaceLastWord = function(oldWord, newWord) {
			switch (this._editMode) {
				case "wysiwyg":
					if (HTMLArea.is_gecko) {
						oldContents = this._iframe.contentWindow.document.body.innerHTML.toString();
						oldWordStart = oldContents.lastIndexOf(oldWord);
						this._iframe.contentWindow.document.body.innerHTML = oldContents.substring(0, oldWordStart) + newWord;
						this._iframe.contentWindow.document.body.selectionStart = 5;
					} else if (HTMLArea.is_ie) {
						oldContents = this._docContent.innerHTML;
						oldWordStart = oldContents.lastIndexOf(oldWord);
						this._docContent.innerHTML = oldContents.substring(0, oldWordStart) + newWord;
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
			window.status = ('last word was ' + lastWord);
			switch (lastWord) {
				case 'tom':
					if (confirm('Tom Barrett?')) editor.replaceLastWord(lastWord, 'Tom Barrett');
					break;
				case 'gsw':
					if (confirm('Gun Shot Wound?')) editor.replaceLastWord(lastWord, 'Gun Shot Wound');
					break;
				case 'ent':
					if (confirm('Ears Nose & Throat?')) editor.replaceLastWord(lastWord, 'Ears, Nose & Throat');
					break;
			}
		}

		function getLastWord(editor) 
		{
			var html = editor.getHTML().replace(stripTagsRE, " ");
			// strip trailing spaces
			while (html.charAt(html.length-1) == " ") {
				html = html.substring(0, html.length-1);
			}
			return html.substring(html.lastIndexOf(" ")+1);
		}

		
		</script>
		<?php 

	}//end paint_generic()


}//end class

?>