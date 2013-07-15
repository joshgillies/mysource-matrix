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
* $Id: cache_manager.inc,v 1.98 2013/07/15 05:08:36 ewang Exp $
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
* @version $Revision: 1.98 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cache_Manager extends Asset
{


	/**
	* Variable containing the loaded Cache Storage asset
	*
	* @var Cache_Storage
	*/
	protected $_storage;
	
	/**
	* Ignore current url, use assetid as cache key to create a global cache enrty
	*
	* @var boolean
	*/
	public $global_cache = FALSE;


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

		$storage_type = $this->attr('cache_storage_type');
		$this->_loadCacheStorage($storage_type);

	}//end constructor
	
	
	/**
	* Wakeup.
	*
	* @access public
	*/
	function __wakeup() 
	{
		$storage_type = $this->attr('cache_storage_type');
		$this->_loadCacheStorage($storage_type);
		
	}//end wakeup


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
		
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_RECACHE_SUFFIX) {
			return FALSE;
		}

		// this will not check specifically set root node options
		// if any root node specific options are configured, it will return TRUE blindly and the DB will be checked for a cache entry
		if (!$this->cacheEnabled($asset_type)) return FALSE;

		$perm_key = $this->_getPermKey($assetid, $cache_key);

		if (empty($perm_key)) return FALSE;

		if ($use_url && !$this->global_cache) {
			$url = current_url();
			// append the query string (if any)
			if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
				$url = $this->_formUniqueUrl($url);
			}
		} else {
			$url = $assetid;
		}
		$cache_key = md5($cache_key.$assetid);

		return $this->_storage->read($cache_key, $perm_key, $url, $assetid);

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

		if ($use_url && !$this->global_cache) {
			$url = current_url();
			// append the query string (if any)
			if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
				$url = $this->_formUniqueUrl($url);
			}
		} else {
			$url = $assetid;
		}
		$cache_key = md5($cache_key.$assetid);

		$now = time();
		if (is_int($expiry_override) && ($expiry_override >= 0) && ($expiry_override + $now > $now)) {
			$expiry = $expiry_override + $now;
		} else {
			$expiry = $now + $this->getExpiry($asset_type, $assetid);
		}
		return $this->_storage->store($cache_key, $perm_key, $url, $assetid, $contents, $expiry);

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
		$mute_list = array_intersect(array_keys($query_vars), Array('SQ_BACKEND_PAGE', 'SQ_ACTION', 'SQ_DESIGN_NAME', 'SQ_CONTEXT_NAME'));
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
		if ($this->_storage->clear($assetids) === FALSE) {
			return FALSE;
		}

		foreach ($assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_matrix_cache_cleared', $asset, NULL);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

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
		return $this->_storage->getFilePaths($assetids);

	}//end getFilePaths()


	/**
	* Can the set cache storage method clear itself when filtered by asset ID?
	*
	* Returns NULL when there is no storage mechanism yet stored. Otherwise
	* TRUE or FALSE based on the advice provided by the storage method class.
	*
	* @return boolean
	*/
	public function canClearByAssetid()
	{
		if (is_object($this->_storage)) {
			return $this->_storage->canClearByAssetid();
		} else {
			return NULL;
		}

	}//end canClearByAssetid()


	/**
	* Removes all cache
	*
	* @return void
	* @access public
	*/
	function clearAllCache()
	{
		$this->_storage->clearAll();

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
		return $this->_storage->getAllFilePaths($option);

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

		$hash = 0;
		$bucketsize = $this->attr('num_cache_dirs');
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
		if(defined('SQ_IN_PERFORMANCE_TIMING') && isset($_SESSION['SQ_PERFORMANCE_SETTING']['caching'])) {
		    if($_SESSION['SQ_PERFORMANCE_SETTING']['caching'] === 'Off') return FALSE;
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
	* Returns TRUE if cachaeable headers needs to be send (regardless of other Matrix cache settings)
	* This function doesn't actually sends the headers of course
	*
	* @return boolean
	* @access public
	*/
	function sendCacheableHeaders($type_code='', $assetid='')
	{
		// first check some obvious things
		if (SQ_ROLLBACK_VIEW) return FALSE;
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_NOCACHE_SUFFIX) {
			return FALSE;
		}
		if (array_get_index($_GET, 'SQ_ACTION') == 'diff') {
			return FALSE;
		}
		if(defined('SQ_IN_PERFORMANCE_TIMING')) return FALSE;

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
		
		if (in_array($current_root_url, $this->attr('non_cacheable_header_urls'))) {
			return FALSE;
		}
		
		// if it's non-public user and we should only send to public user on current URL, do not send header
		$is_public = $GLOBALS['SQ_SYSTEM']->user instanceof Public_User;
		if (!$is_public && !in_array($current_root_url, $this->attr('user_cacheable_header_urls'))) {
			return FALSE;
		}
		
		// check type code and root node specific settings
		// these take priority over the global setting
		$specific = $this->_specificCacheManagementSettings($type_code, $assetid, 'send_cacheable_header');
		if ($specific !== FALSE) {
			return $specific;
		}

		return SQ_CONF_SEND_CACHEABLE_HEADER;

	}//end sendCacheableHeaders()


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
				// if there are a few root nodes, we should use isUnderRootNodes for performance sake
				if(count($root_nodes) < 10) {
					$matched_roots = $GLOBALS['SQ_SYSTEM']->am->isUnderRootNodes($assetid, array_keys($root_nodes), TRUE);
				}
				else {
					$matched_roots = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($assetid));
				}
				foreach ($matched_roots as $rootid) {
					// check that this entry actually manages child entries
					if (isset($root_nodes[$rootid]['all_underneath']) && $root_nodes[$rootid]['all_underneath']) {
						// check the value of the option that was asked for
						if (isset($root_nodes[$rootid][$value])) {
							return $root_nodes[$rootid][$value];
						}
					}
				}//end foreach
			}
		}

		return FALSE;

	}//end _specificCacheManagementSettings()


	//--        CACHE STORAGE         --//


	/**
	* Get the cache storage object
	*
	* @return Cache_Storage
	*/
	public function getCacheStorage()
	{
		return $this->_storage;

	}//end getCacheStorage()


	/**
	* Load the cache storage asset for the specified storage type
	*
	* @param string	$storage_type	The type of storage requested
	*
	* @return void
	*/
	protected function _loadCacheStorage($storage_type)
	{
		$asset_type = 'cache_storage_'.strtolower($storage_type);

		if ($GLOBALS['SQ_SYSTEM']->am->installed($asset_type) === FALSE) {
			trigger_error('Unable to find "'.$storage_type.'" storage type, reverting to default', E_USER_WARNING);
			$asset_type = 'cache_storage_default';
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);

		if (eval('return '.$asset_type.'::isAvailable();') === FALSE) {
			trigger_error('The cache storage type "'.$storage_type.'" cannot be used under current conditions, reverting to default', E_USER_WARNING);
			$asset_type = 'cache_storage_default';
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);
		}

		$this->_storage = new $asset_type();

	}//end _loadCacheStorage()

	/**
	* Deletes caches of assets that were cached when a certain URL was requested
	*
	* This includes caches of nested content that were generated by reaching
	* the passed URL, not just the top-level asset.
	*
	* The actual ability or not to do this is determined by the relevant cache
	* storage method. Being able to efficiently search by URL would be a
	* pre-requisite.
	*
	* @param string $url The url to delete cache entries for
	*
	* @access public
	*/
	public function clearCachedAssetsByUrl($url)
	{
		if (empty($url)) {
			return FALSE;
		}

		return $this->_storage->clearCachedAssetsByUrl($url);

	}//end clearCachedAssetsByUrl()
	

    /**
    * Set the value for the passed variable to the passed value
    *
    * @param string $name   the name of the attribute
    * @param string $value  the new value of the attribute
    *
    * @return boolean
    * @access public
    */
    public function setAttrValue($name, $value)
    {
        $success = parent::setAttrValue($name, $value);

		// If the cache type is being set then cache storage needs to be reloaded
        if ($name === 'cache_storage_type' && $success) {
			$this->_loadCacheStorage($value);
        }
        
        return $success;        

    }//end setAttrValue()


	/**
	* Returns TRUE if cache manager is configured to send cacheable
	* headers over current protocol
	*
	* @return booleab
	* @access public
	*/
	public function cacheableHeadersEnabledForCurrentProtocol()
	{
		$protocol_setting = $this->attr('cacheale_header_protocol');
		if ($protocol_setting != 'both') {
			return current_protocol() == 'http' ? $protocol_setting == 'http' : $protocol_setting == 'https';
		}//end if

		return TRUE;

	}//end cacheableHeadersEnabledForCurrentProtocol()


	/**
	* Returns cache control level (public/private) based on current
	* protocol and cache manager settings
	*
	* @return string
	* @access public
	*/
	public function cacheControlLevel()
	{
		return current_protocol() == 'http' ? $this->attr('cache_control_http') : $this->attr('cache_control_https');
	
	}//end cacheControlLevel()


}//end class

?>
