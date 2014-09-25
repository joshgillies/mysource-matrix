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
* $Id: asset_status_pending_approval.inc,v 1.52 2013/02/15 07:16:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Pending_Approval
*
* Purpose
*    Provide a Pending Approval status for an asset.
*
* Valid Transitions
*    PENDING_APPROVAL -> PENDING_APPROVAL   if there is workflow running and current user can approve
*                                           but is not the last publisher
*    PENDING_APPROVAL -> APPROVED           if there is a workflow and current user can approve
*                                           and is the last publisher
*    PENDING_APPROVAL -> UNDER_CONSTRUCTION if there is workflow running and the current user can approve
*                                           but chooses to reject the changes OR
*                                           there is workflow running and the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.52 $
* @package MySource_Matrix
*/
class Asset_Status_Pending_Approval extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Pending_Approval($owner)
	{
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_PENDING_APPROVAL;

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
		return translate('Pending Approval');


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
		if (!$this->owner->readAccess()) return $statii;

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, TRUE, TRUE);

		if (!empty($running_schemas)) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			// check to see if the user logged in can publish
			$publishers = $wfm->whoCanPublish($this->owner->id);

			// if nobody can publish we better let admins do it
			$admin_access = $this->owner->adminAccess('');

			if (in_array($userid, $publishers) || $admin_access) {
				// the current user can approve this asset to keep the workflow going
				$wf_complete = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId());

				// we also need to check what if all the required metadata fields are completed
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				if ($mm->allowsMetadata($this->owner->id)) {
					$m_complete = $mm->requiredFieldsComplete($this->owner->id);
					$is_dependant =  $GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id);

					if ($wf_complete && (($m_complete && !$is_dependant) || $is_dependant)) {
						$statii[SQ_STATUS_APPROVED] = translate('Approve');

					} else if (in_array($userid, $publishers)) {
						$statii[SQ_STATUS_PENDING_APPROVAL] = translate('Approve Changes');

					}
				} else {
					// this asset needs to allow both statuses because it cant check metadata requirements
					// due to it not allowing metadata, but only if workflow is complete
					if ($wf_complete) {
						$statii[SQ_STATUS_APPROVED] = translate('Approve');

						$statii[SQ_STATUS_PENDING_APPROVAL] = translate('Approve Changes');

					}
				}

				$statii[SQ_STATUS_UNDER_CONSTRUCTION] = translate('Reject Changes');

			}
		} else {
			// workflow is not currently running for this asset
			// but it should be because we are waiting for it to complete
			trigger_error('FIXME: Asset #'.$this->owner->id.' is in workflow status but running workflow is not found');
			log_dump(get_backtrace());
		}//end if workflow running

		return $statii;

	}//end getAvailableStatii()


	/**
	* Processes the changes required when a new status is set
	* Does not change the status of the owner asset or this object
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parent	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parent=TRUE)
	{
		if (!parent::processStatusChange($new_status, $update_parent)) {
			return FALSE;
		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		if ($new_status & SQ_SC_STATUS_CAN_APPROVE) {
			// someone has rejected the changes, so we need to cancel the workflow
			if (!$wfm->cancelWorkflow($this->owner->id)) {
				trigger_localised_error('SYS0260', translate('Workflow cancel failed'), E_USER_WARNING);
				return FALSE;
			} else {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

				// log an internal message
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$msg_reps = Array(
									'user_name'		=> $user->name,
									'workflow_user' => $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'status'		=> get_status_description($new_status),
									'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
								);
					$log->replacements = $msg_reps;
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}
					$log->subject = translate('Asset Changes Rejected');
                    $log->body    = translate('User "%workflow_user%" has rejected the changes made to %type_code% "%asset_name%". The status of "%asset_name%" has reverted to %status%.')."\n\n".
                                        translate('A full report for the Workflow of "%asset_name%" can be found on the <SQ_MSG_LINK href="%workflow_url%">Workflow Screen</SQ_MSG_LINK>.')."\n".
                                        translate('You can also <SQ_MSG_LINK href="%preview_url%">preview this asset.</SQ_MSG_LINK>');
                    $log->type    = 'asset.workflow.log.reject';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$log->send();
				}
			}

		} else {

			$running_schemas = $wfm->getSchemas($this->owner->id, TRUE, TRUE);
			if (!empty($running_schemas)) {

				// if we are trying to approve this asset, we might not actually be a person
				// in the workflow process - sounds weird (I know) but this could be a plain old
				// admin that is trying to approve because metadata is now complete.
				// the result is that this workflow process may already be complete
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$m_complete = !$mm->allowsMetadata($this->owner->id) || $mm->requiredFieldsComplete($this->owner->id) || $GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id);

				if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_SC_STATUS_ALL_APPROVED)) {
					if (!$wfm->completeWorkflow($this->owner->id)) {
						trigger_localised_error('SYS0261', translate('Workflow completion failed'), E_USER_WARNING);
						return FALSE;
					}
				} else {
					// we are doing a workflow and someone has just approved the asset, so record this
					if (!$wfm->recordPublish($this->owner->id, $current_userid)) {
						trigger_localised_error('SYS0077', sprintf(translate('Approval by user %1$s not set in processStatusChange for "%2$s"'), $current_userid,$this->owner->name), E_USER_WARNING);
						return FALSE;
					}

					// if the workflow process is now complete, we can go ahead and clear out the schemas
					// as long as all our metadata is also complete
					if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_SC_STATUS_ALL_APPROVED)) {
						if (!$wfm->completeWorkflow($this->owner->id)) {
							trigger_localised_error('SYS0261', translate('Workflow completion failed'), E_USER_WARNING);
							return FALSE;
						}
					}
				}

				// Since asset_status_editing_approval inherit from this object, it inherits this function, so when the asset go either from pending approval to approved, or editing approval to approved, lets send the asset workflow complete log message.
				if ($new_status == SQ_STATUS_APPROVED || $new_status == SQ_STATUS_EDITING_APPROVED) {

					$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
					if (!$wfm->silentWorkflowParty($this->owner->id)) {
						$log = $ms->newMessage();
						$msg_reps = Array(
										'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
										'asset_name'	=> $this->owner->name,
										'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
									);

						$log->replacements = $msg_reps;
						$asset_edt_fns = $this->owner->getEditFns();
						if (isset($asset_edt_fns->static_screens['preview'])) {
							$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
						} else {
							$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
						}

						$log->subject = translate('Workflow Completed');
                        $log->body    = translate('The workflow process for %type_code% "%asset_name%" has been completed. "%asset_name%" is now approved and can be made live by an administrator.')."\n\n".
                                            translate('A full report for the Workflow of "%asset_name%" can be found on the <SQ_MSG_LINK href="%workflow_url%">Workflow Screen</SQ_MSG_LINK>.')."\n".
                                            translate('You can also <SQ_MSG_LINK href="%preview_url%">preview this asset.</SQ_MSG_LINK>');
                        $log->type    = 'asset.workflow.log.complete';
						$log->parameters['assetid'] = $this->owner->id;
						$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
						$ms->enqueueMessage($log);
					}

				}

			}//end if

		}//end else

		return TRUE;

	}//end processStatusChange()


}//end class

?>
