<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Design_Area_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Design_Area_Edit_Fns() 
	{
		$this->Asset_Edit_Fns();
	}

	/**
	* Echo's the Design Areas id name
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @access public
	*/
	function paintIdName(&$asset, &$o, $prefix) 
	{
		echo $asset->attr('id_name');
	}// end paintIdName()


}//end class
?>
