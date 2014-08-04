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
* $Id: trigger_action.inc,v 1.10 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Trigger Action
*
* Parent Action that all trigger actions extend.
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action extends Asset
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error.
	* Must be overridden by the actions themselves.
	*
	* @param array	$settings	the stored settings for this action
	* @param array	$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		return FALSE;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// create HTML
		// populate HTML with settings
		// this should handle new (empty) conditions as well as saved ones

		// IMPORTANT NOTE:
		// any Form elements that use the prefix should use the prefix as an array index
		// i.e. <input name="$prefix['whatever']" value="someting" >
		// or   <input name="$prefix" value="something" >

		// NOT  <input name="$prefix_something" />
		// if you do this your processInterface function will not be getting any data back.

		// in short, you can either use prefix to either reference one value or be
		// an array of values, you cannot change the prefix to whatever you like as it will
		//  a) conflict with some other prefix, or
		//  b) make the interface useless

		// see the trigger_action_set_status condition for an example of use

		return '';

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* NOTE: Returns false on success
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param array	$request_data	array of data corresponding to this action
	*								as specified by the $prefix in getInterface
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// take and check request data
		// populate the settings
		// return status

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* Default to no locks - the children can override if necessary
	* <PRE>
	*	$locks = Array('assetid' => Array('attributes','links'));
	* </PRE>
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		// by default no locks needed
		return Array();

	}//end getLocks()


}//end class

?>
