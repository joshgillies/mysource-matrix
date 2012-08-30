<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_metadata_triggers_edit_fns.inc,v 1.3 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Metadata_Triggers_Edit_Fns
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Metadata_Triggers_Edit_Fns extends Cron_Job_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Find out if Use Last Run option is diabled
	* Returns TRUE if it is disabled so that Minimum Date attribute needs to be printed
	*
	* @param object	&$asset	the file asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function useCronLastRunDisabled(&$asset, $prefix) {
		$last_run_enabled = $asset->attr('use_cron_last_run');
		
		// this is asking if it's disabled, so, flip the attribute value 
		return (!$last_run_enabled);
	}
}//end class
?>