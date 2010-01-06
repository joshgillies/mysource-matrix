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
* $Id: asset_status_bundle_under_construction.inc,v 1.1 2010/01/06 03:15:59 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Bundle_Under_Construction
*
* Purpose
*    Custom "Editing" status for a Workflow Bundle.
*    Equates to Under Construction (and in some cases Safe Editing) for a normal asset.
*
* Valid Transitions
*    UNDER_CONSTRUCTION -> PENDING_APROVAL (Apply for Approval: "Editing" => "Approving")
*      If at least one bundled asset has workflow applied, and it cannot
*      be completed in one go
*    UNDER_CONSTRUCTION -> APPROVED        (Approve: "Editing" => "Approved")
*      If at least one bundled asset has workflow applied, and it CAN be
*      completed in one go.
*    UNDER_CONSTRUCTION -> LIVE            (Approve and Make Live: "Editing" => "Live")
*      If all bundled assets have no workflow applied.
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Asset_Status_Bundle_Under_Construction extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Bundle_Under_Construction($owner)
	{
		assert_is_a($owner, 'Workflow_Bundle');
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
		return translate('core_workflow_bundle_editing');

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

		// cant do anything without write access
		if (!$this->owner->writeAccess('')) {
			return $available_statuses;
		}

		// Status changes available:
		// - Live:      when all assets have no workflow
		// - Approving: when at least some assets have workflow
		//              (assets without workflow will go live)
		// - Approved:  when at least some assets have workflow,
		//              but would be completed straight away
		//              (assets without workflow will go live)
		//
		// With multiple streams, "would be completed straight away"
		// involves a test for all streams if the user has Admin
		// permission to the bundled asset.
		$has_workflow   = FALSE;
		$needs_workflow = FALSE;

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$mm  = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$our_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->owner->id, SQ_LINK_NOTICE);

		foreach ($our_asset_links as $our_asset_link) {
			// If an asset is in Safe Editing mode, then we must treat it as a
			// special case since it cannot go straight to Live - only to Safe
			// Editing Approved (since Live cancels it). So treat it as if it
			// had a completable workflow (ie. provide "Approve" link, not "Make
			// Live").
			if ($has_workflow === FALSE) {
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($our_asset_link['minorid']);
				$status = (int)$asset_info[$our_asset_link['minorid']]['status'];

				if ($status === SQ_STATUS_EDITING) {
					$has_workflow = TRUE;
				}
			}

			$schemas = $wfm->getSchemas($our_asset_link['minorid']);
			if (count($schemas) > 0) {
				$has_workflow = TRUE;

				// We need to get the asset, since we need to check for
				// admin permission so we can work out what to test the
				// schema with
				$bundled_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($our_asset_link['minorid']);
				if ($bundled_asset->adminAccess() === TRUE) {
					// Test with ALL streams
					$test_with = NULL;
				} else {
					// Test with default stream
					$test_with = '';
				}

				if ($wfm->testPublish($bundled_asset->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), $test_with) === FALSE) {
					$needs_workflow = TRUE;
				} else if ($mm->requiredFieldsComplete($bundled_asset->id) === FALSE) {
					// If we have incomplete metadata, we cannot go straight to
					// Approved for this asset.
					$needs_workflow = TRUE;
				}
			}

			if (($has_workflow === TRUE) && ($needs_workflow === TRUE)) {
				// No need to continue, we have all we need
				break;
			}
		}

		if ($has_workflow === TRUE) {
			if ($needs_workflow === TRUE) {
				$available_statuses[SQ_STATUS_PENDING_APPROVAL] = translate('status_change_apply_for_approval');
			} else {
				$available_statuses[SQ_STATUS_APPROVED] = translate('status_change_approve');
			}
		} else {
			$available_statuses[SQ_STATUS_LIVE] = translate('status_change_approve_and_make_live');
		}

		return $available_statuses;

	}//end getAvailableStatii()


	/**
	* Get a message about status change options
	*
	* @return string
	* @access public
	*/
	function getStatusChangeMessage()
	{
		// TODO: Incomplete metadata message.
		return '';

	}//end getStatusChangeMessage()


}//end class

?>