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
* $Id: content_type_structure_tree_management.inc,v 1.4 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Structure_Tree_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Structure_Tree_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['structure'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'serialise',
									'default'	=> Array(),
								   );
		$this->vars['trailers'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> 'true',
								  );

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/arrow_left_blue.png',
				'files/arrow_down_blue.png',
				'files/arrow_right_blue.png',
				'files/arrow_up_blue.png',
				'files/text_bold.png',
				'files/text_italics.png',
				'files/text_underlined.png',
				'files/spacer.gif',
				'files/add.png',
			   );

	}//end _getFileList()


}//end class

?>
