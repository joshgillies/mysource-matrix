<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: listing.inc,v 1.4 2003/09/26 05:26:37 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Listing Functions
*
* Purpose
*     A WYSIWYG plugin to create numbered
*     or bulleted lists
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class listing extends wysiwyg_plugin {

	/**
	* Constructor
	* Add buttons to print different types of lists
	*
	* @returns object Listing
	* @access  public
	*/
	function listing() {
		$this->_add_button('orderedlist','InsertOrderedList','Ordered List','false','26');
		$this->_add_button('unorderedlist','InsertUnorderedList','Bulleted List','false','27');
	}

}

?>