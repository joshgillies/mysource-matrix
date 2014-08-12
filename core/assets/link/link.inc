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
* $Id: link.inc,v 1.10 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.10 $
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
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['page']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['file']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['data_source'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['link']   = Array('card' => 'M', 'exclusive' => FALSE);

		$allowed[SQ_LINK_TYPE_2]['page']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['file']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['data_source'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['link']   = Array('card' => 'M', 'exclusive' => FALSE);

		$allowed[SQ_LINK_TYPE_3]['page'] = Array('card' => 'M', 'exclusive' => FALSE);

		$allowed[SQ_LINK_NOTICE]['page'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_NOTICE]['link'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()
	
	
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

		// Obtain the attribute value for Name from the Default Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

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
		$replacement = NULL;

		// Remove any modifiers from keyword
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);
		$contextid = extract_context_modifier($modifiers);

		if ($contextid !== NULL) {
			// If we were able to extract a context ID to change to, and it's
			// different to our current one, then change and then reload a copy
			// of the asset in that context (as we would need to do anyway)

			if ((int)$contextid !== $GLOBALS['SQ_SYSTEM']->getContextId()) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);

				// Get the keyword without any modifiers
				$replacement = $contexted_asset->getKeywordReplacement($keyword);

				// Then apply the ones we had remaining, then return it
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $contexted_asset->id));
				
				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return $replacement;

			}//end if contextid is not the currently active context

		}//end if contextid is not NULL
		
		if (preg_match('|^link\_target\_(.*)|', $keyword, $matches)) {
			$linked_asset = $this->getLinkedAsset();
			
			if ($linked_asset) {
				$replacement = $linked_asset->getKeywordReplacement($matches[1]);
			} else {
				$replacement = '';
			}
		} else if ($keyword === 'link_url') {
			$replacement = $this->_getLinkURL();
		} else if ($keyword === 'link_relation') {
			$replacement = $this->_getLinkRelation();
		} else {
			return parent::getKeywordReplacement($full_keyword);
		}

		if (count($modifiers) > 0) {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		}
		
		return $replacement;

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
			$asset = NULL;
			$assetid = $this->attr('link_asset_id');
			$url = '';
			if (!empty($assetid)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
				if (!is_null($asset)) $url = $asset->getURL();
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
