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
* $Id: asset_status_under_construction.inc,v 1.49 2008/02/18 04:10:10 lwright Exp $
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
* @version $Revision: 1.49 $
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
			$statii[SQ_STATUS_ARCHIVED] = translate('status_change_archive');
		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
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
					if (!$GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id)) {
						if (!isset($this->_tmp['metadata_complete'])) {
							$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
							$this->_tmp['metadata_complete'] = $mm->requiredFieldsComplete($this->owner->id);
						}
					} else {
							$this->_tmp['metadata_complete'] = TRUE;
					}
					if ($this->_tmp['metadata_complete']) {
						$statii[SQ_STATUS_LIVE] = translate('status_change_approve_and_make_live');
					}
				}
			} else {
				// so we have workflows, but none of them are running so lets see what will happen
				// if this user approves to determine if they can live edit
				$wf_complete = $wfm->testPublish($this->owner->id, $GLOBALS['SQ_SYSTEM']->currentUserId());

				// we also need to check what if all the required metadata fields are completed
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				if ($mm->allowsMetadata($this->owner->id) && !$GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id)) {
					$m_complete = $mm->requiredFieldsComplete($this->owner->id);

					if ($wf_complete && $m_complete) {
						$statii[SQ_STATUS_APPROVED] = translate('status_change_approve');
					} else {
						$statii[SQ_STATUS_PENDING_APPROVAL] = translate('status_change_apply_for_approval');
					}
				} else {
					// this asset needs to allow both statuses because it cant check metadata requirements
					// due to it not allowing metadata
					$statii[SQ_STATUS_APPROVED] = translate('status_change_approve');
					$statii[SQ_STATUS_PENDING_APPROVAL] = translate('status_change_apply_for_approval');
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
            $metadata_screen_url = replace_query_string_vars($replace_string_vars);
			return translate('cannot_make_live_metadata_incomplete', '<a href="'.$metadata_screen_url.'">'.strtolower(translate('metadata')).'</a>');

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
					trigger_localised_error('SYS0262', E_USER_WARNING, $this->owner->name);
					return false;
				}

				// log a message to let people know that workflow has started
				$ms   = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				$asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name');

				if (!$wfm->silentWorkflowParty($this->owner->id)) {
					$log = $ms->newMessage();
					$msg_reps     = Array(
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

					$log->type = 'asset.workflow.log.started';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$ms->enqueueMessage($log);
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
		}

		return true;

	}//end processStatusChange()


}//end class

?>
