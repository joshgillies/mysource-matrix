<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Status_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Status_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Status_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars =	Array(
							'type' => Array(
								'default'		=> 'one_off',
							),
							'status' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The status to change the asset to',
							),
						);

	}//end constructor

}//end class
?>
