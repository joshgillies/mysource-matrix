<?php
require_once SQ_INCLUDE_PATH.'/asset_status/asset_status_pending_approval.inc';

/**
* Asset_Status_Editing_Approval
*
* Purpose
*    Provide a Safe Editing Approval status for an asset.
*
* Valid Transitions
*    EDITING_APPROVAL -> EDITING_APPROVAL if there is workflow running and current user can approve
*                                         but is not the last publisher
*    EDITING_APPROVAL -> EDITING_APPROVED if there is a workflow and current user can approve
*                                         and is the last publisher
*    EDITING_APPROVAL -> EDITING          if there is workflow running and the current user can approve
*                                         but chooses to reject the changes OR
*                                         there is workflow running and the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status_Editing_Approval extends Asset_Status_Pending_Approval
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing_Approval(&$owner)
	{
		$this->Asset_Status_Pending_Approval($owner);
		$this->status_tag = SQ_STATUS_EDITING_APPROVAL;

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
		return 'Safe Editing Pending Approval';

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
			if ($code == SQ_STATUS_APPROVED) $return_statii[SQ_STATUS_EDITING_APPROVED] = $name;
			else if ($code == SQ_STATUS_PENDING_APPROVAL) $return_statii[SQ_STATUS_EDITING_APPROVAL] = $name;
			else if ($code == SQ_STATUS_UNDER_CONSTRUCTION) $return_statii[SQ_STATUS_EDITING] = $name;
			else $return_statii[$code] = $name;
		}

		return $return_statii;

	}//end getAvailableStatii()

}//end class

?>