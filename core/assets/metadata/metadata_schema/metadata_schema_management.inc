<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Schema_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Metadata_Schema_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
											'added'     => '0.1',
											'type'      => 'text', 'default'   => ''
											)
							);
	}//end constructor

}//end class

?>