<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Page_Edit_Fns extends Asset_Edit_Fns
{
	/**
	* All available protocols
	* @var Array(string)
	*/
	var $protos = Array('http', 'https');

	/**
	* Constructor
	*
	*/
	function Page_Edit_Fns() 
	{
		$this->Asset_Edit_Fns();
	}

}//end class
?>
