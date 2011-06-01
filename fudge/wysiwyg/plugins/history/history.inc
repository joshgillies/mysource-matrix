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
* $Id: history.inc,v 1.8 2006/12/06 05:11:08 bcaldwell Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - History Functions
*
* Purpose
*     A WYSIWYG plugin to provide undo and redo functions.
*     The commands are supported nativly by the editors for
*     all supported browsers - so only need to add the buttons
*     and not provide the functionality
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package Fudge
* @subpackage wysiwyg
*/
class history extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds undo and redo buttons
	*
	* @returns object History
	* @access  public
	*/
	function history()
	{
		$this->_add_button('undo','Undo','Undo','false','12');
		$this->_add_button('redo','Redo','Redo','false','13');
	}

}

?>