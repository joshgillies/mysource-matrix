<?php

global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Help
*
* Purpose
*     A WYSIWYG plugin to provide help to the end user
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class help extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds a help button to the plugin
	*
	* @returns object Help
	* @access  public
	*/
	function help() {
		$this->_add_button('help','showhelp','Help using editor','true','1');
	}


	/**
	* Help is not implemented at this time
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		?>
		case "showhelp":
			alert("Help not implemented");
		break;
		<?php
	}
}

?>