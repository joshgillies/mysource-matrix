<?php

require_once SQ_CORE_PACKAGE_PATH.'/design_area/design_area.inc';

/**
* Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design extends Design_Area
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Design($assetid=0) 
	{
		$this->Design_Area($assetid);
	}


}//end class
?>
