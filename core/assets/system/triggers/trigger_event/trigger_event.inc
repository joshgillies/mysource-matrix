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
* $Id: trigger_event.inc,v 1.5 2006/09/22 00:58:17 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Trigger Event
*
* Parent Event that all trigger events extend.
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Event extends Asset
{


	// broadcaster of the event
	var $broadcaster;


	/**
	* Constructor
	*/
	function Trigger_Event($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Sets the broadcaster of the event to the passed asset
	*
	* @param object Asset   &$asset the broadcaster
	*
	* @access public
	* @return void
	*/
	function setBroadcaster(&$asset)
	{
		// set the broadcaster to the calling asset
		$this->broadcaster = &$asset;

	}//end setBroadcaster()


	/**
	* Returns the broadcaster of the event to the passed asset
	*
	* @access public
	* @return object Asset
	*/
	function &getBroadcaster()
	{
		return $this->broadcaster;

	}//end getBroadcaster()


}//end class

?>
