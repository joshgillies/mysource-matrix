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
* $Id: cron_job_expired_matrix_locks.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Expired_Matrix_Locks
*
*
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Expired_Matrix_Locks extends Cron_Job
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
		return translate('cron_job_expired_matrix_locks');

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
        $class_name = 'locking_method_'.SQ_CONF_LOCKING_METHOD;
        $GLOBALS['SQ_SYSTEM']->am->includeAsset($class_name);

        $canDelete = call_user_func(array($class_name, 'supportsDeletingExpiredLocks'));

        if ($canDelete) {
            $canDelete = call_user_func(array($class_name, 'deleteExpiredLocks'));
        }

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


}//end class



?>
