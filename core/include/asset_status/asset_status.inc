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
* $Id: asset_status.inc,v 1.25 2012/08/30 01:06:24 ewang Exp $
*
*/


/**
* Asset_Status
*
* Purpose
*    Provide a generic framework for status objects
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix
*/
class Asset_Status extends MySource_Object
{

	/**
	* The actual status id that this status represents
	* @var int
	*/
	var $status_tag = 0;

	/**
	* The asset who we belong to
	* @var object Asset
	*/
	var $owner = null;


	/**
	* Constructor
	*
	* @param object Asset	$owner	the asset that this status belongs to
	*/
	function Asset_Status($owner)
	{
		$this->owner = $owner;
		$this->MySource_Object();

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
		return 'Unknown Status';

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
		return Array();

	}//end getAvailableStatii()


	/**
	* Get a message about status change options
	*
	* @return string
	* @access public
	*/
	function getStatusChangeMessage()
	{
		return NULL;

	}//end getStatusChangeMessage()


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
		// we need write access to do any changes
		if (!$this->owner->readAccess()) {
			trigger_localised_error('SYS0309', sprintf(translate('You do not have permission to change the status of "%1$s" (#%2$s)'), $this->owner->name, $this->owner->id), E_USER_WARNING)
			return false;
		}

		// if the new status is the same as our status, we can handle this
		if ($new_status == $this->status_tag) return true;

		// do we need to check if the new status is valid?
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_STATUS_INTEGRITY)) {

			// make sure we can actually set this status
			$valid_statii = $this->getAvailableStatii();
			if (!isset($valid_statii[$new_status])) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

				$code = strtolower((string)get_bit_names('SQ_STATUS_', $new_status));
				$status_code = 'asset_status_'.$code;
				require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
				eval('$desc = '.$status_code.'::getDescription();');

				trigger_localised_error('SYS0250', sprintf('Unable to process the status change.
"%1$s" is not a valid option for the "%2$s" %3$s asset (#%4$s) in its current "%5$s" state.  This may be due to required metadata fields being empty, or permissions for the current user "%6$s" (#%7$s)', $desc, $this->owner->name, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'), $this->owner->id, $this->getDescription(), $GLOBALS['SQ_SYSTEM']->user->name, $GLOBALS['SQ_SYSTEM']->user->id), E_USER_WARNING)

				return false;
			}
		}

		// we are now certain that we will be changing the status that we were asked to
		// lets fire the trigger event 'trigger_event_before_status_change' to the triggers
		// know we will next be changing the status. Last chance to stop before we GO GO GO !!
		$event_data = Array(
						'old_status'			=> $this->status_tag,
						'new_status'			=> $new_status,
						'allow_unrestricted'	=> isset($this->owner->_tmp['allow_unrestricted']) ? $this->owner->_tmp['allow_unrestricted'] : NULL,
					  );

		// if the broadcast returns FALSE, dont go ahead with changing the status
		// NOTE : it is likely to return FALSE only if the trigger is blocking
		// in other cases the asset status change will still go ahead in full swing
		if (!$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_before_status_change', $this->owner, $event_data)) {
			return FALSE;
		}

		return TRUE;

	}//end processStatusChange()


}//end class

?>
