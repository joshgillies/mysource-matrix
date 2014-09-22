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
* $Id: cron_job_expired_workflow.inc,v 1.45 2012/08/30 01:09:11 ewang Exp $
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
* @version $Revision: 1.45 $
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
		return translate('Expired Workflow Scheduled Job');


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
	* For the expired workflow cron job, NO!  There must always be exactly one of these here.
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
		try {
			if (MatrixDAL::getDbType() === 'oci') {
				$sql = 'SELECT
							aw.assetid, aw.schemaid, aw.wflow , aw.granted , a.name
						FROM
							sq_ast_wflow aw, sq_ast a
						WHERE
							aw.assetid = a.assetid
						AND
							DBMS_LOB.GETLENGTH(aw.wflow) > 0
						AND
							aw.granted = 1';

				$query = MatrixDAL::preparePdoQuery($sql);
				$rows  = MatrixDAL::executePdoAll($query);
			} else {
				$rows = MatrixDAL::executeAll('cron_job_expired_workflow', 'getAllRunningWorkflows');
			}
		} catch (DALException $e) {
			throw new Exception ('Unable to get running workflows due to database error: '.$e->getMessage());
		}

		$now = time();
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$last_run = $cm->attr('last_run');

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		foreach ($rows as $row) {
			$ms->openQueue();

			$workflow = unserialize($row['wflow']);
			$schemaid = $row['schemaid'];
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$step =& $wfm->getCurrentStep($workflow); // need by ref

			if (($step['started'] != 0) && ($step['expiry_time'] != 0)) {
				$exp_time = $step['started'] + $step['expiry_time'];

				if ($exp_time < $now) {
					// workflow step has expired since the last cron run

					if ($step['escalate']) {
						// Escalate the job, at any time
						if (!$wfm->escalateWorkflow($row['assetid'], $schemaid)) {
							trigger_localised_error('CRON0052', sprintf(translate('Failed to escalate schema %1$s for asset %2$s.'), $schemaid, $assetid), E_USER_WARNING)
						}
					} else {
						// Send a stale message, but only if this is the first
						// time the cron has run that the workflow has gone stale
						if ($exp_time > $last_run) {
							$this->_processExpiredWorkflowStep($row['assetid'], $row['name'], $workflow['schema_name'], $schemaid, $step);
						}
					}
					//break;
				}
			}

			// Don't send escalation message for all dependants (See #5034)
			$escalation_wf_msgs = Array();
			foreach($ms->queueContents() as $queued_message) {
				$ms_assetid =  $queued_message->parameters['assetid'];
				if ($ms_assetid) {
					$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($ms_assetid, '', TRUE, FALSE);
					foreach($parents as $key => $parentid) {
						if ($ms_assetid == $parentid) {
							unset($parents[$key]);
						}
					}//end foreach

					// If asset has no dependant parents or more than 1 dependant parents, send the workflow message
					if(count($parents) > 1 || empty($parents)) {
						$escalation_wf_msgs[] = $queued_message;
					} else {
						// The asset's only dependant parent is not in workflow, it means workflow was started at this asset
						$dependant_workflows = $wfm->getSchemaWorkflows($parents[0]);
						if (empty($dependant_workflows[$schemaid]['steps']) || (isset($dependant_workflows[$schemaid]['complete']) && $dependant_workflows[$schemaid]['complete'])) {
							$escalation_wf_msgs[] = $queued_message;
						}//end if

					}//end else
				}//end if
			}//end foreach

			$ms->abortQueue();

			// Send all the relevant escalation workflow messages
			$ms->send($escalation_wf_msgs);
			unset($escalation_wf_msgs);
		}
		unset($rows);

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Deal with a workflow step that has been running for longer than its expiry
	* time, but should not be escalated
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
	protected function _processExpiredWorkflowStep($assetid, $asset_name, $schema_name, $schemaid, $step_data)
	{
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		if ($wfm->silentWorkflowParty($assetid)) {
			return;
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$subject = 'Stale Workflow Step';
		$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'workflow'), FALSE);
		$href = str_replace('./', '', current($hrefs));
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$url = 'http://'.current($root_urls).'/'.$href;
		$type = 'asset.workflow.stale';
		$admins = $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, SQ_PERMISSION_ADMIN, TRUE, FALSE, TRUE);

		$schema_workflows = $wfm->getSchemaWorkflows($assetid);
		$workflow =& $schema_workflows[$schemaid];
		$schema_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

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
						'asset_url'		=> $asset->getDependantParentsURL(),
					);

		// If there are no admins, send it to system admins instead
		if (empty($admins)) $admins = Array(0);
		$msg = $ms->newMessage($admins, $type, $msg_reps);
		$msg->parameters['assetid'] = $assetid;

		// Does this step have a custom message for stale workflow?
		if (isset($step_data['message_stale']) && trim($step_data['message_stale']) !== '' ) {
			$msg->body = trim($step_data['message_stale']);
		}

		// Does this step have a custom subject for stale workflow?
		if (isset($step_data['subject_stale']) && trim($step_data['subject_stale']) !== '' ) {
			$subject = trim(array_get_index($step_data, 'subject_stale', 'Stale Workflow Step'));
			$msg->subject = (!empty($subject)) ? $subject : 'Stale Workflow Step';
		}//end if

		if (!is_null($schema_asset)) {
			if ($schema_asset->attr('schema_reply_to_email_address') != '') {
				$msg->parameters['reply_to'] = $schema_asset->attr('schema_reply_to_email_address');
			} else {
				$msg->parameters['reply_to'] = $msg->from;
			}
			if ($schema_asset->attr('schema_from_email_address') != '') {
				$msg->from = $schema_asset->attr('schema_from_email_address');
			}
		}

		$wfm->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
		$ms->enqueueMessage($msg);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

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
