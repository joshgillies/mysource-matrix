<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: asset_status_pending_approval.inc,v 1.24 2004/10/06 01:00:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
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

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
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
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				if ($mm->allowsMetadata($this->owner->id)) {
					$m_complete = $mm->requiredFieldsComplete($this->owner->id);
	
					if ($wf_complete && $m_complete) $statii[SQ_STATUS_APPROVED] = 'Approve';
					else if (in_array($userid, $publishers)) $statii[SQ_STATUS_PENDING_APPROVAL] = 'Approve Changes';
				} else {
					// this asset needs to allow both statuses because it cant check metadata requirements
					// due to it not allowing metadata
					$statii[SQ_STATUS_APPROVED] = 'Approve';
					$statii[SQ_STATUS_PENDING_APPROVAL] = 'Approve Changes';
				}

				$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Reject Changes';
			} else if ($admin_access) {
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
		
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		if ($new_status & SQ_SC_STATUS_CAN_APPROVE) {
			// someone has rejected the changes, so we need to cancel the workflow
			if (!$wfm->cancelWorkflow($this->owner->id)) {
				trigger_error('Workflow cancel failed', E_USER_WARNING);
				return false;
			} else {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

				// log an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				if (!$wfm->silentWorkflowParty($this->owner->id)) { 
					$log = $ms->newMessage();
					$log->subject = 'Asset Changes Rejected';
					$log->body    = 'User "'.$user->name.'" has rejected the changes made to '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'". The status of "'.$this->owner->name.'" has reverted to '.get_status_description($new_status);
					// create a link for navigation to the workflow screen for this asset
					$log->body .= "\n\n".'A full report for the Workflow of "'.$this->owner->name.'" can be found on the <SQ_MSG_LINK href="'.current_url().$this->owner->getBackendHref('workflow').'">Workflow Screen</SQ_MSG_LINK>.';
					$log->type    = 'asset.workflow';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$log->send();
				}
			}

		} else {

			$running_schemas = $wfm->getSchemas($this->owner->id, true, true);
			if (!empty($running_schemas)) {

				// if we are trying to approve this asset, we might not actually be a person
				// in the workflow process - sounds weird (I know) but this could be a plain old
				// admin that is trying to approve because metadata is now complete.
				// the result is that this workflow process may already be complete
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$m_complete = !$mm->allowsMetadata($this->owner->id) || $mm->requiredFieldsComplete($this->owner->id);

				if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_SC_STATUS_ALL_APPROVED)) {
					if (!$wfm->completeWorkflow($this->owner->id)) {
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
					if ($wfm->isWorkflowComplete($this->owner->id) && $m_complete && ($new_status & SQ_SC_STATUS_ALL_APPROVED)) {
						if (!$wfm->completeWorkflow($this->owner->id)) {
							trigger_error('Workflow completion failed', E_USER_WARNING);
							return false;
						}
					}
				}

				if ($new_status == SQ_STATUS_APPROVED) {

					$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
					$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
					if (!$wfm->silentWorkflowParty($this->owner->id)) { 
						$log = $ms->newMessage();
						$log->subject = 'Workflow Completed';
						$log->body    = 'The workflow process for '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'" has been completed. "'.$this->owner->name.'" is now approved and can be made live by an administrator.';
						$log->body .= "\n\n".'A full report for the Workflow of "'.$this->owner->name.'" can be found on the <SQ_MSG_LINK href="'.current_url().$this->owner->getBackendHref('workflow').'">Workflow Screen</SQ_MSG_LINK>.';
						$asset_url = $this->owner->getURL();
						if (!empty($asset_url)) {
							$log->body .= "\n".'You can also <SQ_MSG_LINK href="'.$asset_url.'">preview this asset.</SQ_MSG_LINK>';
						}
						$log->type    = 'asset.workflow';
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
