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
* $Id: help.inc,v 1.11 2006/12/06 05:11:08 bcaldwell Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Help
*
* Purpose
*     A WYSIWYG plugin to provide help to the end user
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package Fudge
* @subpackage wysiwyg
*/
class Help extends WYSIWYG_Plugin
{


	/**
	* Constructor
	*
	* Adds a help button to the toolbar
	*
	*/
	function Help()
	{
		$this->_add_button('help','showhelp','Help using editor','true','1');

	}//end constructor


	/**
	* Help is not implemented at this time
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{
		?>
		case "showhelp":
			this._popupDialog("help", "<?php echo $this->get_popup_href('help.html')?>", 790, 590, true, function() {}, 0);
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>