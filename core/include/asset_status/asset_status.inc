<?php

/**
* Asset_Status
*
* Purpose
*    Provide a generic framework for status objects
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Status extends Resolve_Object
{

	/**
	* The asset who we belong to
	* @var object Asset
	*/
	var $owner = null;

	/**
	* The workflow for the owner asset
	* @var object Workflow_Manager
	*/
	var $workflow = null;


	/**
	* Constructor
	*
	* @param object Asset	&$owner	the asset that this status belongs to
	*/
	function Asset_Status(&$owner)
	{
		$this->owner = $owner;
		$this->Resolve_Object();

	}//end constructor


	/**
	* Returns a string used to describe this status
	*
	* @return string
	* @access public
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

		// make sure we can actually set this status
		$valid_statii = $this->getAvailableStatii();
		if (!isset($valid_statii[$new_status])) return false;
		return true;

	}//end processStatusChange()


	/**
	* Returns a reference to the workflow manager for the owner asset
	*
	* @param boolean	$force_refresh	dont use the cached version of the manager even if it exists
	*
	* @return object Workflow_Manager
	* @access public
	*/
	function &getWorkflow($force_refresh=false)
	{
		if (!$force_refresh && !is_null($this->workflow)) return $this->workflow;
		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		return new Workflow_Manager($this->owner);

	}//end &getWorkflow()

}//end class

?>