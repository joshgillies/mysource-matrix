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
* $Id: asset_manager.inc,v 1.918 2013/09/30 04:17:06 lwright Exp $
*
*/


require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';


/**
* Asset_Manager
*
* Purpose
*    Holds information on installed packages, as represented by the DB
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.918 $
* @package MySource_Matrix
*/
class Asset_Manager extends MySource_Object
{

	/**
	* Info for all the assets types
	*
	* @var Array()
	*/
	var $_asset_types;

	/**
	* This array holds the references to the different types of objects in the system
	* so that there will only ever (with luck :) be one instance of an Asset
	*
	* @var Array(Asset)
	*/
	var $_assets = Array();

	/**
	* This array holds the references to all the assets that have been loaded
	* If its size exceeds the limit, assets without any reference count left
	* will be removed from it.
	*
	* @var object Asset_Cache
	*/
	var $_asset_cache;

	/**
	* This array holds the list of system assets in the format Array(name -> id,...)
	*
	* @var Array(string=>int)
	*/
	var $_system_assetids = Array();

	/**
	* This array holds the references to loaded attributes
	*
	* @var Array(Asset_attribute)
	*/
	var $_attributes = Array();

	/**
	* This array keeps track of what assets have been requested through getAsset calls
	*
	* @var Array(int)
	*/
	var $_get_asset_history = Array();


	/**
	* Constructor
	*
	*/
	function Asset_Manager()
	{
		$this->MySource_Object();
		$this->_loadAssetTypes();

		require_once SQ_INCLUDE_PATH.'/asset_cache.inc';
		$this->_asset_cache = new Asset_Cache();
		$this->_asset_cache->setSizeRules((php_sapi_name() == 'cli') ? SQ_CONF_ASSET_CACHE_SIZE_CLI : SQ_CONF_ASSET_CACHE_SIZE_WEB);

	}//end constructor


//--        ASSET TYPES        --//


	/**
	* Loads all the assets types into the asset array
	*
	* @return void
	* @access private
	*/
	function _loadAssetTypes()
	{
		$this->_asset_types = Array();

		if (is_file(SQ_DATA_PATH.'/private/db/asset_types.inc')) {
			include(SQ_DATA_PATH.'/private/db/asset_types.inc');
			$this->_asset_types = $asset_types;
		} else {
			// if the table columns have not been cached, the database
			// install has not been completed, so we cant get the types from the DB
			if (!is_file(SQ_DATA_PATH.'/private/db/table_columns.inc')) {
				return;
			}

			// load the asset types from the DB, which will also cache
			// them to the asset_types.inc file
			$this->_asset_types = $this->getAssetTypes();
		}

	}//end _loadAssetTypes()


	/**
	* Returns the asset types and their information.
	*
	* @param boolean	$instantiable		does the asset type need to instanstiable or not ? NULL = don't care
	* @param boolean	$non_system_access	we don't want assets that only have system access
	*
	* @return array
	* @access public
	*/
	function getAssetTypes($instantiable=NULL, $non_system_access=FALSE)
	{
		$asset_types = Array();

		$db = MatrixDAL::getDb();
		$sql = 'SELECT type_code, version, name, instantiable, allowed_access, parent_type, dir, customisation, description, lvl
				FROM sq_ast_typ';

		$where = '';
		if (!is_null($instantiable)) {
			$where .= ' WHERE instantiable = '.MatrixDAL::quote((int)$instantiable);
		}
		if ($non_system_access) {
			$where .= (($where) ? ' AND' : ' WHERE').' allowed_access != '.MatrixDAL::quote('system');
		}

		$rows = MatrixDAL::executeSqlAssoc($sql.$where);
		foreach ($rows as $row) {
			$asset_types[$row['type_code']] = $row;
		}

		return $asset_types;

	}//end getAssetTypes()


	/**
	* Refreshes the passed asset types data in the assets array
	* Also updates the sq_ast_typ_inhd table so as to have all
	* links pointing correctly
	*
	* @param string	$type_code	the code name for the asset type that you want to refresh
	*
	* @return void
	* @access public
	*/
	function refreshAssetType($type_code)
	{
		$db = MatrixDAL::getDb();
		$sql = 'SELECT type_code, version, name, instantiable, allowed_access, parent_type, dir, customisation, description, lvl
				FROM sq_ast_typ
				WHERE type_code = :type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get information for type code "'.$type_code.'" due to database error: '.$e->getMessage());
		}

		if (empty($result)) {
			trigger_localised_error('SYS0085', E_USER_WARNING, $type_code);
			return;
		} else {
			$this->_asset_types[$result[0]['type_code']] = $result[0];
		}

		$parents = Array($type_code);
		$tmp_type_code = $type_code;
		while ($this->_asset_types[$tmp_type_code]['parent_type'] != 'asset') {
			// this should NEVER happen, if it does DIE
			assert_isset($this->_asset_types[$this->_asset_types[$tmp_type_code]['parent_type']], 'Unable to get the parent of asset type "'.$this->_asset_types[$tmp_type_code]['parent_type'].'" as this asset is not installed');

			$tmp_type_code = $this->_asset_types[$tmp_type_code]['parent_type'];
			$parents[] = $tmp_type_code;
		}
		$parents[] = 'asset';

		$sql = 'SELECT inhd_type_code
				FROM sq_ast_typ_inhd
				WHERE type_code = :type_code';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
			$db_parents = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get inheritance information for type code "'.$type_code.'" due to database error: '.$e->getMessage());
		}

		$inserts = array_diff($parents, $db_parents);
		$deletes = array_diff($db_parents, $parents);
		$updates = array_intersect($parents, $db_parents);

		$type_code_level = (int) $this->getTypeInfo($type_code, 'lvl');

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($inserts as $inherited_type_code) {
			$inherited_type_code_level = ($inherited_type_code == 'asset') ? 0 : (int) $this->getTypeInfo($inherited_type_code, 'lvl');
			$sql = 'INSERT INTO sq_ast_typ_inhd
					(inhd_type_code, type_code, inhd_type_code_lvl, type_code_lvl)
					VALUES
					(:inhd_type_code, :type_code, :inhd_lvl, :lvl)';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code',      $type_code,                 PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $inherited_type_code,       PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'lvl',            $type_code_level,           PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'inhd_lvl',       $inherited_type_code_level, PDO::PARAM_INT);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to insert inheritance information for type code "'.$type_code.'" due to database error: '.$e->getMessage());
			}
		}

		foreach ($deletes as $inherited_type_code) {
			$sql = 'DELETE FROM
						sq_ast_typ_inhd
					WHERE
						inhd_type_code = :inhd_type_code
						AND type_code = :type_code';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code',      $type_code,           PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $inherited_type_code, PDO::PARAM_STR);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to delete inheritance information for type code "'.$type_code.'" due to database error: '.$e->getMessage());
			}
		}

		foreach ($updates as $inherited_type_code) {
			$inherited_type_code_level = ($inherited_type_code == 'asset') ? 0 : (int) $this->getTypeInfo($inherited_type_code, 'lvl');
			$sql = 'UPDATE
						sq_ast_typ_inhd
					SET
						inhd_type_code_lvl = :inhd_lvl,
						type_code_lvl = :lvl
					WHERE
						inhd_type_code = :inhd_type_code
						AND type_code = :type_code';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code',      $type_code,                 PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $inherited_type_code,       PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'lvl',            $type_code_level,           PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'inhd_lvl',       $inherited_type_code_level, PDO::PARAM_INT);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update inheritance information for type code "'.$type_code.'" due to database error: '.$e->getMessage());
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end refreshAssetType()


	/**
	* Returns whether the passed asset is installed in the system or not
	*
	* @param string	$type_code	the code name for the asset that you want to check
	*
	* @return boolean
	* @access public
	*/
	function installed($type_code)
	{
		return isset($this->_asset_types[$type_code]);

	}//end installed()


	/**
	* Returns an array of type codes that are installed in the system
	*
	* @return array
	* @access public
	*/
	function getTypeList()
	{
		return array_keys($this->_asset_types);

	}//end getTypeList()


	/**
	* Returns an array of asset information or a specific piece of information
	*
	* on success: field value if field specified, array of field values if field blank
	* on failure: null if field specified, empty array otherwise
	*
	* @param mixed	$type_code	the code name (or array of code names) for the asset that you want to check
	* @param string	$field		if exists, returns this information from the array for this specific field
	*
	* @return mixed string|array|NULL
	* @access public
	*/
	function getTypeInfo($type_code, $field='')
	{
		$field = preg_replace('/[^a-zA-Z_0-9]+/', '', trim($field));

		// if the file does not exist, then we must be installing so get the info from the database
		if (!is_file(SQ_DATA_PATH.'/private/db/asset_types.inc')) {

			if (empty($type_code)) {
				// No type code specified
				if ($field == '') {
					return Array();
				} else {
					return NULL;
				}
			}

			$db = MatrixDAL::getDb();

			if (is_array($type_code)) {
				foreach ($type_code as $key => $asset_type) {
					$type_code[$key] = MatrixDAL::quote($type_code[$key]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = ' = '.MatrixDAL::quote($type_code);
			}

			// if they have not specified any fields, then we want to select *
			if ($field == '') {
				$field_list = 'type_code, version, name, description, instantiable, allowed_access, parent_type, lvl, dir, customisation';
			} else {
				$field_list = 'type_code, '.$field;
			}

			$sql = 'SELECT
						'.$field_list.'
					FROM
						sq_ast_typ
					WHERE
						type_code '.$type_code_cond;

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				$result = MatrixDAL::executePdoGroupedAssoc($query);
			} catch (Exception $e) {
				if (is_array($type_code)) {
					throw new Exception('Unable to get type info for multiple type codes due to database error: '.$e->getMessage());
				} else {
					throw new Exception('Unable to get type info for type code "'.$type_code.'" due to database error: '.$e->getMessage());
				}
			}

			if (!is_array($type_code) && !empty($field)) {
				return $result[$type_code][0][$field];
			}

			if (is_array($type_code)) {
				return $result;
			} else {
				return $result[$type_code][0];
			}

		} else {
			// else we should have our own store of asset types
			if (is_array($type_code)) {
				$result = Array();
				foreach ($type_code as $type) {
					if (!isset($this->_asset_types[$type])) {
						trigger_localised_error('SYS0091', E_USER_WARNING, $type);
						continue;
					}
					if (empty($field)) {
						$result[$type] = $this->_asset_types[$type];
					} else {
						if (!isset($this->_asset_types[$type][$field])) {
							trigger_localised_error('SYS0185', E_USER_WARNING, $field);
							continue;
						}
						$result[$type] = $this->_asset_types[$type][$field];
					}
				}
				return $result;
			} else {
				if (!isset($this->_asset_types[$type_code])) {
					trigger_localised_error('SYS0091', E_USER_WARNING, $type_code);
					return (empty($field)) ? Array() : NULL;
				}
				if (empty($field)) {
					return $this->_asset_types[$type_code];
				} else {
					if (!isset($this->_asset_types[$type_code][$field])) {
						trigger_localised_error('SYS0185', E_USER_WARNING, $field);
						return NULL;
					}
					return $this->_asset_types[$type_code][$field];
				}
			}
		}//end else

	}//end getTypeInfo()


	/**
	* Returns an array representing the class hierarchy for the asset types, either for whole system
	* or for under the the passed asset type
	*
	* @param string	$base_type_code	the asset type's hierarchy to return
	* @param string	$allowed_access	the access type to return
	*
	* @return array
	* @access public
	*/
	function getAssetTypeHierarchy($base_type_code='', $allowed_access='')
	{
		if (empty($base_type_code)) $base_type_code = 'asset';

		$type_codes = Array();
		$allowed_access = trim($allowed_access);
		if (!empty($allowed_access)) {
			try {
				$bind_vars	= Array (
								'allowed_access'	=> $allowed_access,
							  );
				$type_codes = MatrixDAL::executeAssoc('core', 'getTypeCodeWithAllowedAccess', 0, $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Failed to get type code with allowed access due to database error: '. $e->getMessage());
			}
		} else {
			$type_codes = array_keys($this->_asset_types);
		}

		$offspring = Array();
		foreach ($type_codes as $type_code) {
			if (is_array($type_code)) {
				$type_code = $type_code['type_code'];
			}

			$parent = $this->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) {
				$offspring[$parent] = Array();
			}
			$offspring[$parent][] = $type_code;
		}

		$hier = $this->_recurseGetAssetTypeHierarchy($offspring, $base_type_code);

		return $hier;

	}//end getAssetTypeHierarchy()


	/**
	* Private function called by getAssetTypeHierarchy()
	*
	* @param array	$offspring		offspring
	* @param string	$base_type_code	the asset type's hierarchy to return
	*
	* @return array
	* @access private
	*/
	function _recurseGetAssetTypeHierarchy($offspring, $base_type_code)
	{
		if (empty($offspring[$base_type_code])) return Array();
		$arr = Array();
		for ($i = 0; $i < count($offspring[$base_type_code]); $i++) {
			$type = $offspring[$base_type_code][$i];
			$arr[$type] = Array(
							'name'	=> $this->_asset_types[$type]['name'],
							'subs'	=> $this->_recurseGetAssetTypeHierarchy($offspring, $type),
						  );
		}
		return $arr;

	}//end _recurseGetAssetTypeHierarchy()


	/**
	* Returns an array of assetid => inherited_types for assets of the passed codes
	*
	* @param array		$assetids			an array of assets to limit
	* @param array		$type_code			an array of types codes to limit the results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	*
	* @return array
	* @access public
	*/
	function getAssetTypeInfo($assetids, $type_code=Array(), $strict_type_code=TRUE)
	{
		if (!is_array($assetids)) {
			$assetids = Array($assetids);
		}
		assert_type($type_code, 'array');
		if (empty($assetids)) return Array();

		$db = MatrixDAL::getDb();
		$where = '';

		$shadowids = Array();
		$normalids = Array();
		$shadow_results = Array();

		// create a list of (hopefully) separated assetids and shadow assetids
		for ($i = 0; $i < count($assetids); $i++) {
			$id_parts = explode(':', $assetids[$i]);

			if (isset($id_parts[1])) {
				$shadowids[$id_parts[0]][] = $assetids[$i];
			} else {
				$normalids[] = $assetids[$i];
			}
		}

		// set it to the array minus the shadowids
		$assetids = $normalids;

		// we have picked up some shadow assets
		if (!empty($shadowids)) {
			$shadow_result = Array();

			// make each bridge get its info
			foreach ($shadowids as $assetid => $shadows) {
				$asset = $this->getAsset($assetid);
				if (method_exists($asset, 'getAssetInfo')) {
					$shadow_results += $asset->getAssetTypeInfo($shadows, $type_code, $strict_type_code);
				}
			}
		}

		// breakout if nothing left to do
		if (empty($assetids)) return $shadow_results;

		for (reset($assetids); NULL !== ($k = key($assetids)); next($assetids)) {
			$assetids[$k] = MatrixDAL::quote( (string) $assetids[$k]);
		}
		$where .= 'a.assetid IN ('.implode(', ', $assetids).')';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for ($i = 0; $i < count($type_code); $i++) {
					$type_code[$i] = MatrixDAL::quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.MatrixDAL::quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				$where .= ' AND a.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE inhd_type_code '.$type_code_cond.'
												)';
			}
		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT a.assetid, at.inhd_type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a INNER JOIN sq_ast_typ_inhd at ON a.type_code = at.type_code
				'.$where.' ORDER BY a.assetid ASC, at.inhd_type_code_lvl DESC';

		try {
			$result = MatrixDAL::executeSqlGrouped($sql);
			$result += $shadow_results;

		} catch (Exception $e) {
			throw new Exception('Unable to get asset type info due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getAssetTypeInfo()


	/**
	* Includes the class file for the passed asset
	*
	* @param string		$type_code		the code name for the asset that you want to check
	* @param boolean	$with_edit_fns	Whether to include the Edit_Fns class file too
	*
	* @return void
	* @access public
	*/
	function includeAsset($type_code, $with_edit_fns=FALSE)
	{
		$error_msg = 'Asset "'.$type_code.'" is not installed on the system, unable to include its source file';
		$type_code = strtolower(basename($type_code));
		assert_isset_array_index($this->_asset_types, $type_code, $error_msg);

		// we only need to require the type_code and its asset string once
		if (isset($this->_asset_types[$type_code]['included']) && $this->_asset_types[$type_code]['included']) {
			return;
		}

		require_once SQ_SYSTEM_ROOT.'/'.$this->_asset_types[$type_code]['dir'].'/'.$type_code.'.inc';
		if ($with_edit_fns) {
			require_once SQ_SYSTEM_ROOT.'/'.$this->_asset_types[$type_code]['dir'].'/'.$type_code.'_edit_fns.inc';
		}

		// include the language strings
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings($type_code);
		foreach ($this->getTypeAncestors($type_code) as $type_parent) {
			$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings($type_parent);
		}

		$this->_asset_types[$type_code]['included'] = TRUE;

	}//end includeAsset()


	/**
	* Returns an array of all assets types that are parents for the passed asset
	* in the order Array ('parent', 'grand-parent', 'great-grand-parent', ..., 'asset')
	*
	* @param string		$type_code		the code name for the asset that you want to check
	* @param boolean	$include_asset	when TRUE adds the "Asset" type to the parent list,
	*									even though it's an uninstantiable object
	* @param boolean	$query_db		whether to ask the DB instead of using the cache file
	*
	* @return array
	* @access public
	*/
	function getTypeAncestors($type_code, $include_asset=TRUE, $query_db=FALSE)
	{
		if ($type_code == 'asset') return Array();
		assert_isset_array_index($this->_asset_types, $type_code, 'Asset Type "'.$type_code.'" is not installed on the system');

		if ($query_db || !isset($this->_asset_types[$type_code]['ancestor_types'])) {

			$db = MatrixDAL::getDb();
			$sql = 'SELECT
						inhd_type_code
					FROM
						sq_ast_typ_inhd
					WHERE
						type_code = :type_code
					AND	inhd_type_code <> type_code
						'.(($include_asset) ? '' : ' AND inhd_type_code <> :asset_type_code').'
					ORDER BY inhd_type_code_lvl DESC';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
				if (!$include_asset) {
					MatrixDAL::bindValueToPdo($query, 'asset_type_code', 'asset', PDO::PARAM_STR);
				}
				$result = MatrixDAL::executePdoAssoc($query, 0);
			} catch (Exception $e) {
				throw new Exception('Unable to load type descendants of asset type "'.$type_code.'" due to database error: '.$e->getMessage());
			}

			return $result;

		} else {
			$res = $this->_asset_types[$type_code]['ancestor_types'];
			if ($include_asset) $res[] = 'asset';
			return $res;

		}

	}//end getTypeAncestors()


	/**
	* Returns an array of all assets types that are descendants for the passed asset type
	* in the order Array ('type_code_sub_class_one', 'type_code_sub_class_two', ..., 'type_code_sub_class_one_sub_class_one', ...)
	*
	* @param string|array	$type_code		the type code that you want the decendants of
	* @param boolean		$include_passed	when TRUE includes the passed $type_code in the result
	*
	* @return array
	* @access public
	*/
	function getTypeDescendants($type_code, $include_passed=FALSE)
	{
		if (!is_array($type_code)) {
			$type_code = Array($type_code);
		}

		for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
			if ($type_code[$i] != 'asset') {
				assert_isset_array_index($this->_asset_types, $type_code[$i], 'Asset Type "'.$type_code[$i].'" is not installed on the system');
			}
		}

		try {
			$bind_vars = Array(
							'type_codes'		=> $type_code,
							'include_passed'	=> $include_passed,
						 );
			$result = MatrixDAL::executeAssoc('core', 'getTypeDescendants', 0, $bind_vars);
			//$result = MatrixDAL::executeSqlAssoc($sql, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to load type descendants of asset type "'.$type_code.'" due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getTypeDescendants()


	/**
	* Returns whether or not a type code is a decendant of another
	*
	* @param string	$type_code1	potential child type code
	* @param string	$type_code2	potential parent type code
	*
	* @return boolean
	* @access public
	*/
	function isTypeDecendant($type_code1, $type_code2)
	{
		// get_class in PHP5 preserves capitalisation unlike PHP4
		$decendants = $this->getTypeDescendants(strtolower($type_code2), TRUE);
		return in_array(strtolower($type_code1), $decendants);

	}//end isTypeDecendant()


	/**
	* Returns whether or not a type code is an ancestor of another
	*
	* @param string	$type_code1	potential parent type code
	* @param string	$type_code2	potential child type code
	*
	* @return boolean
	* @access public
	*/
	function isTypeAncestor($type_code1, $type_code2)
	{
		$ancestors = $this->getTypeAncestors($type_code2, TRUE);
		return in_array($type_code1, $ancestors);

	}//end isTypeAncestor()


	/**
	* Checks the passed attribute type is valid type or not
	*
	* @param string	$attr_type	the type of asset represented by the id
	*
	* @return boolean
	* @access public
	*/
	function validAttributeType($attr_type)
	{
		$file = SQ_ATTRIBUTES_PATH.'/'.$attr_type.'/'.$attr_type.'.inc';
		if (!file_exists($file)) return FALSE;
		require_once($file);
		return class_exists('Asset_Attribute_'.$attr_type);

	}//end validAttributeType()


//--        ATTRIBUTES        --//


	/**
	* Returns an array of information about attributes
	*
	* @param array	$attrids	an array of assets to limit
	*
	* @return array
	* @access public
	*/
	function getAttributeInfo($attrids)
	{
		assert_type($attrids, 'array');
		if (empty($attrids)) return Array();

		$db = MatrixDAL::getDb();

		for ($i = 0; $i < count($attrids); $i++) {
			$attrids[$i] = MatrixDAL::quote($attrids[$i]);
		}
		$where = 'a.attrid IN ('.implode(', ', $attrids).')';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql = 'SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr a '.$where.' ORDER BY a.attrid';

		try {
			// cannot bind multiple values to a single named parameter, that's why we don't use bind vars here
			$result = MatrixDAL::executeSqlAssoc($sql);
		} catch (Exception $e) {
			throw new Exception('Unable to fetch attribute info due to database error: '.$e->getMessage());
		}

		$return_result = Array();
		foreach (array_values($result) as $row) {
			foreach ($row as $name => $value) {
				$return_result[$row['attrid']][$name] = $value;
			}
		}

		return $return_result;

	}//end getAttributeInfo()


	/**
	* Returns a reference to an attribute object
	*
	* @param string		$attrid			the id of the attribute
	* @param string		$value			the value of the attribute
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed
	*									because you can't use the '@' operator when
	*									returning by reference
	*
	* @return mixed object|NULL
	* @access public
	*/
	function getAttribute($attrid, $value=NULL, $mute_errors=FALSE)
	{
		if (!isset($this->_attributes[$attrid]) || !is_object($this->_attributes[$attrid]['object'])) {

			$this->_attributes[$attrid] = Array();
			$this->_attributes[$attrid]['object'] = NULL;
			$this->_attributes[$attrid]['count']  = 0;

			include_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
			$base_attr = new Asset_Attribute();
			$this->_attributes[$attrid]['object'] = $base_attr->loadAttribute($attrid);
			unset($base_attr);
			if (empty($this->_attributes[$attrid]['object']->id)) {
				$this->_attributes[$attrid]['object'] = NULL;
			}

		}

		$this->_attributes[$attrid]['count']++;
		return $this->_attributes[$attrid]['object'];

	}//end getAttribute()


	/**
	* Retrieves the attributes (with null values) for the asset type
	*
	* @param string	$type_code	the type of asset
	* @param array	$details	an array of columns from the DB table you want returned
	*
	* @return array
	* @access public
	*/
	function getAssetTypeAttributes($type_code, $details=Array('name', 'type'))
	{
		if (!is_string($type_code)) return Array();
		foreach ($details as $index => $detail) {
			$details[$index] = preg_replace('/[^a-zA-Z_0-9]+/', '', trim($detail));
		}

		$db = MatrixDAL::getDb();
		require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

		$sql = 'SELECT '.implode(', ', $details).' FROM sq_ast_attr
			WHERE type_code = :type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code);

			if (count($details) > 1) {
				$result_x = MatrixDAL::executePdoGroupedAssoc($query);
				$result = Array();
				foreach ($result_x as $key => $row) {
					$result[$key] = $row[0];
				}
			} else {
				$result = MatrixDAL::executePdoAssoc($query, 0);
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get asset info due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getAssetTypeAttributes()


	/**
	* Get the values of the attribute with the given name for the given assetids
	*
	* The assetids supplied must be assets of the specified type, or assets which
	* inherit the specified attribute from the specified asset type. You may use
	* 'asset' for the $asset_type to skip asset type checking (recommended that this
	* only be used for common attributes like name). It returns the array of
	* (string => mixed) representing assetid => attribute value
	*
	* This method will also allow you to get attribute values for a different context.
	* (The methods in Asset class, like attr(), will only return information for the
	* currently active context, for simplicity in handling.)
	*
	* @param string	$attr_name	The name of the attribute to get the values of
	* @param string	$asset_type	The type code of the asset type that defines the attribute
	* @param array	$assetids	IDs of the assets whose values we want
	* @param int	$contextid	The ID of the context we wish to get the value of the attr for.
	*							(If omitted, use the currently active context.)
	*
	* @return array
	* @access public
	*/
	function getAttributeValuesByName($attr_name, $asset_type, $assetids, $contextid=NULL)
	{
		if (empty($assetids)) return Array();

		// Create a list of separated assetids and shadow assetids
		$shadowids = Array();
		$normalids = Array();
		foreach ($assetids as $one_id) {
			$id_parts = explode(':', $one_id);
			if (isset($id_parts[1])) {
				$shadowids[$id_parts[0]][] = $one_id;
			} else {
				$normalids[] = $one_id;
			}
		}//end foreach

		// Set it to the array minus the shadowids
		$assetids = $normalids;

		// If we have picked up some shadow assets, make each bridge get its info
		$shadow_results = Array();
		foreach ($shadowids as $assetid => $shadows) {
			$asset = $this->getAsset($assetid, '', TRUE);
			if (!is_null($asset) && method_exists($asset, 'getAttributeValuesByName')) {
				$shadow_results += $asset->getAttributeValuesByName($attr_name, $asset_type, $shadows, $contextid);
			}//end if
		}//end foreach

		// breakout if nothing left to do
		if (empty($assetids)) return $shadow_results;

		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// IN clause fix - assetids were not being treated as strings
		$in = 'assetid IN (';

		for ($i = 0; $i < count($assetids); $i++) {
			$in .= MatrixDAL::quote( (string) $assetids[$i] ).(($i == count($assetids) - 1) ? ')' : ',');
		}

		// First get the default values
		$sql = 'SELECT a.assetid, at.default_val
				FROM sq_ast a, sq_ast_attr at
				WHERE a.type_code = at.type_code
					AND at.name = :attr_name
					AND ((at.type_code = :asset_type) OR (at.owning_type_code = :asset_type))
					AND a.'.$in;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'attr_name', $attr_name);
			MatrixDAL::bindValueToPdo($query, 'asset_type', $asset_type);
			$res = DAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get default attribute values due to database error: '.$e->getMessage());
		}

		// We have to convert it here to the assetid=>value format
		foreach ($res as $assetid => $val_info) {
			if (isset($val_info[0]['default_val'])) {
				$res[$assetid]	= $val_info[0]['default_val'];
			}// oracle fix, converting null to empty string. in case isset fail on null value for array entry
			else if (isset($val_info[0]) && is_array($val_info[0]) && array_key_exists('default_val',$val_info[0])){
				$res[$assetid] = '';
			}
		}//end foreach

		// Now get customised values where applicable
		$sub_sql = 'SELECT attrid FROM sq_ast_attr WHERE name = :attr_name';
		if ($asset_type != 'asset') {
			$sub_sql .= ' AND (type_code = :asset_type OR owning_type_code = :asset_type_1)';
		}

		$dbtype = MatrixDAL::getDbType();
		$bind_vars = Array();
		if ($dbtype === 'pgsql') {
			if (count($assetids) <= 10) {
				/**
				* For small number of assetids, we can safely nest them in where clause.
				*/
				$sql = 'SELECT assetid, custom_val FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val';
				$where = 'attrid IN ('.$sub_sql.') AND '.$in;
				$where .= ' AND contextid = :contextid';
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
				$sql .= $where;
				$bind_vars['contextid'] = $contextid;
			}
			else {
				/**
				* We make two parts of a query here
				* This is for performance reasons
				* If we get lots of asset id's in the where clause,
				* performance isn't great
				* so use an intersect query to break it down
				* (this lets the db use different indexes)
				*
				* The first part of the query gets the asset id's with the right attribute id's
				* the second part gets the right asset id's
				* then intersect
				*/

				$sql1 = 'SELECT assetid, custom_val
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val';
				$where1 = 'attrid IN ('.$sub_sql.') ';
				$where1 .= ' AND contextid = :contextid_1';
				$where1 = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where1);

				$sql2 = 'SELECT assetid, custom_val
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val';
				$where2 = $in;
				$where2 .= ' AND contextid = :contextid_2';
				$where2 = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where2);

				$sql = $sql1 . $where1 . ' INTERSECT ' . $sql2 . $where2;
				$bind_vars['contextid_1'] = $contextid;
				$bind_vars['contextid_2'] = $contextid;
			}
		}

		/**
		* If it's an oracle db we're talking to,
		* we can't do an intersect query as above because
		* oracle won't let you use intersect queries where a nvarchar
		* field is included in the query (custom_val)
		*
		* so we're going to have to take the performance hit
		* if there are lots of asset id's to filter through.
		*/
		if ($dbtype === 'oci') {
			$sql = 'SELECT assetid, custom_val FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val';
			$where = 'attrid IN ('.$sub_sql.') AND '.$in;
			$where .= ' AND contextid = :contextid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where;
			$bind_vars['contextid'] = $contextid;
		}

		$bind_vars['attr_name'] = $attr_name;
		if ($asset_type !== 'asset') {
			$bind_vars['asset_type'] = $asset_type;
			$bind_vars['asset_type_1'] = $asset_type;
		}

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$custom_vals = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get customised attribute values for attr "'.$attr_name.'" and type "'.$asset_type.'" due to database error: '.$e->getMessage());
		}

		// Traverse through the custom val array, if it these assets have custom val for theirs attribute, return it
		// If the assets do not have custom val, the array is probably empty and thus it wont go into the loop
		foreach ($custom_vals as $assetid => $rowinfo) {
			if (isset($rowinfo[0]['custom_val'])) {
				$res[$assetid] = $rowinfo[0]['custom_val'];
			} // oracle fix, converting null to empty string. in case isset fail on null value for array entry
			else if (isset($rowinfo[0]) && is_array($rowinfo[0]) && array_key_exists('custom_val',$rowinfo[0])){
				$res[$assetid] = '';
			}
		}//end foreach

		return $res;

	}//end getAttributeValuesByName()


	/**
	* Get all the attribute values of the given assetid and type code.
	* This method is similar to asset:_loadVars() except it does not unserialize any attributes.
	* Return array of name => array (attrid, type, value, is_contextable, use_default)
	*
	* If there are no custom value for an attribute, its default value will be loaded.
	* If no asset ID is provided, this function just loads the default values for the asset type.
	* This method will also allow you to get attribute values for a different context.
	*
	* @param mixed		$assetid		(int | string) the asset id to be loaded
	* @param string		$type_code		the type of $assetid
	* @param int		$contextid		The ID of the context we wish to get the value of the attr for.
	*									(If omitted, use the currently active context.)
	*
	* @return array
	* @access public
	*/
	function getAllAttributeValues($assetid='', $type_code, $contextid=NULL)
	{

		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// Right, now we need to get any values that this asset has customised
		$vars = Array();

		$sql = '';
		// if we have an id then we need to load with current vars
		if ($assetid) {
			// PURPOSLY DONT ADD EXTRA CLAUSES FOR ASSET_ATTRIBUTE_VALUE BECAUSE WE WONT GET
			// DEFAULT VALUES IF WE DO
			$sql = 'SELECT atr.name, atr.attrid, atr.type, COALESCE(v.custom_val, atr.default_val) AS value, atr.is_contextable, v.use_default
					FROM (sq_ast_attr atr
					  LEFT OUTER JOIN (SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val WHERE contextid = :contextid) v
						ON (atr.attrid = v.attrid AND v.assetid = :assetid'
						.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'v', 'AND').'))
						WHERE atr.type_code = :type_code';
		} else {
			// else just load all defaults
			$sql = 'SELECT atr.name, atr.attrid, atr.type, atr.default_val AS value, atr.is_contextable, \'1\' as use_default
					FROM sq_ast_attr atr
					WHERE atr.type_code = :type_code';
		}// end if

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
			// Only bind it if we are using the first query, not binding when going in the else above
			if ($assetid) {
				MatrixDAL::bindValueToPdo($query, 'contextid', $contextid, PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'assetid',   $assetid, PDO::PARAM_STR);
			}//end if
			$result = MatrixDAL::executePdoGroupedAssoc($query);

			if (empty($result)) {
				$vars = Array();
			} else {
				$vars = $result;
				foreach (array_keys($vars) as $name) {
					$vars[$name] = $vars[$name][0];
					unset($vars[$name][0]);
				}
			}

			unset($result);
		} catch (Exception $e) {
			throw new Exception('Unable to load variables of asset #'.$assetid.' ('.$type_code.') due to database error: '.$e->getMessage());
		}

		return $vars;

	}//end getAllAttributeValues()

//--        ASSETS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param mixed		$assetid		(int | string) the asset id to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	*/
	function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		// This function is supposed to take in an assetid of type int or string only, array of assetids should not be accepted.
		if (is_array($assetid)) {
			$msg = translate('assert_assetid', gettype($assetid), '');
			trigger_exception($msg, FALSE, TRUE);
			$asset = NULL;
			return $asset;
		}//end if

		assert_valid_assetid($assetid);
		$asset = $this->_asset_cache->get($contextid.'\\'.$assetid);

		if (empty($asset)) {
			// Try Deja vu
			$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
			if ($deja_vu) {
				$asset = $deja_vu->recall(SQ_DEJA_VU_ASSET, $assetid);
				if (!empty($asset)) {
					if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
						$asset = NULL;
					}
					else {
						// We still want asset cache to do its thing.
						$this->_asset_cache->add($asset, $contextid.'\\'.$asset->id);
					}
				}
			}
		}

		/**
		 * We got the asset from the cache
		 */
		if (!empty($asset)) {
			/**
			 * If we also have a type_code to check,
			 * make sure it matches the one we're loading from cache.
			 */
			if (!empty($type_code) && $asset->type() != $type_code) {
				if (!$mute_errors) {
					trigger_localised_error('SYS0089', E_USER_WARNING, $assetid, $type_code);
				}
				$asset = NULL;
			}
			return $asset;
		}

		// check if we are getting a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$bridge = $this->getAsset($real_assetid, '', TRUE);
			if (is_null($bridge)) {
				// bridge is unknown, we cannot return anything from it
				$asset = NULL;
				return $asset;
			} else if (!method_exists($bridge, 'getAsset')) {
				trigger_localised_error('SYS0203', E_USER_WARNING, $bridge->name);
			} else {
				$asset = $bridge->getAsset($assetid, '', $mute_errors);
			}
			$this->forgetAsset($bridge);

		} else {

			if (empty($type_code)) {
				$sql = 'SELECT type_code FROM '.SQ_TABLE_RUNNING_PREFIX.'ast ';
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = :assetid');
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
				$type_code = MatrixDAL::executePdoOne($query);
			}

			if (empty($type_code)) {
				if (!$mute_errors) {
					trigger_localised_error('SYS0087', E_USER_WARNING, $assetid);
				}
			} else {
				if (!isset($this->_asset_types[$type_code])) {
					if (!$mute_errors) {
						trigger_localised_error('SYS0091', E_USER_WARNING, $type_code);
					}
				} else {
					$this->includeAsset($type_code);
					if ($mute_errors) {
						$asset = @new $type_code($assetid);
					} else {
						$asset = new $type_code($assetid);
					}
					if (empty($asset->id)) {
						$asset = NULL;
					}
				}
			}

		}//end if

		if (isset($asset)) {
			$this->_get_asset_history[] = $assetid;
			$this->_asset_cache->add($asset, $contextid.'\\'.$asset->id);

			$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
			if ($deja_vu &&  !($asset->status & SQ_SC_STATUS_SAFE_EDITING)) {
				$deja_vu->remember(SQ_DEJA_VU_ASSET, $asset->id, $asset);
			}
		}

		return $asset;

	}//end getAsset()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note shadow assets will not be considered as existing unless the bridge that defines
	* it properly defines an assetExists() function, and does more than just return FALSE.
	*
	* If a single asset ID is passed the function will return TRUE or FALSE depending on
	* whether that asset exists. If an array is passed it will return an array of the
	* assetids that exist.
	*
	* @param string|array	$assetids	the ID(s) of the asset(s) to check.
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		$shadows = Array();
		$shadow_asset_ids = Array();

		// number of assetids
		$count = count($assetids);

		$is_array = is_array($assetids);
		$is_empty = empty($assetids);

		// assert assetid format
		if ($is_array && !$is_empty) {
			foreach ($assetids as $key => $value) {
				assert_valid_assetid($value);

				// catch the shadow assets
				$id_parts = explode(':', $value);
				if (isset($id_parts[1])) {
					$shadows[$id_parts[0]][] = $value;
				}
			}
		} else if (!$is_empty) {
			assert_valid_assetid($assetids);
			// catch the shadow assets
			$id_parts = explode(':', $assetids);
			if (isset($id_parts[1])) {
				$shadows[$id_parts[0]][] = $id_parts[0].':'.$id_parts[1];
			}
		}

		// pipe it off to the real asset
		if (!empty($shadows)) {
			foreach ($shadows as $shadow_id => $shadow) {
				$asset = $this->getAsset($shadow_id);
				if (method_exists($asset,'assetExists')) {
					$ret_val = $asset->assetExists($shadow);
					if (is_array($ret_val)) {
						$shadow_asset_ids += $ret_val;
					} else if ($ret_val) {
						$shadow_asset_ids[] = $shadow;
					}
				}
				$this->forgetAsset($asset);
			}
		}

		$db = MatrixDAL::getDb();
		$sql = 'SELECT assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast ';

		// single asset id
		if (!$is_array) {
			if ($is_empty) return FALSE;
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = :assetids');
			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'assetids', $assetids);
				$db_assetids = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to determine if asset with assetid: '.$assetids.' exists due to database error: '.$e->getMessage());
			}

			if (!empty($shadow_asset_ids)) {
				$shadow_asset_ids[] = $db_assetids;
			} else {
				return ($db_assetids == $assetids);
			}
		}

		// array of asset ids
		$existing_asset_ids = Array();
		if (!$is_empty) {
			$in = 'assetid IN (';

			for ($i=0; $i<count($assetids); $i++) {
				$in .= MatrixDAL::quote( (string) $assetids[$i] ).(($i == count($assetids) - 1) ? ')' : ',');
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($in);
			try {
				$db_assetids = MatrixDAL::executeSqlAll($sql.$where);
			} catch (Exception $e) {
				throw new Exception('Unable to determine if multipled assets exists due to database error: '.$e->getMessage());
			}

			foreach ($db_assetids as $db_id) {
				$existing_asset_ids[] = $db_id['assetid'];
			}
		}

		for ($i = 0; $i < count($shadow_asset_ids); $i++) {
			$existing_asset_ids[] = $shadow_asset_ids[$i];
		}

		return $existing_asset_ids;

	}//end assetExists()


	/**
	* Registers the passed object in the store, for others to get
	* Used when a new record has just been "created"
	*
	* @param object	&$obj	the asset to be registered
	*
	* @return void
	* @access public
	* @see forgetAsset()
	*/
	function rememberAsset(&$obj)
	{
		// wrong class or no id, cya
		if (!is_object($obj) || !($obj instanceof Asset) || !$obj->id) {
			return;
		}

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if (!$this->_asset_cache->add($obj, $contextid.'\\'.$obj->id)) {
			trigger_localised_error('SYS0305', E_USER_ERROR, $obj->id);
		}

	}//end rememberAsset()


	/**
	* Tell the asset manager that you have finished with the asset, when everyone has finished with the
	* asset is is removed from the asset store
	*
	* @param object		&$obj			the asset to be unregistered
	* @param boolean	$force_clean	force the asset manager to forget this asset rather
	*									than just reducing its entires (only used by the asset itself
	*									during a purge)
	*
	* @return void
	* @access public
	* @see rememberAsset()
	*/
	function forgetAsset(&$obj, $force_clean=FALSE)
	{
		// wrong class or no id, cya
		if (!is_object($obj) || !($obj instanceof Asset) || !$obj->id) {
			return;
		}

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		$this->_asset_cache->release($contextid.'\\'.$obj->id);
		if ($force_clean) {
			if (!$this->_asset_cache->remove($contextid.'\\'.$obj->id, TRUE)) {
				trigger_localised_error('SYS0306', E_USER_WARNING, $contextid.'\\'.$obj->id);
			} else {
				if (isset($obj->_tmp)) unset($obj->_tmp);
			}
		}

	}//end forgetAsset()


	/**
	* Returns an assetid for a system asset if it exists
	*
	* @param string	$name	the name of the system asset 'root_folder', 'trash_folder' etc
	*
	* @return mixed string|boolean
	* @access public
	*/
	function getSystemAssetid($name)
	{
		if (!is_string($name)) {
			trigger_localised_error('SYS0190', E_USER_WARNING);
			return FALSE;
		}

		if (empty($this->_system_assetids)) {
			$this->_reloadSystemAssetList();
		}

		if (isset($this->_system_assetids[$name])) {
			return $this->_system_assetids[$name];
		} else {
			trigger_localised_error('SYS0209', E_USER_WARNING, $name);
			return FALSE;
		}

	}//end getSystemAssetid()


	/**
	* Refreshes the list of cached system assets
	*
	* @return void
	* @access private
	*/
	function _reloadSystemAssetList()
	{
		require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
		$sys_asset_cfg = new System_Asset_Config();
		// include the system asset config file
		if (!file_exists($sys_asset_cfg->config_file)) {
			// we called this too early
			$system_assets = FALSE;
		} else {
			require $sys_asset_cfg->config_file;
		}

		$this->_system_assetids = $system_assets;

	}//end _reloadSystemAssetList()


	/**
	* Empties the cached list of system assets
	*
	* @return void
	* @access public
	*/
	function resetSystemAssetList()
	{
		$this->_system_assetids = Array();

	}//end resetSystemAssetList()


	/**
	* Returns TRUE if given asset type is already registered as a system asset
	*
	* During install, the output of this function should not be cached as it can change when new
	* asset types are installed
	*
	* @param string	$asset_type	typecode of asset in question
	*
	* @return boolean
	* @access public
	*/
	function isSystemAssetType($asset_type)
	{
		if (empty($this->_system_assetids)) {
			$this->_reloadSystemAssetList();
		}

		return isset($this->_system_assetids[$asset_type]);

	}//end isSystemAssetType()


	/**
	* Registers a system asset, identified by the type,id pair
	*
	* @param string	$asset_type	asset typecode
	* @param string	$assetid	asset id
	*
	* @return boolean
	* @access public
	*/
	function registerSystemAsset($asset_type=NULL, $assetid=NULL)
	{
		if (is_null($asset_type) || is_null($assetid)) {
			return FALSE;
		}

		$this->_system_assetids[$asset_type] = $assetid;
		return TRUE;

	}//end registerSystemAsset()


	/**
	* Returns a reference to a system asset
	*
	* @param string		$name			the name of the system asset 'root_folder', 'trash_folder' etc
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	*/
	function getSystemAsset($name, $mute_errors=FALSE)
	{
		$assetid = ($mute_errors) ? @$this->getSystemAssetid($name) : $this->getSystemAssetid($name);

		if ($assetid !== FALSE) {
			$asset = $this->getAsset($assetid, $name, $mute_errors);
			return $asset;
		} else {
			$null = NULL;
			return $null;
		}

	}//end getSystemAsset()


	/**
	* Determine if this asset can be purged from the trash
	*
	* The function stops system assets and their dependent children from being purged.
	*
	* @param object	&$asset	the asset to purge
	*
	* @return boolean
	* @access public
	*/
	function canPurgeAsset(&$asset)
	{
		if (empty($this->_system_assetids)) {
			$this->_reloadSystemAssetList();
		}

		if (in_array($asset->id, $this->_system_assetids)) {
			return FALSE;
		}

		// if we get to here, it is not one of the
		// system assets, but it may be dependently linked to one
		// so lets check for that
		$parents = $this->getDependantParents($asset->id);
		foreach ($parents as $parentid) {
			if (in_array($parentid, $this->_system_assetids)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end canPurgeAsset()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If $field is empty then all fields are returned, otherwise it can be an array
	* of standard asset fields or a string representing one of them.
	*
	* If one value in $field then it will be returned as array of assetid => field value.
	* If $field is empty or an array the field value will itself be an array keyed
	* by field name.
	*
	* @param array			$assetids			an array of assets to limit
	* @param string|array	$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean		$strict_type_code	ensure returned assets are of the passed type codes
	* @param string|array   $field				single or array of fields from the asset table to return (along with the assetid)
	*											Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		$shadow_results = Array();
		if (!is_array($assetids)) {
			$assetids = Array($assetids);
		}
		if (empty($assetids)) return Array();

		$bind_vars = Array();
		$db = MatrixDAL::getDb();

		$shadowids = Array();
		$normalids = Array();
		$shadow_results = Array();

		// create a list of (hopefully) separated assetids and shadow assetids
		foreach ($assetids as $one_id) {
			$id_parts = explode(':', $one_id);

			if (isset($id_parts[1])) {
				$shadowids[$id_parts[0]][] = $one_id;
			} else {
				$normalids[] = $one_id;
			}
		}

		// set it to the array minus the shadowids
		$assetids = $normalids;

		// we have picked up some shadow assets
		if (!empty($shadowids)) {
			$shadow_result = Array();

			// make each bridge get its info
			foreach ($shadowids as $assetid => $shadows) {
				$asset = $this->getAsset($assetid, '', TRUE);
				if ($asset && method_exists($asset, 'getAssetInfo')) {
					$shadow_results += $asset->getAssetInfo($shadows, $type_code, $strict_type_code, $field);
				}
			}
		}

		// breakout if nothing left to do
		if (empty($assetids)) return $shadow_results;

		// need to do some checking on the field to make sure it's in the table
		// but only do it if the field is specified
		if (!empty($field)) {
			if (empty($this->_tmp['sq_tables'])) {
				require SQ_DATA_PATH.'/private/db/table_columns.inc';
				$this->_tmp['sq_tables'] = $tables;
				unset($tables);
			}

			// field not found
			if (is_array($field) === FALSE) {
				if (!in_array($field, $this->_tmp['sq_tables']['ast']['columns'])) {
					trigger_localised_error('SYS0185', E_USER_WARNING, $field);
					return Array();
				}
			} else {
				$missing = array_diff($field, $this->_tmp['sq_tables']['ast']['columns']);
				if (count($missing) > 0) {
					trigger_localised_error('SYS0185', E_USER_WARNING, implode(', ', $field));
					return Array();
				}
			}
		}

		for ($i = 0; $i < count($assetids); $i++) {
			$assetids[$i] = MatrixDAL::quote($assetids[$i]);
		}

		// break up the assets into chunks of 1000 so that oracle does not complain
		$in_clauses = Array();
		foreach (array_chunk($assetids, 999) as $chunk) {
			$in_clauses[] = ' a.assetid IN ('.implode(', ', $chunk).')';
		}
		$where = '('.implode(' OR ', $in_clauses).')';

		if (!empty($type_code)) {
			$type_code_cond = '';
			$type_query_str = Array();
			if (is_array($type_code)) {
				for ($i = 0; $i < count($type_code); $i++) {
					$bind_vars['type_code_'.$i] = $type_code[$i];
					$type_query_str[] = ':type_code_'.$i;
				}
				$type_code_cond = 'IN ('.implode(', ', $type_query_str).')';
			} else {
				$type_code_cond = '= :type_code';
				$bind_vars['type_code'] = $type_code;
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				$where .= ' AND a.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE inhd_type_code '.$type_code_cond.'
												)';
			}
		}//end if

		if (empty($field)) {
			// if we want all the column info, get a list of columns
			// and append them after the assetid so we can use getAssoc
			// to produce assetid => Array(info)
			if (empty($this->_tmp['sq_tables'])) {
				require SQ_DATA_PATH.'/private/db/table_columns.inc';
				$this->_tmp['sq_tables'] = $tables;
			} else {
				$tables = $this->_tmp['sq_tables'];
			}
			unset($tables['ast']['columns'][array_search('assetid', $tables['ast']['columns'])]);
			$col_string = implode(', ', $tables['ast']['columns']);
		} else if (is_array($field) === FALSE) {
			$col_string = $field;
		} else {
			$col_string = implode(', ', $field);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT assetid, '.$col_string.'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a
				'.$where.'
				ORDER BY a.assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}

			$result = MatrixDAL::executePdoGroupedAssoc($query);
			foreach	($result as $key => &$row) {
				// if the field is equal to asset id we will have to behave slightly differently
				// because when the field is asset id the result_x has the following format:
				// 56 => array (
				//   0 => array (
				//        [Empty]
				//     )
				// ),
				if ((empty($field) === FALSE) && (is_array($field) === FALSE)) {
					// if we are only after a single field
					if ($field === 'assetid') {
						$row = $key;
					} else {
						$row = $row[0][$field];
					}
				} else {
					$row = $row[0];
				}
			}//end foreach
			unset($row);

		} catch (Exception $e) {
			throw new Exception('Unable to get asset info due to database error: '.$e->getMessage());
		}

		// add the shadow assets info
		$result += $shadow_results;

		return $result;

	}//end getAssetInfo()


	/**
	* Get the list of fields that getAssetInfo can return for an asset
	*
	* @return array
	* @access public
	*/
	function getAssetInfoFields()
	{
		if (!isset($this->_tmp['asset_info_fields'])) {
			require SQ_DATA_PATH.'/private/db/table_columns.inc';
			$this->_tmp['asset_info_fields'] = Array();
			foreach ($tables['ast']['columns'] as $col) {
				$this->_tmp['asset_info_fields'][$col] = translate('asset_field_'.$col);
			}
			unset($tables);
		}
		return $this->_tmp['asset_info_fields'];

	}//end getAssetInfoFields()


	/**
	* Returns list of all assetids of a certain type
	*
	* @param string		$type_code		if this exists then this object is used to load the asset,
	*									if not then the DB is queried to find out the asset type
	* @param boolean	$strict			whether we are finding assets that are just a $type_code
	*									or $type_code and any of it's sub-classes
	* @param boolean	$include_type	if FALSE returns Array(assetid, assetid, ...), if TRUE
	*									returns Array(assetid => asset_type, assetid => asset_type, ...)
	*
	* @return array
	* @access public
	*/
	function getTypeAssetids($type_code, $strict=TRUE, $include_type=FALSE)
	{
		if (!is_string($type_code)) return Array();
		$db = MatrixDAL::getDb();

		if ($strict) {
			$where = 'type_code = :type_code';
		} else {
			$where = 'type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE inhd_type_code = :type_code
									)';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT assetid'.(($include_type) ? ', type_code' : '').'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast '.$where;

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code);
			if ($include_type) {
				$result_x = MatrixDAL::executePdoGroupedAssoc($query);
				$result = Array();
				foreach ($result_x as $key => $row) {
					$result[$key] = $row[0];
				}
			} else {
				$result = MatrixDAL::executePdoAssoc($query, 0);
			}
		} catch (Exception $e) {
			throw $e;
		}

		return $result;

	}//end getTypeAssetids()


//--        CONTENT        --//


	/**
	* Get the editable content of any asset below the passed assetid
	* It returns FALSE if assetid is invalid, Array(assetid=>content) otherwise
	*
	* @param int	$assetid	The asset we want to get the contents of
	*
	* @return mixed boolean|array
	* @access public
	*/
	function getEditableContents($assetid)
	{
		if (!$assetid || !$this->assetExists($assetid)) {
			return FALSE;
		}

		$children = array_keys($this->getDependantChildren($assetid));
		$contents = Array();

		foreach ($children as $child_id) {
			$child = $this->getAsset($child_id);
			$dependants_children = array_keys($this->getDependantChildren($child_id));

			// find out if our dependent children are in the children list, otherwise
			// we'll get the same content twice
			// we have to do in array for each key, an array needle gets us nowhere
			for ($i = 0; $i < count($dependants_children); $i++) {
				if (in_array($dependants_children[$i], $children)) {
					continue 2;
				}
			}

			$child_content = $child->getContent();

			if (!empty($child_content)) {
				$contents[$child_id] = $child->getContent();
			}

			$this->forgetAsset($child);
		}

		return empty($contents) ? FALSE : $contents;

	}//end getEditableContents()


	/**
	* Set the editable content of the specified asset
	* It returns FALSE if assetid is invalid, TRUE if successful
	*
	* @param int	$assetid	The asset we want to set the contents of
	* @param string	$content	The content of editable asset
	*
	* @return boolean
	* @access public
	*/
	function setEditableContents($assetid, $content)
	{
		$asset = $this->getAsset($assetid);

		if (is_null($asset)) return FALSE;

		$asset->setContent($content);

		return TRUE;

	}//end setEditableContents()


//--        CLONING        --//


	/**
	* Creates the shell of a clone without cloning any of the source's components
	*
	* The only child assets that are cloned are the ones that are <i>DEPENDENTLY</i> linked to this asset.
	* <i>NO OTHER</i> linked assets are cloned.
	* It returns the reference to the new object or NULL if an error occured
	*
	* @param object		&$source			the source asset that we are creating a clone of
	* @param array		&$link				information used to create the initial link,
	* <pre>
	* 										Array (
	*											'asset'			=> [ref major asset to create link under],
	* 											'link_type'		=> SQ_LINK_?,
	*			 								'value'			=> [link value],
	* 											'sort_order'	=> [link sort order],
	* 											'is_dependant'	=> [0|1],
	* 											'is_exclusive'	=> [0|1]
	* 										)
	* </pre>
	* @param array		&$clone_map			a map for that can be used for re-indexing data held in
	*										assets of the form Array([old asset id] => [new asset id])
	* @param array		$components			the wanted components to clone,
	* <pre>
	* 										Array(
	* 											'attributes',
	* 											'metadata_schemas',
	* 											'metadata',
	* 											'workflow',
	* 											'permissions',
	* 											'data',
	* 											 'content_tags',
	* 											 'roles',
	* 										);
	* 										or alternately
	*		 								Array('all');
	* </pre>
	* @param boolean	$cloning_dependent	by default FALSE, flipped to TRUE if the asset being cloned is another asset's dependent
	* @param int		$initial_staus		initial asset status of the clone
	*
	* @return mixed object|NULL
	* @access private
	*/
	function _cloneAsset(&$source, &$link, &$clone_map, $components, $cloning_dependent=FALSE, $initial_status=SQ_STATUS_UNDER_CONSTRUCTION)
	{
		if (!empty($link)) {
			// make sure the initial link information is passed in
			assert_isset_array_index($link, 'asset', 'Cannot clone asset without an asset to link to');
			assert_isset_array_index($link, 'link_type', 'Cannot clone asset without a link type');
			assert_not_empty(($link['link_type'] & SQ_SC_LINK_SIGNIFICANT), 'Cannot clone asset with an insignificant link type');
		}

		$null = NULL; // needed because we return by reference
		if (!$source->id) return $null;
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			$lock = $this->getLockInfo($source, 'all');
			foreach ($lock as $lock_type => $lock_info) {
				if (empty($lock_info)) {
					trigger_localised_error('SYS0266', E_USER_WARNING, $source->id, $lock_type);
					return $null;
				}
			}
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$assetid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_seq'));

		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		// version number starts at 0.0.0 again because the clone is a brand new asset
		$initial_version = '0.0.0';

		try {
			$bind_vars = Array(
								'date_value' => ts_iso8601(time()),
							  );
			$last_updated = MatrixDAL::executeOne('core', 'toDate', $bind_vars);
			$bind_vars = Array(
							'assetid'				=> $assetid,
							'version'				=> $initial_version,
							'type_code'				=> $source->type(),
							'name'					=> $source->name.' - clone',
							'short_name'			=> $source->short_name.' - clone',
							'status'				=> $initial_status,
							'created'				=> $last_updated,
							'created_userid'		=> $userid,
							'published'				=> NULL,
							'published_userid'		=> NULL,
							'updated'				=> $last_updated,
							'updated_userid'		=> $userid,
							'status_changed'		=> NULL,
							'status_changed_userid'	=> NULL,
						 );
			MatrixDAL::executeQuery('core', 'createAsset', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Failed to create cloned asset due to database error: '.$e->getMessage());
		}

		$clone = $this->getAsset($assetid, $source->type());
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// lock the new clone in the same chain as we are locked
			if (!$this->acquireLock($clone->id, 'all', $source->id)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return $null;
			}
		}

		$parent_components = Array(
								'permissions',
								'metadata_schemas',
								'workflow',
								'roles',
							 );
		// make an exception for tags - tags never cascade to dependent assets, so leave
		// them out if we're cloning a dependent asset
		if (!$cloning_dependent) {
			$parent_components[] = 'content_tags';
		}


		//// clone non-link-based components ////

		// cascade various components from parent asset to new child
		// unless the parent is the root folder or system management folder;
		// components are cascaded from the new parent regardless of what permissions the
		// current user has (to match creation and moving assets)

		// If no $link is specified, just don't clone anything from the parent.
		if (!empty($link)) {
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
				$this->_tmp[__CLASS__.'_in_create_cascading'] = TRUE;
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);

				// remove content_tags from the array, if present; munge array, then give it to cloneComponents
				$nonlink_parent_components = $parent_components;
				foreach ($nonlink_parent_components as $idx => $component) {
					if ($component == 'content_tags') {
						unset($nonlink_parent_components[$idx]);
					}
				}

				if (!$link['asset']->cloneComponents($clone, $nonlink_parent_components)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

					return $null;
				}
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				unset($this->_tmp[__CLASS__.'_in_create_cascading']);
			}
		}

		// now that we (might) have permission, copy all the schemas, attributes,
		// etc. from the original asset
		// (will throw errors but return TRUE if the user doesn't have sufficient permissions
		// to cascade everything)
		$nonlink_components = $components;
		foreach ($nonlink_components as $idx => $component) {
			if ($component == 'content_tags') {
				unset($nonlink_components[$idx]);
			}
		}
		if (!$source->cloneComponents($clone, $nonlink_components, TRUE)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}

		//// clone links ////

		// we've created the asset, now link it to its new parent
		$clone = $this->_cloneLink($clone, $link, $lock, $source);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $clone_result;
		}

		// enhancement #5988 Cloning an asset loses it "Force Secure" option
		// set the required properties of the cloned asset same as the source
		$clone->setForceSecure($source->force_secure);

		// clone the other links
		if (!$source->cloneLinks($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}


		//// go back and clone link-based components ////

		// as per above, cascade components from parent asset to new
		// child, regardless of permission

		// If no $link is specified, just don't clone anything from the parent.
		if (!empty($link)) {
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
				$this->_tmp[__CLASS__.'_in_create_cascading'] = TRUE;
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);

				// only clone content_tags this time (only link-based component so far)
				$link_parent_components = (in_array('content_tags', $parent_components) ? Array('content_tags') : Array());

				if (!empty($link_parent_components) && !$link['asset']->cloneComponents($clone, $link_parent_components)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

					return $null;
				}
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				unset($this->_tmp[__CLASS__.'_in_create_cascading']);
			}
		}

		// copy all link-based components from the original asset
		$link_components = $components;
		foreach ($link_components as $idx => $component) {
			if ($component != 'content_tags') {
				unset($link_components[$idx]);
			}
		}
		if (!empty($link_components) && !$source->cloneComponents($clone, $link_components, TRUE)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}


		// if we got this far all is well,
		// so add to the map and return the clone
		$clone_map[$source->id] = $clone->id;
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $clone;

	}//end _cloneAsset()


	/**
	* Links a cloned asset the under its new parent
	*
	* This function also handles [links] locking of the new parent to
	* accommodate the new link.
	*
	* @param object	&$clone		the new clone
	* @param string	&$link		the link object
	* @param array	&$lock		the lock object
	* @param object	&$source	the source object the clone is based off
	*
	* @return mixed NULL|object
	* @access public
	*/
	function &_cloneLink(&$clone, &$link, &$lock, &$source)
	{
		if (!empty($link)) {
			// create the initial link
			if (!isset($link['value'])) $link['value'] = '';
			if (!isset($link['sort_order'])) {
				$link['sort_order'] = -1;
			}
			if (!isset($link['is_dependant'])) {
				$link['is_dependant'] = 0;
			}
			if (!isset($link['is_exclusive'])) {
				$link['is_exclusive'] = 0;
			}

			$linkid = $link['asset']->createLink($clone, $link['link_type'], $link['value'], $link['sort_order'], $link['is_dependant'], $link['is_exclusive']);
			if (empty($linkid)) return $null;

		}//end if !empty(link)

		return $clone;

	}//end _cloneLink()


	/**
	* Creates a clone of an asset and clones the required components of the source asset
	*
	* if $clone_dependents is set the TRUE, the dependents of the source asset will be cloned,
	* and the specified components will be cloned to them as well.
	* It returns the clone of the source, NULL on error
	*
	* @param object		&$source			the source asset that we are creating a clone of
	* @param array		&$link				information used to create the initial link,
	* <pre>
	* Array (
	*   'asset'         => [ref major asset to create link under],
	*   'link_type'     => SQ_LINK_?,
	*   'value'         => [link value],
	*   'sort_order'    => [link sort order],
	*   'is_dependant'  => [0|1],
	*   'is_exclusive'  => [0|1]
	* );
	* </pre>
	* @param array		&$clone_map			a map for that can be used for re-indexing data held in
	*										assets of the form Array([old asset id] => [new asset id])
	* @param array		$components			the wanted components to clone,
	* <pre>
	* Array(
	*   'attributes',
	*   'metadata_schemas',
	*   'metadata',
	*   'workflow',
	*   'permissions',
	*   'data'// data directory
	* );
	* </pre>
	*										or alternately
	* <pre>
	* Array('all');
	* </pre>
	* @param boolean	$clone_dependents	if TRUE the dependents of the source asset will be cloned
	*										and will have their sources components cloned
	* @param boolean	$cloning_dependent	TRUE mean this call is a recursive call from itself, and it
	*										is cloning the dependent of an asset (in a recursive manner)
	* @param int		$initial_staus		initial asset status of the clone
	*
	* @return object
	* @access public
	*/
	function &cloneAsset(&$source, &$link, &$clone_map, $components, $clone_dependents=TRUE, $cloning_dependent=FALSE, $initial_status=SQ_STATUS_UNDER_CONSTRUCTION)
	{
		$null = NULL;
		// if the source asset is actually a shadow asset, skip it
		$id_parts = explode(':', $source->id);
		if (isset($id_parts[1])) return $null;

		// make sure this type of asset can be cloned
		if (!$source->canClone()) {
			trigger_localised_error('SYS0070', E_USER_WARNING, $source->type());
			return $null;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$clone = $this->_cloneAsset($source, $link, $clone_map, $components, $cloning_dependent, $initial_status);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}

		// OK now that we are linked up let's clone our dependents
		if ($clone_dependents) {

			$dependent_links = $this->getLinks($source->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major', NULL, 1);

			if (!empty($dependent_links)) {
				$create_link = Array(
								'asset'			=> &$clone,
								'link_type'		=> NULL,
								'value'			=> NULL,
								'sort_order'	=> -1,
								'is_dependant'	=> '1',
								'is_exclusive'	=> '0',
							   );

				foreach ($dependent_links as $data) {
					// If this asset has already been cloned in this duplication run
					// then just link it to the new clone of ourselves
					if (isset($clone_map[$data['minorid']])) {

						$cloned_child = $this->getAsset($clone_map[$data['minorid']], $data['minor_type_code']);
						if (is_null($cloned_child)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return $null;
						}

						$linkid = $clone->createLink($cloned_child, $data['link_type'], $data['value'], $data['sort_order'], '1', $data['is_exclusive']);
						if (!$linkid) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return $null;
						}

					// otherwise clone and link
					} else {
						$child = $this->getAsset($data['minorid'], $data['minor_type_code']);
						if (is_null($child)) continue;

						// skip shadow assets
						$id_parts = explode(':', $child->id);
						if (isset($id_parts[1])) continue;

						$create_link['link_type']		= $data['link_type'];
						$create_link['value']			= $data['value'];
						$create_link['sort_order'] 		= $data['sort_order'];
						$create_link['is_exclusive']	= $data['is_exclusive'];
						$new_component = Array();

						$cloned_child = $this->cloneAsset($child, $create_link, $clone_map, $components, $clone_dependents, TRUE);

						if (is_null($cloned_child)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return $null;
						}
					}//end if

					if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
						// lock the new clone in the same chain as we are locked
						$lock = $this->getLockInfo($cloned_child, 'all');
						if (!empty($lock)) {
							if (!$this->releaseLock($cloned_child->id, 'all')) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
								return $null;
							}//end if
						}//end if not empty lock
					}//end if
				}//end foreach
			}//end if empty
		}//end if clone dependents

		// Now all the dependents are clones. cloneComponentsAdditional()
		// will process extra processing required for completing cloning
		if (!$source->cloneComponentsAdditional($clone, Array('all'))) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}

		// contents and some other components are not index for this cloned asset because the
		// asset link wasnt created to its parent.
		// Bug Fix - #4615 Cloning an Asset doesnt re-index all the components
		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		if(!is_null($sm)) {
			$sm->reindexAsset($clone, Array('all'));
			$sm->reindexContents($clone, Array());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// if all went well then broadcast the event telling system a asset has been cloned
		$parameter = Array();
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_cloned', $clone, $parameter);
		return $clone;

	}//end cloneAsset()


//--        LOCKING        --//


	/**
	* Acquires a lock on an asset
	*
	* Note that this method operates on a single asset only.  To get the locks on children
	* too (as per backend behaviour), you need to use the acquire_locks HIPO job.
	* It returns zero on failure, non-zero on success
	* (1 on success if all lock types requested had to be acquired,
	* 2 if any lock was updated [ie if at least one lock type was already held])
	*
	* @param int		$assetid		the assetid of the asset that we want to lock
	* @param string		$lock_type		any valid lock type for the passed asset's type,
	*									plus 'all' which will attempt to acquire all locks that this asset has
	* @param int		$source_assetid	the id of the asset that started this locking process
	* @param boolean	$force			attempt to forcibly acquire (if allowed)
	* @param int|NULL	$expires		when the lock expires (timestamp or NULL)
	*									If expires is null or zero, the lock will be acquired for the
	*									current time plus SQ_CONF_LOCK_LENGTH
	*
	* @return int
	* @access public
	*/
	function acquireLock($assetid, $lock_type, $source_assetid=0, $force=FALSE, $expires=0)
	{
		// type-check lock type BEFORE we get asset, otherwise we're just wasting time allocating it
		if (empty($lock_type) || !is_string($lock_type)) {
			trigger_localised_error('SYS0074', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$asset = $this->getAsset($assetid);
		// Instead of throwing a fatal error, we just return false here, getAsset has already given a warning if the asset does not exists.
		if (is_null($asset)) {
			return FALSE;
		}

		$assetType = $asset->type();

		// because lock_type can be expanded, we need to keep the original type
		// to check if we can force getting it.
		$orig_lock_type = $lock_type;

		$current_locks = $this->getLockInfo($assetid, $lock_type, TRUE, FALSE, TRUE, FALSE, $assetType);
		if (empty($current_locks)) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = TRUE;

		// Set this to unknown for now.
		// This is a fairly expensive operation. we only need this info if
		// a lock is held by someone other than ourselves.
		// On the other hand, if we're not trying to force acquiring a lock,
		// we can immediately set this to FALSE.
		$can_force = NULL;
		if ($force === FALSE) {
			$can_force = FALSE;
		}

		$lock_updated = FALSE;
		foreach ($current_locks as $lock_type => $lock) {

			if (!empty($lock)) {
				if ($lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
					// the user is asking to acquire a lock they already had
					// so just update the lock expiry date
					if ($this->updateLock($assetid, $lock_type, $expires)) {
						$lock_updated = TRUE;
						continue;
					} else {
						$success = FALSE;
						break;
					}
				} else {
					if ($can_force === NULL) {
						$can_force = $asset->canForceablyAcquireLock($orig_lock_type);
					}
				}
			}

			// special case where the current user is actually
			// removing the current lock and taking it for themselves
			if (!empty($lock) && $can_force) {

				// attempt to remove the lock
				$lock = $this->getLockInfo($assetid, $lock_type, FALSE, TRUE, TRUE, FALSE, $assetType);
				if (!$this->releaseLock($assetid, $lock_type)) {
					$success = FALSE;
					break;
				}

				// send an internal message
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$ms->openLog();

				$user = $this->getAsset($lock['userid']);

				$locked_assetids = Array((int) $assetid);

				// Redefine lock with the chained asset info.
				foreach ($lock['chained_assets'] as $row) {
					$locked_assetids[] = $row['assetid'];
				}

				foreach ($locked_assetids as $locked_assetid) {
					$locked_asset = $this->getAsset($locked_assetid);

					// create a new message object and populate it, before adding to
					// the message queue in the internal messaging system to sending later
					$msg_reps = Array(
									'user_name'		=> $GLOBALS['SQ_SYSTEM']->user->name,
									'type_code'		=> $this->getTypeInfo($locked_asset->type(), 'name'),
									'lock_type'		=> ucwords($lock_type),
									'asset_name'	=> $locked_asset->name,
									'old_user_name'	=> $user->name,
								);
					$log = $ms->newMessage(Array(), 'asset.locking.forced', $msg_reps);
					$log->parameters['assetid'] = $locked_asset->id;
					$log->parameters['former_userid'] = $user->id;
					$ms->logMessage($log);

					$this->forgetAsset($locked_asset);
				}

				// close the queue of messages we opened, which sends all the messages in the queue
				$ms->closeQueue();
				$ms->closeLog();

				// refresh the lock info, just in case someone has sneaked in while we are sending the message
				$lock = $this->getLockInfo($assetid, $lock_type, FALSE, TRUE, TRUE, FALSE, $assetType);

			}//end if

			// is this asset already locked
			if (!empty($lock)) {
				$user = $this->getAsset($lock['userid']);
				trigger_localised_error('SYS0101', E_USER_NOTICE, $lock_type, $asset->name, $user->name);
				$success = FALSE;
				break;
			}

			$lockid = 'asset.'.$assetid.'.'.$lock_type;
			$source_lockid = ($source_assetid) ? 'asset.'.$source_assetid.'.'.$lock_type : '';
			if (TRUE !== ($err_msg = $GLOBALS['SQ_SYSTEM']->acquireLock($lockid, $source_lockid, $expires))) {
				trigger_localised_error('SYS0100', E_USER_NOTICE, $lock_type, $asset->name, $err_msg);
				$success = FALSE;
				break;
			}

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction(($success) ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// return 2 for expiry updated only, 1 for success and 0 for failure
		return ($success) ? (($lock_updated) ? 2 : 1) : 0;

	}//end acquireLock()


	/**
	* Release the locks that the passed asset holds
	*
	* Assume that the current user has the lock to release before trying to see if
	* it can be released by force.
	*
	* @param int	$assetid	the assetid of the asset that we want to release
	* @param string	$lock_type	the type of the lock (eg 'contents', 'permissions', etc)
	*
	* @return boolean
	* @access public
	*/
	function releaseLock($assetid, $lock_type)
	{
		// type-check lock type BEFORE we get asset, otherwise we're just wasting time allocating it
		if (empty($lock_type) || !is_string($lock_type)) {
			trigger_localised_error('SYS0075', E_USER_WARNING, $assetid);
			return FALSE;
		}

		// because lock_type can be expanded, we need to keep the original type
		// to check if we can force getting it.
		$orig_lock_type = $lock_type;

		$asset = $this->getAsset($assetid);
		// Instead of throwing a fatal error, we just return here, getAsset has already given a warning if the asset does not exist.
		if (is_null($asset)) {
			return TRUE;
		}

		$assetType = $asset->type();

		$current_locks = $this->getLockInfo($assetid, $lock_type, TRUE, FALSE, TRUE, FALSE, $assetType);
		if (empty($current_locks)) return TRUE;

		$success = TRUE;

		// Set this to unknown for now.
		// This is a fairly expensive operation. we only need this info if
		// a lock is held by someone other than ourselves.
		$can_force = NULL;

		foreach ($current_locks as $lock_type => $lock) {
			if (empty($lock)) continue;

			if ((int) $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserid()) {
				if ($can_force === NULL) {
					$can_force = $asset->canForceablyAcquireLock($orig_lock_type);
				}

				// is this asset already locked by someone else and we can't forceably acquire it, error
				if (!$can_force) {
					$user = $this->getAsset($lock['userid']);
					trigger_localised_error('SYS0264', E_USER_WARNING, $lock_type, $asset->name, $user->name);
					$success = FALSE;
					continue;
				}
			}

			if (TRUE !== ($err_msg = $GLOBALS['SQ_SYSTEM']->releaseLock('asset.'.$assetid.'.'.$lock_type))) {
				trigger_localised_error('SYS0109', E_USER_NOTICE, $lock_type, $asset->name, $err_msg);
				$success = FALSE;
				continue;
			}

		}//end foreach

		return $success;

	}//end releaseLock()


	/**
	* Updates the expiry date of the lock on this asset
	*
	* @param int		$assetid	the assetid of the asset that we want to release
	* @param string		$lock_type	the type of the lock (eg 'contents', 'permissions', etc)
	* @param int|NULL	$expires	when the lock expires (timestamp or NULL)
	*
	* @return boolean
	* @access public
	*/
	function updateLock($assetid, $lock_type, $expires=0)
	{
		// Make sure this is working on a proper asset.
		$asset = $this->getAsset($assetid);
		if (is_null($asset)) {
			return TRUE;
		}

		$assetType = $asset->type();

		$current_locks = $this->getLockInfo($assetid, $lock_type, TRUE, FALSE, TRUE, TRUE, $assetType);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = TRUE;

		foreach (array_keys($current_locks) as $lock_type) {
			if (!empty($lock_type)) {
				if (TRUE !== ($err_msg = $GLOBALS['SQ_SYSTEM']->updateLock('asset.'.$assetid.'.'.$lock_type, $expires))) {
					trigger_localised_error('SYS0122', E_USER_NOTICE, $assetid, $err_msg);
					$success = FALSE;
					break;
				}//end if
			}//end if
		}//end foreach

		if ($success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $success;

	}//end updateLock()


	/**
	* Returns an array of information about the current lock on this asset (if any)
	*
	* @param int|object	$asset		Either an asset (object) or just an id.
	* @param string		$lock_type		any valid lock type for the passed asset's type, plus 'all' which will return all locks that this asset has - will force an array to be returned
	* @param boolean	$force_array	always return the information in an array, as though more than one lock has been requested
	* @param boolean	$full_chain		return the info for every asset in the lock chain
	* @param boolean	$check_expires	check the expiry date and cleanup the lock if it has expired
	* @param boolean	$allow_only_one	Force only one lock returned
	* @param string     $assetType      The asset type we're getting a lock for.
	*                                   If null, it works it out from the asset.
	*                                   This is passed through to getLockTypes.
	*
	* @return array
	* @access public
	*/
	function getLockInfo($asset, $lock_type, $force_array=FALSE, $full_chain=FALSE, $check_expires=TRUE, $allow_only_one=FALSE, $assetType=NULL)
	{
		if (is_object($asset) === FALSE) {
			$assetid = $asset;
		} else {
			$assetType = $asset->type();
			$assetid   = $asset->id;
		}

		$lock_types = $this->getLockTypes($assetid, $lock_type, $assetType);
		$locks = Array();
		$lockids = Array();
		$count = 0;
		foreach ($lock_types as $each_lock_type) {
			$lockids[] = 'asset.'.$assetid.'.'.$each_lock_type;
			$last_lock = 'asset.'.$assetid.'.'.$each_lock_type;
			$count++;
		}

		if (!empty($lockids)) {
			if ($count > 1) {
				$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($lockids, $full_chain, $check_expires, $allow_only_one);
			} else {
				$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo($last_lock, $full_chain, $check_expires, $allow_only_one);
			}

			$all_locks = Array();
			if ($allow_only_one) {
				$all_locks[] = $lock;
			} else {
				$all_locks = $lock;
			}

			foreach ($lock_types as $each_lock_type) {
				$current_lock_id = 'asset.'.$assetid.'.'.$each_lock_type;
				$locks[$each_lock_type] = Array();
				foreach	($all_locks as $each_key => $each_lock) {
					if (isset($each_lock['lockid']) && ($current_lock_id == $each_lock['lockid'])) {
						$each_lock['lock_type']      = $lock_type;
						$each_lock['source_assetid'] = preg_replace('|^asset\.(.*)\.[\w]+$|', '\1', $each_lock['source_lockid']);
						if ($full_chain) {
							for (reset($each_lock['chained_assets']); NULL !== ($k = key($each_lock['chained_assets'])); next($each_lock['chained_assets'])) {
								$each_lock['chained_assets'][$k]['assetid'] = preg_replace('/^asset\.([0-9]+(:.+)?)\..*$/', '\1', $each_lock['chained_assets'][$k]['lockid']);
								$each_lock['chained_assets'][$k]['source_assetid'] = preg_replace('/^asset\.([0-9]+(:.+)?)\..*$/', '\1', $each_lock['chained_assets'][$k]['source_lockid']);
							}
						}
						$locks[$each_lock_type] = $each_lock;
					}
				}
			}
			unset($lock);
		}//end if

		if (!$force_array && count($locks) == 1) {
			return reset($locks);
		} else {
			return $locks;
		}

	}//end getLockInfo()


	/**
	* Returns an array of lock types strings that the passed lock type really represents for the passed asset
	* eg 'content' could mean 'attributes' and 'links'.
	*
	* @param int	$assetid	the assetid of the asset that we want lock info for
	* @param string	$lock_type	any valid lock type for the passed asset's type, plus 'all' which will return all lock types that this asset has
	* @param string $assetType  The asset type we're getting lock info for.
	*
	* @return array
	* @access private
	*/
	function getLockTypes($assetid, $lock_type, $assetType=NULL)
	{

		// If the lock_type is not a string (invalid), just return an empty array.
		if (!is_string($lock_type) || empty($lock_type)) {
			return Array();
		}//end if

		if ($assetType === NULL || !isset($this->_tmp['lock_types'][$assetType])) {
			$asset = $this->getAsset($assetid);
			// If the asset is null, just return an empty array, throwing fatal error is a bit over done.
			if (is_null($asset)) return Array();
			$assetType = $asset->type();
			$this->_tmp['lock_types'][$assetType] = $asset->lockTypes();
		}

		if ($lock_type !== 'all') {
			if (!isset($this->_tmp['lock_types'][$assetType][$lock_type])) {
				return Array();
			}
			$bits = bit_elements($this->_tmp['lock_types'][$assetType][$lock_type]);
			$lock_types = Array();
			foreach ($bits as $bit) {
				if (FALSE !== ($k = array_search($bit, $this->_tmp['lock_types'][$assetType]))) {
					$lock_types[] = $k;
				}
			}
			return $lock_types;
		} else {
			if (isset($this->_tmp['lock_types']['all'][$assetType])) {
				return $this->_tmp['lock_types']['all'][$assetType];
			}

			$lock_types = Array();
			foreach ($this->_tmp['lock_types'][$assetType] as $lock_type => $bit) {
				if (preg_match('/^0*10*$/', decbin($bit))) {
					$lock_types[] = $lock_type;
				}
			}
			$this->_tmp['lock_types']['all'][$assetType] = $lock_types;
			return $lock_types;
		}//end if

	}//end getLockTypes()


//--        LINKING        --//


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding an asset that
	*											is just a $type_code or potentially an inherited type
	*											or $type_code and any of it's sub-classes
	* @param string			$value				the value that is associated with this link
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param boolean		$exclusive			The exclusive status for the link must be
	*											this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
		assert_valid_assetid($assetid);
		$bind_vars = Array();

		// if its a shadow assetid pipe it off to bridge
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$asset = $this->getAsset($id_parts[0]);
			$link =  $asset->getLink($assetid, $link_type, $type_code, $strict_type_code, $value, $side_of_link, $exclusive);
			$this->forgetAsset($asset);
			return $link;
		}

		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = MatrixDAL::getDb();
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						l.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive, l.locked
				FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';

		$where = 'l.'.$side_of_link.'id = :assetid';
		$bind_vars['assetid'] = $assetid;

		if (!is_null($link_type)) {
			$where .= '
				AND l.link_type = :link_type';
			$bind_vars['link_type'] = $link_type;
		}

		if (!is_null($exclusive)) {
			$where .= ' AND l.is_exclusive = :is_exclusive';
			$bind_vars['is_exclusive'] = ($exclusive) ? '1' : '0';
		}

		if ($type_code) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				$type_query_str = Array();
				for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
					$type_query_str[$i] = ':type_code_'.$i;
					$bind_vars['type_code_'.$i] = $type_code[$i];
				}
				$type_code_cond = 'IN ('.implode(', ', $type_query_str).')';
			} else {
				$type_code_cond = '= :type_code';
				$bind_vars['type_code'] = $type_code;
			}

			if ($strict_type_code) {
				$where .= ' AND l.type_code '.$type_code_cond;
			} else {
				$where .= ' AND l.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE inhd_type_code '.$type_code_cond.'
												)';
			}
		}

		if (!is_null($value)) {
			$where .= ' AND (l.value = :link_value';
			if ($value == '') $where .= ' OR l.value IS NULL';
			$where .= ')';

			$bind_vars['link_value'] = $value;
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		$sql .= $where.' ORDER BY l.sort_order';

		try {
			$query  = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoAssoc($query);
			if (!empty($result)) $result = $result[0];
		} catch (Exception $e) {
			throw new Exception('Unable to get link for asset with assetid: '.$assetid.' due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getLink()


	/**
	* Get details of a "shadow" asset link with the specified characteristics
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	id of the the other asset
	* @param int		$link_types		integer that should be a single integer of the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	side of this asset in this link
	* @param boolean	$force_array	if it's TRUE, the returned value is always an array
	*
	* @return array
	* @access public
	*/
	function getShadowLinkByAsset($assetid, $other_assetid=NULL, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE)
	{
		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		if (!is_null($other_assetid)) {
			assert_valid_assetid($other_assetid);

			if (!$other_assetid || is_object($other_assetid)) {
				trigger_localised_error('SYS0108', E_USER_WARNING);
				return Array();
			}
		}

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = MatrixDAL::getDb();
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type
				FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk l';

		$where = 'l.'.$side_of_link.'id = :assetid';

		if (!is_null($other_assetid)) {
			$where .= ' AND l.'.$other_side.'id  = '.MatrixDAL::quote($other_assetid);
		}

		if (!is_null($link_types)) {
			$where .= ' AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', $link_types).' > 0 ';
		}

		if (!is_null($value)) {
			$where .= ' AND l.value = '.MatrixDAL::quote($value);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get shadow link for this assetid: '.$assetid.' due to database error: '.$e->getMessage());
		}

		if (!$force_array && count($result) == 1) {
			return $result[0];
		} else {
			return $result;
		}

	}//end getShadowLinkByAsset()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link
	* @param string	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1])) {
			$bridge = $this->getAsset($id_parts[0]);
			$link = $bridge->getLinkById($linkid, $assetid, $side_of_link);
			$this->forgetAsset($bridge);
			return $link;
		}

		$db = MatrixDAL::getDb();

		$sql = 'SELECT  l.linkid, l.value, l.link_type, l.sort_order, l.is_dependant, l.is_exclusive, l.locked, ';
		$where = 'l.linkid = :linkid';

		// if they only want one side of the link, we can do less work
		if ($assetid !== 0) {
			$sql .= 'l.'.$other_side.'id, l.type_code as '.$other_side.'_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';
			$where .= ' AND l.'.$side_of_link.'id = :assetid';
		} else {
			$sql .= 'l.majorid, l.type_code as minor_type_code, l.minorid, a.type_code as major_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'linkid', (int) $linkid);
			if ($assetid !== 0) {
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			}
			$result = MatrixDAL::executePdoAssoc($query);
			if (empty($result)) {
				$result = NULL;
			} else if (isset($result[0])) {
				$result = $result[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get link details for asset with assetid: '.$assetid.' due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getLinkById()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* It returns array of link info if only one link and force_array=FALSE,
	* array of arrays of link info otherwise
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param mixed		$other_assetid	the asset(s) on the other side of the link - can be an array of assets
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not null)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{
		assert_valid_assetid($assetid);
		assert_valid_assetid($other_assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		if (!$other_assetid || is_object($other_assetid)) {
			trigger_localised_error('SYS0108', E_USER_WARNING);
			return Array();
		}

		if (!is_array($other_assetid)) {
			$other_assetid = Array($other_assetid);
		}

		// handle shadow assets
		$other_shadow_assetids = Array();
		$shadow_links = Array();

		// If the main asset ID is a shadow asset, then we need to run through
		// ALL of the assets with it, as all combinations will have a shadow
		// component. If it's not a shadow asset, then we need to find which
		// other asset IDs are shadow, extract them, and then run the rest
		// through the sq_ast_lnk table as normal.
		$assetid_shadow_parts = explode(':', $assetid);
		$assetid_is_shadow = (count($assetid_shadow_parts) > 1);

		if ($assetid_is_shadow) {
			$other_shadow_assetids = $other_assetid;
			$other_assetid = Array();
		} else {
			foreach ($other_assetid as $key => $other_assetid_value) {
				$shadow_parts = explode(':', $other_assetid_value);

				if (count($shadow_parts) > 1) {
					$other_shadow_assetids[] = $other_assetid_value;
					unset($other_assetid[$key]);
				}
			}
		}

		// If there are no shadow assets then don't bother looking at this
		if (!empty($other_shadow_assetids)) {

			// Assign major and minor asset IDs according to side of link -
			// making the main asset ID a single-element array
			if ($side_of_link == 'major') {
				$majorids = Array($assetid);
				$minorids = $other_shadow_assetids;
			} else {
				$majorids = $other_shadow_assetids;
				$minorids = Array($assetid);

				// the site_of_link also should be swapped
				$side_of_link = 'major';
			}

			// Loop through each of these and get each possible combination of
			// minor/major asset ID. (This isn't as bad as it sounds because
			// one of these WILL be a single asset ID)
			foreach ($majorids as $majorid) {
				$major_shadow_parts = FALSE;
				$id_parts           = explode(':', $majorid);
				if (isset($id_parts[1])) {
					$major_shadow_parts = Array(
											'bridgeid'	=> $id_parts[0],
											'shadowid'	=> $id_parts[1],
										  );
				}

				foreach ($minorids as $minorid) {
					$minor_shadow_parts = FALSE;
					$id_parts           = explode(':', $minorid);
					if (isset($id_parts[1])) {
						$minor_shadow_parts = Array(
												'bridgeid'	=> $id_parts[0],
												'shadowid'	=> $id_parts[1],
											  );
					}

					// If the major asset itself a shadow asset, we need to use its
					// bridge.
					// If the major asset is real, and the minor is owned by it
					// (eg. '123' to '123:abc') then also use the bridge.
					// Otherwise, use the shadow asset link table.
					$use_bridge = NULL;
					if (!empty($major_shadow_parts)) {
						$use_bridge = $major_shadow_parts['bridgeid'];
					} else if (!empty($minor_shadow_parts)) {
						if ((string) $majorid === (string) $minor_shadow_parts['bridgeid']) {
							$use_bridge = $majorid;
						}
					}

					if ($use_bridge !== NULL) {
						$link  = Array();
						$asset = $this->getAsset($use_bridge);
						if (!is_null($asset)) {
							// Always force an array in this case, and merge it in
							$link = $asset->getLinkByAsset($majorid, $minorid, $link_types, $value, $side_of_link, TRUE, $dependant, $exclusive);
							$this->forgetAsset($asset);
						}
						$shadow_links = array_merge($shadow_links, $link);
					} else {
						if (!empty($minor_shadow_parts)) {
							// Look at the shadow link table instead.
							// Always force an array, and merge it in
							$link = $this->getShadowLinkByAsset($majorid, $minorid, $link_types, $value, $side_of_link, TRUE, $dependant, $exclusive);
							$shadow_links = array_merge($shadow_links, $link);
						}
					}

				}//end foreach minor asset ID

			}//end foreach major asset ID

		}//end if there are shadow assets

		// If there are no more assets to run through, return what shadow links
		// we have only
		if (!empty($other_assetid)) {
			$db = MatrixDAL::getDb();
			$bind_vars = Array();

			$prepared_otherids = Array();
			foreach ($other_assetid as $id) {
				$prepared_otherids[] = '\''.$id.'\'';
			}

			$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

			$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						   l.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive, l.locked
					FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';

			$where = 'l.'.$side_of_link.'id = :assetid
					  AND l.'.$other_side.'id  IN ('.implode(',', $prepared_otherids).')';
			$bind_vars['assetid'] = $assetid;

			if (!is_null($link_types)) {
				$where .= ' AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', $link_types).' > 0 ';
			}
			if (!is_null($value)) {
				if (empty($value)) {
					$where .= ' AND (l.value = :value OR l.value IS NULL)';
				} else {
					$where .= ' AND l.value = :value';
				}
				$bind_vars['value'] = $value;
			}
			if (!is_null($dependant)) {
				$where .= ' AND l.is_dependant = :is_dependant';
				$bind_vars['is_dependant'] = ($dependant) ? '1' : '0';
			}
			if (!is_null($exclusive)) {
				$where .= ' AND l.is_exclusive = :is_exclusive';
				$bind_vars['is_exclusive'] = ($exclusive) ? '1' : '0';
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

			$sql .= $where.'
					ORDER BY l.sort_order';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				$result = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get link for assetid: '.$assetid.' due to database error: '.$e->getMessage());
			}

		} else {
			$result = Array();
		}

		// Add shadow links from above to the result
		$result += $shadow_links;

		if (!$force_array && count($result) == 1) {
			return $result[0];
		} else {
			return $result;
		}

	}//end getLinkByAsset()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* It returns array of link info if only one link and force_array=FALSE,
	* array of arrays of link info otherwise
	*
	* @param array	$array1	first array to merge
	* @param array	$array2	second array to merge
	*
	* @return array
	* @access public
	*/
	function arrayMerge($array1,$array2)
	{
		$common_keys = Array();
		$merged_array = Array();
		if (!is_array($array1)) $array1 = Array();
		if (!is_array($array2)) $array2 = Array();

		// add all elements which is not common to both array
		foreach ($array1 as $key => $value) {
			if (array_key_exists($key, $array2)) {
				$common_keys = array_merge($common_keys, Array($key));
				continue;
			}
			// if the key does not exist in the other array
			$merged_array[$key] = $array1[$key];
		}

		foreach ($array2 as $key => $value) {
			if (array_key_exists($key, $array1)) continue;

			// if the key does not exist in the other array
			$merged_array[$key] = $array2[$key];
		}

		foreach ($common_keys as $key) {
			$merged_array[$key] = array_merge($array1[$key], $array2[$key]);
		}
		return $merged_array;

	}//end arrayMerge()


	/**
	* Get all links matching the specified characteristics
	*
	* The query generated to get the links is actually contructed in the Asset Manager's
	* generateGetLinksQuery function.
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string|array	$value				The value for all the links must equal this (if not null)
	* @param boolean		$dependant			The dependant status for all the links must be
	*											this (if not null)
	* @param boolean		$exclusive			The exclusive status for all the links must be
	*											this (if not null)
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$effective			If effective permissions should be considered or not.
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $value=NULL, $dependant=NULL, $exclusive=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE)
	{
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');
		$force_array = FALSE;

		if (!is_array($assetid)) {
			$assetids = Array($assetid);
		} else {
			$force_array = TRUE;
			$assetids = $assetid;
		}

		$links = Array();
		$query_assetids = Array();
		foreach ($assetids as $assetid) {
			assert_valid_assetid($assetid);

			// check if we are getting links for a shadow asset, and palm the request off to the
			// handler of the shadow asset if we are
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				$real_assetid = $id_parts[0];
				$asset = $this->getAsset($real_assetid);
				if (is_null($asset)) continue;

				$links[$assetid] = $asset->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by, $dependant, $exclusive);
				$this->forgetAsset($asset);

				// we also need the links to real assets for this shadow asset
				// if we are looking up the tree
				if (($side_of_link == 'minor') && !$exclusive && !$dependant) {
					$db = MatrixDAL::getDb();
					$sql = 'SELECT
								l.linkid,
								l.majorid,
								l.minorid,
								l.value,
								l.link_type,
								l.locked,
								a.type_code AS major_type_code
							FROM
								'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a on l.majorid = a.assetid';

					$where = 'minorid = '.MatrixDAL::quote($assetid);
					if(!is_null($link_types)){
						$where .= ' AND '.db_extras_bitand(MatrixDAL::GetDbType(), 'l.link_type', MatrixDAL::quote($link_types)).' > 0 ';
					}
					if ($type_code) {
						if ($strict_type_code) {
							$where .= ' AND a.type_code IN (';
							if (is_array($type_code)) {
								foreach ($type_code as $index => $typ_cd) {
									$type_code[$index] = MatrixDAL::quote($typ_cd);
								}
								$where .= implode(', ', $type_code);
							} else {
								$where .=  MatrixDAL::quote($type_code);
							}
							$where .= ') ';
						} else {
							$type_ancestors = $this->getTypeDescendants($type_code);
							foreach ($type_ancestors as $i => $v) {
								$type_ancestors[$i] = MatrixDAL::quote($v);
							}
							if (is_array($type_code)) {
								foreach ($type_code as $index => $typ_cd) {
									$type_ancestors[] = MatrixDAL::quote($typ_cd);
								}
							} else {
								$type_ancestors[] =  MatrixDAL::quote($type_code);
							}
							$where .= ' AND a.type_code IN ('.implode(', ', $type_ancestors).')';
						}
					}
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

					$result = NULL;
					$query = MatrixDAL::preparePdoQuery($sql.$where);
					try {
						$result = MatrixDAL::executePdoAll($query);
					} catch (Exception $e) {
						throw new Exception('Could not get shadow links of asset ID #'.$assetid.' due to database error: '.$e->getMessage());
					}//end

					$shdw_links = Array();
					foreach ($result as $shdw_link) {
						$shdw_links[] = Array(
											'linkid'			=> $shdw_link['linkid'],
											'majorid'			=> $shdw_link['majorid'],
											'major_type_code'	=> $shdw_link['major_type_code'],
											'minorid'			=> $shdw_link['minorid'],
											'value'				=> $shdw_link['value'],
											'link_type'			=> $shdw_link['link_type'],
											'is_dependant'		=> '0',
											'is_exclusive'		=> '0',
											'sort_order'		=> 0,
											'locked'			=> $shdw_link['locked'],
										);

					}
					// in some cases the links[$assetid] value is null. If this is the case return an empty array
					// Now php5 complains if the parameters of the array_merge function is not an array
					$_links_to_merge = is_null($links[$assetid]) ? Array() : $links[$assetid];
					$links[$assetid] = array_merge($_links_to_merge, $shdw_links);

				}//end if side_of_link is major

				continue;

			}//end if shadow asset

			// we are not getting links for a shadow asset, so if we are getting child links of
			// an asset that handles shadow assets, palm the request off to it
			if ($side_of_link == 'major') {
				$asset = $this->getAsset($assetid);
				if (is_null($asset)) return Array();
				if (implements_interface($asset, 'bridge')) {
					$links[$assetid] = $asset->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by, $dependant, $exclusive);
					// still need the asset manager to get asset that are not managed by the bridge
				}
				$this->forgetAsset($asset);
			}

			// we now have an assetid that still requires a query, so we'll add it to its own list
			$query_assetids[] = $assetid;

		}//end foreach

		if (!empty($query_assetids)) {
			// if value is not null and not array, convert it to array
			if (!is_null($value) && !is_array($value)) {
				$value = Array(
							'link_value' 	=> Array($value),
							'equal' 		=> TRUE,
						 );
			}
			// we couldnt palm the request off, so we better do it ourselves
			$links_query = $this->generateGetLinksQuery($query_assetids, $link_types, $type_code, $strict_type_code, $side_of_link, $value, $dependant, $exclusive, $sort_by, $access, $effective);

			if (!empty($links_query)) {
				$sql = implode(' ', $links_query['sql_array']);
				$query = MatrixDAL::preparePdoQuery($sql);

				foreach ($links_query['bind_vars'] as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}

				$result = MatrixDAL::executePdoGroupedAssoc($query);
				$links = $this->arrayMerge($result,$links);
			}
		}

		if (count($links) == 1 && !$force_array) {
			$keys = array_keys($links);
			$links = $links[$keys[0]];
		}

		return $links;

	}//end getLinks()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string		$assetid		the assetid of the asset to find the child links for
	* @param int		$link_type		the type of links to get
	* @param int		$access			SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*									Specify that links should be returned for assets the user has read,
	*									write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean	$effective		If effective permissions should be considered or not.
	* @param string		$bind_prefix	The prefix for bind variables generated by this function
	*									(default: 'gc_')
	* @param int		$depth			The maximum depth we need to get links until
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0, $access=NULL, $effective=TRUE, $bind_prefix='gc_', $depth=0)
	{
		if (!isset($this->_tmp['child_links'][$assetid])) {
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				$asset = $this->getAsset($id_parts[0]);
				$links = $asset->getAllChildLinks($assetid, $link_type);
				$this->forgetAsset($asset);
				return $links;
			}

			// So it is not a shadow asset, but it could be a bridge
			$asset = $this->getAsset($assetid);
			$shadow_links = Array();
			if (implements_interface($asset, 'bridge')) {
				$shadow_links = $asset->getAllChildLinks($assetid, $link_type);
			}

			$db = MatrixDAL::getDb();

			$where = 'l.minorid = :assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid'.$where;
			$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
				$treeid = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get treeid for asset: '.$assetid.' due to database error: '.$e->getMessage());
			}

			$current_level = strlen($treeid) / SQ_CONF_ASSET_TREE_SIZE;

			$where = 't.treeid LIKE :'.$bind_prefix.'treeid_like
					  AND t.treeid > :'.$bind_prefix.'treeid';
			if ($depth != 0) {
				$where .=' AND ((LENGTH(t.treeid) / '.SQ_CONF_ASSET_TREE_SIZE.') '.(($current_level) ? ' - '.$current_level : '').') <= :gc_max_tree_depth';
			}

			// only do a link comparison if they have specified a link type
			if ($link_type != 0) {
				$where .=  ' AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', $link_type).' > 0';
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

			$select = 'SELECT SUBSTR(t.treeid, '.MatrixDAL::quote((int) strlen($treeid) + 1).') as treeid,
							((LENGTH(t.treeid) / '.SQ_CONF_ASSET_TREE_SIZE.') '.(($current_level) ? ' - '.$current_level : '').') as lvl,
							l.linkid, a.assetid, a.short_name, a.type_code, l.link_type, l.sort_order, l.value, l.is_dependant, l.is_exclusive';
			$from = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid';
			$order_by = 'ORDER BY t.treeid';
			$having = '';
			$group_by = '';

			$bind_vars[$bind_prefix.'treeid_like'] = $treeid.'%';
			$bind_vars[$bind_prefix.'treeid'] = $treeid;
			if ($depth != 0) $bind_vars[$bind_prefix.'max_tree_depth'] = $depth;

			if (!is_null($access)) {
				$access = (String)$access;
				$userid_cond = '';
				$group_by = 'GROUP BY t.treeid, l.linkid, l.minorid, l.link_type, l.sort_order, l.value, l.is_dependant, l.is_exclusive, a.assetid, a.short_name, a.type_code, a.name';
				if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					$from .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON p.assetid = a.assetid ';
					$from .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON (p.userid = r.roleid AND r.assetid = a.assetid) ';

					$current_user = $this->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
					$userids = $current_user->getUserGroups();
					$this->forgetAsset($current_user);
					$userids[] = (String)$GLOBALS['SQ_SYSTEM']->user->id;
					$public_userid = (String)$this->getSystemAssetid('public_user');
					$userids[] = $public_userid;

					$p_userids_str = Array();
					$r_userids_str = Array();
					for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
						$p_userids_str[$i] = ':'.$bind_prefix.'p_userids_'.$i;
						$r_userids_str[$i] = ':'.$bind_prefix.'r_userids_'.$i;
						$bind_vars[$bind_prefix.'p_userids_'.$i] = (string)$userids[$i];
						$bind_vars[$bind_prefix.'r_userids_'.$i] = (string)$userids[$i];
					}

					$p_userids_str = implode(',', $p_userids_str);
					$r_userids_str = implode(',', $r_userids_str);
					$userid_cond = ' (p.userid IN ('.$p_userids_str.') OR r.userid IN ('.$r_userids_str.'))';
					$where .= ' AND '.$userid_cond.'
								AND (
									(p.permission = :'.$bind_prefix.'_access AND (
									p.userid <> :'.$bind_prefix.'_public_userid
									OR r.userid <> :'.$bind_prefix.'_public_userid_1
									OR (p.userid = :'.$bind_prefix.'_public_userid_2 AND p.granted = \'1\')
									OR (r.userid = :'.$bind_prefix.'_public_userid_3 AND p.granted = \'1\')
								)
							)';

					$bind_vars[$bind_prefix.'_access'] = $access;
					$bind_vars[$bind_prefix.'_public_userid'] = $public_userid;
					$bind_vars[$bind_prefix.'_public_userid_1'] = $public_userid;
					$bind_vars[$bind_prefix.'_public_userid_2'] = $public_userid;
					$bind_vars[$bind_prefix.'_public_userid_3'] = $public_userid;

					if ($effective) {
						$where .= ' OR (p.permission > :'.$bind_prefix.'_access_effective AND p.granted = \'1\')';
						$bind_vars[$bind_prefix.'_access_effective'] = $access;
					}
					$where .= ') ';
					$having = 'HAVING MIN(p.granted) <> \'0\'';
					$group_by .= ', p.assetid';
				}
			}

			$sql_array = Array(
								'select'		=> '('.$select,
								'from'			=> $from,
								'where'			=> $where,
								'group_by'		=> $group_by,
								'having'		=> $having,
								'order_by'		=> ')'.$order_by,
							);

			try {
				$query = MatrixDAL::preparePdoQuery(implode(' ', $sql_array));
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				$links = MatrixDAL::executePdoGroupedAssoc($query);

				$child_links = Array();
				foreach ($links as $id => $info) {
					$child_links[$id] = $info[0];
				}
			} catch (Exception $e) {
				throw new Exception('Unable to get child links for tree id: '.$treeid.' due to database error: '.$e->getMessage());
			}

			// OK, what we are going to do is set up the effective dependant treeid for each tree link
			for (reset($child_links); NULL !== ($treeid = key($child_links)); next($child_links)) {
				if ($child_links[$treeid]['is_dependant']) {
					$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
					if ($parent_treeid == '') {
						$child_links[$treeid]['dependant_treeid'] = '';
					} else {
						if (!isset($child_links[$parent_treeid])) continue;
						$child_links[$treeid]['dependant_treeid'] = $child_links[$parent_treeid]['dependant_treeid'];
					}
				} else {
					$child_links[$treeid]['dependant_treeid'] = $treeid;
				}
			}//end for

			$this->_tmp['child_links'][$assetid] = $child_links + $shadow_links;

		}//end if

		return $this->_tmp['child_links'][$assetid];

	}//end getAllChildLinks()


	/**
	* For a given asset, get the lineages from the Root folder down to it
	*
	* Also returns basic information about the last link (linkid and link type)
	* linkid => Array ('linkid' => linkid, 'link_type' => link_type, 'lineage' => Array(...))
	*
	* @param string			$assetid			ID of the asset whose linages we want
	* @param int			$result_limit		limits the number of results returned
	* @param string|array	$from_treeid		Uses this/these treeid(s) as the root node
	* @param string|array	$fields				Returns these asset field(s) in lineage details (e.g name, type_code)
	* @param boolean		$only_significant	Only return the significant links, and ignore notice links
	*
	* @return array
	* @access public
	*/
	function getLinkLineages($assetid, $result_limit=0, $from_treeid=NULL, $fields='name', $only_significant=FALSE)
	{
		assert_valid_assetid($assetid);

		$link_lineages = Array();
		$assetids = Array();
		$db = MatrixDAL::getDb();
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';


		if (is_array($fields)) {
				foreach ($fields as $index => $field) {
				$fields[$index] = preg_replace('/[^a-zA-Z_0-9]+/', '', trim($field));
			}
			$fields_str = 'a.'.implode(', a.', $fields);
		} else {
			$fields = preg_replace('/[^a-zA-Z_0-9]+/', '', trim($fields));
			$fields_str = 'a.'.$fields;
		}

		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			// if this asset is a shadow asset the we want the get the lineages of the
			// bridge, and also the lineages of and real assets that this asset is linked under
			$bridge_info   = $this->getAssetInfo(Array($id_parts[0]), '', TRUE);
			$bridge_field  = $this->getAssetInfo(Array($id_parts[0]), '', TRUE, $fields);
			$link_lineages = $this->getLinkLineages($id_parts[0], 0, NULL, $fields);
			$bridge_type   = $bridge_info[$id_parts[0]]['type_code'];

			if (method_exists($bridge_type, 'getLinkLineages') === TRUE) {
				$bridge          = $this->getAsset($id_parts[0]);
				$shadow_lineages = $bridge->getLinkLineages($assetid, $result_limit, $fields, $only_significant);
			} else {
				$shadow_lineages = Array();
			}

			// add the lineage of the actual bridge to the bridge's lineages
			for (reset($link_lineages); NULL !== ($key = key($link_lineages)); next($link_lineages)) {
				$link_lineages[$key]['lineage'][$id_parts[0]] = $bridge_field[$id_parts[0]];

				foreach ($shadow_lineages as $shadow_lineage) {
					$link_lineages[$key]['lineage'] += $shadow_lineage['lineage'];
				}
			}

			$sql = 'SELECT
						l.majorid, '.$fields_str.'
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';

			$where = 'l.minorid = '.MatrixDAL::quote($assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

			try {
				$result = NULL;
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				$result = MatrixDAL::executePdoGroupedAssoc($query);
			} catch (DALException $e) {
				throw new Exception('Could not get shadow links of minor asset ID '.$assetid.' due to database error: '.$e->getMessage());
			}

			// get the lineage of the real assets that this shadow asset is linked under
			// and then append the lineage of that real asset to its lineages
			foreach ($result as $assetid => $value) {
				// Handle inner grouped result, plus pull out of array if the field
				// passed to this function was singular.
				if (is_array($fields) === FALSE) {
					$value = reset($value[0]);
				} else {
					$value = $value[0];
				}

				$lineages = $this->getLinkLineages($assetid);
				for (reset($lineages); NULL !== ($key = key($lineages)); next($lineages)) {
					$lineages[$key]['lineage'][$assetid] = $value;
				}
				$link_lineages = array_merge($link_lineages, $lineages);
				if ($result_limit != 0 && count($link_lineages) >= $result_limit) {
					break;
				}
			}

			if ($result_limit != 0 && count($link_lineages) > $result_limit) {
				$link_lineages = array_slice($link_lineages, 0, $result_limit);
			}

			return $link_lineages;
		}//end if shadow asset

		// Bug Fix #3535 - Binocular icon disappears if NOTICE links overcrowd other links
		if ($only_significant) {
			$link_types = SQ_SC_LINK_SIGNIFICANT;
		} else {
			$link_types = SQ_SC_LINK_ALL;
		}//end if

		// we want to get all the parent links of this assetid and use these
		// as the minorid in the query, effectively getting the lineages of the
		// asset's immediate parents.
		$bind_vars = Array();

		$sub_sql   = 'SELECT l.majorid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l';
		$sub_where = 'l.minorid = :minorid';
		$sub_where .= ' and l.link_type < :link_types';
		$bind_vars['minorid'] = (string)$assetid;
		$bind_vars['link_types'] = $link_types;
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
		$sub_sql  .= $sub_where;

		$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.assetid, '.$fields_str.'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk cl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ON cl.linkid = ct.linkid
						INNER JOIN ('.$sub_sql.') x ON cl.minorid = x.majorid,
					 '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk pl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt ON pl.linkid = pt.linkid
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON a.assetid = pl.minorid
						';

		$where = 'ct.treeid LIKE pt.treeid || '.'\''.'%'.'\''.'
				  AND pt.treeid <= ct.treeid';

		if (!is_null($from_treeid)) {
			if (is_array($from_treeid)) {
				$treeid_bits = Array();
				foreach ($from_treeid as $ftids) {
					$ftid = $ftids[0];
					$treeid_bits[] = '(pt.treeid LIKE '.'\''.($ftid.'%').'\''.')';
				}
				$where .= ' AND ('.implode(' OR ', $treeid_bits).')';
			} else {
				$where .= ' AND pt.treeid LIKE :from_treeid';
				$bind_vars['from_treeid'] = $from_treeid.'%';
			}
		}

		/**
		 * get the parent treeid's for this asset.
		 *
		 * This is done as a separate db call for performance reasons
		 * if it's done as a subquery, then there are multiple
		 * seq-scans of the lnk & lnk_tree tables (which can be quite large)
		 * separating the two calls means the second call looks at
		 * hardcoded id's - which can then be looked up in the index
		 * which is a nice win.
		 *
		 */
		$in_sql = SQ_TABLE_RUNNING_PREFIX.'get_lineage_treeids(:assetid, :tree_size, :rb_date, :link_types)';

		$treeids = array();
		if (MatrixDAL::getDbType() === 'oci') {
			$treeids_query = "SELECT column_value AS treeid FROM TABLE(" . $in_sql . ")";
		} else {
			$treeids_query = "SELECT " . SQ_TABLE_RUNNING_PREFIX . "get_lineage_treeids AS treeid FROM " . $in_sql;
		}
		$treeid_bind_vars['assetid']   	= (string) $assetid;
		$treeid_bind_vars['tree_size'] 	= (int) SQ_CONF_ASSET_TREE_SIZE;
		$treeid_bind_vars['rb_date'] 	= NULL;
		$treeid_bind_vars['link_types'] = (int) $link_types;

		try {
			$tree_query = MatrixDAL::preparePdoQuery($treeids_query);
			foreach ($treeid_bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($tree_query, $bind_var, $bind_value);
			}
			$tree_result = MatrixDAL::executePdoAssoc($tree_query);

		} catch (Exception $e) {
			throw $e;
		}

		/**
		 * we found some tree id's -
		 * quote them and plug them back in to the main query directly.
		 */
		if (!empty($tree_result)) {
			foreach ($tree_result as $row => $info) {
				/**
				 * check if we've seen this tree id before
				 * if we have, skip it.
				 * if we haven't, add it to the list
				 * this check can provide an improvement if an asset
				 * is linked in *lots* of places all over the system
				 */
				$quoted_treeid = MatrixDAL::quote($info['treeid']);
				if (!in_array($quoted_treeid, $treeids)) {
					$treeids[] = $quoted_treeid;
				}
			}

			// Break up the treeids into chunks of 1000 so that oracle does not complain
			$in_clauses = Array();
			foreach (array_chunk($treeids, 999) as $chunk) {
				$in_clauses[] = ' pt.treeid IN ('.implode(', ', $chunk).')';
			}
			$where .= ' AND ('.implode(' OR ', $in_clauses).')';
			unset($in_clauses);
			unset($treeids);

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$sql .= $where.'
				ORDER BY cl.linkid, ct.treeid, pt.treeid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				$result = MatrixDAL::executePdoGroupedAssoc($query);
			} catch (Exception $e) {
				throw $e;
			}
		} else {
			/**
			 * no parent tree ids? then we don't have any lineages
			 * we are on the top level.
			 */
			$result = array();
		}

		$asset_links = $this->getLinks($assetid, $link_types, '', TRUE, 'minor');

		$link_info = Array();
		foreach ($asset_links as $link_data) {
			$link_info[$link_data['majorid']][] = $link_data;
		}

		$result_count = 0;
		foreach (array_values($result) as $link_tree) {
			// work out some values we are going to need
			$parent_data = $link_tree[count($link_tree) -1];
			$majorid = $parent_data['assetid'];

			if (!isset($link_info[$majorid])) continue;

			foreach ($link_info[$majorid] as $link_data) {
				$link_type = $link_data['link_type'];
				$linkid = $link_data['linkid'];
				$link_value = $link_data['value'];

				$asset_lineage = Array();
				foreach ($link_tree as $tree_data) {
					if ($tree_data['assetid'] == $majorid) continue;
					if (is_array($fields)) {
						$asset_lineage[$tree_data['assetid']] = Array();
						foreach ($fields as $field_name) {
							$asset_lineage[$tree_data['assetid']][$field_name] = $tree_data[$field_name];
						}
					} else {
						$asset_lineage[$tree_data['assetid']] = $tree_data[$fields];
					}
				}

				if (is_array($fields)) {
					$asset_lineage[$tree_data['assetid']] = Array();
					foreach ($fields as $field_name) {
						$asset_lineage[$parent_data['assetid']][$field_name] = $parent_data[$field_name];
					}
				} else {
					$asset_lineage[$parent_data['assetid']] = $parent_data[$fields];
				}

				$link_lineages[] = Array(
									'linkid'	=> $linkid,
									'link_type'	=> $link_type,
									'link_value'=> $link_value,
									'lineage'	=> $asset_lineage,
								   );
			}//end foreach

			if ($result_limit != 0 && $result_count == $result_limit) {
				break;
			}
			$result_count++;
		}//end foreach result

		return $link_lineages;

	}//end getLinkLineages()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* It returns Array(int => string)	returns an array of assetids and their type code
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$effective			If effective permissions should be considered or not.
	* @param int			$min_height			Optional parameter that specifies minimum number of levels above
	*											the specified asset for a parent to qualify (immediate parent is 1)
	* @param int			$max_height			Optional parameter that specifies maximum number of levels above
	*											the specified asset for a parent to qualify to be returned
	* @param boolean		$ignore_bridge		Optional parameter that, if true, will prevent this function from
	*											requesting parents from shadow asset bridges
	* @param string|array	$link_value_wanted	Link Value Wanted
	* @param string			$bind_var_prefix	Prefix for bind variables. Use if this query is going to be used
	*											more than once in a larger query (say, a UNION).
	*
	* @return array
	* @access public
	* @see generateGetParentsQuery()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE, $sort_by=NULL, $access=NULL, $effective=TRUE, $min_height=NULL, $max_height=NULL, $ignore_bridge=FALSE, $link_value_wanted=NULL, $bind_var_prefix='gp_')
	{
		// check if we are getting parents for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);

			$parents = Array();
			if (!$ignore_bridge) {
				$parents = $asset->getParents($assetid, $type_code, $strict_type_code);
			}

			$bind_vars = array();
			$db = MatrixDAL::getDb();
			$sql = 'SELECT
						sl.majorid
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk sl';

			$where = 'sl.minorid = :shadow_assetid AND sl.link_type != :link_notice';
			$bind_vars['shadow_assetid'] = $assetid;
			$bind_vars['link_notice'] = SQ_LINK_NOTICE;

			if (!empty($type_code)) {
				$sql .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a
						 ON sl.majorid = a.assetid ';
				$type_codes_quoted = Array();
				$all_type_codes = is_array($type_code) ? $type_code : Array($type_code);
				foreach ($all_type_codes as $tc) {
					$type_codes_quoted[] = MatrixDAL::quote($tc);
				}
				if (!$strict_type_code) {
					foreach ($all_type_codes as $tc) {
						$ancestors = $this->getTypeDescendants($tc);
						foreach ($ancestors as $ancestor) {
							$type_codes_quoted[] = MatrixDAL::quote($ancestor);
						}
					}
				}
				if (count($type_codes_quoted) == 1) {
					$where .= ' AND a.type_code = '.reset($type_codes_quoted);
				} else {
					$where .= ' AND a.type_code IN ('.implode(',', $type_codes_quoted).')';
				}

				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'sl');

			$query = MatrixDAL::preparePdoQuery($sql.$where);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}

			try {
				$shdw_parentids = MatrixDAL::executePdoAssoc($query, 0);
			} catch (Exception $e) {
				throw new Exception('Unable to get parents from due to the following database error:'.$e->getMessage());
			}//end try catch

			$shdw_parents = $this->getAssetInfo($shdw_parentids, Array(), TRUE, 'type_code');

			$real_parents = Array();
			// we also need the parents of the majorid from the shadow asset link table
			foreach ($shdw_parentids as $parentid) {
				$real_parents += $this->getParents($parentid, $type_code, $strict_type_code, $sort_by, $access);
			}

			$parents = $parents + $shdw_parents + $real_parents;

			$this->forgetAsset($asset);
			return $parents;
		}//end if shadow asset

		// if value is not null and not array, convert it to array
		if (!is_null($link_value_wanted) && !is_array($link_value_wanted)) {
			$link_value_wanted = Array(
									'link_value' 	=> Array($link_value_wanted),
									'equal' 		=> TRUE,
								 );
		}

		$ret_val = $this->generateGetParentsQuery($assetid, $type_code, $strict_type_code, $sort_by, $access, $effective, $min_height, $max_height, $link_value_wanted);
		if (empty($ret_val)) return Array();

		$query = MatrixDAL::preparePdoQuery(implode(' ', $ret_val['sql_array']));
		foreach ($ret_val['bind_vars'] as $bind_var => $bind_value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
		}

		$result = MatrixDAL::executePdoGroupedAssoc($query);
		$parents = Array();
		foreach ($result as $assetid => $row) {
			$parents[$assetid] = $row[0]['type_code'];
		}
		unset($result);

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* The query generated to get the children is actually constructed in the Asset Manager's
	* generateGetChildrenQuery function.
	* It returns Array(int => string)	returns an array of assetids and their type code
	* The function might return a list of users without their type code (i.e.: when shadow assets are returned)
	*
	* @param int|string		$assetid				the id of the asset to get its children for
	* @param string|array	$type_code				the type of asset that is linked
	*												(eg 'Page', 'File', etc)
	*												if an array returns link if matches any
	*												of the array values
	* @param boolean		$strict_type_code		whether we are finding assets that are just a $type_code
	*												or $type_code and any of it's sub-classes
	* @param boolean		$dependant				if TRUE, results will be filtered to assets that are dependants.
	*												If FALSE, results will all not be dependants.
	*												If NULL, results will not be filtered
	* @param string			$sort_by				a field in the asset table you would like the results
	*												sorted by (eg. name, short_name etc)
	* @param int			$access					SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*												Specify that links should be returned for assets the user has read,
	*												write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$effective				If effective permissions should be considered or not.
	* @param int			$min_depth				Optional parameter that specifies minimum number of levels below
	*												the specified asset for a child to qualify (immediate child is 1)
	* @param int			$max_depth				Optional parameter that specifies maximum number of levels below
	*												the specified asset for a child to qualify to be returned
	* @param boolean		$direct_shadows_only	If TRUE, ONLY direct shadow asset children will be returned
	*												If FALSE, ALL the shadow asset children will be returned
	* @param string|array	$link_value_wanted		Link value wanted
	* @param array			$link_types_wanted		link_type we want our results to be filtered by. By default it uses
	*												empty array which mean it will return all the link_types
	*												Other values that can be used in the array : SQ_LINK_TYPE_1, SQ_LINK_TYPE_2,
	*												SQ_LINK_TYPE_3, SQ_LINK_NOTICE, SQ_SC_LINK_FRONTEND_NAV,
	*												SQ_SC_LINK_BACKEND_NAV, SQ_SC_LINK_SIGNIFICANT, SQ_SC_LINK_WEB_PATHS
	* @param boolean		$expand_shadows			Get shadow children of shadow asset links. Enable this where necessary but keep performance in mind.
	*
	* @return array
	* @access public
	* @see generateGetChildrenQuery()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE, $min_depth=NULL, $max_depth=NULL, $direct_shadows_only=TRUE, $link_value_wanted=NULL, Array $link_types_wanted=Array(), $expand_shadows=FALSE)
	{
		assert_valid_assetid($assetid);
		// check if we are getting children for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$children = Array();
			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);
			if (!is_null($asset)) {
				if (!method_exists($asset, 'getChildren')) {
					trigger_localised_error('SYS0204', E_USER_WARNING, $asset->name);
				} else {
					$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $dependant, $sort_by);
				}
				$this->forgetAsset($asset);
			} else {
				trigger_localised_error('SYS0206', E_USER_WARNING, $real_assetid);
			}
			return $children;
		}
		// we are not getting children for a shadow asset, so if we are getting children of
		// an asset that handles shadow assets, palm the request off to it
		$asset = $this->getAsset($assetid);
		if (is_null($asset)) return Array();
		if (implements_interface($asset, 'bridge')) {
			$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $dependant, $sort_by);
			$this->forgetAsset($asset);
			return $children;
		}

		// if value is not null and not array, convert it to array
		if (!is_null($link_value_wanted) && !is_array($link_value_wanted)) {
			$link_value_wanted = Array(
									'link_value' 	=> Array($link_value_wanted),
									'equal' 		=> TRUE,
								 );
		}

		$ret_val = $this->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $dependant, $sort_by, $access, $effective, TRUE, $min_depth, $max_depth, $direct_shadows_only, $link_value_wanted, 'gc_', $link_types_wanted);

		$this->forgetAsset($asset);
		unset($asset);

		if (empty($ret_val)) return Array();
		try  {
			$sql_array = $ret_val['sql_array'];
			$bind_vars = $ret_val['bind_vars'];
			$query = MatrixDAL::preparePdoQuery(implode(' ', $sql_array));
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get children for asset: '.$assetid.' due to database error: '.$e->getMessage());
		}

		if ($expand_shadows){
			foreach (array_keys($result) as $child_id){
				$id_parts = explode(':', $child_id);
				if (isset($id_parts[1])) {
					$children = $this->getChildren($child_id, $type_code, $strict_type_code, $dependant, $sort_by);
					$result += $children;
				}
			}
		}

		if (!is_null($sort_by)) {
			$children = Array();
			foreach ($result as $assetid => $asset_info) {
				$children[$assetid] = $asset_info[0]['type_code'];
			}
			return $children;
		} else {
			return $result;
		}

	}//end getChildren()


	/**
	* Get all dependent children of an asset in the various tress in which it exists
	*
	* It returns an array of the form Array([assetid] => [type_code])
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	*/
	function getDependantChildren($assetid, $type_code='', $strict_type_code=TRUE)
	{
		$children = Array();

		$asset = $this->getAsset($assetid);
		if (!is_null($asset)) {

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'major', NULL, 1);
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				if (!$dependant_links[$k]['is_dependant']) continue;
				$children[$dependant_links[$k]['minorid']]	= Array (
																0	=> Array (
																		'type_code'	=> $dependant_links[$k]['minor_type_code'],
																	   ),
															  );
			}

			// get ALL kids, regardless of type code, so we can chase them down
			// for the type code we want
			$all_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 1);
			for (reset($all_links); NULL !== ($k = key($all_links)); next($all_links)) {
				// must still be dependent
				if (!$all_links[$k]['is_dependant']) continue;
				$children = $children + $this->getDependantChildren($all_links[$k]['minorid'],$type_code, $strict_type_code);
			}

			$this->forgetAsset($asset);
		}

		unset($asset);
		return $children;

	}//end getDependantChildren()


	/**
	* Get all dependent parents of an asset in the various trees in which it exists
	*
	* It returns an array of assetids
	*
	* @param int			$assetid				the id of the asset to get its children for
	* @param string|array	$type_code				the type of asset that is linked
	*												(eg 'Page', 'File', etc)
	*												if an array returns link if matches any
	*												of the array values
	* @param boolean		$strict_type_code		whether we are finding assets that are just a $type_code
	*												or $type_code and any of it's sub-classes
	* @param boolean		$include_all_dependants	When FALSE, only show the top level of dependant parents
	*
	* @return array
	* @access public
	*/
	function getDependantParents($assetid, $type_code='', $strict_type_code=TRUE, $include_all_dependants=TRUE)
	{
		$parents = Array();

		$asset = $this->getAsset($assetid);
		if (!is_null($asset)) {

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'minor', NULL, 1);

			if (empty($dependant_links) && !$include_all_dependants) {
				$parents[] = $assetid;
			}
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				$parentid = $dependant_links[$k]['majorid'];
				if ($include_all_dependants) $parents[] = $parentid;
				$parents = array_merge($parents, $this->getDependantParents($parentid, $type_code, $strict_type_code, $include_all_dependants));
			}

			$this->forgetAsset($asset);
		}

		unset($asset);
		return $parents;

	}//end getDependantParents()


	/**
	* Return an array of asset ids in the order that they propogate out from the passed in assetIDs
	*
	* The array returned for each asset is in the following format:
	* <PRE>
	* Array(
	*   'assetid'    => '155',
	*   'majorid'    => '151',
	*   'type_code'  => 'page_standard',
	*   'status'     => '2',
	*   'name'       => '(( page2 ))',
	*   'short_name' => '(( page2 ))',
	*   'path'       => 'page2',
	*   'sort_order' => '0',
	* )
	* </PRE>
	*
	* @param int|array	$majorids			the majorids to traverse the tree from
	* @param int		$levels				the number of levels to traverse (if null, traverse all levels)
	* @param array		$exclude_list		an array of assetid to exclude from the tree
	* @param int		$link_type			the type of wanted links
	* @param array		$include_type_list	Array of asset types to include
	* <pre>
	*										Array(
	*											'type_code' => Array(
	*															0	=> 'type_code1',
	*															1	=> 'type_code2'
	*														   ),
	*											'inherit'	=> Array(
	*															0	=> TRUE,
	*															1	=> FALSE,
	*														   ),
	*										)
	* </pre>
	* @param boolean	$include_dependants	a boolean
	*
	* @return array
	* @access public
	*/
	function getAssetTree($majorids, $levels=NULL, $exclude_list=Array(), $link_type=SQ_SC_LINK_FRONTEND_NAV, $include_type_list=Array(), $include_dependants=TRUE)
	{
		$tree_data = Array();
		if (!is_array($majorids)) $majorids = Array($majorids);
		$this->_getAssetTree($majorids, $tree_data, $levels, $exclude_list, $link_type, $include_type_list, $include_dependants);
		return $tree_data;

	}//end getAssetTree()


	/**
	* Does the work for getAssetTree
	*
	* @param int|array	$majorids			the majorids to traverse the tree from
	* @param array		&$tree_data			the data that is created recursively
	* @param int		$levels				the number of levels to traverse (if NULL, traverse all levels)
	* @param array		$exclude_list		an array of assetid to exclude from the tree
	* @param int		$link_type			the type of wanted links
	* @param array		$include_type_list	Array of asset types to include
	* <pre>
	*										Array(
	*											'type_code' => Array(
	*															0	=> 'type_code1',
	*															1	=> 'type_code2'
	*														   ),
	*											'inherit'	=> Array(
	*															0	=> TRUE,
	*															1	=> FALSE,
	*														   ),
	*										)
	* </pre>
	* @param boolean	$include_dependants	Whether to include dependants
	*
	* @return boolean
	* @access private
	*/
	function _getAssetTree($majorids, &$tree_data, $levels, $exclude_list, $link_type, $include_type_list, $include_dependants)
	{
		// Stuff that is only needed once per design file
		$db = MatrixDAL::getDb();

		$assetids  = Array();
		$majorids_str = '';

		foreach ($majorids as $id) {
			if (empty($tree_data[$id])) {
				$majorids_str .= MatrixDAL::quote((string) $id).',';
			} else {
				foreach ($tree_data[$id] as $row) {
					$assetids[] = (int) $row['assetid'];
				}
			}
		}

		// remove the last reailing comma from the string
		$majorids_str = rtrim($majorids_str, ',');

		if (empty($majorids_str)) return $assetids;

		$exclude_list_before_quote = $exclude_list;
		for (reset($exclude_list); NULL !== ($k = key($exclude_list)); next($exclude_list)) {
			$exclude_list[$k] = MatrixDAL::quote((string) $exclude_list[$k]);
		}

		$exclude_str = implode(',', $exclude_list);

		static $USERIDS_COND = NULL;

		if (is_null($USERIDS_COND)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$USERIDS_COND = '';
			} else {
				$current_user = $this->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
				$userids = $current_user->getUserGroups();
				$this->forgetAsset($current_user);
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$userids[] = $this->getSystemAssetid('public_user');
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = MatrixDAL::quote((string)$userids[$i]);
				}
				$USERIDS_COND = 'AND p.userid  IN ('.implode(',', $userids).')';
			}

			if (!empty($USERIDS_COND)) {
				$USERIDS_COND .= '
					GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order, p.granted
					HAVING MIN(p.granted) = \'1\'';
			}
		}


		$include_types_str = '';
		if (isset($include_type_list['type_code'])) {
			$include_types = Array();
			$include_inherit = Array();

			if (isset($include_type_list['inherit'])) {
				$include_inherit = $include_type_list['inherit'];
			}

			foreach ($include_type_list['type_code'] as $include_type) {
				if (!empty($include_type)) {
					if (isset($include_inherit) && !empty($include_inherit) && array_shift($include_inherit)) {
						$include_types = array_merge($this->getTypeDescendants($include_type, TRUE), $include_types);
					} else {
						$include_types[] = $include_type;
					}
				}
			}

			if (!empty($include_types)) {
				$include_types_str = '\''.implode('\',\'', $include_types).'\'';
			}
		}


		$sql    = 'SELECT DISTINCT a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order
				   FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON a.assetid = l.minorid
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path pt ON a.assetid = pt.assetid ';
		if (!empty($USERIDS_COND)) {
			$sql   .= '
				INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_perm p ON a.assetid = p.assetid ';
		}

		$majorids_array = explode(',', $majorids_str);
		$in_clause = Array();

		foreach (array_chunk($majorids_array, 999) as $chunk) {
			$in_clause[] = ' (l.majorid IN ('.implode(', ', $chunk).'))';
		}


		$where = ' ('.implode(' OR ',$in_clause).')  AND ('.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', $link_type).' > 0) ';

		if (!empty($exclude_list)) {
			$where .= 'AND a.assetid NOT IN ('.$exclude_str.')';
		}

		if (!empty($include_types_str)) {
			$where .= ' AND a.type_code IN ('.$include_types_str.')';
		}

		if (!$include_dependants) {
			$where .= ' AND l.is_dependant = \'0\'';
		}

		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		if (!empty($USERIDS_COND)) {
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'p');
		}
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where .= ' '.$USERIDS_COND.'
				   ORDER BY l.majorid, l.sort_order';

		try {
			$result = MatrixDAL::executeSqlAssoc($sql.$where);
		} catch (Exception $e) {
			throw new Exception('Unable to get order of assets as they propogate from passed majorid(s): '.array_contents($majorids).' due to database error: '.$e->getMessage());
		}


		$assetids = Array();
		foreach ($result as $row) {
			if (!($row['status'] & (SQ_STATUS_LIVE | SQ_STATUS_LIVE_APPROVAL))) {
				$asset = $this->getAsset($row['assetid']);
				$read_access = $asset->readAccess();

				$row['name'] = $asset->name;
				$row['short_name'] = $asset->short_name;

				if ($row['status'] & SQ_SC_STATUS_NOT_LIVE) {
					// somewhere between under construction and live so we show this by altering the name
					$row['name']       = '(( '.$row['name'].' ))';
					$row['short_name'] = '(( '.$row['short_name'].' ))';
				}
				$this->forgetAsset($asset);
				unset($asset);
				if (!$read_access) continue;
			}
			$assetids[] = (int) $row['assetid'];
			if (!isset($tree_data[$row['majorid']])) {
				$tree_data[$row['majorid']] = Array();
			}
			$tree_data[$row['majorid']][$row['assetid']] = $row;

		}

		if (!empty($assetids) && ($levels > 1 || is_null($levels))) {
			$levels--;
			$this->_getAssetTree($assetids, $tree_data, $levels, $exclude_list_before_quote, $link_type, $include_type_list, $include_dependants);
		}
		return TRUE;

	}//end _getAssetTree()


	/**
	* Returns the treeids of any links between the passed assetid(s) and any parents
	*
	* If a single assetid is supplied, a simple array of treeids is returned.  If an array
	* of assetids is supplied then the result is indexed by assetid and each element is an array
	* of treeids for that assetid.
	*
	* @param int|array	$assetid	the id of the asset passed, or array of assetids
	* @param int		$link_type	the type of wanted links
	*
	* @return array
	* @access public
	*/
	function getAssetTreeids($assetid, $link_type=NULL)
	{
		$db = MatrixDAL::getDb();
		if (is_array($assetid)) {
			$assetids_set = Array();
			foreach ($assetid as $this_assetid) {
				assert_valid_assetid($this_assetid);
				$assetids_set[] = MatrixDAL::quote($this_assetid);
			}
		} else {
			assert_valid_assetid($assetid);
		}

		if (empty($link_type)) {
			$link_type = SQ_SC_LINK_SIGNIFICANT;
		}

		if (is_array($assetid)) {
			$sql = 'SELECT l.minorid, t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON l.linkid = t.linkid';
			$where = 'WHERE l.minorid IN ('.implode(', ', $assetids_set).')';
		} else {
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
					INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON l.linkid = t.linkid';
			$where  = 'WHERE l.minorid = '.MatrixDAL::quote($assetid);
		}

		$where .= '	AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', $link_type).' > 0';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.' '.$where);
			if (is_array($assetid)) {
				$result = MatrixDAL::executePdoGrouped($query);
			} else {
				$result = MatrixDAL::executePdoAssoc($query, 0);
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get tree ID for asset: '.$e->getMessage());
		}

		return $result;

	}//end getAssetTreeids()


	/**
	* Returns the treeids for linkids passed in.
	*
	* Accepts a single treeid int or an array of treeids for which to get link ids
	*
	* @param int|array	$linkid	the id of the link passed, or array of linkids
	*
	* @return array
	* @access public
	*/
	function getLinkTreeid($linkid)
	{
		if (!is_array($linkid)) {
			$linkid = Array($linkid);
		}

		$db = MatrixDAL::getDb();
		try {
			$bind_vars['linkid'] = $linkid;
			$result = MatrixDAL::executeGrouped('core', 'getLinkTreeid', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Failed to get link tree id: '.$e->getMessage());
		}

		return $result;

	}//end getLinkTreeid()


	/**
	* Return TRUE if the passed assetid is in the trash, FALSE otherwise
	*
	* Note that if the passed assetid is the id of the trash folder, FALSE will be returned
	* NOTE: If $exclusively is TRUE, TRUE will be returned for assets that are not linked anywhere outside the trash including non existent assets
	*
	* @param int		$assetid		the asset id to check the trash for
	* @param boolean	$exclusively	is the asset ONLY in the trash
	*									(ie. does it have no other significant links
	*									outside the trash?)
	*
	* @return boolean
	* @access public
	*/
	function assetInTrash($assetid, $exclusively=FALSE)
	{
		if (FALSE !== strpos($assetid, ':')) {
			return $this->assetInTrash(strtok($assetid, ':'), $exclusively);
		}
		$trash = $this->getSystemAsset('trash_folder');

		// if we are being asked "Is the trash in the trash"
		// lets say "Are you kidding?"
		if ($assetid == $trash->id) return FALSE;

		$db = MatrixDAL::getDb();

		// we need to work out the treeid of the trash
		$sub_sql = 'SELECT (t.treeid || \'%\')
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid ';
		$sub_where = 'l.minorid = :trashid';
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
		$sub_sql .= $sub_where;
		$sub_sql = db_extras_modify_limit_clause($sub_sql, MatrixDAL::getDbType(), 1);

		$sql   = 'SELECT COUNT(*)
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid';
		$where = 'l.minorid = :assetid
				  AND t.treeid '.(($exclusively) ? 'NOT ' : '').'LIKE ('.$sub_sql.')';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$query = MatrixDAL::preparePdoQuery($sql.$where);
		MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
		MatrixDAL::bindValueToPdo($query, 'trashid', $trash->id);
		$result = MatrixDAL::executePdoOne($query);

		if ($exclusively) {
			return ($result == 0);
		} else {
			return ($result > 0);
		}

	}//end assetInTrash()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param int			$ignore_linkid		ignore the link represented by this link
	*											id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$id_parts = explode(':', $assetid);

		// shadow asset found, pipe the request off to the bridge
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);
			$results = $asset->countLinks($assetid, $side_of_link, $link_types, $type_code, $strict_type_code, $ignore_linkid);

			$this->forgetAsset($asset);
			return $results;
		}

		$bind_vars = Array();

		$extra_table = '';
		$where = 'l.'.$side_of_link.'id = :assetid';
		$bind_vars['assetid'] = $assetid;
		if ($link_types) {
			$where .= ' AND '.db_extras_bitand(MatrixDAL::getDbType(), 'link_type', $link_types).' > 0';
		}
		if ($type_code) {
			$extra_table .= ', '.SQ_TABLE_RUNNING_PREFIX.'ast a';
			$where .= ' AND l.minorid = a.assetid ';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$type_code_cond = '';
			if (is_array($type_code)) {
				for ($i = 0; $i < count($type_code); $i++) {
					$type_code[$i] = MatrixDAL::quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.MatrixDAL::quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				$where .= ' AND a.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE inhd_type_code '.$type_code_cond.'
												)';
			}
		}//end if

		if ($ignore_linkid) {
			$where .= ' AND l.linkid <> :ignore_linkid';
			$bind_vars['ignore_linkid'] = $ignore_linkid;
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l '.$extra_table.'
				'.$where;

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoOne($query);
		} catch (DALException $e) {
			throw new Exception ('Unable to count '.$link_type.' links from asset ID #'.$assetid.' due to database error: '.$e->getMessage());
		}

		return $result;

	}//end countLinks()


	/**
	* Create a link between the two supplied assets
	*
	* It returns the linkid of the created link
	*
	* @param Asset		$major		the major asset that we are linking from
	* @param Asset		$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	* @param string		$locked		whether or not this asset's link is locked
	*
	* @return int
	* @access public
	*/
	function createAssetLink(Asset $major, Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE, $locked='0')
	{
		// Initial Checks
		if (!$major->id) return 0;
		assert_is_a($minor, 'asset');
		if (!is_null($sort_order)) {
			$sort_order = (int) $sort_order;
		}

		assert_false(!($link_type & SQ_SC_LINK_SIGNIFICANT) && $dependant, 'In order for a link to be dependant it must also be a significant link');
		assert_false(!($link_type & SQ_SC_LINK_SIGNIFICANT) && ($locked == '1'), 'In order for a link to be locked it must also be a significant link');

		$original_link_type = $link_type;
		$link_type = (int) $link_type;
		if ($link_type != $original_link_type) {
			trigger_localised_error('SYS0241', E_USER_WARNING);
			return 0;
		}

		// Handle shadow assets
		$majorid_parts = explode(':', $major->id);
		$minorid_parts = explode(':', $minor->id);
		if (isset($minorid_parts[1])) {
			// minor is a shadow
			if (isset($majorid_parts[1]) || ($majorid_parts[0] == $minorid_parts[0])) {
				// major is a shadow or the minor's bridge, so get the bridge to handle the linking
				$bridge = $this->getAsset($majorid_parts[0]);
				$linkid = $bridge->createAssetLink($major, $minor, $link_type, $value, $sort_order, $dependant, $exclusive, $moving);
				$this->forgetAsset($bridge);
			} else {
				// we are linking a shadow asset under a normal asset
				$linkid = $this->createShadowAssetLink($major, $minor, $link_type, $value);
			}
			return $linkid;
		}//end if shadow asset


		// Use Master DB since we are going to change stuff
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$is_oci = (MatrixDAL::getDbType() === 'oci') ? TRUE : FALSE;

		// Can't link to the new parent if it is only in the trash
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
			if ($this->assetInTrash($major->id, TRUE)) {
				trigger_localised_error('SYS0223', E_USER_WARNING, $major->id);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return 0;
			}
		}

		// prepare the link - do the minor first because the major gets the final say in linking
		$minor->prepareLink($major, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
		$major->prepareLink($minor, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);

		// First, we should check that we don't already have a link of this type
		$current_link = $this->getLinkByAsset($major->id, $minor->id, $link_type, $value);
		if (empty($current_link) === FALSE) {
			// The link we are creating already exists, so throw an error
			// unless the major is the trash, in which case we just pretend we linked ok
			$trash_folder_id = $this->getSystemAssetid('trash_folder');
			if ($trash_folder_id != $major->id) {
				trigger_localised_error('SYS0192', E_USER_WARNING, "$major->name (#$major->id)", "$minor->name (#$minor->id)");
			}
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $current_link['linkid'];
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
			// Check if we are allowed to link to these type of assets - if we
			// are moving though, we need a different check which is done in moveLink()
			if (!$moving) {
				if (($err_msg = $major->canCreateLink($minor, $link_type, $exclusive)) !== TRUE) {
					trigger_localised_error('SYS0301', E_USER_WARNING, $err_msg);
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return 0;
				}
			}
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// Acquire lock if necessary, but remember its previous state so we can leave it as we found it
			$lock_info = @$this->getLockInfo($major, 'links');
			$parent_was_locked = !empty($lock_info);
			if (!$this->acquireLock($major->id, 'links')) {
				trigger_localised_error('CORE0012', E_USER_WARNING, $major->name);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return 0;
			}
		}

		// Change to DB3 to deal with the link tables
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$db = MatrixDAL::getDB();

		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {

			// This is a significant link

			// Check we aren't moving the minor asset under itself
			$sql = 'SELECT
						COUNT(DISTINCT ct.linkid)
					FROM sq_ast_lnk_tree pt,
						sq_ast_lnk_tree ct INNER JOIN sq_ast_lnk cl ON ct.linkid = cl.linkid
					WHERE
							ct.treeid LIKE (pt.treeid || \'%\')
						AND	ct.treeid >= pt.treeid
						AND	pt.linkid IN (SELECT linkid
										  FROM sq_ast_lnk
										  WHERE minorid = :minorid)
						AND	cl.minorid = :majorid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
				MatrixDAL::bindValueToPdo($query, 'minorid', $minor->id);
				$moving_under = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to determine if minor asset "'.$minor->name.'" (#'.$minor->id.') in new link is being linked underneath itself, due to database error: '.$e->getMessage());
			}

			if ($moving_under) {
				trigger_localised_error('CORE0114', E_USER_WARNING, $minor->name, $minor->id, $major->name, $major->id);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); //end db3
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); //end db2
				return 0;
			}

			// Check that the major is linked somewhere
			try {
				$bind_vars = Array(
								'minorid'	=> $major->id,
							 );
				$existing_treeid = MatrixDAL::executeOne('core', 'getTreeIdsByMinorid', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to determine if major asset "'.$major->name.'" (#'.$major->id.') in new link is linked somewhere else, due to database error: '.$e->getMessage());
			}

			$existing_treeid = (string) $existing_treeid;

			// if we aren't linked anywhere we can't be linked to, sorry
			// NOTE: exception to rule is root folder
			if ((string) $existing_treeid == '-' && get_class($major) != 'Root_Folder') {
				trigger_localised_error('SYS0240', E_USER_WARNING, $major->name, $major->id);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); //end db3
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); //end db2
				return 0;
			}

		}//end if significant link

		// Make sure the sort order is in a valid range
		$sql = 'SELECT
					COUNT(*) as count, MAX(sort_order) as max
				FROM
					sq_ast_lnk
				WHERE
					majorid = :majorid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
			$result = MatrixDAL::executePdoAll($query);
			$row = $result[0];
			unset($result);
		} catch (Exception $e) {
			throw new Exception('Unable to determine sort order range for new link between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
		}

		$max = ($row['count'] > 0) ? (int) $row['max'] + 1 : 0;
		if (is_null($sort_order) || (int) $sort_order > (int) $max || (int) $sort_order < 0) {
			$sort_order = (int) $max;
		}

		// Get Link ID
		$linkid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_lnk_seq'));

		// Update sort order for other children of the new parent
		$sql = 'UPDATE
					sq_ast_lnk
				SET
					sort_order = sort_order + 1
				WHERE
						majorid			=  :majorid
						AND	sort_order	>= :sort_order';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
			MatrixDAL::bindValueToPdo($query, 'sort_order', $sort_order);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to shift sort orders of children of "'.$major->name.'" (#'.$major->id.') up one, due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// We insert into the link table first to preserve referential integrity
		$sql = 'INSERT INTO
					sq_ast_lnk
					(
						linkid,
						majorid,
						minorid,
						link_type,
						value,
						sort_order,
						is_dependant,
						is_exclusive,
						updated,
						updated_userid,
						locked
					)
					VALUES
					(
						:linkid,
						:majorid,
						:minorid,
						:link_type,
						:value,
						:sort_order,
						:is_dependant,
						:is_exclusive,
						:updated,
						:updated_userid,
						:link_locked
					)';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
			MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
			MatrixDAL::bindValueToPdo($query, 'minorid', $minor->id);
			MatrixDAL::bindValueToPdo($query, 'link_type', $link_type);
			MatrixDAL::bindValueToPdo($query, 'value', $value);
			MatrixDAL::bindValueToPdo($query, 'sort_order', $sort_order);
			MatrixDAL::bindValueToPdo($query, 'is_dependant', (($dependant) ? '1' : '0'));
			MatrixDAL::bindValueToPdo($query, 'is_exclusive', (($exclusive) ? '1' : '0'));
			MatrixDAL::bindValueToPdo($query, 'updated', ts_iso8601(time()));
			MatrixDAL::bindValueToPdo($query, 'updated_userid', $GLOBALS['SQ_SYSTEM']->currentUserId());
			MatrixDAL::bindValueToPdo($query, 'link_locked', (($locked == '1' && $link_type <= 2) ? '1' : '0'));
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to add new link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {
			// This is a significant link, so do stuff with the tree
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			// OK, what we are going to do is get a treeid and then do a "INSERT INTO ... SELECT FROM"
			// into the tree table of all the links that are under the minor asset
			$sql   = 'SELECT
							t.treeid, t.num_kids
						FROM
							sq_ast_lnk_tree t
							INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid
						WHERE
							l.minorid = :minorid';

			try {
				$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'minorid', $minor->id);
				$result = MatrixDAL::executePdoAll($query);
				if (empty($result)) {
					$minor_tree = Array();
				} else {
					$minor_tree = $result[0];
				}
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to get existing child tree information of "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
			}

			if (empty($minor_tree)) {
				$minor_tree = Array('treeid' => '', 'num_kids' => 0);
			} else {
				$minor_tree['treeid'] = (string) $minor_tree['treeid'];
				$minor_tree['num_kids'] = (int) $minor_tree['num_kids'];
			}

			$i = 0;
			$requires_update = FALSE;

			// Yes, this is a do-while loop. Live with it. The following code is to stop
			// constraint violations which may occur if two processes are trying to
			// create a link under the same branch, simultaneously
			do {
				// try to find the last added treeid on the branch that existing_treeid is the parent of
				$sql = 'SELECT
							SUBSTR(ct.treeid, (LENGTH(ct.treeid) + 1) - '.SQ_CONF_ASSET_TREE_SIZE.') AS treeid
						FROM
							sq_ast_lnk_tree ct';

				if ($existing_treeid != '-') {

					$sql .= ' WHERE
						ct.treeid LIKE :existing_treeid_wildcard';
					if (!empty($existing_treeid)) {
						$sql .= ' AND ct.treeid > :existing_treeid';
					}
					$sql .= ' AND LENGTH(ct.treeid) = :length';
				} else {
					$sql .= ' WHERE LENGTH(ct.treeid) = :length';
				}
				$sql .= ' AND linkid > 0 ORDER BY treeid DESC';

				try {
					$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

					$bind_vars = Array();
					if ($existing_treeid != '-') {
						if (!$is_oci) {
							$sql = str_replace(':existing_treeid_wildcard', MatrixDAL::quote($existing_treeid.'%'), $sql);
						} else {
							$bind_vars['existing_treeid_wildcard'] = $existing_treeid.'%';
						}
						if (!empty($existing_treeid)) {
							$bind_vars['existing_treeid'] = $existing_treeid;
						}
						$bind_vars['length'] = strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE;
					} else {
						$bind_vars['length'] = SQ_CONF_ASSET_TREE_SIZE;
					}
					$query = MatrixDAL::preparePdoQuery($sql);
					foreach ($bind_vars as $bind_name => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_name, $bind_value);
					}
					$last_treeid = MatrixDAL::executePdoOne($query);
					//if (!empty($last_treeid)) {
						//$last_treeid = $last_treeid;
					//}
				} catch (Exception $e) {
					throw new Exception('Unable to get tree data due to database error: '.$e->getMessage());
				}

				$treeid = ($existing_treeid == '-') ? '' : $existing_treeid;

				// if we couldn't find any treeids on this branch, then we can just
				// do a straight inert and break the loop as we are done
				if (empty($last_treeid)) {
					$sql = 'asset_link_treeid_convert(\'0\', \'1\', '.SQ_CONF_ASSET_TREE_BASE.', '.SQ_CONF_ASSET_TREE_SIZE.')';
					$free_treeid = db_get_function_result(MatrixDAL::getDbType(), $sql);

					$new_treeid = ($existing_treeid == '-') ? $free_treeid : $existing_treeid.$free_treeid;

					$sql = 'INSERT INTO
								sq_ast_lnk_tree
								(
									treeid,
									linkid,
									num_kids
								)
								VALUES
								(
									:treeid,
									:linkid,
									:num_kids
								)';
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'treeid', $new_treeid);
						MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
						MatrixDAL::bindValueToPdo($query, 'num_kids', 0);
						MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('Unable to add new tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
					}

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

					break;

				}//end no existing treeid

				// otherwise we could find a treeid on this branch, so increment it
				// to get a free childid that we can use for our new link
				$sql = 'asset_link_treeid_convert(:treeid, :encode, :tree_base, :tree_size)';
				$bind_vars = Array(
								'treeid'	=> $last_treeid,
								'encode'	=> '0',
								'tree_base'	=> SQ_CONF_ASSET_TREE_BASE,
								'tree_size'	=> SQ_CONF_ASSET_TREE_SIZE,
							 );
				$child_num = db_get_function_result(MatrixDAL::getDbType(), $sql, $bind_vars);

				$bind_vars = Array(
								'treeid'	=> (string) ($child_num + 1),
								'encode'	=> '1',
								'tree_base'	=> SQ_CONF_ASSET_TREE_BASE,
								'tree_size'	=> SQ_CONF_ASSET_TREE_SIZE,
							 );
				$free_treeid = db_get_function_result(MatrixDAL::getDbType(), $sql, $bind_vars);

				// we want to try and reserve a treeid for us that we have just converted
				// from the last treeid on the branch. So what we are doing here is inserting
				// an entry into the tree table with our linkid appended to the end.
				// We need to do this because someone might have already inserted into the
				// tree table between the time that we selected the last treeid on the branch
				// and this point here

				$sql = 'INSERT INTO
							sq_ast_lnk_tree
							(
								treeid,
								linkid,
								num_kids
							)
							VALUES
							(
								:treeid,
								:linkid,
								:num_kids
							)';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'treeid', $treeid.$free_treeid.':'.$linkid);
					MatrixDAL::bindValueToPdo($query, 'linkid', 0);
					MatrixDAL::bindValueToPdo($query, 'num_kids', 0);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to reserve new tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				// now try to select the last treeid from this branch , ordering
				// by treeid. If a process got in before us, we will select it as
				// the order by will cause it to come first as its linkid will be smaller

				$sql = 'SELECT
							treeid
						FROM
							sq_ast_lnk_tree
						WHERE
							(treeid LIKE :treeid_wildcard OR treeid = :treeid)
						AND	num_kids = 0 ORDER BY treeid DESC';

				try {
					$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

					$bind_vars = Array();
					if (!$is_oci) {
						$sql = str_replace(':treeid_wildcard', MatrixDAL::quote($treeid.$free_treeid.':%'), $sql);
					} else {
						$bind_vars['treeid_wildcard'] = $treeid.$free_treeid.':%';
					}
					$bind_vars['treeid'] = $treeid.$free_treeid;
					$query = MatrixDAL::preparePdoQuery($sql);
					foreach ($bind_vars as $bind_name => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_name, $bind_value);
					}
					$found_treeid = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get tree data due to database error: '.$e->getMessage());
				}

				if ($found_treeid == $treeid.$free_treeid.':'.$linkid) {

					// if the found treeid was like the one that we just inserted, then
					// we MIGHT have a winner. The reason that I say might is because
					// someone else might have thought that they have won between the
					// time of the last select to now

					$sql = 'UPDATE
								sq_ast_lnk_tree
							SET
								linkid = :linkid
							WHERE
								treeid = :treeid_found
							AND
								linkid = 0
							AND NOT EXISTS(
								SELECT
									1
								FROM
									sq_ast_lnk_tree
								WHERE
									(treeid LIKE :treeid_wildcard
								OR	treeid = :treeid)
								AND linkid <> 0)';

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					try {
						$bind_vars = Array();
						if (!$is_oci) {
							$sql = str_replace(':treeid_wildcard', MatrixDAL::quote($treeid.$free_treeid.':%'), $sql);
						} else {
							$bind_vars['treeid_wildcard'] = $treeid.$free_treeid.':%';
						}
						$bind_vars['linkid'] = -$linkid;
						$bind_vars['treeid'] = $treeid.$free_treeid;
						$bind_vars['treeid_found'] = $found_treeid;
						$query = MatrixDAL::preparePdoQuery($sql);
						foreach ($bind_vars as $bind_name => $bind_value) {
							MatrixDAL::bindValueToPdo($query, $bind_name, $bind_value);
						}
						$aff_rows = MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('Unable to attempt to win tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
					}

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

					// Get the number of affected rows - if the update to our
					// link ID worked, then we've won this tree ID
					if ($aff_rows != 0) {
						// before we declare won tree ID, we have to double check if other processes also declare won tree ID
						// this could happen because above "UPDATE .. AND NOT EXISTS" query is not granular enough.
						// every concurrent processes may think others NOT EXIST, and therefore all UPDATE at the same time.
						$sql = 'SELECT count(1) FROM sq_ast_lnk_tree
							WHERE
							(
								treeid LIKE :treeid_wildcard
								OR treeid = :treeid
							)
							AND linkid <> 0
							';
						$bind_vars = Array();
						$bind_vars['treeid'] = $treeid.$free_treeid;
						if (!$is_oci) {
							$sql = str_replace(':treeid_wildcard', MatrixDAL::quote($treeid.$free_treeid.':%'), $sql);
						} else {
							$bind_vars['treeid_wildcard'] = $treeid.$free_treeid.':%';
						}
						$query = MatrixDAL::preparePdoQuery($sql);
						foreach ($bind_vars as $bind_name => $bind_value) {
							MatrixDAL::bindValueToPdo($query, $bind_name, $bind_value);
						}
						$result = MatrixDAL::executePdoOne($query);
						if($result <=1) {
							$requires_update = TRUE;
							break;
						}
					}
				}//end if

				// otherwise we lost this treeid and will have to do another
				// iterator to try and win a different one
				$sql = 'DELETE FROM
							sq_ast_lnk_tree
						WHERE
						treeid = :treeid';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'treeid', $treeid.$free_treeid.':'.$linkid);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to attempt to drop lost tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				$i++;
			// only try to do this 200 times. more tries, better chance to avoid concurrency deadlock.
			// but too many tries could freeze the process by exceeding max execution time
			} while ($i < 200);

			if ($i >= 200) {
				trigger_localised_error('SYS0310', E_USER_ERROR);
			}

			if ($requires_update) {

				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							treeid = :treeid,
							linkid = :linkid
						WHERE
						treeid = :old_treeid';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					// The negative link ID is correct; we will replace with
					// a positive version if we've drawn a winning tree ID
					MatrixDAL::bindValueToPdo($query, 'treeid',     $treeid.$free_treeid);
					MatrixDAL::bindValueToPdo($query, 'linkid',     $linkid);
					MatrixDAL::bindValueToPdo($query, 'old_treeid', $treeid.$free_treeid.':'.$linkid);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to attempt to update won tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}

			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '-') {
				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							linkid   = :linkid,
							num_kids = :num_kids
						WHERE
							treeid = :treeid';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'linkid',   $linkid);
					MatrixDAL::bindValueToPdo($query, 'num_kids', $minor_tree['num_kids']);
					MatrixDAL::bindValueToPdo($query, 'treeid',   $free_treeid);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update top-level tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			} else {
				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							linkid   = :linkid,
							num_kids = :num_kids
						WHERE
							treeid = :treeid';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'linkid',   $linkid);
					MatrixDAL::bindValueToPdo($query, 'num_kids', $minor_tree['num_kids']);
					MatrixDAL::bindValueToPdo($query, 'treeid',   $existing_treeid.$free_treeid);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update tree entry for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				// we have existing tree entries, do a insert..select to create entries for them all

				$sql = 'INSERT INTO
							sq_ast_lnk_tree
							(
								treeid,
								linkid,
								num_kids
							)
							SELECT
								(t.treeid || :free_treeid),
								:linkid,
								:num_kids
							FROM
								sq_ast_lnk_tree t
								INNER JOIN	sq_ast_lnk l ON t.linkid = l.linkid
							WHERE
								l.minorid = :minorid
								AND treeid || :free_treeid_1 <> :treeid';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'free_treeid',   $free_treeid);
					MatrixDAL::bindValueToPdo($query, 'free_treeid_1',   $free_treeid);
					MatrixDAL::bindValueToPdo($query, 'linkid',   $linkid);
					MatrixDAL::bindValueToPdo($query, 'minorid',   $major->id);
					MatrixDAL::bindValueToPdo($query, 'num_kids', $minor_tree['num_kids']);
					MatrixDAL::bindValueToPdo($query, 'treeid',   $existing_treeid.$free_treeid);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to insert child tree entries for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			}//end else

			$sql = 'UPDATE
						sq_ast_lnk_tree
					SET
						num_kids = num_kids + 1
					WHERE
						treeid IN
						(
							SELECT
								CASE WHEN
									LENGTH(SUBSTR(t.treeid, 1, (LENGTH(t.treeid) - :tree_size))) != 0
								THEN
									SUBSTR(t.treeid, 1, (LENGTH(t.treeid) - :tree_size_1))
								ELSE
									:treeid
								END
							FROM
								sq_ast_lnk_tree t
							WHERE
								t.linkid = :linkid
						)';

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'tree_size',   SQ_CONF_ASSET_TREE_SIZE);
				MatrixDAL::bindValueToPdo($query, 'tree_size_1', SQ_CONF_ASSET_TREE_SIZE);
				MatrixDAL::bindValueToPdo($query, 'linkid',      $linkid);
				MatrixDAL::bindValueToPdo($query, 'treeid',   '-');
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to insert child tree entries for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			// if this minor has already been linked so do a select into
			if ($minor_tree['treeid'] != '') {
				$case = "CASE pt.treeid
							WHEN '-' THEN ''
							ELSE pt.treeid
						END";

				$sql = 'INSERT INTO
							sq_ast_lnk_tree
							(
								treeid,
								linkid,
								num_kids
							)
							SELECT
								('.$case.') || :treeid || SUBSTR(ct.treeid, :minor_treeid),
								ct.linkid,
								ct.num_kids
							FROM
											sq_ast_lnk_tree pt
								INNER JOIN	sq_ast_lnk pl ON pt.linkid = pl.linkid,
											sq_ast_lnk_tree ct
							WHERE
									pl.minorid	= :minorid
								AND	ct.treeid	LIKE :minor_treeid_wildcard
								AND	ct.treeid	> :minor_treeid_1';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'treeid', $free_treeid);
					MatrixDAL::bindValueToPdo($query, 'minorid', $major->id);
					MatrixDAL::bindValueToPdo($query, 'minor_treeid_wildcard', $minor_tree['treeid'].'%');
					MatrixDAL::bindValueToPdo($query, 'minor_treeid', strlen($minor_tree['treeid'])+1);
					MatrixDAL::bindValueToPdo($query, 'minor_treeid_1', $minor_tree['treeid']);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to insert child tree entries for link #'.$linkid.', between "'.$major->name.'" (#'.$major->id.') and "'.$minor->name.'" (#'.$minor->id.'), due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}//end if


		}//end if significant link

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); /******** End DB3 ************/

		// only call the linksUpdated() method on the parent if this link is dependent
		if ($dependant) $major->linksUpdated();
		$minor->linksUpdated();

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// Release locks if we acquired them earlier
			if (!$parent_was_locked) {
				$this->releaseLock($major->id, 'links'); // don't care if this fails, too late to revert now
			}
		}

		// Send Internal Message
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'major_name'	=> $major->name,
						'minor_name'	=> $minor->name,
					);
		$message = $ms->newMessage(Array(), 'asset.linking.create', $msg_reps);
		$message->parameters['majorid'] = $major->id;
		$message->parameters['minorid'] = $minor->id;
		$message->parameters['linkid']  = $linkid;
		$message->send();

		// Broadcast Event
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($major, 'CreateLink', Array('linkid' => $linkid));

		// Fire Triggers
		if ($major->type() != 'trash_folder' && $minor->type() != 'trash_folder') {
			// fire the 'Link Created' events, one for the major asset, one for the minor

			$link_info['majorid'] = $major->id;
			$link_info['minorid'] = $minor->id;
			$link_info['linkid'] = $linkid;
			$link_info['value'] = $value;
			$link_info['link_type'] = $link_type;
			$link_info['minor_type_code'] = $minor->type();
			$link_info['major_type_code'] = $major->type();
			$link_info['sort_order'] = $sort_order;
			$link_info['is_dependant'] = $dependant;
			$link_info['is_exclusive'] = $exclusive;
			$link_info['locked'] = $locked;

			$event_data = Array('link_info' => $link_info, 'linkid' => $linkid);

			if ($major->type() != 'root_folder') {
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_link_created', $major, $event_data);
			}
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_link_created', $minor, $event_data);
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db2

		return $linkid;

	}//end createAssetLink()


	/**
	* Create a "shadow" asset link between the two supplied assets
	*
	* It returns the linkid of the created link
	*
	* @param Asset	$major		the major asset that we are linking from
	* @param Asset	$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param string	$value		the value that is to be associated with this link
	*
	* @return int
	* @access public
	*/
	function createShadowAssetLink(Asset $major, Asset $minor, $link_type, $value='')
	{
		// we are linking a shadow asset under a normal asset

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();
		// use the sq_ast_lnk sequence so that when we query any link view later
		// on we don't have any conflicting linkids

		$linkid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_lnk_seq'));
		$now = time();
		$ts_fragment = MatrixDAL::executeOne('core', 'toDate', Array('date_value' => ts_iso8601($now)));
		$sql = 'INSERT INTO
					sq_shdw_ast_lnk
					(
						linkid,
						majorid,
						minorid,
						link_type,
						value,
						updated,
						updated_userid
					)
					VALUES
					(
						:linkid,
						:majorid,
						:minorid,
						:link_type,
						:value,
						:updated,
						:currentUserId
					)';

		$result = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
			MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
			MatrixDAL::bindValueToPdo($query, 'minorid', $minor->id);
			MatrixDAL::bindValueToPdo($query, 'link_type', $link_type);
			MatrixDAL::bindValueToPdo($query, 'updated', $ts_fragment);
			MatrixDAL::bindValueToPdo($query, 'value', $value);
			MatrixDAL::bindValueToPdo($query, 'currentUserId', $GLOBALS['SQ_SYSTEM']->currentUserid());
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('cannot create Shadow Asset Links due to database error: '.$e->getMessage());
		}

		$this->forgetAsset($major);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $linkid;

	}//end createShadowAssetLink()


	/**
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param Asset	$major		the major asset in the proposed link
	* @param Asset	$minor		the minor asset in the proposed link
	* @param string	$link_type	the type of link that is proposed
	* @param int	$exclusive	(0|1) whether this will be the only significant link
	*							with $minor as the minor asset
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(Asset $major, Asset $minor, $link_type, $exclusive)
	{
		if (!$major->id) return FALSE;

		if (!($minor instanceof Asset)) {
			return translate('minor_is_not_asset');
		}

		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canLinkToType($major, $minor->type(), $link_type, 0, $exclusive)) !== TRUE) {
			return $err_msg;
		}

		// Check write access as appropriate
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			if ($link_type == SQ_LINK_NOTICE) {
				// To create a notice link you need effective write access
				// to the major, but you don't need any access to the minor
				if (($major->status == SQ_STATUS_ARCHIVED) || (!$major->writeAccess(''))) {
					return translate('cannot_create_notice_link_permission_denied', $major->name, $major->id, $minor->name, $minor->id);
				}
			} else {
				// To create a significant link you need write permissions
				// (but not effective write access) to both assets
				$majwa = $major->writeAccess('');
				$minwa = $minor->writeAccess('');
				if (!($majwa || $minwa)) {
					return translate('cannot_create_sig_link_no_perm_either', $major->name, $major->id, $minor->name, $minor->id);
				} else if (!$majwa) {
					return translate('cannot_create_sig_link_no_perm_major', $major->name, $major->id, $minor->name, $minor->id);
				} else if (!$minwa) {
					return translate('cannot_create_sig_link_no_perm_minor', $major->name, $major->id, $minor->name, $minor->id);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$db = MatrixDAL::getDb();

		// check for web path conflicts with the new parents children
		if ($link_type & SQ_SC_LINK_WEB_PATHS) {
			$paths = $minor->getWebPaths();
			$bad_paths = $this->webPathsInUse($major, $paths, $minor->id);
			if (!empty($bad_paths)) {
				return translate('cannot_create_link_paths_in_use', $major->name, $major->id, $minor->name, $minor->id, '"'.implode('", "', $bad_paths).'"');
			}
		}

		// if this link is a significant link, then we need to make sure that this
		// minor asset doesn't already have an exclusive link from anything
		// and that we aren't moving under ourselves
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {
			//// Check for exclusive ////
			$sql = 'SELECT majorid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk ';
			$where = 'minorid = :minor_id
						AND '.db_extras_bitand(MatrixDAL::getDbType(), 'link_type', SQ_SC_LINK_SIGNIFICANT).' > 0';
			if (!$exclusive) {
				$where .= ' AND is_exclusive = :is_exclusive';
			}
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where;

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'minor_id', $minor->id);
				if (!$exclusive) {
					MatrixDAL::bindValueToPdo($query, 'is_exclusive', 1);
				}
				$current_majorid = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to check that minor asset doesnt already have an exclusive link due to database error: '.$e->getMessage());
			}

			if ($current_majorid) {
				$current_major = $this->getAsset($current_majorid);
				return translate('cannot_create_link_exclusive_link', $minor->name, $minor->id, $major->name, $major->id, $current_major->name, $current_majorid);
			}

		}//end if

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end canCreateLink()


	/**
	* Returns TRUE if the supplied asset can have assets of the specified type linked under it
	*
	* @param Asset	$major			the major asset that we are linking to
	* @param string	$type_code		the type code of the proposed link
	* @param string	$link_type		the type of the proposed link
	* @param int	$ignore_linkid	if it gets down to the checking of the cardinality,
	*								ignore the link represented by this link id
	* @param int	$exclusive		(0|1) whether this will be the only significant link
	*								with $minor as the minor asset
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canLinkToType(Asset $major, $type_code, $link_type, $ignore_linkid=0, $exclusive=0)
	{
		// type checking
		if (!($major instanceof Asset)) {
			return translate('major_is_not_asset');
		}

		if (!is_string($type_code)) {
			return translate('type_code_is_not_string');
		}

		if (!is_numeric($link_type)) {
			return translate('link_type_is_not_integer');
		}

		// get the minor assets parents and add it's type to the front of the indexed array
		$types = $this->getTypeAncestors($type_code);
		array_unshift($types, $type_code);

		$allowed_links = $major->_getAllowedLinks();

		$typeNames = $this->getTypeInfo(
						array(
						 $type_code,
						 $major->type(),
						),
						'name'
					);

		// we will be ascending up the parent tree from the current asset type.
		// that way the major asset can have specific cardinality for different assets types
		// EG, assets => 'M', user => '1' -> this means that many assets can be linked to this asset
		//     and only one user, but because 'user' is an 'asset' we need to check for any 'user' references
		//     before we check for any 'asset' references
		$type = '';
		for ($i = 0; $i < count($types); $i++) {
			$type = $types[$i];
			if (!empty($allowed_links[$link_type][$type])) break;
		}

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		if (empty($allowed_links[$link_type][$type])) {
			return translate('asset_type_cannot_be_linktype_linked_to_type', $typeNames[$type_code], link_type_name($link_type), $typeNames[$major->type()]);
		}

		if (!($link_type & SQ_SC_LINK_SIGNIFICANT) && $exclusive) {
			return translate('exclusive_links_must_be_significant');
		}

		if (!$exclusive && !empty($allowed_links[$link_type][$type]['exclusive'])) {
			return translate('asset_type_must_be_linktype_exclusively_linked_to_type', $typeNames[$type_code], link_type_name($link_type), $typeNames[$major->type()]);
		}

		//// Now we need to check that the cardinality is kept valid ////

		// if we are only allowed up to a certain number of these links
		if ($allowed_links[$link_type][$type]['card'] != 'M' && $major->id) {
			$num_curr_links = $this->countLinks($major->id, 'major', $link_type, $type, TRUE, $ignore_linkid);
			//  and we already have our quota of links
			if ($num_curr_links >= (int) $allowed_links[$link_type][$type]['card']) {
				$cardTypeName = $this->getTypeInfo($type, 'name');
				return translate('asset_type_can_have_only_linktypes_links_to_type', $typeNames[$major->type()], ((int) $allowed_links[$link_type][$type]['card']), link_type_name($link_type), $cardTypeName);
			}
		}

		// if we get this far all is OK
		return TRUE;

	}//end canLinkToType()


	/**
	* Move a link to a new parent
	*
	* @param int	$linkid			the link id of the link to move
	* @param int	$to_parentid	the asset id of the new parent asset
	* @param int	$link_type		the type of the new link
	* @param int	$to_parent_pos	the position under the new parent
	* @param string $link_value		the value of the new link
	*
	* @return int
	* @access public
	*/
	function moveLink($linkid, $to_parentid, $link_type, $to_parent_pos, $link_value='')
	{
		// parent position type must be numeric
		if (!is_numeric($to_parent_pos)) {
			trigger_localised_error('SYS0227', E_USER_WARNING);
			return 0;
		}

		// parent ID must be a valid assetid
		if (!assert_valid_assetid($to_parentid, '', FALSE, FALSE)) {
			trigger_localised_error('SYS0225', E_USER_WARNING);
			return 0;
		}

		// link type must be numeric (ie. an int, or a numeric string)
		if (!is_numeric($link_type)) {
			trigger_localised_error('SYS0221', E_USER_WARNING);
			return 0;
		}

		// get the link, and the old parent
		$link = $this->getLinkById($linkid);

		if (empty($link)) {
			trigger_localised_error('SYS0138', E_USER_WARNING, $linkid);
			return 0;
		}

		if (isset($link['locked']) && $link['locked'] == '1') {
			trigger_localised_error('SYS0333', E_USER_WARNING, $linkid);
			return 0;
		}

		$old_parent = $this->getAsset($link['majorid'], $link['major_type_code']);
		if (is_null($old_parent)) {
			trigger_localised_error('SYS0226', E_USER_WARNING, $link['majorid']);
			return 0;
		}

		// if you dont have write access to the old parent asset you cant delete any links
		if (!$old_parent->writeAccess('')) {
			trigger_localised_error('SYS0229', E_USER_WARNING, $old_parent->name);
			$this->forgetAsset($old_parent);
			return 0;
		}

		// now the new parent
		$new_parent = $this->getAsset($to_parentid, '', TRUE);
		if (is_null($new_parent)) {
			trigger_localised_error('SYS0224', E_USER_WARNING, $to_parentid);
			$this->forgetAsset($old_parent);
			return 0;
		}

		// if you dont have write access to the new parent asset you cant create any links
		if (!$new_parent->writeAccess('')) {
			trigger_localised_error('SYS0228', E_USER_WARNING, $new_parent->name);
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			return 0;
		}

		// this is the asset we are moving
		$minor = $this->getAsset($link['minorid'], $link['minor_type_code']);
		if (!$minor->id) {
			trigger_localised_error('SYS0222', E_USER_WARNING, $link['minorid']);
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			return 0;
		}

		// are we allowed to move this link?
		if (TRUE !== ($error_msg = $new_parent->canMoveLink($minor, $old_parent, $link_type))) {
			trigger_error($error_msg, E_USER_WARNING);
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			$this->forgetAsset($minor);
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$success = TRUE;

		// now create the new link, it's non-exclusive + non-dependent, skip canCreateLink() checks
		$new_linkid = $new_parent->createLink($minor, $link_type, $link_value, $to_parent_pos, '0', '0', TRUE);
		if (!$new_linkid) $success = FALSE;

		// let's try and delete the old link
		if ($success) {
			$success = $old_parent->deleteLink($linkid);
		}

		if ($success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			if ($new_linkid != 0) $link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($new_linkid);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			//Bug Fix #3886 if we rolling back, delete the link we had already created
			if ($new_linkid != 0) $this->deleteAssetLinkByLink($link);
			$new_linkid = 0;
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$this->forgetAsset($old_parent);
		$this->forgetAsset($new_parent);
		$this->forgetAsset($minor);

		return $new_linkid;

	}//end moveLink()


	/**
	* Move a link from one position in the asset's links order to another
	*
	* @param int	$linkid		the link id of the link to move about
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		return $this->updateLink($linkid, NULL, NULL, $sort_order);

	}//end moveLinkPos()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	* @param string	$locked		the asset's link lock status
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL, $locked=NULL)
	{
		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_localised_error('SYS0139', E_USER_WARNING, $linkid);
			return FALSE;
		}

		$id_parts = explode(':', $linkid);
		$minorid_parts = explode(':', $link['minorid']);
		// shadow asset found, pipe the request of to the bridge
		if (isset($id_parts[1])) {

			$bridge = $this->getAsset($id_parts[0]);
			$result =  $bridge->updateLink($linkid, $link_type, $value, $sort_order);
			$this->forgetAsset($bridge);
			return $result;
		}


		$db = MatrixDAL::getDb();

		$set_clauses = Array();

		// Do not allow link types of type 3 and notice to be locked
		if ($link_type == SQ_LINK_TYPE_3 || $link_type == SQ_LINK_NOTICE) {
			if (!is_null($locked)) $locked = '0';
		}//end if

		$link_type_changed	 = (!is_null($link_type)	&& $link['link_type']	!= $link_type	);
		$value_changed		 = (!is_null($value)		&& $link['value']		!= $value		);
		$sort_order_changed	 = (!is_null($sort_order)	&& $link['sort_order']	!= $sort_order	);
		$link_locked_changed = (!is_null($locked)		&& $link['locked']		!= $locked		);


		$link_type  = (int) $link_type;
		$sort_order = (int) $sort_order;
		$locked		= ($locked == '1') ? '1' : '0';

		$major = $this->getAsset((int) $link['majorid'], $link['major_type_code']);
		$minor = $this->getAsset((int) $link['minorid'], $link['minor_type_code']);

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		if ($link_type_changed) {

			// We need to make sure that we aren't going to have to be stuffing
			// about with tree to get the update working
			// so because the tree only contains significant links if the
			// significant state has changed... error
			$current_is_sig = (bool) ((int) $link['link_type'] & SQ_SC_LINK_SIGNIFICANT);
			$new_is_sig     = (bool) ($link_type & SQ_SC_LINK_SIGNIFICANT);
			if ($current_is_sig !== $new_is_sig) {
				trigger_localised_error('SYS0256', E_USER_WARNING, $linkid);
				$ms->abortLog();
				return FALSE;
			}

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
				if (($err_msg = $this->canLinkToType($major, $minor->type(), $link_type)) !== TRUE) {
					trigger_localised_error('SYS0255', E_USER_WARNING, $linkid, $err_msg);
					$ms->abortLog();
					return FALSE;
				}
			}

			// check for web path conflicts with the new parents children
			// if the old link is not a web path link but the new one is
			if (!($link['link_type'] & SQ_SC_LINK_WEB_PATHS) && ($link_type & SQ_SC_LINK_WEB_PATHS)) {
				$paths     = $minor->getWebPaths();
				$bad_paths = $this->webPathsInUse($major, $paths, $minor->id);
				if (!empty($bad_paths)) {
					trigger_localised_error('SYS0121', E_USER_WARNING, $linkid, implode('", "', $bad_paths), $major->name);
					$ms->abortLog();
					return FALSE;
				}
			}

			$set_clauses['link_type'] = MatrixDAL::quote($link_type);

			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$msg_reps = Array(
							'linkid'		=> $linkid,
							'major_name'	=> $major->name,
							'minor_name'	=> $minor->name,
							'old_link_type'	=> link_type_name($link['link_type']),
							'new_link_type'	=> link_type_name($link_type),
						);
			$message = $ms->newMessage(Array(), 'asset.linking.type', $msg_reps);
			$message->parameters['majorid']  = $major->id;
			$message->parameters['minorid']  = $minor->id;
			$message->parameters['linkid']   = $linkid;
			$ms->logMessage($message);

		}//end if link_type_changed

		if ($value_changed) {

			$set_clauses['value'] = MatrixDAL::quote($value);

			$msg_reps = Array(
							'linkid'			=> $linkid,
							'major_name'		=> $major->name,
							'minor_name'		=> $minor->name,
							'old_link_value'	=> $link['value'],
							'new_link_value'	=> $value,
						);
			$message = $ms->newMessage(Array(), 'asset.linking.value', $msg_reps);
			$message->parameters['majorid']   = $major->id;
			$message->parameters['minorid']   = $minor->id;
			$message->parameters['linkid']    = $linkid;
			$ms->logMessage($message);

		}//end if value_changed

		if ($link_locked_changed) {

			$set_clauses['locked'] = MatrixDAL::quote($locked);

			$msg_reps = Array(
							'linkid'			=> $linkid,
							'major_name'		=> $major->name,
							'minor_name'		=> $minor->name,
							'link_locked_value'	=> (($locked == '1') ? translate('locked') : translate('unlocked')),
						);
			$message = $ms->newMessage(Array(), 'asset.linking.locked', $msg_reps);
			$message->parameters['majorid']   = $major->id;
			$message->parameters['minorid']   = $minor->id;
			$message->parameters['linkid']    = $linkid;
			$ms->logMessage($message);

		}//end if link_locked_changed

		// If the minor is a shadow asset, we need to update the shadow link table instead
		// There's no order field in the shadow table, so we branch before we add order to the set clause
		if (isset($minorid_parts[1])) {

			if (empty($set_clauses)) {
				$ms->abortLog();
				return TRUE;
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db = MatrixDAL::getDb();

			$set_clauses['updated']			= MatrixDAL::quote(MatrixDAL::executeOne('core', 'toDate', Array('date_value' => ts_iso8601(time()))));

			$set_array = Array();

			foreach ($set_clauses as $key => $value) {
				$set_array[] = $key.'='.$value;
			}

			$set_string = implode(',', $set_array);

			$sql = 'UPDATE
						sq_shdw_ast_lnk
					SET
						updated_userid = :updated_userid,
						'.$set_string.'
					WHERE
							linkid	= :linkid
							AND	majorid	= :majorid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
				MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
				MatrixDAL::bindValueToPdo($query, 'updated_userid', $GLOBALS['SQ_SYSTEM']->currentUserId());
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update the link due to database error: '.$e->getMessage());
			}

			$this->forgetAsset($major);

		} else {

			if ($sort_order_changed) {

				$row = NULL;
				try {
					$sql = 'SELECT COUNT(*) AS count, MAX(sort_order) AS max
							FROM
								sq_ast_lnk
							WHERE
							majorid = :majorid';

					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
					$result = MatrixDAL::executePdoAssoc($query);
					$row = $result[0];

				} catch (Exception $e) {
					throw new Exception('Unable to get execute the query: '.$e->getMessage());
				}

				$max = ($row['count'] > 0) ? (int) $row['max'] : 0;
				if ($sort_order > $max || $sort_order < 0) {
					$sort_order = $max;
				}

				$set_clauses['sort_order'] = MatrixDAL::quote($sort_order);

				$msg_reps = Array(
								'linkid'			=> $linkid,
								'major_name'		=> $major->name,
								'minor_name'		=> $minor->name,
								'old_sort_order'	=> $link['sort_order'],
								'new_sort_order'	=> $sort_order,
							);
				$message = $ms->newMessage(Array(), 'asset.linking.order', $msg_reps);
				$message->parameters['majorid'] = $major->id;
				$message->parameters['minorid'] = $minor->id;
				$message->parameters['linkid']  = $linkid;
				$ms->logMessage($message);
			}

			if (empty($set_clauses)) {
				$ms->abortLog();
				return TRUE;
			}
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db = MatrixDAL::getDb();

			$set_clauses['updated']			= MatrixDAL::quote(MatrixDAL::executeOne('core', 'toDate', Array('date_value' => ts_iso8601(time()))));

			$set_array = Array();

			foreach ($set_clauses as $key => $value) {
				$set_array[] = $key.'='.$value;
			}

			$set_string = implode(',', $set_array);

			$sql = 'UPDATE
						sq_ast_lnk
					SET
						updated_userid = :updated_userid,
						'.$set_string.'
					WHERE
							linkid	= :linkid
							AND	majorid	= :majorid';
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
				MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
				MatrixDAL::bindValueToPdo($query, 'updated_userid', $GLOBALS['SQ_SYSTEM']->currentUserId());
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update the link due to database error: '.$e->getMessage());
			}

			if ($sort_order_changed) {
				// move 'em up, higher
				if ($link['sort_order'] > $sort_order) {

					$sql = 'UPDATE
								sq_ast_lnk
							SET
								sort_order = sort_order + 1
							WHERE
									majorid		= :majorid
								AND	linkid		<> :linkid
								AND	sort_order	>= :sort_order_1
								AND	sort_order	<= :sort_order_2';

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
						MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
						MatrixDAL::bindValueToPdo($query, 'sort_order_1', $sort_order);
						MatrixDAL::bindValueToPdo($query, 'sort_order_2', $link['sort_order']);
						MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('Unable to update the link due to database error: '.$e->getMessage());
					}

				} else {

					$sql = 'UPDATE
								sq_ast_lnk
							SET
								sort_order = sort_order - 1
							WHERE
									majorid		= :majorid
								AND	linkid		<> :linkid
								AND	sort_order	>= :sort_order_1
								AND	sort_order	<= :sort_order_2';

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
						MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
						MatrixDAL::bindValueToPdo($query, 'sort_order_1', $link['sort_order']);
						MatrixDAL::bindValueToPdo($query, 'sort_order_2', $sort_order);
						MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('Unable to update the link due to database error: '.$e->getMessage());
					}
				}

			}//end if
		}//end else


		if ($link_type_changed) {
			// if this is a web path link or if the old link type was,
			// then we need to inform the asset to update it's lookups
			if (($link_type & SQ_SC_LINK_WEB_PATHS) || ((int) $link['link_type'] & SQ_SC_LINK_WEB_PATHS)) {
				if (!$minor->updateLookups()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					$ms->abortLog();
					return FALSE;
				}
			}
		}



		// tell, the asset it has updated
		$major->linksUpdated();
		$minor->linksUpdated();

		$ms->closeLog();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid			the link id of the link to remove
	* @param boolean	$check_locked	Check if the link is locked or not
	* @param array|NULL $link           When deleting an asset, we already get the link,
	*                                   we don't need to do it again.
	*                                   If it's NULL here, try to get it ourselves.
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $check_locked=TRUE, $link=NULL)
	{
		if ($linkid == 0) return FALSE;
		// before anything else is done, check if this is a shadow link, i.e. a link that is managed by a bridge
		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1])) {
			$bridge = $this->getAsset($id_parts[0]);
			$result = $bridge->deleteAssetLink($linkid);
			$this->forgetAsset($bridge);
			return $result;
		}

		// Use the master DB since we are changing things
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		if ($link === NULL || empty($link)) {
			// First, we should try and find the link
			$link = $this->getLinkById($linkid);
		}

		if (empty($link)) {
			trigger_localised_error('SYS0137', E_USER_WARNING, $linkid);
			return FALSE;
		}

		return $this->deleteAssetLinkByLink($link, $check_locked);

	}//end deleteAssetLink()


	/**
	* Remove a link by the link that is passed
	* This function the initial checking that is done by  deleteAssetLink()
	* Fix to #3877  	Matrix will create and retain links in DB if asset fails to get created
	* a new function call made to deleteAssetLinkByLink() from _abortCreate() from asset.inc
	*
	* @param Array		$link			the link we are supposed to delete
	* @param boolean	$check_locked	Check if the link is locked or not
	* @param boolean	$aborting	    set to TRUE when link to be deleted belongs an asset
										that was just aborted (TRUE when called by Asset->_abortCreate())
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLinkByLink(Array $link, $check_locked=TRUE, $aborting=FALSE)
	{
		if (empty($link)) {
			return FALSE;
		}
		$linkid = $link['linkid'];

		// Check if link is locked and cannot be deleted
		if ($check_locked && $link['locked'] == '1') {
			trigger_localised_error('SYS0334', E_USER_WARNING, $linkid);
			return FALSE;
		}//end if

		$major = $this->getAsset($link['majorid'], $link['major_type_code']);
		$minor = $this->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($major) || is_null($minor)) {
			return FALSE;
		}

		$asset_was_locked = FALSE;
		$parent_was_locked = FALSE;

		if (!$aborting) {
			// Acquire some locks if necessary
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {

				// If we're purging trash, the hipo has already checked locks.
				// We don't need to do it again.
				if (!$GLOBALS['SQ_PURGING_TRASH']) {
					// Keep a record of the locking state beforehand so we can leave it as we found it
					$lock_info = @$this->getLockInfo($major, 'links');
					$parent_was_locked = !empty($lock_info);
					if (!$this->acquireLock($major->id, 'links')) {
						trigger_localised_error('SYS0126', E_USER_WARNING, 'major', $major->name);
						return FALSE;
					}
					// To check if we can delete this link, we need to lock current asset
					$minor_lock_info = @$this->getLockInfo($minor, 'links');
					$asset_was_locked = !empty($minor_lock_info);
					if (!$this->acquireLock($minor->id, 'links')) {
						trigger_localised_error('SYS0126', E_USER_WARNING, 'minor', $minor->name);
						return FALSE;
					}
				}
			}

			// Check permissions if necessary
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
				if ($link['link_type'] == SQ_LINK_NOTICE) {
					// to delete a notice link we need effective write access to the major asset
					if (!($major->writeAccess() && $major->accessEffective()) && empty($GLOBALS['SQ_PURGING_TRASH'])) {
						trigger_localised_error('SYS0319', E_USER_WARNING, $linkid, $major->id, $major->name);
						return FALSE;
					}
				} else {
					// to delete a significant link we need write permission (but not
					// necessarily effective write access) to both assets
					if (($err_msg = $minor->canDeleteLink($linkid)) !== TRUE) {
						trigger_localised_error('SYS0318', E_USER_WARNING, $linkid, $minor->id, $minor->name, $err_msg);
						return FALSE;
					}
					if (($err_msg = $major->canDeleteLink($linkid)) !== TRUE) {
						trigger_localised_error('SYS0302', E_USER_WARNING, $linkid, $major->id, $major->name, $err_msg);
						return FALSE;
					}
				}
			}
		}

		$minorid_parts = explode(':', $link['minorid']);
		if (isset($minorid_parts[1])) {
			// Shadow asset found
			if (implements_interface($major, 'bridge')) {
				// Let the bridge handle it
				$result = $major->deleteAssetLink($linkid);
				$this->forgetAsset($asset);
				if ($result != FALSE) return $result;
			}
			// Link between a normal asset and a shadow asset
			$this->deleteShadowAssetLink($linkid);
			return TRUE;

		}//end if shadow asset

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$db = MatrixDAL::getDb();

		if ($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) {
			// This is a significant link

			$num_other_links = $this->countLinks($minor->id, 'minor', SQ_SC_LINK_SIGNIFICANT, '', TRUE, $linkid);

			if (!$GLOBALS['SQ_PURGING_TRASH'] && ($num_other_links <= 1)) {
				$linked_outside_trash = FALSE;
				if ($num_other_links == 1) {
					// we need to see if the other remaining link is to the trash
					$trash_links = $this->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'trash_folder', TRUE, 'minor');
					if (empty($trash_links)) {
						// the other link must be to something else
						$linked_outside_trash = TRUE;
					}
				}
				if (!$linked_outside_trash) {
					// it's about to become linked to the trash only, so run the trigger
					if (!$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_before_asset_deleted', $minor, Array())) {
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db3
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db2
						return FALSE;
					}
				}
			}

			if (!$aborting) {
				// we create a new link to the trash if this is the last significant link
				// being deleted and if we are not purging the trash. We check the moving
				// flag so that if we are the last significant link, we dont create a link
				// to the trash as the move operation will re-create the link elsewhere
				if (!$num_other_links && !$GLOBALS['SQ_PURGING_TRASH']) {

					if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {

						// some assets may not be able to have their last significant link
						// deleted, so lets check first before going ahead
						if (!$minor->canDelete()) {
							trigger_localised_error('SYS0069', E_USER_WARNING, $minor->name);
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db3
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db2
							return FALSE;
						}

						$trash_folder = $this->getSystemAsset('trash_folder');
						if (is_null($trash_folder)) {
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db3
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db2
							return FALSE;
						}

						// this asset is destined for the trash, so we need to set its permissions to
						// whatever it is inheriting right now so it keeps them while in the trash
						$perms = Array(SQ_PERMISSION_ADMIN, SQ_PERMISSION_WRITE, SQ_PERMISSION_READ);
						foreach ($perms as $perm) {
							$all_permissions = $this->getPermission($minor->id, $perm, NULL, FALSE, FALSE, TRUE);
							foreach ($all_permissions as $userid => $granted) {
								// we are deliberatly not checking for the return value here
								// because if for some reason the permission can't be set that is really just
								// bad luck
								@$this->setPermission($minor->id, $userid, $perm, $granted);
							}
						}

						if (!$trash_folder->createLink($minor, SQ_LINK_TYPE_2)) {
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db3
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // end db2
							return FALSE;
						}
					}//end if
				}//end if last significant link
			}

			//// UPDATE THE TREE ////

			// update the parents to tell them that they are going to be one kid less
			$sql = 'UPDATE
				sq_ast_lnk_tree
				SET
				num_kids = num_kids - 1
				WHERE
				treeid IN
				(
				 SELECT
				 CASE WHEN
				 LENGTH(SUBSTR(t.treeid, 1, LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.')) != 0
				 THEN
				 SUBSTR(t.treeid, 1, LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.')
				 ELSE
				 \'-\'
				 END
				 FROM
				 sq_ast_lnk_tree t
				 WHERE
				 t.linkid = :linkid
				)';

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update the link tree for linkid: '.$linkid.' due to database error: '.$e->getMessage());
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			// we can delete all the links under these nodes because it will be a clean start
			// when we insert into the gap's we create below

			$sql = 'DELETE FROM
				sq_ast_lnk_tree
				WHERE
				treeid in
				(
				 SELECT
				 ct.treeid
				 FROM
				 sq_ast_lnk_tree pt, sq_ast_lnk_tree ct
				 WHERE
				 pt.linkid	= :linkid
				 AND	(ct.treeid	LIKE pt.treeid || '.'\''.'%'.'\''.'
					 OR ct.treeid	= pt.treeid)
				)';

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to delete tree links for linkid: '.$linkid.' due to database error: '.$e->getMessage());
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}//end if significant link

		// Update sort orders of other children of this parent
		$sql = 'UPDATE
			sq_ast_lnk
			SET
			sort_order = sort_order - 1
			WHERE
			majorid	= :majorid
			AND sort_order > :sort_order';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'majorid', $major->id);
			MatrixDAL::bindValueToPdo($query, 'sort_order', $link['sort_order']);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update sort orders for majorid: '.$major->id.' due to database error: '.$e->getMessage());
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// Delete from the link table
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$bind_vars	= Array (
					'linkid'	=> $linkid,
					'majorid'	=> $major->id,
					);
			MatrixDAL::executeQuery('core', 'deleteLink', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete link with linkid: '.$linkid.' due to database error: '.$e->getMessage());
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// Tell the assets that they have been updated
		$major->linksUpdated();
		$minor->linksUpdated();

		// Release locks on parent and/or asset if we acquired them
		// If we're purging trash, the hipo deals with it.
		//
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			if (!$GLOBALS['SQ_PURGING_TRASH']) {
				if (!$parent_was_locked) {
					$this->releaseLock($major->id, 'links'); // we don't care if it fails, it's too late to reverse anything
				}
				if (!$asset_was_locked) {
					$this->releaseLock($minor->id, 'links');
				}
			}
		}
		if (!$aborting) {
			// Send Internal Message
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg_reps = Array(
					'major_name'	=> $major->name,
					'minor_name'	=> $minor->name,
					);
			$message = $ms->newMessage(Array(), 'asset.linking.delete', $msg_reps);
			$message->parameters['majorid'] = $major->id;
			$message->parameters['minorid'] = $minor->id;
			$message->parameters['linkid']  = $linkid;
			$message->send();

			// Broadcast Event
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($major, 'DeleteLink', Array('linkid' => $linkid));

			// Fire triggers
			if ($major->type() != 'trash_folder' && $minor->type() != 'trash_folder') {
				$event_data = Array('link_info' => $link, 'linkid' => $linkid);
				if (!$this->assetInTrash($minor->id, TRUE)) {
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_link_deleted', $minor, $event_data);
				}
				if (!$this->assetInTrash($major->id, TRUE)) {
					$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_link_deleted', $major, $event_data);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // finished with db2

		return TRUE;

	}//end deleteAssetLinkByLink()


	/**
	* Remove a SHADOW asset link by id
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteShadowAssetLink($linkid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = MatrixDAL::getDb();
		try {
			$bind_vars = Array('linkid' => $linkid);
			$result = MatrixDAL::executeQuery('core', 'deleteShadowAssetLink', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete shadow asset link with linkid: '.$linkid.' due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteShadowAssetLink()


	/**
	* See whether we COULD send the specified asset to the trash IF we got the locks on its parents
	*
	* It reutns TRUE if we can trash, FALSE if assetid=0, Array(linkid=>problem) otherwise
	*
	* @param int	$assetid	The asset we want to trash
	*
	* @return mixed boolean|array
	* @access public
	*/
	function couldTrashAsset($assetid, $check_locked=TRUE)
	{
		if (!$assetid) return FALSE;
		// We can trash the asset if we can delete all the parent links
		$msgs = Array();
		$parent_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', TRUE, 'minor');
		foreach ($parent_links as $id => $link_details) {
			$asset = $this->getAsset($link_details['majorid']);
			$msg = $asset->canDeleteLink($link_details['linkid']);
			if (($check_locked && $link_details['locked'] == '1') || TRUE !== $msg) {
				$msgs[$link_details['linkid']] = $msg;
			}
		}
		return empty($msgs) ? TRUE : $msgs;

	}//end couldTrashAsset()


	/**
	* Return list of errors if the asset would be blocked from being trashed if
	* Safe Trash feature is on, empty array otherwise
	*
	* @param int		$assetid			The asset we want to trash
	* @param int		$ignore_linkid		Link ID to ignore when calculating the number
	*										of type 1/2 links remaining (typically the
	*										link we are moving)
	* @param boolean	$ignore_other_links	Whether to check whether there are any remaining
	*										type 1/2 links before providing link errors
	*										(default = TRUE, as most use of this is for
	*										status of safe trash cron job)
	*
	* @return array
	* @access public
	*/
	function canSafeTrashAsset($assetid, $ignore_linkid=0, $ignore_other_links=TRUE)
	{
		$trash_errors = Array();

		// if its a shadow asset, this check is irrelevant
		if (strpos($assetid, ':') !== FALSE) {
			return $trash_errors;
		}

		$asset = $this->getAsset($assetid);

		// If we are ignoring other backend links (so we can provide a status),
		// then skip this part and always run the links check. Otherwise, we
		// don't consider links a problem if we are not the last link going into
		// the trash, and therefore only look at links if there are no more
		// links outside the trash

		if (!$ignore_other_links) {
			$trash_assetid = $this->getSystemAssetid('trash_folder');
			$trash_link = $this->getLinkByAsset($trash_assetid, $asset->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
			$num_other_links = $this->countLinks($asset->id, 'minor', SQ_LINK_TYPE_1 + SQ_LINK_TYPE_2, '', TRUE, $ignore_linkid);

			if (!empty($trash_link)) $num_other_links--;
		} else {
			$num_other_links = 0;
		}

		if (!$num_other_links) {
			// Get all the TYPE_3|NOTICE links this asset has in the system and
			// display them to the user
			// note that we are also going to ask the major asset in the link
			// to describe it so it makes sense to the user looking at it
			// NB: if there is a Safe Trash cron job attached to this, don't
			//     count that towards this
			$affected_links = $this->getLinks($asset->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, '', TRUE, 'minor');
			$safe_trash_cron_job = $this->getLinks($asset->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, 'minor');

			// We can't use array_diff() because it does a string comparison
			// ("Array" == "Array".... ooh nasty). So we will trawl through the
			// array to find the cron job and pluck it out
			if (!empty($safe_trash_cron_job)) {
				foreach (array_keys($affected_links) as $affected_link_key) {
					if ($affected_links[$affected_link_key]['linkid'] == $safe_trash_cron_job[0]['linkid']) {
						unset($affected_links[$affected_link_key]);
						break;
					}
				}
			}

			if (!empty($affected_links)) {
				// if type3 or notice linked to other asset, it is not safe to be trashed
				$trash_errors['links'] = $affected_links;
			}
		}

		// if the status is not under construction and not archived, it is not safe to be trashed
		if ($asset->status >= SQ_STATUS_LIVE) {
			$trash_errors['status'] = $asset->status;
		}

		// check the same thing for all children
		$children = $this->getChildren($assetid, '', TRUE, FALSE);

		if (!empty($children)) {
			try {
				$children_ids = array_keys($children);
				$bind_vars = Array(
								'link_type'	=> Array(SQ_LINK_TYPE_3, SQ_LINK_NOTICE),
								'status'	=> SQ_STATUS_LIVE,
								'assetid'	=> $children_ids,
							 );
				$result = MatrixDAL::executeAssoc('core', 'canSafeTrashAsset', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to check safe trash condition: '.$e->getMessage());
			}

			if (!empty($result)) {
				$used_minorids = Array();
				// Reduce the children array so there is only one entry for
				// each violating asset
				foreach (array_keys($result) as $result_key) {
					$minorid = $result[$result_key]['minorid'];
					if (in_array($minorid, $used_minorids)) {
						unset($result[$result_key]);
					} else {
						$used_minorids[] = $minorid;
					}//end if
				}

				$trash_errors['children'] = $result;
			}//end if
		}//end if

		return $trash_errors;

	}//end canSafeTrashAsset()


	/**
	* Send the specified asset directly to the trash, where possible
	*
	* This deletes its links to all its current parents, and creates a link for it in the trash folder
	*
	* @param string		$assetid		the asset to trash
	* @param boolean	$force_trash	do not abort trashing if any error occurs
	*
	* @return boolean
	* @access public
	*/
	function trashAsset($assetid, $force_trash=FALSE)
	{
		if ($this->assetInTrash($assetid, TRUE)) {
			// already in trash exclusively
			return TRUE;
		}

		// trash requests are charged at $4.95 a minute
		// please ask your parents for permission before trashing
		$parent_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', TRUE, 'minor');

		$locked = Array();
		foreach ($parent_links as $index => $link_details) {
			$asset = $this->getAsset($link_details['majorid']);
			// try to acquire the lock
			if ($this->acquireLock($link_details['majorid'], 'links')) {
				$locked[] = $link_details['majorid'];
			} else {
				trigger_localised_error('SYS0119', E_USER_WARNING, $assetid, $link_details['majorid']);
				if ($force_trash) {
					unset($parent_links[$index]);
					continue;
				} else {
					foreach ($locked as $aid) {
						$this->releaseLock($aid, 'links');
					}
					return FALSE;
				}
			}
			if (TRUE !== ($msg = $asset->canDeleteLink($link_details['linkid']))) {
				trigger_localised_error('SYS0120', E_USER_WARNING, $assetid, $link_details['majorid'], $msg);
				if ($force_trash) {
					unset($parent_links[$index]);
					continue;
				} else {
					foreach ($locked as $aid) {
						$this->releaseLock($aid, 'links');
					}
					return FALSE;
				}
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$trashid = $this->getSystemAssetid('trash_folder');

		$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() | SQ_SECURITY_LINK_INTEGRITY);
		foreach ($parent_links as $link_details) {
			// don't delete the link to the trash, we don't want to end up with an orphan asset
			if ($trashid == $link_details['majorid']) continue;

			// deleteAssetLink will create the trash link when no other links remain
			if (!$this->deleteAssetLink($link_details['linkid'])) {
				trigger_localised_error('SYS0152', E_USER_WARNING, $link_details['linkid']);
				foreach ($locked as $aid) {
					$this->releaseLock($aid, 'links');
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		foreach ($locked as $aid) {
			$this->releaseLock($aid, 'links');
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end trashAsset()


//--        PERMISSIONS        --//


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : NULL = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of userids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are FALSE)
	* @param boolean	$collapse_roles	When TRUE, only use the permissions table (ie. don't expand roles)
	*									Otherwise, the permissions view (which includes role expansions) are used
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE, $collapse_roles=FALSE)
	{
		$permission = (int) $permission;
		if (!is_null($granted)) $granted = (bool) $granted;

		// check if we are getting permissions for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);

			if (!is_null($asset)) {
				if (method_exists($asset, 'getPermission')) {
					$ret_val = $asset->getPermission($assetid, $permission, $granted, $and_greater, $expand_groups, $all_info);
				} else {
					$ret_val = $GLOBALS['SQ_SYSTEM']->am->getPermission($real_assetid, $permission, $granted, $and_greater, $expand_groups, $all_info);
				}
				$this->forgetAsset($asset);

				return $ret_val;
			}
		}

		if (($and_greater || $expand_groups) && $all_info) {
			trigger_localised_error('SYS0273', E_USER_NOTICE, __CLASS__, __FUNCTION__);
			$all_info = FALSE;
		}

		if (!isset($this->_tmp['permission_cache'])) {
			$this->_tmp['permission_cache'] = Array();
		}
		if (!isset($this->_tmp['permission_cache'][$assetid])) {
			$this->_tmp['permission_cache'][$assetid] = Array();
		}

		// return cached version if we can
		$suffix = $collapse_roles ? '1' : '0';
		$storage_name =(($and_greater) ? 'effective_' : '').'permission_'.$permission.'_'.$suffix;
		if (!isset($this->_tmp['permission_cache'][$assetid][$storage_name])) {

			$db = MatrixDAL::getDb();

			$table = ($collapse_roles) ? 'ast_perm' : 'vw_ast_perm';
			$sql = '  SELECT DISTINCT assetid, userid, granted
					  FROM '.SQ_TABLE_RUNNING_PREFIX.$table.' ';
			$where = 'assetid = :assetid
						AND permission '.(($and_greater) ? '>= ' : '= ').':permission';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);


			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'assetid',    $assetid, PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'permission', $permission, PDO::PARAM_INT);
				$result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get permissions of asset ID #'.$assetid.' due to database error: '.$e->getMessage());
			}

			// cache the result for next time
			$this->_tmp['permission_cache'][$assetid][$storage_name] = $result;

		}//end if

		$ret_val = Array();
		foreach ($this->_tmp['permission_cache'][$assetid][$storage_name] as $data) {

			if ($granted === FALSE && $data['granted'] != '0') {
				continue;
			} else if ($granted === TRUE  && $data['granted'] != '1') {
				continue;
			}

			// return all info
			if ($all_info) {
				$ret_val[$data['userid']] = $data['granted'];

			// else we just want user ids
			} else {

				// if we are expanding user groups and we aren't public access
				if ($expand_groups && $data['userid']) {
					$user = $this->getAsset($data['userid']);
					if (!is_null($user)) {
						if ($user instanceof User_Group) {
							$ret_val = array_merge($ret_val, array_keys($this->getChildren($user->id, Array('user'), FALSE)));
						}
						// always include the current user/group id, even if it is a user group
						$ret_val[] = $user->id;
					}
				} else {
					$ret_val[] = $data['userid'];
				}

			}//end if all info

		}//end foreach

		if ($all_info) {
			return $ret_val;
		} else {
			return array_unique($ret_val);
		}

	}//end getPermission()


	/**
	* Get list of permissions for a certain asset, filtered by cascading setting
	*
	* This is primarily used for showing settings on the Permissions screen.
	* Therefore, roles will NOT be collapsed.
	*/
	function getAssetPermissionByCascade($assetid, $permission, $userid=NULL, $cascades=NULL)
	{
		$sql = '  SELECT DISTINCT userid, granted, cascades
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_perm ';
		$where = 'assetid = :assetid AND permission = :permission';
		if (!is_null($cascades)) {
			$where .= ' AND cascades = :cascades';
		}
		if (!is_null($userid)) {
			$where .= ' AND userid = :userid';
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid',    $assetid, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'permission', $permission, PDO::PARAM_STR);
			if (!is_null($cascades)) {
				MatrixDAL::bindValueToPdo($query, 'cascades', $cascades ? '1' : '0', PDO::PARAM_STR);
			}
			if (!is_null($userid)) {
				MatrixDAL::bindValueToPdo($query, 'userid', $userid, PDO::PARAM_STR);
			}
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get set permissions of asset ID #'.$assetid.' due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getAssetPermissionByCascade()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$granted	whether this permission grants access or not
	* @param boolean	$cascades	whether this permission cascades to children or not
	* @param boolean	$force_set	if TRUE, existing permissions will be overridden even if the new permissions are found to conflict with them
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted, $cascades=TRUE, $force_set=FALSE)
	{
		$permission = (int)  $permission;
		$granted    = (bool) $granted;
		$db_action  = 'insert';

		// check if we are getting links for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);

			$ret_val = NULL;
			if (is_null($asset)) continue;
			if (method_exists($asset, 'setPermission')) {
				$ret_val = $asset->setPermission($assetid, $userid, $permission, $granted);
				$this->forgetAsset($asset);
			}

			return $ret_val;
		}

		$asset = $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('permissions')) {
			trigger_localised_error('SYS0111', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// check if this permission is already set
		$current = $this->getPermission($assetid, $permission, $granted, FALSE, FALSE, FALSE, TRUE);
		if (in_array($userid, array_values($current))) {
			// Check whether the cascade setting is also matched
			$perm_info = $this->getAssetPermissionByCascade($assetid, $permission, $userid, $cascades);
			if (!empty($perm_info)) {
				return TRUE;
			} else {
				// We are merely changing the cascade setting, so
				// set it to update
				$db_action = 'update';
			}
		}

		// if we dont have an userid, we are granting public access
		if (!empty($userid)) {
			// check that the passed userid is a user or user_group
			$user = $this->getAsset($userid, '', TRUE);
			if (!$user->id) {
				trigger_localised_error('SYS0112', E_USER_WARNING);
				return FALSE;
			} else if (!($user instanceof User) && !($user instanceof User_Group)) {
				trigger_localised_error('SYS0113', E_USER_WARNING, $user->type());
				return FALSE;
			}
			$user_name = $user->name;
		} else {
			$user_name = 'General Public';
		}
		// check if the reverse access of this permission is set
		$current = $this->getPermission($assetid, $permission, !$granted, FALSE, FALSE, FALSE, TRUE);
		if (in_array($userid, array_values($current))) {
			if ($force_set) {
				$db_action = 'update';
			} else {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				$perm_name = permission_type_name($permission);

				$new_access     = ($granted) ? 'grant'   : 'revoke';
				$current_access = ($granted) ? 'revoked' : 'granted';
				trigger_localised_error('SYS0123', E_USER_WARNING, $new_access, $perm_name, $user_name, $asset->name, $current_access);
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = MatrixDAL::getDb();

		switch ($db_action) {
			case 'insert':
				$sql = 'INSERT INTO
							sq_ast_perm
							(
								assetid,
								userid,
								permission,
								granted,
								cascades
							)
							VALUES
							(
								:assetid,
								:userid,
								:permission,
								:granted,
								:cascades
							)';

			break;
			case 'update':
				$sql = 'UPDATE
							sq_ast_perm
						SET
							granted  = :granted,
							cascades = :cascades
						WHERE
							assetid        = :assetid
							AND userid     = :userid
							AND permission = :permission';

			break;
		}//end switch

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'assetid',    $assetid);
					MatrixDAL::bindValueToPdo($query, 'userid',     $userid);
					MatrixDAL::bindValueToPdo($query, 'permission', $permission);
					MatrixDAL::bindValueToPdo($query, 'granted',    $granted ? '1' : '0', PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'cascades',   $cascades ? '1' : '0', PDO::PARAM_STR);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to '.$db_action.' permissions for asset "'.$asset->name.'" (#'.$asset->id.') due to database error: '.$e->getMessage());
				}

		if (!$asset->permissionsUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// clear the permission cache
		if (isset($this->_tmp['permission_cache'][$assetid])) {
			unset($this->_tmp['permission_cache'][$assetid]);
		}

		// log the action
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'perm_name'		=> permission_type_name($permission),
						'asset_name'	=> $asset->name,
						'user_name'		=> $user_name,
					);
		$message = $ms->newMessage(Array(), 'asset.permissions.'.($granted ? 'grant' : 'deny'), $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$message->send();

		// notify anyone interested that permission changed
		$changed_array = Array(
							'perm_name'		=> permission_type_name($permission),
							'asset_name'	=> $asset->name,
							'user_name'		=> $user_name,
						 );
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($asset, 'PermissionChange', $changed_array);

		return TRUE;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$assetid	the assetid for asset whose permissions to delete
	* @param int	$userid		the id of the user or user_group to remove the permission from
	* @param string	$permission	the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $userid, $permission)
	{
		$permission = (int) $permission;

		// check if we are getting links for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);

			if (is_null($asset)) continue;

			$ret_val = FALSE;
			if (method_exists($asset, 'deletePermission')) {
				$ret_val = $asset->deletePermission($assetid, $userid, $permission);
				$this->forgetAsset($asset);
			}

			return $ret_val;
		}

		$asset = $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('permissions')) {
			trigger_localised_error('SYS0104', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// if we dont have an userid, we are deleting public access
		if (!empty($userid)) {
			// if the permission is in the database then we know that it must have
			// been set up correctly, and if an user or LDAP user's asset has been deleted then we
			// know we want it out. So we'll use getAssetInfo() instead which does not throw an
			// assertion, just instead just gives an empty array
			$user_info = $this->getAssetInfo(Array($userid), 'user', FALSE);

			// check that the passed userid is a user or user_group
			if (empty($user_info)) {
				$user_name = 'Unknown User';
			} else {
				$user_name = $user_info[$userid]['name'];
			}
		} else {
			$user_name = 'General Public';
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars	= Array (
							'userid'		=> $userid,
							'permission'	=> $permission,
							'assetid'		=> $assetid,
						  );
			$result	= MatrixDAL::executeQuery('core', 'deletePermissionForUserOnAsset', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete permission '.$permission.' of userid #'.$userid.' on assetid #'.$assetid .' due to the following database error.'.$e->getMessage());
		}//end try catch

		if (!$asset->permissionsUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// clear the permission cache
		if (isset($this->_tmp['permission_cache'][$assetid])) {
			unset($this->_tmp['permission_cache'][$assetid]);
		}

		// log the action
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'perm_name'		=> permission_type_name($permission),
						'asset_name'	=> $asset->name,
						'user_name'		=> $user_name,
					);
		$message = $ms->newMessage(Array(), 'asset.permissions.delete', $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$message->send();

		// notify anyone interested that permission changed
		$changed_array = Array(
							'perm_name'		=> permission_type_name($permission),
							'asset_name'	=> $asset->name,
							'user_name'		=> $user_name,
						 );
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($asset, 'PermissionChange', $changed_array);

		return TRUE;

	}//end deletePermission()


//--        ROLES        --//


	/**
	* Returns an array of roles and the users/groups which can perform it
	*
	* @param int		$assetid			the assetid of the asset the role is applied to
	* @param int		$roleid				the assetid of the the role that is applied
	* @param int		$userid				the assetid of the user performing the role
	* @param boolean	$include_assetid	whether to include the assetid in the returned array
	* @param boolean	$include_globals	whether to query the role view which includes
	*										expanded global roles as individual users
	* @param boolean	$expand_groups		when TRUE, any groups defined within a role will be
	*										replaced with the userids in that group.
	*										If FALSE, return the groupids
	* @param boolean	$inc_dependants		If false it will filter out the dependant assets
	* @param boolean	$include_parents		When $userid is specified. This will include the roles applied directly to the user and indirectly to user parent groups.
	* @param array	$type_codes		When $include_assetid is TRUE. This is a type code filter for returned assetids.
	* @param array	$strict_type_code		Use strict type code for type code filter
	*
	* @return array
	* @access public
	*/
	function getRole($assetid=NULL, $roleid=NULL, $userid=NULL, $include_assetid=FALSE, $include_globals=FALSE, $expand_groups=FALSE, $inc_dependants=TRUE, $include_parents=FALSE, $type_codes=Array(),  $strict_type_code=TRUE)
	{
		if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '0' && SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '0') return Array();

		$db = MatrixDAL::getDb();

		$where = Array();
		if (!is_null($assetid)) {
			$where[] = 'r.assetid = :assetid';
		}

		if (!is_null($roleid)) {
			$where[] = 'r.roleid = :roleid';
		}

		if (!is_null($userid)) {
			// find all parent user groups of user id to search roles
			if($include_parents && !empty($userid)) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($userid, 'user_group', FALSE);
			foreach ($parents as $usergroupid => $type_code) {
				if($type_code == 'role') {
				unset ($parents[$usergroupid]);
				}
			}
			$parents = array_keys($parents);
			for ($i = 0; $i < count($parents); $i++) {
				$parents[$i] = MatrixDAL::quote($parents[$i]);
			}
			$parents[] =':userid';
			$in_clause = Array();
			// break up the assets into chunks of 1000 so that oracle does not complain
			foreach (array_chunk($parents, 999) as $chunk) {
				$in_clause[] = ' r.userid IN ('.implode(', ', $chunk).')';
			}
			$where[] = '('.implode(' OR ',$in_clause).')';
			}
		   else {
			   // just use the user id
			$where[] = 'r.userid = :userid';
		   }
		}

		$where_clause = '';
		if (!empty($where)) {
			$where_clause = 'WHERE '.implode(' AND ', $where);
		}

		$table = ($include_globals) ? 'vw_ast_role r' : 'ast_role r';
		$additional = $include_assetid ? ', r.assetid' : '';

		if (!$inc_dependants) {
			// join to link table to get the non dependant assets
			$where_or_and = ' AND ';
			if(empty($where_clause)) {
				$where_or_and = 'WHERE ';
			}
			$where_clause .= $where_or_and.'r.assetid = l.minorid AND l.is_dependant = :is_dependant';
			$table .= ', '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l';
			$where_clause = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, 'l');
		}

		// type code filter
		if ($include_assetid && !empty($type_codes)) {
			// join to asset table to get type codes
				$where_or_and = ' AND ';
			if(empty($where_clause)) {
				$where_or_and = 'WHERE ';
			}
			$where_clause .= $where_or_and.'r.assetid = a.assetid';
			$table .= ', '.SQ_TABLE_RUNNING_PREFIX.'ast a';
			$type_codes_quoted = Array();
			foreach ($type_codes as $tc) {
				$type_codes_quoted[] = MatrixDAL::quote($tc);
			}
			if (!$strict_type_code) {
				foreach ($type_codes as $tc) {
					$ancestors = $this->getTypeDescendants($tc);
					foreach ($ancestors as $ancestor) {
						$type_codes_quoted[] = MatrixDAL::quote($ancestor);
					}
				}
			}
			$where_clause .= ' AND a.type_code IN ('.implode(',', $type_codes_quoted).')';
		}
		$where_clause = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, 'r');
		$sql = 'SELECT DISTINCT r.roleid, r.userid'.$additional.'
				FROM '.SQ_TABLE_RUNNING_PREFIX.$table.' '.$where_clause;

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			if (!is_null($assetid)) {
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid, PDO::PARAM_STR);
			}
			if (!is_null($roleid)) {
				MatrixDAL::bindValueToPdo($query, 'roleid',  $roleid,  PDO::PARAM_STR);
			}
			if (!is_null($userid)) {
				MatrixDAL::bindValueToPdo($query, 'userid',  $userid,  PDO::PARAM_STR);
			}
			if (!$inc_dependants) {
				MatrixDAL::bindValueToPdo($query, 'is_dependant', '0', PDO::PARAM_STR);
			}
			$result = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get role data due to database error: '.$e->getMessage());
		}

		$roles = Array();
		foreach ($result as $roleid => $userid_assetids) {
			foreach ($userid_assetids as $index => $userid_assetid) {
				if ($include_assetid) {
					if ($userid_assetid['userid'] == '0') continue;
					$assetid = $userid_assetid['assetid'];
					$userid = $userid_assetid['userid'];
					$roles[$roleid][$userid][] = $assetid;
				} else {
					$userid = $userid_assetid['userid'];
					$roles[$roleid][] = $userid;
				}
			}
		}
		$result = $roles;

		if ($expand_groups) {
			$roles = Array();
			foreach ($result as $roleid => $userid_assetids) {
				// need to handle include_assetid changes
				if ($include_assetid) {
					$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($userid_assetids), Array(), FALSE, 'type_code');
					foreach ($userid_assetids as $userid_assetid => $assetids) {
						if ($userid_assetid == '0') continue;
						if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($info[$userid_assetid], 'user_group')) {
							$expanded_users = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($userid_assetid, Array('user'), FALSE));
							if (!empty($expanded_users)) {
								if (isset($roles[$roleid])) $roles[$roleid] = Array();
								foreach ($expanded_users as $expanded_userid) {
									$roles[$roleid][$expanded_userid] = $assetids;
								}
							}
						} else {
							$roles[$roleid][$userid] = $assetids;
						}
					}
				} else {
					$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid_assetids, Array(), FALSE, 'type_code');
					foreach ($userid_assetids as $index => $userid_assetid) {
						if ($userid_assetid == '0') continue;
						if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($info[$userid_assetid], 'user_group')) {
							$expanded_users = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($userid_assetid, Array('user'), FALSE));
							if (!empty($expanded_users)) {
								if (isset($roles[$roleid])) {
									$roles[$roleid] = array_merge($roles[$roleid], $expanded_users);
								} else {
									$roles[$roleid] = $expanded_users;
								}
							}
						} else {
							$roles[$roleid][] = $userid_assetid;
						}
					}
				}
			}//end foreach result
			$result = $roles;
		}//end if expanding groups

		return $result;

	}//end getRole()


	/**
	* Apply a role to an asset and specify a user to perform the role
	*
	* @param int	$assetid	the assetid of the asset
	* @param int	$roleid		the assetid the role
	* @param int	$userid		the assetid of the user performing the role
	*
	* @return boolean
	* @access public
	*/
	function setRole($assetid, $roleid, $userid)
	{
		if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '0' && SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '0') return FALSE;

		// check if we have permission to set roles
		$asset = $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('roles')) {
			trigger_localised_error('SYS0313', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// Validate role and user assets
		if (!empty($roleid)) {
			$role_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($roleid, Array('role'));
			if (empty($role_info)) {
				trigger_localised_error('CORE0306', E_USER_WARNING, $roleid);
				return FALSE;
			}
		}
		if (!empty($userid)) {
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid, Array('user', 'user_group'), FALSE);
			if (empty($user_info)) {
				trigger_localised_error('CORE0307', E_USER_WARNING, $userid);
				return FALSE;
			}
		}

		// check if this role is already set
		$current = $this->getRole($assetid, $roleid, $userid);
		if (in_array($userid, array_get_index($current, $roleid, Array()))) {
			return TRUE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars = Array(
							'assetid'	=> $assetid,
							'roleid'	=> $roleid,
							'userid'	=> $userid,
						 );
			MatrixDAL::executeQuery('core', 'setRole', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to set the role for asset: '.$assetid.' due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setRole()


	/**
	* Remove a role from an asset
	*
	* @param int	$assetid	the assetid of the asset
	* @param int	$roleid		the assetid the role
	* @param int	$userid		the assetid of the user performing the role
	*
	* @return boolean
	* @access public
	*/
	function deleteRole($assetid, $roleid=NULL, $userid=NULL)
	{
		if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '0' && SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '0') return FALSE;

		// check if we have permission to set roles
		$asset = $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('roles')) {
			trigger_localised_error('SYS0313', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$where_and = '';
		if (!is_null($roleid)) {
			$where_and .= ' AND roleid = '.MatrixDAL::quote($roleid);
		}

		if (!is_null($userid)) {
			$where_and .= ' AND userid = '.MatrixDAL::quote($userid);
		}

		$sql = 'DELETE FROM sq_ast_role
				WHERE assetid = :assetid';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where_and);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$result = MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to delete roleid on assetid #'.$assetid.' due to the following database error:'.$e->getMessage());
		}//end try catch

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteRole()


//--        URL's & HREF's        --//


	/**
	* Returns a reference to the asset that has the passed url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol		the protocol to match -> null means it is ignored
	* @param string		$url			the urls to check for -> null defaults it to current url
	* @param boolean	$exact_match	when TRUE only returns an asset if there is an exact
	*									match on the url,
	*									if FALSE searches back along the URL path for the
	*									asset that matches
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getAssetFromURL($protocol=NULL, $url=NULL, $exact_match=TRUE, $mute_errors=FALSE)
	{
		if (is_null($url)) $url = current_url(FALSE, TRUE);
		if (!is_string($url)) {
			trigger_localised_error('SYS0189', E_USER_WARNING);
			$null = NULL;
			return $null;
		}

		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') {
			$protocol = 'http';
		}

		$db = MatrixDAL::getDb();

		$url_condition = '';
		if ($exact_match) {
			$url_condition = ' = '.MatrixDAL::quote($url);
		} else {
			$sections = explode('/', $url);
			$option  = '';
			$url_condition = ' IN (';
			foreach ($sections as $piece) {
				$url_condition .= (($option) ? ',' : '').MatrixDAL::quote($option.$piece);
				$option .= $piece.'/';
			}
			$url_condition .= ') ';
		}//end if

		$where = 'l.url '.$url_condition;

		if (!is_null($protocol)) {
			$where .= ' AND l.'.$protocol.' = '.MatrixDAL::quote('1');
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT l.url, l.assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
				'.$where.'
				ORDER BY LENGTH(l.url) DESC';
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

		$row = MatrixDAL::executeSqlAll($sql);

		// URL not found
		if (empty($row)) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			if (!$mute_errors) {
				trigger_localised_error('SYS0245', E_USER_WARNING, $fancy_url);
			}
			$null = NULL; // 'cause we are meant to return by reference
			return $null;

		} else {
			$row = $row[0];
			$asset = $this->getAsset($row['assetid'], '', $mute_errors);
			return $asset;
		}//end if

	}//end getAssetFromURL()


	/**
	* Returns the information from the sq_asset_url table for the root url closest to the passed URL
	* i.e. returns the info for the site closest to this url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string	$url	the urls to check for (excluding protocol) -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getRootURL($url=NULL)
	{
		if (is_null($url)) $url = current_url(FALSE, TRUE);
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		$db = MatrixDAL::getDb();

		// so much for SQL standards...
		if (strpos($url, '/') === FALSE) {
			$where = MatrixDAL::quote($url).' LIKE (url || \'%\')';
		} else {
			$url_parts = explode('/', $url);
			$waste = array_pop($url_parts);
			if (count($url_parts) > 1) {
				$parent_url = implode('/', $url_parts);
			} else {
				$parent_url = array_pop($url_parts);
			}//end if
			$where = MatrixDAL::quote($parent_url.'/').' LIKE (url || \'/%\') OR url ='.MatrixDAL::quote($url);
		}//end if
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT urlid, assetid, url, http, https, base_contextid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_url
				'.$where.'
				ORDER BY url DESC';
		// TODO: replace in queries.xml with <limit> tag
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);
		$result = MatrixDAL::executeSqlAll($sql);
		return empty($result) ? NULL : $result[0];

	}//end getRootURL()


	/**
	* Returns an array of URLs of an asset, including the protocol information
	*
	* @param int	$assetid	the assetid of the asset
	*
	* @return array
	* @access public
	*/
	function getURLs($assetid)
	{
		$sql = 'SELECT
					http, https, url
				FROM
					sq_ast_lookup
				WHERE
					assetid = :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get URLs for asset ID '.$assetid.' due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getURLs()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from the site url
	*
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol		the protocol to match -> null means it is ignored
	* @param string		$url			the urls to check for -> null defaults it to current url
	* @param boolean	$mute_errors	Mute errors - to stop errors appearing on a clean system
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($protocol=NULL, $url=NULL, $mute_errors=FALSE)
	{
		if (is_null($url)) $url = current_url(FALSE, TRUE);

		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') {
			$protocol = 'http';
		}

		$db = MatrixDAL::getDb();

		$non_standard_url = FALSE;
		// get the root(site) url for this url
		$root_url = $this->getRootURL($url);
		if (empty($root_url)) {
			$found = FALSE;
			$non_standard_url = TRUE;
			// We want to check if this is an asset (file) with a publicly served url
			$fancy_asset = $this->getAssetFromURL($protocol, $url, TRUE, TRUE);
			if (empty($fancy_asset)) {
				$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
				if (!$mute_errors) {
					trigger_localised_error('SYS0247', E_USER_WARNING, $fancy_url);
				}//end if
				return Array();
			}
			$lineages = $this->getLinkLineages($fancy_asset->id);

			// Determine which site we are in, and find one of our lineages that lies in that site
			$all_sites = $this->getParents($fancy_asset->id, 'site');
			$valid_sites = Array();
			foreach ($all_sites as $id => $type_code) {
				$current_site = $this->getAsset($id);
				if (!is_object($current_site)) continue;
				$site_url = $original_site_url = $current_site->getUrl();
				$original_site_url = $site_url = str_replace(Array('http://', 'https://'), '', $site_url);
				$valid = FALSE;
				if (strpos($url, $site_url) !== FALSE) {
					$valid = TRUE;
				} else {
					while (strrpos($site_url, '/') !== FALSE) {
						$site_url = substr($site_url, 0, strrpos($site_url, '/'));
						if (strpos($url, $site_url) !== FALSE) {
							$valid = TRUE;
							break;
						}
					}
				}
				// We found one, add it to the array
				if ($valid) $valid_sites[$id] = $original_site_url;
			}

			$root_id = 0;
			if (!empty($valid_sites)) {
				if (count($valid_sites) == 1) {
					// Only 1 valid site, lets go with it
					$root_id = current(array_keys($valid_sites));
				} else {
					// More than one site, we need more filtering

					// Go on current url?
					$current_site = $this->getRootUrl(current_url(FALSE, TRUE));
					$urls = array_flip($valid_sites);
					if (isset($urls[$current_site['url']])) {
						$root_id = $urls[$current_site['url']];
					}
				}
			}

			$link_id = 0;
			if ($root_id != 0) {
				foreach ($lineages as $id => $data) {
					if (isset($data['lineage'][$root_id])) {
						$link_id = $data['linkid'];
						break;
					}
				}
			}
			if ($link_id == 0) {
				$link_id = current(array_keys($lineages));
			}

			$link = $this->getLinkById($link_id);
			if (empty($link)) return Array();
			$parent = $this->getAsset($link['majorid']);
			if (empty($parent)) return Array();
			$parent_url = $parent->getUrl();

			// Lets use our parent's url to get our lineage
			$parent_url = str_replace(Array('http://', 'https://'), '', $parent_url);

			if (empty($parent_url)) return Array();

			// Now lets set the url to our parents url, and continue with the rest of the function as normal
			$root_url = $this->getRootURL($parent_url);

			$url = $parent_url;

			if (empty($root_url)) {
				$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
				trigger_localised_error('SYS0247', E_USER_WARNING, $fancy_url);
				return Array();
			}

		}//end if empty $root_url

		$urls = get_url_lineage($url, TRUE);
		$urls[] = MatrixDAL::quote($url);
		$in_sql = implode(',', $urls);

		$db_type = MatrixDAL::getDbType();

		$where = 'l.root_urlid = '.(int)$root_url['urlid'].'
				  AND l.url IN (
								'.$in_sql.'
							)';

		if (!is_null($protocol)) {
			$where .= ' AND l.'.$protocol.' = '.MatrixDAL::quote('1');
			$protocol_select = MatrixDAL::quote($protocol);
		} else {
			$curr_proto = current_protocol();
		$protocol_select = 'CASE WHEN l.'.$curr_proto.' = '.MatrixDAL::quote('1').' THEN '.MatrixDAL::quote($curr_proto).' ELSE '.MatrixDAL::quote(($curr_proto == 'http') ? 'https' : 'http').' END';
		}

		// if we are using oracle, we cannot specify an alias
		// in a group by clause. We must use the result set definition
		if ($db_type == 'pgsql') {
			$group_by_protocol = 'protocol';
		} else if ($db_type == 'oci') {
			$group_by_protocol = $protocol_select;
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT l.url, l.assetid, a.name, a.short_name, a.type_code, a.status, '.$protocol_select.' as protocol, MIN(al.link_type) AS link_type
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
				  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.assetid = a.assetid
				  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk al ON a.assetid = al.minorid
				'.$where.'
				GROUP BY l.url, l.assetid, a.name, a.short_name, a.type_code, a.status, '.$group_by_protocol.'
				ORDER BY l.url ASC';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get lineage for URL "'.$protocol.'://'.$url.'" due to database error: '.$e->getMessage());
		}

		$url_asset = $this->getAssetFromURL($protocol, $url);
		if (!is_null($url_asset)) {
			$id_parts = explode(':', $url_asset->id);
		}
		if (isset($id_parts[1])) {

			$real_assetid = $id_parts[0];
			$asset = $this->getAsset($real_assetid);
			if (!method_exists($asset, 'getLineageFromURL')) {
				trigger_localised_error('SYS0205', E_USER_WARNING, $asset->name);
				return Array();
			}
			$bridge_lineage = $asset->getLineageFromURL($id_parts[1], $protocol, $url);

			$result = array_merge($result, $bridge_lineage);

		} else if (empty($result) || (($result[count($result) - 1]['url'] != $url) && (!$non_standard_url))) {

			// no urls found or we only found some of the parents, but not the passed url
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			trigger_localised_error('SYS0246', E_USER_WARNING, $fancy_url);
			return Array();

		} else {
			// if there is only one entry then this is the site root, so for completeness
			// we will manually add in the index page to the index
			if (count($result) == 1) {
				$site = $this->getAsset($result[0]['assetid'], $result[0]['type_code']);
				if (is_null($site)) return Array();
				$index_page = $site->getSpecialPage('index');
				if (!is_null($index_page)) {
					$paths = $index_page->getWebPaths();
					$result[] = Array(
									'url'			=> $result[0]['url'].((empty($paths)) ? '' : '/'.$paths[0]),
									'assetid'		=> $index_page->id,
									'name'			=> $index_page->name,
									'short_name'	=> $index_page->short_name,
									'type_code'		=> $index_page->type(),
									'status'		=> $index_page->status,
									'link_type'		=> SQ_LINK_TYPE_3,
									'protocol'		=> current_protocol(),
								);
				}//end if
			}//end if
		}//end if



		// Check whether the asset is in Safe edit. If it is, load the save version of asset.
		// for name and short_name attribute
		foreach ($result as $key => $entry) {
			// We only need to worry about the safe edit statii, save retrieving all assets
			if ($entry['status'] & SQ_SC_STATUS_SAFE_EDITING) {
				$asset = $this->getAsset($entry['assetid']);
				if ($asset->useSystemVersion()) {
					$asset->_loadDataPaths();
					// make sure our data directory exists
					if (!file_exists($asset->data_path.'/.object_data')) {
						trigger_localised_error('SYS0088', E_USER_WARNING, $asset->id);
						$this->forgetAsset($asset);
						return;
					}

					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$data_object = get_object_vars(unserialize(file_to_string($asset->data_path.'/.object_data')));
					$result[$key]['name']			= $data_object['name'];
					$result[$key]['short_name']	= $data_object['short_name'];
				}
				$this->forgetAsset($asset);
			}//end if
		}//end foreach

		return $result;

	}//end getLineageFromURL()


	/**
	* Given lineage of asset ids from top to bottom, this funciton will pick an URL of the bottom asset (current asset)
	* that matches with given lineage.
	*
	*
	* @param array		$lineage		array of asset ids from top to bottom
	* @param string		$root_url		additional root url to help filtering the results in case the root site has multiple urls
	*
	* @return array
	* @access public
	*/
	function getUrlFromLineage($lineage = Array(), $root_url = NULL, $protocol = NULL)
	{
		// get the current asset's all urls
		$current_asset_id = array_pop($lineage);
		$candidate_urls = $this->getURLs($current_asset_id);

		// reverse traverse the lineage array to allow filtering special site urls
		// e.g siteA has url site.com,  siteB has url site.com/B
		// if an asset is linked to both sites and given lineage of siteA. we can inspect from bottom lineage to top,
		// and only select the shortest matched URL, filter out rest. so it won't accidentally include siteB url in the result.
		$lineage = array_reverse($lineage);
		foreach ($lineage as $assetid) {
			$done_urls = Array();
			$asset_urls = $this->getURLs(trim($assetid));

			foreach ($candidate_urls as $candidate_url) {
			foreach ($asset_urls as $asset_url) {
				if(strpos($candidate_url['url'], $asset_url['url']) === 0) {
				// for candidate urls that matched with same asset url, we only keep the shortest
				if(!isset($done_urls[$asset_url['url']]) || (strlen($done_urls[$asset_url['url']]['url']) > strlen($candidate_url['url'])))  {
					$done_urls[$asset_url['url']] = $candidate_url;
				}
				}
			}
			}
			$candidate_urls = array_values($done_urls);
		}


		$result = Array();
		foreach ($candidate_urls as $url) {
			// filter out those don't match with root url
			if($root_url) {
			 $root_url = preg_replace('/^http(s)?:\/\//', '', $root_url);
			if(strpos($url['url'], $root_url) !== 0) continue;
			}

			// https and http are different urls
			if($url['http']) {
				if(empty($protocol) ||  $protocol === 'http')
					$result[] = 'http://' . $url['url'];
			}
			if($url['https']) {
				if(empty($protocol) ||  $protocol === 'https')
					$result[] = 'https://' . $url['url'];
			}

		}

		return $result;

	}//end getUrlFromLineage()


	/**
	* Return the design for the passed URL
	*
	* @param string	$url			the url to get the design for
	* @param string	$design_name	the code for the design (eg. design::system::frontend)
	*
	* @return array
	* @access public
	*/
	function getDesignFromURL($url, $design_name='design::system::frontend')
	{
		$db = MatrixDAL::getDb();

		$urls = get_url_lineage($url, TRUE); //inherited
		$urls[] = MatrixDAL::quote($url); //applied
		$urls[] = MatrixDAL::quote($url.'/'); //override
		$urls = implode(',', $urls);

		/**
		* Since we have the url,
		* we don't need to include the ast_lookup table in the query.
		* we can just join the ast_lookup_value table to ast directly.
		*/
		$sql = 'SELECT lv.value as designid, a.type_code
			FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv
			INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON lv.value = a.assetid
			';
		$where = 'lv.name = :design_name AND ' .
					'lv.url IN ( '.$urls.' )';

		$order_by = ' ORDER BY depth DESC';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where . $order_by;

		$query = MatrixDAL::preparePdoQuery($sql);

		MatrixDAL::bindValueToPdo($query, 'design_name', $design_name);

		$result = MatrixDAL::executePdoAll($query);
		return empty($result) ? NULL : $result[0];

	}//end getDesignFromURL()


	/**
	* Return an inheritable design value, given a URL
	*
	* Currently called by Asset::updateLookups() when a design or paint layout is being
	* handled and the asset's gone to its parents for an inherited value. In the case
	* of designs we need to now go directly to links, because we can't use the lookup
	* table values by itself anymore - we might end up inheriting an override instead
	*
	* @param string	$url			the url to get the design for
	* @param string	$design_name	the code for the design (eg. design::system::frontend)
	*
	* @return mixed array|NULL
	* @access public
	* @see Asset::updateLookups()
	*/
	function getInheritedDesignValueFromURL($url, $design_name='design::system::frontend')
	{
		$asset = $this->getAssetFromURL(NULL, $url, TRUE, TRUE);
		$design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $design_name);

		while (empty($design_link) && ($url != '')) {
			$slash_pos = strrpos($url, '/');
			if ($slash_pos === FALSE) $slash_pos = 0;
			$url = substr_replace($url, '', $slash_pos);

			$asset = $this->getAssetFromURL(NULL, $url, TRUE, TRUE);
			if (empty($asset)) break;

			$design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $design_name);
		}

		if (empty($design_link)) return NULL;

		return Array(
				'designid'	=> $design_link['minorid'],
				'type_code'	=> $design_link['minor_type_code'],
			   );

	}//end getInheritedDesignValueFromURL()


	/**
	* Return the lookup value for the passed URL
	*
	* @param string	 $url			the url to get the design for
	* @param string	 $value_name	the code for the design/layout (eg. design/paint_layout::system::frontend),
	* 								the code pattern if using $like_value (eg. design/paint_layout::%)
	* @param boolean $like_value	TRUE = LIKE $value_name, otherwise direct match to $value_name
	*
	* @return string
	* @access public
	*/
	function getValueFromURL($url, $value_name, $like_value = FALSE)
	{
		$db = MatrixDAL::getDb();

		$urls = get_url_lineage($url, TRUE); //inherited
		$urls[] = MatrixDAL::quote($url); //applied
		$urls[] = MatrixDAL::quote($url.'/'); //override
		$urls = implode(',', $urls);

		$sql = 'SELECT value, depth
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value';

		if ($like_value){
			$where = 'name LIKE :value_name';
		} else {
			$where = 'name = :value_name';
		}

		$where .= ' AND url IN ( '.$urls.' )';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$order_by = ' ORDER BY depth DESC';

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where.$order_by);
			MatrixDAL::bindValueToPdo($query, 'value_name', $value_name);
			$result = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get lookup value: '.$value_name.' for URL: '.$url.' due to database error: '.$e->getMessage());
		}

		return $result;

	}//end getValueFromURL()


	/**
	* Get assetids that match the given lookup name and value
	*
	* @param string		$lookup_name_exp	The name of the lookup value, or a SQL expression to match it
	* @param string		$lookup_value_exp	The value of the lookup value, or a SQL expression to match it
	* @param boolean	$include_treeids	If true, the result will be an array indexed by assetid where the values are arrays of treeids for that assetid.  Otherwise it will be a simple array of treeids.
	*
	* @return array
	* @access public
	*/
	function getAssetidsByLookupValue($lookup_name_exp, $lookup_value_exp, $include_treeids)
	{
		$use_like_for_name = ((FALSE !== strpos($lookup_name_exp, '?')) || (FALSE !== strpos($lookup_name_exp, '%')));
		$name_operator = $use_like_for_name ? 'LIKE' : '=';
		$use_like_for_value = ((FALSE !== strpos($lookup_value_exp, '?')) || (FALSE !== strpos($lookup_value_exp, '%')));
		$value_operator = $use_like_for_value ? 'LIKE' : '=';
		$db = MatrixDAL::getDb();

		/**
		 * Query 1 gets all the override values.
		 * Query 2 gets all the inherited values.
		 *
		 * These are both DISTINCT's because we're potentially joining to the ast_lnk
		 * table which can have different link values for the same assetid.
		 */
		$query1 = 'SELECT DISTINCT l.assetid'.($include_treeids ? ', lt.treeid' : '').', \'override\' as override';
		$query2 = 'SELECT DISTINCT l.assetid'.($include_treeids ? ', lt.treeid' : '').', \'not_override\' as override';

		/**
		 * The joins are a little different.
		 * override values are based on (almost) matching urls.
		 */
		$query1 .= '
			FROM sq_ast_lookup l
			JOIN sq_ast_lookup_value lv ON (lv.url = l.url OR (lv.url = l.url || \'/\'))
			';

		/**
		 * inherited values are based on like urls (basically, they start the same).
		 */
		$query2 .= '
			FROM sq_ast_lookup l
			JOIN sq_ast_lookup_value lv ON (l.url LIKE lv.url || \'/%\')';

		/**
		 * If we're including tree id's, we need to join to the lnk table,
		 * which has the matching of lookup.assetid = lnk.minorid
		 * so then we can get to the treeid.
		 */
		$where = '';
		if ($include_treeids) {
			$where = '
				JOIN sq_ast_lnk al ON l.assetid = al.minorid
				JOIN sq_ast_lnk_tree lt ON lt.linkid = al.linkid
				';
		}

		/**
		 * Both queries need to restrict based on the lookup name and the value.
		 */
		$where .= '
			WHERE
			lv.name '.$name_operator.' '.MatrixDAL::quote($lookup_name_exp).'
			AND lv.value '.$value_operator.' '.MatrixDAL::quote($lookup_value_exp);

		$sql = $query1.$where.' UNION ALL '.$query2.$where;

		$res = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			$res = MatrixDAL::executePdoGrouped($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get asset ids: '.$e->getMessage());
		}

		return $res;

	}//end getAssetidsByLookupValue()


	/**
	* Returns the URL for an asset, closest to the $base_url
	*
	* It returns string if $assetids is an int and an array(int => string) if $assetids is an array
	*
	* @param int|array	$assetids			the id of the asset you want the url for or an array
	*										for a whole bunch of them
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks
	* @param int		$base_contextid		the id of the context that you want the url from
	*
	* @return mixed int|array
	* @access public
	*/
	function getAssetURL($assetids, $base_url=NULL, $ignore_rollback=FALSE, $base_contextid = NULL)
	{
		if (!isset($this->_tmp['urls'])) {
			$this->_tmp['urls'] = Array();
		}

		$return_array = TRUE;
		$in_assetid = 0;
		if (!is_array($assetids)) {
			if (empty($assetids)) return '';
			$in_assetid = $assetids;
			$assetids = Array($assetids);
			$return_array = FALSE;
		} else {
			if (empty($assetids)) return Array();
		}

		$return_urls = Array();

		if (!$ignore_rollback && SQ_ROLLBACK_VIEW) {
			if ($return_array) {
				foreach ($assetids as $assetid) {
					$asset = $this->getAsset($assetid);
					$return_urls[$assetid] = $asset->getURL($base_url, TRUE, $base_contextid);
				}
				return $return_urls;
			} else {
				$asset = $this->getAsset($assetids[0]);
				return $asset->getURL($base_url, TRUE, $base_contextid);
			}
		}

		if (is_null($base_url)) {
			$base_url = current_url(FALSE, TRUE);
		}

		$matches = Array();
		if (!preg_match('/^(([a-z]+):\/\/)?(.*)$/', strip_url($base_url), $matches)) {
			return ($return_array) ? Array() : '';
		}
		$protocol = ($matches[2] == '') ? current_protocol() : (($matches[2] == 'https') ? 'https' : 'http');
		$base_url = $matches[3];

		// URL index for the URL cache should include protocol that we are looking
		// for, even if we return a forced secure/insecure URL.
		$url_index = $protocol.'://'.$base_url;

		if (is_null($base_contextid)) {
			$cache_index = $GLOBALS['SQ_SYSTEM']->getContextId();
		} else {
			$cache_index = $base_contextid;
		}

		// work out if any of these are cached
		foreach ($assetids as $assetid) {
			if (!isset($this->_tmp['urls'][$assetid])) {
				$this->_tmp['urls'][$assetid] = Array();
			}

			if (isset($this->_tmp['urls'][$assetid][$url_index][$cache_index])) {
				// use the cached version
				$return_urls[$assetid] = $this->_tmp['urls'][$assetid][$url_index][$cache_index];
			}
		}

		// now hit the database for the rest of them
		$db = MatrixDAL::getDb();

		$the_rest = array_diff($assetids, array_keys($return_urls));

		if (!empty($the_rest)) {
			$assetid_cond_array = $the_rest;

			// IN query needs to be chunked to be Oracle-safe (is also slow
			// in big doses on PostgreSQL). 200 at a time should be fine
			$result = Array();
			foreach (array_chunk($assetid_cond_array, 200) as $chunk) {
				foreach ($chunk as $key => $value) {
					$chunk[$key] = MatrixDAL::quote($chunk[$key]);
				}
				$assetid_cond = 'IN ('.implode(', ', $chunk).')';

				$url_bits = explode('/', preg_replace('/\\/+$/', '', $base_url));

				$where = 'l.assetid '.$assetid_cond;
				if (!is_null($base_contextid)) {
					$where .= ' AND u.base_contextid = '.MatrixDAL::quote($base_contextid);
				}
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
				$sql = 'SELECT l.assetid, l.url,
					CASE WHEN l.'.$protocol.' = \'1\' THEN \''.$protocol.'\' ELSE \''.(($protocol == 'https') ? 'http' : 'https').'\' END as protocol,
				(
					(CASE WHEN l.'.$protocol.' = \'1\' THEN 1 ELSE 0 END)';

				$str = '';
				foreach (array_values($url_bits) as $bit) {
					$str .= $bit.'/';
					$sql .= ' +
					(CASE WHEN SUBSTR(l.url || \'/\', 1, '.strlen($str).') = '.MatrixDAL::quote($str).' THEN 1 ELSE 0 END)';
				}

				if (is_null($base_contextid)) {
					$from = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l ';
				} else {
					$from = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_url u ON l.root_urlid = u.urlid ';
				}

				$sql .= '
				) as weighting
				'.$from.$where.'
				ORDER BY weighting DESC, LENGTH(l.url) ASC';

				$partial_result = MatrixDAL::executeSqlGroupedAssoc($sql);
				$result += $partial_result;
				unset($partial_result);
			}

			foreach ($the_rest as $assetid) {
				if (!isset($result[$assetid])) {
					$this->_tmp['urls'][$assetid][$url_index][$cache_index] = '';
				} else {
					// go through each result and find __data url
					// if there is __data url, and there are other local urls, we should only pick __data url for public users
					// for public user, we just return  __data url because having __data url meaning the file asset is effectively unrestricted
					if(count($result[$assetid]) > 1) {
						foreach ($result[$assetid] as $data) {
						$file_url = current_protocol().'://'.$data['url'];
						$is_unrestricted_url = SQ_CONF_STATIC_ROOT_URL ? strpos($file_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE : strpos($file_url, '/__data') !== FALSE;
						$is_matrix_url = SQ_CONF_STATIC_ROOT_URL ? strpos($file_url, SQ_CONF_STATIC_ROOT_URL.'/') === FALSE : strpos($file_url, '/__data') === FALSE;
						if((($GLOBALS['SQ_SYSTEM']->user instanceof Public_User) && $is_unrestricted_url)
						|| (!($GLOBALS['SQ_SYSTEM']->user instanceof Public_User) && $is_matrix_url)) {
							$result[$assetid][0] = $data;
							break;
						}
						}
					}
					$this->_tmp['urls'][$assetid][$url_index][$cache_index] = $result[$assetid][0]['protocol'].'://'.$result[$assetid][0]['url'];
				}
				$return_urls[$assetid] = $this->_tmp['urls'][$assetid][$url_index][$cache_index];
			}
		}//end if

		if ($return_array) {
			return $return_urls;
		} else {
			return $return_urls[$in_assetid];
		}

	}//end getAssetURL()


	/**
	* Returns the href for an asset closest to the $base_url, but relative to the current url
	*
	* It returns string if $assetids is an in and an array(int => string) if $assetids is an array
	*
	* @param int|array	$assetids			the id of the asset you want the href for or an array
	*										for a whole bunch of them
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks
	*
	* @return mixed int|array
	* @access public
	*/
	function getAssetHref($assetids, $base_url=NULL, $ignore_rollback=FALSE)
	{
		$return_array = TRUE;
		$in_assetid = 0;
		if (!is_array($assetids)) {
			if (empty($assetids)) return '';
			$in_assetid = $assetids;
			$assetids = Array($assetids);
			$return_array = FALSE;
		} else {
			if (empty($assetids)) return Array();
		}

		$return_hrefs = Array();

		if (!$ignore_rollback && SQ_ROLLBACK_VIEW) {
			if ($return_array) {
				foreach ($assetids as $assetid) {
					$asset = $this->getAsset($assetid);
					$return_hrefs[$assetid] = $asset->getHref($base_url, TRUE);
				}
				return $return_hrefs;
			} else {
				$asset = $this->getAsset($assetids[0]);
				return $asset->getHref($base_url, TRUE);
			}
		}

		if (is_null($base_url)) $base_url = current_url(FALSE);

		if (is_null($base_url)) $base_url = current_url(FALSE);
		$base_url = strip_url($base_url, TRUE);

		// work out if any of these are cached
		foreach ($assetids as $assetid) {
			if (!isset($this->_tmp['hrefs'][$assetid])) {
				$this->_tmp['hrefs'][$assetid] = Array();
			}

			if (isset($this->_tmp['hrefs'][$assetid][$base_url])) {
				// use the cached version
				$return_hrefs[$assetid] = $this->_tmp['hrefs'][$assetid][$base_url];
			}
		}

		$the_rest = array_diff($assetids, array_keys($return_hrefs));
		$urls = $this->getAssetURL($the_rest, $base_url, $ignore_rollback);

		foreach ($urls as $assetid => $url) {
			if ($url != '') {
				require_once SQ_FUDGE_PATH.'/general/www.inc';
				// deliberatley don't strip url here because if there is any trailing slashes then
				// relative_href will work to accomodate them
				$this->_tmp['hrefs'][$assetid][$base_url] = relative_href(current_url(), $url);
			} else {
				$this->_tmp['hrefs'][$assetid][$base_url] = '';
			}
			$return_hrefs[$assetid] = $this->_tmp['hrefs'][$assetid][$base_url];
		}

		if ($return_array) {
			return $return_hrefs;
		} else {
			return $return_hrefs[$in_assetid];
		}

	}//end getAssetHref()


	/**
	* Clear the asset manager's internal cache of URLs and HREFs for the specified asset
	*
	* @param int	$assetid	The assetid whose cached lookup values we want cleared
	*
	* @return void
	* @access public
	*/
	function clearLookupsCache($assetid)
	{
		if (isset($this->_tmp['urls'])) {
			unset($this->_tmp['urls'][$assetid]);
		}
		if (isset($this->_tmp['hrefs'])) {
			unset($this->_tmp['hrefs'][$assetid]);
		}

	}//end clearLookupsCache()


	/**
	* Returns the href(s) of an asset based on its assetid and the wanted screen
	*
	* It reutrns an array of assetid and hrefs to the wanted screen
	*
	* @param array		$asset_info		an array of asset info in the form assetid => screen code name
	* @param boolean	$backend_page	whether to include the 'SQ_BACKEND_PAGE=main' query string
	*									include this bit when we are switching screens in the backend
	*									do not include when generating URL (e.g. for workflow emails)
	*
	* @return array
	* @access public
	*/
	function getAssetBackendHref($asset_info, $backend_page=TRUE)
	{
		if (empty($asset_info)) return Array();

		$current_url_complete = substr(current_url(), -1) == '/';
		$return_hrefs = Array();
		foreach ($asset_info as $assetid => $screen) {
			$href = '';
			//Patch for #4372 Feature Request : JS API ability to specify _admin/_edit access
			// if our $_REQUEST var is set then we are faking that we are in simple edit...so generate links accordingly
			if (SQ_IN_LIMBO || (isset($_REQUEST['force_simple_edit']) && $_REQUEST['force_simple_edit']) ) {
				$href = sq_web_path('edit').'/?asset_ei_screen='.$screen.'&limbo_assetid='.$assetid;
			} else {
				$frame = ($backend_page) ? 'SQ_BACKEND_PAGE=main&' : '';
				$href = sq_web_path('admin').'/?'.$frame.'backend_section=am&am_section=edit_asset&assetid='.$assetid.'&asset_ei_screen='.$screen;
			}
			if (!$current_url_complete && strpos($href, './') === 0) {
				$href = substr($href, 2);
			}
			$return_hrefs[$assetid] = $href;
		}
		return $return_hrefs;

	}//end getAssetBackendHref()


	/**
	* Get a URL to access the icon for a type of asset
	*
	* @param string	$asset_type	the type code to get the icon for
	*
	* @return string
	* @access public
	*/
	function getAssetIconURL($asset_type)
	{
		return sq_web_path('data').'/asset_types/'.$asset_type.'/icon.png';

	}//end getAssetIconURL()


	/**
	* Returns an array of the passed web paths that are currently in use by any the children of the major asset
	*
	* Ideally we could do this in only a couple of queries:
	* - check which paths already exist
	* - get the last path variation for each one that does exist
	* - increment variations appropriately.
	* Unfortunately due to db sorting (sometimes '#99' is being returned when '#120 is the real last variation)
	* this isn't possible.
	* Instead, it checks only problem paths in chunks of 1000 to see which exist, remembers the last id from each
	* chunk (worked out in php, it's more consistent), and then modifies path variations as required.
	*
	* @param Asset		$major					the major asset, whose children we are to check
	* @param array		$paths					array of web paths to check
	* @param int		$ignore_assetid			ignore this assetid when checking
	* @param boolean	$return_alternatives	return all paths but replace the bad paths
	*											with a valid alternative
	*
	* @return array
	* @access protected
	*/
	function webPathsInUse(Asset $major, $paths, $ignore_assetid=0, $return_alternatives=FALSE)
	{
		if (empty($paths)) return Array();
		$return_paths = Array();

		$db     = MatrixDAL::getDb();
		$dbType = MatrixDAL::getDbType();

		$sql = 'SELECT p.path
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_path p
				INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
				ON p.assetid = l.minorid ';
		$baseWhere = 'WHERE l.majorid = :majorid ';
		$baseBindVars['majorid'] = (string) $major->id;

		if ($ignore_assetid) {
			$baseWhere .= 'AND l.minorid <> :ignore_assetid ';
			$baseBindVars['ignore_assetid'] = (string) $ignore_assetid;
		}

		$baseWhere .= 'AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', SQ_SC_LINK_WEB_PATHS).' > 0 ';

		$bindVars = $baseBindVars;
		$paths_string_array = array();
		foreach (array_chunk($paths, 499) as $chunkid => $chunk) {
			$paths_string = '(p.path IN (';
			foreach ($chunk as $pathid => $path) {
				$bindVars['path'.$chunkid.$pathid] = $path;
				$paths_string .= ':path'.$chunkid.$pathid.',';
			}
			$paths_string = rtrim($paths_string, ',');
			$paths_string .= '))';
			$paths_string_array[] = $paths_string;
		}
		$where = $baseWhere . 'AND '.implode(' OR ', $paths_string_array);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$fullSql = $sql.$where;

		try {
			$paths_result = MatrixDAL::preparePdoQuery($fullSql);
			foreach ($bindVars as $bindVar => $bindValue) {
				MatrixDAL::bindValueToPdo($paths_result, $bindVar, $bindValue);
			}
			$foundPaths = MatrixDAL::executePdoAssoc($paths_result, 0);
		} catch (Exception $e) {
			throw new Exception("Unable to get web paths in use due to the following database error:".$e->getMessage());
		}//end try catch

		if ($return_alternatives === FALSE) {
			return $foundPaths;
		}

		/**
		 * fixPaths will be a list of paths that are already taken and need to be adjusted.
		 */
		$fixPaths = array_intersect($paths, $foundPaths);

		/**
		 * There are none? Yay!
		 */
		if (empty($fixPaths) === TRUE) {
			return $paths;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		/**
		 * We only need to do this for the paths we need to fix.
		 * We'll go through them one by one and work out the closest match.
		 * We do that by stripping numbers off the end of the path and seeing
		 * if there are any matches.
		 */
		foreach ($fixPaths as $fixid => $pathToFix) {
			// remove the trailing digits of path, because we will work out new digits
			$pathToFix = preg_replace('/(.*?)\d+$/', '$1', $pathToFix);
			$pathToFix = preg_replace('/(.*?)\d+(\.[0-9a-zA-Z_]+)$/', '$1$2', $pathToFix);

			/**
			 * endPaths keeps a lits of the last path used in each chunk received from the database.
			 * See function comments about #99 vs #120 for why
			 */
			$endPaths = array();

			// Used for chunking when looking at existing paths.
			$offset = 0;
			$limit  = 1000;
			do {
				$bindVars = $baseBindVars;
				$where    = $baseWhere;
				if (empty($pathToFix) === FALSE) {
					$where .= ' AND p.path LIKE :pathToFix';

					// if it's a file name, digits is added to file name part before extension
					$extension = strrchr($pathToFix,'.');
					if($extension === FALSE) {
						$bindVars['pathToFix'] = $pathToFix.'%';
					}
					else {
						$pathToFix = substr($pathToFix, 0, strrpos($pathToFix, $extension));
						$bindVars['pathToFix'] = $pathToFix.'%'.$extension;
					}
				}

				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

				$fullSql = $sql.$where;

				/**
				 * Since we're fetching in chunks we need to make sure we order the results.
				 * So we don't double up and we don't miss any.
				 *
				 * (path, assetid) is the primary key - so use that for ordering.
				 */
				$fullSql .= ' ORDER BY path, assetid DESC';
				$fullSql = db_extras_modify_limit_clause($fullSql, $dbType, $limit, $offset);

				try {
					$paths_result = MatrixDAL::preparePdoQuery($fullSql);
					foreach ($bindVars as $bindVar => $bindValue) {
						MatrixDAL::bindValueToPdo($paths_result, $bindVar, $bindValue);
					}
					$foundPaths = MatrixDAL::executePdoAssoc($paths_result, 0);
				} catch (Exception $e) {
					throw new Exception("Unable to get web paths in use due to the following database error:".$e->getMessage());
				}//end try catch

				/**
				 * If we're only looking at numeric paths, get rid of non-numeric paths
				 * we can't use 'max' here to do it because '1' and 'page-1' will get returned
				 * depending on which path was found first (see php.net/max for details/examples)
				 * and also if a path starts with a number it will be included (which is not
				 * what we want at all).
				 */
				if (empty($pathToFix) === TRUE) {
					foreach ($foundPaths as $_idx => $_foundPath) {
						if (is_numeric($_foundPath) === FALSE) {
							unset($foundPaths[$_idx]);
						}
					}
				}

				/**
				 * Keep a list of the last path in this chunk.
				 * We'll use this later on in case we need to increment
				 * the path id's.
				 */
				natcasesort($foundPaths);
				$endPaths[] = end($foundPaths);

				$offset += $limit;

				// If we only get 100 results, don't bother going back for more.
				// We know there are no more!
				if (sizeof($foundPaths) < $limit) {
					break;
				}
			} while (empty($foundPaths) === FALSE);
			/*
			 * The above should never be needed since we're checking the size
			 * of the returned paths against the limit, but it can't hurt to have
			 * this check anyway.
			 */

			// Work out the real end path based on all the end paths of the chunks.
			natcasesort($endPaths);
			$last_path = end($endPaths);

			/**
			 * increment each modified path based on the last_path above.
			 * no need to check if the id is taken again, we've done that.
			 */
			$paths[$fixid] = increment_filename($last_path);
		}
		return $paths;
	}//end webPathsInUse()


	//--        PAINTING        --//


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @param object	&$backend	backend
	*
	* @return void
	* @access public
	*/
	function paintBackend(&$backend)
	{
		require_once SQ_INCLUDE_PATH.'/asset_manager_edit_fns.inc';
		$edit_fns = new Asset_Manager_Edit_Fns($this, $backend);
		$edit_fns->paintBackend();

	}//end paintBackend()


//--        QUERY GENERATORS        --//


	/**
	* Return all links between the specified asset and other assets
	*
	* The return value is array consisting of a select statement, a from statement, a where clause
	* (which may be blank is we are getting children of the root node) and an order by clause
	* (which may be blank if we are not sorting). The return array is structed so you can implode
	* it using ' ' as glue to form the full query.
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link the specified asset
	*											is on ('major' or 'minor')
	* @param array			$value				The value for all the links must equal this (if not null)
	* @param boolean		$dependant			The dependant status for all the links must be
	*											this (if not null)
	* @param boolean		$exclusive			The exclusive status for all the links must be
	*											this (if not null)
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$effective			If effective permissions should be considered or not.
	* @param string			$bind_prefix		The prefix for bind variables generated by this function
	*											(default: 'gc_'). Set this parameter if you are planning to use more than
	*											one call to this function in a query (through a UNION, say).
	*
	* @return array
	* @access public
	*/
	function generateGetLinksQuery($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $value=NULL, $dependant=NULL, $exclusive=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE, $bind_prefix='gl_')
	{

		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');
		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$bind_vars = Array();

		$assetid_cond = '';
		// TODO: queries.xml format will just send arrays unedited
		if (is_array($assetid)) {
			$assetid_binds = Array();
			for (reset($assetid); NULL !== ($i = key($assetid)); next($assetid)) {
				assert_valid_assetid($assetid[$i]);
				$assetid_binds[] = ':'.$bind_prefix.'assetids_'.$i;
				$bind_vars[$bind_prefix.'assetids_'.$i] = $assetid[$i];
			}
			$assetid_cond = 'IN ('.implode(', ', $assetid_binds).')';
		} else {
			assert_valid_assetid($assetid);
			$assetid_cond = '= :'.$bind_prefix.'assetid';
			$bind_vars[$bind_prefix.'assetid'] = $assetid;
		}

		$select = 'SELECT  l.'.$side_of_link.'id, l.'.$other_side.'id, l.linkid, l.value, l.link_type,
						l.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive, l.locked';
		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';

		$where  = 'l.'.$side_of_link.'id '.$assetid_cond.'
			AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', ':'.$bind_prefix.'link_types').' > 0 ';
		$bind_vars[$bind_prefix.'link_types'] = $link_types;

		// If $link_value_wanted is  '', we should get all records including '' and NULL,
		// because Oracle will think '' same as NULL.
		$is_oci = (MatrixDAL::getDbType() === 'oci') ? TRUE : FALSE;
		if (!is_null($value) && is_array($value)) {
			$link_value = Array();
			$has_empty = FALSE;
			for (reset($value['link_value']); NULL !== ($i = key($value['link_value'])); next($value['link_value'])) {
				if ($value['link_value'][$i] === '') {
					$has_empty = TRUE;
					if ($is_oci === TRUE) {
						unset($value['link_value'][$i]);
						continue;
					}
				}
				$link_value[] = ':'.$bind_prefix.'value_'.$i;
				$bind_vars[$bind_prefix.'value_'.$i] = $value['link_value'][$i];
			}

			$equal_prefix = $value['equal'] ? '': 'NOT ';
			if (count($link_value) === 0) {
				$in_clause = '';
			} else {
				$in_clause = '(l.value '.$equal_prefix.'IN ('.implode(', ', $link_value).'))';
			}
			$null_clause = '';
			if ($is_oci === TRUE) {
				if ((!$value['equal']) ^ $has_empty) {
					$null_clause = ' l.value IS NULL';
					if (!($in_clause === '')) $null_clause = ' OR '.$null_clause;
				} else {
					if ($in_clause === '') {
						$null_clause = ' l.value IS NOT NULL';
					}
				}
			}
			$where .= ' AND ('.$in_clause.$null_clause.')';
		}
		if (!is_null($dependant)) {
			$where .= ' AND l.is_dependant = :'.$bind_prefix.'is_dependant';
			$bind_vars[$bind_prefix.'is_dependant'] = ($dependant) ? '1' : '0';
		}
		if (!is_null($exclusive)) {
			$where .= ' AND l.is_exclusive = :'.$bind_prefix.'is_exclusive';
			$bind_vars[$bind_prefix.'is_exclusive'] = ($exclusive) ? '1' : '0';
		}

		$group_by = '';
		$having = '';
		if (!is_null($access)) {
			$access = (Int)$access;
			$userid_cond = '';
			$group_by = 'GROUP BY l.linkid, l.'.$side_of_link.'id, l.'.$other_side.'id, l.value, l.link_type,
								  l.type_code, l.sort_order, l.is_dependant, l.is_exclusive, l.locked';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$from .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON p.assetid = l.'.$other_side.'id';

				/**
				 * See bug #6200 for why this is here.
				 * Even though we're not using the result from the lnk_tree table, it makes
				 * oracle behave in a much nicer way when it works out how to execute the query.
				 */
				if(!($link_types & SQ_LINK_NOTICE))
					$from .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON t.linkid=l.linkid';

				// get user and group ids
				$userid1_conds = Array(':'.$bind_prefix.'userids_1_current', ':'.$bind_prefix.'userids_1_public_user');
				$userid2_conds = Array(':'.$bind_prefix.'userids_2_current', ':'.$bind_prefix.'userids_2_public_user');

				$current_user = $this->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
				$userids = $current_user->getUserGroups();
				$this->forgetAsset($current_user);

				$public_userid = (string)$this->getSystemAssetid('public_user');

				$bind_vars[$bind_prefix.'userids_1_current'] = (string)$GLOBALS['SQ_SYSTEM']->user->id;
				$bind_vars[$bind_prefix.'userids_1_public_user']  = $public_userid;
				$bind_vars[$bind_prefix.'userids_2_current'] = (string)$GLOBALS['SQ_SYSTEM']->user->id;
				$bind_vars[$bind_prefix.'userids_2_public_user']  = $public_userid;

				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userid1_conds[] = ':'.$bind_prefix.'userids_1_'.$i;
					$bind_vars[$bind_prefix.'userids_1_'.$i] = $userids[$i];
					$userid2_conds[] = ':'.$bind_prefix.'userids_2_'.$i;
					$bind_vars[$bind_prefix.'userids_2_'.$i] = $userids[$i];
				}

				$userids1_str = implode(',', $userid1_conds);
				$userids2_str = implode(',', $userid2_conds);

				$userid_cond = ' AND (p.userid IN ('.$userids1_str.') OR r.userid IN ('.$userids2_str.'))';
				// join to roles table
				$from .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON (p.userid = r.roleid AND r.assetid = l.minorid)';


				$where .= $userid_cond.'
					AND (
						(p.permission = '.$access.' AND (
									p.userid <> :'.$bind_prefix.'public_userid
									OR r.userid <> :'.$bind_prefix.'public_userid_1
									OR (p.userid = :'.$bind_prefix.'public_userid_2 AND p.granted = \'1\')
									OR (r.userid = :'.$bind_prefix.'public_userid_3 AND p.granted = \'1\')
								)
						)';

				$bind_vars[$bind_prefix.'public_userid']   = $public_userid;
				$bind_vars[$bind_prefix.'public_userid_1'] = $public_userid;
				$bind_vars[$bind_prefix.'public_userid_2'] = $public_userid;
				$bind_vars[$bind_prefix.'public_userid_3'] = $public_userid;

				if ($effective) {
					$where .= ' OR (p.permission > :'.$bind_prefix.'access AND p.granted = \'1\')';
					$bind_vars[$bind_prefix.'access'] = $access;
				}
				$where .= ')';
				$having .= 'HAVING MIN(p.granted) <> \'0\'';
				$group_by .= ', p.assetid';

			}//end if

		}//end if access is not null

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				$type_code_binds = Array();
				for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
					$type_code_binds[] = ':'.$bind_prefix.'type_code_'.$i;
					$bind_vars[$bind_prefix.'type_code_'.$i] = $type_code[$i];
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code_binds).')';
			} else {
				$type_code_cond = '= :'.$bind_prefix.'type_code';
				$bind_vars[$bind_prefix.'type_code'] = $type_code;
			}

			if ($strict_type_code) {
				$where .= ' AND l.type_code '.$type_code_cond;
			} else {
				$where .= ' AND l.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE inhd_type_code '.$type_code_cond.'
												)';
			}
		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		if (is_null($sort_by)) {
			// order by sort order (by default)
			$order_by = 'ORDER BY l.sort_order, l.type_code';
		} else {
			$sort_by = preg_replace('/[^a-zA-Z_0-9]+/', '', $sort_by);
			// order by a specific field in the asset table
			$order_by = 'ORDER BY l.'.$sort_by;
			if ($sort_by != 'type_code') {
				$select .= ', l.'.$sort_by.' AS sort_value';
				if (!empty($group_by)) $group_by .= ', l.'.$sort_by;
			}
		}

		$sql_array = Array(
						'select'	=> $select,
						'from'		=> $from,
						'where'		=> $where,
						'group_by'	=> $group_by,
						'having'	=> $having,
						'order_by'	=> $order_by,
					);

		return Array(
				'sql_array'	=> $sql_array,
				'bind_vars'	=> $bind_vars,
			   );

	}//end generateGetLinksQuery()


	/**
	* Generate the parts of the query needed to get children of the passed asset
	*
	* The return value is array consisting of a select statement, a from statement, a where clause
	* (which may be blank is we are getting children of the root node) and an order by clause
	* (which may be blank if we are not sorting). The return array is structed so you can implode
	* it using ' ' as glue to form the full query.
	*
	* @param object			&$asset					the asset to get its children for
	* @param string|array	$type_code				the type of asset that is linked
	*												(eg 'Page', 'File', etc)
	*												if an array returns link if matches any
	*												of the array values
	* @param boolean		$strict_type_code		whether we are finding assets that are just a $type_code
	*												or $type_code and any of it's sub-classes
	* @param boolean		$dependant				if TRUE, results will be filtered to assets that are dependants.
	*												If FALSE, results will all not be dependants.
	*												If NULL, results will not be filtered
	* @param string			$sort_by				a field in the asset table you would like the results
	*												sorted by (eg. name, short_name etc)
	* @param int			$access					SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*												Specify that links should be returned for assets the user has read,
	*												write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$effective				If effective permissions should be considered or not.
	* @param boolean		$inc_type_code			whether to include the type code in the SELECT or not
	* @param int			$min_depth				Optional parameter that specifies minimum number of levels below
	*												the specified asset for a child to qualify (immediate child is 1)
	* @param int			$max_depth				Optional parameter that specifies maximum number of levels below
	*												the specified asset for a child to qualify to be returned
	* @param boolean		$direct_shadows_only	If TRUE, ONLY direct shadow asset children will be returned
	*												If FALSE, ALL the shadow asset children will be returned
	* @param string			$link_value_wanted		Link Value Wanted
	* @param string			$bind_prefix			The prefix for bind variables generated by this function
	*												(default: 'gc_'). Set this parameter if you are planning to use more than
	*												one call to this function in a query (through a UNION, say).
	* @param array			$link_types_wanted		link_type we want our results to be filtered by. By default it uses
	*												empty array which mean it will return all the link_types
	*												Other values that can be used in the array : SQ_LINK_TYPE_1, SQ_LINK_TYPE_2,
	*												SQ_LINK_TYPE_3, SQ_LINK_NOTICE, SQ_SC_LINK_FRONTEND_NAV,
	*												SQ_SC_LINK_BACKEND_NAV, SQ_SC_LINK_SIGNIFICANT, SQ_SC_LINK_WEB_PATHS
	*
	*
	* @return array
	* @access public
	* @see getChildren()
	*/
	function generateGetChildrenQuery(&$asset, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE, $inc_type_code=TRUE, $min_depth=NULL, $max_depth=NULL, $direct_shadows_only=TRUE, $link_value_wanted=NULL, $bind_prefix='gc_', Array $link_types_wanted=Array())
	{
		assert_valid_assetid($asset->id);
		$bind_vars = Array();

		// OK, normal asset, let's find a reference to them in the tree and go from there
		$where = 'l.minorid = :minorid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT t.treeid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
				'.$where;
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'minorid', $asset->id);
			$treeid = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get treeid for asset: '.$asset->id.' due to database error: '.$e->getMessage());
		}

		if (empty($treeid)) {
			trigger_localised_error('SYS0345', E_USER_WARNING, $asset->id);
			return Array();
		}

		$select = 'SELECT DISTINCT l.minorid'.(($inc_type_code) ? ', a.type_code' : '');
		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid
				';

		/**
		 * We do need the ast table if we're in rollback mode though
		 * so shadow assets can check dates/times
		 */
		if (SQ_ROLLBACK_VIEW) {
			$union_from_list = $from;
		}

		// Keep WHERE bind vars in a separate array (then merge in) - as this
		// segment could well be duplicated by the "direct shadows only" part
		// of the query.
		$where_bind_vars = Array();

		if ($asset->type() == 'root_folder') {
			// Since Root Folder is the parent of all the assets in the System,
			// dont filter the link tree table
			$where = '1=1';
			// Treeid for the Root Folder is '-', which is 1 char long, instead of expected 0 length
			// The min/max tree depth paramrter in the query will not work expectedly if the treeid
			// length is not the multiple of SQ_CONF_ASSET_TREE_SIZE
			$treeid_len = 0;
		} else {
			$where = 't.treeid LIKE '.MatrixDAL::Quote($treeid.'%').'
						AND t.treeid > '.MatrixDAL::Quote($treeid);
			$treeid_len = strlen($treeid);
		}

		if (!is_null($min_depth) && !is_null($max_depth)) {
			if ($min_depth < $max_depth) {
				$where .= ' AND LENGTH(t.treeid) BETWEEN :'.$bind_prefix.'treeid_min_length AND :'.$bind_prefix.'treeid_max_length';
				$where_bind_vars[$bind_prefix.'treeid_min_length'] = ($treeid_len + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
				$where_bind_vars[$bind_prefix.'treeid_max_length'] = ($treeid_len + $max_depth * SQ_CONF_ASSET_TREE_SIZE);
			} else if ($min_depth > $max_depth) {
				$where .= ' AND LENGTH(t.treeid) BETWEEN :'.$bind_prefix.'treeid_max_length AND :'.$bind_prefix.'treeid_min_length';
				$where_bind_vars[$bind_prefix.'treeid_min_length'] = ($treeid_len + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
				$where_bind_vars[$bind_prefix.'treeid_max_length'] = ($treeid_len + $max_depth * SQ_CONF_ASSET_TREE_SIZE);
			} else {
				$where .= ' AND LENGTH(t.treeid) = :'.$bind_prefix.'treeid_length';
				$where_bind_vars[$bind_prefix.'treeid_length'] = ($treeid_len + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
			}
		} else if (!is_null($min_depth)) {
			$where .= ' AND LENGTH(t.treeid) >= :'.$bind_prefix.'treeid_length';
			$where_bind_vars[$bind_prefix.'treeid_length'] = ($treeid_len + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
		} else if (!is_null($max_depth)) {
			$where .= ' AND LENGTH(t.treeid) <= :'.$bind_prefix.'treeid_length';
			$where_bind_vars[$bind_prefix.'treeid_length'] = ($treeid_len + $max_depth * SQ_CONF_ASSET_TREE_SIZE);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		// adding a new filter 'link value'
		// If $link_value_wanted is  '', we should get all records including '' and NULL,
		// because Oracle will think '' same as NULL.
		$is_oci = (MatrixDAL::getDbType() === 'oci') ? TRUE : FALSE;
		if (!is_null($link_value_wanted) && is_array($link_value_wanted)) {
			$link_value = Array();
			$has_empty = FALSE;
			for (reset($link_value_wanted['link_value']); NULL !== ($i = key($link_value_wanted['link_value'])); next($link_value_wanted['link_value'])) {
				if ($link_value_wanted['link_value'][$i] === '') {
					$has_empty = TRUE;
					if ($is_oci === TRUE) {
						unset($link_value_wanted['link_value'][$i]);
						continue;
					}
				}
				$link_value[] = ':'.$bind_prefix.'link_value_wanted_'.$i;
				$where_bind_vars[$bind_prefix.'link_value_wanted_'.$i] = $link_value_wanted['link_value'][$i];
			}

			$equal_prefix = $link_value_wanted['equal'] ? '': 'NOT ';
			if (count($link_value) === 0) {
				$in_clause = '';
			} else {
				$in_clause = '(l.value '.$equal_prefix.'IN ('.implode(', ', $link_value).'))';
			}
			$null_clause = '';
			if ($is_oci === TRUE) {
				if ((!$link_value_wanted['equal']) ^ $has_empty) {
					$null_clause = ' l.value IS NULL';
					if (!($in_clause === '')) $null_clause = ' OR '.$null_clause;
				} else {
					if ($in_clause === '') {
						$null_clause = ' l.value IS NOT NULL';
					}
				}
			}
			$where .= ' AND ('.$in_clause.$null_clause.')';
		}

		if (!empty($link_types_wanted)) {
			foreach ($link_types_wanted as $index => $link_type_wanted) {
				if(!is_numeric($link_type_wanted)) $link_type_wanted = SQ_LINK_TYPE_1;
				$link_types_wanted[$index] = db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', $link_type_wanted).' > 0';
			}
			$where .= ' AND (('.implode(') OR (', $link_types_wanted).'))';
		}

		// now merge the bind vars into the master array
		$bind_vars = array_merge($bind_vars, $where_bind_vars);

		// if direct shadow asset children should be returned
		$union_select = ' UNION ALL SELECT sl.minorid';
		if ($inc_type_code) {
			$union_select .= ', null AS type_code';
		}
		if (!is_null($sort_by)) {
			$union_select .= ', null AS sort_value';
		}
		$union_from = ' FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk sl';
		// if all shadow asset children should be returned
		$union_where = 'sl.majorid = :'.$bind_prefix.'union_majorid';
		$union_where .= ' AND sl.link_type != '.SQ_LINK_NOTICE;
		$bind_vars[$bind_prefix.'union_majorid'] = $asset->id;
		if (!$direct_shadows_only) {
			// need to dupe where bind vars here, because we're trying to include
			// the "where" section of the query again.
			foreach ($where_bind_vars as $bind_var => $bind_value) {
				$new_bind_var = str_replace($bind_prefix, $bind_prefix.'ds_', $bind_var);
				$bind_vars[$new_bind_var] = $bind_value;
			}

			$ds_where = str_replace(':'.$bind_prefix, ':'.$bind_prefix.'ds_', $where);

			# See bug 4914 for why this is a left join and not a subselect.
			# We also need to do the link_type check when doing the shadow_ids part.
			$union_where = "((".$union_where.") OR (shadow_ids.minorid IS NOT NULL AND sl.link_type != ".SQ_LINK_NOTICE."))";
			$union_from .= " LEFT JOIN (SELECT DISTINCT l.minorid $from $ds_where) shadow_ids ON (sl.majorid=shadow_ids.minorid)";
		}

		$union_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($union_where, 'sl');

		if (!empty($where) && (!empty($type_code) || !is_null($access) || !is_null($dependant))) {
			$where .= ' AND ';
		}

		if (!is_null($dependant)) {
			$where .= ' l.is_dependant = :'.$bind_prefix.'dependant';
			$bind_vars[$bind_prefix.'dependant'] = $dependant ? '1' : '0';
			if (!empty($type_code) || !is_null($access)) {
				$where .= ' AND ';
			}
		}

		$group_by = '';
		$having = '';
		if (!is_null($access)) {
			$access = (String)$access;
			$userid_cond = '';
			$group_by = 'GROUP BY l.minorid, a.type_code, a.name';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$from .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON p.assetid = a.assetid ';
				$from .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON (p.userid = r.roleid AND r.assetid = a.assetid) ';

				$current_user = $this->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
				$userids = $current_user->getUserGroups();
				$this->forgetAsset($current_user);

				$userids[] = (String)$GLOBALS['SQ_SYSTEM']->user->id;
				$public_userid = (String)$this->getSystemAssetid('public_user');
				$userids[] = $public_userid;

				$p_userids_str = Array();
				$r_userids_str = Array();
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$p_userids_str[$i] = ':'.$bind_prefix.'p_userids_'.$i;
					$r_userids_str[$i] = ':'.$bind_prefix.'r_userids_'.$i;
					$bind_vars[$bind_prefix.'p_userids_'.$i] = (string)$userids[$i];
					$bind_vars[$bind_prefix.'r_userids_'.$i] = (string)$userids[$i];
				}

				$p_userids_str = implode(',', $p_userids_str);
				$r_userids_str = implode(',', $r_userids_str);
				$userid_cond = ' (p.userid IN ('.$p_userids_str.') OR r.userid IN ('.$r_userids_str.'))';
				$where .= $userid_cond.'
					AND (
							(p.permission = :'.$bind_prefix.'_access AND (
									p.userid <> :'.$bind_prefix.'_public_userid
									OR r.userid <> :'.$bind_prefix.'_public_userid_1
									OR (p.userid = :'.$bind_prefix.'_public_userid_2 AND p.granted = \'1\')
									OR (r.userid = :'.$bind_prefix.'_public_userid_3 AND p.granted = \'1\')
								)
							)';

				$bind_vars[$bind_prefix.'_access'] = $access;
				$bind_vars[$bind_prefix.'_public_userid'] = $public_userid;
				$bind_vars[$bind_prefix.'_public_userid_1'] = $public_userid;
				$bind_vars[$bind_prefix.'_public_userid_2'] = $public_userid;
				$bind_vars[$bind_prefix.'_public_userid_3'] = $public_userid;

				if ($effective) {
					$where .= ' OR (p.permission > :'.$bind_prefix.'_access_effective AND p.granted = \'1\')';
					$bind_vars[$bind_prefix.'_access_effective'] = $access;
				}
				$where .= ') AND ';
				$having .= 'HAVING MIN(p.granted) <> \'0\'';
				$group_by .= ', p.assetid';
			}
		}//end if access is not null

		if (!empty($type_code)) {

			if (empty($where)) $where = ' WHERE ';

			if (is_array($type_code)) {
				$type_code_cond = Array();
				for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
					$type_code_cond[$i] = ':'.$bind_prefix.'_type_code_'.$i;
					$bind_vars[$bind_prefix.'_type_code_'.$i] = $type_code[$i];
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code_cond).')';
			} else {
				$type_code_cond = '= :'.$bind_prefix.'_type_code';
				$bind_vars[$bind_prefix.'_type_code'] = $type_code;
			}

			if ($strict_type_code) {
				$where .= 'a.type_code '.$type_code_cond;
			} else {
				$where .= 'a.type_code IN (
											SELECT type_code
											FROM sq_ast_typ_inhd
											WHERE inhd_type_code '.$type_code_cond.'
											)';
			}
		}//end if

		// order by a field if required
		$order_by = '';

		if (!is_null($sort_by)) {
			$sort_by = preg_replace('/[^a-zA-Z_0-9]+/', '', $sort_by);
			$order_by = 'ORDER BY sort_value';
			$select .= ', a.'.$sort_by.' AS sort_value';
			if (!is_null($access)) {
				$group_by .= ', a.'.$sort_by;
			}
		}

		$where = rtrim($where, 'AND ');

		$sql_array = Array(
						'select'		=> '('.$select,
						'from'			=> $from,
						'where'			=> $where,
						'group_by'		=> $group_by,
						'having'		=> $having,
						'union_select'	=> $union_select,
						'union_from'	=> $union_from,
						'union_where'	=> $union_where,
						'order_by'		=> ')'.$order_by,
					 );

		$ret_val = Array(
					'sql_array'	=> $sql_array,
					'bind_vars'	=> $bind_vars,
				);

		return $ret_val;

	}//end generateGetChildrenQuery()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$effective			If effective permissions should be considered or not.
	* @param int			$min_height			Optional parameter that specifies minimum number of levels above
	*											the specified asset for a parent to qualify (immediate parent is 1)
	* @param int			$max_height			Optional parameter that specifies maximum number of levels above
	*											the specified asset for a parent to qualify to be returned
	* @param array			$link_value_wanted	the link value wanted
	* @param string			$bind_prefix		The prefix for bind variables generated by this function
	*											(default: 'gp_'). Set this parameter if you are planning to use more than
	*											one call to this function in a query (through a UNION, say).
	*
	* @return array
	* @access public
	* @see getParents()
	*/
	function generateGetParentsQuery($assetid, $type_code='', $strict_type_code=TRUE, $sort_by=NULL, $access=NULL, $effective=TRUE, $min_height=NULL, $max_height=NULL, $link_value_wanted=NULL, $bind_prefix='gp_')
	{
		assert_valid_assetid($assetid);

		$bind_vars = Array();
		$assetid = (string) $assetid;

		$select = 'SELECT DISTINCT(ll.majorid), ll.type_code, ll.treeid, length(ll.treeid)/:'.$bind_prefix.'tree_size as depth';
		$bind_vars[$bind_prefix.'tree_size'] = SQ_CONF_ASSET_TREE_SIZE;

		$from = 'FROM
					(
					SELECT a.assetid, l.majorid, t.treeid, a.type_code, a.name, a.short_name, a.version, a.status, a.languages, a.charset, a.force_secure, a.created, a.created_userid, a.updated, a.updated_userid, a.published, a.published_userid, a.status_changed, a.status_changed_userid ';

		$perm_join = '';
		if (!is_null($access) && !$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$from .= ', p.userid, p.permission, p.granted ';
			$perm_join = ' INNER JOIN sq_ast_perm p ON p.assetid = l.majorid';
		}

		$from  .= 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';

		$from .= $perm_join;

		if (!is_null($min_height) || !is_null($max_height)) {
			$treeids = $this->getAssetTreeids($assetid);

			$where_comps = Array();

			/**
			 * keep an array of id's we've already added to where clauses.
			 * this is used so we don't duplicate looking for the same treeid's in the query
			 * (though the db should be smart enough to do that itself)
			*/
			$_treeids_processed = array();

			$i = 0;
			foreach ($treeids as $treeid) {
				$i++;

				//if min_height is null, that means we specified a max_height
				if (is_null($min_height)) {
					if ($max_height < 1) {
						// where clause that will return nothing
						$where_comps[] = '1=0';
						continue;
					} else {
						$prefixes = Array();
						$length = strlen($treeid);
						for ($prefix = 0; $prefix < $max_height; $prefix++){
							$next_treeid = substr($treeid, 0, $length);
							if (empty($next_treeid)) break;
							$prefixes[] = MatrixDAL::quote($next_treeid);
							$length -= SQ_CONF_ASSET_TREE_SIZE;
						}
						$where_comps[] = 't.treeid IN ('.implode(',', $prefixes).')';
						continue;
					}
				}

				/**
				 * if max_height is null, that means we specified a min_height
				 *
				 * this means we can look for specific treeid's rather than doing a like search,
				 * and get rid of the length(treeid) check.
				 *
				 * ($treeid = t.treeid or $treeid2 = t.treeid or $treeid3 = t.treeid)
				 *
				 * it loops over the treeid and takes SQ_CONF_ASSET_TREE_SIZE chars off the end
				 * until it ends up with the root tree node.
				*/
				if (is_null($max_height)) {
					$_tree_length = strlen($treeid);
					$_required_tree_level = $min_height ? $_tree_length - (($min_height -  1) * SQ_CONF_ASSET_TREE_SIZE) : $_tree_length;

					$where_comp = '(';
					$j = 0;
					while ($_tree_length > 0) {
						$j++;

						// Don't include treeids within minimum tree height range
						if ($_tree_length <= $_required_tree_level) {
							$where_comp .= ':' . $bind_prefix . 'treeid_' . $i . '_' . $j . ' = t.treeid OR ';
							$bind_vars[$bind_prefix.'treeid_'.$i . '_' . $j] = $treeid;
						}
						$treeid = substr($treeid, 0, ($_tree_length - SQ_CONF_ASSET_TREE_SIZE));
						$_tree_length = strlen($treeid);

					}
					$where_comp = rtrim($where_comp, ' OR ') . ')';

					$where_comps[] = $where_comp;
					continue;
				}

				/**
				 * in the get children query function this section gets minor
				 * links, but we want the major links in this case, so we have
				 * to go one further link down - hence the '-1' in these queries
				*/

				/**
				 * we've got this far - so we've specified a min & max heights.
				 * if they are the same, we can look for specific treeid's.
				 * we need to take off one SQ_CONF_ASSET_TREE_SIZE section to get the right id to search for
				 *
				 * also only add the treeid if we haven't done it already to save duplicate id's
				 * being passed in.
				 *
				 * so we end up with this (which works much better performance wise):
				 * $treeid = t.treeid
				 */
				if ($min_height == $max_height) {
					$treeid = $max_height ? substr($treeid, 0, strlen($treeid) - ((($max_height - 1) * SQ_CONF_ASSET_TREE_SIZE))) : '';

					/**
					* if we've already processed this treeid, continue
					* no need to put it in the query multiple times.
					*/
					if (in_array($treeid, $_treeids_processed)) {
						continue;
					}

					$where_comp = ':' . $bind_prefix . 'treeid_' . $i . ' = t.treeid';
					$bind_vars[$bind_prefix.'treeid_'.$i] = $treeid;

					$where_comps[] = $where_comp;
					continue;
				}

				/**
				 * Here we have both min_height & max_height
				 * and they are also different values
				 */

				/**
				 * If min_height > max_height, just switch them
				 * We would end up with the same queries anyway.
				*/
				if ($min_height > $max_height) {
					$temp_min_height = $min_height;
					$min_height = $max_height;
					$max_height = $temp_min_height;
					unset($temp_min_height);
				}

				/**
				* Set some temporary variables in case the originals are used elsewhere.
				*/
				$temp_min_height = (strlen($treeid) - ($min_height - 1) * SQ_CONF_ASSET_TREE_SIZE);
				$temp_max_height = (strlen($treeid) - ($max_height - 1) * SQ_CONF_ASSET_TREE_SIZE);
				$treeid_counter = 1;

				if ($temp_max_height <= strlen($treeid)) {
					// Treeids outside the minimum height criteria needn't be looked into, hence strip them off
					$temp_treeid = ($min_height > 1) ? substr($treeid, 0, -1 * ($min_height - 1) * SQ_CONF_ASSET_TREE_SIZE) : $treeid;

					$where_comp = 't.treeid IN (';

					/**
					* Get exact treeid's by taking off SQ_CONF_ASSET_TREE_SIZE chunks from the end
					* until we end up with the length we're looking for (i.e. between temp_min_height and temp_max_height)
					*/
					while (strlen($temp_treeid) && strlen($temp_treeid) <= $temp_min_height && strlen($temp_treeid) >= $temp_max_height) {
						$tree_var_name = $bind_prefix . 'treeid_' . $i . '_' . $treeid_counter;
						$bind_vars[$tree_var_name] = $temp_treeid;
						$where_comp .= ':' . $tree_var_name.',';
						$treeid_counter++;
						$temp_treeid = substr($temp_treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
					}

					$where_comp = rtrim($where_comp, ',');
					$where_comp .= ')';

					$where_comps[] = $where_comp;
				} else {
					// If max treeid length is less the asset's treeid, we're not going get any result
					$where_comps[] ='FALSE';
				}
			}

			$where = '(('.implode(') OR (', $where_comps).'))';

		} else {

			/**
			 * If there are no conditions, then work out which treeid's to look at.
			 *
			 * This is a separate query for performance reasons.
			 * Previously it was an 'IN' clause in the main query, but because it runs through a function
			 * the query optimizers can't cope with it efficiently.
			 * (this affects both oracle and postgres)
			 *
			 * So we do a query here to get the treeid's
			 * which are then put into the main query.
			 *
			 * and since we're now looking at static id's basically,
			 * the optimizers are a lot happier
			 *
			 * This is also done with the assumption that you'll never have a lot of parents
			 * as oracle has a limit on the number of id's you can use in an in clause (999).
			 *
			 * Instead of calling the sq_get_parent_treeids database function/stored procedure,
			 * take the query directly and run it here.
			 * Then we need to chop the treeid's returned from that into SQ_CONF_ASSET_TREE_SIZE chunks
			 * and use those in the where clause.
			 */
			$tree_sql = 'SELECT treeid FROM ' . SQ_TABLE_RUNNING_PREFIX . 'ast_lnk l INNER JOIN ' . SQ_TABLE_RUNNING_PREFIX .
				'ast_lnk_tree t ON l.linkid = t.linkid WHERE l.minorid=:' . $bind_prefix . 'treeids_assetid';

			$tree_bind_vars[$bind_prefix.'treeids_assetid'] = $assetid;
			$query = MatrixDAL::preparePdoQuery($tree_sql);
			foreach ($tree_bind_vars as $tree_bind_var => $tree_bind_value) {
				MatrixDAL::bindValueToPdo($query, $tree_bind_var, $tree_bind_value);
			}
			$tree_result = MatrixDAL::executePdoAll($query);

			/**
			 * If we didn't find any tree id's, make a where clause that will return nothing
			 * which then causes the parent query to return an empty result set.
			 * If we leave the where clause blank, we actually return every treeid
			 * which is not what we want.
			 */
			if (empty($tree_result)) {
				$where = '1=0';
			}

			if (!empty($tree_result)) {
				$trees_done = array();
				foreach ($tree_result as $tree_row) {
					$treeid = $tree_row['treeid'];
					while (strlen($treeid) > 0) {
						/**
						 * See if we've dealt with this id before.
						 * It's not necessarily all the results from $tree_result we need to worry about
						 * Rather the shared 'parent' tree id's.
						 * eg they all have base tree id 0000, we only need to include that once.
						 */
						if (!in_array($treeid, $trees_done, TRUE)) {
							$trees_done[] = $treeid;
						}
						$treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
					}//end while
				}//end foreach

				// Break up the treeids into chunks of 1000 so that Oracle does not complain
				$where_array = Array();
				foreach(array_chunk($trees_done, 999) as $chunk) {
					$where_string = '';
					foreach($chunk as $treeid) {
						$where_string .= MatrixDAL::Quote($treeid) . ',';

					}//end foreach
					$where_string = '(t.treeid IN ('.rtrim($where_string, ',').'))';
					$where_array[] = $where_string;
				}//end foreach
				$where = '('.implode(' OR ', $where_array).')';
			}
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		// If $link_value_wanted is  '', we should get all records including '' and NULL,
		// because Oracle will think '' same as NULL.
		$is_oci = (MatrixDAL::getDbType() === 'oci') ? TRUE : FALSE;
		if (!is_null($link_value_wanted) && is_array($link_value_wanted)) {
			$link_value = Array();
			$has_empty = FALSE;
			for (reset($link_value_wanted['link_value']); NULL !== ($i = key($link_value_wanted['link_value'])); next($link_value_wanted['link_value'])) {
				if ($link_value_wanted['link_value'][$i] === '') {
					$has_empty = TRUE;
					if ($is_oci === TRUE) {
						unset($link_value_wanted['link_value'][$i]);
						continue;
					}
				}
				$link_value[] = ':'.$bind_prefix.'link_value_wanted_'.$i;
				$bind_vars[$bind_prefix.'link_value_wanted_'.$i] = $link_value_wanted['link_value'][$i];
			}

			$equal_prefix = $link_value_wanted['equal'] ? '': 'NOT ';
			if (count($link_value) === 0) {
				$in_clause = '';
			} else {
				$in_clause = '(l.value '.$equal_prefix.'IN ('.implode(', ', $link_value).'))';
			}
			$null_clause = '';
			if ($is_oci === TRUE) {
				if ((!$link_value_wanted['equal']) ^ $has_empty) {
					$null_clause = ' l.value IS NULL';
					if (!($in_clause === '')) $null_clause = ' OR '.$null_clause;
				} else {
					if ($in_clause === '') {
						$null_clause = ' l.value IS NOT NULL';
					}
				}
			}
			$where .= ' AND ('.$in_clause.$null_clause.')';
		}

		if (!empty($type_code)) {
			$type_code_cond = '';

			// we are cloning $type_code and using the clone instead
			// this will prevent the error of double quoting $type_code in the future
			$type_code_clone = $type_code;
			if (is_array($type_code_clone)) {
				for (reset($type_code_clone); NULL !== ($i = key($type_code_clone)); next($type_code_clone)) {
					$bind_vars[$bind_prefix.'type_code_'.$i] = $type_code_clone[$i];
					$type_code_clone[$i] = ':'.$bind_prefix.'type_code_'.$i;
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code_clone).')';
			} else {
				$type_code_cond = '= :'.$bind_prefix.'type_code';
				$bind_vars[$bind_prefix.'type_code'] = $type_code_clone;
			}
			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				$where .= ' AND a.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE inhd_type_code '.$type_code_cond.'
												)';
			}
		}//end if

		// order by a field if required
		$order_by = '';
		if (!is_null($sort_by)) {
			$sort_by = preg_replace('/[^a-zA-Z_0-9]+/', '', $sort_by);
			$order_by = 'ORDER BY sort_value';
			$select .= ', ll.'.$sort_by.' AS sort_value';
		} else {
			$order_by = 'ORDER BY depth DESC';
		}

		if (!is_null($access) && !$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$where .= ' AND p.assetid = a.assetid';
		}

		$from .= $where.') ll ';

		$group_by      = '';
		$having        = '';
		$outer_where   = '';
		$public_userid = '';
		$userids_str   = '';

		if (!is_null($access)) {
			$group_by = 'GROUP BY ll.majorid, ll.treeid, ll.type_code, ll.name';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$having .= 'HAVING MIN(ll.granted) <> \'0\'';

				$from .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON (ll.userid = r.roleid AND r.assetid = ll.assetid)';

				$current_user = $this->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
				$userids      = $current_user->getUserGroups();
				$this->forgetAsset($current_user);

				$userids[]     = $GLOBALS['SQ_SYSTEM']->user->id;
				$public_userid = (String)$this->getSystemAssetid('public_user');
				$userids[]     = $public_userid;

				// Need to regenerate the user conditions because of the need for different bind vars
				$userid_cond_ll = Array();
				$userid_cond_r  = Array();

				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userid_cond_ll[] = ':'.$bind_prefix.'userids_ll_'.$i;
					$userid_cond_r[]  = ':'.$bind_prefix.'userids_r_'.$i;
					$bind_vars[$bind_prefix.'userids_ll_'.$i] = (string)$userids[$i];
					$bind_vars[$bind_prefix.'userids_r_'.$i]  = (string)$userids[$i];
				}

				$userids_str_ll = implode(', ', $userid_cond_ll);
				$userids_str_r  = implode(', ', $userid_cond_r);

				// Main where caluse. This hsould only be printed if we have user conditions i.e not root/sys_admin
				$outer_where = ' WHERE (
					ll.userid IN ('.$userids_str_ll.') OR r.userid IN ('.$userids_str_r.')
				)
				AND (
					(
						ll.permission = :'.$bind_prefix.'outer_access AND
						(
							ll.userid <> :'.$bind_prefix.'public_userid
							OR r.userid <> :'.$bind_prefix.'public_userid_1
							OR (ll.userid = :'.$bind_prefix.'public_userid_2 AND ll.granted = \'1\')
							OR (r.userid = :'.$bind_prefix.'public_userid_3 AND ll.granted = \'1\')
						)
					)';

				$bind_vars[$bind_prefix.'outer_access'] = $access;
				$bind_vars[$bind_prefix.'public_userid'] = $public_userid;
				$bind_vars[$bind_prefix.'public_userid_1'] = $public_userid;
				$bind_vars[$bind_prefix.'public_userid_2'] = $public_userid;
				$bind_vars[$bind_prefix.'public_userid_3'] = $public_userid;

				if ($effective) {
					$outer_where .= ' OR
						(
							ll.permission > :'.$bind_prefix.'outer_access_effective AND ll.granted = \'1\'
						)';
					$bind_vars[$bind_prefix.'outer_access_effective'] = $access;
				}
				$outer_where .= ')';
			}
		}


		$union_select = ' UNION ALL SELECT sl.majorid, null, null as treeid, null as depth';
		if (!is_null($sort_by)) {
			$union_select .= ', null as sort_value';
		}

		$union_from   = ' FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk sl';
		$union_where  = 'sl.minorid = :'.$bind_prefix.'minorid';
		$bind_vars[$bind_prefix.'minorid'] = $assetid;
		$union_where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($union_where, 'sl');

		$sql_array = Array(
						'select'		=> '('.$select,
						'from'			=> $from,
						'where'			=> $outer_where,
						'group_by'		=> $group_by,
						'having'		=> $having,
						'union_select'	=> $union_select,
						'union_from'	=> $union_from,
						'union_where'	=> $union_where,
						'order_by'		=> ')'.$order_by,
					 );

		$ret_val = Array(
					'bind_vars'	=> $bind_vars,
					'sql_array'	=> $sql_array,
				);

		return $ret_val;

	}//end generateGetParentsQuery()


	/**
	* Returns a information string about all the assets in the cache which Ref
	* Count > 0
	*
	* @return string
	* @access public
	*/
	function getForgottenItemsReport()
	{
		return $this->_asset_cache->getForgottenItemsReport();

	}//end getForgottenItemsReport()


	/**
	* isDependant function: check if an asset is linked dependantly somewhere
	*
	* @param string	$assetid	The id of the asset that we are checking for
	*
	* @return boolean
	* @access public
	*/
	function isDependant($assetid)
	{
		assert_valid_assetid($assetid);
		try {
			$bind_vars    = Array('assetid' => (string)$assetid);
			$links_result = MatrixDAL::executeAll('core', 'isAssetDependentlyLinked', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to determine if asset #'.$assetid.' is dependently linked due to database error: '.$e->getMessage());
		}

		// If there is no row, that means it is not a dependant
		if (!empty($links_result)) {
			return TRUE;
		} else {
			return FALSE;
		}

	}//end isDependant()


	/**
	* Returns an array of global snippet keywords that are available to the current user
	*
	* @param boolean	$more_info	returns description if this is true
	*
	* @return array
	* @access public
	*/
	function getSnippetKeywords($more_info=FALSE)
	{
		/**
		 * Check if we've generated this list before.
		 * If we have, return the old list.
		 * include the more_info var in case we get a call
		 * with & without more_info requested
		 */
		$_tmp_key = 'snippetkeywords_' . (int)$more_info;
		if (isset($this->_tmp[$_tmp_key])) {
			return $this->_tmp[$_tmp_key];
		}

		if (!$this->installed('content_type_snippet')) {
			return Array();
		}
		$rootid = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_snippet', 'SQ_SNIPPET_ROOT');
		if (empty($rootid) || !$this->assetExists($rootid)) {
			// snippet root preference not set
			return Array();
		}

		$containers = $this->getChildren($rootid, 'bodycopy_container', FALSE);
		if (empty($containers)) {
			// no container found under the specified snippet root
			return Array();
		}
		$parent = $this->getChildren($rootid, Array('bodycopy','folder'), TRUE);

		// rearrange the snippets according to asset map sort order
		$minorids = array_keys($containers);
		$majorids = array_keys($parent);
		$majorids[] = $rootid;

		foreach ($minorids as $idx => $minorid) {
			$minorids[$idx] = MatrixDAL::quote($minorid);
		}

		foreach ($majorids as $idx => $majorid) {
			$majorids[$idx] = MatrixDAL::quote($majorid);
		}

		$snippet_sql = 'SELECT minorid FROM sq_ast_lnk';

		foreach (array_chunk($minorids, 999) as $chunk) {
			$minor_in_clauses[] = ' minorid IN ('.implode(', ', $chunk).')';
		}
		$snippet_sql .= ' WHERE ('.implode(' OR ', $minor_in_clauses).') AND';

		foreach (array_chunk($majorids, 999) as $chunk) {
			$major_in_clauses[] = ' majorid IN ('.implode(', ', $chunk).')';
		}
		$snippet_sql .= ' ('.implode(' OR ', $major_in_clauses).')';

		$snippet_sql .= ' GROUP BY majorid, sort_order, minorid ORDER BY majorid, sort_order';

		try {
			$query = MatrixDAL::preparePdoQuery($snippet_sql);
			$db_results = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get snippet sort order due to database error: '.$e->getMessage());
		}
		foreach ($db_results as $db_result) {
			$results[] = $db_result['minorid'];
		}

		// generate list of snippet name and content
		$keywords = Array();
		$permission_check = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_snippet', 'SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK');

		foreach ($results as $id) {
			// Get every asset and do permission check (slow)
			if($permission_check) {
				$container = $this->getAsset($id);
				if (is_null($container)) continue;
				// only list what the current user has access to
				if ($container->readAccess()) {
					$safe_name = make_valid_web_paths(Array($container->name));
					if ($more_info) {
						$keywords[$id] = Array(
											'name'		=> $container->name,
											'safe_name'	=> $safe_name[0],
											'attr'		=> $container->attr('attributes'),
										 );
					} else {
						$keywords['globals_snippet_'.$id.'_'.$safe_name[0]] = "Snippet $id: $container->name";
					}
				}
				$this->forgetAsset($container);
			}
			else {
				$sql = 'select custom_val as name from sq_ast_attr_val a_value, sq_ast_attr a where a_value.attrid = a.attrid and a.name = :attr_name and a_value.assetid = :asset_id';
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'attr_name', 'name');
					MatrixDAL::bindValueToPdo($query, 'asset_id', $id);
					$result = MatrixDAL::executePdoAssoc($query);
					$container_name = isset($result[0]['name']) ? $result[0]['name'] : '';
				} catch (Exception $e) {
					throw new Exception($e->getMessage());
				}

				$safe_name = make_valid_web_paths(Array($container_name));
				if ($more_info) {
					$sql = 'select custom_val as attributes from sq_ast_attr_val a_value, sq_ast_attr a where a_value.attrid = a.attrid and a.name = :attr_name and a_value.assetid = :asset_id';
					$result = NULL;
					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'attr_name', 'attributes');
						MatrixDAL::bindValueToPdo($query, 'asset_id', $id);
						$result = MatrixDAL::executePdoAssoc($query);
						$container_attr = isset($result[0]['attributes']) ? $result[0]['attributes'] : '';
					} catch (Exception $e) {
						throw new Exception($e->getMessage());
					}
					$keywords[$id] = Array(
										'name'		=> $container_name,
										'safe_name'	=> $safe_name[0],
										'attr'		=> $container_attr,
									 );
				} else {
					$keywords['globals_snippet_'.$id.'_'.$safe_name[0]] = "Snippet $id: $container_name";
				}
			}
		}//end foreach container

		$this->_tmp[$_tmp_key] = $keywords;

		return $keywords;

	}//end getSnippetKeywords()


	/**
	* Generate the raw content to be sent, ignoring user switching and recpient keyword replacement
	*
	* @param array	$details	The details of the job
	*
	* @return string
	* @access public
	*/
	public function generateRawContent($details)
	{
		$page = $this->getAsset($details['content_id']);
		if (empty($page)) return '';

		$design_id = array_get_index($details, 'content_design');
		$layout_id = array_get_index($details, 'content_layout');

		ob_start();
			// generate content based on design and paint layout supplied
			if ($design_id && $layout_id) {
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_PAINT_LAYOUT_ID', $layout_id);
				$design = $this->getAsset($design_id);
				$design->paint($page);
			} else if ($design_id) {
				$design = $this->getAsset($design_id);
				$design->paint($page);
			} else if ($layout_id) {
				$layout = $this->getAsset($layout_id);
				$layout->paint($page);
			} else {
				$page->printFrontend();
			}
			$content = ob_get_contents();
		ob_end_clean();
		return $content;

	}//end generateRawContent()


	/**
	* Deletes an asset for good - bypassing the trash.
	* PLEASE NOTE: This is a permanent and irreversible action, so please use with extreme care.
	*
	* @param int	$asset_id	the asset ID of the asset to delete
	*
	* @return void
	* @access public
	*/
	public function purgeAsset($asset_id)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Delete the attribute values
		$sql = 'DELETE FROM sq_ast_attr_val WHERE assetid = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
		MatrixDAL::execPdoQuery($query);

		// Delete the unique attribute values for this asset
		$sql = 'DELETE FROM sq_ast_attr_uniq_val WHERE assetid = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
		MatrixDAL::execPdoQuery($query);

		// Delete the asset itself
		$sql = 'DELETE FROM sq_ast WHERE assetid = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
		MatrixDAL::execPdoQuery($query);

		// Find all existing links for the asset...
		$db = MatrixDAL::getDb();
		$sql = 'SELECT linkid
				FROM sq_ast_lnk
				WHERE minorid = :minorid
				OR majorid = :majorid';

		$result = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'minorid', $asset_id);
			MatrixDAL::bindValueToPdo($query, 'majorid', $asset_id);
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}

		// ...and delete them too
		if ($result != NULL) {
			$links_array = Array();
			foreach ($result as $value) {
				array_push($links_array, $value['linkid']);
			}
			$links_set = '('.implode(', ', $links_array).')';

			// Be nice and cleanup the sq_ast_lnk and sq_ast_lnk_tree as well before deleting
			foreach ($links_array as $linkids_to_delete) {
				$link_info = $this->getLinkById($linkids_to_delete);
				$majorid_delete = array_get_index($link_info, 'majorid', 0);
				$sort_order_delete = array_get_index($link_info, 'sort_order', 0);
				$lnk_tree_sql = 'UPDATE sq_ast_lnk_tree SET num_kids = num_kids - 1 WHERE treeid IN (SELECT CASE WHEN LENGTH(SUBSTR(t.treeid, 1, LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.')) != 0 THEN SUBSTR(t.treeid, 1, LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.') ELSE \'-\'END FROM sq_ast_lnk_tree t WHERE t.linkid = :linkid)';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				try {
					$lnk_tree_query = MatrixDAL::preparePdoQuery($lnk_tree_sql);
					MatrixDAL::bindValueToPdo($lnk_tree_query, 'linkid', $linkids_to_delete);
					MatrixDAL::execPdoQuery($lnk_tree_query);
				} catch (Exception $e) {
					throw new Exception('Unable to update the link tree for linkid: '.$linkids_to_delete.' due to database error: '.$e->getMessage());
				}//end try-catch
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				if (!empty($sort_order_delete) && !empty($majorid_delete)) {
					$lnk_sql = 'UPDATE sq_ast_lnk SET sort_order = sort_order - 1 WHERE majorid = :majorid AND sort_order > :sort_order';

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					try {
						$lnk_query = MatrixDAL::preparePdoQuery($lnk_sql);
						MatrixDAL::bindValueToPdo($lnk_query, 'majorid', $majorid_delete);
						MatrixDAL::bindValueToPdo($lnk_query, 'sort_order', $sort_order_delete);
						MatrixDAL::execPdoQuery($lnk_query);
					} catch (Exception $e) {
						throw new Exception('Unable to update sort orders for majorid: '.$majorid_delete.' due to database error: '.$e->getMessage());
					}//end try-catch
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				}//end if
			}//end foreach

			// Delete links
			$sql = 'DELETE FROM sq_ast_lnk WHERE linkid IN '.$links_set;
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::execPdoQuery($query);

			// Delete link tree entry
			$sql = 'DELETE FROM sq_ast_lnk_tree WHERE linkid IN '.$links_set;
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::execPdoQuery($query);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end purgeAsset()


	/**
	* This function returns the hide frame value for a design.
	*
	* @param int	$asset_id	the asset ID of the asset to delete
	*
	* @return void
	* @access public
	*/
	public function getDesignHideFrameValues($designid)
	{
		$db = MatrixDAL::getDb();
		$sql = "SELECT v.custom_val
				FROM sq_ast_attr_val v JOIN sq_ast_attr t ON v.attrid = t.attrid
				WHERE t.name = 'limbo_no_frames' and v.assetid = :designid";

		$result = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'designid', $designid);
			$result = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}

		return $result;

	}//end getDesignHideFrameValues()


	/**
	* Checks whehter the given asset is under specific root nodes
	*
	* @param string	$assetid	the asset ID of the asset to see if it's under specific root nodes
	* @param array	$root_nodes	an array of assetids of root nodes
	*
	* @return mixed	Either a boolean or an array of matched nodes
	* @access public
	*/
	public function isUnderRootNodes($assetid, $root_nodes=Array(), $return_matched_nodes=FALSE)
	{
		assert_valid_assetid($assetid);
		if (empty($root_nodes)) return FALSE;
		$matched_root_nodes = Array();

		// get treeids of root nodes
		$root_node_treeids = $this->getAssetTreeids($root_nodes);

		// if checking for the root nodes that have been deleted by the user
		// then the treeids wont be populated. best is the bail out from here
		if(empty($root_node_treeids)) return FALSE;

		// put all the treeids of root nodes into an array of 'where' conditions
		// i.e.
		//Array
		//(
		//    [0] => t.treeid LIKE '00060000%'
		//    [1] => t.treeid LIKE '00060001%'
		//    [2] => t.treeid LIKE '000600020000%'
		//)
		$all_root_treeids = Array();
		foreach ($root_node_treeids as $root_node_assetid => $treeids) {
			foreach ($treeids as $treeid) {
				$all_root_treeids[] = 't.treeid LIKE '. MatrixDAL::quote($treeid[0] . '%');
			}//end foreach
		}//end foreach

		// query the db to see if the asset is under the treeids
		$sql = 'SELECT l.majorid, t.treeid
				FROM sq_ast_lnk l JOIN sq_ast_lnk_tree t ON l.linkid=t.linkid
				WHERE l.minorid=:assetid and (' . implode(' OR ', $all_root_treeids) . ')';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}
		// if we don't have to return which root nodes matched, just return if we found a match or not
		if (!$return_matched_nodes) {
			// if there is a match, that means the asset in question is under at least one of the root nodes
			return !empty($result);
		} else {
			// if we need to return the matched root nodes, find which ones match the root nodes
			foreach ($result as $root_node) {
				foreach ($root_node_treeids as $root_node_assetid => $treeids) {
					foreach ($treeids as $treeid) {
						if (substr($root_node['treeid'], 0, strlen($treeid[0])) == $treeid[0]) {
							$matched_root_nodes[] = $root_node_assetid;
						}
					}
				}
			}

			// return the root nodes that matched
			return $matched_root_nodes;
		}


	}//end isUnderRootNodes()

}//end class
?>
