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
* $Id: asset_status_live_approval.inc,v 1.55 2012/08/30 01:06:24 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Live_Approval
*
* Purpose
*    Provide an Up For Review status for an asset.
*
* Valid Transitions
*    LIVE_APPROVAL -> LIVE_APPROVAL if there is workflow running and current user can approve
*                                   but is not the last publisher
*    LIVE_APPROVAL -> LIVE          if there is a workflow and current user can approve
*                                   and is the last publisher
*    LIVE_APPROVAL -> EDITING       if there is workflow running and the current user can approve
*                                   but chooses to reject the changes OR
*                                   there is workflow running and the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.55 $
* @package MySource_Matrix
*/
class Asset_Status_Live_Approval extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Live_Approval($owner)
	{
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_LIVE_APPROVAL;

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
		return translate('Up For Review');


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

		if ($this->owner->adminAccess('')) {
			$statii[SQ_STATUS_EDITING] = translate('Safe Edit');

		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

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
				$is_dependant = $GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id);

				if ($mm->allowsMetadata($this->owner->id)) {
					$m_complete = $mm->requiredFieldsComplete($this->owner->id);
					if ($wf_complete && ((!$is_dependant && $m_complete) || $is_dependant)) {
						$statii[SQ_STATUS_LIVE] = translate('Approve and Make Live');

					} else if (in_array($userid, $publishers)) {
						$statii[SQ_STATUS_LIVE_APPROVAL] = translate('Approve Review');

					}
				} else {
					// this asset needs to allow both statuses because it cant check metadata requirements
					// due to it not allowing metadata
					$statii[SQ_STATUS_LIVE] = translate('Approve and Make Live');

					$statii[SQ_STATUS_LIVE_APPROVAL] = translate('Approve Review');

				}

				$statii[SQ_STATUS_EDITING] = translate('Safe Edit');

			}
		} else {
			// no running workflow means that anyone with admin access can
			// make this live (up for review is just a flag in this case)
			$statii[SQ_STATUS_LIVE] = translate('Approve and Make Live');

			if ($this->owner->adminAccess('')) {
				$statii[SQ_STATUS_ARCHIVED] = translate('Archive');

			}

		}//end if workflow running

		return $statii;

	}//end getAvailableStatii()


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

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		if ($new_status == SQ_STATUS_EDITING) {

			$running_schemas = $wfm->getSchemas($this->owner->id, true, true);
			if (!empty($running_schemas)) {
				// someone has rejected the approval, so we need to cancel the workflow
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());

				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$msg = $ms->newMessage();
				$msg_reps     = Array(
									'workflow_user'	=> $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'assetid'		=> $this->owner->id,
									'status'		=> get_status_description($new_status),
									'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
								);
				$msg->replacements = $msg_reps;
				$msg->type = 'asset.workflow.review.cancel';
				$asset_edt_fns = $this->owner->getEditFns();
				if (isset($asset_edt_fns->static_screens['preview'])) {
					$msg->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
				} else {
					$msg->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
				}

				if (!$wfm->cancelWorkflow($this->owner->id, $msg)) {
					trigger_localised_error('SYS0260', translate('Workflow cancel failed'), E_USER_WARNING)
					return false;
				}
			}

			// we are safe editing
			if (!$this->owner->saveSystemVersion()) {
				trigger_localised_error('SYS0208', translate('Status change failed, could not save to cache'), E_USER_WARNING)
				return false;
			}

			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			// send an internal message
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

			if (!$wfm->silentWorkflowParty($this->owner->id)) {
				$log = $ms->newMessage();
				$msg_reps = Array(
								'workflow_user'	=> $user->name,
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

				$log->type = 'asset.workflow.log.review.cancel';
				$log->parameters['assetid'] = $this->owner->id;
				$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
				$log->send();
			}

		} else {

			$running_schemas = $wfm->getSchemas($this->owner->id, true, true);
			if (!empty($running_schemas)) {

				// if we are trying to approve this asset, we might not actually be a person
				// in the workflow process - sounds weird (I know) but this could be a plain old
				// admin that is trying to approve because metadata is now complete.
				// the result is that this workflow process may already be complete
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$m_complete = !$mm->allowsMetadata($this->owner->id) || $mm->requiredFieldsComplete($this->owner->id) || $GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id);

				if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_STATUS_LIVE)) {
					if (!$wfm->completeWorkflow($this->owner->id)) {
						trigger_localised_error('SYS0261', translate('Workflow completion failed'), E_USER_WARNING)
						return false;
					}
				} else {
					// we are doing a workflow and someone has just approved the asset, so record this
					if (!$wfm->recordPublish($this->owner->id, $current_userid)) {
						trigger_localised_error('SYS0077', sprintf(translate('Approval by user %1$s not set in processStatusChange for "%2$s"'), $current_userid, $this->owner->name), E_USER_WARNING)
						return false;
					}

					// if the workflow process is now complete, we can go ahead and clear out the schemas
					// as long as all our metadata is also complete
					if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_STATUS_LIVE)) {
						if (!$wfm->completeWorkflow($this->owner->id)) {
							trigger_localised_error('SYS0261', translate('Workflow completion failed'), E_USER_WARNING)
							return false;
						}
					}
				}

				if ($new_status == SQ_STATUS_LIVE) {
					// notify people that the asset has gone live
					$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
					if (!$wfm->notifyOnLive($this->owner->id, $this->status_tag)) {
						return false;
					}

					// update the last published date
					if (!$this->owner->setDate('published', time())) {
						return false;
					}

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
						//$log->to = Array(0);
						$log->type = 'asset.workflow.log.review.complete';
						$asset_edt_fns = $this->owner->getEditFns();
						if (isset($asset_edt_fns->static_screens['preview'])) {
							$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
						} else {
							$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
						}

						$log->parameters['assetid'] = $this->owner->id;
						$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
						$ms->enqueueMessage($log);
					}
				}

			}//end if

		}//end else

		return true;

	}//end processStatusChange()


}//end class

?>
