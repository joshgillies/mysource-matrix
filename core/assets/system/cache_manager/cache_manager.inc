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
* $Id: cache_manager.inc,v 1.74 2009/02/24 04:23:05 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

/**
* Cache Manager
*
* Manages the configuration for Caching of assets
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.74 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Return FALSE always because we don't want to be able to clone a cache manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Loads the contents from a cache file for the specified asset
	* and unique key
	*
	* A return of boolean FALSE should be interpreted as a cache MISS. This can
	* happen if:
	* - the cache is disabled (either globally or for that asset type), or
	* - the cache file does not exist on the file system, or is not readable, or
	* - the cache file exists, but has expired.
	*
	* A return of '' (empty string) should be interpreted as a cache HIT where
	* the cached result is actually an empty string. Therefore, when checking
	* for a cache MISS, you should directly compare to FALSE using "===".
	*
	* @param string		$assetid	id of the the asset that owns the cache file
	* @param string		$asset_type	type_code of that asset
	* @param string		$cache_key	the cache key for this entry
	* @param boolean	$use_url	whether to use the current url
	*
	* @return mixed string|boolean
	* @access public
	*/
	function loadFromCache($assetid, $asset_type, $cache_key='', $use_url=TRUE)
	{
		// this will not check specifically set root node options
		// if any root node specific options are configured, it will return TRUE blindly and the DB will be checked for a cache entry
		if (!$this->cacheEnabled($asset_type)) return FALSE;

		$perm_key = $this->_getPermKey($assetid, $cache_key);
		if (empty($perm_key)) return FALSE;

		if ($use_url) {
			$url = current_url();
			// append the query string (if any)
			if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
				$url = $this->_formUniqueUrl($url);
			}
		} else {
			$url = $assetid;
		}
		$cache_key = md5($cache_key.$assetid);

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
						trigger_localised_error('CORE0244', E_USER_WARNING, SQ_CACHE_PATH.'/'.$cached['path']);
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

	}//end loadFromCache()


	/**
	* Saves the specified contents to the cache, which will be identified
	* by the specfied asset and key/url
	*
	* If cache entry already exists, it will be overwritten with the new contents
	* and expiry time
	*
	* @param string		$assetid			id of the the asset that owns the cache file
	* @param string		$asset_type			type_code of that asset
	* @param string		$cache_key			the cache key for this entry
	* @param string		$contents			the contents to save to cache
	* @param boolean	$use_url			whether to use the current url
	* @param int		$expiry_override	override expiry length in seconds
	*
	* @return boolean
	* @access public
	*/
	function saveToCache($assetid, $asset_type, $cache_key, $contents, $use_url=TRUE, $expiry_override='')
	{
		if (!$this->cacheEnabled($asset_type, $assetid)) return FALSE;

		$perm_key = $this->_getPermKey($assetid, $cache_key);
		if (empty($perm_key)) return FALSE;

		if ($use_url) {
			$url = current_url();
			// append the query string (if any)
			if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
				$url = $this->_formUniqueUrl($url);
			}
		} else {
			$url = $assetid;
		}
		$cache_key = md5($cache_key.$assetid);
		// work out the file path and expiry time
		$group = $this->getAssetHash($cache_key).$this->getAssetHash($perm_key);
		$file_name = md5($cache_key.':'.$perm_key.':'.$url);
		$file_path = $group.'/'.$file_name;

		$now = time();
		if (is_int($expiry_override) && ($expiry_override >= 0) && ($expiry_override + $now > $now)) {
			$expiry = $expiry_override + $now;
		} else {
			$expiry = $now + $this->getExpiry($asset_type, $assetid);
		}

		// replace/update existing cache entry
		// warning: will NOT cause duplicate primary key db error
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (!is_null($cached)) {
			$this->_updateCache($cache_key, $perm_key, $url, $expiry, $file_path, $contents);
			return TRUE;
		}

		// cache the content file
		$old_umask = umask(0);
		if (!is_dir(SQ_CACHE_PATH.'/'.$group)) {
			$status = mkdir(SQ_CACHE_PATH.'/'.$group, 0755);
			clearstatcache();
			if (!$status) {
				trigger_localised_error('CORE0245', E_USER_WARNING, SQ_CACHE_PATH.'/'.$group);
				return FALSE;
			}
		}
		$status = string_to_file($contents, SQ_CACHE_PATH.'/'.$file_path);
		if (!$status) {
			trigger_localised_error('CORE0243', E_USER_WARNING, SQ_CACHE_PATH.'/'.$file_path);
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
								'cache_key'	=> $cache_key,
								'perm_key'	=> $perm_key,
								'url'		=> $url,
								'assetid'	=> $assetid,
								'expires'	=> ts_iso8601($expiry),
								'path'		=> $file_path,
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

	}//end saveToCache()


	/**
	* Saves the specified contents to the cache, which will be identified
	* by the specfied asset and key/url
	*
	* If cache entry already exists, it will be overwritten with the new contents
	* and expiry time
	*
	* @param string	$url	id of the the asset that owns the cache file
	*
	* @return boolean
	* @access public
	*/
	function _formUniqueUrl($url)
	{
		// we don't want this query vars for cache key, e.g. SQ_ACTION=login
		$query_vars=Array();
		parse_str($_SERVER['QUERY_STRING'], $query_vars);
		$mute_list = array_intersect(array_keys($query_vars), Array('SQ_BACKEND_PAGE', 'SQ_ACTION', 'SQ_DESIGN_NAME'));
		foreach ($mute_list as $name) {
			unset($query_vars[$name]);
		}

		if (!empty($query_vars)) {
			$new_query_str = '';
			foreach ($query_vars as $key => $val) {
				if (is_array($val)) {
					foreach($val as $mkey => $mval) {
						$new_query_str .= $key.'['.$mkey.']='.$mval.'&';
					}
				} else {
					$new_query_str .= $key.'='.$val.'&';
				}
			}
			$url .= '?'.rtrim($new_query_str, '&');
		}

		return $url;

	}//end _formUniqueUrl()


	/**
	* Removes all cache entries related to this assetid
	*
	* @param array	$assetids	id of the the asset that owns the cache file
	*
	* @return void
	* @access public
	*/
	function clearCache($assetids)
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

	}//end clearCache()


	/**
	* Returns an array of file paths for clear cache HIPO job
	*
	* @param array	$assetids	id of the the asset that owns the cache file
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
			$result = Array();
			foreach (array_chunk($assetids, 999) as $chunk) {
				$bind_vars = Array('assetids' => &$chunk);
				$result = array_merge($result, MatrixDAL::executeAll('cache_manager', 'getCacheFilePathsByAsset', $bind_vars));
				$result = array_unique($result);
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $result;
		}

	}//end getFilePaths()


	/**
	* Removes all cache
	*
	* @return void
	* @access public
	*/
	function clearAllCache()
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

	}//end clearAllCache()


	/**
	* Returns an array of all file paths for clear cache HIPO job
	*
	* @param string	$option	the option of the result
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
	* Return the key suffix that represents a cached object based on cache levels
	*
	* Takes logged in status, permissions and user groups in account
	* Also takes the type of asset being cached into account (standard or listing based)
	*
	* @param string	$assetid	the id of asset that identifies this cache entry
	* @param string	$cache_key	the cache key passed by the asset being cached
	*
	* @return string
	* @access private
	*/
	function _getPermKey($assetid, $cache_key)
	{
		$perm_key = '';
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		if (is_null($current_user) || ($current_user instanceof Public_User)) {
			// if your not logged in and public level caching is on - you get the public copy
			if ($this->attr('public_caching')) {
				if (is_null($current_user)) {
					$perm_key = 'public';
				} else {
					// get the group that this public user is in
					// for which satisfies the security restrictions
					$groups = array_values($current_user->getUserGroups());
					sort($groups);
					$perm_key = 'public'.implode('|',$groups);
				}
			}
		} else {
			if (empty($cache_key)) {
				// if permission level caching is on - check read/write/admin access for this asset
				// if the key is blank (representing a non list style asset)
				if ($this->attr('permission_caching')) {
					$groups = array_keys($current_user->getGroups());
					$userids = Array((string) $current_user->id) + $groups;

					// note that we dont use the cache in rollback view mode so we dont have
					// to run this query through the rollback functions
					try {
						$bind_vars = Array(
										'assetid'	=> $assetid,
										'userids'	=> $userids,
									 );
						$permission = MatrixDAL::executeOne('cache_manager', 'getHighestPermissionForUsers', $bind_vars);
					} catch (DALException $e) {
						throw new Exception('Unable to get maximum permission level of asset ID #'.$assetid.' due to database error: '.$e->getMessage());
					}

					switch ($permission) {
						case SQ_PERMISSION_READ  :
							$perm_key .= ':read';
						break;
						case SQ_PERMISSION_WRITE :
							$perm_key .= ':write';
						break;
						case SQ_PERMISSION_ADMIN :
							$perm_key .= ':admin';
						break;
					}
				}
			} else {
				// if group level caching is on - check the groups the user is in if they are logged in
				// and if user satisfies the group security restrictions
				if ($this->attr('group_caching')) {
					$groups = array_values($current_user->getUserGroups());
					sort($groups);
					$perm_key = implode('|', $groups);
				}
			}
		}//end else - if logged in
		$perm_key = (empty($perm_key)) ? '' : md5($perm_key);
		return $perm_key;

	}//end _getPermKey()


	/**
	* Return the hash of and asset between 0 and num_cache_dirs
	*
	* @param string	$assetid	the assetid to create the hash from
	*
	* @return string
	* @access public
	*/
	function getAssetHash($assetid=0)
	{
		$assetid = trim($assetid);
		do {
			$hash = 0;
			$len = strlen($assetid);
			for ($i = 0; $i < $len; $i++) {
				if ((int) $assetid{$i} != $assetid{$i}) {
					$hash += ord($assetid{$i});
				} else {
					$hash += (int) $assetid{$i};
				}
			}
			$assetid = (string) $hash;
		} while ($hash > $this->attr('num_cache_dirs'));

		while (strlen($hash) != strlen($this->attr('num_cache_dirs'))) {
			$hash = '0'.$hash;
		}
		return $hash;

	}//end getAssetHash()


	/**
	* Check if caching is enabled, generally, for the current root URL and for the specfied type code or assetid
	*
	* If no type code or assetid is specified this function will return the system wide setting
	*
	* @param string	$type_code	the type_code to check to see if caching is enabled
	* @param string	$assetid	the assetid to check to see if caching is enabled
	*
	* @return boolean
	* @access public
	*/
	function cacheEnabled($type_code='', $assetid='')
	{
		// first check some obvious things
		if (SQ_ROLLBACK_VIEW) return FALSE;
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_NOCACHE_SUFFIX) {
			return FALSE;
		}
		if (array_get_index($_GET, 'SQ_ACTION') == 'diff') {
			return FALSE;
		}

		// check if the current URL is excluded
		$current_url = current_url(FALSE, TRUE);
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$current_root_url = '';
		foreach ($root_urls as $url) {
			if (0 === strpos($current_url, $url)) {
				if (strlen($url) > strlen($current_root_url)) {
					$current_root_url = $url;
				}
			}
		}
		if (in_array($current_root_url, $this->attr('non_cachable_urls'))) {
			return FALSE;
		}

		// check type code and root node specific settings
		// these take priority over the global setting
		// in order for a cache entry to be saved and therefore loaded, a valid perm key must be attained
		// this perm key is only attainable if a cache level is specified
		$specific = $this->_specificCacheManagementSettings($type_code, $assetid, 'enabled');
		if ($specific !== FALSE) {
			return $specific;
		}

		return $this->attr('enabled');

	}//end cacheEnabled()


	/**
	* Returns the expiry time for the specified type code or assetid
	*
	* If the expiry value for the provided type code or assetid has not been customised,
	* then the system wide expiry is returned
	*
	* @param string	$type_code	the type code of the wanted expiry value
	* @param string	$assetid	the assetid of the wanted expiry value
	*
	* @return int
	* @access public
	*/
	function getExpiry($type_code='', $assetid='')
	{
		$specific = $this->_specificCacheManagementSettings($type_code, $assetid, 'expiry');
		if ($specific !== FALSE) {
			return $specific;
		}

		return $this->attr('expiry');

	}//end getExpiry()


	/**
	* Returns the browser cache expiry time for the specified type code or assetid
	*
	* If the expiry value for provided type code or assetid has not been customised,
	* then the system wide browser cache expiry is returned
	*
	* @param string $type_code 	the type code of the requested browser cache expiry value
	* @param string $assetid	the assetid of the requested browser cache expiry value
	*
	* @return int
	* @access public
	*/
	function getBrowserCacheExpiry($type_code='', $assetid='')
	{
		$specific = $this->_specificCacheManagementSettings($type_code, $assetid, 'browser_cache_expiry');
		if ($specific !== FALSE) {
			return $specific;
		}

		return $this->attr('browser_cache_expiry');

	}//end getBrowserCacheExpiry()


	/**
	* Returns specified setting for the provided type code or assetid
	*
	* Type code specific settings take priority over root node configured settings
	*
	* @param string	$type_code	the type code of the wanted value
	* @param string	$assetid	the assetid of the wanted value
	* @param string	$value		the value required
	*
	* @return mixed int|boolean
	* @access private
	*/
	function _specificCacheManagementSettings($type_code, $assetid, $value)
	{
		// check for type-code-specific settings
		$type_codes = $this->attr('type_codes');
		if (isset($type_codes[$type_code][$value])) {
			return $type_codes[$type_code][$value];
		}

		$root_nodes = $this->attr('root_nodes');

		// an empty assetid indicates the caller does not want to perform the search for possible tree locations
		// if there are configured root node specific options return TRUE blindly
		if (empty($assetid) && !empty($root_nodes)) {
			return TRUE;
		} else if (!empty($root_nodes)) {
			// check for root node specific settings
			if (isset($root_nodes[$assetid][$value])) {
				return $root_nodes[$assetid][$value];
			} else {
				// there are possible tree locations set so search for them because an exact match was not found
				$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($assetid));
				foreach ($parents as $parentid) {
					// check that this entry actually manages child entries
					if (isset($root_nodes[$parentid]['all_underneath']) && $root_nodes[$parentid]['all_underneath']) {
						// check the value of the option that was asked for
						if (isset($root_nodes[$parentid][$value])) {
							return $root_nodes[$parentid][$value];
						}
					}
				}
			}
		}

		return FALSE;

	}//end _specificCacheManagementSettings()


	/**
	* Returns the expiry time and file path of a cache entry
	*
	* @param string	$cache_key	the cache key for this entry
	* @param string	$perm_key	the permission key for this entry
	* @param string	$url		the url identifier
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
						'cache_key'	=> $cache_key,
						'perm_key'	=> $perm_key,
						'url'		=> $url,
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
	* @param string	$cache_key	the cache key for this entry
	* @param string	$perm_key	the permission key for this entry
	* @param string	$url		the url identifier
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
						'cache_key'	=> $cache_key,
						'perm_key'	=> $perm_key,
						'url'		=> $url,
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
	* @param string	$cache_key			the cache key for this entry
	* @param string	$perm_key			the permission key for this entry
	* @param string	$url				the url identifier
	* @param string	$expiry_sql			the updated expiry time
	* @param string	$file_path			the file path to the cached content
	* @param array	$updated_content	the updated content
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
						'expires'	=> ts_iso8601($expiry),
						'cache_key'	=> $cache_key,
						'perm_key'	=> $perm_key,
						'url'		=> $url,
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
				trigger_localised_error('CORE0243', E_USER_WARNING, SQ_CACHE_PATH.'/'.$file_path);
				return FALSE;
			}
			umask($old_umask);
		}

	}//end _updateCache()


}//end class

?>
