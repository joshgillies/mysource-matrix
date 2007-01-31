<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_expired_workflow.inc,v 1.16.2.1 2007/01/31 00:17:04 rong Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Cron_Job_Expired_WorkFlow
*
* Purpose
*	Checks all the running workflows in the system and send messages to admins
*	of assets that have been in a workflow step for too long (ie longer than the
*	step expiry time defined in the workflow schema)
*
*	Note that this is a singleton asset - when the asset type is installed it creates a single
*	instance of itself under the cron manager, which should remain there always.
*
* @version $Revision: 1.16.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Expired_WorkFlow extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Job_Expired_WorkFlow($assetid=0)
	{
		$this->Cron_Job($assetid);

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
	function create(&$link)
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
	function _getName($short_name=FALSE)
	{
		return translate('cron_job_expired_workflow');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone this cron job
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Morphs this asset to either one of its parent types or one of its children types.
	*
	* Only succeeds if $new_type_code assets are allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CRON0024', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return FALSE;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the expired workflow cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT w.*, a.name
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_wflow w
					INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a
					ON w.assetid = a.assetid
				WHERE w.wflow IS NOT NULL
					AND w.granted = '.$db->quote('1');
		$rows = $db->getAll($sql);
		assert_valid_db_result($rows);
		$now = time();
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$last_run = $cm->attr('last_run');
		foreach ($rows as $row) {
			$workflow = unserialize($row['wflow']);
			$schemaid = $row['schemaid'];

			$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$step =& $wfm->getCurrentStep($workflow);

			if (($step['started'] != 0) && ($step['expiry_time'] != 0)) {
				$exp_time = $step['started'] + $step['expiry_time'];
				if (($exp_time < $now) && ($exp_time > $last_run)) {
					// workflow step has expired since the last cron run
					$this->_processExpiredWorkflowStep($row['assetid'], $row['name'], $workflow['schema_name'], $schemaid, $step);
					break;
				}
			}

		}
		unset($rows);
		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Deal with a workflow step that has been running for longer than its expiry time
	*
	* To do this we find the administrators of the asset and send them a message
	* explaining the situation.
	*
	* @param string	$assetid		ID of the asset whose workflow has expired
	* @param string	$asset_name		Name of that asset
	* @param string	$schema_name	Name of the workflow schema
	* @param string	$schemaid		Asset id of the workflow schema
	* @param array	$step_data		Workflow step data
	*
	* @return void
	* @access private
	*/
	function _processExpiredWorkflowStep($assetid, $asset_name, $schema_name, $schemaid, $step_data)
	{

		if (Workflow_Manager::silentWorkflowParty($assetid)) {
			return;
		}
		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$subject = 'Stale Workflow Step';
		$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'workflow'), FALSE);
		$href = str_replace('./', '', current($hrefs));
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$url = 'http://'.current($root_urls).'/'.$href;
		$type = 'asset.workflow.stale';
		$admins = $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, SQ_PERMISSION_ADMIN, TRUE, FALSE, TRUE);

		$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		if ($step_data['escalate']) {

			// ask the manager to escalate this workflow

			if (!$wfm->escalateWorkflow($assetid, $schemaid)) {
				trigger_localised_error('CRON0052', E_USER_WARNING, $schemaid, $assetid);
			}
		} else {
			$schema_workflows = $wfm->getSchemaWorkflows($assetid);
			$workflow =& $schema_workflows[$schemaid];

			// don't automatically escalate, just notify admins
			$msg_reps = Array(
							'asset_name'	=> $asset_name,
							'assetid'		=> $assetid,
							'step_id'		=> implode('.', $workflow['current_step']),
							'step_name'		=> $step_data['step_name'],
							'schema'		=> $schema_name,
							'started_time'	=> easy_datetime($step_data['started']),
							'expiry_time'	=> easy_time_total($step_data['expiry_time']),
							'workflow_url'	=> $url,
						);
			$msg = $ms->newMessage($admins, $type, $msg_reps);
			$msg->send();
		}

	}//end _processExpiredWorkflowStep()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the cron manager never deletes the
	* expired_workflow cron job
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
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
