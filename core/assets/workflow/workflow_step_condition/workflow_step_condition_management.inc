<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Condition_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Workflow_Step_Condition_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Workflow_Step_Condition_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('userid'    => Array(
											'added' => '0.1',
											'type'  => 'int', 'default'   => ''
											),
							'logic'     => Array(
											'added' => '0.1',
											'type'  => 'text', 'default'   => '>='
											),
							'logic_num' => Array(
											'added' => '0.1',
											'type'  => 'int', 'default'   => '1'
											)
							);
	}//end constructor

}//end class

?>