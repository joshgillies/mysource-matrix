<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_management.inc,v 1.8 2003/09/26 05:26:40 brobertson Exp $
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
* @subpackage cms
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
											'type'      => 'text',  'default'   => 'BodyCopy',
											'update_fn' => ''
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
					 'images/icons/create_table.png',
					 'images/icons/table_properties.png',
					 'images/icons/row_properties.png',
					 'images/icons/cell_properties.png',
					 'images/icons/edit_mode.png',
					 'js/bodycopy_edit_tables.js'
					 );

	}//end _getFileList()

}//end class

?>