<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: justify_text.inc,v 1.5 2003/11/07 01:37:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class Justify_Text extends WYSIWYG_Plugin {


	/**
	* Constructor
	* Add buttons to justify text to this plugin
	*
	* @returns object Justify_Text
	* @access  public
	*/
	function justify_text()
	{
		$this->_add_button('justifyleft','JustifyLeft','Justify Left','false','22');
		$this->_add_button('justifycenter','JustifyCenter','Justify Center','false','23');
		$this->_add_button('justifyright','JustifyRight','Justify Right','false','24');
		$this->_add_button('justifyfull','JustifyFull','Justify Full','false','25');

	}//end consturctor

	
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


}//end class

?>