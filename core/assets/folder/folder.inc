<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Folder extends Asset
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Folder($assetid=0) 
	{
		$this->Asset($assetid);
	}


	/*
	* Create this asset
	*
	* @param string $name   the name of this folder
	*
	* @return boolean
	* @access public
	*/
	function create($name)
	{

		if (parent::create()) {

			$this->setAttribute('name',   $name);
			return true;

		}#end if

		return false;

	}#end create()

	/*
	* Returns name of the asset
	*
	* @ see Asset::name()
	* @access public
	*/
	function name() {
		return $this->attr('name');
	}#end name();


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_EXCLUSIVE => Array('asset' => 'M'),
					 SQ_LINK_UNITE     => Array('asset' => 'M'),
					 SQ_LINK_HREF      => Array('asset' => 'M')
					);

	}#end _getAllowedLinks()


}#end class
?>
