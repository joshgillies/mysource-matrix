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
* $Id: trigger_event.inc,v 1.7 2012/08/30 01:09:15 ewang Exp $
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
* @version $Revision: 1.7 $
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
