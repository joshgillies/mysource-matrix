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
* $Id: session_handler_memcache.inc,v 1.14 2013/07/25 06:59:33 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/session_handling/session_handler/session_handler.inc';


/**
* Session_Handler_Memcache
*
* Purpose
*    Implementation file for the Memcache Session Handler asset.
*    This handler currently uses the "memcache" session handling pre-defined
*    within PHP's memcache PECL extension.
*
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.14 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Memcache extends Session_Handler
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
		$host_str = self::getHostString();

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
		// Now, set to the 'memcache' module and set the
		// appropriate save path (ie. the connection string to thee
		// memcached server).
		session_name('SQ_SYSTEM_SESSION');
		if ($php_version_suits) {
			session_set_cookie_params(0, $path, NULL, $secure, $http_only);
		} else {
			session_set_cookie_params(0, $path, NULL, $secure);
		}
		session_module_name('memcache');

		session_save_path($host_str);

	}//end init()


	/**
	* Unserialises a session gathered from the Memcache object
	*
	* It breaks each of the session variables up and stores them as the key elements
	* of an array with their values as the values of the array elements.
	*
	* @param Memcache	$memcache	the Memcache connection object
	* @param string		$session_id	the ID of the session to unserialise
	*
	* @return array
	* @access public
	*/
	function unserialiseSession($session_id)
	{
		$host_str = self::getHostString();
		$memcache = new Memcache();
		$memcache->addServer($host_str);
		$session_str = $memcache->get($session_id);

		if ($session_str === FALSE) {
			throw new Exception('Passed primary session does not exist on Memcache server');
		}

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
	* Serialises the specified session contents and writes it to the Memcache server
	*
	* @param Memcache	$memcache			the Memcache connection object
	* @param string		$session_id			the ID of the session to write to
	* @param array		$session_contents	the contents to write to file
	*
	* @return boolean
	* @access public
	*/
	function serialiseSession($session_id, $session_contents)
	{
		$host_str = self::getHostString();
		$memcache = new Memcache();
		$memcache->addServer($host_str);

		if (!is_array($session_contents)) {
			trigger_localised_error('CORE0004', E_USER_WARNING, gettype($session_contents));
			return FALSE;
		}

		$session_str = '';
		foreach ($session_contents as $key => $val) {
			$session_str .= $key.'|'.serialize($val);
		}

		$result = $memcache->set($session_id, $session_str);

		if ($result === FALSE) {
			throw new Exception('Unable to update session\'s contents into Memcache');
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
		$host_str = self::getHostString();
		$memcache = new Memcache();
		$memcache->addServer($host_str);
		$return = $memcache->get($session_id);
		return ($return !== FALSE);

	}//end sessionExists()


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
	public static function syncSession($primary_session_id)
	{
		$host_str = self::getHostString();
		$memcache = new Memcache();
		$memcache->addServer($host_str);

		$pri_sess = self::unserialiseSession($primary_session_id);

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
		$_SESSION['PRIMARY_SESSIONID'] = $primary_session_id;
		$pri_sess['PRIMARY_SESSIONID'] = $primary_session_id;

		self::serialiseSession($primary_session_id, $pri_sess);

		return TRUE;

	}//end syncSession()


	public static function getHostString()
	{
		    assert_true(extension_loaded('memcache'), 'Cannot use Memcache Session Handler; it requires the memcache PECL extension installed within PHP, which is not installed');
		    assert_true(file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc'), 'Cannot use Memcache Session Handler; the Memcache configuration file is not set');

		    $memcache_conf = require(SQ_DATA_PATH.'/private/conf/memcache.inc');
		    $hosts =& $memcache_conf['hosts'];
		    $services =& $memcache_conf['services'];

		    assert_true(count($hosts) > 0, 'Cannot use Memcache Session Handler; no hosts are defined in the Memcache configuration file');
		    assert_true(array_key_exists('session_handling', $services) === TRUE, 'Cannot use Memcache Session Handler; no Memcache hosts are assigned to session handling');
		    assert_true(count($services['session_handling']) > 0, 'Cannot use Memcache Session Handler; no Memcache hosts are assigned to session handling');

		    // Convert the host array (or the ones to be used for sessions) to the
		    // URL format expected by the session handler, which requires options to
		    // be specified by GET variables.
		    $host_str = Array();
		    foreach ($services['session_handling'] as $host_key => $weight) {
			    assert_true(array_key_exists($host_key, $hosts) === TRUE, 'Cannot use Memcache Session Handler; host key "'.$host_key.'" assigned for use in session handling but not defined as a host');
			    $this_host = $hosts[$host_key];
			    $this_host_str = 'tcp://'.$this_host['host'].':'.$this_host['port'];

			    $query_str = Array();
			    $query_str[] = 'weight='.rawurlencode($weight);

			    if (array_key_exists('persistent', $this_host) === TRUE) {
				    $query_str[] = 'persistent='.rawurlencode($this_host['persistent'] ? '1' : '0');
			    }
			    if (array_key_exists('timeout', $this_host) === TRUE) {
				    $query_str[] = 'timeout='.rawurlencode($this_host['timeout']);
			    }
			    if (array_key_exists('retry_interval', $this_host) === TRUE) {
				    $query_str[] = 'retry_interval='.rawurlencode($this_host['retry_interval']);
			    }
			    if (array_key_exists('status', $this_host) === TRUE) {
				    $query_str[] = 'status='.rawurlencode($this_host['status'] ? '1' : '0');
			    }
			    $host_str[] = $this_host_str.'?'.implode('&', $query_str);
		    }
		    $host_str = implode(', ', $host_str);
		    return $host_str;
	}
}//end class


?>
