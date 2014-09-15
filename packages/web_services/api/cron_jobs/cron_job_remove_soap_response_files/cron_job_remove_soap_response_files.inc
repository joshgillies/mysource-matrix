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
* $Id: cron_job_remove_soap_response_files.inc,v 1.2 2012/08/30 00:59:45 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Cron_Job_Expired_Matrix_Cache
*
* Purpose : This cron job will remove all the folders and saved file
*			that have been saved under data/private/soap_action_file_responses
*			by our 'SOAP make request' Trigger Actions or Form Sub Actions
*
*
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Cron_Job_Remove_SOAP_Response_Files extends Cron_Job
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
	* Create this asset
	*
	* Return either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


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
		return translate('cron_job_remove_soap_response_files');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone this cron job
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* This Cron Job must stay put.
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Run this cron job
	*
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
		$dir_to_check = SQ_DATA_PATH.'/private/soap_action_file_responses/';
		// no dir found...nothing to do
		if (!is_dir($dir_to_check)) return SQ_CRON_JOB_COMPLETED;

		$session_dirs = list_dirs($dir_to_check, TRUE);
		// no sessions dirs found safely bail out
		// as we got nothing to delete here
		if (empty($session_dirs)) return SQ_CRON_JOB_COMPLETED;

		if (!self::remove_temp_files($session_dirs)) return SQ_CRON_JOB_ERROR;

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Runs this job
	*
	* This Cron Job is supposed to have eternal life (-:, so Cron Manager
	* is not allowed to remove it even if it wants to.
	*
	* @return int
	* @access public
	*/
	public function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_REMOVE) {
			$res = $res - SQ_CRON_JOB_REMOVE;
		}
		return $res;

	}//end run()


	/**
	* This function will get the temporary files
	* stored under the supplied directories and remove them
	* Once removed the files will also remove the directory
	*
	* @param array	$dirs	directories to look under
	*
	* @return boolean
	* @access public
	*/
	public static function remove_temp_files($dirs=Array())
	{
		$success = TRUE;
		foreach ($dirs as $dir) {
			// delete all(1) files under this dir
			if (!clear_directory($dir)) {
				$success = FALSE;
			} else {
				// if the file deletion was super awesome
				// go ahead and remove the dir itself too
				if (!delete_directory($dir)) $success = FALSE;
			}
		}// end foreach

		return $success;

	}// end remove_temp_files()


}//end class



?>
