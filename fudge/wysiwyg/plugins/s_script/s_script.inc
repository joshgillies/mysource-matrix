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
* $Id: s_script.inc,v 1.8 2006/12/06 05:11:10 bcaldwell Exp $
*
*/


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
* @version $Revision: 1.8 $
* @package Fudge
* @subpackage wysiwyg
*/
class s_script extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the super and subscript buttons for the plugin
	*
	* @returns object S_Script
	* @access  public
	*/
	function s_script()
	{
		$this->_add_button('subscript','SubScript','Subscript','false','18');
		$this->_add_button('superscript','SuperScript','SuperScript','false','19');
	}

}

?>