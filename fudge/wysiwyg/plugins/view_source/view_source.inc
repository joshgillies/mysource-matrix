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
* $Id: view_source.inc,v 1.10 2006/12/06 05:11:11 bcaldwell Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - View Source
*
* Purpose
*     A WYSIWYG plugin to allow the user to
*     switch between HTML source and editing views
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package Fudge
* @subpackage wysiwyg
*/
class View_Source extends Wysiwyg_Plugin
{

	/**
	* Constructor
	* Adds the view source button (that switches the mode)
	*
	* @returns object View_Source
	* @access  public
	*/
	function View_Source()
	{
		$this->_add_button('htmlmode','HtmlMode','Toggle HTML Source','true','3');

	}//end constructor


	/**
	* On click, switch the mode of the editor based on current mode
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "htmlmode":
			this.setMode(this._editMode != "textmode" ? "textmode" : "wysiwyg");
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Show the view source button as active
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "htmlmode":
			btn.state("active", text);
		break;
		<?php

	}//end print_plugin_update_toolbar()

}//end class

?>