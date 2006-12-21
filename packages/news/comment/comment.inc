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
* $Id: comment.inc,v 1.11 2006/12/21 00:42:48 arailean Exp $
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
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment extends Page
{


	/**
	* Return a name to use to describe this instance of the asset
	*
	* This function should be overridden by children as it is used to set the <i>name</i> and <i>short_name</i>
	* values for this asset when updating.
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see _updated
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


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
	* Return the keywords for this asset type.
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['comment_body'] = translate('news_comment_body');
		$keywords['comment_title'] = translate('news_comment_title');
		$keywords['comment_max_length'] = translate('news_comment_max_length');

		return $keywords;

	}//end getAvailableKeywords()


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
