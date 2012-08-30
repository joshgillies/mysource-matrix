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
* $Id: trigger_condition.inc,v 1.10 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
/**
* Trigger Condition
*
* Parent Condition that all trigger conditions extend.
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition extends Asset
{


	/**
	* Evaluate this condition - needs to be overridden by the conditions themselves.
	*
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		return FALSE;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
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
		// Any Form elements that use the prefix should use the prefix as an array index
		// i.e. <input name="$prefix['whatever']" value="someting" >
		// or   <input name="$prefix" value="something" >

		// NOT  <input name="$prefix_something" />
		// if you do this your processInterface function will not be getting any data back.

		// in short, you can either use prefix to either reference one value or be
		// an array of values, you cannot change the prefix to whatever you like as it will
		//  a) conflict with some other prefix, or
		//  b) make the interface useless

		// see the trigger_condition_asset_type condition for an example of use

		return $interface_string;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* NOTE: returns false if there is no error
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// decompose interface
		// populate the settings
		// return status

		return FALSE;

	}//end processInterface()


	/**
	* Sets the hash using the settings from the condition
	* Consult the hash object to see the list of setter functions
	* NOTE: returns false if there is no error, like processInterface()
	* Returns a string on error
	*
	* @param array	&$settings	a container for any data the condition might want to save
	* @param object	&$hash		hash object that takes data that helps to speed up trigger firing
	*
	* @return mixed string|boolean
	* @access public
	*/
	public static function setHash(&$settings, &$hash)
	{
		return FALSE;

	}//end setHash()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* Default to false, children can override if necessary
	*
	* @return boolean
	* @access public
	*/
	public static function allowMultiple()
	{
		// default to to disallow
		return FALSE;

	}//end allowMultiple()


}//end class

?>
