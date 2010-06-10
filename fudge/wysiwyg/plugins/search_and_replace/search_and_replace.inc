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
* $Id: search_and_replace.inc,v 1.1 2010/06/10 06:37:18 cupreti Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* A WYSIWYG plugin to search and replace the text
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package Fudge
* @subpackage wysiwyg
*/
class Search_And_Replace extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object
	* @access public
	*/
	function Search_And_Replace()
	{
		$this->_add_button('searchandreplace','SearchAndReplace','Search And Replace','true','69');

	}//end Search_And_Replace()


    /** 
    * Prints generic functions
    *
    * @return void
    * @access public
    */
    function paint_generic()
    {   
        ?>  
        <script type="text/javascript" language="Javascript">

		// Global variable to store the text nodes in the document
		var global_text_nodes = Array();
		
		// Selects the given text in the supplied textnode
		HTMLArea.prototype._selectTextContent = function(node, keyword, index, mcase, moption, direction) {
    		
			textNode = this._getTextNode(node);
			if (textNode === null || textNode.nodeType != 3) return -1; 
    
    		var text = textNode.nodeValue;
    		if (text === null || keyword === null || !text.length || !keyword.length) return -1; 
    		
			// Case sensitive match
    		text = mcase ? text : text.toLowerCase();
    		keyword = mcase ? keyword : keyword.toLowerCase();

			// Check if we are end of or the start of the text content
			if (direction === undefined) direction = true;
			if ((direction && (index + keyword.length) > text) || (!direction && (index - keyword.length) < 0)) return -1;

    
			var non_word_chars = '[^A-Za-z0-9]';
    		var keyword_reg;
			
			keyword_escaped_reg = this._escapeRegexChars(keyword);
			// Word match options
    		if (moption == 'whole') {
        		keyword_reg = non_word_chars + keyword_escaped_reg + non_word_chars;
    		} else if (moption == 'start') {
        		keyword_reg = non_word_chars + keyword_escaped_reg;
   			} else if (moption == 'end') {
        		keyword_reg = keyword_escaped_reg + non_word_chars;
    		} else {
        		keyword_reg = keyword_escaped_reg;
    		}
    
    		if (index === undefined || index < 0) index = 0;
    		// Search the word

    		var word_pos;
			var whole_keyword;

			// To pad space around the text if its at the start or the end of a text node
			var empty_left;
			var empty_right;

			if (direction) {
        		do {
					word_pos = text.indexOf(keyword, index);
					empty_left = !text.charAt(word_pos - 1) ? 1 : 0;
					empty_right = !text.charAt(word_pos + keyword.length) ?  1 : 0;
					whole_keyword = (empty_left ? ' ' : '') + text.substr(word_pos - 1 + empty_left, keyword.length + 2 - empty_left - empty_right) + (empty_right ? ' ' : '');
					index += keyword.length;
				} while (index < text.length && word_pos > -1 && !whole_keyword.match(keyword_reg));
				
    		} else {
        		do {
        			word_pos = text.lastIndexOf(keyword, index - keyword.length);
					empty_left = !text.charAt(word_pos - 1) ? 1 : 0;
					empty_right = !text.charAt(word_pos + keyword.length) ?  1 : 0;
					whole_keyword = (empty_left ? ' ' : '') + text.substr(word_pos - 1 + empty_left, keyword.length + 2 - empty_left - empty_right) + (empty_right ? ' ' : '');
					index -= keyword.length;
				} while (index > 0 && word_pos > -1 && !whole_keyword.match(keyword_reg));
				
    		}
			
			// If text was not found then we needn't select anything
   			if (word_pos < 0) return -1;
			
			var range;
    		if (HTMLArea.is_ie) {        		
				range = this._createTextRange();
				// Can't move range to the individual text nodes in IE for some weirdest reasons
				// Hence the "previous text nodes counting" workaround
				range.moveToElementText(this._getTextNode(0).parentNode);
				range.collapse(true);        		
				var real_pos = word_pos + this._previousTextNodesLength(node);
				range.moveStart("character", real_pos);
        		range.moveEnd("character", keyword.length);
       			range.select();
    		} else {
        		var sel = this._getSelection();
        		range = this._doc.createRange();
        		range.setStart(textNode, word_pos);
        		range.setEnd(textNode, word_pos + keyword.length);
        		sel.removeAllRanges();
        		sel.addRange(range);
    		}

    		
			return direction ? word_pos + keyword.length : word_pos;
		};

		
		// Get the number of occurance of given regular expression
		HTMLArea.prototype._countWord = function(text, regex){
			var count = 0;
			
			text.replace(
				regex,
				function(){
 					count++;
 				}
			);
			
			return count;
		}


		// DOM tree walker to get all the text nodes
		HTMLArea.prototype._getTextNodes = function() {
			
			// If the we have parsed the DOM tree before for the text nodes
			if (global_text_nodes.length > 0) {
				return global_text_nodes;
			}
		
			var root = !HTMLArea.is_ie ? this._iframe.contentWindow.document.body : this._docContent;

			var text_nodes = Array();
    		var node = root.childNodes[0];
			var count = 0;
    		while(node != null) {
        		if (node.nodeType == 3) {
            		text_nodes[count++] = node;
        		}

        		if (node.hasChildNodes()) {
            		node = node.firstChild;
        		}
        		else {
            		while(node.nextSibling == null && node != root) {
                		node = node.parentNode;
            		}
            		node = node.nextSibling;
        		}
    		}
			
			// Store the value in the global variable for reuse
			global_text_nodes = text_nodes;
			return text_nodes;
		}


		// Get the text node from the WYSIWYG body
		HTMLArea.prototype._getTextNode = function(index){
			
			if (index < 0) return null;

    		var node = null;
			var count = 0;
			
    		var child_nodes = this._getTextNodes();
			while(node = child_nodes[count]) {
					
				if (count == index) break;
				count++;
    		}
			return node;
		}


		// Get the total length of the value of all textnodes preceeding given one
		HTMLArea.prototype._previousTextNodesLength = function(index){
			
			if (index <= 0) return 0;

    		var node = null;
			var count = 0;
			var length = 0;
    		var child_nodes = this._getTextNodes();
			while(node = child_nodes[count]) {
				if (count >= index) break;
				length += node.nodeValue.length;

				count++;
    		}
			return length;
		}


		// Returns the index of the last text node from the WYSIWYG body
		HTMLArea.prototype._getLastTextNodeIndex = function(){
			var count = 0;
			var text_nodes = this._getTextNodes();

			return text_nodes.length - 1;
		}

			
		// Empty the global variable storing all the text nodes in the document
		HTMLArea.prototype._refreshStoredTextNodes = function(){
			global_text_nodes = Array();
		}

		
		// Escapes all the regular expression characters in the given text
		HTMLArea.prototype._escapeRegexChars = function(text){
			
			var regex_chars = Array('/', '{', '}', '[', ']', '(', ')', '-', '+', '*', '^', '$', '?', '.');				
			arguments.callee.regex = new RegExp('(\\'+regex_chars.join('|\\') + ')', 'g');
			
			return text.replace(arguments.callee.regex, '\\$1');
		}

        </script>
        <?php

    }//end paint_generic()


	/**
	* Open the Replace Text plugin dialog
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{
		?>
		case "searchandreplace":
				
				// Global var to store the start search position in WYSIWYG 
				var start_pos = 0;
				// Global var to keep track of textnodes in the body
				var current_node = 0;
				// Global var to check if the search text exists after complete document search
				var search_text_not_found = false;


				this._popupDialog("SearchAndReplace", "<?php echo $this->get_popup_href('search_and_replace.php')?>", 380, 300, false, function(types){
					
					if (types == null) return false;
					
					var action_type = types[0];
					var search_str 	= types[1];
					var replace_str = types[2];

					var match_case = false;
					if (types[3]) {
						match_case = true;
					}
					
					var match_option = 'none';
					if (types[4]) {
						match_option = 'whole';
					} else if (types[5]) {
						match_option = 'start';
					} else if (types[6]) { 
						match_option = 'end';
					} else if (types[7]) {
						match_option = 'regex';
					}
					
					selected_content = editor.getSelectedHTML();
					whole_content = editor.getHTML();

					//Replace direction, "forward" by default
					var direction = true;

					switch (action_type) {
						case "search_next":
							break;

						case "search_previous":
							direction = false;
							break;

						case "replace_current":
							if (start_pos != -1 && selected_content.length > 0) {
								editor.insertHTML(replace_str);
								editor.deleteEmpty();
								
								editor._refreshStoredTextNodes();
							}
							break;

						case "replace_all":
							
						case "replace_selection":
							if (selected_content.length > 0 || action_type === "replace_all") {

								var new_content = (action_type === "replace_all") ? whole_content : selected_content;
								var regex_modifier = "g"+ (match_case ? "" : "i");
								
								var regex_search_str = search_str;
								var regex_replace_str = replace_str;

								if (match_option != "regex") {
									new_content = " "+new_content+" ";
									search_str = editor._escapeRegexChars(search_str);
								}
								
								// Characters that speperates the word (might need add more later)								
								var non_word_chars = '[^A-Za-z0-9]';

								if (match_option == "whole") {
									regex_search_str = "("+non_word_chars+")" + search_str + "("+non_word_chars+")";
									regex_replace_str = "$1"+replace_str+"$2";
								} else if (match_option == "start") {							
									regex_search_str = "("+non_word_chars+")" + search_str;
									regex_replace_str = "$1"+replace_str;
								} else if (match_option == "end") {								
									regex_search_str = search_str + "("+non_word_chars+")";
									regex_replace_str = replace_str+"$1";
								} else if (match_option == "none") {									
									regex_search_str = search_str;
									regex_replace_str = replace_str;
								}

								replace_regex = new RegExp(regex_search_str, regex_modifier);
								replacement_count = editor._countWord(new_content, replace_regex);
								new_content = new_content.replace(replace_regex, regex_replace_str);

								if (match_option != "regex") {
									new_content = new_content.replace(/^\s(.*)\s$/, "$1");
								}
								
								if (action_type === "replace_all") {
									editor.setHTML(new_content);
								} else {
									editor.insertHTML(new_content);
								}
								editor.deleteEmpty();
								editor._refreshStoredTextNodes();
								
								if (match_option == "regex") {
									alert(replacement_count+" number of matching occurrences for given regular expression have been replaced");
								} else {								
									alert(replacement_count+" occurrences of the given text have been replaced");
								}

								return false;
							}
							break;

					}//end switch

					
					var match_option = 'none';
					if (types[4]) {
						match_option = 'whole';
					} else if (types[5]) {
						match_option = 'start';
					} else if (types[6]) {
						match_option = 'end';
					}
					
					// Select and highlight the selected text in the document body					
					var node;
					var last_node_index = editor._getLastTextNodeIndex();
					while(current_node >= -1  && current_node <= (last_node_index + 1)) {
						node = editor._getTextNode(current_node);
						
						// Node is null, we have reach at either end of node list,
						if (!node) {
						
							if (search_text_not_found) {
								alert("The given text was not found");
								search_text_not_found = false;
								break;
							}
							
							if (direction) {
								alert("Reached the end of the WYSIWYG content. Starting from the beginning");
								start_pos = 0;
								current_node = 0;
							} else {
								alert("Reached the start of the WYSIWYG content. Starting from the end");
								last_node = editor._getTextNode(last_node_index);
								start_pos = last_node.nodeValue.length + search_str.length;
								current_node = last_node_index;
							}
							
							// if this flag is still true by the next time we reach the either end of document
							// then it will mean that search text is not there in the document body
							search_text_not_found = true;
						} 
	
						node = !node ? editor._getTextNode(current_node) : node;
						if (node) {
							// If the backward search in previous node (actaully in node at right side) failed,
							// then we need to start the search from the end of the next node
							if (!direction && start_pos < 0) {
								start_pos = node.nodeValue.length + search_str.length;
							}
							
							// Search and highlight the text in the current text node
							start_pos = editor._selectTextContent(current_node, search_str, start_pos, match_case, match_option, direction);

						}
					
						// Continue searchin to next textnode
						if (start_pos < 0) {
							direction ? current_node++ : current_node--;
							continue;
						} else {
							// Search text was found hence, ...
							search_text_not_found = false;
						}
					
						break;
					}//end while
					
				}, null);
				
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>
