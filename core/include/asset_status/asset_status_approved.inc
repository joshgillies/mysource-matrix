<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: asset_status_approved.inc,v 1.8 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Status_Approved extends Asset_Status
{

	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status_Approved(&$owner)
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
	function getDescription()
	{
		return 'Approved To Go Live';

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

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($this->owner->id, true, true);

		if (!empty($running_schemas)) {
			// workflow is currently running, but it should not be
			// because APPROVED means workflow is finished
			bam('FIXME: SOMETHING HAS GONE WRONG HERE');
		} else {
			// workflow is not currently running for this asset
			$statii[SQ_STATUS_LIVE] = 'Make Live';
			$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Reject Changes';
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
		return true;

	}//end processStatusChange()

}//end class

?>