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
* $Id: asset_status_archived.inc,v 1.19 2012/08/30 01:06:24 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Archived
*
* Purpose
*    Provide an Archived status for an asset.
*
* Valid Transitions
*    ARCHIVED -> UNDER_CONSTRUCTION if user is an admin
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix
*/
class Asset_Status_Archived extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Archived($owner)
	{
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_ARCHIVED;

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
		return translate('Archived');


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
		$statii[SQ_STATUS_UNDER_CONSTRUCTION] = translate('Place Under Construction');


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
		if (!parent::processStatusChange($new_status)) {
			return false;
		}
		return true;

	}//end processStatusChange()


}//end class

?>
