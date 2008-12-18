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
* $Id: session_handler_default.inc,v 1.2 2008/12/18 05:34:40 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/session_handling/session_handler/session_handler.inc';


/**
* Session_Handler_Default
*
* Purpose
*    Implementation file for the Default Session Handler asset.
*    This handler just uses the default "files" session handling pre-
*    defined within PHP.
*
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.2 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Default extends Session_Handler
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
		// Calculate the domain and path for the session cookie.
		$domain = strip_url(sq_web_path('root_url'), TRUE);
		$pos = strpos($domain, '/');
		$path = '/';
		if ($pos !== FALSE) {
			$path = substr($domain, $pos);
			$domain = substr($domain, 0, $pos);
		}

		// Remove the port number from the host part, if any.
		// Session cookies cannot have a port in the domain.
		$domain = preg_replace('|:\d+$|', '', $domain);

		// Now, set to the default 'files' module and set the
		// appropriate cookie parameters.
		session_module_name('files');
		session_set_cookie_params(0, $path, $domain);
		session_name('SQ_SYSTEM_SESSION');

		// If this config setting is ON, then we use the save path
		// defined in php.ini. If OFF (default), we use the cache
		// directory inside the Matrix install directory.
		if (!SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH) {
			session_save_path(SQ_CACHE_PATH);
		}

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
