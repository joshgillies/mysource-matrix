<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Manager_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Cron_Manager_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars =	Array(
							'refresh_time' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 900,
								'description'	=> 'The number of seconds between cron job calls',
							),
							'last_run' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The last time that the cron manager was run',
							),
							'running' => Array(
								'added'			=> '0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'Are we currently running ?',
							),
						);


	}//end constructor

}//end class
?>
