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
* $Id: about.inc,v 1.13 2012/08/30 00:56:51 ewang Exp $
*
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
* @version $Revision: 1.13 $
* @package Fudge
* @subpackage wysiwyg
*/
class about extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds an about button to the plugin
	*
	* @access public
	*/
	function about()
	{
		$this->_add_button('about','about',translate('About this editor'),'true','0');

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
			this._popupDialog("about", "<?php echo $this->get_popup_href('about.php')?>", 380, 185, true, null);
		break;
		<?php

	}//end print_plugin_button_click()

}//end class

?>