<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars =	Array(
							'type' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'default'		=> 'one_off',
								'parameters'	=> Array(
									'options' => Array(
										'one_off'	=> 'One Off', 
										'repeating'	=> 'Repeating'
									),
								),
							),
							'when' => Array(
								'added'			=> '0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'This is an encoded description of when this job is to run',
							),
						);

	}//end constructor

}//end class
?>
