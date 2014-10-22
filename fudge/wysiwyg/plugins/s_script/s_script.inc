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
* $Id: s_script.inc,v 1.9 2012/08/30 00:56:52 ewang Exp $
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
* @version $Revision: 1.9 $
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
		$this->_add_button('subscript','SubScript',translate('Subscript'),'false','18');
		$this->_add_button('superscript','SuperScript',translate('Superscript'),'false','19');
	}

}

?>