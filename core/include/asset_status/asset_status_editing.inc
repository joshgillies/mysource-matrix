<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: asset_status_editing.inc,v 1.42 2008/03/13 00:32:35 bshkara Exp $
*
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
*                                the workflow the workflow and all required metadata is complete
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.42 $
* @package MySource_Matrix
*/
class Asset_Status_Editing extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing($owner)
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
	public static function getDescription()
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
		$statii[SQ_STATUS_LIVE] = translate('status_change_cancel_safe_edit');

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because EDITING does not require workflow
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			if ($mm->allowsMetadata($this->owner->id) && !$GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id)) {
				if ($mm->requiredFieldsComplete($this->owner->id)) {
					// metadata is complete
					$schemas = $wfm->getSchemas($this->owner->id, true);
					if (empty($schemas)) {
						// no workflow schemas applied, so any admin can make this live
						if ($this->owner->adminAccess('')) {
							$statii[SQ_STATUS_EDITING_APPROVED] = translate('status_change_approve');
						}
					} else {
						// workflow applied but not running;
						// see whether the current user could publish if we started workflow
						if ($wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId())) {
							$statii[SQ_STATUS_EDITING_APPROVED] = translate('status_change_approve');
						} else {
							$statii[SQ_STATUS_EDITING_APPROVAL] = translate('status_change_apply_for_approval');
						}
					}
				} else {
					// can't grant approval yet because metadata incomplete
					$statii[SQ_STATUS_EDITING_APPROVAL] = translate('status_change_apply_for_approval');
				}

			} else {
				// this asset needs to allow both statuses because it cant check metadata requirements
				// due to it not allowing metadata
				$statii[SQ_STATUS_EDITING_APPROVED] = translate('status_change_approve');
				$statii[SQ_STATUS_EDITING_APPROVAL] = translate('status_change_apply_for_approval');
			}
		}//end if workflow running

		return $statii;

	}//end getAvailableStatii()


	/**
	* Get a message about status change options
	*
	* @return string
	* @access public
	*/
	function getStatusChangeMessage()
	{
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		if ($mm->allowsMetadata($this->owner->id) && !$mm->requiredFieldsComplete($this->owner->id)) {
			return translate('cannot_approve_metadata_incomplete', '<a href="'.replace_query_string_vars(Array('asset_ei_screen' => 'metadata')).'">'.strtolower(translate('metadata')).'</a>');
		}
		return '';

	}//end getStatusChangeMessage()


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

		switch ($new_status) {

			case SQ_STATUS_LIVE :
				// cancelling a safe edit
				if (!$this->owner->revertToSystemVersion()) {
					return false;
				}

				// send an internal message
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$msg_reps = Array(
									'user_name'		=> $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'status'		=> get_status_description($new_status),
								);
					$log->replacements = $msg_reps;
					$log->type = 'asset.workflow.log.safeedit.cancel';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$log->send();
				}

				return true;

			break;

			case SQ_STATUS_EDITING_APPROVAL :

				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

				// we are going to make this asset live by starting the workflow process
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

				if (!$wfm->startWorkflow($this->owner->id)) {
					trigger_localised_error('SYS0262', E_USER_WARNING, $this->owner->name);
					return false;
				}

				// send an internal message
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$msg_reps = Array(
									'user_name'		=> $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'status'		=> $this->getDescription(),
									'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
								);
					$log->replacements = $msg_reps;
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}
					$log->type    = 'asset.workflow.log.started';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$ms->enqueueMessage($log);
				}

			break;
		}

		return true;

	}//end processStatusChange()


}//end class

?>
