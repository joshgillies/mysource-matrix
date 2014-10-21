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
* $Id: session_handler_default.inc,v 1.14 2013/07/25 23:23:48 lwright Exp $
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
* @version    $Revision: 1.14 $
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
		}


		$secure = FALSE;
		if (current_protocol() === 'https' && (defined('SQ_CONF_COOKIE_OPTION_SECURE') && SQ_CONF_COOKIE_OPTION_SECURE)) $secure = TRUE;

		$http_only = FALSE;
		$php_version_suits = (version_compare(PHP_VERSION, '5.2.0') >= 0);
		if ($php_version_suits && (defined('SQ_CONF_COOKIE_OPTION_HTTP_ONLY') && SQ_CONF_COOKIE_OPTION_HTTP_ONLY)) $http_only = TRUE;
		// Now, set to the default 'files' module and set the
		// appropriate cookie parameters.
		session_module_name('files');
		if ($php_version_suits) {
			session_set_cookie_params(0, $path, NULL, $secure, $http_only);
		} else {
			session_set_cookie_params(0, $path, NULL, $secure);
		}
		session_name('SQ_SYSTEM_SESSION');

        // Hardening.
		if (version_compare(PHP_VERSION, '5.3.0') >= 0) {
            ini_set('session.hash_bits_per_character', 5);
            ini_set('session.hash_function', 'sha512');
        } else {
            ini_set('session.hash_function', '1');
        }

		// If this config setting is ON, then we use the save path
		// defined in php.ini. If OFF (default), we use the cache
		// directory inside the Matrix install directory.
		if (!SQ_CONF_USE_DEFAULT_SESSION_SAVE_PATH) {
			if (SQ_CONF_CUSTOM_SESSION_SAVE_PATH !== '') {
				$cache_path = SQ_CONF_CUSTOM_SESSION_SAVE_PATH;
			} else {
				$cache_path = SQ_CACHE_PATH;
			}

			session_save_path($cache_path);
		}

	}//end init()


	/**
	* Unserialises a session
	*
	* It breaks each of the session variables up and stores them as the key elements
	* of an array with their values as the values of the array elements.
	*
	* @param string	$session_id	the ID of the session to unserialise
	*
	* @return array
	* @access public
	*/
	function unserialiseSession($session_id)
	{
		$session_file = session_save_path().'/sess_'.$session_id;

		$session_str = file_get_contents($session_file);
		// break the session at the word boundaries and the pipes
		$parts = preg_split('/\w+\|/', $session_str, -1, PREG_SPLIT_OFFSET_CAPTURE);
		$session_arr = Array();

		for ($i = 0; $i < count($parts); $i++) {
			if ($i == count($parts) - 1) continue;
			$offset = $parts[$i][1] + strlen($parts[$i][0]);
			$len = $parts[$i + 1][1] - 1 - $offset;

			$key = substr($session_str, $offset, $len);
			$session_arr[$key] = unserialize($parts[$i + 1][0]);
		}
		return $session_arr;

	}//end unserialiseSession()


	/**
	* Serialises the specified session contents and writes it to the specified session file
	*
	* @param string	$session_id			the ID of the session to write to
	* @param array	$session_contents	the contents to write to file
	*
	* @return boolean
	* @access public
	*/
	function serialiseSession($session_id, $session_contents)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$session_file = session_save_path().'/sess_'.$session_id;

		if (!file_exists($session_file)) {
			trigger_localised_error('CORE0072', sprintf(translate('Session file "%s" does not exist'), $session_file), E_USER_WARNING);
			return FALSE;
		}

		if (!is_array($session_contents)) {
			trigger_localised_error('CORE0004', sprintf(translate('Array expected for second argument, got %s'), gettype($session_contents)), E_USER_WARNING);
			return FALSE;
		}

		$session_str = '';
		foreach ($session_contents as $key => $val) {
			$session_str .= $key.'|'.serialize($val);
		}

		if (!string_to_file($session_str, $session_file)) {
			trigger_localised_error('CORE0021', sprintf(translate('Unable to write to session file "%s"'), $session_file), E_USER_WARNING);
			return FALSE;
		}
		return TRUE;

	}//end serialiseSession()


	/**
	* Returns TRUE if the session exists
	*
	* @param string	$session_id	The ID to check
	*
	* @return boolean
	*/
	public static function sessionExists($session_id)
	{
		$session_file = session_save_path().'/sess_'.$session_id;
		return file_exists($session_file);

	}//end sessionExists()


	/**
	* Synchronises the specified primary session with the current (secondary) session
	*
	* The current session will be overridden with the primary session if the
	* primary's timestamp is the most recent, and vice versa. $_SESSION is
	* used for the current session, and will be written to at script completion.
	*
	* @param string	$pri_sessionid	the sessionid of the primary session
	*
	* @return boolean
	* @access public
	*/
	public static function syncSession($pri_sessionid)
	{
		$pri_sess = self::unserialiseSession($pri_sessionid);

		if (!is_array($pri_sess)) {
			// something is definately wrong
			trigger_localised_error('CORE0071', translate('Primary session is corrupt'), E_USER_ERROR);
			return FALSE;
		}

		$pri_timestamp = array_get_index($pri_sess, 'SQ_SESSION_TIMESTAMP', -1);
		$sec_timestamp = array_get_index($_SESSION, 'SQ_SESSION_TIMESTAMP', -1);
		$pri_login_key = array_get_index($pri_sess, 'SQ_LOGIN_KEY', NULL);
		$sec_login_key = array_get_index($_SESSION, 'SQ_LOGIN_KEY', NULL);

		// if primary is younger
		if ($pri_timestamp > $sec_timestamp || $pri_timestamp == -1) {
			// copy primary to secondary
			$_SESSION = $pri_sess;
		} else {
			// copy secondary to primary
			$pri_sess = $_SESSION;
		}

		$now = time();
		$pri_sess['SQ_SESSION_TIMESTAMP'] = $now;
		$_SESSION['SQ_SESSION_TIMESTAMP'] = $now;

		// preserve login keys
		if (!is_null($pri_login_key)) {
			$pri_sess['SQ_LOGIN_KEY'] = $pri_login_key;
		}
		if (!is_null($sec_login_key)) {
			$_SESSION['SQ_LOGIN_KEY'] = $sec_login_key;
		}

		// save the sessionid of the primary so that we
		// know that we have run this script before. We won't have to
		// do anther browser refresh if we know this.
		$_SESSION['PRIMARY_SESSIONID'] = $pri_sessionid;
		$pri_sess['PRIMARY_SESSIONID'] = $pri_sessionid;

		// *** JEDI MIND TRICK *** you did not see us doing this... move along
		if (!self::serialiseSession($pri_sessionid, $pri_sess)) {
			trigger_localised_error('CORE0020', translate('Unable to write session back to primary session file'), E_USER_ERROR);
			return FALSE;
		}
		return TRUE;

	}//end syncSession()


}//end class


?>
