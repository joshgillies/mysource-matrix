<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_status_under_construction.inc,v 1.11 2003/10/10 06:49:25 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

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
*                                           the workflow and all required metadata is complete
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
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
	* @static
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
		if (!$this->owner->writeAccess('')) return $statii;

		// any admin can change the status to ARCHIVED
		if ($this->owner->adminAccess('')) {
			$statii[SQ_STATUS_ARCHIVED] = 'Archive';
		}

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because UNDER_CONSTRUCTION does not require workflow
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$schemas = $wfm->getSchemas($this->owner->id, true);
			if (empty($schemas)) {
				// no workflow defined, so any admin can make this live
				if ($this->owner->adminAccess('')) {
					$statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
				}
			} else {
				// so we have workflows, but none of them are running so lets see what will happen 
				// if this user approves to determine if they can live edit
				$wf_complete = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId());

				// we also need to check what if all the required metadata fields are completed
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$m_complete = $mm->requiredFieldsComplete($this->owner->id);

				if ($wf_complete && $m_complete) $statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
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
			$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();

			// we are going to make this asset live by starting the workflow process
			$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			if (!$wfm->startWorkflow($this->owner->id)) {
				trigger_error('Workflow failed to start for "'.$this->owner->name.'"', E_USER_WARNING);
				return false;
			}

			// if the user who wants this asset to go live can also
			// approve it as part of workflow, let their action to make this
			// asset live signal their intention to approve it as well
			$publishers = $wfm->whoCanPublish($this->owner->id);
			if (in_array($current_userid, $publishers)) {
				if (!$wfm->recordPublish($this->owner->id, $current_userid)) {
					trigger_error('Approval by user '.$current_userid.' not set in processStatusChange', E_USER_WARNING);
					// dont die here because they can try to approve again later
				}
			}

			// send an internal message
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

			$log = $ms->newMessage();
			$log->subject = 'Workflow Started';
			$log->body    = 'User "'.$user->name.'" has requested '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'" to go live from '.$this->getDescription().'.';
			$log->type    = 'asset.workflow';
			$log->parameters['assetid'] = $this->owner->id;
			$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
			$log->send();

		}//end if new status is PENDING_APPROVAL

		return true;

	}//end processStatusChange()

}//end class

?>