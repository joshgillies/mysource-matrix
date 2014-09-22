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
* $Id: asset_status_bundle_under_construction.inc,v 1.3 2012/08/30 01:09:17 ewang Exp $
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
* @version $Revision: 1.3 $
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
		return translate('Editing');


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
		$can_complete   = FALSE;

		$m_complete = TRUE;

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$mm  = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$our_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->owner->id, SQ_LINK_NOTICE);

		foreach ($our_asset_links as $our_asset_link) {
			// If an asset is in Safe Editing mode, then we must treat it as a
			// special case since it cannot go straight to Live - only to Safe
			// Editing Approved (since Live cancels it). So treat it as if it
			// had a completable workflow (ie. provide "Approve" link, not "Make
			// Live").
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($our_asset_link['minorid']);

			if ($has_workflow === FALSE) {
				$status = (int)$asset_info[$our_asset_link['minorid']]['status'];
				if ($status === SQ_STATUS_EDITING) {
					$has_workflow = TRUE;
				} else {
					$schemas = $wfm->getSchemas($our_asset_link['minorid'], TRUE);
					if (count($schemas) > 0) {
						$has_workflow = TRUE;
					}
				}
			}

			// Check for complete metadata
			if ($m_complete === TRUE) {
				if ($mm->allowsMetadata($this->owner->id) && !$GLOBALS['SQ_SYSTEM']->am->isDependant($this->owner->id)) {
					$m_complete = $mm->requiredFieldsComplete($this->owner->id);
				}
			}

			// Both trigger conditions
			if (($has_workflow === TRUE) && ($m_complete === FALSE)) {
				break;
			}

		}//end foreach

		if ($has_workflow === TRUE) {
			if ($m_complete === FALSE) {
				$available_statuses[SQ_STATUS_PENDING_APPROVAL] = translate('Apply for Approval');

			} else {
				$all_streams        = $this->owner->getAllBundleStreams();
				$bypassable_streams = $this->owner->getAllBypassableBundleStreams();
				$default_bypassable = $this->owner->isDefaultStreamBypassable();

				$needs_approve = FALSE;
				$needs_apply   = FALSE;

				if ($default_bypassable === TRUE) {
					// Default stream is bypassable
					$needs_approve = TRUE;
				} else if (count($bypassable_streams) > 0) {
					// At least one bypassable stream
					$needs_approve = TRUE;
				}

				if (($default_bypassable === FALSE) || (count($bypassable_streams) < count($all_streams))) {
					// Default stream is not bypassable, or at least one un-bypassable stream
					$needs_apply = TRUE;
				}

				// If we have to apply for approval on at least one stream
				if ($needs_apply === TRUE) {
					$available_statuses[SQ_STATUS_PENDING_APPROVAL] = translate('Apply for Approval');

				}

				// If we can complete something through at least one stream
				if ($needs_approve === TRUE) {
					$available_statuses[SQ_STATUS_APPROVED] = translate('Approve');

				}
			}
		} else {
			$available_statuses[SQ_STATUS_LIVE] = translate('Approve and Make Live');

		}//end if has workflow

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