<?php
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
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status_Pending_Approval extends Asset_Status
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Pending_Approval(&$owner)
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
	function getDescription()
	{
		return 'Pending Approval';

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

		$wf = &$this->getWorkflow();

		if ($wf->state == 'running') {
			// check to see if the user logged in can publish
			$publishers = $wf->whoCanPublish();

			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
			if (in_array($userid, $publishers)) {
				// the current user can approve this asset to keep the workflow going
				$status = $wf->testPublish($GLOBALS['SQ_SYSTEM']->currentUserId());
				if ($status == 'complete') $statii[SQ_STATUS_APPROVED] = 'Approve';
				else $statii[SQ_STATUS_PENDING_APPROVAL] = 'Approve Changes';
				$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Reject Changes';
			} else if ($this->owner->adminAccess(Array(), false)) {
				$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Reject Changes';
			}
		} else {
			// workflow is not currently running for this asset
			// but it should be because we are waiting for it to complete
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
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
		
		$wf = &$this->getWorkflow();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		if ($new_status & SQ_SC_STATUS_CAN_APPROVE) {
			// someone has rejected the changes, so we need to cancel the workflow
			if (!$wf->cancel()) {
				trigger_error('Workflow cancel failed', E_USER_WARNING);
				return false;
			} else {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

				$message = $ms->newMessage();
				$message->subject = 'Asset Changes Rejected';
				$message->body = 'User "'.$user->name.'" has rejected the changes made to '.$this->owner->type().' "'.$this->owner->name.'". The status of "'.$this->owner->name.'" has reverted to '.getStatusDescription($new_status);
				$message->to = $this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false);
				$message->from = 0; // a system message
				$ms->enqueueMessage($message);

				$ms->closeQueue();
			}

		} else if ($wf->state == 'running') {
			
			// we are doing a workflow and someone has just approved the asset, so record this
			if (!$wf->recordPublish($current_userid)) {
				trigger_error('Approval by user '.$current_userid.' not set in processStatusChange', E_USER_WARNING);
				return false;
			}

			// send an internal message
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$ms->openQueue();
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
			
			$message = $ms->newMessage();
			$message->subject = 'Asset Changes Approved';
			$message->body = 'User "'.$user->name.'" has approved '.$this->owner->type().' "'.$this->owner->name.'" to go live.';
			$message->to = $this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false);
			$message->from = 0; // a system message
			$ms->enqueueMessage($message);

			if ($new_status == SQ_STATUS_APPROVED) {
				// send another internal message
				$message = $ms->newMessage();
				$message->subject = 'Workflow Completed';
				$message->body = 'The workflow process for '.$this->owner->type().' "'.$this->owner->name.'" has been completed. "'.$this->owner->name.'" is now approved and can be made live by an administrator.';
				$message->to = $this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false);
				$message->from = 0; // a system message
				$ms->enqueueMessage($message);
			}

			$ms->closeQueue();
		}

		// save the workflow
		$wf->save();

		return true;

	}//end processStatusChange()

}//end class
?>
