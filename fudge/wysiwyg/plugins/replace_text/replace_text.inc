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
* $Id: replace_text.inc,v 1.4 2006/12/06 05:11:10 bcaldwell Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Format Text
*
* Purpose
*		A WYSIWYG plugin to separate text replace
*		from other formatting functions
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.4 $
* @package Fudge
* @subpackage wysiwyg
*/
class Replace_Text extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object Format_Text
	* @access  public
	*/
	function replace_text()
	{
		$this->_add_button('replacetext','ReplaceText','Replace Text','false','11');
	}


}//end class

?>
