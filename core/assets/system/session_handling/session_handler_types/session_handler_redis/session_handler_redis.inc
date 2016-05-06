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
* $Id: session_handler_redis.inc,v 1.15 2013/07/25 23:23:48 csmith Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/session_handling/session_handler/session_handler.inc';
require_once SQ_LIB_PATH.'/MatrixRedis/MatrixRedis.inc';

/**
* Session_Handler_Redis
*
* Purpose
*    Implementation file for the Redis Session Handler asset.
*    This handler currently uses the "redis" session handling pre-defined
*    within PHP's redis PECL extension.
*
* @author     Chris Smith <csmith@squiz.net>
* @version    $Revision: 1.15 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Redis extends Session_Handler
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
        $host_str = MatrixRedis::getConnectionString('session_handling');

        session_module_name('redis');

        session_save_path($host_str);

    }//end init()


    /**
    * Unserialises a session gathered from the Redis object
    *
    * It breaks each of the session variables up and stores them as the key elements
    * of an array with their values as the values of the array elements.
    *
    * @param string    $session_id    the ID of the session to unserialise
    *
    * @return array
    * @access public
    */
    function unserialiseSession($session_id)
    {
        $host_str = MatrixRedis::getConnectionString('session_handling');
        $redis    = new Redis();
        $redis->connect($host_str);
        $session_str = $redis->get($session_id);

        if ($session_str === FALSE) {
            throw new Exception('Passed primary session does not exist on Redis server');
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
    * Serialises the specified session contents and writes it to the Redis server
    *
    * @param string        $session_id            the ID of the session to write to
    * @param array        $session_contents    the contents to write to file
    *
    * @return boolean
    * @access public
    */
    function serialiseSession($session_id, $session_contents)
    {
        $host_str = MatrixRedis::getConnectionString('session_handling');
        $redis    = new Redis();
        $redis->connect($host_str);

        if (!is_array($session_contents)) {
            trigger_localised_error('CORE0004', sprintf(translate('Array expected for second argument, got %s'), gettype($session_contents)), E_USER_WARNING);
            return FALSE;
        }

        $session_str = '';
        foreach ($session_contents as $key => $val) {
            $session_str .= $key.'|'.serialize($val);
        }

        $result = $redis->set($session_id, $session_str);

        if ($result === FALSE) {
            throw new Exception('Unable to update session\'s contents into Redis');
        }

        return TRUE;

    }//end serialiseSession()


    /**
    * Returns TRUE if the session exists
    *
    * @param string    $session_id    The ID to check
    *
    * @return boolean
    */
    public static function sessionExists($session_id)
    {
        $host_str = MatrixRedis::getConnectionString('session_handling');
        $redis    = new Redis();
        $redis->connect($host_str);
        $return = $redis->get($session_id);
        return ($return !== FALSE);

    }//end sessionExists()


    /**
    * Synchronises two sessions together
    *
    * Required for Site Network assets to work properly. In this method,
    * the session passed will be synchronised to be the same as the
    * specified "primary" session ID.
    *
    * @param string    $primary_session_id    The ID that will provide the
    *                                    session details to sync with
    *
    * @return boolean
    */
    public static function syncSession($primary_session_id)
    {
        $host_str = MatrixRedis::getConnectionString('session_handling');
        $redis    = new Redis();
        $redis->connect($host_str);

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

}//end class


?>
