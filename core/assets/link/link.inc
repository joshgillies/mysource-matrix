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
* $Id: link.inc,v 1.2 2009/09/09 07:23:22 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Link
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Link extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Links ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()
	
	
	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Links add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);
		
	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Called by the design to print the body of this asset
	*
	* Print the HTML Link based on the selected link and relation on the Details screen
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		$link_str = '<a href="'.$this->_getLinkURL().'"';
		
		$relation = $this->attr('link_relation');
		if (!empty($relation)) {
			$link_str .= ' rel="'.$relation.'"';
		}
		
		$link_str .= '>'.$this->_getName().'</a>';
		
		echo $link_str;

	}//end printBody()
	
	
	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();
		
		$asset = new Asset();
		
		foreach ($asset->getAvailableKeywords() as $keyword => $desc) {
			$keywords['link_target_'.$keyword] = 'Referring Asset: '.$desc;
		}
		$keywords['link_url'] = 'The Link URL for this asset';
		$keywords['link_relation'] = 'Link relation for this asset';
		
		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		if (preg_match('|^link\_target\_(.*)|', $keyword, $matches)) {
			$linked_asset = $this->getLinkedAsset();
			
			if ($linked_asset) {
				return $linked_asset->getKeywordReplacement($matches[1]);
			} else {
				return '';
			}
		} else if ($keyword === 'link_url') {
			return $this->_getLinkURL();
		} else if ($keyword === 'link_relation') {
			return $this->_getLinkRelation();
		} else {
			return parent::getKeywordReplacement($keyword);
		}

	}//end getKeywordReplacement()


	/**
	* Get linked asset (only if its an internal Matrix asset)
	*
	* @return object|NULL
	* @access public
	*/
	function &getLinkedAsset()
	{	
		$linked_assetid = $this->attr('link_asset_id');
		$linked_url = $this->attr('link_url');
		
		$linked_asset = NULL;
		
		// If we have assetid of linked asset then just get the asset, else try getting it from url
		if (!empty($linked_assetid)) {
			$linked_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($linked_assetid);
			
		} else if (!empty($linked_url)) {
			// Strip protocal info and trailing query string from the url
			$linked_url = preg_replace('|^.*://|', '', $linked_url);
			$linked_url = preg_replace('|\?.*$|', '', $linked_url);

			$linked_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $linked_url, FALSE, TRUE);
		}

		return $linked_asset;

	}//end getLinkedAsset()
	
	
	/**
	* Get the replacement for the link_url keyword
	*
	* @return string
	* @access public
	*/
	public function getLinkUrlKeywordReplacement()
	{
		return $this->_getLinkURL();

	}//end getLinkUrlKeywordReplacement()
	
	
	/**
	* Get the URL specified in this asset. It can be a text attribute or the URL of a selected asset
	* 
	* @return string
	* @access private
	*/
	private function _getLinkURL()
	{
		$url = $this->attr('link_url');
		if (empty($url) || (trim($url) == '')) {
			$assetid = $this->attr('link_asset_id');
			if (empty($assetid)) {
				$url = '';
			} else {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$url = $asset->getURL();
			}
		}
		
		return $url;
		
	}//end _getLinkURL()
	
	
	/**
	* Get the Link relation attribute of this asset
	* 
	* @return string
	* @access private
	*/
	private function _getLinkRelation()
	{
		return $this->attr('link_relation') ? $this->attr('link_relation') : '';
		
	}//end _getLinkRelation()

}//end class
?>