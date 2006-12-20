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
* $Id: comment.inc,v 1.4 2006/12/20 03:35:01 arailean Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Comment
*
*   Purpose
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment extends Page
{


	/**
	* Get the maximum allowed length of comment input box
	*
	* Returns the setting from the Global Preferences screen.
	* 0 (zero) means no limit
	*
	* @return string
	* @access public
	*/
	function getCommentMaxLengthKeywordReplacement()
	{
		$length = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_COMMENT_MAX_LENGTH');

		return $length;

	}//end getCommentMaxLengthKeywordReplacement()


}//end class

?>
