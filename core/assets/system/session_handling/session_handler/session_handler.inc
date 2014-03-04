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
* $Id: session_handler.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Session_Handler
*
* Purpose
*    Implementation file for the generic Session Handler asset.
*    This asset is not to be used by itself; subclass it to implement
*    your own Session Handler assets.
*
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.2 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler extends Asset
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Initialise the functionality of this session handler
	*
	* This may involve the changing of PHP's internal session save
	* handler and path, if we are using a built-in handler (like
	* 'files'), or custom functions if we are creating our own
	* session handler (using session_set_save_handler() and the 'user'
	* INI setting).
	*
	* @return void
	*/
	public static function init()
	{

	}//end init()


	/**
	* Synchronises two sessions together
	*
	* Required for Site Network assets to work properly. In this method,
	* the session passed will be synchronised to be the same as the
	* specified "primary" session ID.
	*
	* @param string	$primary_session_id	The ID that will provide the
	*									session details to sync with
	*
	* @return boolean
	*/
	public static function sessionSync($primary_session_id)
	{

	}//end sessionSync()


}//end class


?>
