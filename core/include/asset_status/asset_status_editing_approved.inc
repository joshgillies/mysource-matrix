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
* $Id: asset_status_editing_approved.inc,v 1.22 2012/08/30 01:06:24 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status_approved.inc';

/**
* Asset_Status_Editing_Approved
*
* Purpose
*    Provide an Safe Editing Approved status for an asset.
*
* Valid Transitions
*    EDITING_APPROVED -> LIVE    if the user is an admin
*    EDITING_APPROVED -> EDITING if the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix
*/
class Asset_Status_Editing_Approved extends Asset_Status_Approved
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing_Approved($owner)
	{
		$this->Asset_Status_Approved($owner);
		$this->status_tag = SQ_STATUS_EDITING_APPROVED;

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
		return translate('Safe Edit Approved To Go Live');


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
			if ($code == SQ_STATUS_UNDER_CONSTRUCTION) {
				$return_statii[SQ_STATUS_EDITING] = $name;
			} else {
				$return_statii[$code] = $name;
			}
		}

		return $return_statii;

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

		if ($new_status == SQ_STATUS_LIVE) {
			// safe edit is finished, so we dont need the system version any more
			if (!$this->owner->clearSystemVersion()) return false;
		}

		// Log the workflow messsage for rejecting the changes and putting the asset back into safe edit
		if ($new_status == SQ_STATUS_EDITING) {
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
			$ms   = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			// Save the comment for approval
			$log_message = $wfm->getEditorCommentOnApplyForApproval($this->owner->id);
			if (!empty($log_message)) {
				$msg_type = 'asset.workflow.userlog';
				$msg_reps = Array(
					'assetid'	=> $this->owner->id,
					'version'	=> $this->owner->version,
					'message'	=> $log_message,
				);
				$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
				$message->parameters['assetid'] = $msg_reps['assetid'];
				$message->parameters['version'] = substr($msg_reps['version'], 0, strrpos($msg_reps['version'], '.'));
				$message->subject = translate('Workflow User Log');
				$message->body    = $log_message;
				$message->send();
			}
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
				$asset_edt_fns = $this->owner->getEditFns();
				if (isset($asset_edt_fns->static_screens['preview'])) {
					$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
				} else {
					$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
				}
				$log->subject = translate('Asset Changes Rejected');
                $log->body    = translate('User "%workflow_user%" has rejected the changes made to %type_code% "%asset_name%". The status of "%asset_name%" has reverted to %status%.')."\n\n".
                                    translate('A full report for the Workflow of "%asset_name%" can be found on the <SQ_MSG_LINK href="%workflow_url%">Workflow Screen</SQ_MSG_LINK>.')."\n".
                                    translate('You can also <SQ_MSG_LINK href="%preview_url%">preview this asset.</SQ_MSG_LINK>');
                $log->type    = 'asset.workflow.log.reject';
				$log->parameters['assetid'] = $this->owner->id;
				$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
				$log->send();
			}
		}
		return true;

	}//end processStatusChange()


}//end class

?>
