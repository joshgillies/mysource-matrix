<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Redirect_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Page_Redirect_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Redirect_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('redirect_url' => Array(
													'added'     => '0.0.1',
													'type'      => 'text',  'default'   => '',
													'update_fn' => ''
													),
							);

	}//end constructor


}//end class

?>