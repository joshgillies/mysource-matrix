<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Page
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Page extends Asset
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Page($assetid=0) 
	{
		$this->Asset($assetid);
	}

	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_EXCLUSIVE => Array('page_template' => 1),
					 SQ_LINK_UNITE     => Array('page' => 'M', 'file' => 'M', 'permissions_group' => 'M'),
					 SQ_LINK_NOTICE    => Array('page' => 'M')
					);

	}#end _getAllowedLinks()

}#end class
?>
