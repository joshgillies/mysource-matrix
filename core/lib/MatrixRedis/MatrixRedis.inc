<?php

class MatrixRedis
{

    /**
     * A cache of already established connections.
     */
    private static $_connections = array(
        'cache_manager'    => NULL,
        'deja_vu'          => NULL,
        'locking'          => NULL,
        'session_handling' => NULL,
    );


    /**
     * Get a connection for a particular type.
     * The type must match up with the redis.inc file, so valid types are currently:
     * - cache_manager
     * - deja_vu
     * - locking
     * - session_handling
     *
     * @param string $type The type of connection to get.
     *                     If you're getting the same type multiple times, a ping is sent
     *                     to make sure it's still valid.
     *
     * @return resource|false Returns false if a connection can't be made (or it has been dropped),
     *                        otherwise it returns the connection resource.
     */
    public static function getConnection($type)
    {
        if (in_array($type, array_keys(self::$_connections)) === FALSE) {
            trigger_error('Trying to use MatrixRedis for an invalid type ('.$type.')', E_USER_ERROR);
            return FALSE;
        }

        if (self::$_connections[$type] !== NULL) {
            if (self::_checkConnection($type)) {
                return self::$_connections[$type];
            }

            self::$_connections[$type] = NULL;
            return FALSE;
        }

        if (!extension_loaded('redis')) {
			trigger_error('Cannot use Redis for '.$type.'; it requires the redis PECL extension installed within PHP, which is not installed', E_USER_NOTICE);
			return FALSE;
		}
		if (!is_file(SQ_DATA_PATH.'/private/conf/redis.inc')) {
			trigger_error('Cannot use Redis for '.$type.'; the Redis configuration file is not set', E_USER_NOTICE);
			return FALSE;
		}

        $redis_conf = require(SQ_DATA_PATH.'/private/conf/redis.inc');
        if (!array_key_exists($type, $redis_conf['services'])) {
			trigger_error('Cannot use Redis for '.$type.'; the service is not set up.', E_USER_NOTICE);
			return FALSE;
		}

        $services = $redis_conf['services'][$type];
        $hosts = $redis_conf['hosts'];

        if (count($hosts) < 1) {
			trigger_error('Cannot use Redis for '.$type.'; no hosts are defined in the Redis configuration file', E_USER_NOTICE);
			return FALSE;
		}

        self::$_connections[$type] = new Redis();

        // Attempt to connect to defined redis hosts in order, break on success
        foreach ($services as $host_key => $weight) {
            $hostExists = array_key_exists($host_key, $hosts);
            if ($hostExists === FALSE) {
                log_error(get_class().': Cannot use Redis '.$type.'; host key "'.$host_key.'" assigned for use in '.$type.' but not defined as a host');
                continue;
            }

            $host = $hosts[$host_key];

            $conn_timeout = array_get_index($host, 'connection_timeout', 2);
            // in case there was a 'timeout' setting from the old config.
            $old_timeout  = array_get_index($host, 'timeout', 0);
            $conn_timeout = max($old_timeout, $conn_timeout);

            $pid          = array_get_index($host, 'persistent_id', '');
            $retry        = array_get_index($host, 'retry_interval', 2);
            $prefix       = array_get_index($host, 'prefix', '');
            $persistent   = array_get_index($host, 'persistent', FALSE);
            $read_timeout = array_get_index($host, 'read_timeout', 0);
            $database     = array_get_index($host, 'database', 0);

            $connFunction = 'connect';
            if ($persistent === TRUE) {
                $connFunction = 'pconnect';
            }

            if (isset($host['socket']) && $host['socket'] !== FALSE) {
                $host['host'] = $host['socket'];
            }

            if (substr($host['host'], 0, 1) === '/') {
                $connected = self::$_connections[$type]->$connFunction($host['host'], NULL, $conn_timeout, $pid);
            } else {
                if ($persistent === FALSE) {
                    $connected = self::$_connections[$type]->$connFunction($host['host'], $host['port'], $conn_timeout, $pid, $retry);
                } else {
                    $connected = self::$_connections[$type]->$connFunction($host['host'], $host['port'], $conn_timeout, $pid);
                }
            }

            if ($connected === FALSE) {
                log_error(get_class().' host '.$host_key.' not set up correctly for '.$type, E_USER_WARNING);
                continue;
            }

            if (!self::$_connections[$type]->select($database)) {
                log_error(get_class().' failed to select database '.$database.' for '.$type, E_USER_WARNING);
                continue;
            }

            if (!empty($prefix)) {
                self::$_connections[$type]->setOption(Redis::OPT_PREFIX, $prefix);
            }

            if ($read_timeout > 0) {
                self::$_connections[$type]->setOption(Redis::OPT_READ_TIMEOUT, $read_timeout);
            }

            break;

        }

        if (self::_checkConnection($type)) {
            return self::$_connections[$type];
        }

        self::$_connections[$type] = NULL;
        return FALSE;

    }//end getConnection()


    /**
     * Get the connection string for a particular connection type.
     *
     * The session handler uses a different format for the connection string, which is passed in to the
     * session_save_path() function.
     * This just returns the string format, it doesn't do any sort of connection handling.
     *
     * @param string $type The type of connection to get the connection string for.
     *
     * @return string
     */
    public static function getConnectionString($type)
    {
        if (in_array($type, array_keys(self::$_connections)) === FALSE) {
            trigger_error('Trying to use MatrixRedis for an invalid type ('.$type.')', E_USER_ERROR);
            return FALSE;
        }

        assert_true(extension_loaded('redis'), 'Cannot use Redis for '.$type.'; it requires the redis PECL extension installed within PHP, which is not installed');
        assert_true(is_file(SQ_DATA_PATH.'/private/conf/redis.inc'), 'Cannot use Redis for '.$type.'; the Redis configuration file is not set');

        $redis_conf = require(SQ_DATA_PATH.'/private/conf/redis.inc');

        assert_true(array_key_exists($type, $redis_conf['services']), 'Cannot use Redis for '.$type.'; the service is not set up.');
        $services = $redis_conf['services'][$type];
        $hosts = $redis_conf['hosts'];

        assert_true(count($hosts) > 0, 'Cannot use Redis for '.$type.'; no hosts are defined in the Redis configuration file');

        // Convert the host array (or the ones to be used for sessions) to the
        // URL format expected by the session handler, which requires options to
        // be specified by GET variables.
        $host_str = Array();
        foreach ($services as $host_key => $weight) {
            $hostExists = array_key_exists($host_key, $hosts);
            if ($hostExists === FALSE) {
                log_error(get_class().': Cannot use Redis '.$type.'; host key "'.$host_key.'" assigned for use in '.$type.' but not defined as a host');
                continue;
            }

            $hostInfo = $hosts[$host_key];

            if (isset($hostInfo['socket']) && $hostInfo['socket'] !== FALSE) {
                $hostInfo['host'] = $hostInfo['socket'];
            }

            if (substr($hostInfo['host'], 0, 1) === '/') {
                $hostInfo_str = 'unix://'.$hostInfo['host'];
            } else {
                $hostInfo_str = 'tcp://'.$hostInfo['host'].':'.$hostInfo['port'];
            }

            $query_str = Array();

            $conn_timeout = array_get_index($hostInfo, 'connection_timeout', 2);
            // in case there was a 'timeout' setting from the old config.
            $old_timeout  = array_get_index($hostInfo, 'timeout', 0);

            // Passing it as a query string, it uses 'timeout'.
            $timeout = max($old_timeout, $conn_timeout);

            $pid      = array_get_index($hostInfo, 'persistent_id', '');
            $retry    = array_get_index($hostInfo, 'retry_interval', 2);
            $prefix   = array_get_index($hostInfo, 'prefix', '');
            $database = array_get_index($hostInfo, 'database', '');

            // For sessions, this needs to be a 1 or 0.
            $persistent = array_get_index($hostInfo, 'persistent', FALSE);
            if ($persistent === TRUE) {
                $persistent = 1;
            } else {
                $persistent = 0;
            }

            $query_str[] = 'weight='.intval($weight);
            foreach (array('timeout', 'prefix', 'persistent', 'database') as $opt) {
                if (!empty($$opt)) {
                    $query_str[] = $opt.'='.rawurlencode($$opt);
                }
            }

            $host_str[] = $hostInfo_str.'?'.implode('&', $query_str);
        }

        $host_str = implode(', ', $host_str);
        return $host_str;

    }//end getConnectionString()


    /**
     * Check that the connection is ok by sending it a ping.
     *
     * @param string $type The type of connection.
     *
     * @return boolean
     */
    private static function _checkConnection($type)
    {
        try {
            // Ping the server to make sure the connection is working
            self::$_connections[$type]->ping();
        } catch(RedisException $e) {
            trigger_error('Redis connection error: '.$e->getMessage(), E_USER_NOTICE);

            return FALSE;
        }

        return TRUE;
    }

}//end class
