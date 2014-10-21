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
* $Id: asset_status_approved.inc,v 1.27 2013/02/15 07:16:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Approved
*
* Purpose
*    Provide an Approved status for an asset.
*
* Valid Transitions
*    APPROVED -> LIVE               if the user is an admin
*    APPROVED -> UNDER_CONSTRUCTION if the user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix
*/
class Asset_Status_Approved extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Approved($owner)
	{
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
		return translate('Approved To Go Live');


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

		// cant do anything without admin access
		if (!$this->owner->adminAccess('')) return $statii;

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because APPROVED means workflow is finished
			trigger_error('FIXME: Asset #'.$this->owner->id.' is not in workflow status but running workflow is found');
			log_dump(get_backtrace());
		} else {
			// workflow is not currently running for this asset
			$statii[SQ_STATUS_LIVE] = translate('Make Live');

			$statii[SQ_STATUS_UNDER_CONSTRUCTION] = translate('Reject Changes');

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

		if ($new_status == SQ_STATUS_LIVE) {
			// notify people that the asset has gone live
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			if (!$wfm->notifyOnLive($this->owner->id, $this->status_tag)) {
				return false;
			}

			// update the last published date
			if (!$this->owner->setDate('published', time())) {
				return false;
			}
		}

		return true;

	}//end processStatusChange()


}//end class

?>
