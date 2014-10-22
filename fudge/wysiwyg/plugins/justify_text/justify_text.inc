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
* $Id: justify_text.inc,v 1.11 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Justify Text
*
* Purpose
*     A WYSIWYG plugin to allow text alignment
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package Fudge
* @subpackage wysiwyg
*/
class Justify_Text extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Add buttons to justify text to this plugin
	*
	* @returns object Justify_Text
	* @access  public
	*/
	function justify_text()
	{
		$this->_add_button('justifyleft','JustifyLeft',translate('Justify Left'),'false','22');
		$this->_add_button('justifycenter','JustifyCenter',translate('Justify Center'),'false','23');
		$this->_add_button('justifyright','JustifyRight',translate('Justify Right'),'false','24');
		$this->_add_button('justifyfull','JustifyFull',translate('Justify Full'),'false','25');

	}//end consturctor


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

		HTMLArea.prototype._addAlignAttribute = function(node) {
			var current = node.getAttribute("align");
			if (current != "") {
				node.style.textAlign = current;
				node.removeAttribute("align");
			}
		}
		HTMLArea.prototype._operateTags = function(tagname) {
			nodes = document.getElementsByTagName(tagname);
			for (var i=0; i<nodes.length; i++) {
				this._addAlignAttribute(nodes[i]);
			}
		}
		</script>
		<?php

	}//end paint_generic()


	/**
	* Print keyboard shortcuts to align text
	*
	* @return void
	* @access public
	*/
	function print_plugin_shortcuts()
	{
		?>
		case 'l':     // KEY justify left
			cmd = "justifyleft";
		break;
			case 'e': // KEY justify center
			cmd = "justifycenter";
		break;
		case 'r':     // KEY justify right
			cmd = "justifyright";
		break;
		case 'j':     // KEY justify full
			cmd = "justifyfull";
		break;
		<?php

	}//end print_plugin_shortcuts()


	/**
	* Create the link a URL entered by the user
	* IE has its own linking interface, while we use a JS prompt for Mozilla
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "justifyleft":
		case "justifyright":
		case "justifycenter":
		case "justifyfull":
			this._doc.execCommand(cmdID, UI, param);
			if (HTMLArea.is_ie) {
				this._operateTags("p");
				this._operateTags("div");
				this._operateTags("h1");
				this._operateTags("h2");
				this._operateTags("h3");
				this._operateTags("h4");
				this._operateTags("h5");
				this._operateTags("h6");
			}
		break;
		<?php
	}


}//end class

?>
