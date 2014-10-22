<?php
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
* $Id: search_and_replace.inc,v 1.3 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* A WYSIWYG plugin to search and replace the text
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3 $
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
		$this->_add_button('searchandreplace','SearchAndReplace',translate('Search And Replace'),'false','69');

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
		HTMLArea.prototype._selectTextContent = function(node, keyword, index, mcase, moption, direction, find_only) {
    		
			textNode = this._getTextNode(node);
			if (textNode === null || textNode.nodeType != 3) return -1; 
    
    		var text = textNode.nodeValue;
    		if (text === null || keyword === null || !text.length || !keyword.length) return -1; 
    		
			// Case sensitive match
    		text = mcase ? text : text.toLowerCase();
    		keyword = mcase ? keyword : keyword.toLowerCase();

			// Check if we are at end of or the start of the text content
			if (direction === undefined) direction = true;
			if ((direction && (index + keyword.length) > text) || (!direction && (index - keyword.length) < 0)) return -1;    
    		
			if (index === undefined || index < 0) index = 0;

			keyword = this._escapeRegexChars(keyword);
    
    		var word_pos;
			var correct_match = false;
			var left_char_empty;
			var right_char_empty;
			
			// Search the keyword
        	do {
				word_pos = direction ?  text.indexOf(keyword, index) : text.lastIndexOf(keyword, index - keyword.length);
				if (word_pos > -1) {
					// So we found the keyword. Now make sure it adheres the partial/whole word condition thing
					left_char_empty = text.charAt(word_pos - 1).match(/[a-z0-9]/i) === null ? true : false;
					right_char_empty = text.charAt(word_pos + keyword.length).match(/[a-z0-9]/i) === null ? true: false;
					
					correct_match = (moption == 'whole' && left_char_empty && right_char_empty) ||
									(moption == 'start' && left_char_empty) ||
									(moption == 'end' 	&& right_char_empty) ||
									(moption == 'none');
				}
				// Move the index
				index = direction ? index + keyword.length : index - keyword.length;
			} while (!correct_match && index < text.length && index >= 0);
				
			
			// If text was not found then we needn't select anything
   			if (!correct_match) return -1;

			// If we just need the word position then we needn't proceed further
			if (find_only !== undefined && find_only) return word_pos;
			
			var range;
    		if (HTMLArea.is_ie) {        		
				range = this._createTextRange();
				// Can't move range to the individual text nodes in IE for some reasons
				// So to workarond this, create and place an empty span element right before the text node
				// and then move the range to this span element
				var span_el = document.createElement("span");
				textNode.parentNode.insertBefore(span_el, textNode);
				range.moveToElementText(span_el);
				
				range.collapse(true);        		
				range.moveStart("character", word_pos);
        		range.moveEnd("character", keyword.length);
       			range.select();
				range.scrollIntoView();

				// Once done selecting, get rid of the span element created above
				textNode.parentNode.removeChild(span_el);
    		} else {
        		var sel = this._getSelection();
        		range = this._doc.createRange();
        		range.setStart(textNode, word_pos);
        		range.setEnd(textNode, word_pos + keyword.length);
        		sel.removeAllRanges();
        		sel.addRange(range);
				textNode.parentNode.scrollIntoView();
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
		HTMLArea.prototype._getTextNodes = function(refresh) {
			
			refresh = refresh === undefined || !refresh ? false : true;
			// If the we have parsed the DOM tree before for the text nodes
			if (!refresh  && global_text_nodes.length > 0) {
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
	* Open the Search and Replace Text plugin dialog
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


				this._popupDialog("SearchAndReplace", "<?php echo $this->get_popup_href('search_and_replace.php')?>", 430, 360, false, function(types){
					
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
								editor._refreshStoredTextNodes();
							}
							break;

						case "replace_all":
							
						case "replace_selection":
							if (selected_content.length > 0 || action_type === "replace_all") {
								
								var match_count = 0;

								if (match_option != 'regex') {
									var text_nodes = editor._getTextNodes(true);
									var node_count = 0
									var index = 0;
									var node_value;

									// If selection is made then only include the textnodes inside the selection
									if (action_type === "replace_selection") {
										
										if (selected_content.length > 0) {
											var sel = editor._getSelection();
											var selected_nodes = Array();
											var start_selection = false;
											alert(sel.anchorNode.nodeValue);	
											for(var count=0; count < text_nodes.length; count++) {
												if (!start_selection && sel.anchorNode == text_nodes[count])
													start_selection = true;
	
												selected_nodes[count] = start_selection ? text_nodes[count] : null;
												if (sel.focusNode == text_nodes[count]) break;
											}
											text_nodes = selected_nodes;
											// Start and end offset of anchor and focus node respectively
											index = sel.anchorOffset;
											var endOffset = sel.focusOffset;

										} else {
											text_nodes = Array();
										}
										
									}//end if replace_selection 

									while(node_count < text_nodes.length) {
										
										if (text_nodes[node_count] === null) {
											node_count++;
											continue;
										}
										index = editor._selectTextContent(node_count, search_str, index, match_case, match_option, true, true);
										
										// If text was selected, make sure we dont get across the unselected portion of the focus node
										if (endOffset !== undefined && index+search_str.length > endOffset && node_count == text_nodes.length - 1) break;
										
										if (index > -1) {
											node_value = text_nodes[node_count].nodeValue;
											node_value = 	(index > 0 ? node_value.substring(0, index) : '')
															+ replace_str
															+ node_value.substring(index+search_str.length, node_value.length);
											// Update the node value with the replacement
											text_nodes[node_count].nodeValue = node_value
											index += replace_str.length;
											match_count++;
										} else {
											// No more match in this node, move to the next
											node_count++;
											index = 0;
										}
									}
								
									alert(match_count+" occurrences of the given text have been replaced");

								} else {
									var wysiwyg_content = (action_type === "replace_all") ? whole_content : selected_content;	
									search_regex = new RegExp(search_str, match_case ? 'g' : 'gi');
                                	match_count = editor._countWord(wysiwyg_content, search_regex);
                                	wysiwyg_content = wysiwyg_content.replace(search_regex, replace_str);

                                	if (action_type === "replace_all") {
                                    	editor.setHTML(wysiwyg_content);
                                	} else {
                                    	editor.insertHTML(wygiwyg_content);
                                	}

									alert(match_count+" number of matching occurrences for given regular expression have been replaced");
								}
								
								if (match_count > 0) {
									editor._refreshStoredTextNodes();
								}

								return false;
							}
							break;

					}//end switch

					
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
