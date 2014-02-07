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
* $Id: bodycopy_management.inc,v 1.20 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'	=> Array(
									'added'			=> '0.0.1',
									'type'			=> 'text',
									'default'		=> 'BodyCopy',
									'update_fn'		=> '',
									'description'	=> 'the name of the bodycopy asset',
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
					  );

	}//end constructor


	/**
	* Gets a list of all files that the bodycopy wants web accessable
	*
	* @return array
	* @access private
	*/
	public function _getFileList()
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
				'images/icons/source_mode.png',
				'images/icons/accessibility_pass.png',
				'images/icons/accessibility_fail.png',
				'images/icons/htmltidy_disabled.png',
				'images/icons/htmltidy_pass.png',
				'images/icons/htmltidy_fail.png',
				'images/icons/htmltidy_wait.png',
				'images/icons/conditions.png',
				'images/icons/conditions_set.png',
				'images/icons/viper_logo.png',
				'images/up_arrow.gif',
				'images/down_arrow.gif',
				'js/bodycopy_edit_tables.js',
				'js/bodycopy_edit_divs.js',
				'js/bodycopy_edit_fns.js',
			   );

	}//end _getFileList()


}//end class

?>
