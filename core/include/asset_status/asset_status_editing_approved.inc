<?php
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
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Status_Editing_Approved extends Asset_Status_Approved
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Editing_Approved(&$owner)
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
	function getDescription()
	{
		return 'Safe Edit Approved To Go Live';

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
			if ($code == SQ_STATUS_UNDER_CONSTRUCTION) $return_statii[SQ_STATUS_EDITING] = $name;
			else $return_statii[$code] = $name;
		}

		return $return_statii;

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

		if ($new_status == SQ_STATUS_LIVE) {
			// safe edit is finished, so we dont need the system version any more
			if (!$this->owner->clearSystemVersion()) {
				return false;
			}
		}
		return true;

	}//end processStatusChange()

}//end class

?>