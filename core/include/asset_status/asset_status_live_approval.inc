<?php
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
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status_Live_Approval extends Asset_Status
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Live_Approval(&$owner)
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
	function getDescription()
	{
		return 'Up For Review';

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

		if ($this->owner->adminAccess(Array(), false)) {
			$statii[SQ_STATUS_EDITING] = 'Safe Edit';
		}

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			// check to see if the user logged in can publish
			$publishers = $wfm->whoCanPublish($this->owner->id);

			// if nobody can publish we better let admins do it
			$admin_access = $this->owner->adminAccess(Array(), false);

			if (in_array($userid, $publishers) || $admin_access) {
				// the current user can approve this asset to keep the workflow going
				$wf_complete = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId());

				// we also need to check what if all the required metadata fields are completed
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$m_complete = $mm->requiredFieldsComplete($this->owner->id);

				if ($wf_complete && $m_complete) $statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
				else if (in_array($userid, $publishers)) $statii[SQ_STATUS_LIVE_APPROVAL] = 'Approve Review';

				$statii[SQ_STATUS_EDITING] = 'Safe Edit';
			}
		} else {
			// now running workflow means that anyone with admin access can
			// make this live (up for review is just a flag in this case)
			$statii[SQ_STATUS_LIVE] = 'Approve and Make Live';

		}//end if workflow running

		return $statii;

	}//end getAvailableStatii()


	/**
	* Processes the changes required when a new status is set
	* Does not change the status of the owner asset or this object
	*
	* @param int	$new_status	the new status to set for this asset
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status)
	{
		if (!parent::processStatusChange($new_status)) return false;
		
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		if ($new_status == SQ_STATUS_EDITING) {
			// someone has rejected the changes, so we need to cancel the workflow
			if (!$wfm->cancelWorkflow($this->owner->id)) {
				trigger_error('Workflow cancel failed', E_USER_WARNING);
				return false;
			} else {
				// we are safe editing
				if (!$this->owner->saveSystemVersion()) {
					trigger_error('Status change failed, could not save to cache', E_USER_WARNING);
					return false;
				}

				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

				$message = $ms->newMessage();
				$message->subject = 'Asset Review Cancelled';
				$message->body = 'User "'.$user->name.'" has started a safe edit of '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'" during a review. The review has been cancelled and the status of "'.$this->owner->name.'" is now '.getStatusDescription($new_status);
				$message->to   = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->owner->id, SQ_PERMISSION_ADMIN, true, false);
				$message->from = 0; // a system message
				$ms->enqueueMessage($message);

				$ms->closeQueue();
			}

		} else {
			
			$running_schemas = $wfm->getSchemas($this->owner->id, true, true);
			if (!empty($running_schemas)) {
			
				// if we are trying to approve this asset, we might not actually be a person
				// in the workflow process - sounds weird (I know) but this could be a plain old
				// admin that is trying to approve because metadata is now complete.
				// the result is that this workflow process may already be complete
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$m_complete = $mm->requiredFieldsComplete($this->owner->id);

				if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_STATUS_LIVE)) {
					if (!$wfm->cancelWorkflow($this->owner->id)) {
						trigger_error('Workflow completion failed', E_USER_WARNING);
						return false;
					}
				} else {
					// we are doing a workflow and someone has just approved the asset, so record this
					if (!$wfm->recordPublish($this->owner->id, $current_userid)) {
						trigger_error('Approval by user '.$current_userid.' not set in processStatusChange for "'.$this->owner->name.'"', E_USER_WARNING);
						return false;
					}

					// if the workflow process is now complete, we can go ahead and clear out the schemas
					// as long as all our metadata is also complete
					if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_STATUS_LIVE)) {
						if (!$wfm->cancelWorkflow($this->owner->id)) {
							trigger_error('Workflow completion failed', E_USER_WARNING);
							return false;
						}
					}
				}

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				
				$message = $ms->newMessage();
				$message->subject = 'Asset Review Approved';
				$message->body = 'User "'.$user->name.'" has approved the review on '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'".';
				$message->to   = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->owner->id, SQ_PERMISSION_ADMIN, true, false);
				$message->from = 0; // a system message
				$ms->enqueueMessage($message);

				if ($new_status == SQ_STATUS_APPROVED) {
					// send another internal message
					$message = $ms->newMessage();
					$message->subject = 'Review Completed';
					$message->body = 'The review process for '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'" has been completed. "'.$this->owner->name.'" is now live.';
					$message->to   = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->owner->id, SQ_PERMISSION_ADMIN, true, false);
					$message->from = 0; // a system message
					$ms->enqueueMessage($message);
				}

				$ms->closeQueue();

			}//end if

		}//end else

		return true;

	}//end processStatusChange()

}//end class

?>