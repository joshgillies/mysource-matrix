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
* $Id: comment.inc,v 1.20 2012/08/30 00:59:01 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

if (!defined('SQ_COMMENT_HTML_ALLOW')) {
	define ('SQ_COMMENT_HTML_ALLOW',	0);
}

if (!defined('SQ_COMMENT_HTML_ESCAPE')) {
	define ('SQ_COMMENT_HTML_ESCAPE',	1);
}

if (!defined('SQ_COMMENT_HTML_STRIP')) {
	define ('SQ_COMMENT_HTML_STRIP',	2);
}

/**
* Comment
*
*   Purpose
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment extends Page
{


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return strip_tags(parent::_getName($short_name, $contextid));
		} else {
			return strip_tags($values[$this->id]);
		}

	}//end _getName()


	/**
	* Overwrite the keyword replacement function for the name attribute
	*
	* @return string
	* @access public
	* @see Asset::getKeywordReplacement()
	*/
	function getAssetAttributeNameKeywordReplacement()
	{
		return $this->attr('name');

	}//end getAssetAttributeNameKeywordReplacement()


	/**
	* Overwrite the keyword replacement function for the name attribute
	*
	* @return string
	* @access public
	* @see Asset::getKeywordReplacement()
	*/
	function getAssetAttributeCommentKeywordReplacement()
	{
		return $this->attr('comment');

	}//end getAssetAttributeCommentKeywordReplacement()


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
	* Get the maximum rating keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCommentMaxRatingKeywordReplacement()
	{
		$length = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_MAX_RATING');
		return $length;

	}//end getCommentMaxRatingKeywordReplacement()


	/**
	* Get the integer rating keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCommentRatingIntegerKeywordReplacement()
	{
		$rating = $this->attr('rating');
		if ($rating == -1) return;

		return $this->_getRatingScaledInteger();

	}//end getCommentRatingIntegerKeywordReplacement()


	/**
	* Get the current rating keyword replacement (may not be an integer)
	*
	* @return string
	* @access public
	*/
	function getCommentRatingKeywordReplacement()
	{
		$rating = $this->attr('rating');
		if ($rating == -1) return;

		return round($this->_getRatingScaled(), 2);

	}//end getCommentRatingKeywordReplacement()


	/**
	* Get the rating scaled to the current maximum rating setting
	*
	* @return string
	* @access private
	*/
	function _getRatingScaled()
	{
		$rating_scale = $this->_getRatingLimit();
		$rating_absolute = $this->attr('rating');

		if (empty($rating_scale)) return 0;

		$rating = $rating_absolute/100 * $rating_scale;

		return $rating;

	}//end _getRatingScaled()


	/**
	* Get the rating scaled to the current maximum rating setting rounded to an integer
	*
	* @return string
	* @access private
	*/
	function _getRatingScaledInteger()
	{
		return round($this->_getRatingScaled());

	}//end _getRatingScaledInteger()


	/**
	* Convert an integer value to percent based on the current max setting
	*
	* @param int	$integer	value to convert to percent
	*
	* @return string
	* @access private
	*/
	function _convertIntegerRatingToPercent($integer)
	{
		if (empty($integer) || !is_numeric($integer) || $integer < 0) {
			return 0;
		}

		$rating_scale = $this->_getRatingLimit();

		if (empty($rating_scale)) return 0;

		if ($integer > $rating_scale) return 100;

		$percentage = $integer/$rating_scale * 100;

		return $percentage;

	}//end _convertIntegerRatingToPercent()


	/**
	* Get the value of the maximum possible rating setting
	*
	* @return string
	* @access private
	*/
	function _getRatingLimit()
	{
		$rating_max = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_MAX_RATING');
		return $rating_max;

	}//end _getRatingLimit()


	/**
	* Get the HTML block for printing the rating stars
	*
	* @return string
	* @access public
	*/
	function getCommentRatingImageHtmlBlockKeywordReplacement()
	{
		$max_rating = $this->_getRatingLimit();
		$rating = $this->attr('rating');
		if ($rating == -1) return;

		$rating = $this->_getRatingScaledInteger();

		$image_location_prefix = sq_web_path('data').'/asset_types/comment/files';

		$image_rated = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_IMG_RATED');
		if (empty($image_rated)) {
			$image_rated = $image_location_prefix.'/star.png';
		}

		$image_blank = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_IMG_BLANK');
		if (empty($image_blank)) {
			$image_blank = $image_location_prefix.'/star_grey.png';
		}

		$html = '';

		for ($i = 0; $i < $max_rating; $i++) {
			if ($i < $rating) {
				$alt = '*';
				$image = $image_rated;
			} else {
				$alt = '.';
				$image = $image_blank;
			}
			$html .= '<img src="'.htmlspecialchars($image).'" alt="'.$alt.'" />';
		}

		return $html;

	}//end getCommentRatingImageHtmlBlockKeywordReplacement()


	/**
	* Get the percentage keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCommentRatingPercentageKeywordReplacement()
	{
		$rating = $this->attr('rating');
		if ($rating == -1) return;

		return round($rating, 2);

	}//end getCommentRatingPercentageKeywordReplacement()


	/**
	* Return the keywords for this asset type.
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['comment_max_length'] = translate('Number of allowed characters in comment body');

		$keywords['comment_max_rating'] = 'Max Rating Value';
		$keywords['comment_rating'] = 'Rating Assigned to the comment (possibly a decimal)';
		$keywords['comment_rating_integer'] = 'Rating Assigned to the comment as integer';
		$keywords['comment_rating_percentage'] = 'Rating Assigned to the comment as percent';
		$keywords['comment_rating_image_html_block'] = 'Rating Assigned to the comment in form of images';

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get the value of an attribute
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed string|NULL
	* @access public
	*/
	function attr($name)
	{
		if (in_array($name, Array('name', 'short_name', 'comment'))) {
			return $this->_formatValueBasedOnPreference(parent::attr($name));
		} else if (in_array($name, Array('name_raw', 'short_name_raw', 'comment_raw'))) {
			return parent::attr(str_replace('_raw', '', $name));
		}
		return parent::attr($name);

	}//end attr()


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
		$caller = array_get_index(debug_backtrace(), 1);
		if (array_get_index($caller, 'class') == 'page_asset_builder') {
			if (in_array($name, Array('name', 'short_name', 'comment'))) {
				return TRUE;
			}
		}

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
				$value = nl2br(strip_tags($value));
			break;

			case SQ_COMMENT_HTML_ESCAPE:
				$value = nl2br(htmlspecialchars($value));
			break;

			case SQ_COMMENT_HTML_ALLOW:
				// nothing at the moment
			break;
		}

		$disable_keyword_conf = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS');

		if ($disable_keyword_conf) {
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = extract_keywords($value);

			foreach ($keywords as $keyword) {
				$value = str_replace('%'.$keyword.'%', $keyword, $value);
			}
		}//end if disable keyword replacements

		return $value;

	}//end _formatValueBasedOnPreference()


}//end class

?>
