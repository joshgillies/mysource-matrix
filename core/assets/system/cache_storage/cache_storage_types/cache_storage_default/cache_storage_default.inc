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
 * $Id: cache_storage_default.inc,v 1.10 2012/10/16 22:37:50 ewang Exp $
 *
 */

require_once SQ_CORE_PACKAGE_PATH.'/system/cache_storage/cache_storage/cache_storage.inc';

/**
 * Cache_Storage_Default
 *
 * Cache storage using the database and filesystem. This is the pre-3.24.1
 * incumbent storage method, and remains the default.
 *
 * @author  James Hurst <james@riverratrecords.com>
 * @author  Luke Wright <lwright@squiz.net>
 * @version $Revision: 1.10 $
 * @package MySource_Matrix_Packages
 * @subpackage __core__
 */
class Cache_Storage_Default extends Cache_Storage
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
	 * @param string     $cache_key  the cache key for this entry
	 * @param string     $perm_key   the permissions key for this entry
	 * @param string    	$url		the url for this entry
	 * @param integer    $assetid    the asset id for this entry
	 * @param string		$data		the cache contents
	 * @param integer	$expiry		the expiry time in seconds
	 *
	 * @return boolean
	 * @access public
	 */
	function store($cache_key, $perm_key, $url, $assetid, $data, $expiry)
	{
		// work out the file path
		$group = $this->_getAssetHash($cache_key).$this->_getAssetHash($perm_key);
		$file_name = md5($cache_key.':'.$perm_key.':'.$url);
		$file_path = $group.'/'.$file_name;

		// replace/update existing cache entry
		// warning: will NOT cause duplicate primary key db error
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (!is_null($cached)) {
			$this->_updateCache($cache_key, $perm_key, $url, $expiry, $file_path, $data);
			return TRUE;
		}

		// cache the content file
		$old_umask = umask(0);
		if (!is_dir(SQ_CACHE_PATH.'/'.$group)) {
			$status = mkdir(SQ_CACHE_PATH.'/'.$group, 0755);
			clearstatcache();
			if (!$status) {
				trigger_localised_error('CORE0245', sprintf(translate('Unable to create cache directory "%s"'), SQ_CACHE_PATH.'/'.$group), E_USER_WARNING)
				return FALSE;
			}
		}
		$status = string_to_file($data, SQ_CACHE_PATH.'/'.$file_path);
		if (!$status) {
			trigger_localised_error('CORE0243', sprintf(translate('Unable to write to cache file "%s"'), SQ_CACHE_PATH.'/'.$file_path), E_USER_WARNING)
			return FALSE;
		}
		umask($old_umask);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$error = FALSE;
		// start a transaction to prevent reinsert due to concurrency issues
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (is_null($cached)) {
			try {
				$bind_vars = Array(
					'cache_key' => $cache_key,
					'perm_key'  => $perm_key,
					'url'       => $url,
					'assetid'   => $assetid,
					'expires'   => ts_iso8601($expiry),
					'path'      => $file_path,
				);
				MatrixDAL::executeQuery('cache_manager', 'insertCacheEntry', $bind_vars);
			} catch (DALException $e) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				// if there is an error, check if it is already cached
				$cached = $this->_getCache($cache_key, $perm_key, $url);
				if (is_null($cached)) {
					throw new Exception('Unable to cache asset ID # '.$assetid.' due to database error: '.$e->getMessage());
					return FALSE;
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end store()


	/**
	 * Loads the contents from a cache file for the specified asset
	 * and unique key
	 *
	 * returns FALSE if no cache version exist
	 *
	 * @param string     $cache_key  the cache key for this entry
	 * @param string     $perm_key   the permissions key for this entry
	 * @param string    	$url		the url for this entry
	 * @param integer    $assetid    the asset id for this entry
	 *
	 * @return string
	 * @access public
	 */
	function read($cache_key, $perm_key, $url, $assetid)
	{
		// check expiry time if entry already exists
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (!is_null($cached)) {
			// expired
			if (time() > strtotime($cached['expires'])) {
				$this->_deleteCache($cache_key, $perm_key, $url);
				return FALSE;
			} else {
				if (file_exists(SQ_CACHE_PATH.'/'.$cached['path'])) {
					$contents = file_to_string(SQ_CACHE_PATH.'/'.$cached['path']);
					if ($contents === FALSE) {
						// if we cannot read from the cache file, remove this cache entry
						$this->_deleteCache($cache_key, $perm_key, $url);
						trigger_localised_error('CORE0244', sprintf(translate('Unable to read from cache file "%s"'), SQ_CACHE_PATH.'/'.$cached['path']), E_USER_WARNING)
						return FALSE;
					}
					return $contents;
				} else {
					// if we cannot find this cache file, remove this cache entry
					$this->_deleteCache($cache_key, $perm_key, $url);
					return FALSE;
				}
			}
		}
		return FALSE;

	}//end read()


	/**
	 * Removes all cache
	 *
	 * @return void
	 * @access public
	 */
	function clearAll()
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			MatrixDAL::executeQuery('cache_manager', 'clearAllCache');
		} catch (DALException $e) {
			throw new Exception ('Unable to purge cache due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end clearAll()


	/**
	 * Returns true if this storage method can clear cache for specific assetids.
	 *
	 * @return	bool
	 * @access	public
	 */
	function canClearByAssetid()
	{
		return TRUE;

	}//end canClearByAssetid()


	/**
	 * Removes all cache entries related to this assetid
	 *
	 * @param array  $assetids   id of the the asset that owns the cache file
	 *
	 * @return void
	 * @access public
	 */
	function clear($assetids)
	{
		if (!empty($assetids)) {

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// break up the assets into chunks of 1000 so that oracle does not complain
			//$in_clauses = Array();
			try {
				$result = Array();
				foreach (array_chunk($assetids, 999) as $chunk) {
					$bind_vars = Array('assetids' => &$chunk);
					MatrixDAL::executeQuery('cache_manager', 'deleteCacheEntriesByAsset', $bind_vars);
				}
			} catch (DALException $e) {
				throw new Exception('Unable to delete cache entries due to database error: '.$e->getMessage());
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

	}//end clear()


	/**
	* Cleans up expired entries.
	*
	* Delete any cache files on the filesystem that have expired, then remove
    * the cache entries from the table if we managed to clear all cache files.
    * We abort if a file can't be deleted so all the expired DB entries will remain until the next successful run.
	*
	* For this to work, the Cron run script needs to run as the Apache user.
	* It normally creates cache bucket directories with write access only to
	* itself, and this write access to the directory is required to delete
	* files.
	*
	* @return void
	* @access public
	*/
	function cleanExpiredEntries()
	{
		$bind_vars = Array('now' => ts_iso8601(time()));

		try {
			$paths  = MatrixDAL::executeAssoc('cache_storage_default', 'selectExpiredEntryPaths', 0, $bind_vars);
		} catch (Exception $e) {
			trigger_error('Unable to select expired cache entry paths due to database error: '.$e->getMessage(), E_USER_WARNING);
			return FALSE;
		}

		foreach ($paths as $k => $path) {
			$cache_file = SQ_CACHE_PATH.'/'.$path;
			if (is_file($cache_file) === TRUE) {
				$result = unlink($cache_file);
				if ($result !== TRUE) {
					trigger_error('Unable to remove expired cache file "'.$cache_file.'"', E_USER_WARNING);
					return FALSE;
				}
			}
		}

		try {
			MatrixDAL::executeQuery('cache_storage_default', 'deleteExpiredEntries', $bind_vars);
		} catch (Exception $e) {
			trigger_error('Unable to delete expired cache entries due to database error: '.$e->getMessage(), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end cleanExpiredEntries()


	/**
	 * Returns an array of file paths for clear cache HIPO job
	 *
	 * @param array  $assetids   id of the the asset that owns the cache file
	 *
	 * @return array
	 * @access public
	 */
	function getFilePaths($assetids)
	{
		if (!empty($assetids)) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');

			// break up the assets into chunks of 1000 so that oracle does not complain
			// $in_clauses = Array();
			$paths = Array();
			foreach (array_chunk($assetids, 999) as $chunk) {
				$bind_vars = Array('assetids' => &$chunk);
				$result = MatrixDAL::executeAll('cache_manager', 'getCacheFilePathsByAsset', $bind_vars);
				if (!empty($result)) {
					foreach($result as $row) {
						if (isset($row['path']) && !isset($paths[$row['path']])) {
							$paths[$row['path']] = Array('path' => $row['path']);
						}
					}//end foreach
				}//end if
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return array_values($paths);
		}

	}//end getFilePaths()


	/**
	 * Returns an array of all file paths for clear cache HIPO job
	 *
	 * @param string $option the option of the result
	 *
	 * @return array
	 * @access public
	 */
	function getAllFilePaths($option='')
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');

		try {
			if ($option == 'col') {
				$result = MatrixDAL::executeAssoc('cache_manager', 'getAllCacheFilePaths', 0);
			} else {
				$result = MatrixDAL::executeAll('cache_manager', 'getAllCacheFilePaths');
			}
		} catch (DALException $e) {
			throw new Exception ('Unable to get all cache file paths due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $result;

	}//end getAllFilePaths()



	/**
	 * Returns the expiry time and file path of a cache entry
	 *
	 * @param string $cache_key  the cache key for this entry
	 * @param string $perm_key   the permission key for this entry
	 * @param string $url        the url identifier
	 *
	 * @return mixed array|NULL
	 * @access private
	 */
	function _getCache($cache_key, $perm_key, $url)
	{
		// to get rid of replication lag if the system has multiple db
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');

		try {
			$bind_vars = Array(
				'cache_key' => $cache_key,
				'perm_key'  => $perm_key,
				'url'       => $url,
			);
			$result = MatrixDAL::executeAll('cache_manager', 'getCacheExpiryAndFilePath', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to get cache entries due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return array_shift($result);

	}//end _getCache()


	/**
	 * Deletes an expired entry from the cache table
	 *
	 * @param string $cache_key  the cache key for this entry
	 * @param string $perm_key   the permission key for this entry
	 * @param string $url        the url identifier
	 *
	 * @return void
	 * @access private
	 */
	function _deleteCache($cache_key, $perm_key, $url)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars = Array(
				'cache_key' => $cache_key,
				'perm_key'  => $perm_key,
				'url'       => $url,
			);
			MatrixDAL::executeQuery('cache_manager', 'deleteCacheEntry', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to delete cache entries due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end _deleteCache()


	/**
	 * Extends the expiry time (and updates the cache file)
	 *
	 * @param string $cache_key          the cache key for this entry
	 * @param string $perm_key           the permission key for this entry
	 * @param string $url                the url identifier
	 * @param string $expiry_sql         the updated expiry time
	 * @param string $file_path          the file path to the cached content
	 * @param array  $updated_content    the updated content
	 *
	 * @return void
	 * @access private
	 */
	function _updateCache($cache_key, $perm_key, $url, $expiry, $file_path='', $updated_content='')
	{
		// update the expiry time
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars = Array(
				'expires'   => ts_iso8601($expiry),
				'cache_key' => $cache_key,
				'perm_key'  => $perm_key,
				'url'       => $url,
			);
			MatrixDAL::executeQuery('cache_manager', 'updateCacheExpiry', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to update cache expiry time due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// update the content file
		if ($file_path) {
			$old_umask = umask(0);
			$status = string_to_file($updated_content, SQ_CACHE_PATH.'/'.$file_path);
			if (!$status) {
				trigger_localised_error('CORE0243', sprintf(translate('Unable to write to cache file "%s"'), SQ_CACHE_PATH.'/'.$file_path), E_USER_WARNING)
				return FALSE;
			}
			umask($old_umask);
		}

	}//end _updateCache()


	/**
	 * Return the hash of and asset between 0 and num_cache_dirs
	 *
	 * @param string $assetid    the assetid to create the hash from
	 *
	 * @return string
	 * @access public
	 */
	function _getAssetHash($assetid=0)
	{
		$cache_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

		$assetid = trim($assetid);
		$hash = 0;
		$bucketsize = $cache_manager->attr('num_cache_dirs');
		$len = strlen($assetid);
		for ($i = 0; $i < $len; $i++) {
			if (!is_numeric($assetid{$i})) {
				$hash += ord($assetid{$i});
			} else {
				$hash += (int) $assetid{$i};
			}
		}
		$hash = $hash % $bucketsize;

		while (strlen($hash) != strlen($bucketsize)) {
			$hash = '0'.$hash;
		}
		return $hash;

	}//end getAssetHash()


	/**
	 * Deletes caches of assets that were cached when a certain URL was requested
	 *
	 * This includes caches of nested content that were generated by reaching
	 * the passed URL, not just the top-level asset.
	 *
	 * For Default cache storage, we search for the passed URL in the sq_cache
	 * table, delete the cache files on disc, then remove the DB entries. An
	 * exception is thrown if the DB query fails.
	 *
	 * @param string $url The url to delete cache entries for
	 *
	 * @access public
	 */
	function clearCachedAssetsByUrl($url)
	{
		if (empty($url)) {
			return;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');

		$bind_vars = Array(
						'url1'	=> $url,		// exact match for:	url.com
						'url2'	=> $url.'/',	// exact match for:	url.com/
						'url3'	=> $url.'?%',	// like match for:	url.com?%
						'url4'	=> $url.'/?%',	// like match for:	url.com/?%
					 );
		$cached_assets = MatrixDAL::executeAll('cache_manager', 'getCachedAssetByUrl', $bind_vars);

		if (!empty($cached_assets)) {
			// Remove the content file for each assetid
			foreach ($cached_assets as $cached_asset) {
				if (isset($cached_asset['path'])) {
					$asset_cache_path = SQ_CACHE_PATH.'/'.$cached_asset['path'];
					if (is_file($asset_cache_path)) {
						if (!unlink($asset_cache_path)) {
							log_dump('Failed to delete cache file: '.$asset_cache_path);
						}//end if
					}//end if
				}//end if
			}//end foreach

			// Purge assetids from cache table based on url
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			try {
				$delete_bind_vars = Array(
										'url1'	=> $url,		// exact match for:	url.com
										'url2'	=> $url.'/',	// exact match for:	url.com/
										'url3'	=> $url.'?%',	// like match for:	url.com?%
										'url4'	=> $url.'/?%',	// like match for:	url.com/?%
									);
				MatrixDAL::executeQuery('cache_manager', 'deleteCachedAssetByUrl', $delete_bind_vars);
			} catch (DALException $e) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				throw new Exception('Unable to delete cache entries due to database error: '.$e->getMessage());
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}

}//end class

?>
