<?php

require_once dirname(__FILE__).'/../../page/page.inc';

/**
* Page_Standard
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Page_Standard extends Page
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Standard($assetid=0) 
	{
		$this->Page($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	&$link	information used to create the initial link
	* @param string	$name	the name of this page
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Standard Page with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create($link, $name)) {
			
			// add a bodycopy to this page when creating
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
			$bodycopy = new Bodycopy();
			$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_EXCLUSIVE, 'dependant' => 1);
			if (!$bodycopy->create($copy_link, 'Page Contents')) return false;
			
			return true;

		}//end if

		return false;

	}//end create()

	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant);

	}// end createLink()



	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_EXCLUSIVE]['bodycopy'] = 'M';
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns actual link info for all links that this asset (but not any of it's kids) deems as dependant
	*
	* @return Array()
	* @access private
	*/
	function _getDependantLinks()
	{
		// we are dependant on our inbox and our workspace
		return array_merge(parent::_getDependantLinks(), $this->getLinks(SQ_LINK_EXCLUSIVE, 'bodycopy'));
	}// end _getDependantLinks()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		// get the bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy_data = $this->getLink(SQ_LINK_EXCLUSIVE, 'bodycopy');
		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['type_code']);
		$bodycopy->printBody();
	}// end printBody()


}//end class
?>
