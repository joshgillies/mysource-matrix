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
 * $Id: cache_storage_memcache.inc,v 1.14 2013/04/29 09:40:53 cupreti Exp $
 *
 */

require_once SQ_CORE_PACKAGE_PATH.'/system/cache_storage/cache_storage/cache_storage.inc';

define('SQ_CACHE_MEMCACHE_PREFIX', 'sq_cache|');
define('SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX', 'sq_cache_asset_index|');
define('SQ_CACHE_MEMCACHE_URL_INDEX_PREFIX', 'sq_cache_url_index|');

/**
 * Cache_Storage_Memcache
 *
 * Cache storage using memcache.
 *
 * This cache storage method will only be available if the Memcache PECL module
 * is available.
 *
 * @author  James Hurst <james@riverratrecords.com>
 * @author  Luke Wright <lwright@squiz.net>
 * @version $Revision: 1.14 $
 * @package MySource_Matrix_Packages
 * @subpackage __core__
 */
class Cache_Storage_Memcache extends Cache_Storage
{
	/**
	* Memcache
	*
	* @var object Memcache
	*/
	private $_memcache;


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
	* Initialise the memcache storage
	*
	* @return boolean
	* @access public
	*/
	public function initMemcache()
	{
		if (!extension_loaded('memcache')) {
			trigger_error('Cannot use Memcache Cache Storage; it requires the memcache PECL extension installed within PHP, which is not installed', E_USER_NOTICE);
			return FALSE;
		}
		if (!is_file(SQ_DATA_PATH.'/private/conf/memcache.inc')) {
			trigger_error('Cannot use Memcache Cache Storage; the Memcache configuration file is not set', E_USER_NOTICE);
			return FALSE;
		}

		$memcache_conf = require(SQ_DATA_PATH.'/private/conf/memcache.inc');
		$hosts =& $memcache_conf['hosts'];
		$services =& $memcache_conf['services'];

		if (count($hosts) < 1) {
			trigger_error('Cannot use Memcache Cache Storage; no hosts are defined in the Memcache configuration file', E_USER_NOTICE);
			return FALSE;
		}
		if (array_key_exists('cache_manager', $services) !== TRUE) {
			trigger_error('Cannot use Memcache Cache Storage; no Memcache hosts are assigned to cache manager', E_USER_NOTICE);
			return FALSE;
		}
		if (count($services['cache_manager']) < 1) {
			trigger_error('Cannot use Memcache Cache Storage; no Memcache hosts are assigned to cache manager', E_USER_NOTICE);
			return FALSE;
		}

		// If PHP has the memcache module installed, instantiate it and try to load some config.
		$this->_memcache = new Memcache;

		foreach ($services['cache_manager'] as $host_key => $weight) {
			if (array_key_exists($host_key, $hosts) !== TRUE) {
				trigger_error('Cannot use Memcache Cache Storage; host key "'.$host_key.'" assigned for use for cache manager but not defined as a host', E_USER_NOTICE);
				return FALSE;
			}
			$host = $hosts[$host_key];
			$this->_memcache->addServer($host['host'], $host['port'], $host['persistent'], $weight, $host['timeout'], $host['retry_interval'], $host['status'], Array($this, 'failureCallback'));
		}

		$this->_memcache->setCompressThreshold($memcache_conf['compression_threshold'], $memcache_conf['compression_min_saving']);

		return TRUE;

	}//end initMemcache()


	/**
	 * Is this cache storage mechanism available?
	 *
	 * Memcache storage only available when Memcache extension is available.
	 *
	 * @return boolean
	 */
	public static function isAvailable()
	{
		if (extension_loaded('memcache') === TRUE) {
			if (is_file(SQ_DATA_PATH.'/private/conf/memcache.inc')) {
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
	 * The URL is MD5'd before being used as part of the cache key for Memcache.
	 * Memcache keys have a limit of 250 characters before being truncated; if
	 * cache and perm keys are both set, the URL has a useful length of about
	 * 170 characters before truncating the key (and potentially the asset ID)
	 * off it - less in the case of a shadow asset.
	 *
	 * Expiry is sent to Memcache itself as an offset in seconds. Sending a timestamp
	 * adds a risk because the timestamp has to be in the local time of the
	 * Memcache server. If the server is on a dedicated hardware/VM and its date
	 * is out of sync with the server hosting Matrix, the cache period will be
	 * affected (in either direction).
	 *
	 * Indexes are maintained, for listing cache keys associated with a URL or with an assetid. This
	 * is so that we can delete cache entries for a given URL or assetid.
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
		if (!$this->_memcache) $this->initMemcache();
		
		if ($this->_memcache) {
			// 0 is handled as infinite rather than 0 seconds in memcache. If we have 0,
			// don't set in cache, so that we're compatible with the standard cache storage.
			$expiry_length = $expiry - time();
			if ($expiry_length === 0) return FALSE;
			
			// For the indexes, we want just the base URL, no query string or trailing slash.
			$index_url = md5(trim(preg_replace('/\?.*/', '', $url), '/'));
			
			$url = md5($url);
			$mcache_key = SQ_CACHE_MEMCACHE_PREFIX."$cache_key|$perm_key|$url|$assetid";
			
			// Get both the indexes at once (faster).
			$indexes = $this->_memcache->get(Array(SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX.$assetid, SQ_CACHE_MEMCACHE_URL_INDEX_PREFIX.$index_url));
			
			// Update the assetid index if necessary.
			$assetid_hits = isset($indexes[SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX.$assetid]) ? $indexes[SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX.$assetid] : Array();
			if (!in_array($mcache_key, $assetid_hits)) {
				array_push($assetid_hits, $mcache_key);
				if (!$this->_memcache->set(SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX.$assetid, $assetid_hits)) {
					return FALSE;
				}
			}
			
			// Update the url index if necessary.
			$url_hits = isset($indexes[SQ_CACHE_MEMCACHE_URL_INDEX_PREFIX.$index_url]) ? $indexes[SQ_CACHE_MEMCACHE_URL_INDEX_PREFIX.$index_url] : Array();
			if (!in_array($mcache_key, $url_hits)) {
				array_push($url_hits, $mcache_key);
				if (!$this->_memcache->set(SQ_CACHE_MEMCACHE_URL_INDEX_PREFIX.$index_url, $url_hits)) {
					return FALSE;
				}
			}

			// Save the actual cache entry
			$ok = $this->_memcache->set($mcache_key, $data, FALSE, $expiry_length);
			return $ok;
		} else {
			return FALSE;
		}

	}//end store()


	/**
	 * Loads the contents from a cache file for the specified asset
	 * and unique key
	 *
	 * The URL is MD5'd before being used as part of the cache key for Memcache.
	 * Memcache keys have a limit of 250 characters before being truncated; if
	 * cache and perm keys are both set, the URL has a useful length of about
	 * 170 characters before truncating the key (and potentially the asset ID)
	 * off it - less in the case of a shadow asset.
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
		if (!$this->_memcache) $this->initMemcache();

		if ($this->_memcache) {
			$url = md5($url);
			return $this->_memcache->get(SQ_CACHE_MEMCACHE_PREFIX."$cache_key|$perm_key|$url|$assetid", FALSE);
		} else {
			return FALSE;
		}

	}//end read()


	/**
	 * Removes all cache
	 *
	 * @return void
	 * @access public
	 */
	function clearAll()
	{
		if (!$this->_memcache) $this->initMemcache();

		if ($this->_memcache) {
			return $this->_memcache->flush();
		} else {
			return FALSE;
		}

	}//end clearAll()


	/**
	 * Cleans up expired entries.
	 *
	 * Assumed that memcache does this for us, so nothing to be done here.
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
	* Removes all cache entries related to this assetid
	*
	* @param array	$assetids	id of the the asset that owns the cache file
	*
	* @return void
	* @access public
	*/
	function clear($assetids)
	{
		if (!$this->_memcache) $this->initMemcache();
		
		// For each assetid, fetch the asset index and delete everything we find. Including the index.
		foreach ($assetids as $assetid) {
			$assetid_hits = $this->_memcache->get(SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX.$assetid, FALSE);
			if ($assetid_hits) {
				foreach ($assetid_hits as $key) {
					$this->_memcache->delete($key);
				}
				$this->_memcache->delete(SQ_CACHE_MEMCACHE_ASSET_INDEX_PREFIX.$assetid);
			}
		}
		

	}//end clear()
	
	
	/**
	* Deletes caches of assets that were cached when a certain URL was requested
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
		if (!$this->_memcache) $this->initMemcache();
		
		// Fetch the URL index and delete anything we find. Including the index.
		$index_key = SQ_CACHE_MEMCACHE_URL_INDEX_PREFIX.md5($url);
		$url_hits = $this->_memcache->get($index_key);
		
		if ($url_hits) {
			foreach ($url_hits as $key) {
				$this->_memcache->delete($key);
			}
			$this->_memcache->delete($index_key);
		}
		
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
	

	/**
	* Failure Callback function for Memcache::addServer
	* 
	* @param $hostname
	* @param $port
	* 
	* @return void
	* @access public
	*/
	function failureCallback($hostname, $port)
	{
		log_error(get_class()." failure communicating with $hostname:$port", E_USER_WARNING);

	}//end failureCallback()


	/**
	* Returns TRUE if the cache storge configuration is valid
	*
	* @return boolean
	* @access public
	*/
	public function isValid()
	{
		// If the cache storage object can be initialised, its all good
		return $this->initMemcache();

	}//end isValid()


}//end class

?>
