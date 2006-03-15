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
* $Id: bodycopy_management.inc,v 1.10 2006/03/15 23:02:43 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
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

		$this->vars = Array(
						'name'	=> Array(
									'added'			=> '0.0.1',
									'type'			=> 'text',
									'default'		=> 'BodyCopy',
									'update_fn'		=> '',
									'description'	=> 'the name of the bodycopy asset',
								   ),
					  );

	}//end constructor


	/**
	* Gets a list of all files that the bodycopy wants web accessable
	*
	* @return array
	* @access private
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
				'images/icons/htmltidy_disabled.png',
				'images/icons/htmltidy_pass.png',
				'images/icons/htmltidy_fail.png',
				'images/icons/htmltidy_wait.png',
				'js/bodycopy_edit_tables.js',
				'js/bodycopy_edit_divs.js',
				'js/bodycopy_edit_fns.js',
			   );

	}//end _getFileList()


}//end class

?>
