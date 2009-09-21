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
 * $Id: cache_storage_memcache.inc,v 1.5.2.2 2009/09/21 03:47:58 lwright Exp $
 *
 */

require_once SQ_CORE_PACKAGE_PATH.'/system/cache_storage/cache_storage/cache_storage.inc';

/**
 * Cache_Storage_Memcache
 *
 * Cache storage using memcache.
 *
 * This cache storage method will only be available if the Memcache PECL module
 * is available.
 *
 * There are limitations to using this storage method: clearing cache by asset
 * ID or type will not be available. This is because of the way memcache's keys
 * work - they are not easily deletable via a wildcard or similar means.
 *
 * @author  James Hurst <james@riverratrecords.com>
 * @author  Luke Wright <lwright@squiz.net>
 * @version $Revision: 1.5.2.2 $
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
		assert_true(extension_loaded('memcache'), 'Cannot use Memcache Cache Storage; it requires the memcache PECL extension installed within PHP, which is not installed');
		assert_true(file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc'), 'Cannot use Memcache Cache Storage; the Memcache configuration file is not set');

		$memcache_conf = require(SQ_DATA_PATH.'/private/conf/memcache.inc');
		$hosts =& $memcache_conf['hosts'];
		$services =& $memcache_conf['services'];

		assert_true(count($hosts) > 0, 'Cannot use Memcache Cache Storage; no hosts are defined in the Memcache configuration file');
		assert_true(array_key_exists('cache_manager', $services) === TRUE, 'Cannot use Memcache Cache Storage; no Memcache hosts are assigned to cache manager');
		assert_true(count($services['cache_manager']) > 0, 'Cannot use Memcache Cache Storage; no Memcache hosts are assigned to cache manager');

		// If PHP has the memcache module installed, instantiate it and try to load some config.
		$this->_memcache = new Memcache;

		foreach ($services['cache_manager'] as $host_key => $weight) {
			assert_true(array_key_exists($host_key, $hosts) === TRUE, 'Cannot use Memcache Cache Storage; host key "'.$host_key.'" assigned for use for cache manager but not defined as a host');
			$host = $hosts[$host_key];
			$this->_memcache->addServer($host['host'], $host['port'], $host['persistent'], $weight, $host['timeout'], $host['retry_interval'], $host['status']);
		}

		$this->_memcache->setCompressThreshold($memcache_conf['compression_threshold'], $memcache_conf['compression_min_saving']);

		parent::__construct($assetid);

	}//end constructor


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
			if (file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc')) {
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
		if ($this->_memcache) {
			$url = md5($url);
			$ok = $this->_memcache->set("contentcache:$cache_key|$perm_key|$url|$assetid", $data, FALSE, $expiry - time());
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
		if ($this->_memcache) {
			$url = md5($url);
			return $this->_memcache->get("contentcache:$cache_key|$perm_key|$url|$assetid", FALSE);
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
		if ($this->_memcache) {
			return $this->_memcache->flush();
		} else {
			return FALSE;
		}

	}//end clearAll()


	/**
	 * Returns true if this storage method can clear cache for specific assetids.
	 *
	 * @return bool
	 * @access public
	 */
	function canClearByAssetid()
	{
		return FALSE;

	}//end canClearByAssetid()


	/**
	* Removes all cache entries related to this assetid
	*
	* Unused by this storage method - limitations in the key storage of memcache
	* mean that there is no guaranteed way to delete by asset ID or type.
	*
	* @param array	$assetids	id of the the asset that owns the cache file
	*
	* @return void
	* @access public
	*/
	function clear($assetids)
	{
		return FALSE;

	}//end clear()


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
