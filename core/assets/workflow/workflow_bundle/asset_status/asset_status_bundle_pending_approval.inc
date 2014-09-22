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
* $Id: asset_status_bundle_pending_approval.inc,v 1.2 2012/08/30 01:09:17 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Bundle_Pending_Approval
*
* Purpose
*    Custom "Approving" status for a Workflow Bundle.
*    Equates to Pending Approval (or Safe Edit Pending Approval) for a normal asset.
*
* Valid Transitions
*    PENDING_APPROVAL -> UNDER_CONSTRUCTION (Reject Changes: "Approving" => "Editing")
*      Rejects changes and moves bundled assets (in workflow) to Under Construction
*      or Safe Editing where possible.
*    PENDING_APPROVAL -> PENDING_APPROVAL   (Approve Changes)
*      Progress the workflow for all assets in workflow.
*    PENDING_APPROVAL -> APPROVED           (Make Live: "Approving" => "Approved")
*      If all remaining assets in workflow can complete their workflow.
*
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Asset_Status_Bundle_Pending_Approval extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Bundle_Pending_Approval($owner)
	{
		assert_is_a($owner, 'Workflow_Bundle');
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
	public static function getDescription()
	{
		return translate('Approving');


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
		$available_statuses = Array();

		// The workflow can be advanced if at least one asset in the
		// bundle can have its changes approved
		$can_approve = FALSE;

		// The workflow can be completed only if all remaining assets
		// with workflow will be completed if its changes are approved
		$can_complete = TRUE;

		// The workflow can be rejected if:
		// - The user has *effective* Admin permission to the bundle
		// - The user has *effective* Admin permission to any asset in
		//   the bundle
		// - At least one asset in the bundle can have its changes
		//   approved (since ability to approve comes with the ability
		//   to reject)
		$can_reject = FALSE;

		// If we have admin access to the bundle,
		// we can always reject	changes
		if ($this->owner->adminAccess('') === TRUE) {
			$can_reject = TRUE;
		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$mm  = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$our_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->owner->id, SQ_LINK_NOTICE);
		foreach ($our_asset_links as $our_asset_link) {
			// Check for admin permission
			if ($can_reject === FALSE) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($our_asset_link['minorid']);
				if ($asset->adminAccess('') === TRUE) {
					$can_reject = TRUE;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			}

			if ($can_approve === FALSE) {
				$next_publishers = $wfm->whoCanPublish($our_asset_link['minorid']);
				if (in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $next_publishers) !== FALSE) {
					$can_approve = TRUE;
					$can_reject = TRUE;
				}
			}

			if ($can_complete === TRUE) {
				if ($wfm->testPublish($our_asset_link['minorid'], $GLOBALS['SQ_SYSTEM']->currentUserId()) === FALSE) {
					$can_complete = FALSE;
				}
			}
		}

		// Either the workflow can be completed, or it can't but workflow
		// can progress, or we can't progress workflow at all
		if ($can_complete === TRUE) {
			$available_statuses[SQ_STATUS_APPROVED] = translate('Approve');

		} else if ($can_approve === TRUE) {
			$available_statuses[SQ_STATUS_PENDING_APPROVAL] = translate('Approve Changes');

		}

		// But may be allowed to reject regardless
		if ($can_reject === TRUE) {
			$available_statuses[SQ_STATUS_UNDER_CONSTRUCTION] = translate('Reject Changes');

		}

		return $available_statuses;

	}//end getAvailableStatii()


}//end class

?>