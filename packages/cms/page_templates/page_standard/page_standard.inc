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
	* @param array(string)	$link	information used to create the initial link
	* @param string	$name	the name of this page
	*
	* @return boolean
	* @access public
	*/
	function create($link, $name)
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
			$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_EXCLUSIVE);
			if (!$bodycopy->create($copy_link, 'Page Contents')) return false;
			
			return true;

		}//end if

		return false;

	}//end create()


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
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['type_code']);
		$bodycopy->printBody();
	}// end printBody()


}//end class
?>
