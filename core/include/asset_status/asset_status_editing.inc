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
* $Id: asset_status_editing.inc,v 1.55 2013/09/25 07:42:08 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Editing
*
* Purpose
*    Provide a Safe Edit status for an asset.
*
* Valid Transitions
*    EDITING -> EDITING_APPROVAL if there is a workflow and user has write access
*    EDITING -> LIVE             if the user has write access (cancel safe edit)
*    EDITING -> EDITING_APPROVED if no workflow and user is an admin OR
*                                there is workflow but current user's approval will complete
*                                the workflow the workflow and all required metadata is complete
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.55 $
* @package MySource_Matrix
*/
class Asset_Status_Editing extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing($owner)
	{
		Asset_Status::Asset_Status($owner);
		$this->status_tag = SQ_STATUS_EDITING;

	}//end constructor


	/**
	* Returns a string used to describe this status
	*
	* @return string
	* @access public
	* @static
	*/
	public static function getDescription()
	{
		return translate('Safe Editing');


	}//end getDescription()


	/**
	* Returns an array of statii that the current user can set for this asset
	* Value returned is a key => value array for a drop down
	*
	* @return array
	* @access public
	*/
	function getAvailableStatii()
	{
		$statii = Array();

		// cant do anything without write access
		if (!$this->owner->writeAccess('')) return $statii;

		// any editor can cancel the safe edit
		$statii[SQ_STATUS_LIVE] = translate('Cancel Safe Edit');


		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because EDITING does not require workflow
			trigger_error('FIXME: Asset #'.$this->owner->id.' is not in workflow status but running workflow is found');
			log_dump(get_backtrace());
		} else {
			// workflow is not currently running for this asset
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			//$m_allow = $mm->allowsMetadata($this->owner->id);
			$m_complete = $mm->requiredFieldsComplete($this->owner->id);
			$schemas = $wfm->getSchemas($this->owner->id, true);
			if (empty($schemas) && $m_complete) {
				// no workflow schemas applied, so any admin can make this live
				if ($this->owner->adminAccess('')) {
					$statii[SQ_STATUS_EDITING_APPROVED] = translate('Approve');

				}
			} else if ($m_complete) {

				$needs_workflow = FALSE;
				$wf_complete    = FALSE;
				// we have to test publish to determine if we should include "approve" or "apply for approval" status
				// this gets complicated when we have to check who can choose stream setting and decide first which streams of schema to test with
				foreach ($schemas as $schemaid) {
					$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					$who_can_choose_stream = $schema->attr('who_can_choose_stream');
					$streams = $wfm->getStreams($schemaid);

					// should we just test publish on default stream or having to test all streams?
					$just_check_default = FALSE;
					if($who_can_choose_stream == 'admin_only' && !$this->owner->adminAccess('')) {
						$just_check_default = TRUE;
					}
					else if($who_can_choose_stream == 'admin_write' && !$this->owner->writeAccess('')) {
						$just_check_default = TRUE;
					}

					if($just_check_default) {
						// it's simple because only default stream is used
						$publish_result = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), $streams[$wfm->getDefaultStream($schemaid)]);
						if ($publish_result === TRUE) {
							$wf_complete = TRUE;
						} else {
							$needs_workflow = TRUE;
						}
					}
					else {
						$valid_streams = $wfm->getValidStreams($schemaid, $this->owner->id);
						foreach ($streams as $streamid => $stream_name) {
							// if this stream has selection rules set and not match, we shouldn't test publish on this stream
							if(!in_array($streamid, $valid_streams)) continue;
							$publish_result = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), $stream_name);
							if ($publish_result === TRUE) {
								$wf_complete = TRUE;
							} else {
								$needs_workflow = TRUE;
							}
						}
					}
				}

				$parent_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->owner->id, SQ_SC_LINK_ALL, '', TRUE, 'minor', NULL, TRUE, TRUE);
				if(!empty($parent_asset_link)) {
					$parent_assetid = $parent_asset_link[0]['majorid'];
					$parent_typecode = $parent_asset_link[0]['major_type_code'];
					$parent_status = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($parent_assetid, $parent_typecode,TRUE, 'status');
					if ($parent_status[$parent_assetid] === $this->owner->status) {
						$needs_workflow = FALSE;	
					} 
				}
				// We could have BOTH Approve and Apply for Approval,
				// if some streams can be approved straight off and
				// some must go through workflow
				if ($wf_complete === TRUE) {
					$statii[SQ_STATUS_EDITING_APPROVED] = translate('Approve');
				}
				if ($needs_workflow === TRUE) {
					// if it's dependant asset, we have to provide both possibilities, because we don't know the situation of its parent
					$statii[SQ_STATUS_EDITING_APPROVAL] = translate('Apply for Approval');

				}
			}
		}//end if workflow running

		return $statii;

	}//end getAvailableStatii()


	/**
	* Get a message about status change options
	*
	* @return string
	* @access public
	*/
	function getStatusChangeMessage()
	{
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		if ($mm->allowsMetadata($this->owner->id) && !$mm->requiredFieldsComplete($this->owner->id)) {
			return sprintf(translate('This asset cannot be approved to go live because some mandatory %s fields are empty'), '<a href="'.htmlspecialchars(replace_query_string_vars(Array('asset_ei_screen' => 'metadata'))).'">'.strtolower(translate('Metadata')).'</a>');


		}
		return '';

	}//end getStatusChangeMessage()


	/**
	* Processes the changes required when a new status is set
	* Does not change the status of the owner asset or this object
	*
	* @param int         $new_status        the new status to set for this asset
	* @param boolean     $update_parent     Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parent=true)
	{
		if (!parent::processStatusChange($new_status, $update_parent)) {
			return false;
		}

		switch ($new_status) {

			case SQ_STATUS_LIVE :
				// cancelling a safe edit
				if (!$this->owner->revertToSystemVersion()) {
					return false;
				}

				// send an internal message
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$msg_reps = Array(
									'user_name'		=> $user->name,
									'workflow_user' => $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'status'		=> get_status_description($new_status),
								);
					$log->replacements = $msg_reps;
					$log->type = 'asset.workflow.log.safeedit.cancel';
					$log->subject = translate('Asset Safe Edit Cancelled');
					$log->body    = translate('User "%workflow_user%" has cancelled the safe edit of %type_code% "%asset_name%". "%asset_name%" has reverted to the previously saved version and the status of "%asset_name%" is now %status%');
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$log->send();
				}

				return true;

			break;

			case SQ_STATUS_EDITING_APPROVAL :
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

				// we are going to make this asset live by starting the workflow process
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

				if (!$wfm->startWorkflow($this->owner->id)) {
					trigger_localised_error('SYS0262', sprintf(translate('Workflow failed to start for "%s"'), $this->owner->name), E_USER_WARNING);
					return false;
				}

				// send an internal message
				$ms   = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				$msg_reps   = Array();

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$workflow_backend_href = $this->owner->getBackendHref('workflow', FALSE);
					$accept_url = $workflow_backend_href.'&asset_version='.$this->owner->version.'&workflow_link_action=approve';
					$reject_url = $workflow_backend_href.'&asset_version='.$this->owner->version.'&workflow_link_action=reject';

					$msg_reps = Array(
									'workflow_user' => $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'assetid'		=> $this->owner->id,
									'workflow_url'  => current_url().$workflow_backend_href,
									'accept_url'    => current_url().$accept_url,
									'reject_url'    => current_url().$reject_url,
									'asset_url'	    => $this->owner->getDependantParentsURL(),
									'asset_version' => $this->owner->version,
									'status'		=> $this->getDescription(),
								);
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$msg_reps['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$msg_reps['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}
					$log->replacements = $msg_reps;

					$log->type    = 'asset.workflow.log.started';
					$log->subject = translate('Workflow Started');
					$log->body    = translate('User "%workflow_user%" has requested %type_code% "%asset_name%" to go live from %status%.')."\n\n".
translate('A full report for the Workflow of "%asset_name%" can be found on the <SQ_MSG_LINK href="%workflow_url%">Workflow Screen</SQ_MSG_LINK>.')."\n".
translate('You can also <SQ_MSG_LINK href="%preview_url%">preview this asset.</SQ_MSG_LINK>');
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$ms->enqueueMessage($log);

					$running_schemas = $wfm->getSchemas($this->owner->id, TRUE, TRUE);
					foreach ($running_schemas as $schemaid) {
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						$workflow = $wfm->getSchemaWorkflows($this->owner->id, $schemaid);
						$step_data =& $wfm->getCurrentStep($workflow);

						if (!empty($step_data)) {
							$msg_reps['current_step_name'] = $step_data['step_name'];
							$msg_reps['current_step_id']   = implode('.', $workflow['current_step']);
						}

						$publishers = $wfm->whoCanPublish($this->owner->id, $schemaid);
						if (count($publishers) > 0) {
							$msg = $ms->newMessage();
							$msg->type = 'asset.workflow.announce.started';

							// see if we have the 'from' field set
							if ($schema->attr('schema_reply_to_email_address') != '') {
								$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
							} else {
								$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
							}
							if ($schema->attr('schema_from_email_address') != '') {
								$msg->from = $schema->attr('schema_from_email_address');
							}

							$msg->to = array_merge($msg->to, $publishers);

							if (trim($schema->attr('subject_started')) !== '' ) {
								$msg->subject = trim($schema->attr('subject_started'));
							} else {
								$msg->subject = translate('Workflow Started');
							}

							if (trim($schema->attr('message_started')) !== '' ) {
								$msg->body = trim($schema->attr('message_started'));
							} else {
                            	$msg->body    = translate('User "%workflow_user%" has requested %type_code% "%asset_name%" to go live from %status%.')."\n\n".
        										translate('A full report for the Workflow of "%asset_name%" can be found on the <SQ_MSG_LINK href="%workflow_url%">Workflow Screen</SQ_MSG_LINK>.')."\n".
        										translate('You can also <SQ_MSG_LINK href="%preview_url%">preview this asset.</SQ_MSG_LINK>');
							}
                            $msg->replacements = $msg_reps;
							$msg->parameters['assetid'] = $this->owner->id;
							$ms->enqueueMessage($msg);
						}
					}
				}

			break;

			case SQ_STATUS_EDITING_APPROVED :

				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				$ms   = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				// Save the comment for approval
				$log_message = $wfm->getEditorCommentOnApplyForApproval($this->owner->id);
				if (!empty($log_message)) {
					$msg_type = 'asset.workflow.userlog';
					$msg_reps = Array(
						'assetid'	=> $this->owner->id,
						'version'	=> $this->owner->version,
						'message'	=> $log_message,
					);
					$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
					$message->parameters['assetid'] = $msg_reps['assetid'];
					$message->parameters['version'] = substr($msg_reps['version'], 0, strrpos($msg_reps['version'], '.'));
					$message->subject = translate('Workflow User Log');
					$message->body    = $log_message;
					$message->send();
				}
				// send an internal message
				$msg_reps   = Array();
				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$workflow_backend_href = $this->owner->getBackendHref('workflow', FALSE);
					$accept_url = $workflow_backend_href.'&asset_version='.$this->owner->version.'&workflow_link_action=approve';
					$reject_url = $workflow_backend_href.'&asset_version='.$this->owner->version.'&workflow_link_action=reject';

					$msg_reps = Array(
									'workflow_user' => $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'assetid'		=> $this->owner->id,
									'workflow_url'  => current_url().$workflow_backend_href,
									'accept_url'    => current_url().$accept_url,
									'reject_url'    => current_url().$reject_url,
									'asset_url'	    => $this->owner->getDependantParentsURL(),
									'asset_version' => $this->owner->version,
									'status'		=> $this->getDescription(),
								);
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$msg_reps['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$msg_reps['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}
					$log->replacements = $msg_reps;

					$log->type    = 'asset.workflow.log.approve';
					$log->subject = translate('Asset Changes Approved');
					$log->body    = translate('User "%workflow_user%" has approved the changes to %type_code% "%asset_name%" to go live from %status%.')."\n\n".
translate('A full report for the Workflow of "%asset_name%" can be found on the <SQ_MSG_LINK href="%workflow_url%">Workflow Screen</SQ_MSG_LINK>.')."\n".
translate('You can also <SQ_MSG_LINK href="%preview_url%">preview this asset.</SQ_MSG_LINK>');
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$ms->enqueueMessage($log);

				}

				// we are going approve this asset instead of initiating the workflow
				$wfm->setCurrentUserAsLastStarted($current_userid, $this->owner->id);

			break;
		}

		return true;

	}//end processStatusChange()


}//end class

?>
