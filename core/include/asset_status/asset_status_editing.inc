<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_status_editing.inc,v 1.11 2003/10/24 02:44:53 mmcintyre Exp $
* $Name: not supported by cvs2svn $
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
*                                the workflow
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Status_Editing extends Asset_Status
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing(&$owner)
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
	function getDescription()
	{
		return 'Safe Editing';

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
		$statii[SQ_STATUS_LIVE] = 'Cancel Safe Edit';

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because EDITING does not require workflow
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$schemas = $wfm->getSchemas($this->owner->id, true);
			if (empty($schemas)) {
				// no workflow defined, so any admin can make this live
				if ($this->owner->adminAccess('')) {
					$statii[SQ_STATUS_EDITING_APPROVED] = 'Approve';
				}
			} else {
				// so we have workflows, but none of them are running
				// so lets see what will happen if this user approves
				// to determine if they can live edit
				$wf_complete = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId());
				if ($wf_complete) $statii[SQ_STATUS_EDITING_APPROVED] = 'Approve';
				else $statii[SQ_STATUS_EDITING_APPROVAL] = 'Make Live';
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
			case SQ_STATUS_LIVE :
				// cancelling a safe edit
				if (!$this->owner->revertToSystemVersion()) return false;
				return true;
			break;

			case SQ_STATUS_EDITING_APPROVAL :
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
				
				if (!$wfm->silentWorkflowParty($this->owner->id)) { 
					$log = $ms->newMessage();
					$log->subject = 'Workflow Started';
					$log->body    = 'User "'.$user->name.'" has requested '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'" to go live from '.$this->getDescription().'.';
					// create a link for navigation to the workflow screen for this asset
					$log->body .= "\n\n".'A full report for the Workflow of "'.$this->owner->name.'" can be found on the <a href="#" onClick="self.parent.location=\''.$this->owner->getBackendHref('workflow').'\'; return false;">Workflow Screen.</a>';
					$log->type    = 'asset.workflow';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$log->send();
				}

			break;
		}

		return true;

	}//end processStatusChange()

}//end class

?>