<?php

require_once SQ_CORE_PACKAGE_PATH.'/file/file.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Image
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Image extends File
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array('gif', 'jpg', 'jpeg', 'png');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Image($assetid=0) 
	{
		$this->File($assetid);
	}//end constructor


	/**
	* Create this asset
	*
	* @param string			$name	the name of this file
	* @param array(string)	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create an Image with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create($link, $name)) return true;

		return false;

	}//end create()

}//end class
?>
