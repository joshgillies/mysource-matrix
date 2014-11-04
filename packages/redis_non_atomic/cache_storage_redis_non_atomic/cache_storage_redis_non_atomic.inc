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
 */

require_once SQ_CORE_PACKAGE_PATH.'/system/cache_storage/cache_storage_types/cache_storage_redis/cache_storage_redis.inc';


/**
 * Cache storage using redis, without atomic operations (ie, the multi and exec commands)
 *
 * This cache storage method will only be available if the Redis PECL module
 * is available.
 * 
 * NOTE: this implementation is less than ideal, since we're now maintaining the core cache saving in two places
 * TODO: refactor Cache_Storage_Redis to isolate the operations we're after to support overriding in a
 * cleaner/supportable manner
 *
 * @package MySource_Matrix_Packages
 * @subpackage Redis_Non_Atomic
 */
class Cache_Storage_Redis_Non_Atomic extends Cache_Storage_Redis
{


	/**
	 * Constructor
	 *
	 * @param string	$assetid	the asset id to be loaded
	 *
	 */
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	 * Saves the specified contents to the cache, which will be identified
	 * by the specfied asset and key/url
	 *
	 * If cache entry already exists, it will be overwritten with the new contents
	 * and expiry time
	 *
	 * Expiry is sent to Redis itself as an offset in seconds. Sending a timestamp
	 * adds a risk because the timestamp has to be in the local time of the
	 * Redis server. If the server is on a dedicated hardware/VM and its date
	 * is out of sync with the server hosting Matrix, the cache period will be
	 * affected (in either direction).
	 *
	 * Indexes are maintained as redis sets, for listing cache keys associated with
	 * a URL or with an assetid. This is so that we can invalidate cache entries for a
	 * given URL or assetid.
	 *
	 * @param string	$cache_key  the cache key for this entry
	 * @param string	$perm_key   the permissions key for this entry
	 * @param string	$url		the url for this entry
	 * @param integer	$assetid    the asset id for this entry
	 * @param string	$data		the cache contents
	 * @param integer	$expiry		the expiry time (UTC)
	 *
	 * @return boolean
	 * @access public
	 */
	function store($cache_key, $perm_key, $url, $assetid, $data, $expiry)
	{
		if (!$this->_initRedis()) {
			return FALSE;
		}

		// 0 is handled as infinite rather than 0 seconds in redis. If we have 0,
		// don't set in cache, so that we're compatible with the standard cache storage.
		$expiry_length = $expiry - time();
		if ($expiry_length === 0) return FALSE;

		// For the indexes, we want just the base URL, no query string or trailing slash.
		$index_url = trim(preg_replace('/\?.*/', '', $url), '//');
		$mcache_key = SQ_CACHE_REDIS_PREFIX."$cache_key|$perm_key|$url|$assetid";

		 // Update the assetid and url indexes and save the cache entry in an atomic transaction
		try{
			$ret[] = $this->_redis->sadd(SQ_CACHE_REDIS_ASSET_INDEX_PREFIX.$assetid, $mcache_key);
			if ($ret[0] === FALSE) {
				log_write('Redis Error: Failed writing asset index to Redis, is this a slave?', 'error', E_USER_NOTICE);

				// We can't close because there may be a subsequent (valid) attempt to use the closed
				// connection, triggering an exception

				return FALSE;	
			}
			$ret[] = $this->_redis->sadd(SQ_CACHE_REDIS_URL_INDEX_PREFIX.$index_url, $mcache_key);
			if ($ret[1] === FALSE) {
				log_write('Redis Error: Failed writing url index to Redis, is this a slave?', 'error', E_USER_NOTICE);

				// We can't close because there may be a subsequent (valid) attempt to use the closed
				// connection, triggering an exception

				return FALSE;	
			}
			$ret[] = $this->_redis->setex($mcache_key, $expiry_length, $data);
			if ($ret[2] === FALSE) {
				log_write('Redis Error: Failed writing cache data to Redis, is this a slave?', 'error', E_USER_NOTICE);

				// We can't close because there may be a subsequent (valid) attempt to use the closed
				// connection, triggering an exception

				return FALSE;	
			}
		} catch(RedisException $e) {
			log_write('Redis Write Error: '.$e->getMessage(), 'error', E_USER_NOTICE);
			return FALSE;
		}
		//return result of setting cache entry
		return (isset($ret[2])) ? $ret[2] : FALSE;

	}//end store()


	/**
	 * Loads the contents from a cache file for the specified asset
	 * and unique key
	 * Cache keys are not valid unless they exist in both relevant indexes.
	 *
	 * returns FALSE if no cache version exist
	 *
	 * @param string	$cache_key  the cache key for this entry
	 * @param string	$perm_key   the permissions key for this entry
	 * @param string	$url		the url for this entry
	 * @param integer	$assetid    the asset id for this entry
	 *
	 * @return string
	 * @access public
	 */
	function read($cache_key, $perm_key, $url, $assetid)
	{
		if (!$this->_initRedis()) {
			return FALSE;
		}

		$index_url = trim(preg_replace('/\?.*/', '', $url), '//');
		$mcache_key = SQ_CACHE_REDIS_PREFIX."$cache_key|$perm_key|$url|$assetid";

		try {
			$exists[] = $this->_redis->sismember(SQ_CACHE_REDIS_ASSET_INDEX_PREFIX.$assetid, $mcache_key);
			if ($exists[0] === FALSE) {
				// We can't call this an error since sismember can return false as a "successful" result

				// We also can't close because there may be a subsequent attempt to read from the closed
				// connection, triggering an exception

				return FALSE;
			}

			$exists[] = $this->_redis->sismember(SQ_CACHE_REDIS_URL_INDEX_PREFIX.$index_url, $mcache_key);
			if ($exists[1] === FALSE) {
				// We can't call this an error since sismember can return false as a "successful" result

				// We also can't close because there may be a subsequent attempt to read from the closed
				// connection, triggering an exception

				return FALSE;
			}
		} catch(RedisException $e) {
			log_write('Redis Read Error: '.$e->getMessage(), 'error', E_USER_NOTICE);
			return FALSE;
		}

		return $this->_redis->get($mcache_key);

	}//end read()


	/**
	 * Removes all cache
	 *
	 * @return void
	 * @access public
	 */
	function clearAll()
	{
		// not supported, see https://github.com/twitter/twemproxy/blob/master/notes/redis.md
		return FALSE;
	}//end clearAll()


}//end class

