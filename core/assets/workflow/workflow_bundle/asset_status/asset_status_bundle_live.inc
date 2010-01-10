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
* $Id: asset_status_bundle_live.inc,v 1.1.2.2 2010/01/10 23:13:48 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_status/asset_status.inc';

/**
* Asset_Status_Bundle_Live
*
* Purpose
*    Custom "Live" status for a Workflow Bundle.
*    Equates to Live for a normal asset.
*
* Valid Transitions
*    LIVE -> UNDER_CONSTRUCTION       (Start Editing: "Live" => "Editing")
*      If at least one bundled asset has workflow applied, and it cannot
*      be completed in one go
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix
*/
class Asset_Status_Bundle_Live extends Asset_Status
{


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status_Bundle_Live($owner)
	{
		assert_is_a($owner, 'Workflow_Bundle');
		$this->Asset_Status($owner);
		$this->status_tag = SQ_STATUS_LIVE;

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
		return translate('status_live');

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

		if ($this->owner->writeAccess() === TRUE) {
			$available_statuses[SQ_STATUS_UNDER_CONSTRUCTION] = translate('core_workflow_bundle_start_editing');
		}

		return $available_statuses;

	}//end getAvailableStatii()


}//end class

?>