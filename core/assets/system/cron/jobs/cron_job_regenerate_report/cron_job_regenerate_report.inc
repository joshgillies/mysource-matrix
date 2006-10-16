<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_regenerate_report.inc,v 1.1 2006/10/16 05:14:29 rong Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

/**
* Cron_Job_Regenerate_Report
*
* Purpose
*   Regenerates the report asset some time in the future
*   If repeat frequency is set, a repeating time interval job will be scheduled
*   Else a one off job wil be scheduled
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Regenerate_Report extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Job_Regenerate_Report ($assetid=0)
	{
		$this->Cron_Job($assetid);

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
	function _getName($short_name=FALSE)
	{
		$id = $this->attr('asset');
		if ($GLOBALS['SQ_SYSTEM']->am->assetExists($id)) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
			$name = $asset->name;
		}
		return "Regenerate \"$name\" [#$asset->id]";

	}//end _getName()


	/**
	* Run this cron job
	*
	* Looks at all the currently-running workflows and processes any whose current step has
	* expired since the last run of the cron manager
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	function _exec(&$msg)
	{
		$this->setAttrValue('running', TRUE);
		$this->saveAttributes();
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$assetid = $this->attr('asset');
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$user =& $this->getRunningUser();

		// pass who/when info to the hipo var
		$vars = Array(
					'userid'			=> $user->id,
					'generated_date'	=> date('F j, Y, g:i a'),
					'assetid'			=> $assetid,
					'asset'				=> $asset,
					'type_code'			=> $asset->type(),
					'complete'			=> FALSE,
				);

		// doing freestyle hipo because we cant queue hipo without browser refresh
		$status_errors = $hh->freestyleHipo('hipo_job_generate_report', $vars);
		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Runs this job
	*
	* @return int
	* @access public
	*/
	function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_COMPLETED) {
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'all')) {
				$this->setAttrValue('running', FALSE);
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');
			}
		}

		return $res;

	}//end run()


}//end class

?>
