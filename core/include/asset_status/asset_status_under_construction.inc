<?php
require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Under_Construction
*
* Purpose
*    Provide an Under Construction status for an asset.
*
* Valid Transitions
*    UNDER_CONSTRUCTION -> ARCHIVED         if user is an admin
*    UNDER_CONSTRUCTION -> PENDING_APROVAL  if there is a workflow and user has write access
*    UNDER_CONSTRUCTION -> LIVE             if no workflow and user is an admin OR
*                                           there is workflow but current user's approval will complete
*                                           the workflow
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status_Under_Construction extends Asset_Status
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Under_Construction(&$owner)
	{
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_UNDER_CONSTRUCTION;

	}//end constructor


	/**
	* Returns a string used to describe this status
	*
	* @return string
	* @access public
	*/
	function getDescription()
	{
		return 'Under Construction';

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
		if (!$this->owner->writeAccess(Array(), false)) return $statii;

		// any admin can change the status to ARCHIVED
		if ($this->owner->adminAccess(Array(), false)) {
			$statii[SQ_STATUS_ARCHIVED] = 'Archive';
		}

		$wf = &$this->getWorkflow();

		if ($wf->state == 'running') {
			// workflow is currently running, but it should not be
			// because UNDER_CONSTRUCTION does not require workflow
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			if (empty($wf->steps) && empty($wf->inherited_workflows)) {
				// no workflow defined, so any admin can make this live
				if ($this->owner->adminAccess(Array(), false)) {
					$statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
				}
			} else {
				// so we have workflows, but none of them are running
				// so lets see what will happen if this user approves
				// to determine if they can live edit
				$wf_status = $wf->testPublish($GLOBALS['SQ_SYSTEM']->currentUserId());
				if ($wf_status == 'complete') $statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
				else $statii[SQ_STATUS_PENDING_APPROVAL] = 'Make Live';
			}
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

		// so we know we can set the status but we need to
		// do some workflow stuff if we are changing to PENDING APPROVAL
		if ($new_status == SQ_STATUS_PENDING_APPROVAL) {
			$wf = &$this->getWorkflow();

			// we are going to make this asset live by starting the workflow process
			$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			if (!$wf->start()) {
				trigger_error('Workflow failed to start for "'.$this->owner->name.'"', E_USER_WARNING);
				return false;
			}

			// save the workflow and force a refresh to load it up again
			$wf->save();
			$wf = &$this->getWorkflow(true);

			// if the user who wants this asset to go live can also
			// approve it as part of workflow, let their action to make this
			// asset live signal their intention to approve it as well
			$publishers = $wf->whoCanPublish();
			if (in_array($current_userid, $publishers)) {
				if (!$wf->recordPublish($current_userid)) {
					trigger_error('Approval by user '.$current_userid.' not set in processStatusChange', E_USER_WARNING);
					// dont die here because they can try to approve again later
				} else {
					$wf->save();
				}
			}

			// send an internal message
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$ms->openQueue();
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

			$message = $ms->newMessage();
			$message->subject = 'Workflow Started';
			$message->body = 'User "'.$user->name.'" has requested '.$this->owner->type().' "'.$this->owner->name.'" to go live from '.$this->getDescription().'.';
			$message->to = $this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false);
			$message->from = 0; // a system message
			$ms->enqueueMessage($message);

			$ms->closeQueue();

		}//end if new status is PENDING_APPROVAL

		return true;

	}//end processStatusChange()

}//end class

?>