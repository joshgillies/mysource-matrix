<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Lineage_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Lineage_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars =	Array(
							'type' => Array(
								'default'		=> 'one_off',
							),
							'delete_linkid' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'If we need to delete an existing link in this process, this is it\'s linkid',
							),
							'link_info' => Array(
								'added'			=> '0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'	=> 'The link information similar in form to what is passed to Asset::create(), minus the "asset" element',
							),
						);

	}//end constructor

}//end class
?>
