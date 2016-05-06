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
* $Id: cron_job_refresh_cache.inc,v 1.7 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Refresh_Cache
*
* Purpose
*   Regenerates the report asset some time in the future
*   If repeat frequency is set, a repeating time interval job will be scheduled
*   Else a one off job wil be scheduled
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Refresh_Cache extends Cron_Job
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
		if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($id)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
			$name = $asset->name;
		} else {
			$name = translate('Unknown asset');
		}
		return "Refresh Cache \"$name\" [#$id]";

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
		// Caching Off? Do nothing
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		if (!$cm->attr('enabled')) {
			return SQ_CRON_JOB_COMPLETED;
		}
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$assetid = $this->attr('asset');
		if (!$am->getAssetInfo($assetid)) {
			$readable_asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
			trigger_localised_error('CRON0066', sprintf('Unable to run the %1$s #%2$s, the assoicated asset #%3$s does not exist', $readable_asset_type, $this->id, $assetid), E_USER_WARNING);

			return SQ_CRON_JOB_COMPLETED;
		}
		
		// Start
		$this->setAttrValue('running', TRUE);
		$this->saveAttributes();

		// Clear the existing cache
		$cm->clearCache(Array($assetid));
		$asset = $am->getAsset($assetid);
		$urls_info = $asset->getURLs();
		if (empty($urls_info)) {
			return SQ_CRON_JOB_COMPLETED;
		} else {
			foreach ($urls_info as $url_info) {
				if ($url_info['http'] == '1') {
					$urls[] = 'http://'.$url_info['url'];
				}
				if ($url_info['https'] == '1') {
					$urls[] = 'https://'.$url_info['url'];
				}
			}
		}

		// Request each URL as public user
		$public_user  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
		$GLOBALS['SQ_SYSTEM']->setCurrentUser($public_user);

		// Re-generate the cache per URL
		foreach ($urls as $url) {
			$url_info = parse_url($url);

			// Let's pretend this is web accessed
			$_SERVER['HTTP_HOST'] = $url_info['host'];
			$_SERVER['PHP_SELF'] = $url_info['path'];
			$_SERVER['HTTP'] = ($url_info['scheme'] == 'http') ? 'on' : 'off';
			$_SERVER['HTTPS'] = ($url_info['scheme'] == 'https') ? 'on' : 'off';

			// Re-generate cache
			ob_start();
			$asset->printBody();
			ob_end_clean();
		}
		$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

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
				$this->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');
			}
		}

		return $res;

	}//end run()


}//end class


?>
