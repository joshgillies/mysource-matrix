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
							'epoch' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The "first" run that the Cron Manager made (or will make) is at this time, this is used in conjunction with the refresh time to generate the times that the Operating System\'s Cron (or other scheduler) will be initiating a cron run, mainly so that if a current script that is already running runs over into that period it does the next periods work as well',
							),
							'refresh_time' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 900,
								'description'	=> 'The number of seconds between cron runs, whenever you change this update the "epoch" to the first time that this change will be felt',
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
							'run_check' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The number of times that a cron run has been started and stopped because the running flag is set true, this get\'s reset in run() after each pass over what jobs have to be done',
							),
							'warn_after_num_run_checks' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 3,
								'description'	=> 'When run_check hits this number a warning will be sent out to announce the possible un-removed locking',
							),
							'num_failed_attempts' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 3,
								'description'	=> 'The number of times that a cron job can fail and request to have another attempt at running',
							),
						);


	}//end constructor

}//end class
?>
