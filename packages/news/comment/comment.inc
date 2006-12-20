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
* $Id: comment.inc,v 1.8 2006/12/20 23:40:20 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';


define ('SQ_COMMENT_HTML_ALLOW',	0);
define ('SQ_COMMENT_HTML_ESCAPE',	1);
define ('SQ_COMMENT_HTML_STRIP',	2);

/**
* Comment
*
*   Purpose
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment extends Page
{


	/**
	* Get keyword replacement for the formatted comment title
	*
	* @return string
	* @access public
	* @see Comment::_formatValueBasedOnPreference()
	*/
	function getCommentTitleKeywordReplacement()
	{
		return $this->_formatValueBasedOnPreference($this->attr('name'));

	}//end getCommentTitleKeywordReplacement()


	/**
	* Get keyword replacement for the formatted comment body
	*
	* @return string
	* @access public
	* @see Comment::_formatValueBasedOnPreference()
	*/
	function getCommentBodyKeywordReplacement()
	{
		return $this->_formatValueBasedOnPreference($this->attr('comment'));

	}//end getCommentBodyKeywordReplacement()


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
		$length = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_MAX_LENGTH');
		return $length;

	}//end getCommentMaxLengthKeywordReplacement()


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'comment':
				$length = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_MAX_LENGTH');
				if (!empty($length) && (strlen($value) > $length)) {
					$value = substr($value, 0, $length);
				}
				$value = trim($value);
			break;

			case 'name':
				$value = trim($value);
			break;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Format a value based on user preferences
	*
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function _formatValueBasedOnPreference($value)
	{
		$html_conf = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_HTML_CONF');
		$value = html_entity_decode($value);

		switch ($html_conf) {
			case SQ_COMMENT_HTML_STRIP:
				$value = strip_tags($value);
			break;

			case SQ_COMMENT_HTML_ESCAPE:
				$value = htmlspecialchars($value);
			break;

			case SQ_COMMENT_HTML_ALLOW:
				// nothing at the moment
			break;
		}

		return $value;

	}//end _formatValueBasedOnPreference()


}//end class

?>
