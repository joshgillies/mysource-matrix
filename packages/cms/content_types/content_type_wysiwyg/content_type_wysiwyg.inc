<?php

require_once dirname(__FILE__).'/../../content_type/content_type.inc';

/**
* Content_Type_WYSIWYG
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Content_Type_WYSIWYG extends Content_Type
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_WYSIWYG($assetid=0) 
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param string	$name	the name of this page
	*
	* @return boolean
	* @access public
	*/
	function create($name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a WYSIWYG Content Type with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {
			$this->setAttrValue('name', $name);
			return true;
		}

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
		return Array();

	}//end _getAllowedLinks()


	/*
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend()
	{
		switch($_POST['asset_action']) {

			default :
				return parent::processBackend();

		}// end switch

	}// end processBackend()


}//end class
?>
