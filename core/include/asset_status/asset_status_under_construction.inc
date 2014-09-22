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
* $Id: asset_status_under_construction.inc,v 1.64 2013/09/25 07:42:08 cupreti Exp $
*
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
*    UNDER_CONSTRUCTION -> APPROVED         if there is a workflow and user is admin
*    UNDER_CONSTRUCTION -> LIVE             if no workflow and user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.64 $
* @package MySource_Matrix
*/
class Asset_Status_Under_Construction extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Under_Construction($owner)
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
	public static function getDescription()
	{
		return translate('Under Construction');


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
			$statii[SQ_STATUS_ARCHIVED] = translate('Archive');

		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because UNDER_CONSTRUCTION does not require workflow
			trigger_error('FIXME: Asset #'.$this->owner->id.' is not in workflow status but running workflow is found');
			log_dump(get_backtrace());
		} else {
			// workflow is not currently running for this asset
			$schemas = $wfm->getSchemas($this->owner->id, true);
			if (empty($schemas)) {
				// no workflow defined, so any admin can make this live
				if ($this->owner->adminAccess('')) {
					if (!$GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id)) {
						if (!isset($this->_tmp['metadata_complete'])) {
							$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
							$this->_tmp['metadata_complete'] = $mm->requiredFieldsComplete($this->owner->id);
						}
					} else {
							$this->_tmp['metadata_complete'] = TRUE;
					}
					if ($this->_tmp['metadata_complete']) {
						$statii[SQ_STATUS_LIVE] = translate('Approve and Make Live');

					}
				}
			} else {

				$needs_workflow = FALSE;
				$wf_complete    = FALSE;
				// we have to test publish to determine if we should include "approve" or "apply for approval" status
				// this gets complicated when we have to check who can choose stream setting and decide first which streams of schema to test with
				foreach ($schemas as $schemaid) {
					$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					$who_can_choose_stream = $schema->attr('who_can_choose_stream');
					$streams = $wfm->getStreams($schemaid);

					// should we just test publish on default stream or having to test all streams?
					$just_check_default = FALSE;
					if($who_can_choose_stream == 'admin_only' && !$this->owner->adminAccess('')) {
						$just_check_default = TRUE;
					}
					else if($who_can_choose_stream == 'admin_write' && !$this->owner->writeAccess('')) {
						$just_check_default = TRUE;
					}

					if($just_check_default) {
						// it's simple because only default stream is used
						$publish_result = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), $streams[$wfm->getDefaultStream($schemaid)]);
						if ($publish_result === TRUE) {
							$wf_complete = TRUE;
						} else {
							$needs_workflow = TRUE;
						}
					}
					else {
						$valid_streams = $wfm->getValidStreams($schemaid, $this->owner->id);
						foreach ($streams as $streamid => $stream_name) {
							// if this stream has selection rules set and not match, we shouldn't test publish on this stream
							if(!in_array($streamid, $valid_streams)) continue;
							$publish_result = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), $stream_name);
							if ($publish_result === TRUE) {
								$wf_complete = TRUE;
							} else {
								$needs_workflow = TRUE;
							}
						}
					}
				}


				// we also need to check what if all the required metadata fields are completed
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				if ($mm->allowsMetadata($this->owner->id)) {

					$m_complete = $mm->requiredFieldsComplete($this->owner->id);
					$is_dependant = $GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id);

					// In the case of multiple streams, we may need to provide
					// BOTH 'approve' and 'apply for approval' if some streams
					// can be approved bypassing workflow, and some can't.
					if ($wf_complete === TRUE && (($m_complete === TRUE && !$is_dependant) || $is_dependant)) {
						$statii[SQ_STATUS_APPROVED] = translate('Approve');

					}
					if (($needs_workflow === TRUE) || ($m_complete === FALSE && !$is_dependant)) {
						$statii[SQ_STATUS_PENDING_APPROVAL] = translate('Apply for Approval');

					}
				} else {
					// this asset needs to allow status "apply for approval" and "approve change" (if workflow is complete),
					// because asset does not allows metadata
					if ($wf_complete) {
						$statii[SQ_STATUS_APPROVED] = translate('Approve');

					}
					$statii[SQ_STATUS_PENDING_APPROVAL] = translate('Apply for Approval');

				}
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
		if (!isset($this->_tmp['metadata_complete'])) {
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$this->_tmp['metadata_complete'] = $mm->requiredFieldsComplete($this->owner->id);
		}
		if (!$this->_tmp['metadata_complete']) {
			$replace_string_vars    = Array (
										'asset_ei_screen'   => 'metadata',
										'am_section'        => 'edit_asset',
										'assetid'           => $this->owner->id,
									  );
			$metadata_screen_url = htmlspecialchars(replace_query_string_vars($replace_string_vars));
			return sprintf(translate('This asset cannot be made live because some mandatory %s fields are empty'), '<a href="'.$metadata_screen_url.'">'.strtolower(translate('Metadata')).'</a>');



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

		// so we know we can set the status but we need to
		// do some workflow stuff if we are changing to PENDING APPROVAL
		switch ($new_status) {
			case SQ_STATUS_PENDING_APPROVAL:
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

				// we are going to make this asset live by starting the workflow process
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

				if (!$wfm->startWorkflow($this->owner->id)) {
					trigger_localised_error('SYS0262', sprintf(translate('Workflow failed to start for "%s"'), $this->owner->name), E_USER_WARNING)
					return false;
				}

				// log a message to let people know that workflow has started
				$ms   = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				$msg_reps   = Array();

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$msg_reps     = Array(
										'user_name'		=> $user->name,
										'workflow_user' => $user->name,
										'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'),
										'asset_name'	=> $this->owner->name,
										'status'		=> $this->getDescription(),
										'workflow_url'	=> current_url().$this->owner->getBackendHref('workflow', FALSE),
									);
					$asset_edt_fns = $this->owner->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$msg_reps['preview_url'] = current_url().$this->owner->getBackendHref('preview', FALSE);
					} else {
						$msg_reps['preview_url'] = current_url().$this->owner->getBackendHref('details', FALSE);
					}
					$log->replacements = $msg_reps;

					$log->type = 'asset.workflow.log.started';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$ms->enqueueMessage($log);

					$running_schemas = $wfm->getSchemas($this->owner->id, TRUE, TRUE);
					foreach ($running_schemas as $schemaid) {
						$publishers = $wfm->whoCanPublish($this->owner->id, $schemaid);
						if (count($publishers) > 0) {
							$msg = $ms->newMessage();
							$msg->type = 'asset.workflow.announce.started';

							// see if we have the 'from' field set
							$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
							if ($schema->attr('schema_reply_to_email_address') != '') {
								$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
							} else {
								$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
							}
							if ($schema->attr('schema_from_email_address') != '') {
								$msg->from = $schema->attr('schema_from_email_address');
							}

							$msg->to = array_merge($msg->to, $publishers);
							$msg->replacements = $msg_reps;
							$msg->parameters['assetid'] = $this->owner->id;
							$ms->enqueueMessage($msg);
						}
					}
				}
			break;

			case SQ_STATUS_LIVE:
				// notify people that the asset has gone live
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				if (!$wfm->notifyOnLive($this->owner->id, $this->status_tag)) {
					return false;
				}

				// update the last published date
				if (!$this->owner->setDate('published', time())) {
					return false;
				}
			break;

			case SQ_STATUS_APPROVED :

				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

				// we are going approve this asset instead of initiating the workflow
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
				$wfm->setCurrentUserAsLastStarted($current_userid, $this->owner->id);

			break;
		}

		return true;

	}//end processStatusChange()


}//end class

?>
