<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_status_live.inc,v 1.12 2003/11/11 04:40:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
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
	* @static
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
		if (!$this->owner->writeAccess('')) return $statii;

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because LIVE doesnt need workflow
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$statii[SQ_STATUS_LIVE_APPROVAL]          = 'Place Up For Review';
			$statii[SQ_STATUS_EDITING]                = 'Safe Edit';
			if ($this->owner->adminAccess('')) {
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
				$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

				if (!$wfm->startWorkflow($this->owner->id)) {
					trigger_error('Workflow failed to start for "'.$this->owner->name.'"', E_USER_WARNING);
					return false;
				}

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
				
				if (!$wfm->silentWorkflowParty($this->owner->id)) { 
					$log = $ms->newMessage();
					$log->subject = 'Asset Up For Review';
					$log->body    = 'User "'.$user->name.'" has placed '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name').' "'.$this->owner->name.'" up for review. The workflow process has been started.';
					// create a link for navigation to the workflow screen for this asset
					$log->body .= "\n\n".'<SQ_MSG_LINK href="'.current_url().$this->owner->getBackendHref('workflow').'">Click here for the Workflow Screen for this asset</SQ_MSG_LINK>.';
					$log->type    = 'asset.workflow';
					$log->parameters['assetid'] = $this->owner->id;
					$log->parameters['version'] = substr($this->owner->version, 0, strrpos($this->owner->version, '.'));
					$log->send();
				}
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