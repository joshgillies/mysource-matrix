<?php

require_once dirname(__FILE__).'/../../content_type/content_type.inc';

/**
* Content_Type_Nest_Content
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Content_Type_Nest_Content extends Content_Type
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_Nest_Content($assetid=0) 
	{
		$this->Content_Type($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	&$link	information used to create the initial link
	* @param string			$name	the name of this page
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Nest Content Content Type with a blank name', E_USER_WARNING);
			return false;
		}

		if ($linkid = parent::create($link)) {
			$this->setAttrValue('name', $name);
			return $linkid;
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


}//end class
?>
