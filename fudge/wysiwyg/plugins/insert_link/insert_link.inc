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
* $Id: insert_link.inc,v 1.9 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Insert Link
*
* Purpose
*     A WYSIWYG plugin to allow the user to insert and remove
*     a link in the editable document
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package Fudge
* @subpackage wysiwyg
*/
class insert_link extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the insert and remove link buttons to this plugin
	*
	* @returns object Insert_Link
	* @access  public
	*/
	function insert_link()
	{
		$this->_add_button('createlink','CreateLink',translate('Insert Web Link'),'false','32');
		$this->_add_button('unlink','Unlink',translate('Remove Link'),'false','33');
	}


	/**
	* Call the internal _execCommand function when the create button is clicked
	* The unlink functonality is supported nativly by all supported browsers
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "createlink":
			this._execCommand(cmd, true);
		break;
		<?php
	}


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
		case "createlink":
			if (HTMLArea.is_ie || !UI) {
				this._doc.execCommand(cmdID, UI, param);
			} else {
				// browser is Mozilla & wants UI
				if ((param = prompt("Enter URL"))) {
					this._doc.execCommand(cmdID, false, param);
				}
			}
		break;
		<?php
	}
}

?>