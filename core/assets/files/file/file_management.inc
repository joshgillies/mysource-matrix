<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class File_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function File_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  'default'   => '',
												'update_fn' => ''
												)
							);

	}//end constructor

}//end class
?>
