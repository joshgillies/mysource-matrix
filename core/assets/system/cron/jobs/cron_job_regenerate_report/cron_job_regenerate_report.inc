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
* $Id: cron_job_regenerate_report.inc,v 1.6 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Regenerate_Report
*
* Purpose
*   Regenerates the report asset some time in the future
*   If repeat frequency is set, a repeating time interval job will be scheduled
*   Else a one off job wil be scheduled
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.6 $
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
		$id = $this->attr('asset');
		if ($GLOBALS['SQ_SYSTEM']->am->assetExists($id)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
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
	protected function _exec(&$msg)
	{
		$this->setAttrValue('running', TRUE);
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$assetid = $this->attr('asset');
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$user = $this->getRunningUser();

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
	public function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_COMPLETED) {
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'all')) {
				$this->setAttrValue('running', FALSE);
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');
			}
		}

		return $res;

	}//end run()


}//end class

?>
