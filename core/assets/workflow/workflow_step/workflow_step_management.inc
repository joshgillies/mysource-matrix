<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Workflow_Step_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name'      => Array(
											'added'     => '0.1',
											'type'      => 'text', 'default'   => ''
											),
							'logic'     => Array(
											'added'     => '0.1',
											'type'      => 'text', 'default'   => 'all'
											),
							'logic_num' => Array(
											'added'     => '0.1',
											'type'      => 'int', 'default'   => '0'
											)
							);
	}//end constructor

}//end class

?>