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
* $Id: asset_manager.inc,v 1.558 2007/03/08 03:10:23 tbarrett Exp $
*
*/


/**
* Asset_Manager
*
* Purpose
*    Holds information on installed packages, as represented by the DB
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.558 $
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
		$this->_asset_cache =& new Asset_Cache();
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

		if (file_exists(SQ_DATA_PATH.'/private/db/asset_types.inc')) {
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT type_code, version, name, instantiable, allowed_access, parent_type, dir, customisation, description, lvl
				FROM sq_ast_typ';

		$where = '';
		if (!is_null($instantiable)) {
			$where .= ' WHERE instantiable = '.$db->quote((int)$instantiable);
		}
		if ($non_system_access) {
			$where .= (($where) ? ' AND' : ' WHERE').' allowed_access != '.$db->quote('system');
		}

		$result = $db->query($sql.$where);
		assert_valid_db_result($result);

		while (NULL !== ($row = $result->fetchRow())) {
			$asset_types[$row['type_code']] = $row;
		}
		$result->free();

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT type_code, version, name, instantiable, allowed_access, parent_type, dir, customisation, description, lvl
				FROM sq_ast_typ
				WHERE type_code = '.$db->quote($type_code);
		$result = $db->getRow($sql);
		assert_valid_db_result($result);

		if (empty($result)) {
			trigger_localised_error('SYS0085', E_USER_WARNING, $type_code);
			return;
		} else {
			$this->_asset_types[$result['type_code']] = $result;
		}

		$parents = Array($type_code);
		$tmp_type_code = $type_code;
		while ($this->_asset_types[$tmp_type_code]['parent_type'] != 'asset') {
			// this should NEVER happen, if it does DIE
			assert_isset($this->_asset_types[$this->_asset_types[$tmp_type_code]['parent_type']], 'Asset "'.$this->_asset_types[$tmp_type_code]['parent_type'].'" is not installed on the system,<br/>'
							.'therefore we cannot get the parent type of Asset "'.$tmp_type_code.'".<br/>'
							.'Check that you got the Asset_Manager by reference');

			$tmp_type_code = $this->_asset_types[$tmp_type_code]['parent_type'];
			$parents[] = $tmp_type_code;
		}
		$parents[] = 'asset';

		$sql = 'SELECT inhd_type_code
				FROM sq_ast_typ_inhd
				WHERE type_code = '.$db->quote($type_code);
		$db_parents = $db->getCol($sql);
		assert_valid_db_result($db_parents);

		$inserts = array_diff($parents, $db_parents);
		$deletes = array_diff($db_parents, $parents);
		$updates = array_intersect($parents, $db_parents);

		$type_code_level = (int) $this->getTypeInfo($type_code, 'lvl');

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($inserts as $inherited_type_code) {
			$inherited_type_code_level = ($inherited_type_code == 'asset') ? 0 : (int) $this->getTypeInfo($inherited_type_code, 'lvl');
			$sql = 'INSERT INTO sq_ast_typ_inhd
					(inhd_type_code, type_code, inhd_type_code_lvl, type_code_lvl)
					VALUES
					('.$db->quote($inherited_type_code).', '.$db->quote($type_code).',
					'.$db->quote($inherited_type_code_level).', '.$db->quote($type_code_level).')';
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		foreach ($deletes as $inherited_type_code) {
			$sql = 'DELETE FROM sq_ast_typ_inhd
					WHERE inhd_type_code = '.$db->quote($inherited_type_code).'
					  AND type_code = '.$db->quote($type_code);
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		foreach ($updates as $inherited_type_code) {
			$inherited_type_code_level = ($inherited_type_code == 'asset') ? 0 : (int) $this->getTypeInfo($inherited_type_code, 'lvl');
			$sql = 'UPDATE sq_ast_typ_inhd
					SET inhd_type_code_lvl = '.$db->quote($inherited_type_code_level).',
						type_code_lvl = '.$db->quote($type_code_level).'
					WHERE inhd_type_code = '.$db->quote($inherited_type_code).'
					  AND type_code = '.$db->quote($type_code);
			$result = $db->query($sql);
			assert_valid_db_result($result);
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
		$field = trim($field);

		// if the file does not exist, then we must be installing so get the info from the database
		if (!file_exists(SQ_DATA_PATH.'/private/db/asset_types.inc')) {

			if (empty($type_code)) {
				// No type code specified
				if ($field == '') {
					return Array();
				} else {
					return NULL;
				}
			}

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			if (is_array($type_code)) {
				for (reset($type_code); NULL !== ($key = key($type_code)); next($type_code)) {
					$type_code[$key] = $db->quoteSmart($type_code[$key]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = ' = '.$db->quoteSmart($type_code);
			}

			// if they have not specified any fields, then we want to select *
			if ($field == '') {
				$field = 'type_code, version, name, description, instantiable, allowed_access, parent_type, lvl, dir, customisation';
			} else {
				$field = 'type_code, '.$field;
			}

			$sql = 'SELECT
						'.$field.'
					FROM
						sq_ast_typ
					WHERE
						type_code '.$type_code_cond;

			$result = $db->getAssoc($sql);
			assert_valid_db_result($result);

			if (!is_array($type_code) && !empty($field)) {
				return $result[$type_code];
			}
			return $result;

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
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT type_code
					FROM sq_ast_typ
					WHERE allowed_access = '.$db->quote($allowed_access);
			$type_codes = $db->getCol($sql);
			assert_valid_db_result($type_codes);
		} else {
			$type_codes = array_keys($this->_asset_types);
		}

		$offspring = Array();
		foreach ($type_codes as $type_code) {
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;
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
				$asset =& $this->getAsset($assetid);
				if (method_exists($asset, 'getAssetInfo')) {
					$shadow_results += $asset->getAssetTypeInfo($shadows, $type_code, $strict_type_code);
				}
			}
		}

		// breakout if nothing left to do
		if (empty($assetids)) return $shadow_results;

		for (reset($assetids); NULL !== ($k = key($assetids)); next($assetids)) {
			$assetids[$k] = $db->quote($assetids[$k]);
		}
		$where .= 'a.assetid IN ('.implode(', ', $assetids).')';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for ($i = 0; $i < count($type_code); $i++) {
					$type_code[$i] = $db->quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
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

		$result = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_DEFAULT, TRUE);
		assert_valid_db_result($result);

		$result += $shadow_results;

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
		$type_code = strtolower($type_code);
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

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT
						inhd_type_code
					FROM
						sq_ast_typ_inhd
					WHERE
						type_code = '.$db->quote($type_code).'
					AND	inhd_type_code <> type_code
						'.(($include_asset) ? '' : ' AND inhd_type_code <> '.$db->quote('asset')).'
					ORDER BY inhd_type_code_lvl DESC';

			$result = $db->getCol($sql);
			assert_valid_db_result($result);
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$type_code_cond = '';
		if (is_array($type_code)) {
			for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
				if ($type_code != 'asset') {
					assert_isset_array_index($this->_asset_types, $type_code[$i], 'Asset Type "'.$type_code[$i].'" is not installed on the system');
				}
				$type_code[$i] = $db->quote($type_code[$i]);
			}
			$type_code_cond = 'IN ('.implode(', ', $type_code).')';
		} else {
			if ($type_code != 'asset') {
				assert_isset_array_index($this->_asset_types, $type_code, 'Asset Type "'.$type_code.'" is not installed on the system');
			}
			$type_code_cond = '= '.$db->quote($type_code);
		}

		$result = $db->getCol('SELECT type_code
								FROM sq_ast_typ_inhd
								WHERE inhd_type_code '.$type_code_cond.'
								'.(($include_passed) ? '' : '  AND type_code <> inhd_type_code').'
								ORDER BY inhd_type_code, type_code_lvl ASC'
								);
		assert_valid_db_result($result);

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
		$decendants = $this->getTypeDescendants($type_code2, TRUE);
		return in_array($type_code1, $decendants);

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
		if (!is_file($file)) return FALSE;
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		for ($i = 0; $i < count($attrids); $i++) {
			$attrids[$i] = $db->quote($attrids[$i]);
		}
		$where = 'a.attrid IN ('.implode(', ', $attrids).')';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql = 'SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr a '.$where.' ORDER BY a.attrid';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

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
	function &getAttribute($attrid, $value=NULL, $mute_errors=FALSE)
	{
		if (!isset($this->_attributes[$attrid]) || !is_object($this->_attributes[$attrid]['object'])) {

			$this->_attributes[$attrid] = Array();
			$this->_attributes[$attrid]['object'] = NULL;
			$this->_attributes[$attrid]['count']  = 0;

			include_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
			$this->_attributes[$attrid]['object'] = Asset_Attribute::loadAttribute($attrid);
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

		$sql = 'SELECT '.implode(', ', $details).' FROM sq_ast_attr
				WHERE type_code = '.$db->quote($type_code);

		if (count($details) > 1) {
			$result = $db->getAssoc($sql);
		} else {
			$result = $db->getCol($sql);
		}

		assert_valid_db_result($result);

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
	* @param string	$attr_name	The name of the attribute to get the values of
	* @param string	$asset_type	The type code of the asset type that defines the attribute
	* @param array	$assetids	IDs of the assets whose values we want
	*
	* @return array
	* @access public
	*/
	function getAttributeValuesByName($attr_name, $asset_type, $assetids)
	{
		if (empty($assetids)) return Array();
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// IN clause fix - assetids were not being treated as strings
		$in = 'assetid IN (';

		for ($i = 0; $i < count($assetids); $i++) {
			$in .= $db->quoteSmart( (string) $assetids[$i] ).(($i == count($assetids) - 1) ? ')' : ',');
		}

		// First get the default values
		$sql = 'SELECT a.assetid, at.default_val
				FROM sq_ast a, sq_ast_attr at
				WHERE a.type_code = at.type_code
					AND at.name = '.$db->quote($attr_name).'
					AND ((at.type_code = '.$db->quote($asset_type).') OR (at.owning_type_code = '.$db->quote($asset_type).'))
					AND a.'.$in;
		$res = $db->getAssoc($sql);
		assert_valid_db_result($res);

		// Now get customised values where applicable
		$sub_sql = 'SELECT attrid FROM sq_ast_attr WHERE name = '.$db->quote($attr_name);
		if ($asset_type != 'asset') {
			$sub_sql .= ' AND (type_code = '.$db->quote($asset_type).' OR owning_type_code = '.$db->quote($asset_type).')';
		}
		$sql = 'SELECT assetid, custom_val
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val
				WHERE attrid IN ('.$sub_sql.')
					AND '.$in;
		$custom_vals = $db->getAssoc($sql);
		assert_valid_db_result($custom_vals);

		// Combine the two
		foreach ($custom_vals as $assetid => $val) {
			$res[$assetid] = $val;
		}
		return $res;

	}//end getAttributeValuesByName()


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
	function &getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$this->_get_asset_history[] = $assetid;
		assert_valid_assetid($assetid);
		$asset =& $this->_asset_cache->get($assetid);
		if (!empty($asset) && !empty($type_code)) {
			if ($asset->type() != $type_code) {
				if (!$mute_errors) {
					trigger_localised_error('SYS0089', E_USER_WARNING, $assetid, $type_code);
				}
				$asset = NULL;
				return $asset;
			}
		}
		if (empty($asset)) {
			// check if we are getting a shadow asset, and palm the request off to the
			// handler of the shadow asset if we are
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				$real_assetid = $id_parts[0];
				$bridge =& $this->getAsset($real_assetid, '', TRUE);
				if (is_null($bridge)) {
					// bridge is unknown, we cannot return anything from it
					$asset = NULL;
					return $asset;
				} else if (!method_exists($bridge, 'getAsset')) {
					trigger_localised_error('SYS0203', E_USER_WARNING, $asset->name);
				} else {
					$asset =& $bridge->getAsset($assetid, '', $mute_errors);
				}
				$this->forgetAsset($bridge);

			} else {

				if (empty($type_code)) {
					$where = 'assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($assetid);
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
					$type_code = $GLOBALS['SQ_SYSTEM']->db->getOne('SELECT type_code FROM '.SQ_TABLE_RUNNING_PREFIX.'ast '.$where);
					assert_valid_db_result($type_code);
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
							$asset =& new $type_code($assetid);
						}
						if (empty($asset->id)) $asset = NULL;
					}
				}

			}//end if

			if (isset($asset)) $this->_asset_cache->add($asset);

		}//end if

		return $asset;

	}//end getAsset()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param int|array	$assetids	the ID of the asset we are going to the check for existence
	*								or an array of assetids we want to check
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
				$asset =& $this->getAsset($shadow_id);
				if (method_exists($asset,'assetExists')) {
					$shadow_asset_ids += $asset->assetExists($shadow);
				}
				$this->forgetAsset($asset);
			}
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast ';

		// single asset id
		if (!$is_array) {
			if ($is_empty) return FALSE;
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quoteSmart($assetids));
			$db_assetids = $db->getOne($sql.$where);
			assert_valid_db_result($db_assetids);
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
				$in .= $db->quoteSmart( (string) $assetids[$i] ).(($i == count($assetids) - 1) ? ')' : ',');
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($in);
			$db_assetids = $db->getAll($sql.$where);
			assert_valid_db_result($db_assetids);

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
		if (!is_object($obj) || !is_a($obj, 'asset') || !$obj->id) {
			return;
		}

		if (!$this->_asset_cache->add($obj)) {
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
		if (!is_object($obj) || !is_a($obj, 'asset') || !$obj->id) {
			return;
		}

		$this->_asset_cache->release($obj->id);
		if ($force_clean) {
			if (!$this->_asset_cache->remove($obj->id, TRUE)) {
				trigger_localised_error('SYS0306', E_USER_WARNING, $obj->id);
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
		$sys_asset_cfg =& new System_Asset_Config();
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
	function &getSystemAsset($name, $mute_errors=FALSE)
	{
		$assetid = ($mute_errors) ? @$this->getSystemAssetid($name) : $this->getSystemAssetid($name);

		if ($assetid !== FALSE) {
			$asset =& $this->getAsset($assetid, $name, $mute_errors);
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
		// check if we are trying to purge a system asset
		require SQ_DATA_PATH.'/private/conf/system_assets.inc';
		$system_asset_ids = array_values($system_assets);
		if (in_array($asset->id, $system_asset_ids)) {
			return FALSE;
		}

		// if we get to here, it is not one of the
		// system assets, but it may be dependently linked to one
		// so lets check for that
		$parents = $this->getDependantParents($asset->id);
		foreach ($parents as $parentid) {
			if (in_array($parentid, $system_asset_ids)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end canPurgeAsset()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array			$assetids			an array of assets to limit
	* @param string|array	$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean		$strict_type_code	ensure returned assets are of the passed type codes
	* @param string			$field				single field from the asset table to return (along with the assetid)
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;

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
				$asset =& $this->getAsset($assetid, '', TRUE);
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
			if (!in_array($field, $this->_tmp['sq_tables']['ast']['columns'])) {
				trigger_localised_error('SYS0185', E_USER_WARNING, $field);
				return Array();
			}
		}

		for ($i = 0; $i < count($assetids); $i++) {
			$assetids[$i] = $db->quoteSmart((string) $assetids[$i]);
		}

		// break up the assets into chunks of 1000 so that oracle does not complain
		$in_clauses = Array();
		foreach (array_chunk($assetids, 999) as $chunk) {
			$in_clauses[] = ' a.assetid IN ('.implode(', ', $chunk).')';
		}
		$where = '('.implode(' OR ', $in_clauses).')';

		if (!empty($type_code)) {
			$type_code_cond = '';
			if (is_array($type_code)) {
				for ($i = 0; $i < count($type_code); $i++) {
					$type_code[$i] = $db->quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
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
			require SQ_DATA_PATH.'/private/db/table_columns.inc';
			unset($tables['ast']['columns'][array_search('assetid', $tables['ast']['columns'])]);
			$col_string = implode(', ', $tables['ast']['columns']);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT assetid, '.((empty($field)) ? $col_string : $field).'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a
				'.$where.'
				ORDER BY a.assetid';

		$result = $db->getAssoc($sql);
		assert_valid_db_result($result);

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if ($strict) {
			$where = 'type_code = '.$db->quote($type_code);
		} else {
			$where = 'type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE inhd_type_code = '.$db->quote($type_code).'
									)';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT assetid'.(($include_type) ? ', type_code' : '').'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast '.$where;

		if ($include_type) {
			$result = $db->getAssoc($sql);
		} else {
			$result = $db->getCol($sql);
		}

		assert_valid_db_result($result);

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
			$child =& $this->getAsset($child_id);
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
		$asset =& $this->getAsset($assetid);

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
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_cloneAsset(&$source, &$link, &$clone_map, $components, $cloning_dependent=FALSE)
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
			$lock = $this->getLockInfo($source->id, 'all');
			foreach ($lock as $lock_type => $lock_info) {
				if (empty($lock_info)) {
					trigger_localised_error('SYS0266', E_USER_WARNING, $source->id, $lock_type);
					return $null;
				}
			}
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextId('sq_ast');
		assert_valid_db_result($assetid);

		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		// version number starts at 0.0.0 again because the clone is a brand new asset
		$initial_version = '0.0.0';

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$sql = 'INSERT INTO
					sq_ast
					(
						assetid,
						version,
						type_code,
						name,
						short_name,
						status,
						created,
						created_userid,
						updated,
						updated_userid
					)
					VALUES
					(
						'.$db->quoteSmart($assetid).',
						'.$db->quoteSmart($initial_version).',
						'.$db->quoteSmart($source->type()).',
						'.$db->quoteSmart($source->name.' - clone').',
						'.$db->quoteSmart($source->short_name.' - clone').',
						'.$db->quoteSmart(SQ_STATUS_UNDER_CONSTRUCTION).',
						'.db_extras_todate($db, ts_iso8601($now)).',
						'.$db->quoteSmart($userid).',
						'.db_extras_todate($db, ts_iso8601($now)).',
						'.$db->quoteSmart($userid).'
					)';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$clone =& $this->getAsset($assetid, $source->type());
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
		$clone = $this->_cloneLink($clone, $link, $lock);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $clone_result;
		}

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
	* Acquires a lock on an asset
	*
	* @param int	&$clone	the new clone
	* @param string	&$link	the link object
	* @param int	&$lock	the lock object
	*
	* @return mixed NULL|object
	* @access public
	*/
	function &_cloneLink(&$clone, &$link, &$lock)
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

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				// OK what we are going to do is make sure that the new parents lock is
				// in the same chain as our current lock
				$parent_lock = $this->getLockInfo($link['asset']->id, 'links');
				if (empty($parent_lock) || $parent_lock['source_assetid'] != $lock['links']['source_assetid']) {

					if (!empty($parent_lock)) {
						if (!$this->releaseLock($link['asset']->id, 'links')) {
							return $null;
						}
					}

					if (!$this->acquireLock($link['asset']->id, 'links', $lock['links']['source_assetid'])) {
						return $null;
					}
				}
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
	*
	* @return object
	* @access public
	*/
	function &cloneAsset(&$source, &$link, &$clone_map, $components, $clone_dependents=TRUE, $cloning_dependent=FALSE)
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

		$clone =& $this->_cloneAsset($source, $link, $clone_map, $components, $cloning_dependent);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $null;
		}

		// OK now that we are linked up let's our dependents
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

						$cloned_child =& $this->getAsset($clone_map[$data['minorid']], $data['minor_type_code']);
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
						$child =& $this->getAsset($data['minorid'], $data['minor_type_code']);
						if (is_null($child)) continue;

						// skip shadow assets
						$id_parts = explode(':', $child->id);
						if (isset($id_parts[1])) continue;

						$create_link['link_type']		= $data['link_type'];
						$create_link['value']			= $data['value'];
						$create_link['sort_order'] 		= $data['sort_order'];
						$create_link['is_exclusive']	= $data['is_exclusive'];
						$new_component = Array();

						$cloned_child =& $this->cloneAsset($child, $create_link, $clone_map, $components, $clone_dependents, TRUE);

						if (is_null($cloned_child)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return $null;
						}
					}//end if

					if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
					// lock the new clone in the same chain as we are locked
					$lock = $this->getLockInfo($cloned_child->id,'all');
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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

		$asset =& $this->getAsset($assetid);
		assert_not_null($asset);

		$current_locks = $this->getLockInfo($assetid, $lock_type, TRUE, TRUE);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = TRUE;

		$can_force = ($force) ? $asset->canForceablyAcquireLock($lock_type) : FALSE;
		$lock_updated = FALSE;
		foreach ($current_locks as $lock_type => $lock) {

			if (!empty($lock) && $lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
				// the user is asking to acquire a lock they already had
				// so just update the lock expiry date
				if ($this->updateLock($assetid, $lock_type, $expires)) {
					$lock_updated = TRUE;
					continue;
				} else {
					$success = FALSE;
					break;
				}
			}

			// special case where the current user is actually
			// removing the current lock and taking it for themselves
			if (!empty($lock) && $can_force) {

				// attempt to remove the lock
				if (!$this->releaseLock($assetid, $lock_type)) {
					$success = FALSE;
					break;
				}

				// send an internal message
				$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$ms->openLog();

				$user =& $this->getAsset($lock['userid']);

				$locked_assetids = Array((int) $assetid);
				foreach ($lock['chained_assets'] as $row) {
					$locked_assetids[] = $row['assetid'];
				}

				foreach ($locked_assetids as $locked_assetid) {
					$locked_asset =& $this->getAsset($locked_assetid);

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
				$lock = $this->getLockInfo($assetid, $lock_type, FALSE, TRUE);

			}//end if

			// is this asset already locked
			if (!empty($lock)) {
				$user =& $this->getAsset($lock['userid']);
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

		$current_locks = $this->getLockInfo($assetid, $lock_type, TRUE, FALSE);
		if (empty($current_locks)) return TRUE;

		$asset =& $this->getAsset($assetid);
		assert_not_null($asset);

		$success = TRUE;

		$can_force = $asset->canForceablyAcquireLock($lock_type);

		foreach ($current_locks as $lock_type => $lock) {
			if (empty($lock)) continue;

			// is this asset already locked by someone else and we can't forceably acquire it, error
			if ((int) $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserid() && !$can_force) {
				$user =& $this->getAsset($lock['userid']);
				trigger_localised_error('SYS0264', E_USER_WARNING, $lock_type, $asset->name, $user->name);
				$success = FALSE;
				continue;
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
		$current_locks = $this->getLockInfo($assetid, $lock_type, TRUE, TRUE);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = TRUE;

		foreach (array_keys($current_locks) as $lock_type) {

			if (TRUE !== ($err_msg = $GLOBALS['SQ_SYSTEM']->updateLock('asset.'.$assetid.'.'.$lock_type, $expires))) {
				trigger_localised_error('SYS0122', E_USER_NOTICE, $assetid, $err_msg);
				$success = FALSE;
				break;
			}

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
	* @param int		$assetid		the assetid of the asset that we want lock info for
	* @param string		$lock_type		any valid lock type for the passed asset's type, plus 'all' which will return all locks that this asset has - will force an array to be returned
	* @param boolean	$force_array	always return the information in an array, as though more than one lock has been requested
	* @param boolean	$full_chain		return the info for every asset in the lock chain
	* @param boolean	$check_expires	check the expiry date and cleanup the lock if it has expired
	*
	* @return array
	* @access public
	*/
	function getLockInfo($assetid, $lock_type, $force_array=FALSE, $full_chain=FALSE, $check_expires=TRUE)
	{
		$lock_types = $this->getLockTypes($assetid, $lock_type);
		$locks = Array();
		foreach ($lock_types as $lock_type) {

			$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo('asset.'.$assetid.'.'.$lock_type, $full_chain, $check_expires);
			if (!empty($lock)) {
				$lock['lock_type']      = $lock_type;
				$lock['source_assetid'] = preg_replace('|^asset\.(.*)\.[\w]+$|', '\1', $lock['source_lockid']);
				if ($full_chain) {
					for (reset($lock['chained_assets']); NULL !== ($k = key($lock['chained_assets'])); next($lock['chained_assets'])) {
						$lock['chained_assets'][$k]['assetid'] = preg_replace('/^asset\.([0-9]+(:.+)?)\..*$/', '\1', $lock['chained_assets'][$k]['lockid']);
						$lock['chained_assets'][$k]['source_assetid'] = preg_replace('/^asset\.([0-9]+(:.+)?)\..*$/', '\1', $lock['chained_assets'][$k]['source_lockid']);
					}
				}
			}
			$locks[$lock_type] = $lock;
			unset($lock);
		}//end foreach
		if (!$force_array && count($locks) == 1) {
			return reset($locks);
		} else {
			return $locks;
		}

	}//end getLockInfo()


	/**
	* Returns an array of lock types strings that the passed lock type really represents for the passed asset
	* eg 'content' could mean 'attributes' and 'links'
	*
	* @param int	$assetid	the assetid of the asset that we want lock info for
	* @param string	$lock_type	any valid lock type for the passed asset's type, plus 'all' which will return all lock types that this asset has
	*
	* @return array
	* @access private
	*/
	function getLockTypes($assetid, $lock_type)
	{
		if (!isset($this->_tmp['lock_types'][$assetid])) {
			$asset =& $this->getAsset($assetid);
			assert_not_null($asset);
			$this->_tmp['lock_types'][$assetid] = $asset->lockTypes();
		}

		if ($lock_type !== 'all') {
			if (!isset($this->_tmp['lock_types'][$assetid][$lock_type])) {
				return Array();
			}
			$bits = bit_elements($this->_tmp['lock_types'][$assetid][$lock_type]);
			$lock_types = Array();
			foreach ($bits as $bit) {
				if (FALSE !== ($k = array_search($bit, $this->_tmp['lock_types'][$assetid]))) {
					$lock_types[] = $k;
				}
			}
			return $lock_types;
		} else {
			$lock_types = Array();
			foreach ($this->_tmp['lock_types'][$assetid] as $lock_type => $bit) {
				if (preg_match('/^0*10*$/', decbin($bit))) {
					$lock_types[] = $lock_type;
				}
			}
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

		// if its a shadow assetid pipe it off to bridge
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$asset =& $this->getAsset($id_parts[0]);
			$link =  $asset->getLink($assetid, $link_type, $type_code, $strict_type_code, $value, $side_of_link, $exclusive);
			$this->forgetAsset($asset);
			return $link;
		}

		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						l.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';

		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid);

		if (!is_null($link_type)) {
			$where .= '
				  AND l.link_type = '.$db->quote($link_type);
		}

		if (!is_null($exclusive)) {
			$where .= ' AND l.is_exclusive = '.$db->quote(($exclusive) ? '1' : '0');
		}

		if ($type_code) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
					$type_code[$i] = $db->quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
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
			$where .= ' AND (l.value = '.$db->quote($value);
			if ($value == '') $where .= ' OR l.value IS NULL';
			$where .= ')';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		$sql .= $where.' ORDER BY l.sort_order';

		$result = $db->getRow($sql);
		assert_valid_db_result($result);

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
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

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

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type
				FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk l';

		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid);

		if (!is_null($other_assetid)) {
			$where .= ' AND l.'.$other_side.'id  = '.$db->quote($other_assetid);
		}

		if (!is_null($link_types)) {
			$where .= ' AND '.db_extras_bitand($db, 'l.link_type', $link_types).' > 0 ';
		}

		if (!is_null($value)) {
			$where .= ' AND l.value = '.$db->quote($value);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		$result = $db->getAll($sql.$where);
		assert_valid_db_result($result);

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
			$bridge =& $this->getAsset($id_parts[0]);
			$link = $bridge->getLinkById($linkid, $assetid, $side_of_link);
			$this->forgetAsset($bridge);
			return $link;
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT  l.linkid, l.value, l.link_type, l.sort_order, l.is_dependant, l.is_exclusive, ';
		$where = 'l.linkid = '.$db->quote((int) $linkid);

		// if they only want one side of the link, we can do less work
		if ($assetid !== 0) {
			$sql .= 'l.'.$other_side.'id, l.type_code as '.$other_side.'_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';
			$where .= ' AND l.'.$side_of_link.'id = '.$db->quote($assetid);
		} else {
			$sql .= 'l.majorid, l.type_code as minor_type_code, l.minorid, a.type_code as major_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

		$result = $db->getRow($sql.$where);
		assert_valid_db_result($result);

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
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

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

					if (!empty($major_shadow_parts)) {
						// major is shadow asset, use bridge
						$link = Array();
						$asset =& $this->getAsset($major_shadow_parts['bridgeid']);
						if (!is_null($asset)) {
							$link = $asset->getLinkByAsset($majorid, $minorid, $link_types, $value, $side_of_link, $force_array, $dependant, $exclusive);
							$this->forgetAsset($asset);
						}
						$shadow_links[] = $link;
					} else {
						if (!empty($minor_shadow_parts)) {
							// minor is shadow asset, look at the shadow link table
							// instead of the regular link table
							$link = $this->getShadowLinkByAsset($majorid, $minorid, $link_types, $value, $side_of_link, $force_array, $dependant, $exclusive);
							$shadow_links[] = $link;
						}
					}

				}//end foreach minor asset ID

			}//end foreach major asset ID

		}//end if there are shadow assets

		// If there are no more assets to run through, return what shadow links
		// we have only
		if (!empty($other_assetid)) {

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$prepared_otherids = Array();
			foreach ($other_assetid as $id) {
				$prepared_otherids[] = $db->quote($id);
			}

			$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

			$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						   l.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive
					FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';

			$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid).'
					  AND l.'.$other_side.'id  IN ( '.implode(',', $prepared_otherids).')';

			if (!is_null($link_types)) {
				$where .= ' AND '.db_extras_bitand($db, 'l.link_type', $link_types).' > 0 ';
			}
			if (!is_null($value)) {
				if (empty($value)) {
					$where .= ' AND (l.value = '.$db->quote($value).' OR l.value IS NULL)';
				} else {
					$where .= ' AND l.value = '.$db->quote($value);
				}
			}
			if (!is_null($dependant)) {
				$where .= ' AND l.is_dependant = '.$db->quote(($dependant) ? '1' : '0');
			}
			if (!is_null($exclusive)) {
				$where .= ' AND l.is_exclusive = '.$db->quote(($exclusive) ? '1' : '0');
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);

			$sql .= $where.'
					ORDER BY l.sort_order';

			$result = $db->getAll($sql);
			assert_valid_db_result($result);
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
	* @param string			$value				The value for all the links must equal this (if not null)
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
				$asset =& $this->getAsset($real_assetid);
				if (is_null($asset)) continue;

				$links[$assetid] = $asset->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by, $dependant, $exclusive);
				$this->forgetAsset($asset);

				// we also need the links to real assets for this shadow asset
				// if we are looking up the tree
				if (($side_of_link == 'minor') && !$exclusive && !$dependant) {
					$db =& $GLOBALS['SQ_SYSTEM']->db;
					$sql = 'SELECT
								l.linkid,
								l.majorid,
								l.minorid,
								l.value,
								l.link_type,
								a.type_code AS major_type_code
							FROM
								'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a on l.majorid = a.assetid';

					$where = 'minorid = '.$db->quoteSmart($assetid);
					if ($type_code) {
						if ($strict_type_code) {
							$where .= ' AND a.type_code = '.$db->quoteSmart($type_code);
						} else {
							$type_ancestors = $this->getTypeDescendants($type_code);
							foreach ($type_ancestors as $i => $v) {
								$type_ancestors[$i] = $db->quoteSmart($v);
							}
							$type_ancestors[] = $db->quoteSmart($type_code);
							$where .= ' AND a.type_code IN ('.implode(', ', $type_ancestors).')';
						}
					}
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

					$result = $db->getAll($sql.$where);
					assert_valid_db_result($result);

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
										);

					}
					$links[$assetid] = array_merge($links[$assetid], $shdw_links);
				}//end if side_of_link is major

				continue;

			}//end if shadow asset

			// we are not getting links for a shadow asset, so if we are getting child links of
			// an asset that handles shadow assets, palm the request off to it
			if ($side_of_link == 'major') {
				$asset =& $this->getAsset($assetid);
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
			// we couldnt palm the request off, so we better do it ourselves
			$query = $this->generateGetLinksQuery($query_assetids, $link_types, $type_code, $strict_type_code, $side_of_link, $value, $dependant, $exclusive, $sort_by, $access, $effective);
			if (!empty($query)) {
				$db =& $GLOBALS['SQ_SYSTEM']->db;
				$result = $db->getAssoc(implode(' ', $query), FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
				assert_valid_db_result($result);
				$links = $this->arrayMerge($result,$links);
			}
		}

		return (count($links) == 1 && !$force_array) ? array_shift($links) : $links;

	}//end getLinks()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string	$assetid	the assetid of the asset to find the child links for
	* @param int	$link_type	the type of links to get
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!isset($this->_tmp['child_links'][$assetid])) {

			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				$asset =& $this->getAsset($id_parts[0]);
				$links = $asset->getAllChildLinks($assetid, $link_type);
				$this->forgetAsset($asset);
				return $links;
			}

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$where = 'l.minorid = '.$db->quote($assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid'.$where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$treeid = $db->getOne($sql);
			assert_valid_db_result($treeid);

			$current_level = strlen($treeid) / SQ_CONF_ASSET_TREE_SIZE;

			$where = 't.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid);

			// only do a link comparison if they have specified a link type
			if ($link_type != 0) {
				$where .=  ' AND '.db_extras_bitand($db, 'l.link_type', $link_type).' > 0';
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT SUBSTR(t.treeid, '.$db->quoteSmart((int) strlen($treeid) + 1).') as treeid,
							((LENGTH(t.treeid) / '.SQ_CONF_ASSET_TREE_SIZE.') '.(($current_level) ? ' - '.$current_level : '').') as lvl,
							l.linkid, a.assetid, a.short_name, a.type_code, l.link_type, l.sort_order, l.value, l.is_dependant, l.is_exclusive
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid
					'.$where.'
					ORDER BY t.treeid';

			$child_links = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
			assert_valid_db_result($child_links);


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

			$this->_tmp['child_links'][$assetid] = $child_links;

		}//end if

		return $this->_tmp['child_links'][$assetid];

	}//end getAllChildLinks()


	/**
	* For a given asset, get the lineages from the Root folder down to it
	*
	* Also returns basic information about the last link (linkid and link type)
	* linkid => Array ('linkid' => linkid, 'link_type' => link_type, 'lineage' => Array(...))
	*
	* @param string			$assetid		ID of the asset whose linages we want
	* @param int			$result_limit	limits the number of results returned
	* @param string|array	$from_treeid	Uses this/these treeid(s) as the root node
	* @param string|array	$fields			Returns these asset field(s) in lineage details (e.g name, type_code)
	*
	* @return array
	* @access public
	*/
	function getLinkLineages($assetid, $result_limit=0, $from_treeid=NULL, $fields='name')
	{
		assert_valid_assetid($assetid);

		$link_lineages = Array();
		$assetids = Array();
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		if (is_array($fields)) {
			$fields_str = 'a.'.implode(', a.', $fields);
		} else {
			$fields_str = 'a.'.$fields;
		}

		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			// if this asset is a shadow asset the we want the get the lineages of the
			// bridge, and also the lineages of and real assets that this asset is linked under
			$asset_field = is_array($fields) ? 'name' : $fields;
			$bridge_info   = $this->getAssetInfo(Array($id_parts[0]), '', TRUE, $asset_field);
			$link_lineages = $this->getLinkLineages($id_parts[0]);

			// add the lineage of the actual bridge to the bridge's lineages
			for (reset($link_lineages); NULL !== ($key = key($link_lineages)); next($link_lineages)) {
				$link_lineages[$key]['lineage'][$id_parts[0]] = $bridge_info[$id_parts[0]];
			}
			$sql = 'SELECT
						l.majorid, '.$fields_str.'
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';

			$where = 'l.minorid = '.$db->quoteSmart($assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

			$result = $db->getAssoc($sql.$where);
			assert_valid_db_result($result);

			// get the lineage of the real assets that this shadow asset is linked under
			// and then append the lineage of that real asset to its lineages
			foreach ($result as $assetid => $name) {
				$lineages = $this->getLinkLineages($assetid);
				for (reset($lineages); NULL !== ($key = key($lineages)); next($lineages)) {
					$lineages[$key]['lineage'][$assetid] = $name;
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

		$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.assetid, '.$fields_str.'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk cl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ON cl.linkid = ct.linkid,
					 '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk pl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt ON pl.linkid = pt.linkid
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON a.assetid = pl.minorid
				';

		// we want to get all the parent links of this assetid and use these
		// as the minorid in the query, effectively getting the lineages of the
		// asset's immediate parents.

		$sub_sql   = 'SELECT l.majorid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l';
		$sub_where = 'l.minorid = '.$db->quoteSmart((string) $assetid);
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
		$sub_sql  .= $sub_where;

		$where = 'cl.minorid IN ('.$sub_sql.')
				  AND ct.treeid LIKE pt.treeid || '.$db->quote('%').'
				  AND pt.treeid <= ct.treeid';

		if (!is_null($from_treeid)) {
			if (is_array($from_treeid)) {
				$treeid_bits = Array();
				foreach ($from_treeid as $ftid) {
					$treeid_bits[] = '(pt.treeid LIKE '.$db->quoteSmart((string)$ftid.'%').')';
				}
				$where .= ' AND ('.implode(' OR ', $treeid_bits).')';
			} else {
				$where .= ' AND pt.treeid LIKE '.$db->quoteSmart((string) $from_treeid.'%');
			}
		}

		$in_sql = SQ_TABLE_RUNNING_PREFIX.'get_lineage_treeids('.$db->quoteSmart((string) $assetid).', '.
		$db->quoteSmart((int) SQ_CONF_ASSET_TREE_SIZE).')';

		if ($db->phptype == 'oci8') {
			$in_sql = 'SELECT * FROM table('.$in_sql.')';
		} else {
			$in_sql = 'SELECT * FROM '.$in_sql;
		}

		$where .= ' AND pt.treeid IN ('.$in_sql.')';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.'
				ORDER BY cl.linkid, ct.treeid, pt.treeid';

		$result = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
		assert_valid_db_result($result);

		$asset_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', TRUE, 'minor');

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
	*
	* @return array
	* @access public
	* @see generateGetParentsQuery()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE, $sort_by=NULL, $access=NULL, $effective=TRUE, $min_height=NULL, $max_height=NULL, $ignore_bridge=FALSE)
	{
		// check if we are getting parents for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset =& $this->getAsset($real_assetid);

			$parents = Array();
			if (!$ignore_bridge) {
				$parents = $asset->getParents($assetid, $type_code, $strict_type_code);
			}

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT
						sl.majorid
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk sl';

			$where = 'sl.minorid = '.$db->quoteSmart($assetid);

			if (!empty($type_code)) {
				$sql .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a
						 ON sl.majorid = a.assetid ';
				// we are cloning $type_code and using the clone instead
				// this will prevent double quoting $type_code in the future
				$type_code_clone = $type_code;
				if (is_array($type_code_clone)) {
					for (reset($type_code_clone); NULL !== ($key = key($type_code_clone)); next($type_code_clone)) {
						$type_code_clone[$key] = $db->quoteSmart($type_code_clone[$key]);
					}
					$where .= ' AND a.type_code IN ('.implode(',', $type_code_clone).')';
				} else {
					$where .= ' AND a.type_code = '.$db->quoteSmart($type_code_clone);
				}
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$shdw_parentids = $db->getCol($sql.$where);
			assert_valid_db_result($shdw_parentids);
			$shdw_parents = $this->getAssetInfo($shdw_parentids, Array(), TRUE, 'type_code');

			$real_parents = Array();
			// we also need the parents of the majorid from the shadow asset link table
			foreach ($shdw_parentids as $parentid) {
				$real_parents = $this->getParents($parentid, $type_code, $strict_type_code, $sort_by, $access);
			}

			$parents = $parents + $shdw_parents + $real_parents;
			$this->forgetAsset($asset);
			return $parents;
		}//end if shadow asset

		$query = $this->generateGetParentsQuery($assetid, $type_code, $strict_type_code, $sort_by, $access, $effective, $min_height, $max_height);
		if (empty($query)) return Array();
		$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc(implode(' ', $query));
		assert_valid_db_result($result);

		$parents = Array();
		foreach ($result as $assetid => $asset_info) {
			$parents[$assetid] = $asset_info['type_code'];
		}
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
	*
	* @return array
	* @access public
	* @see generateGetChildrenQuery()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE, $min_depth=NULL, $max_depth=NULL, $direct_shadows_only=TRUE)
	{
		assert_valid_assetid($assetid);
		// check if we are getting children for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$children = Array();
			$real_assetid = $id_parts[0];
			$asset =& $this->getAsset($real_assetid);
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
		$asset =& $this->getAsset($assetid);
		if (is_null($asset)) return Array();
		if (implements_interface($asset, 'bridge')) {
			$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $dependant, $sort_by);
			$this->forgetAsset($asset);
			return $children;
		}
		$query = $this->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $dependant, $sort_by, $access, $effective, TRUE, $min_depth, $max_depth, $direct_shadows_only);
		$this->forgetAsset($asset);
		unset($asset);
		if (empty($query)) return Array();
		$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc(implode(' ', $query));
		assert_valid_db_result($result);

		if (!is_null($sort_by)) {
			$children = Array();
			foreach ($result as $assetid => $asset_info) {
				$children[$assetid] = $asset_info['type_code'];
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

		$asset =& $this->getAsset($assetid);
		if (!is_null($asset)) {

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'major', NULL, 1);
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				if (!$dependant_links[$k]['is_dependant']) continue;
				$children[$dependant_links[$k]['minorid']] = $dependant_links[$k]['minor_type_code'];
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

		$asset =& $this->getAsset($assetid);
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
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		// Stuff that is only needed once per design file
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$assetids  = Array();
		$majorids_str = '';

		foreach ($majorids as $id) {
			if (empty($tree_data[$id])) {
				$majorids_str .= $db->quoteSmart((string) $id).',';
			} else {
				foreach ($tree_data[$id] as $row) {
					$assetids[] = (int) $row['assetid'];
				}
			}
		}

		if (empty($majorids_str)) return $assetids;

		$exclude_list_before_quote = $exclude_list;
		for (reset($exclude_list); NULL !== ($k = key($exclude_list)); next($exclude_list)) {
			$exclude_list[$k] = $db->quoteSmart((string) $exclude_list[$k]);
		}

		$exclude_str = implode(',', $exclude_list);

		static $USERIDS_COND = NULL;

		if (is_null($USERIDS_COND)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$USERIDS_COND = '';
			} else {
				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$userids[] = $db->quoteSmart($this->getSystemAssetid('public_user'));
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quoteSmart((string)$userids[$i]);
				}
				$USERIDS_COND = 'AND p.userid  IN ('.implode(',', $userids).')';
			}

			if (!empty($USERIDS_COND)) {
				$USERIDS_COND .= '
					GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order, p.granted
					HAVING MIN(p.granted) = 1';
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
						$include_types += $this->getTypeDescendants($include_type, TRUE);
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
		$where  = 'l.majorid IN ('.substr($majorids_str, 0, -1).')
					 AND ('.db_extras_bitand($db, 'l.link_type', $link_type).' > 0) ';
		if (!empty($exclude_list)) {
			$where .= 'AND a.assetid NOT IN ('.$exclude_str.')';
		}

		if (!empty($include_types_str)) {
			$where .= ' AND a.type_code IN ('.$include_types_str.')';
		}

		if (!$include_dependants) {
			$where .= ' AND l.is_dependant = 0';
		}

		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		if (!empty($USERIDS_COND)) {
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'p');
		}
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where .= ' '.$USERIDS_COND.'
				   ORDER BY l.majorid, l.sort_order';
		$result = $db->query($sql.$where);
		assert_valid_db_result($result);


		$assetids = Array();
		while (DB_OK === $result->fetchInto($row)) {
			if (!($row['status'] & (SQ_STATUS_LIVE | SQ_STATUS_LIVE_APPROVAL))) {
				$asset =& $this->getAsset($row['assetid']);
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
		$result->free();

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		if (is_array($assetid)) {
			$assetids_set = Array();
			foreach ($assetid as $this_assetid) {
				assert_valid_assetid($this_assetid);
				$assetids_set[] = $db->quoteSmart((string)$this_assetid);
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
			$where  = 'WHERE l.minorid = '.$db->quoteSmart($assetid);
		}

		$where .= '	AND '.db_extras_bitand($db, 'l.link_type', $link_type).' > 0';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		if (is_array($assetid)) {
			$result = $db->getAssoc($sql.$where, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
		} else {
			$result = $db->getCol($sql.$where);
		}
		assert_valid_db_result($result);

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (is_array($linkid)) {
			$sql = 'SELECT linkid, treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ';
			$where = 'WHERE linkid IN ('.implode(', ', $linkid).')';
		} else {
			$sql = 'SELECT treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ';
			$where = 'WHERE linkid  = '.$db->quoteSmart($linkid);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		if (is_array($linkid)) {
			$result = $db->getAssoc($sql.$where, FALSE, Array(), DB_FETCHMODE_ASSOC);
		} else {
			$result = $db->getCol($sql.$where);
		}
		assert_valid_db_result($result);

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
		$trash =& $this->getSystemAsset('trash_folder');

		// if we are being asked "Is the trash in the trash"
		// lets say "Are you kidding?"
		if ($assetid == $trash->id) return FALSE;

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// we need to work out the treeid of the trash
		$sub_sql = 'SELECT (t.treeid || '.$db->quote('%').')
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid ';
		$sub_where = 'l.minorid = '.$db->quote($trash->id);
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
		$sub_sql .= $sub_where;
		$sub_sql = $db->modifyLimitQuery($sub_sql, 0, 1);

		$sql   = 'SELECT COUNT(*)
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid';
		$where = 'l.minorid = '.$db->quote($assetid).'
				  AND t.treeid '.(($exclusively) ? 'NOT ' : '').'LIKE ('.$sub_sql.')';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$result = $db->getOne($sql.$where);
		assert_valid_db_result($result);

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
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$id_parts = explode(':', $assetid);

		// shadow asset found, pipe the request off to the bridge
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset =& $this->getAsset($real_assetid);
			$results = $asset->countLinks($assetid, $side_of_link, $link_types, $type_code, $strict_type_code, $ignore_linkid);

			$this->forgetAsset($asset);
			return $results;
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$extra_table = '';
		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid);
		if ($link_types) {
			$where .= ' AND '.db_extras_bitand($db, 'link_type', $link_types).' > 0';
		}
		if ($type_code) {
			$extra_table .= ', '.SQ_TABLE_RUNNING_PREFIX.'ast a';
			$where .= ' AND l.minorid = a.assetid ';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$type_code_cond = '';
			if (is_array($type_code)) {
				for ($i = 0; $i < count($type_code); $i++) {
					$type_code[$i] = $db->quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
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
			$where .= ' AND l.linkid <> '.$db->quote($ignore_linkid);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l '.$extra_table.'
				'.$where;

		$result = $db->getOne($sql);
		assert_valid_db_result($result);

		return $result;

	}//end countLinks()


	/**
	* Create a link between the two supplied assets
	*
	* It returns the linkid of the created link
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		// Initial Checks
		if (!$major->id) return 0;
		assert_is_a($minor, 'asset');
		if (!is_null($sort_order)) {
			$sort_order = (int) $sort_order;
		}

		assert_false(!($link_type & SQ_SC_LINK_SIGNIFICANT) && $dependant, 'In order for a link to be dependant it must also be a significant link');

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
				$bridge =& $this->getAsset($majorid_parts[0]);
				$linkid = $bridge->createAssetLink($major, $minor, $link_type, $value, $sort_order, $dependant, $exclusive, $moving);
				$this->forgetAsset($bridge);
			} else {
				// we are linking a shadow asset under a normal asset
				$linkid = $this->createShadowAssetLink($major, $minor, $link_type, $value);
			}
			return $linkid;
		}// end if shadow asset


		// Use Master DB since we are going to change stuff
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

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
		$current_links = $this->getLinks($major->id, $link_type, $minor->type());
		$num_curr_links = count($current_links);
		for ($i = 0; $i < $num_curr_links; $i++) {
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				// The link we are creating already exists, so throw an error
				// unless the major is the trash, in which case we just pretend we linked ok
				if ($this->getSystemAssetid('trash_folder') != $major->id) {
					trigger_localised_error('SYS0192', E_USER_WARNING);
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return $current_links[$i]['linkid'];
			}
		}//end for

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
			$lock_info = @$this->getLockInfo($major->id, 'links');
			$parent_was_locked = !empty($lock_info);
			if (!$this->acquireLock($major->id, 'links')) {
				trigger_localised_error('CORE0012', E_USER_WARNING, $major->name);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return 0;
			}
		}

		// Change to DB to deal with the link tables
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {

			// This is a significant link

			// Check we aren't moving the minor asset under itself
			$sub_sql = 'SELECT
							linkid
						FROM
							sq_ast_lnk
						WHERE
							minorid = '.$db->quoteSmart($minor->id);

			$sql = 'SELECT
						COUNT(DISTINCT ct.linkid)
					FROM sq_ast_lnk_tree pt,
						sq_ast_lnk_tree ct INNER JOIN sq_ast_lnk cl ON ct.linkid = cl.linkid
					WHERE
							ct.treeid LIKE (pt.treeid || '.$db->quoteSmart('%').')
						AND	ct.treeid >= pt.treeid
						AND	pt.linkid IN ('.$sub_sql.')
						AND	cl.minorid = '.$db->quoteSmart($major->id);

			$moving_under = $db->getOne($sql);
			assert_valid_db_result($moving_under);

			if ($moving_under) {
				trigger_localised_error('CORE0114', E_USER_WARNING, $minor->name, $minor->id, $major->name, $major->id);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); //end db3
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); //end db2
				return 0;
			}

			// Check that the major is linked somewhere
			$sql   = 'SELECT
							t.treeid
						FROM
							sq_ast_lnk_tree t
							INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid
						WHERE
							l.minorid = '.$db->quoteSmart($major->id);

			$sql = $db->modifyLimitQuery($sql, 0, 1);
			$existing_treeid = $db->getOne($sql);
			assert_valid_db_result($existing_treeid);
			$existing_treeid = (string) $existing_treeid;

			// if we aren't linked anywhere we can't be linked to, sorry
			// NOTE: exception to rule is root folder
			if ((string) $existing_treeid == '-' && get_class($major) != 'root_folder') {
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
					majorid = '.$db->quoteSmart($major->id);
		$row = $db->getRow($sql);
		assert_valid_db_result($row);
		$max = ($row['count'] > 0) ? (int) $row['max'] + 1 : 0;
		if (is_null($sort_order) || (int) $sort_order > (int) $max || (int) $sort_order < 0) {
			$sort_order = (int) $max;
		}

		// Get Link ID
		$linkid = $db->nextId('sq_ast_lnk');
		assert_valid_db_result($linkid);

		// Update sort order for other children of the new parent
		$sql = 'UPDATE
					sq_ast_lnk
				SET
					sort_order = sort_order + 1
				WHERE
						majorid		= '.$db->quoteSmart($major->id).'
					AND	sort_order	>= '.$db->quoteSmart($sort_order);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$result = $db->query($sql);
		assert_valid_db_result($result);
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
						updated_userid
					)
					VALUES
					(
						'.$db->quoteSmart($linkid).',
						'.$db->quoteSmart($major->id).',
						'.$db->quoteSmart($minor->id).',
						'.$db->quoteSmart($link_type).',
						'.$db->quoteSmart($value).',
						'.$db->quoteSmart($sort_order).',
						'.$db->quoteSmart((($dependant) ? '1' : '0')).',
						'.$db->quoteSmart((($exclusive) ? '1' : '0')).',
						'.db_extras_todate($db, ts_iso8601(time())).',
						'.$db->quoteSmart($GLOBALS['SQ_SYSTEM']->currentUserId()).'
					)';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$result = $db->query($sql);
		
		assert_valid_db_result($result);
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
							l.minorid = '.$db->quoteSmart($minor->id);

			$sql = $db->modifyLimitQuery($sql, 0, 1);
			$minor_tree = $db->getRow($sql);
			assert_valid_db_result($minor_tree);

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
								ct.treeid LIKE '.$db->quoteSmart($existing_treeid.'%').'
							AND	ct.treeid > '.$db->quoteSmart($existing_treeid).'
							AND	LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE);
				} else {
					$sql .= ' WHERE LENGTH(ct.treeid) = '.SQ_CONF_ASSET_TREE_SIZE;
				}
				$sql .= ' AND linkid > 0 ORDER BY treeid DESC';

				$sql = $db->modifyLimitQuery($sql, 0, 1);
				$last_treeid = $db->getOne($sql);
				assert_valid_db_result($last_treeid);

				$treeid = ($existing_treeid == '-') ? '' : $existing_treeid;

				// if we couldn't find any treeids on this branch, then we can just
				// do a straight inert and break the loop as we are done

				if (empty($last_treeid)) {
					$sql = 'asset_link_treeid_convert('.$db->quoteSmart('0').', '.$db->quoteSmart('1').', '.SQ_CONF_ASSET_TREE_BASE.', '.SQ_CONF_ASSET_TREE_SIZE.')';
					$free_treeid = db_get_function_result($db, $sql);

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
									'.$db->quoteSmart($new_treeid).',
									'.$linkid.',
									0
								)';

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$result = $db->query($sql);
					assert_valid_db_result($result);
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

					break;

				}//end no existing treeid

				// otherwise we could find a treeid on this branch, so increment it
				// to get a free childid that we can use for our new link
				$sql = 'asset_link_treeid_convert('.$db->quoteSmart($last_treeid).', '.$db->quoteSmart('0').', '.SQ_CONF_ASSET_TREE_BASE.', '.SQ_CONF_ASSET_TREE_SIZE.')';
				$child_num = db_get_function_result($db, $sql);

				$sql = 'asset_link_treeid_convert('.$db->quoteSmart((string) ($child_num + 1)).', '.$db->quoteSmart('1').', '.SQ_CONF_ASSET_TREE_BASE.', '.SQ_CONF_ASSET_TREE_SIZE.')';
				$free_treeid = db_get_function_result($db, $sql);

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
								num_kids,
								linkid
							)
							VALUES
							(
								'.$db->quoteSmart($treeid.$free_treeid.':'.$linkid).',
								0,
								0
							)';

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				// now try to select the last treeid from this branch , ordering
				// by treeid. If a process got in before us, we will select it as
				// the order by will cause it to come first as its linkid will be smaller

				$sql = 'SELECT
							treeid
						FROM
							sq_ast_lnk_tree
						WHERE
							(treeid LIKE '.$db->quoteSmart($treeid.$free_treeid.':%').'
						OR	treeid = '.$db->quoteSmart($treeid.$free_treeid).')
						AND	num_kids = 0 ORDER BY treeid DESC';

				$sql = $db->modifyLimitQuery($sql, 0, 1);
				$found_treeid = $db->getOne($sql);
				assert_valid_db_result($found_treeid);

				if ($found_treeid == $treeid.$free_treeid.':'.$linkid) {

					// if the found treeid was like the one that we just inserted, then
					// we MIGHT have a winner. The reason that I say might is because
					// someone else might have thought that they have won between the
					// time of the last select to now

					$sql = 'UPDATE
								sq_ast_lnk_tree
							SET
								linkid = -'.$linkid.'
							WHERE
								treeid LIKE '.$db->quoteSmart($treeid.$free_treeid.':%').'
							AND
								linkid = 0
							AND NOT EXISTS(
								SELECT
									1
								FROM
									sq_ast_lnk_tree
								WHERE
									(treeid LIKE '.$db->quoteSmart($treeid.$free_treeid.':%').'
								OR	treeid = '.$db->quoteSmart($treeid.$free_treeid).')
								AND linkid <> 0)';

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$result = $db->query($sql);
					assert_valid_db_result($result);
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$aff_rows = $db->affectedRows();
					assert_valid_db_result($aff_rows);

					// if we could update the linkid to our linkid
					// then we have won this treeid
					if ($aff_rows != 0) {
						$requires_update = TRUE;
						break;
					}
				}//end if

				// otherwise we lost this treeid and will have to do another
				// iterator to try and win a different one
				$sql = 'DELETE FROM
							sq_ast_lnk_tree
						WHERE
							treeid = '.$db->quoteSmart($treeid.$free_treeid.':'.$linkid);

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				$i++;
			// only try to do this 50 times. this will only be exceeded if the number
			// of concurrent requests to create a link under this branch exceeds about 100,
			// so its very unlikely that this will occur
			} while ($i < 50);

			if ($i >= 50) {
				trigger_localised_error('SYS0310', E_USER_ERROR);
			}

			if ($requires_update) {

				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							treeid = '.$db->quoteSmart($treeid.$free_treeid).',
							linkid = '.$linkid.'
						WHERE
							treeid = '.$db->quoteSmart($treeid.$free_treeid.':'.$linkid);

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}


			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '-') {
				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							linkid   = '.$db->quoteSmart($linkid).',
							num_kids = '.$db->quoteSmart($minor_tree['num_kids']).'
						WHERE
							treeid = '.$db->quoteSmart($free_treeid);

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			} else {
				$sql = 'UPDATE
							sq_ast_lnk_tree
						SET
							linkid   = '.$db->quoteSmart($linkid).',
							num_kids = '.$db->quoteSmart($minor_tree['num_kids']).'
						WHERE
							treeid = '.$db->quoteSmart($existing_treeid.$free_treeid);

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
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
								(t.treeid || '.$db->quoteSmart($free_treeid).'),
								'.$db->quoteSmart($linkid).',
								'.$db->quoteSmart($minor_tree['num_kids']).'
							FROM
											sq_ast_lnk_tree t
								INNER JOIN	sq_ast_lnk l ON t.linkid = l.linkid
							WHERE
								l.minorid = '.$db->quoteSmart($major->id).'
							AND treeid || '.$db->quoteSmart($free_treeid).' <> '.$db->quoteSmart($existing_treeid.$free_treeid);

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
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
									LENGTH(SUBSTR(t.treeid, 1, (LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))) != 0
								THEN
									SUBSTR(t.treeid, 1, (LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
								ELSE
									'.$db->quoteSmart('-').'
								END
							FROM
								sq_ast_lnk_tree t
							WHERE
								t.linkid = '.$db->quoteSmart($linkid).'
						)';

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$result = $db->query($sql);
			assert_valid_db_result($result);
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
								('.$case.') || '.$db->quoteSmart($free_treeid).' || SUBSTR(ct.treeid, '.$db->quoteSmart(strlen($minor_tree['treeid']) + 1).'),
								ct.linkid,
								ct.num_kids
							FROM
											sq_ast_lnk_tree pt
								INNER JOIN	sq_ast_lnk pl ON pt.linkid = pl.linkid,
											sq_ast_lnk_tree ct
							WHERE
									pl.minorid	= '.$db->quoteSmart($major->id).'
								AND	ct.treeid	LIKE '.$db->quoteSmart($minor_tree['treeid'].'%').'
								AND	ct.treeid	> '.$db->quoteSmart($minor_tree['treeid']);

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$result = $db->query($sql);
				assert_valid_db_result($result);
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}//end if


		}//end if significant link

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); /******** ENd DB3 ************/

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
		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
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
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
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
	* @param object	&$major		the major asset that we are linking from
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param string	$value		the value that is to be associated with this link
	*
	* @return int
	* @access public
	*/
	function createShadowAssetLink(&$major, &$minor, $link_type, $value='')
	{
		// we are linking a shadow asset under a normal asset
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		// use the sq_ast_lnk sequence so that when we query any link view later
		// on we don't have any conflicting linkids
		$linkid = $db->nextId('sq_ast_lnk');
		assert_valid_db_result($linkid);

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
						'.$db->quoteSmart($linkid).',
						'.$db->quoteSmart($major->id).',
						'.$db->quoteSmart($minor->id).',
						'.$db->quoteSmart($link_type).',
						'.$db->quoteSmart($value).',
						'.db_extras_todate($db, ts_iso8601(time())).',
						'.$db->quoteSmart($GLOBALS['SQ_SYSTEM']->currentUserid()).'
					)';
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$this->forgetAsset($major);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $linkid;

	}//end createShadowAssetLink()


	/**
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param object	&$major		the major asset in the proposed link
	* @param object	&$minor		the minor asset in the proposed link
	* @param string	$link_type	the type of link that is proposed
	* @param int	$exclusive	(0|1) whether this will be the only significant link
	*							with $minor as the minor asset
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$major, &$minor, $link_type, $exclusive)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		if (!$major->id) return FALSE;

		if (!is_a($minor, 'Asset')) {
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

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
			$where = 'minorid = '.$db->quote($minor->id).'
						AND '.db_extras_bitand($db, 'link_type', SQ_SC_LINK_SIGNIFICANT).' > 0';
			if (!$exclusive) {
				$where .= ' AND is_exclusive = '.$db->quote('1');
			}
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where;

			$current_majorid = $db->getOne($sql);
			if (!assert_valid_db_result($current_majorid, '', TRUE, FALSE)) {
				return translate('database_error');
			}

			if ($current_majorid) {
				$current_major =& $this->getAsset($current_majorid);
				return translate('cannot_create_link_exclusive_link', $minor->name, $minor->id, $major->name, $major->id, $current_major->name, $current_majorid);
			}

		}//end if

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end canCreateLink()


	/**
	* Returns TRUE if the supplied asset can have assets of the specified type linked under it
	*
	* @param object	&$major			the major asset that we are linking to
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
	function canLinkToType(&$major, $type_code, $link_type, $ignore_linkid=0, $exclusive=0)
	{
		// type checking
		if (!is_a($major, 'asset')) {
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

		if (empty($allowed_links[$link_type][$type])) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return translate('asset_type_cannot_be_linktype_linked_to_type', $type_code, link_type_name($link_type), $major->type());
		}

		if (!($link_type & SQ_SC_LINK_SIGNIFICANT) && $exclusive) {
			return translate('exclusive_links_must_be_significant');
		}

		if (!$exclusive && !empty($allowed_links[$link_type][$type]['exclusive'])) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return translate('asset_type_must_be_linktype_exclusively_linked_to_type', $type_code, link_type_name($link_type), $major->type());
		}

		//// Now we need to check that the cardinality is kept valid ////

		// if we are only allowed up to a certain number of these links
		if ($allowed_links[$link_type][$type]['card'] != 'M' && $major->id) {
			$num_curr_links = $this->countLinks($major->id, 'major', $link_type, $type, TRUE, $ignore_linkid);
			//  and we already have our quota of links
			if ($num_curr_links >= (int) $allowed_links[$link_type][$type]['card']) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				return translate('asset_type_can_have_only_linktypes_links_to_type', $this->getTypeInfo($major->type(), 'name'), ((int) $allowed_links[$link_type][$type]['card']), link_type_name($link_type), $type);
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
	*
	* @return int
	* @access public
	*/
	function moveLink($linkid, $to_parentid, $link_type, $to_parent_pos)
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
			trigger_localised_error('SYS1038', E_USER_WARNING, $linkid);
			return 0;
		}

		$old_parent =& $this->getAsset($link['majorid'], $link['major_type_code']);
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
		$new_parent =& $this->getAsset($to_parentid, '', TRUE);
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
		$minor =& $this->getAsset($link['minorid'], $link['minor_type_code']);
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
		$new_linkid = $new_parent->createLink($minor, $link_type, '', $to_parent_pos, '0', '0', TRUE);
		if (!$new_linkid) $success = FALSE;

		// let's try and delete the old link
		if ($success) {
			$success = $old_parent->deleteLink($linkid);
		}

		if ($success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL)
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

			$bridge =& $this->getAsset($id_parts[0]);
			$result =  $bridge->updateLink($linkid, $link_type, $value, $sort_order);
			$this->forgetAsset($bridge);
			return $result;
		}


		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$set_clauses = Array();

		$link_type_changed	= (!is_null($link_type)		&& $link['link_type']	!= $link_type	);
		$value_changed		= (!is_null($value)			&& $link['value']		!= $value		);
		$sort_order_changed	= (!is_null($sort_order)	&& $link['sort_order']	!= $sort_order	);


		$link_type  = (int) $link_type;
		$sort_order = (int) $sort_order;

		$major =& $this->getAsset((int) $link['majorid'], $link['major_type_code']);
		$minor =& $this->getAsset((int) $link['minorid'], $link['minor_type_code']);

		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
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

			$set_clauses['link_type'] = $db->quoteSmart($link_type);

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

			$set_clauses['value'] = $db->quoteSmart($value);

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

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		// If the minor is a shadow asset, we need to update the shadow link table instead
		// There's no order field in the shadow table, so we branch before we add order to the set clause
		if (isset($minorid_parts[1])) {

			if (empty($set_clauses)) {
				$ms->abortLog();
				return TRUE;
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$set_clauses['updated_userid']	= $db->quoteSmart($GLOBALS['SQ_SYSTEM']->currentUserId());
			$set_clauses['updated']			= db_extras_todate($db, ts_iso8601(time()));

			$set_array = Array();

			foreach ($set_clauses as $key => $value) {
				$set_array[] = $key.'='.$value;
			}

			$set_string = implode(',', $set_array);

			$sql = 'UPDATE
						sq_shdw_ast_lnk
					SET
						'.$set_string.'
					WHERE
							linkid	= '.$db->quoteSmart($linkid).'
						AND	majorid	= '.$db->quoteSmart($major->id);
			$result = $db->query($sql);
			assert_valid_db_result($result);

			$this->forgetAsset($major);

		} else {

			if ($sort_order_changed) {

				$sql = 'SELECT COUNT(*) AS count, MAX(sort_order) AS max
						FROM
							sq_ast_lnk
						WHERE
							majorid = '.$db->quoteSmart($major->id);

				$row = $db->getRow($sql);
				assert_valid_db_result($row);

				$max = ($row['count'] > 0) ? (int) $row['max'] : 0;
				if ($sort_order > $max || $sort_order < 0) {
					$sort_order = $max;
				}

				$set_clauses['sort_order'] = $db->quoteSmart($sort_order);

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
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$set_clauses['updated_userid']	= $db->quoteSmart($GLOBALS['SQ_SYSTEM']->currentUserId());
			$set_clauses['updated']			= db_extras_todate($db, ts_iso8601(time()));

			$set_array = Array();

			foreach ($set_clauses as $key => $value) {
				$set_array[] = $key.'='.$value;
			}

			$set_string = implode(',', $set_array);

			$sql = 'UPDATE
						sq_ast_lnk
					SET
						'.$set_string.'
					WHERE
							linkid	= '.$db->quoteSmart($linkid).'
						AND	majorid	= '.$db->quoteSmart($major->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			if ($sort_order_changed) {
				// move 'em up, higher
				if ($link['sort_order'] > $sort_order) {

					$sql = 'UPDATE
								sq_ast_lnk
							SET
								sort_order = sort_order + 1
							WHERE
									majorid		= '.$db->quoteSmart($major->id).'
								AND	linkid		<> '.$db->quoteSmart($linkid).'
								AND	sort_order	>= '.$db->quoteSmart($sort_order).'
								AND	sort_order	<= '.$db->quoteSmart($link['sort_order']);

					$result = $db->query($sql);
					assert_valid_db_result($result);

				} else {

					$sql = 'UPDATE
								sq_ast_lnk
							SET
								sort_order = sort_order - 1
							WHERE
									majorid		= '.$db->quoteSmart($major->id).'
								AND	linkid		<> '.$db->quoteSmart($linkid).'
								AND	sort_order	>= '.$db->quoteSmart($link['sort_order']).'
								AND	sort_order	<= '.$db->quoteSmart($sort_order);

					$result = $db->query($sql);
					assert_valid_db_result($result);
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
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid)
	{
		if ($linkid == 0) return FALSE;
		// before anything else is done, check if this is a shadow link, i.e. a link that is managed by a bridge
		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1])) {
			$bridge =& $this->getAsset($id_parts[0]);
			$result = $bridge->deleteAssetLink($linkid);
			$this->forgetAsset($bridge);
			return $result;
		}

		// Use the master DB since we are changing things
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_localised_error('SYS0137', E_USER_WARNING, $linkid);
			return FALSE;
		}

		$major =& $this->getAsset($link['majorid'], $link['major_type_code']);
		$minor =& $this->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($major) || is_null($minor)) {
			return FALSE;
		}

		// Acquire some locks if necessary
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// Keep a record of the locking state beforehand so we can leave it as we found it
			$lock_info = @$this->getLockInfo($major->id, 'links');
			$parent_was_locked = !empty($lock_info);
			if (!$this->acquireLock($major->id, 'links')) {
				trigger_localised_error('SYS0126', E_USER_WARNING, 'major', $major->name);
				return FALSE;
			}
			// To check if we can delete this link, we need to lock current asset
			$minor_lock_info = @$this->getLockInfo($minor->id, 'links');
			$asset_was_locked = !empty($minor_lock_info);
			if (!$this->acquireLock($minor->id, 'links')) {
				trigger_localised_error('SYS0126', E_USER_WARNING, 'minor', $minor->name);
				return FALSE;
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
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'DELETE FROM
						sq_shdw_ast_lnk
					WHERE
						linkid = '.$db->quoteSmart($linkid);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;

		}//end if shadow asset

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

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

					$trash_folder =& $this->getSystemAsset('trash_folder');
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
									'.$db->quoteSmart('-').'
								END
							FROM
								sq_ast_lnk_tree t
							WHERE
								t.linkid = '.$db->quoteSmart($linkid).'
						)';

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$result = $db->query($sql);
			assert_valid_db_result($result);
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
									pt.linkid	= '.$db->quoteSmart($linkid).'
								AND	(ct.treeid	LIKE pt.treeid || '.$db->quoteSmart('%').'
								OR ct.treeid	= pt.treeid)
						)';

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$result = $db->query($sql);
			assert_valid_db_result($result);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}//end if significant link

		// Update sort orders of other children of this parent
		$sql = 'UPDATE
					sq_ast_lnk
				SET
					sort_order = sort_order - 1
				WHERE
						majorid		= '.$db->quoteSmart($major->id).'
					AND	sort_order	> '.$db->quoteSmart($link['sort_order']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$result = $db->query($sql);
		assert_valid_db_result($result);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// Delete from the link table
		$sql = 'DELETE FROM
					sq_ast_lnk
				WHERE
						linkid	= '.$db->quoteSmart($linkid).'
					AND	majorid	= '.$db->quoteSmart($major->id);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$result = $db->query($sql);
		assert_valid_db_result($result);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// Tell the assets that they have been updated
		$major->linksUpdated();
		$minor->linksUpdated();

		// Release locks on parent and/or asset if we acquired them
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			if (!$parent_was_locked) {
				$this->releaseLock($major->id, 'links'); // we don't care if it fails, it's too late to reverse anything
			}
			if (!$asset_was_locked) {
				$this->releaseLock($minor->id, 'links');
			}
		}

		// Send Internal Message
		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
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
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
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

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection(); // finished with db2

		return TRUE;

	}//end deleteAssetLink()


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
		$sql = 'DELETE FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk
					WHERE linkid = '.$GLOBALS['SQ_SYSTEM']->db->quoteSmart($linkid);

		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);
		assert_valid_db_result($result);

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
	function couldTrashAsset($assetid)
	{
		if (!$assetid) return FALSE;
		// We can trash the asset if we can delete all the parent links
		$msgs = Array();
		$parent_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', TRUE, 'minor');
		foreach ($parent_links as $id => $link_details) {
			$asset =& $this->getAsset($link_details['majorid']);
			if (TRUE !== ($msg = $asset->canDeleteLink($link_details['linkid']))) {
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
		$asset =& $this->getAsset($assetid);

		$trash_errors = Array();

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
		// generate the get children query
		$children = $this->generateGetChildrenQuery($asset, '', TRUE, FALSE);
		if (!empty($children)) {
			// modify the selection so that we only get 1 column
			$children['select'] = '(SELECT DISTINCT l.minorid';
			if (isset($children['union_select'])) {
				$children['union_select'] = 'UNION ALL SELECT sl.minorid';
			}
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT
						majorid, minorid, name, linkid, status, link_type
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk lnk
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast ast ON (ast.assetid = lnk.minorid)
					WHERE
						(
							(link_type = '.$db->quoteSmart(SQ_LINK_TYPE_3).' OR link_type = '.$db->quoteSmart(SQ_LINK_NOTICE).') OR
							(status >= '.$db->quoteSmart(SQ_STATUS_LIVE).')
						) AND
						assetid IN ('.implode(' ', $children).')';
			$result = $db->getAll($sql);
			assert_valid_db_result($result);
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
	* @param string	$assetid	the asset to trash
	*
	* @return boolean
	* @access public
	*/
	function trashAsset($assetid)
	{
		if ($this->assetInTrash($assetid, TRUE)) {
			// already in trash exclusively
			return TRUE;
		}

		// trash requests are charged at $4.95 a minute
		// please ask your parents for permission before trashing
		$parent_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', TRUE, 'minor');
		$locked = Array();
		foreach ($parent_links as $link_details) {
			$asset =& $this->getAsset($link_details['majorid']);
			// try to acquire the lock
			if ($this->acquireLock($link_details['majorid'], 'links')) {
				$locked[] = $link_details['majorid'];
			} else {
				trigger_localised_error('SYS0119', E_USER_WARNING, $assetid, $link_details['majorid']);
				foreach ($locked as $aid) {
					$this->releaseLock($aid, 'links');
				}
			}
			if (TRUE !== ($msg = $asset->canDeleteLink($link_details['linkid']))) {
				trigger_localised_error('SYS0120', E_USER_WARNING, $assetid, $link_details['majorid'], $msg);
				foreach ($locked as $aid) {
					$this->releaseLock($aid, 'links');
				}
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$trashid = $this->getSystemAssetid('trash_folder');
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
			$asset =& $this->getAsset($real_assetid);

			if (is_null($asset)) continue;

			$ret_val = $asset->getPermission($assetid, $permission, $granted, $and_greater, $expand_groups, $all_info);
			$this->forgetAsset($asset);

			return $ret_val;
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

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$table = ($collapse_roles) ? 'ast_perm' : 'vw_ast_perm';
			$sql = '  SELECT DISTINCT assetid, userid, granted
					  FROM '.SQ_TABLE_RUNNING_PREFIX.$table.' ';
			$where = 'assetid = '.$db->quote($assetid).'
					  AND permission '.(($and_greater) ? '>= ' : '= ').$db->quote($permission);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			assert_valid_db_result($result);

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
					$user =& $this->getAsset($data['userid']);
					if (!is_null($user)) {
						if (is_a($user, 'user_group')) {
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
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$granted	whether this permission grants access or not
	* @param boolean	$force_set	if TRUE, existing permissions will be overridden even if the new permissions are found to conflict with them
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted, $force_set=FALSE)
	{
		$permission = (int)  $permission;
		$granted    = (bool) $granted;
		$db_action  = 'insert';

		// check if we are getting links for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset =& $this->getAsset($real_assetid);

			$ret_val = NULL;
			if (is_null($asset)) continue;
			if (method_exists($asset, 'setPermission')) {
				$ret_val = $asset->setPermission($assetid, $userid, $permission, $granted);
				$this->forgetAsset($asset);
			}

			return $ret_val;
		}

		$asset =& $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('permissions')) {
			trigger_localised_error('SYS0111', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// check if this permission is already set
		$current = $this->getPermission($assetid, $permission, $granted, FALSE, FALSE, FALSE, TRUE);
		if (in_array($userid, array_values($current))) {
			return TRUE;
		}

		// if we dont have an userid, we are granting public access
		if (!empty($userid)) {
			// check that the passed userid is a user or user_group
			$user =& $this->getAsset($userid, '', TRUE);
			if (!$user->id) {
				trigger_localised_error('SYS0112', E_USER_WARNING);
				return FALSE;
			} else if (!is_a($user, 'user') && !is_a($user, 'user_group')) {
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		switch ($db_action) {
			case 'insert':
				$sql = 'INSERT INTO
							sq_ast_perm
							(
								assetid,
								userid,
								permission,
								granted
							)
							VALUES
							(
								'.$db->quoteSmart($assetid).',
								'.$db->quoteSmart($userid).',
								'.$db->quoteSmart($permission).',
								'.$db->quoteSmart((($granted) ? '1' : '0')).'
							)';

			break;
			case 'update':
				$sql = 'UPDATE
							sq_ast_perm
						SET
							granted = '.$db->quoteSmart(($granted) ? '1' : '0').'
						WHERE
							assetid = '.$db->quoteSmart($assetid).'
						AND
							userid  = '.$db->quoteSmart($userid).'
						AND
							permission = '.$db->quoteSmart($permission);

			break;
		}//end switch

		$result = $db->query($sql);
		assert_valid_db_result($result);

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
		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'perm_name'		=> permission_type_name($permission),
						'asset_name'	=> $asset->name,
						'user_name'		=> $user_name,
					);
		$message = $ms->newMessage(Array(), 'asset.permissions.'.($granted ? 'grant' : 'deny'), $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$message->send();

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
			$asset =& $this->getAsset($real_assetid);

			if (is_null($asset)) continue;

			$ret_val = $asset->deletePermission($assetid, $userid, $permission);
			$this->forgetAsset($asset);

			return $ret_val;
		}

		$asset =& $this->getAsset($assetid);
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_ast_perm
				WHERE
						userid		= '.$db->quoteSmart($userid).'
					AND	permission	= '.$db->quoteSmart($permission).'
					AND	assetid		= '.$db->quoteSmart($assetid);


		$result = $db->query($sql);
		assert_valid_db_result($result);

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
		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'perm_name'		=> permission_type_name($permission),
						'asset_name'	=> $asset->name,
						'user_name'		=> $user_name,
					);
		$message = $ms->newMessage(Array(), 'asset.permissions.delete', $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$message->send();

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
	*
	* @return array
	* @access public
	*/
	function getRole($assetid=NULL, $roleid=NULL, $userid=NULL, $include_assetid=FALSE, $include_globals=FALSE, $expand_groups=FALSE, $inc_dependants=TRUE)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$where = Array();
		if (!is_null($assetid)) {
			$where[] = 'r.assetid = '.$db->quoteSmart((string) $assetid);
		}

		if (!is_null($roleid)) {
			$where[] = 'r.roleid = '.$db->quoteSmart((string) $roleid);
		}

		if (!is_null($userid)) {
			$where[] = 'r.userid = '.$db->quoteSmart((string) $userid);
		}

		$where_clause = '';
		if (!empty($where)) {
			$where_clause = 'WHERE '.implode(' AND ', $where);
		}

		$table = ($include_globals) ? 'vw_ast_role r' : 'ast_role r';
		$additional = $include_assetid ? ', r.assetid' : '';

		if (!$inc_dependants) {
			// join to link table to get the non dependant assets
			$where_clause .= ' AND r.assetid = l.minorid AND l.is_dependant = \'0\'';
			$table .= ', '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l';
			$where_clause = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, 'l');
		}
		$where_clause = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, 'r');
		$sql = 'SELECT DISTINCT r.roleid, r.userid'.$additional.'
				FROM '.SQ_TABLE_RUNNING_PREFIX.$table.' '.$where_clause;


		$result = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
		assert_valid_db_result($result);

		if ($include_assetid) {
			$roles = Array();
			foreach ($result as $roleid => $userid_assetids) {
				foreach ($userid_assetids as $index => $userid_assetid) {
					if ($userid_assetid == '0') continue;
					$assetid = $userid_assetid['assetid'];
					$userid =$userid_assetid['userid'];
					$roles[$roleid][$userid][] = $assetid;
				}
			}
			$result = $roles;
		}

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
		// check if we have permission to set roles
		$asset =& $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('roles')) {
			trigger_localised_error('SYS0313', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// check if this role is already set
		$current = $this->getRole($assetid, $roleid, $userid);
		if (in_array($userid, array_get_index($current, $roleid, Array()))) {
			return TRUE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'INSERT INTO sq_ast_role
				(
					assetid,
					roleid,
					userid
				)
				VALUES
				(
					'.$db->quoteSmart($assetid).',
					'.$db->quoteSmart($roleid).',
					'.$db->quoteSmart($userid).'
				)';

		$result = $db->query($sql);
		assert_valid_db_result($result);

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
		// check if we have permission to set roles
		$asset =& $this->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		if (!$asset->adminAccess('roles')) {
			trigger_localised_error('SYS0313', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$where_and = '';
		if (!is_null($roleid)) {
			$where_and .= ' AND roleid = '.$db->quoteSmart((string) $roleid);
		}

		if (!is_null($userid)) {
			$where_and .= ' AND userid = '.$db->quoteSmart((string) $userid);
		}

		$sql = 'DELETE FROM sq_ast_role
				WHERE assetid = '.$db->quoteSmart($assetid).$where_and;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$result = $db->query($sql);
		assert_valid_db_result($result);

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

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$url_condition = '';
		if ($exact_match) {
			$url_condition = ' = '.$db->quote($url);

		} else {
			$sections = explode('/', $url);
			$option  = '';
			$url_condition = ' IN (';
			foreach ($sections as $piece) {
				$url_condition .= (($option) ? ',' : '').$db->quote($option.$piece);
				$option .= $piece.'/';
			}
			$url_condition .= ') ';
		}//end if

		$where = 'l.url '.$url_condition;

		if (!is_null($protocol)) {
			$where .= ' AND l.'.$protocol.' = '.$db->quote(1);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT l.url, l.assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
				'.$where.'
				ORDER BY LENGTH(l.url) DESC';
		$sql = $db->modifyLimitQuery($sql, 0, 1);

		$row = $db->getRow($sql);
		assert_valid_db_result($row);

		// URL not found
		if (empty($row)) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			if (!$mute_errors) {
				trigger_localised_error('SYS0245', E_USER_WARNING, $fancy_url);
			}
			$null = NULL; // 'cause we are meant to return by reference
			return $null;

		} else {
			$asset =& $this->getAsset($row['assetid'], '', $mute_errors);
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

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// so much for SQL standards...
		$where = $db->quote($url).' LIKE (url || '.$db->quote('%').')';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT urlid, assetid, url, http, https
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_url
				'.$where.'
				ORDER BY url DESC';
		$sql = $db->modifyLimitQuery($sql, 0, 1);
		$result = $db->getRow($sql);
		assert_valid_db_result($result);
		return $result;

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT
					http, https, url
				FROM
					sq_ast_lookup
				WHERE
					assetid = '.$db->quote($assetid);
		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		return $result;

	}//end getURLs()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from the site url
	*
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the urls to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($protocol=NULL, $url=NULL)
	{
		if (is_null($url)) $url = current_url(FALSE, TRUE);

		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') {
			$protocol = 'http';
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$non_standard_url = FALSE;
		// get the root(site) url for this url
		$root_url = $this->getRootURL($url);
		if (empty($root_url)) {
			$found = FALSE;
			$non_standard_url = TRUE;
			// We want to check if this is an asset (file) with a publicly served url
			$fancy_asset =& $this->getAssetFromURL($protocol, $url, TRUE, TRUE);
			if (empty($fancy_asset)) {
				$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
				trigger_localised_error('SYS0247', E_USER_WARNING, $fancy_url);
				return Array();
			}
			$lineages = $this->getLinkLineages($fancy_asset->id);

			// Determine which site we are in, and find one of our lineages that lies in that site
			$all_sites = $this->getParents($fancy_asset->id, 'site');
			$valid_sites = Array();
			foreach ($all_sites as $id => $type_code) {
				$current_site =& $this->getAsset($id);
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
			$parent =& $this->getAsset($link['majorid']);
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

		// the get_lineage_from_url procedure takes a url and splits it into the
		// possible urls that we are looking for.
		// we do this because its faster than performing a 'url LIKE l.url%'
		$in_sql = 'sq_get_lineage_from_url('.$db->quoteSmart($url.'/').')';

		// if we are oracle type-cast the function result to a table
		if ($db->phptype == 'oci8') {
			$in_sql = 'SELECT * FROM table('.$in_sql.')';
			$result = $db->getCol($in_sql);
			assert_valid_db_result($result);

			if (empty($result)) return Array();

			for (reset($result); NULL !== ($key = key($result)); next($result)) {
				$result[$key] = $db->quoteSmart($result[$key]);
			}
			$in_sql = implode(',', $result);

		} else {
			$in_sql = 'SELECT * FROM '.$in_sql;
		}
		$where = 'l.root_urlid = '.$db->quote((int) $root_url['urlid']).'
				  AND l.url IN (
								'.$in_sql.'
							)';

		if (!is_null($protocol)) {
			$where .= ' AND l.'.$protocol.' = '.$db->quote('1');
			$protocol_select = $db->quote($protocol);
		} else {
			$curr_proto = current_protocol();
			$protocol_select = 'CASE WHEN l.'.$curr_proto.' = '.$db->quote('1').' THEN '.$db->quote($curr_proto).' ELSE '.$db->quote(($curr_proto == 'http') ? 'https' : 'http').' END';
		}

		// if we are using oracle, we cannot specify an alias
		// in a group by clause. We must use the result set definition
		if ($db->phptype == 'pgsql') {
			$group_by_protocol = 'protocol';
		} else if ($db->phptype == 'oci8') {
			$group_by_protocol = $protocol_select;
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT l.url, l.assetid, a.name, a.short_name, a.type_code, '.$protocol_select.' as protocol, MIN(al.link_type) AS link_type
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
				  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.assetid = a.assetid
				  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk al ON a.assetid = al.minorid
				'.$where.'
				GROUP BY l.url, l.assetid, a.name, a.short_name, a.type_code, '.$group_by_protocol.'
				ORDER BY l.url ASC';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		$url_asset =& $this->getAssetFromURL($protocol, $url);
		$id_parts = explode(':', $url_asset->id);
		if (isset($id_parts[1])) {

			$real_assetid = $id_parts[0];
			$asset =& $this->getAsset($real_assetid);
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
				$site =& $this->getAsset($result[0]['assetid'], $result[0]['type_code']);
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
									'link_type'		=> SQ_LINK_TYPE_3,
									'protocol'		=> current_protocol(),
								);
				}//end if
			}//end if
		}//end if

		// Check whether the asset is in Safe edit. If it is, load the save version of asset.
		// for name and short_name attribute
		foreach ($result as $key => $entry) {
			$asset =& $this->getAsset($entry['assetid']);
			if ($asset->useSystemVersion()) {
				$asset->_loadDataPaths();
				$original_asset = $asset;
				if (!$asset->loadSystemVersion()) {
					trigger_localised_error('SYS0088', E_USER_WARNING, $asset->id);
					$this->forgetAsset($asset);
					return;
				}

				$result[$key]['name']			= $asset->name;
				$result[$key]['short_name']	= $asset->short_name;

				// Restore the original state of the asset - it should be left untouched as we were only after keywords
				$asset = $original_asset;
			}
			$this->forgetAsset($asset);
		}

		return $result;

	}//end getLineageFromURL()


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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT lv.value as designid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON lv.value = a.assetid
				';
		$where ='lv.url = '.$db->quote($url).'
				   AND lv.name = '.$db->quote($design_name);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where;

		$result = $db->getRow($sql);
		assert_valid_db_result($result);
		return $result;

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
		$asset =& $this->getAssetFromURL(NULL, $url, TRUE, TRUE);
		$design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'asset', FALSE, $design_name);

		while (empty($design_link) && ($url != '')) {
			$slash_pos = strrpos($url, '/');
			if ($slash_pos === FALSE) $slash_pos = 0;
			$url = substr_replace($url, '', $slash_pos);

			$asset =& $this->getAssetFromURL(NULL, $url, TRUE, TRUE);
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
	* @param string	$url		the url to get the design for
	* @param string	$value_name	the code for the design (eg. design::system::frontend)
	*
	* @return string
	* @access public
	*/
	function getValueFromURL($url, $value_name)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT lv.value
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
				';

		$where ='l.url = '.$db->quote($url).'
				   AND lv.name = '.$db->quote($value_name);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql .= $where;

		$result = $db->getOne($sql);
		assert_valid_db_result($result);
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.assetid'.($include_treeids ? ', lt.treeid' : '').'
				FROM sq_ast_lookup l
					JOIN sq_ast_lookup_value lv ON l.url = lv.url';
		if ($include_treeids) {
			$sql .= '
					JOIN sq_ast_lnk al ON l.assetid = al.minorid
					JOIN sq_ast_lnk_tree lt ON lt.linkid = al.linkid';
		}
		$sql .= '
				WHERE lv.name '.$name_operator.' '.$db->quoteSmart($lookup_name_exp).'
					AND lv.value '.$value_operator.' '.$db->quoteSmart($lookup_value_exp);
		if ($include_treeids) {
			$res = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
		} else {
			$res = $db->getCol($sql);
		}
		assert_valid_db_result($res);
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
	*
	* @return mixed int|array
	* @access public
	*/
	function getAssetURL($assetids, $base_url=NULL, $ignore_rollback=FALSE)
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
					$asset =& $this->getAsset($assetid);
					$return_urls[$assetid] = $asset->getURL($base_url, TRUE);
				}
				return $return_urls;
			} else {
				$asset =& $this->getAsset($assetids[0]);
				return $asset->getURL($base_url, TRUE);
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

		// work out if any of these are cached
		foreach ($assetids as $assetid) {
			if (!isset($this->_tmp['urls'][$assetid])) {
				$this->_tmp['urls'][$assetid] = Array();
			}

			if (isset($this->_tmp['urls'][$assetid][$base_url])) {
				// use the cached version
				$return_urls[$assetid] = $this->_tmp['urls'][$assetid][$base_url];
			}
		}

		// now hit the database for the rest of them
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$the_rest = array_diff($assetids, array_keys($return_urls));

		if (!empty($the_rest)) {
			$assetid_cond_array = $the_rest;
			for (reset($assetid_cond_array); NULL !== ($k = key($assetid_cond_array)); next($assetid_cond_array)) {
				$assetid_cond_array[$k] = $db->quoteSmart((string) $assetid_cond_array[$k]);
			}

			// IN query needs to be chunked to be Oracle-safe (is also slow
			// in big doses on PostgreSQL). 200 at a time should be fine
			$result = Array();
			foreach (array_chunk($assetid_cond_array, 200) as $chunk) {
				$assetid_cond = 'IN ('.implode(', ', $chunk).')';

				$url_bits = explode('/', preg_replace('/\\/+$/', '', $base_url));

				$where = 'l.assetid '.$assetid_cond;
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
				$sql = 'SELECT l.assetid, l.url,
					CASE WHEN l.'.$protocol.' = '.$db->quote(1).' THEN '.$db->quote($protocol).' ELSE '.$db->quote(($protocol == 'https') ? 'http' : 'https').' END as protocol,
				(
					(CASE WHEN l.'.$protocol.' = '.$db->quote(1).' THEN 1 ELSE 0 END)';

				$str = '';
				foreach (array_values($url_bits) as $bit) {
					$str .= $bit.'/';
					$sql .= ' +
					(CASE WHEN SUBSTR(l.url || '.$db->quote('/').', 1, '.strlen($str).') = '.$db->quote($str).' THEN 1 ELSE 0 END)';
				}

				$sql .= '
				) as weighting
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
				'.$where.'
				ORDER BY weighting DESC, LENGTH(l.url) ASC';

				$partial_result = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
				assert_valid_db_result($partial_result);
				$result += $partial_result;
				unset($partial_result);
			}

			foreach ($the_rest as $assetid) {
				if (!isset($result[$assetid])) {
					$this->_tmp['urls'][$assetid][$base_url] = '';
				} else {
					$this->_tmp['urls'][$assetid][$base_url] = $result[$assetid][0]['protocol'].'://'.$result[$assetid][0]['url'];
				}
				$return_urls[$assetid] = $this->_tmp['urls'][$assetid][$base_url];
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
					$asset =& $this->getAsset($assetid);
					$return_hrefs[$assetid] = $asset->getHref($base_url, TRUE);
				}
				return $return_hrefs;
			} else {
				$asset =& $this->getAsset($assetids[0]);
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
			if (SQ_IN_LIMBO) {
				$href = sq_web_path('edit').'/?asset_ei_screen='.$screen;
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
	* @param object		&$major					the major asset, whose children we are to check
	* @param array		$paths					array of web paths to check
	* @param int		$ignore_assetid			ignore this assetid when checking
	* @param boolean	$return_alternatives	return all paths but replace the bad paths
	*											with a valid alternative
	*
	* @return array
	* @access protected
	*/
	function webPathsInUse(&$major, $paths, $ignore_assetid=0, $return_alternatives=FALSE)
	{
		if (empty($paths)) return Array();
		$return_paths = Array();

		// db_extras_bitand
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		while (!empty($paths)) {
			$paths_string = '';
			foreach ($paths as $path) {
				if (!empty($paths_string)) $paths_string .= ', ';
				$paths_string .= $db->quote($path);
			}

			$sql = 'SELECT p.path
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_path p
					INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
					ON p.assetid = l.minorid ';
			$where = 'WHERE l.majorid = '.$db->quoteSmart((string) $major->id).' ';
			if ($ignore_assetid) {
				$where .= 'AND l.minorid <> '.$db->quoteSmart((string) $ignore_assetid).' ';
			}
			$where .= 'AND p.path IN ('.$paths_string.')
					AND '.db_extras_bitand($db, 'l.link_type', SQ_SC_LINK_WEB_PATHS).' > 0 ';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$used_paths = $db->getCol($sql.$where);
			assert_valid_db_result($used_paths);

			// if we do not need to get alternate we paths, just return
			// the ones that are currently in use
			if (!$return_alternatives) return $used_paths;

			$unused_paths = array_diff($paths, $used_paths);
			$return_paths = array_merge($return_paths, $unused_paths);

			// we now update the paths we are looking for to be the ones we know
			// are currently in use, except we will increment the paths using
			// numerials and check thse new valids to see if they are in use
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$paths = $used_paths;
			for ($i = count($paths) - 1; $i >= 0; $i--) {
				$paths[$i] = increment_filename($paths[$i]);
			}
		}//end while

		return $return_paths;

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
		$edit_fns =& new Asset_Manager_Edit_Fns($this, $backend);
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
	* @param string			$value				The value for all the links must equal this (if not null)
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
	function generateGetLinksQuery($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $value=NULL, $dependant=NULL, $exclusive=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');
		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$assetid_cond = '';
		if (is_array($assetid)) {
			for (reset($assetid); NULL !== ($i = key($assetid)); next($assetid)) {
				assert_valid_assetid($assetid[$i]);
				$assetid[$i] = $db->quote($assetid[$i]);
			}
			$assetid_cond = 'IN ('.implode(', ', $assetid).')';
		} else {
			assert_valid_assetid($assetid);
			$assetid_cond = '= '.$db->quote($assetid);
		}

		$select = 'SELECT  l.'.$side_of_link.'id, l.'.$other_side.'id, l.linkid, l.value, l.link_type,
						l.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive';
		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_'.$other_side.' l';

		$where  = 'l.'.$side_of_link.'id '.$assetid_cond.'
				   AND '.db_extras_bitand($db, 'l.link_type', $db->quote($link_types)).' > 0 ';
		if (!is_null($value)) {
			$where .= ' AND (l.value = '.$db->quote($value);
			if ($value == '') $where .= ' OR l.value IS NULL';
			$where .= ')';
		}
		if (!is_null($dependant)) {
			$where .= ' AND l.is_dependant = '.$db->quote(($dependant) ? '1' : '0');
		}
		if (!is_null($exclusive)) {
			$where .= ' AND l.is_exclusive = '.$db->quote(($exclusive) ? '1' : '0');
		}

		$group_by = '';
		$having = '';
		if (!is_null($access)) {
			$access = (String)$access;
			$userid_cond = '';
			$group_by = 'GROUP BY l.linkid, l.'.$side_of_link.'id, l.'.$other_side.'id, l.value, l.link_type,
								  l.type_code, l.sort_order, l.is_dependant, l.is_exclusive';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$from .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON p.assetid = l.minorid';

				// join to roles table
				$from .= '
					LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r on p.userid = r.roleid';

				// get user and group ids
				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				$userids[] = (String)$GLOBALS['SQ_SYSTEM']->user->id;
				$public_userid = (String)$this->getSystemAssetid('public_user');
				$userids[] = $public_userid;
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote((String)$userids[$i]);
				}

				$userids_str = implode(',', $userids);
				$userid_cond = ' AND (p.userid IN ('.$userids_str.') OR r.userid IN ('.$userids_str.'))';
				$where .= $userid_cond.'
					AND (
						(p.permission = '.$db->quote($access).' AND (
									p.userid <> '.$db->quote($public_userid).'
									OR r.userid <> '.$db->quote($public_userid).'
									OR (p.userid = '.$db->quote($public_userid).' AND p.granted = 1)
									OR (r.userid = '.$db->quote($public_userid).' AND p.granted = 1)
								)
						)';
				if ($effective) {
					$where .= ' OR (p.permission > '.$db->quote($access).' AND p.granted = 1)';
				}
				$where .= ')';
				$having .= 'HAVING MIN(p.granted) <> 0';
				$group_by .= ', p.assetid';
			}//end if

		}//end if access is not null

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
					$type_code[$i] = $db->quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
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
			// order by a specific field in the asset table
			$order_by = 'ORDER BY l.'.$sort_by;
			if ($sort_by != 'type_code') {
				$select .= ', l.'.$sort_by.' AS sort_value';
				if (!empty($group_by)) $group_by .= ', l.'.$sort_by;
			}
		}

		return Array(
				'select'	=> $select,
				'from'		=> $from,
				'where'		=> $where,
				'group_by'	=> $group_by,
				'having'	=> $having,
				'order_by'	=> $order_by,
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
	*
	* @return array
	* @access public
	* @see getChildren()
	*/
	function generateGetChildrenQuery(&$asset, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL, $access=NULL, $effective=TRUE, $inc_type_code=TRUE, $min_depth=NULL, $max_depth=NULL, $direct_shadows_only=TRUE)
	{
		assert_valid_assetid($asset->id);
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// if we are the root folder then we just need to select from the asset table
		if (get_class($asset) == 'root_folder') {
			$select = 'SELECT a.assetid'.(($inc_type_code) ? ', a.type_code' : '');
			$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'a');
		} else {
			// OK, normal asset, let's find a reference to them in the tree and go from there
			$where = 'l.minorid = '.$db->quote($asset->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
					'.$where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$treeid = $db->getOne($sql);
			assert_valid_db_result($treeid);

			$select = 'SELECT DISTINCT l.minorid'.(($inc_type_code) ? ', a.type_code' : '');
			$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
							 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
							 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid
					';
			$where = 't.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid);

			if (!is_null($min_depth) && !is_null($max_depth)) {
				if ($min_depth < $max_depth) {
					$where .= ' AND LENGTH(t.treeid) BETWEEN '.$db->quoteSmart(strlen($treeid) + $min_depth * SQ_CONF_ASSET_TREE_SIZE)
									.' AND '.$db->quoteSmart(strlen($treeid) + $max_depth * SQ_CONF_ASSET_TREE_SIZE);
				} else if ($min_depth > $max_depth) {
					$where .= ' AND LENGTH(t.treeid) BETWEEN '.$db->quoteSmart(strlen($treeid) + $max_depth * SQ_CONF_ASSET_TREE_SIZE)
									.' AND '.$db->quoteSmart(strlen($treeid) + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
				} else {
					$where .= ' AND LENGTH(t.treeid) = '.$db->quoteSmart(strlen($treeid) + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
				}
			} else if (!is_null($min_depth)) {
				$where .= ' AND LENGTH(t.treeid) >= '.$db->quoteSmart(strlen($treeid) + $min_depth * SQ_CONF_ASSET_TREE_SIZE);
			} else if (!is_null($max_depth)) {
				$where .= ' AND LENGTH(t.treeid) <= '.$db->quoteSmart(strlen($treeid) + $max_depth * SQ_CONF_ASSET_TREE_SIZE);
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		}//end else

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
		$union_where = 'sl.majorid = '.$db->quoteSmart($asset->id);
		if (!$direct_shadows_only) {
			$union_where = '(('.$union_where.") or (sl.majorid IN (SELECT DISTINCT l.minorid $from $where)))";
		}

		$union_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($union_where, 'sl');

		if (!empty($where) && (!empty($type_code) || !is_null($access) || !is_null($dependant))) {
			$where .= ' AND ';
		}

		if (!is_null($dependant)) {
			$where .= ' l.is_dependant = '.$db->quote(($dependant) ? '1' : '0');
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
				$from .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON p.userid = r.roleid ';

				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				$userids[] = (String)$GLOBALS['SQ_SYSTEM']->user->id;
				$public_userid = (String)$this->getSystemAssetid('public_user');
				$userids[] = $public_userid;
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote((String)$userids[$i]);
				}

				$userids_str = implode(',', $userids);
				$userid_cond = ' (p.userid IN ('.$userids_str.') OR r.userid IN ('.$userids_str.'))';
				$where .= $userid_cond.'
					AND (
							(p.permission = '.$db->quote($access).' AND (
									p.userid <> '.$db->quote($public_userid).'
									OR r.userid <> '.$db->quote($public_userid).'
									OR (p.userid = '.$db->quote($public_userid).' AND p.granted = 1)
									OR (r.userid = '.$db->quote($public_userid).' AND p.granted = 1)
								)
							)';
				if ($effective) {
					$where .= ' OR (p.permission > '.$db->quote($access).' AND p.granted = 1)';
				}
				$where .= ') AND ';
				$having .= 'HAVING MIN(p.granted) <> 0';
				$group_by .= ', p.assetid';
			}
		}//end if access is not null

		if (!empty($type_code)) {

			if (empty($where)) $where = ' WHERE ';

			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); NULL !== ($i = key($type_code)); next($type_code)) {
					$type_code[$i] = $db->quote($type_code[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
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
			$order_by = 'ORDER BY sort_value';
			$select .= ', a.'.$sort_by.' AS sort_value';
		}

		return Array(
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
	*
	* @return array
	* @access public
	* @see getParents()
	*/
	function generateGetParentsQuery($assetid, $type_code='', $strict_type_code=TRUE, $sort_by=NULL, $access=NULL, $effective=TRUE, $min_height=NULL, $max_height=NULL)
	{
		assert_valid_assetid($assetid);

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$assetid = (string) $assetid;

		$in_sql = SQ_TABLE_RUNNING_PREFIX.'get_parent_treeids('.$db->quoteSmart($assetid).', '.$db->quoteSmart(SQ_CONF_ASSET_TREE_SIZE).')';

		if ($db->phptype == 'oci8') {
			$in_sql = 'SELECT * FROM table('.$in_sql.')';
			$result = $db->getCol($in_sql);
			assert_valid_db_result($result);

			if (empty($result)) return Array();

			for (reset($result); NULL !== ($key = key($result)); next($result)) {
				$result[$key] = $db->quoteSmart($result[$key]);
			}

			$in_sql = implode(',', $result);
		} else {
			$in_sql = 'SELECT * FROM '.$in_sql;
		}

		$select = 'SELECT DISTINCT(ll.majorid), ll.type_code, ll.treeid, length(ll.treeid)/'.SQ_CONF_ASSET_TREE_SIZE.' as depth';

		$from = 'FROM
					(
					SELECT l.majorid, t.treeid, a.type_code, a.name ';

		$perm_join = '';
		if (!is_null($access) && !$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$from .= ', p.userid, p.permission, p.granted ';
			$perm_join = ' INNER JOIN sq_ast_perm p ON p.assetid = l.majorid';
		}

		$from  .= 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';

		$from .= $perm_join;


		// TODO: Is this feasible in getParents?
		if (!is_null($min_height) || !is_null($max_height)) {
			$treeids = $this->getAssetTreeids($assetid);

			$where_comps = Array();

			foreach ($treeids as $treeid) {
				$where_comp = $db->quote($treeid).' LIKE t.treeid || '.$db->quote('%');

				// in the get children query function this section gets minor
				// links, but we want the major links in this case, so we have
				// to go one further link down - hence the '-1' in these queries
				if (!is_null($min_height) && !is_null($max_height)) {

					if ($min_height < $max_height) {
						$where_comp .= ' AND LENGTH(t.treeid) BETWEEN '.$db->quoteSmart(strlen($treeid) - ($max_height-1) * SQ_CONF_ASSET_TREE_SIZE)
										.' AND '.$db->quoteSmart(strlen($treeid) - ($min_height-1) * SQ_CONF_ASSET_TREE_SIZE);
					} else if ($min_height > $max_height) {
						$where_comp .= ' AND LENGTH(t.treeid) BETWEEN '.$db->quoteSmart(strlen($treeid) - ($min_height-1) * SQ_CONF_ASSET_TREE_SIZE)
										.' AND '.$db->quoteSmart(strlen($treeid) - ($max_height-1) * SQ_CONF_ASSET_TREE_SIZE);
					} else {
						$where_comp .= ' AND LENGTH(t.treeid) = '.$db->quoteSmart(strlen($treeid) - ($min_height-1) * SQ_CONF_ASSET_TREE_SIZE);
					}
				} else if (!is_null($min_height)) {
					$where_comp .= ' AND LENGTH(t.treeid) <= '.$db->quoteSmart(strlen($treeid) - ($min_height-1) * SQ_CONF_ASSET_TREE_SIZE);
				} else if (!is_null($max_height)) {
					$where_comp .= ' AND LENGTH(t.treeid) >= '.$db->quoteSmart(strlen($treeid) - ($max_height-1) * SQ_CONF_ASSET_TREE_SIZE);
				}

				$where_comps[] = $where_comp;

			}

			$where = '(('.implode(') OR (', $where_comps).'))';

		} else {
			// No conditions, just stick the treeids query straight into the main query
			$where  ='t.treeid IN (
						'.$in_sql.'
					  )';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		if (!empty($type_code)) {
			$type_code_cond = '';

			// we are cloning $type_code and using the clone instead
			// this will prevent the error of double quoting $type_code in the future
			$type_code_clone = $type_code;
			if (is_array($type_code_clone)) {
				for (reset($type_code_clone); NULL !== ($i = key($type_code_clone)); next($type_code_clone)) {
					$type_code_clone[$i] = $db->quote($type_code_clone[$i]);
				}
				$type_code_cond = 'IN ('.implode(', ', $type_code_clone).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code_clone);
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
			$order_by = 'ORDER BY sort_value';
			$select .= ', ll.'.$sort_by.' AS sort_value';
		} else {
			$order_by = 'ORDER BY depth DESC';
		}

		$group_by = '';
		$having = '';
		$userids_str = '';
		$public_userid = '';
		if (!is_null($access)) {
			$userid_cond = '';
			$group_by = 'GROUP BY ll.majorid, ll.treeid, ll.type_code, ll.name';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$where .= ' AND p.assetid = a.assetid';

				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$public_userid = (String)$this->getSystemAssetid('public_user');
				$userids[] = $public_userid;
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote((String)$userids[$i]);
				}
				$userids_str = implode(',', $userids);
				$userid_cond = ' AND p.userid IN ('.$userids_str.')';
				$where .= $userid_cond.'
					AND (
							(p.permission = '.$db->quote($access).' AND (
								p.userid <> '.$db->quote($public_userid).'
								OR (userid = '.$db->quote($public_userid).' AND granted = 1)
								)
							)';
				if ($effective) {
					$where .= ' OR (p.permission > '.$db->quote($access).' AND p.granted = 1)';
				}
				$where .= ')';
				$having .= 'HAVING MIN(ll.granted) <> 0';
			}

		}

		$from .= $where.') ll ';
		$outer_where = '';
		if (!is_null($access) && !$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$from .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON ll.userid = r.roleid';

			// Main where caluse. This hsould only be printed if we have user conditions i.e not root/sys_admin
			$outer_where = ' WHERE (
								ll.userid IN ('.$userids_str.') OR r.userid IN ('.$userids_str.')
							 )
							 AND (
									(
										ll.permission = '.$db->quote($access).' AND
																					(
																						ll.userid <> '.$db->quote($public_userid).'
																						OR r.userid <> '.$db->quote($public_userid).'
																						OR (ll.userid = '.$db->quote($public_userid).' AND ll.granted = 1)
																						OR (r.userid = '.$db->quote($public_userid).' AND ll.granted = 1)
																					)
									)';
			if ($effective) {
				$outer_where .= ' OR
									(
										ll.permission > '.$db->quote($access).' AND ll.granted = 1
									)';
			}
			$outer_where .= ')';
		}


		$union_select = ' UNION ALL SELECT sl.majorid, null, null as treeid, null as depth';
		if (!is_null($sort_by)) {
			$union_select .= ', null as sort_value';
		}

		$union_from   = ' FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk sl';
		$union_where  = 'sl.minorid = '.$db->quoteSmart($assetid);
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
		return $sql_array;

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


}//end class

?>
