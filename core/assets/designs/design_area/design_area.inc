<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Design_Area
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area extends Asset
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Design_Area($assetid=0) 
	{
		$this->Asset($assetid);
	}


}//end class
?>
