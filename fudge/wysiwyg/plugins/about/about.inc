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
* $Id: about.inc,v 1.10 2003/11/26 00:51:19 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - About
*
* Purpose
*     A WYSIWYG plugin to show an about box for the editor
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class about extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds an about button to the plugin
	*
	* @access public
	*/
	function about()
	{
		$this->_add_button('about','about','About this editor','true','0');

	}//end constructor


	/**
	* When the about button is clicked, show the about.html file in a popup
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "about":
			this._popupDialog("about", "<?php echo $this->get_popup_href('about.html')?>", 380, 185, true, null);
		break;
		<?php

	}//end print_plugin_button_click()

}//end class

?>