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
* $Id: comment_edit_fns.inc,v 1.6 2006/12/20 03:36:39 arailean Exp $
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
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
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

		$size   = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_COMMENT_INPUT_SIZE');
		$length = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_COMMENT_MAX_LENGTH');

		if ($write_access) {
			text_area($prefix.'_comment', $asset->attr('comment'), 0, 0, $length, 'style="width: '.$size['width'].'; height: '.$size['height'].';"');
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

		if ($write_access) {
			$comment = array_get_index($_REQUEST, $prefix.'_comment');
			$length = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_COMMENT_MAX_LENGTH');

			if (!empty($length) && (strlen($comment) > $length)) {
				$comment = substr($comment, 0, $length);
			}

			$asset->setAttrValue('comment', $comment);
		}

		return TRUE;

	}//end processComment()


}//end class
?>