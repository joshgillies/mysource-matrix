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
* $Id: asset_status_bundle_approved.inc,v 1.2 2012/08/30 01:09:17 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Bundle_Approved
*
* Purpose
*    Custom "Approved" status for a Workflow Bundle.
*    Equates to Approved to Go Live (or Safe Edit-Approved to Go Live) for a normal asset.
*
* Valid Transitions
*    APPROVED -> UNDER_CONSTRUCTION (Reject Changes: "Approved" => "Editing")
*      Rejects changes and moves bundled assets to Under Construction or
*      Safe Editing where possible. Requires admin access.
*    APPROVED -> LIVE               (Make Live: "Approved" => "Live")
*      Moves bundled assets to Live where possible. Requires admin access.
*
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Asset_Status_Bundle_Approved extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Bundle_Approved($owner)
	{
		assert_is_a($owner, 'Workflow_Bundle');
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_APPROVED;

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
		return translate('Approved');


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

		// cant do anything without admin access to at least one bundled asset
		if ($this->_bundledAssetAdminAccess() === TRUE) {
			$available_statuses[SQ_STATUS_UNDER_CONSTRUCTION] = translate('Reject Changes');

			$available_statuses[SQ_STATUS_LIVE]               = translate('Make Live');

		}

		return $available_statuses;

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
	function processStatusChange($new_status, $update_parent=TRUE)
	{
		if (!parent::processStatusChange($new_status, $update_parent)) {
			return FALSE;
		}

		if ($new_status == SQ_STATUS_LIVE) {
			// notify people that the asset has gone live
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			if (!$wfm->notifyOnLive($this->owner->id, $this->status_tag)) {
				return FALSE;
			}

			// update the last published date
			if (!$this->owner->setDate('published', time())) {
				return FALSE;
			}
		}

		return TRUE;

	}//end processStatusChange()


	/**
	* Returns TRUE if at least one asset has admin permission for the current user
	*
	* @param Workflow_Bundle	$owner	Bundle using this status
	*
	* @return boolean
	*/
	protected function _bundledAssetAdminAccess()
	{
		if (array_key_exists('bundled_asset_admin', $this->_tmp) === FALSE) {
			$this->_tmp['bundled_asset_links'] = FALSE;
			$bundled_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->owner->id, SQ_LINK_NOTICE);

			foreach ($bundled_asset_links as $linkid => $bundled_asset_link) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bundled_asset_link['minorid']);
				if ($asset->adminAccess() === TRUE) {
					$this->_tmp['bundled_asset_links'] = TRUE;
					break;
				} else {
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				}
			}
		}//end if already cached

		return $this->_tmp['bundled_asset_links'];

	}//end _bundledAssetAdminAccess()


}//end class

?>