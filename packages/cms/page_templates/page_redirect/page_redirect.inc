<?php

require_once dirname(__FILE__).'/../../page/page.inc';

/**
* Page_Redirect
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Page_Redirect extends Page
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Redirect($assetid=0)
	{
		$this->Page($assetid);

	}//end constructor


	/**
	* Prints out the Frontend for the asset
	*
	* The user is redirected to another location, so no design is printed
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		header('Location: '.$this->attr('redirect_url'));

	}// end printFrontend()


	/**
	* Prints out the body of the asset
	*
	* This function just calls printFrotnend()
	*
	* @return void
	* @access public
	* @see printFrontend()
	*/
	function printBody()
	{
		$this->printFrontend();

	}// end printBody()


}//end class

?>