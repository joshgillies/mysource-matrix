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
* $Id: bodycopy_management.inc,v 1.12 2006/12/05 05:34:15 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
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
				'images/up_arrow.gif',
				'images/down_arrow.gif',
				'js/bodycopy_edit_tables.js',
				'js/bodycopy_edit_divs.js',
				'js/bodycopy_edit_fns.js',
			   );

	}//end _getFileList()


}//end class

?>
