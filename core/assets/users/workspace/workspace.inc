<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Workspace
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Workspace extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Workspace($assetid=0)
	{
		$this->Asset($assetid);
	}//end constructor


	/**
	* Gets the user that this workspace is attached to
	*
	* @return object User
	* @access private
	*/
	function &_getUser()
	{
		$link = $this->getLink(SQ_LINK_EXCLUSIVE, 'user', false, '', 'minor');
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['type_code']);
		return $user;
	}


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return 'Workspace';
	}//end _getName();

}//end class
?>
