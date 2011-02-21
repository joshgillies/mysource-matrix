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
* $Id: comment_management.inc,v 1.8 2011/02/21 02:48:28 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Comment_Management
*
* @see Asset_Management
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Comment_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['comment'] = Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_admin'	=> FALSE,
								 );

		$this->vars['rating'] = Array(
									'added'		=> '0.3',
									'type'		=> 'float',
									'default'	=> -1,
									'is_admin'	=> FALSE,
								);

	}//end constructor


	/**
	* Gets a list of all files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/star.png',
				'files/star_grey.png',
			   );

	}//end _getFileList()

	
}//end class

?>
