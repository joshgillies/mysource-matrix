<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: listing.inc,v 1.7 2004/12/06 14:39:54 brobertson Exp $
*
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
* @version $Revision: 1.7 $
* @package Fudge
* @subpackage wysiwyg
*/
class listing extends wysiwyg_plugin
{

	/**
	* Constructor
	* Add buttons to print different types of lists
	*
	* @returns object Listing
	* @access  public
	*/
	function listing()
	{
		$this->_add_button('orderedlist','InsertOrderedList','Ordered List','false','26');
		$this->_add_button('unorderedlist','InsertUnorderedList','Bulleted List','false','27');
	}

}

?>