<?php

global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Indenting
*
* Purpose
*     A WYSIWYG plugin to increase and decrease the
*     indent of the select elements
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge::wysiwyg
*/
class indenting extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds indent and outdent buttons to the plugin
	*
	* @returns object Indenting
	* @access  public
	*/
	function indenting() {
		$this->_add_button('outdent','Outdent','Decrease Indent','false','28');
		$this->_add_button('indent','Indent','Increase Indent','false','29');
	}

}

?>