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
* $Id: cron_job_clear_matrix_cache.inc,v 1.8 2012/08/30 01:09:10 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Clear_Matrix_Cache
*
* Purpose
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Clear_Matrix_Cache extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Return name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		// the ownerid must be set at the creation of this cron job
		return 'Clear Matrix Cache Cron Job #'.$this->attr('ownerid');

	}//end _getName()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* Returns bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	accepted by reference, used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	protected function _exec(&$msg)
	{
		// based on the options in the tool interface and settings in the global prefs
		$assetid = Array($this->attr('ownerid'));
		$vars = Array(
					'assetid'		=> $assetid,
					'level'			=> $this->attr('level'),
					'type_codes'	=> $this->attr('type_codes'),
					'delete_all'	=> 'no',
					'repopulate'	=> $this->attr('repopulate'),
				);

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$status_errors = $hh->freestyleHipo('hipo_job_clear_cache', $vars);
		if (empty($status_errors)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(isset($assetid[0]) ? $assetid[0] : NULL);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

			if ($this->attr('type') == 'one_off') {
				// all OK, and we can be removed because we have done our one off task
				return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;
			} else if ($this->attr('type') == 'repeating') {
				// repeating time interval job, do not remove
				return SQ_CRON_JOB_COMPLETED;
			}
		} else {
			// process failed
			trigger_localised_error('CRON0057', translate('Failed to run clear Matrix cache cron job.'), E_USER_WARNING)
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}//end _exec()


	/**
	* Returns an array of scheduled (purge cache) jobs
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getActiveJobs()
	{
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		return $cm->getJobs('cron_job_clear_matrix_cache');

	}//end getActiveJobs()


}//end class

?>
