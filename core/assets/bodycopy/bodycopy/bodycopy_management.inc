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
* $Id: bodycopy_management.inc,v 1.2.2.2 2004/03/02 18:35:50 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array('added'     => '0.0.1',
											'type'      => 'text',  
											'default'   => 'BodyCopy',
											'update_fn' => '',
											'description' => 'the name of the bodycopy asset',
											)
							);
	}//end constructor


	/**
	* Gets a list of all files that the bodycopy wants web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'images/icons/add.png',
					 'images/icons/delete.png',
					 'images/icons/reorder.png',
					 'images/icons/create_container.png',
					 'images/icons/table_properties.png',
					 'images/icons/row_properties.png',
					 'images/icons/cell_properties.png',
					 'images/icons/edit_mode.png',
					 'js/bodycopy_edit_tables.js',
					 'js/bodycopy_edit_divs.js'
					 );

	}//end _getFileList()

}//end class

?>