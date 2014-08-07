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
* $Id: citation.inc,v 1.3 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Citation
*
* Purpose
*     A WYSIWYG plugin to allow the user to create
*     and manipulate tables
*
* @author  Scott Kim <skim@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class Citation extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds buttons needed for table editing
	*
	* @returns object Table_Editing
	* @access  public
	*/
	function Citation(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		//$this->_add_button('citation','Citation','Insert Citation','false','35');
		$this->_add_button('cite','Cite','Cite','false','35');

	}//end constructor


	/**
	* Highlight the selected text or change the font colour
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command()
	{
		?>
		case "cite":
			if (!this._doc.queryCommandState("cite")) {
				this.surroundHTML("<cite>", "</cite>");
			} else {
				this._doc.execCommand(cmdID, UI, param);
				alert("hello2 cmdID:" + cmdID + " UI:" + UI + " param:" + param);
			}
		break;
		<?php

	}//end print_plugin_exec_command()


	/**
	* Call the appropriate function when a button is clicked
	*
	* @returns void
	* @access  public
	*/
/*	function print_plugin_button_click()
	{
		?>
		case "citation":
			alert('citation');
			this.surroundHTML('<cite>', '</cite>');
			alert('citation');
		break;
		<?php

	}//end print_plugin_button_click()
*/


}//end class

?>