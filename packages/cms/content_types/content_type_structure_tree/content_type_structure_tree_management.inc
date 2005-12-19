<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: content_type_structure_tree_management.inc,v 1.1 2005/12/19 01:05:04 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Structure_Tree_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Structure_Tree_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Structure_Tree_Management(&$pm)
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