<?php

require_once SQ_CORE_PACKAGE_PATH.'/image/image.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Thumbnail
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Thumbnail extends Image
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Thumbnail($assetid=0)
	{
		$this->Image($assetid);
	}//end constructor


	/**
	* Create this asset
	*
	* @param array(string)	&$link	information used to create the initial link
	* @param string			$name	the name of this file
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create an Thumbnail with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create($link, $name)) return true;

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowedLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array(),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


}//end class
?>
