<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Cron_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Cron_Manager_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Cron_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens =	Array(
									'details' => Array(
										'name'			=> 'View Jobs',
										'force_unlock'	=> true,
									),
								);

	}//end constructor


	/**
	* Prints the tasks interface
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintJobs(&$asset, &$o, $prefix)
	{
		$o->openField('', 'new_line');

		pre_echo($asset->getLinks(SQ_LINK_TYPE_3, 'cron_job', false));

		return true;
	}//end paintJobs()

}//end class
?>
