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
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Generate_Google_Analytics_Report
*
* Purpose
*	 Generates the data required by Edit+ to display the reports
*    for Google Analytics Views in the system.
*
* @author  Squiz Pty Ltd <products@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Cron_Job_Generate_Google_Analytics_Report extends Cron_Job
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
		return translate('Generate Google Analytics Report Scheduled Job');

	}//end _getName()


	/**
	* Get cron job display name
	*
	* @return string
	* @access public
	*/
	public function displayName()
	{
		return $this->_getName();

	}//end displayName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(SQ_LINK_NOTICE => Array('google_analytics_view' => Array('card' => 'M')));

	}//end _getAllowedLinks()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* This is a system job so it can't be removed.
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do.
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	protected function _exec(&$msg)
	{
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$viewids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('google_analytics_view');
		foreach ($viewids as $viewid) {
			$view = $GLOBALS['SQ_SYSTEM']->am->getAsset($viewid);
			$conn = $view->getConnector(); 
			
			if ($conn !== NULL) {
				$vars = Array('assetid' => $viewid);
				$hh->freestyleHipo('hipo_job_refresh_google_analytics_view', $vars, SQ_PACKAGES_PATH.'/google_analytics/hipo_jobs/');
			}	
		}//end foreach

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the
	* cron manager never deletes this job
	*
	* @return int
	* @access public
	*/
	function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_REMOVE) {
			$res = $res - SQ_CRON_JOB_REMOVE;
		}
		return $res;

	}//end run()


	


}//end class

?>
