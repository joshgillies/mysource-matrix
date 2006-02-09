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
* $Id: asset_status.inc,v 1.15.2.2 2006/02/09 06:33:32 lwright Exp $
*
*/


/**
* Asset_Status
*
* Purpose
*    Provide a generic framework for status objects
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15.2.2 $
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
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status(&$owner)
	{
		$this->owner =& $owner;
		$this->MySource_Object();

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
		// we need write access to do any changes
		if (!$this->owner->readAccess()) return false;

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

				trigger_localised_error('SYS0250', E_USER_WARNING, $desc, $this->owner->name, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->owner->type(), 'name'), $this->owner->id, $this->getDescription(), $GLOBALS['SQ_SYSTEM']->user->name, $GLOBALS['SQ_SYSTEM']->user->id);

				return false;
			}
		}

		return true;

	}//end processStatusChange()


}//end class

?>
