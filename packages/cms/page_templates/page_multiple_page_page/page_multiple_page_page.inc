<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | Squiz.net Commercial Module Licence                                |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: page_multiple_page_page.inc,v 1.7.2.2 2006/06/07 04:36:04 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Page_Multiple_Page_Page
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.7.2.2 $
* @package MySource_Matrix_Packages
* @subpackage CMS
*/
class Page_Multiple_Page_Page extends Bodycopy
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Multiple_Page_Page($assetid=0)
	{
		$this->Bodycopy($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_1]['page'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_1]['file'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['container'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['page'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['file'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Standard pages create a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$multi_page = $link['asset'];

		$new_page_number = strval($multi_page->getNumberOfPages());

		return $this->saveWebPaths(Array($new_page_number));

	}//end _createAdditional()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// We have to redirect to our parent, so that page numbers, and other stuff can be printed
		$parent = $this->getMultiplePageAsset();
		$parent->printPage($this);

	}//end printBody()


	/**
	* Return the ID of this assets parent multiple page
	*
	* @return string
	* @access public
	*/
	function getMultiplePageId()
	{
		if ($this->id != '0') {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'page_multiple_page');
			if (!empty($parents)) {
				return current(array_keys($parents));
			}
		}
		return '0';

	}//end getMultiplePageId()


	/**
	* Return this assets parent multiple page
	*
	* @return string
	* @access public
	*/
	function getMultiplePageAsset()
	{
		$multi_id = $this->getMultiplePageId();
		if ($multi_id != '0') {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($multi_id);
		}
		$null_minder = NULL;
		return $null_minder;

	}//end getMultiplePageAsset()


	/**
	* Return this assets parent multiple page
	*
	* @return string
	* @access public
	*/
	function getRawContent()
	{
		return parent::getRawBodycopyContent();

	}//end getRawContent()


	/**
	* As printBody() is overridden, this basically acts as that function
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		ob_start();
			parent::printBody();
		$content = ob_get_contents();
		ob_end_clean();

		return $content;

	}//end getContent()


	/**
	* Return the keywords for this asset type.
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('page_multiple_page');
		$parent =& new Page_Multiple_Page();
		return $parent->getAvailableKeywords();

	}//end getAvailableKeywords()


	/**
	* Performs a little bit of trickery, in order to get the parent to replace the keywords
	*
	* @param string	$value	The keyword to be replaced
	*
	* @return array
	* @access public
	*/
	function getKeywordReplacement($value)
	{
		$replacement = parent::getKeywordReplacement($value);
		if (is_null($replacement)) {
			$parent = $this->getMultiplePageAsset();
			$replacement = $parent->getKeywordReplacement($value);
		}

		return $replacement;

	}//end getKeywordReplacement()


}//end class

?>
