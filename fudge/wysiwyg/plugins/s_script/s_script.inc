<?php

global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Super/Subscript
*
* Purpose
*     A WYSIWYG plugin to allow superscript
*     and subscript text
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class s_script extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds the super and subscript buttons for the plugin
	*
	* @returns object S_Script
	* @access  public
	*/
	function s_script() {
		$this->_add_button('subscript','SubScript','Subscript','false','18');
		$this->_add_button('superscript','SuperScript','SuperScript','false','19');
	}

}

?>