<?php
require_once SQ_INCLUDE_PATH.'/asset_status/asset_status_pending_approval.inc';

/**
* Asset_Status_Editing_Approval
*
* Purpose
*    Provide a Safe Editing Approval status for an asset.
*
* Valid Transitions
*    EDITING_APPROVAL -> EDITING_APPROVAL if there is workflow running and current user can approve
*                                         but is not the last publisher
*    EDITING_APPROVAL -> EDITING_APPROVED if there is a workflow and current user can approve
*                                         and is the last publisher
*    EDITING_APPROVAL -> EDITING          if there is workflow running and the current user can approve
*                                         but chooses to reject the changes OR
*                                         there is workflow running and the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status_Editing_Approval extends Asset_Status_Pending_Approval
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing_Approval(&$owner)
	{
		$this->Asset_Status_Pending_Approval($owner);

	}//end constructor


	/**
	* Returns a string used to describe this status
	*
	* @return string
	* @access public
	*/
	function getDescription()
	{
		return 'Safe Editing Pending Approval';

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
		$return_statii = Array();
		$statii = parent::getAvailableStatii();

		// change some of the status codes to relect that we are safe editing
		foreach ($statii as $code => $name) {
			if ($code == SQ_STATUS_APPROVED) $return_statii[SQ_STATUS_EDITING_APPROVED] = $name;
			else if ($code == SQ_STATUS_PENDING_APPROVAL) $return_statii[SQ_STATUS_EDITING_APPROVAL] = $name;
			else if ($code == SQ_STATUS_UNDER_CONSTRUCTION) $return_statii[SQ_STATUS_EDITING] = $name;
			else $return_statii[$code] = $name;
		}

		return $return_statii;

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

		if ($new_status == SQ_STATUS_UNDER_CONSTRUCTION) {
			// someone has rejected the changes, so we need to cancel the workflow
			if (!$wf->cancel()) {
				trigger_error('Workflow cancel failed', E_USER_WARNING);
				return false;
			} else {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				$body = 'User "'.$user->name.'" has rejected the changes made to '.$this->owner->type().' "'.$this->owner->name.'". The status of "'.$this->owner->name.'" has reverted to '.getStatusDescription($new_status);
				
				$ms->sendMessage($this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false), 0, 'Asset Changes Rejected', $body);
			}

		} else if ($wf->state == 'running') {
			
			// we are doing a workflow and someone has just approved the asset, so record this
			if (!$wf->recordPublish($current_userid)) {
				trigger_error('Approval by user '.$current_userid.' not set in processStatusChange', E_USER_WARNING);
				return false;
			}

			// send an internal message
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
			$body = 'User "'.$user->name.'" has approved '.$this->owner->type().' "'.$this->owner->name.'" to go live.';
			$ms->sendMessage($this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false), 0, 'Asset Changes Approved', $body);

			if ($new_status == SQ_STATUS_APPROVED) {
				// send another internal message
				$body = 'The workflow process for '.$this->owner->type().' "'.$this->owner->name.'" has been completed. "'.$this->owner->name.'" is now approved and can be made live by an administrator.';
				$ms->sendMessage($this->owner->getPermission(SQ_PERMISSION_ADMIN, true, false), 0, 'Workflow Completed', $body);
			}
		}

		// save the workflow
		$wf->save();

		return true;

	}//end processStatusChange()

}//end class

?>