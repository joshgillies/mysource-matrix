<?php

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
	* @returns object About
	* @access  public
	*/
	function about() {
		$this->_add_button('about','about','About this editor','true','0');
	}


	/**
	* When the about button is clicked, show the about.html file in a popup
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		?>
		case "about":
			this._popupDialog("<?php echo $this->get_popup_href('about.html')?>", null, null);
		break;
		<?php
	}

}

?>