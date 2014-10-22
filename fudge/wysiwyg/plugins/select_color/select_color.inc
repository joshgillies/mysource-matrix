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
* $Id: select_color.inc,v 1.16 2012/11/20 05:22:38 cupreti Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Colour Functions
*
* Purpose
*     A WYSIWYG plugin to allow the user to
*     set text and background colours
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.16 $
* @package Fudge
* @subpackage wysiwyg
*/
class Select_Color extends WYSIWYG_Plugin
{

	/**
	* Constructor
	* Adds the fore and background colour buttons to the plugin
	*
	* @returns object Select_Color
	* @access  public
	*/
	function select_color()
	{
		$this->_add_button('forecolor','foreColor',translate('Text Colour'),'false','20');
		$this->_add_button('backcolor','BackColor',translate('Background Colour'),'false','21');

	}//end constructor


	/**
	* Open the popup colour selector to get the colour from the user
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "forecolor":
		case "backcolor":
			this._popupDialog("selectColor", "<?php echo $this->get_popup_href('select_color.html')?>", 248, 190, true, function(color) {
				if (color) {
					// Let the browser do the job
					editor._doc.execCommand(cmd, false, color);
				}

			}, HTMLArea._colorToRgb(this._doc.queryCommandValue(btn[0])));
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>
