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
 * $Id: cache_storage_redis.inc,v 1.3 2013/03/11 23:29:11 ewang Exp $
 *
 */

require_once SQ_CORE_PACKAGE_PATH.'/system/cache_storage/cache_storage/cache_storage.inc';

define('SQ_CACHE_REDIS_PREFIX', 'sq_cache|');
define('SQ_CACHE_REDIS_ASSET_INDEX_PREFIX', 'sq_cache_asset_index|');
define('SQ_CACHE_REDIS_URL_INDEX_PREFIX', 'sq_cache_url_index|');

/**
 * Cache_Storage_Redis
 *
 * Cache storage using redis.
 *
 * This cache storage method will only be available if the Redis PECL module
 * is available.
 *
 * @author  James Hurst <james@riverratrecords.com>
 * @author  Hamish Forbes <hforbes@squiz.co.uk>
 * @version $Revision: 1.3 $
 * @package MySource_Matrix_Packages
 * @subpackage __core__
 */
class Cache_Storage_Redis extends Cache_Storage
{
	/**
	* Redis
	*
	* @var object Redis
	*/
	private $_redis;


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


	public function initRedis()
	{
		assert_true(extension_loaded('redis'), 'Cannot use Redis Cache Storage; it requires the redis PECL extension installed within PHP, which is not installed');
		assert_true(is_file(SQ_DATA_PATH.'/private/conf/redis.inc'), 'Cannot use Redis Cache Storage; the Redis configuration file is not set');

		$redis_conf = require(SQ_DATA_PATH.'/private/conf/redis.inc');
		$services =& $redis_conf['services']['cache_manager'];
		$hosts =& $redis_conf['hosts'];

		assert_true(count($hosts) > 0, 'Cannot use Redis Cache Storage; no hosts are defined in the Redis configuration file');

		// If PHP has the redis module installed, instantiate it and try to load some config.
		$this->_redis = new Redis();

		// Attempt to connect to defined redis hosts in order, break on success
		foreach ($services as $host_key => $weight) {
			$host = $hosts[$host_key];
			if ($host['socket'] !== FALSE) {
				if ($this->_redis->pconnect($host['socket'])) {
					break;
				}
			} else {
				if ($this->_redis->pconnect($host['host'], $host['port'], 2)) {
					break;
				}
			}
		}

		try {
			// Ping the server to make sure the connection is working
			$this->_redis->ping();
		} catch(RedisException $e) {
			// Bad connection, set the redis object to false
			// Cache functions below will return without trying to send redis commands on a dead connection

			// TODO: Change this to a proper Matrix error/warning
			log_write('Redis Connect Error: '.$e->getMessage(), 'error', E_USER_NOTICE);

			$this->_redis = FALSE;
			return FALSE;
		}
		return TRUE;

	}//end initRedis()


	/**
	 * Is this cache storage mechanism available?
	 *
	 * Redis storage only available when Redis extension is available.
	 *
	 * @return boolean
	 */
	public static function isAvailable()
	{
		if (extension_loaded('redis') === TRUE) {
			if (is_file(SQ_DATA_PATH.'/private/conf/redis.inc')) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			return FALSE;
		}

	}//end isAvailable()


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
		if (!$this->_redis) {
			if (!$this->initRedis()) {
				return FALSE;
			}
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
		    $ret = $this->_redis->multi()
				    ->sadd(SQ_CACHE_REDIS_ASSET_INDEX_PREFIX.$assetid, $mcache_key)
				    ->sadd(SQ_CACHE_REDIS_URL_INDEX_PREFIX.$index_url, $mcache_key)
				    ->setex($mcache_key, $expiry_length, $data)
				    ->exec();
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
		if (!$this->_redis) {
			if (!$this->initRedis()) {
				return FALSE;
			}
		}

		$index_url = trim(preg_replace('/\?.*/', '', $url), '//');
		$mcache_key = SQ_CACHE_REDIS_PREFIX."$cache_key|$perm_key|$url|$assetid";
		try {
		$exists = $this->_redis->multi()
							   ->sismember(SQ_CACHE_REDIS_ASSET_INDEX_PREFIX.$assetid, $mcache_key)
							   ->sismember(SQ_CACHE_REDIS_URL_INDEX_PREFIX.$index_url, $mcache_key)
							   ->exec();
		} catch(RedisException $e) {
			log_write('Redis Read Error: '.$e->getMessage(), 'error', E_USER_NOTICE);
			return FALSE;
		}
		if (!$exists[0] || !$exists[1]) {
			// Cache key has been removed from index sets, consider this key invalid
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
		if (!$this->_redis) {
			if (!$this->initRedis()) {
				return FALSE;
			}
		}

		return $this->_redis->flushDB();

	}//end clearAll()


	/**
	 * Cleans up expired entries.
	 *
	 * Assumed that redis does this for us, so nothing to be done here.
	 *
	 * @return void
	 * @access public
	 */
	function cleanExpiredEntries()
	{
		return TRUE;

	}//end cleanExpiredEntries()


	/**
	 * Returns true if this storage method can clear cache for specific assetids.
	 *
	 * @return bool
	 * @access public
	 */
	function canClearByAssetid()
	{
		return TRUE;

	}//end canClearByAssetid()


	/**
	* Invalidates all cache entries related to this assetid
	*
	* @param array	$assetids	id of the the asset that owns the cache file
	*
	* @return void
	* @access public
	*/
	function clear($assetids)
	{
		if (!$this->_redis) {
			if (!$this->initRedis()) {
				return FALSE;
			}
		}

		foreach ($assetids as $assetid) {
			$this->_redis->del(SQ_CACHE_REDIS_ASSET_INDEX_PREFIX.$assetid);
		}

	}//end clear()


	/**
	* Invalidates caches of assets that were cached when a certain URL was requested
	*
	* This includes caches of nested content that were generated by reaching
	* the passed URL, not just the top-level asset.
	*
	* @param string $url The url to delete cache entries for
	*
	* @access public
	*/
	function clearCachedAssetsByUrl($url)
	{
		if (!$this->_redis) {
			if (!$this->initRedis()) {
				return FALSE;
			}
		}

		$this->_redis->del(SQ_CACHE_REDIS_URL_INDEX_PREFIX.$url);

	}//end clearCachedAssetsByUrl


	/**
	* Returns an array of file paths for certain assetids, for clear cache HIPO job
	*
	* Not used by this storage method, since filesystem is unused. Always
	* returns an empty array.
	*
	* @param array	$assetids	id of the the asset that owns the cache file
	*
	* @return array
	* @access public
	*/
	function getFilePaths($assetids)
	{
		return Array();

	}//end getFilePaths()


	/**
	* Returns an array of all file paths for clear cache HIPO job
	*
	* Not used by this storage method, since filesystem is unused. Always
	* returns an empty array.
	*
	* @param string	$option	the option of the result
	*
	* @return array
	* @access public
	*/
	function getAllFilePaths($option='')
	{
		return Array();

	}//end getAllFilePaths()



}//end class

?>