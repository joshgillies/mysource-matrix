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
* $Id: comment_edit_fns.inc,v 1.3 2006/12/19 23:34:06 arailean Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Comment_Edit_Fns
*
*   Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Paint the comment interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintComment(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$width = 60;
		$height = 20;
		$max_length = NULL;

		if ($write_access) {
			text_area($prefix.'_comment', $asset->attr('comment'), $width, $height, $max_length);
		} else {
			echo $asset->attr('comment');
		}

		return TRUE;

	}//end paintComment()


	/**
	* Process the comment interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processComment(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$comment = array_get_index($_REQUEST, $prefix.'_comment');

		if ($write_access) {
			$asset->setAttrValue('comment', $comment);
		}

		return TRUE;

	}//end processComment()

}//end class
?>