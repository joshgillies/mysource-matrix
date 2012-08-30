<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge.inc,v 1.7 2012/08/30 00:59:22 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Squid_Cache_Purge
*
* Purpose
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Cron_Job_Squid_Cache_Purge extends Cron_Job
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
		return translate('squid_cron_name', $this->attr('ownerid'));

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();

		// make sure that we can link to every asset
		if (empty($links[SQ_LINK_NOTICE]['asset'])) {
			$links[SQ_LINK_NOTICE]['asset'] = Array('card' => 1);
		}

		return $links;

	}//end _getAllowedLinks()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For Cron Jobs this can only be done by the running user or a sys admin
	* For Future Status Jobs, admins of the asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		if (parent::canDelete()) return TRUE;

		// read-only?
		if ($this->attr('read_only')) return FALSE;

		// if we have admin access to the asset owning this job
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('ownerid'));
		if (!is_null($asset)) return $asset->adminAccess('');
		return FALSE;

	}//end canDelete()


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
		$assetid = $this->attr('ownerid');
		$vars = Array(
					'assetid'		=> $assetid,
					'cascade'		=> $this->attr('cascade'),
					'repopulate'	=> $this->attr('repopulate'),
				);

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$status_errors = $hh->freestyleHipo('hipo_job_tool_clear_squid_cache', $vars, SQ_PACKAGES_PATH.'/squid/hipo_jobs');

		if (empty($status_errors)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$msg = translate('squid_cron_done_msg', $asset->name, $assetid);
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
			trigger_localised_error('SQUID0002', E_USER_WARNING);
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
		return $cm->getJobs('cron_job_squid_cache_purge');

	}//end getActiveJobs()


}//end class

?>