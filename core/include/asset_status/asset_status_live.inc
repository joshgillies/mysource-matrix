<?php
require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Live
*
* Purpose
*    Provide a Live status for an asset.
*
* Valid Transitions
*    LIVE -> EDITING            if the user has write access
*    LIVE -> LIVE_APPROVAL      if the user has write access
*    LIVE -> ARCHIVED           if the user is an admin
*    LIVE -> UNDER_CONSTRUCTION if the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status_Live extends Asset_Status
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Live(&$owner)
	{
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_LIVE;

	}//end constructor


	/**
	* Returns a string used to describe this status
	*
	* @return string
	* @access public
	*/
	function getDescription()
	{
		return 'Live';

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

		$wf = &$this->getWorkflow();

		if ($wf->state == 'running') {
			// workflow is currently running, but it should not be
			// because LIVE doesnt need workflow
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$statii[SQ_STATUS_LIVE_APPROVAL]          = 'Place Up For Review';
			$statii[SQ_STATUS_EDITING]                = 'Safe Edit';
			if ($this->owner->adminAccess(Array(), false)) {
				$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Place Under Construction';
				$statii[SQ_STATUS_ARCHIVED]           = 'Archive';
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

		switch ($new_status) {
			case SQ_STATUS_LIVE_APPROVAL :
				// up for review basically means that we need to start
				// the workflow process and get this asset re-approved
				$wf = &$this->getWorkflow();
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

				if (!$wf->start()) {
					trigger_error('Workflow failed to start for "'.$this->owner->name.'"', E_USER_WARNING);
					return false;
				}

				$wf->save();

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				
				$message = $ms->newMessage();
				$message->subject = 'Asset Up For Review';
				$message->body = 'User "'.$user->name.'" has placed '.$this->owner->type().' "'.$this->owner->name.'" up for review. The workflow process has been started.';
				$message->to = $this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false);
				$message->from = 0; // a system message
				$ms->enqueueMessage($message);

				$ms->closeQueue();
			break;

			case SQ_STATUS_EDITING :
				// we are safe editing
				if (!$this->owner->saveSystemVersion()) {
					trigger_error('Status change failed, could not save to cache', E_USER_WARNING);
					return false;
				}
			break;
		}

		return true;

	}//end processStatusChange()

}//end class

?>