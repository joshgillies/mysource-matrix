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
* $Id: listing.inc,v 1.8 2006/12/06 05:11:09 bcaldwell Exp $
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
* @version $Revision: 1.8 $
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