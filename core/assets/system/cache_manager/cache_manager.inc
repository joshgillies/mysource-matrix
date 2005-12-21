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
* $Id: cache_manager.inc,v 1.21 2005/12/21 21:55:28 rong Exp $
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
* @version $Revision: 1.21 $
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
		$this->_ser_attrs = true;
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
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

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
	function _getName($short_name=false)
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
		return false;

	}//end canDelete()


	/**
	* Return FALSE always because we don't want to be able to clone a cache manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Loads the contents from a cache file for the specified asset
	* and unique key
	*
	* returns false if no cache version exist
	*
	* @param string		$assetid	id of the the asset that owns the cache file
	* @param string		$asset_type	type_code of that asset
	* @param string		$cache_key	the cache key for this entry
	* @param boolean	$use_url	whether to use the current url
	*
	* @return string
	* @access public
	*/
	function loadFromCache($assetid, $asset_type, $cache_key='', $use_url=true)
	{
		if (SQ_ROLLBACK_VIEW) return '';
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_NOCACHE_SUFFIX) {
			return '';
		}

		if (!$this->cacheEnabled($asset_type)) return false;

		$perm_key = $this->_getPermKey($assetid, $cache_key);
		if (empty($perm_key)) return '';

		// use assetid if we are not using current url as an identifier
		$url = ($use_url) ? current_url() : $assetid;
		$cache_key = md5($cache_key);

		// check expiry time if entry already exists
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (!is_null($cached)) {
			// expired
			if (time() > strtotime($cached['expires'])) {
				// TODO: last modified header
				//extend expiry
				$this->_deleteCache($cache_key, $perm_key, $url);
				return '';
			} else {
				$contents = file_to_string(SQ_CACHE_PATH.'/'.$cached['path']);
				if (!$contents && !file_exists(SQ_CACHE_PATH.'/'.$cached['path'])) {
					trigger_localised_error('CORE0244', E_USER_WARNING, SQ_CACHE_PATH.'/'.$cached['path']);
					return '';
				}
				//$contents .= ' (cached version)';
				return $contents;
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
	function saveToCache($assetid, $asset_type, $cache_key, $contents, $use_url=true)
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (basename($_SERVER['PHP_SELF']) == SQ_CONF_NOCACHE_SUFFIX) {
			return false;
		}

		if (!$this->cacheEnabled($asset_type)) return false;

		$perm_key = $this->_getPermKey($assetid, $cache_key);
		if (empty($perm_key)) return false;

		// use assetid if we are not using current url as an identifier
		$url = ($use_url) ? current_url() : $assetid;
		$cache_key = md5($cache_key);

		// work out the file path and expiry time
		$group = $this->getAssetHash($cache_key).$this->getAssetHash($perm_key);
		$file_name = md5($cache_key.':'.$perm_key.':'.$url);
		$file_path = $group.'/'.$file_name;
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$expiry = time() + $this->getExpiry($asset_type);
		$expiry_sql = db_extras_todate($db, ts_iso8601($expiry));

		// entry is not expired, this is an update
		$cached = $this->_getCache($cache_key, $perm_key, $url);
		if (!is_null($cached) && (time() < strtotime($cached['expires']))) {
			$this->_updateCache($cache_key, $perm_key, $url, $expiry_sql, $file_path, $contents);
			return true;
		}

		// cache the content file
		$old_umask = umask(0);
		if (!is_dir(SQ_CACHE_PATH.'/'.$group)) {
			$status = mkdir(SQ_CACHE_PATH.'/'.$group, 0755);
			clearstatcache();
			if (!$status) {
				trigger_localised_error('CORE0245', E_USER_WARNING, SQ_CACHE_PATH.'/'.$group);
				return false;
			}
		}
		$status = string_to_file($contents, SQ_CACHE_PATH.'/'.$file_path);
		if (!$status) {
			trigger_localised_error('CORE0243', E_USER_WARNING, SQ_CACHE_PATH.'/'.$file_path);
			return false;
		}
		umask($old_umask);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


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
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

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
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'DELETE FROM
						sq_cache
					WHERE
						assetid in ('.implode(',', $assetids).')';
			$result = $db->query($sql);
			error_log(implode(',', $assetids));
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}

	}//end clearCache()


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
		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
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
					$db = &$GLOBALS['SQ_SYSTEM']->db;

					$groups = array_keys($current_user->getGroups());
					$userids = Array($db->quote($current_user->id));
					foreach ($groups as $groupid) {
						$userids[] = $db->quote($groupid);
					}

					// note that we dont use the cache in rollback view mode so we dont have
					// to run this query through the rollback functions
					$sql = 'SELECT MAX(permission) FROM sq_ast_perm
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
		}//end if logged in

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
	* @return mixed array|null
	* @access private
	*/
	function _getCache($cache_key, $perm_key, $url)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM
					sq_cache
				WHERE
					cache_key = '.$db->quoteSmart($cache_key).' AND
					perm_key = '.$db->quoteSmart($perm_key).' AND
					url = '.$db->quoteSmart($url);
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end _deleteCache()


	/**
	* Updates the cache file and the extends the expiry time
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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
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

		// update the content file
		if ($file_path) {
			$old_umask = umask(0);
			$status = string_to_file($updated_content, SQ_CACHE_PATH.'/'.$file_path);
			if (!$status) {
				trigger_localised_error('CORE0243', E_USER_WARNING, SQ_CACHE_PATH.'/'.$file_path);
				return false;
			}
			umask($old_umask);
		}

	}//end _updateCache()


}//end class

?>
