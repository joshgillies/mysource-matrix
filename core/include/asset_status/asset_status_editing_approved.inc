<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: asset_status_editing_approved.inc,v 1.10.2.1 2004/10/07 05:39:35 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

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
			if (!$this->owner->clearSystemVersion()) return false;

			// update the last published date
			if (!$this->owner->setDate('published', time())) return false;

			// notify people that the asset has gone live
			$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			if (!$wfm->notifyOnLive($this->owner->id, $this->status_tag)) return false;
		}
		return true;

	}//end processStatusChange()

}//end class

?>