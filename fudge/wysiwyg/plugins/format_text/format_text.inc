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
* $Id: format_text.inc,v 1.14 2012/11/20 05:20:26 cupreti Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Format Text
*
* Purpose
*     A WYSIWYG plugin to make select bold, itlaic, underline
*     and strikethrough. Also has remove all formatting function
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package Fudge
* @subpackage wysiwyg
*/
class Format_Text extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object Format_Text
	* @access  public
	*/
	function format_text()
	{
		$this->_add_button('bold','Bold',translate('Bold'),'false','14');
		$this->_add_button('italic','Italic',translate('Italic'),'false','15');
		$this->_add_button('underline','Underline',translate('Underline'),'false','16');
		$this->_add_button('strikethrough','StrikeThrough',translate('Strikethrough'),'false','17');
	}


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

		// Replaces the 'fromTag' tag to 'toTag' in the given selection
		HTMLArea.prototype._convertTags = function(fromTag, toTag)	{

			if (HTMLArea.is_ie) {
				// We dont need to convert tags for IE, as IE will have the required tags already
				return;
			}

			if (fromTag == null || toTag == null) {
				return;
			}

			fromTag = fromTag.toLowerCase();
			toTag = toTag.toLowerCase();

			// Get the required tags within the selection
			var sel = this._getSelection();
			var parent = this.parentBlock(this.getParentElement());
			var elements = parent.getElementsByTagName(fromTag);

			// Restrict the element list to within the selection
			var elementsArray = [];
			for(var i = 0; i<elements.length; i++) {
				if (sel.containsNode(elements[i], true)) {
					elementsArray.push(elements[i]);
				}
			}

			// Get the selection anchor and focus nodes
			var anchorNode = sel.anchorNode;
			var anchorOffset = sel.anchorOffset;
			var focusNode = sel.focusNode;
			var focusOffset = sel.focusOffset;

			// Carry out the replacement
			for(var j = 0; j< elementsArray.length; j++) {
				var replacementEl = document.createElement(toTag);
				replacementEl.innerHTML = elementsArray[j].innerHTML;
				elementsArray[j].parentNode.replaceChild(replacementEl, elementsArray[j]);

				// Update the anchor and focus nodes
				var oldAnchorTextNode = elementsArray[j].firstChild;
				var newAnchorTextNode = replacementEl.firstChild;
				while(oldAnchorTextNode.nodeName != '#text' && oldAnchorTextNode.hasChildNodes()) {
					newAnchorTextNode = newAnchorTextNode.firstChild;
					oldAnchorTextNode = oldAnchorTextNode.firstChild;
				}
				var oldFocusTextNode = elementsArray[j].lastChild;
				var newFocusTextNode = replacementEl.lastChild;
				while(oldFocusTextNode.nodeName != '#text' && oldFocusTextNode.hasChildNodes()) {
					newFocusTextNode = newFocusTextNode.lastChild;
					oldFocusTextNode = oldFocusTextNode.lastChild;
				}
				if (oldAnchorTextNode === anchorNode) {
					anchorNode = newAnchorTextNode;
				}
				if (oldFocusTextNode === focusNode) {
					focusNode = newFocusTextNode;
				}
			}

			// Update the selection range
			if (anchorNode != null && focusNode != null) {
				var range = this._createRange();
				range.setStart(anchorNode, anchorOffset);
				range.setEnd(focusNode, focusOffset);
				sel.removeAllRanges();
				sel.addRange(range);
			}
		}
		</script>

		<?php

	}//end paint_generic()


	/**
	* Prints shortcuts for the buttons
	*
	* All of these except strikethrough are supported nativly in IE
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts()
	{
		?>
		case 'b':	// KEY bold
			(!HTMLArea.is_ie) && (cmd = "bold");
		break;
		case 'i':	// KEY italic
			(!HTMLArea.is_ie) && (cmd = "italic");
		break;
		case 'u':	// KEY underline
			(!HTMLArea.is_ie) && (cmd = "underline");
		break;
		case 's':	// KEY strikethrough
			cmd = "strikethrough";
		break;
		<?php

	}//end print_plugin_shortcuts()


	/**
	* Highlight the selected text or change the font colour
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "bold":
			this._doc.execCommand(cmdID, UI, param);
			// Convert the added 'b' tags to 'strong'
			if (this._doc.queryCommandState("bold")) {
				this._convertTags('b', 'strong');
			}
		break;
		case "italic":
			this._doc.execCommand(cmdID, UI, param);
			// Convert the added 'i' tags to 'em'
			if (this._doc.queryCommandState("italic")) {
				this._convertTags('i', 'em');
			}
		break;
		case "underline":
			this._doc.execCommand(cmdID, UI, param);
		break;
		case "strikethrough":
			this._doc.execCommand(cmdID, UI, param);
		break;
		<?php

	}//end print_plugin_exec_command()


}//end class

?>
