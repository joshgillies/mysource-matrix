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
* $Id: cache_manager.inc,v 1.2 2004/08/25 02:20:56 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once 'Cache.php';

/**
* Cache Manager
*
* Manages the configuration for Caching of assets
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.2 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Cache_Manager extends Asset
{

	/**
	* The PEAR cache object
	*
	* @var Object
	*/
	var $cache = null;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cache_Manager($assetid=0)
	{
		$this->_ser_attrs = true;

		// the cache object REQUIRES a slash on the end
		$this->cache = new Cache('file', Array('cache_dir' => SQ_CACHE_PATH.'/'));
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this))  {
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
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName();


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
	* @param object Asset	&$asset		the asset that owns the cache file
	* @param string 		$key		the unique identifer for the cache file
	*
	* @return string or false if no cache version exist
	* @access public
	*/
	function loadFromCache(&$asset, $key='')
	{
		if (SQ_ROLLBACK_VIEW) return '';
		if (isset($_REQUEST['no_cache']) && $_REQUEST['no_cache']) return '';

		assert_is_a($asset, 'asset', 'Cannot load cache of non asset object');
		if (!$this->cacheEnabled($asset->type())) return '';

		$suffix = $this->_getCacheKey($asset, $key);
		if (empty($suffix)) return '';
		$assetid_code = md5($asset->id);
		$cacheid_code = $this->cache->generateID($asset->id.$key.$suffix);

		$group = $this->getAssetHash($assetid_code).$this->getAssetHash($cacheid_code);
		$contents = $this->cache->get($assetid_code.':'.$cacheid_code, $group);

		return ($contents) ? $contents : '';

	}//end loadFromCache()


	/**
	* Saves the specified contents to the cache, which will be identified
	* by the specfied asset and key
	*
	* @param object	Asset 	&$asset		the asset that identifies this cache entry
	* @param string			$key		the unique key for this cache entry
	* @param string			$contents 	the contents to save to cache
	*
	* @return boolean
	* @access public
	*/
	function saveToCache(&$asset, $key, $contents)
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (isset($_REQUEST['no_cache']) && $_REQUEST['no_cache']) return false;

		assert_is_a($asset, 'asset', 'Cannot save cache of non asset object');
		if (!$this->cacheEnabled($asset->type())) return false;

		$suffix = $this->_getCacheKey($asset, $key);
		if (empty($suffix)) return false;
		$assetid_code = md5($asset->id);
		$cacheid_code = $this->cache->generateID($asset->id.$key.$suffix);

		$group = $this->getAssetHash($assetid_code).$this->getAssetHash($cacheid_code);
		$expiry = time() + $this->getExpiry($asset->type());

		return $this->cache->save($assetid_code.':'.$cacheid_code, $contents, $expiry, $group);

	}//end saveToCache()
	
	
	/**
	* Return the key suffix that represents a cached object based on cache levels
	*
	* Takes logged in status, permissions and user groups in account
	* Also takes the type of asset being cached into account (standard or listing based)
	*
	* @param object	Asset 	&$asset		the asset that identifies this cache entry
	* @param string	$key	the cache key passed by the asset being cached
	*
	* @return string
	* @access private
	*/
	function _getCacheKey(&$asset, $key)
	{
		$cache_key = '';
		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		if (is_null($current_user)) {
			// if your not logged in and public level caching is on - you get the public copy
			if ($this->attr('public_caching')) $cache_key = 'public';
		} else {
			if (empty($key)) {
				// if permission level caching is on - check read/write/admin access for this asset
				// if the key is blank (representing a non list style asset)
				if ($this->attr('permission_caching')) {
					$db = &$GLOBALS['SQ_SYSTEM']->db;

					$groups = $current_user->getGroups();
					$userids = Array($db->quote($current_user->id));
					foreach ($groups as $groupid) $userids[] = $db->quote($groupid);

					// note that we dont use the cache in rollback view mode so we dont have
					// to run this query through the rollback functions
					$sql = 'SELECT MAX(permission) FROM '.SQ_TABLE_PREFIX.'asset_permission
							WHERE assetid = '.$db->quote($asset->id).' 
							  AND userid IN ('.implode(',', $userids).')
							  AND access = 1';

					$permission = $db->getOne($sql);
					assert_valid_db_result($permission);

					switch ($permission) {
						case SQ_PERMISSION_READ  :
							$cache_key .= ':read';
						break;
						case SQ_PERMISSION_WRITE :
							$cache_key .= ':write';
						break;
						case SQ_PERMISSION_ADMIN :
							$cache_key .= ':admin';
						break;
					}
				}
			} else {
				// if group level caching is on - check the groups the user is in if they are logged in
				if ($this->attr('group_caching')) {
					$groups = $current_user->getGroups();
					sort($groups);
					$cache_key = implode('|', $groups);
				}
			}
		}//end if logged in

		return $cache_key;

	}//end getCacheKey()


	/**
	* Return the hash of and asset between 0 and num_cache_dirs
	*
	* @param string	$assetid	the assetid to create the hash from
	*
	* @return string the hash of the assetid
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

		while (strlen($hash) != strlen($this->attr('num_cache_dirs'))) $hash = '0'.$hash;
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
		if (isset($type_codes[$type_code]['enabled'])) return $type_codes[$type_code]['enabled'];
		return $this->attr('enabled');

	}//end cacheEnabled()


	/**
	* Returns the expiry time for the specified type code.
	* 
	* If the expiry value for that type code has not been customised,
	* then the default expiry is returned
	*
	* $param string	$type_code	the type code of the wanted expiry value
	*
	* @return int
	* @access public
	*/
	function getExpiry($type_code='')
	{
		$type_codes = $this->attr('type_codes');
		if (isset($type_codes[$type_code]['expiry'])) return $type_codes[$type_code]['expiry'];
		return $this->attr('expiry');

	}//end getExpiry()


}//end class

?>
