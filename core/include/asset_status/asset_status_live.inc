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
* $Id: asset_status_live.inc,v 1.39.4.1 2011/08/19 02:03:28 mhaidar Exp $
*
*/

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
* @version $Revision: 1.39.4.1 $
* @package MySource_Matrix
*/
class Asset_Status_Live extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Live($owner)
	{
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_LIVE;

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
		return translate('status_live');

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

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because LIVE doesnt need workflow
			pre_echo('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$statii[SQ_STATUS_LIVE_APPROVAL]          = translate('status_change_place_up_for_review');
			$statii[SQ_STATUS_EDITING]                = translate('status_change_safe_edit');
			if ($this->owner->adminAccess('')) {
				$statii[SQ_STATUS_UNDER_CONSTRUCTION] = translate('status_change_place_under_construction');
				$statii[SQ_STATUS_ARCHIVED]           = translate('status_change_archive');
			}
		}//end if workflow running

		return $statii;

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

		switch ($new_status) {
			case SQ_STATUS_LIVE_APPROVAL :
				// up for review basically means that we need to start
				// the workflow process and get this asset re-approved
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

				// send an internal message
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);

				$schemas = $wfm->getSchemas($this->owner->id, true);
				if (!empty($schemas)) {
					$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
					$msg = $ms->newMessage();
					$msg->subject = 'Asset Up For Review';
					$msg->type = 'asset.workflow.review';

					$msg_reps = Array(
									'workflow_user'	=> $user->name,
                                    'assetid'       => $this->owner->id,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
								);
					$msg->replacements = $msg_reps;
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$msg->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$msg->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}

					if (!$wfm->startWorkflow($this->owner->id, false, $msg)) {
						trigger_localised_error('SYS0262', E_USER_WARNING, $this->owner->name);
						return false;
					}
				}

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$log->type = 'asset.workflow.log.review';
					$msg_reps = Array(
									'user_name'		=> $user->name,
									'workflow_user' => $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
									'asset_name'	=> $this->owner->name,
									'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
								);
					//$log->to = Array(0);
					$log->replacements = $msg_reps;
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$log->replacements['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$ms->enqueueMessage($log);
				}

			break;

			case SQ_STATUS_EDITING :
				// we are safe editing
				if (!$this->owner->saveSystemVersion()) {
					trigger_localised_error('SYS0208', E_USER_WARNING);
					return false;
				}
			break;
		}

		return true;

	}//end processStatusChange()


}//end class

?>
