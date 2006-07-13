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
* $Id: cache_manager.inc,v 1.34.2.11 2006/07/13 00:13:33 rong Exp $
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
* @version $Revision: 1.34.2.11 $
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
	function Cache_Manager($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

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
	* returns FALSE if no cache version exist
	*
	* @param string		$assetid	id of the the asset that owns the cache file
	* @param string		$asset_type	type_code of that asset
	* @param string		$cache_key	the cache key for this entry
	* @param boolean	$use_url	whether to use the current url
	*
	* @return string
	* @access public
	*/
	function loadFromCache($assetid, $asset_type, $cache_key='', $use_url=TRUE)
	{
		if (SQ_ROLLBACK_VIEW) return '';
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_NOCACHE_SUFFIX) {
			return '';
		}

		if (!$this->cacheEnabled($asset_type)) return '';

		$perm_key = $this->_getPermKey($assetid, $cache_key);
		if (empty($perm_key)) return '';

		if ($use_url) {
			$url = current_url();
			// append the query string (if any)
			if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
				$url .= '?'.$_SERVER['QUERY_STRING'];
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
				return '';
			} else {
				if (file_exists(SQ_CACHE_PATH.'/'.$cached['path'])) {
					$contents = file_to_string(SQ_CACHE_PATH.'/'.$cached['path']);
					if (!$contents) {
						// if we cannot read from the cache file, remove this cache entry
						$this->_deleteCache($cache_key, $perm_key, $url);
						// trigger_localised_error('CORE0244', E_USER_WARNING, SQ_CACHE_PATH.'/'.$cached['path']);
						return '';
					}
					return $contents;
				} else {
					// if we cannot find this cache file, remove this cache entry
					$this->_deleteCache($cache_key, $perm_key, $url);
					return '';
				}
			}
		}
		return '';

	}//end loadFromCache()


	/**
	* Saves the specified contents to the cache, which will be identified
	* by the specfied asset and key/url
	*
	* If cache entry already exists, it will be overwritten with the new contents
	* and expiry time
	*
	* @param string		$assetid	id of the the asset that owns the cache file
	* @param string		$asset_type	type_code of that asset
	* @param string		$cache_key	the cache key for this entry
	* @param string		$contents	the contents to save to cache
	* @param boolean	$use_url	whether to use the current url
	*
	* @return boolean
	* @access public
	*/
	function saveToCache($assetid, $asset_type, $cache_key, $contents, $use_url=TRUE)
	{
		if (SQ_ROLLBACK_VIEW) return FALSE;
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_NOCACHE_SUFFIX) {
			return FALSE;
		}

		if (!$this->cacheEnabled($asset_type)) return FALSE;

		$perm_key = $this->_getPermKey($assetid, $cache_key);
		if (empty($perm_key)) return FALSE;

		if ($use_url) {
			$url = current_url();
			// append the query string (if any)
			if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
				$url .= '?'.$_SERVER['QUERY_STRING'];
			}
		} else {
			$url = $assetid;
		}
		$cache_key = md5($cache_key.$assetid);

		// work out the file path and expiry time
		$group = $this->getAssetHash($cache_key).$this->getAssetHash($perm_key);
		$file_name = md5($cache_key.':'.$perm_key.':'.$url);
		$file_path = $group.'/'.$file_name;
		$expiry = time() + $this->getExpiry($asset_type);
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$expiry_sql = db_extras_todate($db, ts_iso8601($expiry));

		// replace/update existing cache entry
		// warning: will NOT cause duplicate primary key db error
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (!is_null($cached)) {
			$this->_updateCache($cache_key, $perm_key, $url, $expiry_sql, $file_path, $contents);
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$error = FALSE;
		// start a transaction to prevent reinsert due to concurrency issues
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (is_null($cached)) {
			$sql = 'INSERT INTO
						sq_cache
					(
						cache_key,
						perm_key,
						url,
						assetid,
						expires,
						path
					)
					VALUES
					(
						'.$db->quoteSmart($cache_key).',
						'.$db->quoteSmart($perm_key).',
						'.$db->quoteSmart($url).',
						'.$db->quoteSmart($assetid).',
						'.$expiry_sql.',
						'.$db->quoteSmart($file_path).'
					)';
			$result = $db->query($sql);
			// silent the error and check for cache entry (concurrency issue)
			if (!assert_valid_db_result($result, '', TRUE, FALSE)) {
				$error = TRUE;
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		if ($error) {
			// we can only do query here because transaction will be aborted when the assertion fails
			$cached = $this->_getCache($cache_key, $perm_key, $url);
			if (is_null($cached)) {
				// if the cache entry is empty, this is a real error
				assert_valid_db_result($result);
			}
		}

		return TRUE;

	}//end saveToCache()


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

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			foreach ($assetids as $key => $assetid) {
				$assetids[$key] = $db->quoteSmart((String)$assetid);
			}

			// break up the assets into chunks of 1000 so that oracle does not complain
			$in_clauses = Array();
			foreach (array_chunk($assetids, 999) as $chunk) {
				$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
			}

			$sql = 'DELETE FROM
						sq_cache
					WHERE
						('.implode(' OR ', $in_clauses).')';

			$result = $db->query($sql);
			assert_valid_db_result($result);
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
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			foreach ($assetids as $key => $assetid) {
				$assetids[$key] = $db->quoteSmart((String)$assetid);
			}

			// break up the assets into chunks of 1000 so that oracle does not complain
			$in_clauses = Array();
			foreach (array_chunk($assetids, 999) as $chunk) {
				$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
			}

			$sql = 'SELECT
						path
					FROM
						sq_cache
					WHERE
						('.implode(' OR ', $in_clauses).')';
			$result = $db->getAll($sql);
			assert_valid_db_result($result);
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $result;
		}

	}//end getFilePaths()


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
		$current_user =& $GLOBALS['SQ_SYSTEM']->user;
		if (is_null($current_user) || is_a($current_user, 'public_user')) {
			// if your not logged in and public level caching is on - you get the public copy
			if ($this->attr('public_caching')) {
				$perm_key = 'public';
			}
		} else {
			if (empty($cache_key)) {
				// if permission level caching is on - check read/write/admin access for this asset
				// if the key is blank (representing a non list style asset)
				if ($this->attr('permission_caching')) {
					$db =& $GLOBALS['SQ_SYSTEM']->db;

					$groups = array_keys($current_user->getGroups());
					$userids = Array($db->quote($current_user->id));
					foreach ($groups as $groupid) {
						$userids[] = $db->quote($groupid);
					}

					// note that we dont use the cache in rollback view mode so we dont have
					// to run this query through the rollback functions
					$sql = 'SELECT MAX(permission) FROM sq_vw_ast_perm
							WHERE assetid = '.$db->quote($assetid).'
							  AND userid IN ('.implode(',', $userids).')
							  AND granted = 1';

					$permission = $db->getOne($sql);
					assert_valid_db_result($permission);

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
				if ($this->attr('group_caching')) {
					$groups = array_keys($current_user->getGroups());
					sort($groups);
					$perm_key = implode('|', $groups);
				}
			}
		}//end else - if logged in

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
	* Check if caching is enabled for the specfied type code
	*
	* If no type code is specified this function will check the system wide setting
	*
	* @param string	$type_code	the type_code to check to see if caching is enabled
	*
	* @return boolean
	* @access public
	*/
	function cacheEnabled($type_code='')
	{
		$type_codes = $this->attr('type_codes');
		if (isset($type_codes[$type_code]['enabled'])) {
			return $type_codes[$type_code]['enabled'];
		}
		return $this->attr('enabled');

	}//end cacheEnabled()


	/**
	* Returns the expiry time for the specified type code.
	*
	* If the expiry value for that type code has not been customised,
	* then the default expiry is returned
	*
	* @param string	$type_code	the type code of the wanted expiry value
	*
	* @return int
	* @access public
	*/
	function getExpiry($type_code='')
	{
		$type_codes = $this->attr('type_codes');
		if (isset($type_codes[$type_code]['expiry'])) {
			return $type_codes[$type_code]['expiry'];
		}
		return $this->attr('expiry');

	}//end getExpiry()


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

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT
					expires, path
				FROM
					sq_cache
				WHERE
					cache_key = '.$db->quoteSmart($cache_key).' AND
					perm_key = '.$db->quoteSmart($perm_key).' AND
					url = '.$db->quoteSmart($url);
		$result = $db->getAll($sql);
		assert_valid_db_result($result);

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

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM
					sq_cache
				WHERE
					cache_key = '.$db->quoteSmart($cache_key).' AND
					perm_key = '.$db->quoteSmart($perm_key).' AND
					url = '.$db->quoteSmart($url);
		$result = $db->query($sql);
		assert_valid_db_result($result);

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
	function _updateCache($cache_key, $perm_key, $url, $expiry_sql, $file_path='', $updated_content='')
	{
		// update the expiry time
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'UPDATE
					sq_cache
				SET
					expires = '.$expiry_sql.'
				WHERE
					cache_key = '.$db->quoteSmart($cache_key).' AND
					perm_key = '.$db->quoteSmart($perm_key).' AND
					url = '.$db->quoteSmart($url);
		$result = $db->query($sql);
		assert_valid_db_result($result);
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
