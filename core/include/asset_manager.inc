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
* $Id: asset_manager.inc,v 1.322.2.33 2006/04/07 05:25:54 sdanis Exp $
*
*/


/**
* Asset_Manager
*
* Purpose
*    Holds information on installed packages, as represented by the DB
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.322.2.33 $
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
	* @return Array(type_code => Array(info))
	* @access public
	*/
	function getAssetTypes($instantiable=null, $non_system_access=false)
	{
		$asset_types = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
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

		while (null !== ($row = $result->fetchRow())) {
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
	* @access public
	*/
	function refreshAssetType($type_code)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
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

	}//end refreshAssetType()


	/**
	* Returns whether the passed asset is installed in the system or not
	*
	* @param string $type_code  the code name for the asset that you want to check
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
	* @param array|string	$type_code	the code name (or array of code names) for the asset that you want to check
	* @param string			$field		if exists, returns this information from the array for this specific field
	*
	* @return mixed - on success: field value if field specified, array of field values if field blank
	*				  on failure: null if field specified, empty array otherwise
	* @access public
	*/
	function getTypeInfo($type_code, $field='')
	{
		$field = trim($field);

		// if the file does not exist, then we must be installing so get the info from the database
		if (!file_exists(SQ_DATA_PATH.'/private/db/asset_types.inc')) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			if (is_array($type_code)) {
				for (reset($type_code); null !== ($key = key($type_code)); next($type_code)) {
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
					return (empty($field)) ? Array() : null;
				}
				if (empty($field)) {
					return $this->_asset_types[$type_code];
				} else {
					if (!isset($this->_asset_types[$type_code][$field])) {
						trigger_localised_error('SYS0185', E_USER_WARNING, $field);
						return null;
					}
					return $this->_asset_types[$type_code][$field];
				}
			}
		}//end if asset_types.inc exists

	}//end getTypeInfo()


	/**
	* Retrieves the default component weightings for an asset type
	*
	* @param string	$type_code	the type code of the wanted default weightings
	*
	* @return Array()
	* @access public
	*/
	function getTypeDefaultWeightings($type_code)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$default_weightings = Array();

		if (is_array($type_code)) {
			$type_codes = $type_code;
			// load the weightings
			$sql = 'SELECT type_code, dw.component, dw.weighting, dw.description
					FROM
						sq_ast_typ at JOIN sq_sch_idx_wgt_dflt dw
							ON at.type_code = dw.type_code
					WHERE
						at.type_code IN (';
			$first = true;
			foreach ($type_codes as $type_code) {
				if (!$first) $sql .= ', ';

				$first = false;

				$sql .= $db->quote($type_code);
			}
			$sql .= ')';

			$results = $db->getAll($sql);
			assert_valid_db_result($results);

			$default_weightings = Array();
			foreach ($results as $result) {
				$default_weightings[$result['type_code']] = Array();
				$default_weightings[$result['type_code']][$result['component']] =
					Array('default' => 1, 'weight' => $result['weight'], 'description' => $result['description']);
			}
		} else {
			if ($type_code != '') {
				// load the weightings for this type code only
				$sql = 'SELECT dw.component, dw.weight, dw.description
						FROM
							sq_ast_typ at JOIN sq_sch_idx_wgt_dflt dw
								ON at.type_code = dw.type_code
						WHERE
							at.type_code = '.$db->quote($type_code);

				$results = $db->getAll($sql);
				assert_valid_db_result($results);

				foreach ($results as $result) {
					$default_weightings[$result['component']] =
						Array('default' => 1, 'weight' => $result['weight'], 'description' => $result['description']);
				}

			}
		}

		return $default_weightings;

	}//end getTypeDefaultWeightings()


	/**
	* Returns an array representing the class hierarchy for the asset types, either for whole system
	* or for under the the passed asset type
	*
	* @param string	$base_type_code	the asset type's hierarchy to return
	* @param string	$allowed_access
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
			$db = &$GLOBALS['SQ_SYSTEM']->db;
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
	* @param array(int)		$assetids			an array of assets to limit
	* @param array(string)	$type_code			an array of types codes to limit the results to
	* @param boolean		$strict_type_code	ensure returned assets are of the passed type codes
	*
	* @return mixed
	* @access public
	*/
	function getAssetTypeInfo($assetids, $type_code=Array(), $strict_type_code=true)
	{
		assert_type($assetids, 'array');
		assert_type($type_code, 'array');
		if (empty($assetids)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$where = '';

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
				'.$where.' ORDER BY a.assetid';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		assert_valid_db_result($result);

		return $result;

	}//end getAssetTypeInfo()


	/**
	* Includes the class file for the passed asset
	*
	* @param string	$type_code	the code name for the asset that you want to check
	*
	* @access public
	*/
	function includeAsset($type_code)
	{
		$error_msg = 'Asset "'.$type_code.'" is not installed on the system, unable to include its source file';

		$type_code = strtolower($type_code);
		assert_isset_array_index($this->_asset_types, $type_code, $error_msg);

		// we only need to require the type_code and its asset string once
		if (isset($this->_asset_types[$type_code]['included']) && $this->_asset_types[$type_code]['included']) {
			return;
		}
		require_once SQ_SYSTEM_ROOT.'/'.$this->_asset_types[$type_code]['dir'].'/'.$type_code.'.inc';

		// include the language strings
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings($type_code);
		foreach ($this->getTypeAncestors($type_code) as $type_parent) {
			$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings($type_parent);
		}

		$this->_asset_types[$type_code]['included'] = true;

	}//end includeAsset()


	/**
	* Returns an array of all assets types that are parents for the passed asset
	* in the order Array ('parent', 'grand-parent', 'great-grand-parent', ..., 'asset')
	*
	* @param string		$type_code		the code name for the asset that you want to check
	* @param boolean	$include_asset	when true adds the "Asset" type to the parent list,
	*									even though it's an unistantiable object
	*
	* @access public
	*/
	function getTypeAncestors($type_code, $include_asset=true)
	{
		if ($type_code == 'asset') return Array();
		assert_isset_array_index($this->_asset_types, $type_code, 'Asset Type "'.$type_code.'" is not installed on the system');

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$result = $db->getCol('SELECT inhd_type_code
								FROM sq_ast_typ_inhd
								WHERE type_code = '.$db->quote($type_code).'
								  AND inhd_type_code <> type_code
								'.(($include_asset) ? '' : ' AND inhd_type_code <> '.$db->quote('asset')).'
								ORDER BY inhd_type_code_lvl DESC'
								);
		assert_valid_db_result($result);
		return $result;

	}//end getTypeAncestors()


	/**
	* Returns an array of all assets types that are descendants for the passed asset type
	* in the order Array ('type_code_sub_class_one', 'type_code_sub_class_two', ..., 'type_code_sub_class_one_sub_class_one', ...)
	*
	* @param string|Array	$type_code		the type code that you want the decendants of
	* @param boolean		$include_passed	when true includes the passed $type_code in the result
	*
	* @access public
	*/
	function getTypeDescendants($type_code, $include_passed=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$type_code_cond = '';
		if (is_array($type_code)) {
			for (reset($type_code); null !== ($i = key($type_code)); next($type_code)) {
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
	* Checks the passed attribute type is valid type or not
	*
	* @param string	$attr_type		the type of asset represented by the id
	*
	* @return boolean
	* @access public
	*/
	function validAttributeType($attr_type)
	{
		$file = SQ_ATTRIBUTES_PATH.'/'.$attr_type.'/'.$attr_type.'.inc';
		if (!is_file($file)) return false;
		require_once($file);
		return class_exists('Asset_Attribute_'.$attr_type);

	}//end validAttributeType()


//--        ATTRIBUTES        --//


	/**
	* Returns an array of information about attributes
	*
	* @param array(int)	$attrids	an array of assets to limit
	*
	* @return array( int => array() )
	* @access public
	*/
	function getAttributeInfo($attrids)
	{
		assert_type($attrids, 'array');
		if (empty($attrids)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed
	*									because you can't use the '@' operator when
	*									returning by reference
	*
	* @return mixed object Asset_Attribute or NULL
	* @access public
	*/
	function &getAttribute($attrid, $value=null, $mute_errors=false)
	{
		if (!isset($this->_attributes[$attrid]) || !is_object($this->_attributes[$attrid]['object'])) {

			$this->_attributes[$attrid] = Array();
			$this->_attributes[$attrid]['object'] = null;
			$this->_attributes[$attrid]['count']  = 0;

			include_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
			$this->_attributes[$attrid]['object'] = Asset_Attribute::loadAttribute($attrid);
			if (empty($this->_attributes[$attrid]['object']->id)) {
				$this->_attributes[$attrid]['object'] = null;
			}

		}

		$this->_attributes[$attrid]['count']++;
		return $this->_attributes[$attrid]['object'];

	}//end getAttribute()


	/**
	* Retrieves the attributes (with null values) for the asset type
	*
	* @param string			$type_code		the type of asset
	* @param array(string)	$details		an array of columns from the DB table you want returned
	*
	* @return array
	* @access public
	*/
	function getAssetTypeAttributes($type_code, $details=Array('name', 'type'))
	{
		if (!is_string($type_code)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
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
	* @access public
	* @return &object Asset or null on error
	*/
	function &getAsset($assetid, $type_code='', $mute_errors=false)
	{
		assert_valid_assetid($assetid);
		$asset = &$this->_asset_cache->get($assetid);

		if (empty($asset)) {

			// check if we are getting links for a shadow asset, and palm the request off to the
			// handler of the shadow asset if we are
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {

				$real_assetid = $id_parts[0];
				$asset = &$this->getAsset($real_assetid, '', $mute_errors);
				if (!method_exists($asset, 'getAsset')) {
					trigger_localised_error('SYS0203', E_USER_WARNING, $asset->name);
				} else {
					$asset = &$asset->getAsset($id_parts[1]);
				}

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
						$asset = @new $type_code($assetid);
						if (empty($asset->id)) $asset = null;
					}
				}

			}//end if

			if (isset($asset)) $this->_asset_cache->add($asset);

		}//end if

		return $asset;

	}//end getAsset()


	/**
	* Determine if the passed assetid refers to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param int	$assetid	the ID of the asset we are going to the check for existence
	*
	* @return boolean
	* @access public
	*/
	function assetExists($assetid)
	{
		assert_valid_assetid($assetid);

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast ';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($assetid));

		// query the database, expect only one row
		$db_assetid = $db->getOne($sql.$where);
		assert_valid_db_result($db_assetid);

		return ($db_assetid == $assetid);

	}//end assetExists()


	/**
	* Registers the passed object in the store, for others to get
	* Used when a new record has just been "created"
	*
	* @param object Asset	&$obj	the asset to be registered
	*
	* @see forgetAsset()
	* @access public
	* @return void
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
	* @param object Asset	&$obj			the asset to be unregistered
	* @param boolean 		$force_clean	force the asset manager to forget this asset rather
	*										than just reducing its entires (only used by the asset itself
	*										during a purge)
	*
	* @see rememberAsset()
	* @access public
	* @return void
	*/
	function forgetAsset(&$obj, $force_clean=false)
	{
		// wrong class or no id, cya
		if (!is_object($obj) || !is_a($obj, 'asset') || !$obj->id) {
			return;
		}

		$this->_asset_cache->release($obj->id);
		if ($force_clean) {
			if (!$this->_asset_cache->remove($obj->id, true)) {
				trigger_localised_error('SYS0306', E_USER_WARNING, $obj->id);
			}
		}

	}//end forgetAsset()


	/**
	* Returns an assetid for a system asset if it exists
	*
	* @param string		$name	the name of the system asset 'root_folder', 'trash_folder' etc
	*
	* @return mixed [string|boolean] assetid if found, false if not
	* @access public
	*/
	function getSystemAssetid($name)
	{
		if (!is_string($name)) {
			trigger_localised_error('SYS0190', E_USER_WARNING);
			return false;
		}

		if (empty($this->_system_assetids)) {
			$this->_reloadSystemAssetList();
		}

		if (isset($this->_system_assetids[$name])) {
			return $this->_system_assetids[$name];
		} else {
			trigger_localised_error('SYS0209', E_USER_WARNING, $name);
			return false;
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
			$system_assets = false;
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
	* Returns true if given asset type is already registered as a system asset
	*
	* During install, the output of this function should not be cached as it can change when new
	* asset types are installed
	*
	* @param string	$asset_type	typecode of asset in question
	*
	* @return bool
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
	* @return bool
	* @access public
	*/
	function registerSystemAsset($asset_type=null, $assetid=null)
	{
		if (is_null($asset_type) || is_null($assetid)) {
			return false;
		}

		$this->_system_assetids[$asset_type] = $assetid;
		return true;

	}//end registerSystemAsset()


	/**
	* Returns a reference to a system asset
	*
	* @param string		$name			the name of the system asset 'root_folder', 'trash_folder' etc
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @access public
	*/
	function &getSystemAsset($name, $mute_errors=false)
	{
		$assetid = ($mute_errors) ? @$this->getSystemAssetid($name) : $this->getSystemAssetid($name);

		if ($assetid !== false) {
			return $this->getAsset($assetid, $name, $mute_errors);
		} else {
			$null = null;
			return $null;
		}

	}//end getSystemAsset()


	/**
	* Determine if this asset can be purged from the trash
	*
	* The function stops system assets and their dependant children from being purged.
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
			return false;
		}

		// if we get to here, it is not one of the
		// system assets, but it may be dependantly linked to one
		// so lets check for that
		$parents = $this->getDependantParents($asset->id);
		foreach ($parents as $parentid) {
			if (in_array($parentid, $system_asset_ids)) {
				return false;
			}
		}

		return true;

	}//end canPurgeAsset()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array(int)		$assetids			an array of assets to limit
	* @param string|array	$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean		$strict_type_code	ensure returned assets are of the passed type codes
	* @param string			$field				single field from the asset table to return (along with the assetid)
	*											Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=true, $field='')
	{
		assert_type($assetids, 'array');
		if (empty($assetids)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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

		$where = 'a.assetid IN ('.implode(', ', $assetids).')';

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

		return $result;

	}//end getAssetInfo()


	/**
	* Returns list of all assetids of a certain type
	*
	* @param string		$type_code		if this exists then this object is used to load the asset,
	*									if not then the DB is queried to find out the asset type
	* @param boolean	$strict			whether we are finding assets that are just a $type_code
	*									or $type_code and any of it's sub-classes
	* @param boolean	$include_type	if false returns Array(assetid, assetid, ...), if true
	*									returns Array(assetid => asset_type, assetid => asset_type, ...)
	*
	* @return array
	* @access public
	*/
	function getTypeAssetids($type_code, $strict=true, $include_type=false)
	{
		if (!is_string($type_code)) return Array();
		$db = &$GLOBALS['SQ_SYSTEM']->db;

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


	/**
	* Get the values of the attribute with the given name for the given assetids
	*
	* The assetids supplied must be assets of the specified type, or assets which
	* inherit the specified attribute from the specified asset type. You may use
	* 'asset' for the $asset_type to skip asset type checking (recommended that this
	* only be used for common attributes like name).
	*
	* @param string	$attr_name	The name of the attribute to get the values of
	* @param string	$asset_type	The type code of the asset type that defines the attribute
	* @param array	$assetids	IDs of the assets whose values we want
	*
	* @return array	(string => mixed) representing assetid => attribute value
	* @access public
	*/
	function getAttributeValuesByName($attr_name, $asset_type, $assetids)
	{
		if (empty($assetids)) return Array();
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// First get the default values
		$sql = 'SELECT a.assetid, at.default_val
				FROM sq_ast a, sq_ast_attr at
				WHERE a.type_code = at.type_code
					AND at.name = '.$db->quote($attr_name).'
					AND a.assetid IN ('.implode(', ', $assetids).')';
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
					AND assetid IN ('.implode(', ', $assetids).')';
		$custom_vals = $db->getAssoc($sql);
		assert_valid_db_result($custom_vals);

		// Combine the two
		foreach ($custom_vals as $assetid => $val) {
			$res[$assetid] = $val;
		}
		return $res;


	}//end getAttributeValuesByName()


//--        CONTENT        --//


	/**
	* Get the editable content of any asset below the passed assetid
	*
	* @param int	$assetid	The asset we want to get the contents of
	*
	* @return bool|array	FALSE if assetid is invalid, Array(assetid=>content) otherwise
	* @access public
	*/
	function getEditableContents($assetid)
	{
		if (!$assetid || !$this->assetExists($assetid)) {
			return false;
		}

		$children = $this->getDependantChildren($assetid);
		$contents = Array();

		foreach ($children as $childid => $type_code) {
			$child = &$this->getAsset($childid);
			$child_content = $child->getContent();

			if (!empty($child_content)) {
				$contents[$childid] = $child->getContent();
			}

			$this->forgetAsset($child);
		}

		return empty($contents) ? false : $contents;

	}//end getEditableContents()


	/**
	* Set the editable content of the specified asset
	*
	* @param int	$assetid	The asset we want to set the contents of
	*
	* @return boolean	FALSE if assetid is invalid, true if successful
	* @access public
	*/
	function setEditableContents($assetid, $content)
	{
		$asset = &$this->getAsset($assetid);

		if (is_null($asset)) return false;

		$asset->setContent($content);

		return true;

	}//end setEditableContents()


//--        CLONING        --//


	/**
	* Create the shell of a clone without cloning any of the source's components
	*
	* The only child assets that are cloned are the ones that are <i>DEPENDANTLY</i> linked to this asset.
	* <i>NO OTHER</i> linked assets are cloned.
	*
	* @param object Asset	&$source			the source asset that we are creating a clone of
	* @param Array			&$link				information used to create the initial link,
	* <PRE>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </PRE>
	* @param Array			&$clone_map			a map for that can be used for re-indexing data held in
	*												assets of the form Array([old asset id] => [new asset id])
	*
	* @return mixed reference to the new object or NULL if an error occured
	* @access private
	*/
	function &_cloneAsset(&$source, &$link, &$clone_map)
	{
		if (!empty($link)) {
			// make sure the initial link information is passed in
			assert_isset_array_index($link, 'asset', 'Cannot clone asset without an asset to link to');
			assert_isset_array_index($link, 'link_type', 'Cannot clone asset without a link type');
			assert_not_empty(($link['link_type'] & SQ_SC_LINK_SIGNIFICANT), 'Cannot clone asset with an insignificant link type');
		}

		$null = null; // needed because we return by reference
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

		$db = &$GLOBALS['SQ_SYSTEM']->db;
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

		$clone = &$this->getAsset($assetid, $source->type());
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $null;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// lock the new clone in the same chain as we are locked
			if (!$this->acquireLock($clone->id, 'all', $source->id)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}
		}

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
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return $null;
						}
					}

					if (!$this->acquireLock($link['asset']->id, 'links', $lock['links']['source_assetid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return $null;
					}
				}
			}

			$linkid = $link['asset']->createLink($clone, $link['link_type'], $link['value'], $link['sort_order'], $link['is_dependant'], $link['is_exclusive']);
			if (empty($linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}

		}//end if !empty(link)


		// if we got this far all is well,
		// so add to the map and return the clone
		$clone_map[$source->id] = $clone->id;
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $clone;

	}//end _cloneAsset()


	/**
	* Creats a clone of an asset and clones the required components of the source asset
	*
	* if $clone_dependants is set the true, the dependants of the source asset will be cloned,
	* and the specified components will be cloned to them as well.
	*
	* @param object Asset	&$source			the source asset that we are creating a clone of
	* @param array			&$link				information used to create the initial link,
	* <PRE>
	* Array (
	*   'asset'         => [ref major asset to create link under],
	*   'link_type'     => SQ_LINK_?,
	*   'value'         => [link value],
	*   'sort_order'    => [link sort order],
	*   'is_dependant'  => [0|1],
	*   'is_exclusive'  => [0|1]
	* );
	* </PRE>
	* @param array			&$clone_map			a map for that can be used for re-indexing data held in
	*											assets of the form Array([old asset id] => [new asset id])
	* @param array			$components	the wanted components to clone,
	* <PRE>
	* Array(
	*   'attributes',
	*   'metadata_schemas',
	*   'metadata',
	*   'workflow',
	*   'permissions',
	*   'data'// data directory
	* );
	* </PRE>
	* or alternately
	* <PRE>
	* Array('all');
	* </PRE>
	* @param boolean		$clone_dependants	if TRUE the dependants of the source asset will be cloned
	*											and will have their sources components cloned
	*
	* @return &object Asset		the clone of the source, NULL on error
	* @access public
	*/
	function &cloneAsset(&$source, &$link, &$clone_map, $components, $clone_dependants=true)
	{
		$null = null;

		// if the source asset is actually a shadow asset, skip it
		$id_parts = explode(':', $source->id);
		if (isset($id_parts[1])) return $null;

		// make sure this type of asset can be cloned
		if (!$source->canClone()) {
			trigger_localised_error('SYS0070', E_USER_WARNING, $source->type());
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$clone = &$this->_cloneAsset($source, $link, $clone_map);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $null;
		}

		if (!$source->cloneComponents($clone, $components)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $null;
		}


		// we do not want to clone any assets that are type_3 or type_notice linked to the asset we are
		// cloning. Instead, we will just link them up to the cloner's orginial link counter parts
		$orig_links = $this->getLinks($source->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, '', true, 'major', null, false, null);
		foreach ($orig_links as $orig_link) {
			$asset = &$this->getAsset($orig_link['minorid'], $orig_link['minor_type_code']);
			$linkid = $clone->createLink($asset, $orig_link['link_type'], $orig_link['value'], $orig_link['sort_order'], $orig_link['is_dependant'], $orig_link['is_exclusive']);

			if (!$linkid) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}

		}//end foreach

		// OK now that we are linked up let's our dependants
		if ($clone_dependants) {

			$dependant_links = $this->getLinks($source->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);

			if (!empty($dependant_links)) {
				$create_link = Array(
								'asset'			=> &$clone,
								'link_type'		=> null,
								'value'			=> null,
								'sort_order'	=> -1,
								'is_dependant'	=> '1',
								'is_exclusive'	=> '0',
							   );

				foreach ($dependant_links as $data) {
					// If this asset has already been cloned in this duplication run
					// then just link it to the new clone of ourselves
					if (isset($clone_map[$data['minorid']])) {

						$cloned_child = &$this->getAsset($clone_map[$data['minorid']], $data['minor_type_code']);
						if (is_null($cloned_child)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return $null;
						}

						$linkid = $clone->createLink($cloned_child, $data['link_type'], $data['value'], $data['sort_order'], '1', $data['is_exclusive']);
						if (!$linkid) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return $null;
						}

					// otherwise clone and link
					} else {
						$child = &$this->getAsset($data['minorid'], $data['minor_type_code']);
						if (is_null($child)) continue;

						// skip shadow assets
						$id_parts = explode(':', $child->id);
						if (isset($id_parts[1])) continue;

						$create_link['link_type']		= $data['link_type'];
						$create_link['value']			= $data['value'];
						$create_link['sort_order'] 		= $data['sort_order'];
						$create_link['is_exclusive']	= $data['is_exclusive'];

						$cloned_child = &$this->cloneAsset($child, $create_link, $clone_map, $components, $clone_dependants);

						if (is_null($cloned_child)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return $null;
						}
					}//end if
				}//end foreach
			}//end if empty
		}//end if clone dependants

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $clone;

	}//end cloneAsset()


//--        LOCKING        --//


	/**
	* Acquires a lock on an asset
	*
	* Note that this method operates on a single asset only.  To get the locks on children
	* too (as per backend behaviour), you need to use the acquire_locks HIPO job.
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
	* @return int		zero on failure, non-zero on success
						(1 on success if all lock types requested had to be acquired,
	*					 2 if any lock was updated [ie if at least one lock type was already held])
	* @access public
	*/
	function acquireLock($assetid, $lock_type, $source_assetid=0, $force=false, $expires=0)
	{
		// type-check lock type BEFORE we get asset, otherwise we're just wasting time allocating it
		if (empty($lock_type) || !is_string($lock_type)) {
			trigger_localised_error('SYS0074', E_USER_WARNING, $assetid);
			return false;
		}

		$asset = &$this->getAsset($assetid);
		assert_not_null($asset);

		$current_locks = $this->getLockInfo($assetid, $lock_type, true, true);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = true;

		$can_force = ($force) ? $asset->canForceablyAcquireLock($lock_type) : false;
		$lock_updated = false;
		foreach ($current_locks as $lock_type => $lock) {

			if (!empty($lock) && $lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
				// the user is asking to acquire a lock they already had
				// so just update the lock expiry date
				if ($this->updateLock($assetid, $lock_type, $expires)) {
					$lock_updated = true;
					continue;
				} else {
					$success = false;
					break;
				}
			}

			// special case where the current user is actually
			// removing the current lock and taking it for themselves
			if (!empty($lock) && $can_force) {

				// attempt to remove the lock
				if (!$this->releaseLock($assetid, $lock_type)) {
					$success = false;
					break;
				}

				// send an internal message
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				$ms->openLog();

				$user = &$this->getAsset($lock['userid']);

				$locked_assetids = Array((int) $assetid);
				foreach ($lock['chained_assets'] as $row) {
					$locked_assetids[] = $row['assetid'];
				}

				foreach ($locked_assetids as $locked_assetid) {
					$locked_asset = &$this->getAsset($locked_assetid);

					// create a new message object and populate it, before adding to
					// the message queue in the internal messaging system to sending later
					$msg_reps = Array(
									'user_name'		=> $GLOBALS['SQ_SYSTEM']->user->name,
									'type_code'		=> $this->getTypeInfo($locked_asset->type(), 'name'),
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
				$lock = $this->getLockInfo($assetid, $lock_type, false, true);

			}//end if

			// is this asset already locked
			if (!empty($lock)) {
				$user = &$this->getAsset($lock['userid']);
				trigger_localised_error('SYS0101', E_USER_NOTICE, $lock_type, $asset->name, $user->name);
				$success = false;
				break;
			}

			$lockid = 'asset.'.$assetid.'.'.$lock_type;
			$source_lockid = ($source_assetid) ? 'asset.'.$source_assetid.'.'.$lock_type : '';
			if (true !== ($err_msg = $GLOBALS['SQ_SYSTEM']->acquireLock($lockid, $source_lockid, $expires))) {
				trigger_localised_error('SYS0100', E_USER_NOTICE, $lock_type, $asset->name, $err_msg);
				$success = false;
				break;
			}

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction(($success) ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return ($success) ? (($lock_updated) ? 2 : 1) : 0;

	}//end acquireLock()


	/**
	* Release the locks that the passed asset holds
	*
	* @param int		$assetid	the assetid of the asset that we want to release
	* @param string		$lock_type	the type of the lock (eg 'contents', 'permissions', etc)
	*
	* @return boolean
	* @access public
	*/
	function releaseLock($assetid, $lock_type)
	{
		// type-check lock type BEFORE we get asset, otherwise we're just wasting time allocating it
		if (empty($lock_type) || !is_string($lock_type)) {
			trigger_localised_error('SYS0075', E_USER_WARNING, $assetid);
			return false;
		}

		$current_locks = $this->getLockInfo($assetid, $lock_type, true, false);
		if (empty($current_locks)) return true;

		$asset = &$this->getAsset($assetid);
		assert_not_null($asset);

		$success = true;

		$can_force = $asset->canForceablyAcquireLock($lock_type);

		foreach ($current_locks as $lock_type => $lock) {
			if (empty($lock)) continue;

			// is this asset already locked by someone else and we can't forceably acquire it, piss off
			if ((int) $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserid() && !$can_force) {
				$user = &$this->getAsset($lock['userid']);
				trigger_localised_error('SYS0264', E_USER_WARNING, $lock_type, $asset->name, $user->name);
				$success = false;
				continue;
			}

			if (true !== ($err_msg = $GLOBALS['SQ_SYSTEM']->releaseLock('asset.'.$assetid.'.'.$lock_type))) {
				trigger_localised_error('SYS0109', E_USER_NOTICE, $lock_type, $asset->name, $err_msg);
				$success = false;
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
		$current_locks = $this->getLockInfo($assetid, $lock_type, true, true);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = true;

		foreach (array_keys($current_locks) as $lock_type) {

			if (true !== ($err_msg = $GLOBALS['SQ_SYSTEM']->updateLock('asset.'.$assetid.'.'.$lock_type, $expires))) {
				trigger_localised_error('SYS0122', E_USER_NOTICE, $assetid, $err_msg);
				$success = false;
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
	function getLockInfo($assetid, $lock_type, $force_array=false, $full_chain=false, $check_expires=true)
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
	* @param int		$assetid		the assetid of the asset that we want lock info for
	* @param string		$lock_type		any valid lock type for the passed asset's type, plus 'all' which will return all lock types that this asset has
	*
	* @return array
	* @access private
	*/
	function getLockTypes($assetid, $lock_type)
	{
		if (!isset($this->_tmp['lock_types'][$assetid])) {
			$asset = &$this->getAsset($assetid);
			assert_not_null($asset);
			$this->_tmp['lock_types'][$assetid] = $asset->lockTypes();
		}

		if ($lock_type != 'all') {
			if (!isset($this->_tmp['lock_types'][$assetid][$lock_type])) {
				return Array();
			}
			$bits = bit_elements($this->_tmp['lock_types'][$assetid][$lock_type]);
			$lock_types = Array();
			foreach ($bits as $bit) {
				if (false !== ($k = array_search($bit, $this->_tmp['lock_types'][$assetid]))) {
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
	* @param bool			$strict_type_code	whether we are finding an asset that
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
	function getLink($assetid, $link_type, $type_code='', $strict_type_code=true, $value=null, $side_of_link='major', $exclusive=null)
	{
		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.'.$other_side.'id  = a.assetid
				';
		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid).'
				  AND l.link_type = '.$db->quote($link_type);

		if (!is_null($exclusive)) {
		  $where .= ' AND l.is_exclusive = '.$db->quote(($exclusive) ? '1' : '0');
		}

		if ($type_code) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); null !== ($i = key($type_code)); next($type_code)) {
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
		}

		if (!is_null($value)) {
			$where .= ' AND (l.value = '.$db->quote($value);
			if ($value == '') $where .= ' OR l.value IS NULL';
			$where .= ')';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.' ORDER BY l.sort_order';

		$result = $db->getRow($sql);
		assert_valid_db_result($result);

		return $result;

	}//end getLink()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	* @return Array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT  l.linkid, l.value, l.link_type, l.sort_order, l.is_dependant, l.is_exclusive, ';
		$where = 'l.linkid = '.$db->quote((int)$linkid);

		$assetid = (int) $assetid;
		if ($assetid > 0) {
			$sql .= 'l.'.$other_side.'id, a.type_code as '.$other_side.'_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON  l.'.$other_side.'id = a.assetid';
			$where .= ' AND l.'.$side_of_link.'id = '.$db->quote($assetid);

		} else {
			$sql .= 'l.majorid, a.type_code as major_type_code, l.minorid, a2.type_code as minor_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON  l.majorid = a.assetid
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a2 ON  l.minorid = a2.assetid';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a2');

		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$result = $db->getRow($sql.$where);
		assert_valid_db_result($result);

		return $result;

	}//end getLinkById()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$other_assetid		the asset on the other side of the link
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link the first assetid is on
	*										('major' or 'minor')
	* @param boolean	$force_array		force the results to return an array of links
	*										even if there is only one link
	* @param boolean	$dependant			The dependant status for all the links must be
	*										this (if not null)
	* @param boolean	$exclusive			The exclusive status for all the links must be
	*										this (if not null)
	*
	* @return array		array of link info if only one link and force_array=false,
	* 					array of arrays of link info otherwise
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=null, $value=null, $side_of_link='major', $force_array=false, $dependant=null, $exclusive=null)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		assert_valid_assetid($assetid);
		assert_valid_assetid($other_assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		if (!$other_assetid || is_object($other_assetid)) {
			trigger_localised_error('SYS0108', E_USER_WARNING);
			return Array();
		}

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type,
					   a.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.'.$other_side.'id  = a.assetid
				';

		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid).'
				  AND l.'.$other_side.'id  = '.$db->quote($other_assetid);



		if (!is_null($link_types)) {
		  $where .= ' AND '.db_extras_bitand($db, 'l.link_type', $link_types).' > 0 ';
		}
		if (!is_null($value)) {
			$where .= ' AND l.value = '.$db->quote($value);
		}
		if (!is_null($dependant)) {
			$where .= ' AND l.is_dependant = '.$db->quote(($dependant) ? '1' : '0');
		}
		if (!is_null($exclusive)) {
			$where .= ' AND l.is_exclusive = '.$db->quote(($exclusive) ? '1' : '0');
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.'
				ORDER BY l.sort_order';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		if (!$force_array && count($result) == 1) {
			return $result[0];
		} else {
			return $result;
		}

	}//end getLinkByAsset()


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
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
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
	* 											Specify that links should be returned for assets the user has read,
	* 											write, or admin access for. Leave NULL to ignore permissions.
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null, $dependant=null, $exclusive=null, $sort_by=null, $access=null)
	{
		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');


		// check if we are getting links for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = &$this->getAsset($real_assetid);
			$links = $asset->getLinks($id_parts[1], $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by);
			$this->forgetAsset($asset);
			return $links;
		}


		// we are not getting links for a shadow asset, so if we are getting child links of
		// an asset that handles shadow assets, palm the request off to it
		if ($side_of_link == 'major') {
			$asset = &$this->getAsset($assetid);
			if (implements_interface($asset, 'bridge')) {
				$links = $asset->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by);
				$this->forgetAsset($asset);
				return $links;
			}
			$this->forgetAsset($asset);
			unset($asset);
		}


		// we couldnt palm the request off, so we better do it ourselves
		$query = $this->generateGetLinksQuery($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $value, $dependant, $exclusive, $sort_by, $access);

		if (empty($query)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$result = $db->getAll(implode(' ', $query));
		assert_valid_db_result($result);

		return $result;

	}//end getLinks()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string		$assetid	the assetid of the asset to find the child links for
	* @param integer	$link_type	the type of links to get
	*
	* @access public
	* @return array
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!isset($this->_tmp['child_links'][$assetid])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

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
							l.linkid, a.assetid, a.short_name, a.type_code, l.link_type, l.sort_order, l.value, l.is_dependant
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.minorid = a.assetid
					'.$where.'
					ORDER BY t.treeid';

			$child_links = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
			assert_valid_db_result($child_links);


			// OK, what we are going to do is set up the effective dependant treeid for each tree link
			for (reset($child_links); null !== ($treeid = key($child_links)); next($child_links)) {
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
	*
	* @param object	Asset	&$asset		the asset whose links lineages we are getting
	*
	* @return array	linkid => Array ('linkid' => linkid, 'link_type' => link_type, 'lineage' => Array(...))
	* @access public
	*/
	function getLinkLineages(&$asset)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = Array();
		$asset_links = $this->getLinks($asset->id, SQ_SC_LINK_ALL, '', true, 'minor');

		foreach ($asset_links as $link) {
			$assetids[$link['majorid']] = Array();
		}

		if (empty($assetids)) {
			$result = Array();
		} else {
			$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.name, a.assetid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk cl
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ON cl.linkid = ct.linkid,
						 '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk pl
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt ON pl.linkid = pt.linkid
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON a.assetid = pl.minorid
					';
			$where = 'cl.minorid IN ('.implode(', ', array_keys($assetids)).')
					  AND ct.treeid LIKE pt.treeid || '.$db->quote('%').'
					  AND pt.treeid <= ct.treeid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$sql .= $where.'
					ORDER BY cl.linkid, ct.treeid, pt.treeid';


			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
			assert_valid_db_result($result);

		}

		foreach ($asset_links as $link_data) {
			$link_info[$link_data['majorid']][] = $link_data;
		}

		$link_lineages = Array();
		foreach (array_values($result) as $link_tree) {

			// work out some values we are going to need
			$parent_data = $link_tree[count($link_tree) -1];
			$majorid = $parent_data['assetid'];

			foreach ($link_info[$majorid] as $link_data) {

				$link_type = $link_data['link_type'];
				$linkid = $link_data['linkid'];

				$asset_lineage = Array();
				foreach ($link_tree as $tree_data) {
					if ($tree_data['assetid'] == $majorid) continue;
					$asset_lineage[$tree_data['assetid']] = $tree_data['name'];
				}
				$asset_lineage[$parent_data['assetid']] = $parent_data['name'];

				$link_lineages[] = Array(
										'linkid'	=> $linkid,
										'link_type'	=> $link_type,
										'lineage'	=> $asset_lineage,
								   );
			}

		}//end foreach result

		return $link_lineages;

	}//end getLinkLineages()


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
	*
	* @return array	Array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see generateGetParentsQuery()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true, $sort_by=null, $access=null)
	{
		// check if we are getting parents for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = &$this->getAsset($real_assetid);
			$parents = $asset->getParents($id_parts[1], $type_code, $strict_type_code);
			$this->forgetAsset($asset);
			return $parents;
		}

		$query = $this->generateGetParentsQuery($assetid, $type_code, $strict_type_code, $sort_by, $access);
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
	* The query generated to get the children is actually contructed in the Asset Manager's
	* generateGetChildrenQuery function.
	*
	* @param int|string		$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	*
	* @return array	Array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see generateGetChildrenQuery()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null, $access=null)
	{
		assert_valid_assetid($assetid);

		// check if we are getting children for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$children = Array();
			$real_assetid = $id_parts[0];
			$asset = &$this->getAsset($real_assetid);
			if (!is_null($asset)) {
				if (!method_exists($asset, 'getChildren')) {
					trigger_localised_error('SYS0204', E_USER_WARNING, $asset->name);
				} else {
					$children = $asset->getChildren($id_parts[1], $type_code, $strict_type_code, $sort_by);
				}
				$this->forgetAsset($asset);
			} else {
				trigger_localised_error('SYS0206', E_USER_WARNING, $real_assetid);
			}
			return $children;
		}

		// we are not getting children for a shadow asset, so if we are getting children of
		// an asset that handles shadow assets, palm the request off to it
		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) {
			return Array();
		}
		if (implements_interface($asset, 'bridge')) {
			$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $sort_by);
			$this->forgetAsset($asset);
			return $children;
		}

		$query = $this->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $sort_by, $access);
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
	* Get all dependant children of an asset in the various tress in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array	an array of the form Array([assetid] => [type_code])
	* @access public
	*/
	function getDependantChildren($assetid, $type_code='', $strict_type_code=true)
	{
		$children = Array();

		$asset = &$this->getAsset($assetid);
		if (!is_null($asset)) {

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'major', null, 1);
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				if (!$dependant_links[$k]['is_dependant']) continue;
				$children[$dependant_links[$k]['minorid']] = $dependant_links[$k]['minor_type_code'];
			}

			// get ALL kids, regardless of type code, so we can chase them down
			// for the type code we want
			$all_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', false, 'major', null, 1);
			for (reset($all_links); NULL !== ($k = key($all_links)); next($all_links)) {
				// must still be dependant
				if (!$all_links[$k]['is_dependant']) continue;
				$children = $children + $this->getDependantChildren($all_links[$k]['minorid'],$type_code, $strict_type_code);
			}

			$this->forgetAsset($asset);
		}

		unset($asset);
		return $children;

	}//end getDependantChildren()


	/**
	* Get all dependant parents of an asset in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array	returns an array of assetids
	* @access public
	*/
	function getDependantParents($assetid, $type_code='', $strict_type_code=true)
	{
		$parents = Array();

		$asset = &$this->getAsset($assetid);
		if (!is_null($asset)) {

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'minor', null, 1);
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				$parentid = $dependant_links[$k]['majorid'];
				$parents[] = $parentid;
				$parents = array_merge($parents, $this->getDependantParents($parentid, $type_code, $strict_type_code));
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
	* @param int|array(int)	$majorids		the majorids to traverse the tree from
	* @param integer		$levels			the number of levels to traverse (if null, traverse all levels)
	* @param array			$exclude_list	an array of assetid to exclude from the tree
	* @param integer		$link_type		the type of wanted links
	*
	* @access public
	* @return array
	*/
	function getAssetTree($majorids, $levels=null, $exclude_list=Array(), $link_type=SQ_SC_LINK_FRONTEND_NAV)
	{
		$tree_data = Array();
		if (!is_array($majorids)) $majorids = Array($majorids);
		$this->_getAssetTree($majorids, $tree_data, $levels, $exclude_list, $link_type);
		return $tree_data;

	}//end getAssetTree()


	/**
	* Does the work for getAssetTree
	*
	* @param int|array(int)	$majorids		the majorids to traverse the tree from
	* @param &Array			$tree_data		the data that is created recursively
	* @param integer		$levels			the number of levels to traverse (if null, traverse all levels)
	* @param Array			$exclude_list	an array of assetid to exclude from the tree
	* @param integer		$link_type		the type of wanted links
	*
	* @access private
	* @return boolean
	*/
	function _getAssetTree($majorids, &$tree_data, $levels, $exclude_list, $link_type)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		// Stuff that is only needed once per design file
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$assetids  = Array();
		$majorids_str = '';
		$exclude_str = implode(',', $exclude_list);
		foreach ($majorids as $id) {
			if (empty($tree_data[$id])) {
				$majorids_str .= $db->quote($id).',';
			} else {
				foreach ($tree_data[$id] as $row) {
					$assetids[] = (int) $row['assetid'];
				}
			}
		}

		if (empty($majorids_str)) return $assetids;
		static $USERIDS_COND = null;

		if (is_null($USERIDS_COND)) {
			// because even public users are users now add their parents to the list
			if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$USERIDS_COND = '';
			} else {
				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$userids[] = $db->quote($this->getSystemAssetid('public_user'));
				for (reset($userids); null !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote((string)$userids[$i]);
				}
				$USERIDS_COND = 'AND p.userid  IN ('.implode(',', $userids).')';
			}

			if (!empty($USERIDS_COND)) {
				$USERIDS_COND .= '
					GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order, p.granted
					HAVING MIN(p.granted) = 1';
			}
		}

		$sql    = 'SELECT DISTINCT a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order
				   FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON a.assetid = l.minorid
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path pt ON a.assetid = pt.assetid ';
		if (!empty($USERIDS_COND)) {
		$sql   .= '
				INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON a.assetid = p.assetid ';
		}
		$where  = 'l.majorid IN ('.substr($majorids_str, 0, -1).')
					 AND '.db_extras_bitand($db, 'l.link_type', $link_type).' > 0';
		if (!empty($exclude_list)) {
			$where .= 'AND a.assetid NOT IN ('.$exclude_str.')';
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
				$asset = &$this->getAsset($row['assetid']);
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
			$this->_getAssetTree($assetids, $tree_data, $levels, $exclude_list, $link_type);
		}
		return true;

	}//end _getAssetTree()


	/**
	* Returns the treeids of any links between the passed assetid and any of its parents
	*
	* @param integer	$assetid	the id of the asset passed
	* @param integer	$link_type	the type of wanted links
	*
	* @access public
	* @return array
	*/
	function getAssetTreeids($assetid, $link_type=null)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		assert_valid_assetid($assetid);

		if (empty($link_type)) {
			$link_type = SQ_SC_LINK_SIGNIFICANT;
		}
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT t.treeid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
				INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON l.linkid = t.linkid';

		$where  = 'WHERE l.minorid = '.$db->quoteSmart($assetid);
		$where .= '	AND '.db_extras_bitand($db, 'l.link_type', $link_type).' > 0';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

		$result = $db->getCol($sql.$where);
		assert_valid_db_result($result);

		return $result;

	}//end getAssetTreeids()


	/**
	* Return true if the passed assetid is in the trash, false otherwise
	*
	* Note that if the passed assetid is the id of the trash folder, false will be returned
	*
	* @param int		$assetid		the asset id to check the trash for
	* @param boolean	$exclusively	is the asset ONLY in the trash
	*									(ie. does it have no other significant links
	*									outside the trash?)
	*
	* @return boolean
	* @access public
	*/
	function assetInTrash($assetid, $exclusively=false)
	{
		$trash = &$this->getSystemAsset('trash_folder');

		// if we are being asked "Is the trash in the trash"
		// lets say "Are you kidding?"
		if ($assetid == $trash->id) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param int			$ignore_linkid		ignore the link represented by this link
	*											id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=true, $ignore_linkid=0)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
	* @param object Asset	&$major		the major asset that we are linking from
	* @param object Asset	&$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param string			$value		the value that is to be associated with this link
	* @param string			$sort_order	the position in the links list that this link should take,
	*									if null or less than zero places at end of list
	* @param string			$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string			$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	*
	* @return int the linkid of the created link
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0', $moving=false)
	{
		if (!$major->id) return 0;
		assert_is_a($minor, 'asset');
		if (!is_null($sort_order)) {
			$sort_order = (int) $sort_order;
		}

		$original_link_type = $link_type;
		$link_type = (int) $link_type;
		if ($link_type != $original_link_type) {
			trigger_localised_error('SYS0241', E_USER_WARNING);
			return 0;
		}

		// can't link to the new parent if it is only in the trash
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
			if ($this->assetInTrash($major->id, true)) {
				trigger_localised_error('SYS0223', E_USER_WARNING, $major->id);
				return 0;
			}
		}

		// prepare the link - do the minor first because the major gets the final say in linking
		$minor->prepareLink($major, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
		$major->prepareLink($minor, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);

		assert_false(!($link_type & SQ_SC_LINK_SIGNIFICANT) && $dependant, 'In order for a link to be dependant it must also be a significant link');
		assert_false((strpos($major->id.$minor->id, ':') !== false), 'You cannot create a link to or from a shadow asset');

		// First, we should check that we don't already have a link of this type
		$current_links = $this->getLinks($major->id, $link_type, $minor->type());
		$num_curr_links = count($current_links);

		for ($i = 0; $i < $num_curr_links; $i++) {
			// it already exists, so we don't need to do anything
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				// if we are trashing the asset and it is already in the trash, just
				// pretend that we created the link
				$trash = &$this->getSystemAsset('trash_folder');
				if ($trash->id == $major->id) {
					return $current_links[$i]['linkid'];
				}

				trigger_localised_error('SYS0192', E_USER_WARNING);
				return $current_links[$i]['linkid'];
			}
		}//end for

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
			// check if we are allowed to link to these type of assets
			if (!$moving) {
				if (($err_msg = $major->canCreateLink($minor, $link_type, $exclusive)) !== true) {
					trigger_localised_error('SYS0301', E_USER_WARNING, $err_msg);
					return 0;
				}
			}
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// to create a link we need to lock the new parent, but we need to make sure
			// we dont unlock it if it was already locked
			$lock_info = @$this->getLockInfo($major->id, 'links');
			$parent_was_locked = !empty($lock_info);
			if (!$this->acquireLock($major->id, 'links')) {
				trigger_localised_error('CORE0012', E_USER_WARNING, $major->name);
				return 0;
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if this link is a significant link,
		// then that we aren't moving under ourselves
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {

			//// Check we aren't moving the minor asset under itself ////
			// get all the links where the minor asset is a parent
			// and check to see if our id is under it
			$sub_sql   = 'SELECT linkid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk ';
			$sub_where = 'minorid = '.$db->quote($minor->id);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where);
			$sub_sql .= $sub_where;

			$sql   = 'SELECT COUNT(DISTINCT ct.linkid)
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt,
						'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk cl ON ct.linkid = cl.linkid ';
			$where = 'ct.treeid LIKE (pt.treeid || '.$db->quote('%').')
					  AND ct.treeid >= pt.treeid
					  AND pt.linkid IN ('.$sub_sql.')
					  AND cl.minorid = '.$db->quote($major->id);

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');

			$moving_under = $db->getOne($sql.$where);
			assert_valid_db_result($moving_under);

			if ($moving_under) {
				trigger_localised_error('CORE0114', E_USER_WARNING, $minor->name, $minor->id, $major->name, $major->id);
				return 0;
			}

		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// make sure the sort order is in a valid range
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

		// sequences are cool, see the PEAR::DB docs for more info
		$linkid = $db->nextId('sq_ast_lnk');
		assert_valid_db_result($linkid);

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$sql = 'UPDATE
					sq_ast_lnk
				SET
					sort_order = sort_order + 1
				WHERE
						majorid		= '.$db->quoteSmart($major->id).'
					AND	sort_order	>= '.$db->quoteSmart($sort_order);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// we need to insert into the link table
		// before we can insert into the link_tree table to ensure
		// referential constraints

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

		$result = $db->query($sql);
		assert_valid_db_result($result);


		// if this is a significant link, then do stuff with the tree
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			//// UPDATE THE TREE TABLE ////

			// Get one of the treeids that we have
			$sql   = 'SELECT t.treeid
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid ';
			$where = 'l.minorid = '.$db->quote($major->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql .= $where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$existing_treeid = $db->getOne($sql);
			assert_valid_db_result($existing_treeid);

			// if we aren't linked anywhere we can't be linked to, sorry
			// NOTE: exception to rule is root folder
			if ((string) $existing_treeid == '-' && get_class($major) != 'root_folder') {
				trigger_localised_error('SYS0240', E_USER_WARNING, $major->name, $major->id);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// OK, what we are going to get a treeid and then do a "INSERT INTO ... SELECT FROM"
			// into the tree table of all the links that are under the minor asset
			$sql   = 'SELECT t.treeid, t.num_kids
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid ';
			$where = 'l.minorid = '.$db->quote($minor->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql .= $where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$minor_tree = $db->getRow($sql);
			assert_valid_db_result($minor_tree);

			$existing_treeid = (string) $existing_treeid;
			if (empty($minor_tree)) {
				$minor_tree = Array('treeid' => '', 'num_kids' => 0);
			} else {
				$minor_tree['treeid'] = (string) $minor_tree['treeid'];
				$minor_tree['num_kids'] = (int) $minor_tree['num_kids'];
			}

			// when links are deleted their entries in the tree table are set to have a linkid of zero
			// (see deleteLink()) so we can attempt to find unused treeid's that are forming gaps
			$sql   = 'SELECT ct.treeid
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ';
			$where = 'ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
					  AND ct.treeid > '.$db->quote($existing_treeid).'
					  AND LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE).'
					  AND ct.linkid = 0';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$sql .= $where;
			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$free_childid = $db->getOne($sql);
			assert_valid_db_result($free_childid);

			if ((string) $free_childid == '') {
				// no free children, generate one from the max value (note that
				// we have to do this by a LIMIT query as the binary string fields
				// do not allow the MAX() aggregate to be run over it)
				$sql   = 'SELECT ct.treeid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ';

				// if we are not the root folder, the we need to get the correct treeid length
				if ($existing_treeid != '-') {
					$where = 'ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
						  AND ct.treeid > '.$db->quote($existing_treeid).'
						  AND LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE);
				} else {
					$where = 'LENGTH(ct.treeid) = '.(SQ_CONF_ASSET_TREE_SIZE);
				}
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
				$order = ' ORDER BY treeid DESC';

				$free_childid = $db->getOne($db->modifyLimitQuery($sql.$where.$order, 0, 1));
				assert_valid_db_result($free_childid);

				if ((string) $free_childid == '') {
					// no max, we must be the first
					if (($free_childid = asset_link_treeid_convert(0, true)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
				} else {
					// we found a max so increment it

					// get only the child part
					$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

					if (($child_num = asset_link_treeid_convert($free_childid, false)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
					if (($free_childid = asset_link_treeid_convert($child_num + 1, true)) === false)  {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
				}//end if

			} else {
				// there was a free entry

				// get only the child part
				$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

				// remove the current zeroed entries in the tree table,
				// as we will be inserting over the top of them

				$sub_where = 'l.minorid = '.$db->quote($major->id);
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');

				$sql = 'DELETE FROM
							sq_ast_lnk_tree
						WHERE
							treeid IN
							(
								SELECT
									t.treeid || '.$db->quoteSmart($free_childid).'
								FROM
									'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
								INNER JOIN
									'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
								'.$sub_where.'
							)';

				$result = $db->query($sql);
				assert_valid_db_result($result);

			}//end if

			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '-') {

				$sql = 'INSERT INTO
							sq_ast_lnk_tree
							(
								treeid,
								linkid,
								num_kids
							)
							VALUES
							(
								'.$db->quoteSmart($free_childid).',
								'.$db->quoteSmart($linkid).',
								'.$db->quoteSmart($minor_tree['num_kids']).'
							)';

				$result = $db->query($sql);
				assert_valid_db_result($result);

			} else {
				// we have existing tree entries, do a insert..select to create entries for them all

				$sql = 'INSERT INTO
							sq_ast_lnk_tree
							(
								treeid,
								linkid,
								num_kids
							)
							SELECT
								(t.treeid || '.$db->quoteSmart($free_childid).'),
								'.$db->quoteSmart($linkid).',
								'.$db->quoteSmart($minor_tree['num_kids']).'
							FROM
											sq_ast_lnk_tree t
								INNER JOIN	sq_ast_lnk l ON t.linkid = l.linkid
							WHERE
								l.minorid = '.$db->quoteSmart($major->id);

				$result = $db->query($sql);
				assert_valid_db_result($result);
			}//end if

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

			$result = $db->query($sql);
			assert_valid_db_result($result);

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
								('.$case.') || '.$db->quoteSmart($free_childid).' || SUBSTR(ct.treeid, '.$db->quoteSmart(strlen($minor_tree['treeid']) + 1).'),
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

				$result = $db->query($sql);
				assert_valid_db_result($result);

			}//end if

		}//end if significant link

		// tell, the asset it has updated
		if (!$major->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}
		if (!$minor->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// release locks depending on if the parent was locked before the link
			if (!$parent_was_locked) {
				if (!$this->releaseLock($major->id, 'links')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}
		}

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'major_name'	=> $major->name,
						'minor_name'	=> $minor->name,
					);
		$message = $ms->newMessage(Array(), 'asset.linking.create', $msg_reps);
		$message->parameters['majorid'] = $major->id;
		$message->parameters['minorid'] = $minor->id;
		$message->parameters['linkid']  = $linkid;
		$message->send();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// Event Manager
		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($major, 'CreateLink', Array('linkid' => $linkid));

		// Trigger Manager
		// check if the link involves the trash
		if ($major->type() != 'trash_folder' && $minor->type() != 'trash_folder') {
			// fire the 'Link Created' events, one for the major asset, one for the minor
			$event_data = Array('linkid' => $linkid);

			if ($major->type() != 'root_folder') {
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_link_created', $major, $event_data);
			}
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_link_created', $minor, $event_data);
		}

		return $linkid;

	}//end createAssetLink()


	/**
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param object Asset	$major		the major asset in the proposed link
	* @param object Asset	$minor		the minor asset in the proposed link
	* @param string			$link_type	the type of link that is proposed
	* @param int			$exclusive	(0|1) whether this will be the only significant link
	*									with $minor as the minor asset
	*
	* @return mixed TRUE or error message string
	* @access public
	* @protected
	*/
	function canCreateLink(&$major, &$minor, $link_type, $exclusive)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		if (!$major->id) return false;

		// if you dont have write access to this asset you cant create any links
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			if (!$major->writeAccess('')) {
				return 'Cannot create link from '.$major->name.' (# '.$major->id.') to '.$minor->name.' (# '.$minor->id.'), permission denied (no write access to "'.$major->name.'")';
			}
		}

		if (!is_a($minor, 'Asset')) {
			return 'Minor is not an asset';
		}

		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canLinkToType($major, $minor->type(), $link_type, 0, $exclusive)) !== true) {
			return $err_msg;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// check for web path conflicts with the new parents children
		if ($link_type & SQ_SC_LINK_WEB_PATHS) {
			$paths = $minor->getWebPaths();
			$bad_paths = $this->webPathsInUse($major, $paths, $minor->id);
			if (!empty($bad_paths)) {
				return 'Cannot create link from '.$major->name.' (# '.$major->id.') to '.$minor->name.' (# '.$minor->id.'), path(s) "'.implode('", "', $bad_paths).'" already in use by one of the children of "'.$major->name.'"';
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
			if (!assert_valid_db_result($current_majorid, '', true, false)) {
				return 'Database Error';
			}

			if ($current_majorid) {
				$current_major = &$this->getAsset($current_majorid);
				return 'Asset "'.$minor->name.'" (Asset #'.$minor->id.') cannot be linked to "'.$major->name.'" (Asset #'.$major->id.') it is exclusively linked to Asset #'.$current_majorid.' ("'.$current_major->name.'")';
			}

		}//end if

		return true;

	}//end canCreateLink()


	/**
	* Returns TRUE if the supplied asset can have assets of the specified type linked under it
	*
	* @param object Asset	$major			the major asset that we are linking to
	* @param string			$type_code		the type code of the proposed link
	* @param string			$link_type		the type of the proposed link
	* @param int			$ignore_linkid	if it gets down to the checking of the cardinality,
	*										ignore the link represented by this link id
	* @param int			$exclusive		(0|1) whether this will be the only significant link
	*										with $minor as the minor asset
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canLinkToType(&$major, $type_code, $link_type, $ignore_linkid=0, $exclusive=0)
	{
		// type checking
		if (!is_a($major, 'asset')) {
			return 'Variable passed in major asset parameter is not an asset object';
		}

		if (!is_string($type_code)) {
			return 'Variable passed in type code parameter is not a string';
		}

		if (!is_numeric($link_type)) {
			return 'Variable passed in link type parameter is not numeric';
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
			return 'Assets of Type "'.$type_code.'" cannot be "'.link_type_name($link_type).'" linked to a "'.$major->type().'"';
		}

		if (!($link_type & SQ_SC_LINK_SIGNIFICANT) && $exclusive) {
			return 'In order for a link to be exclusive it must also be a significant link';
		}

		if (!$exclusive && !empty($allowed_links[$link_type][$type]['exclusive'])) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return 'Assets of Type "'.$type_code.'" must be "'.link_type_name($link_type).'" exclusivly linked to a "'.$major->type().'"';
		}

		//// Now we need to check that the cardinality is kept valid ////

		// if we are only allowed up to a certain number of these links
		if ($allowed_links[$link_type][$type]['card'] != 'M' && $major->id) {
			$num_curr_links = $this->countLinks($major->id, 'major', $link_type, $type, true, $ignore_linkid);
			//  and we already have our quota of links, bugger off
			if ($num_curr_links >= (int) $allowed_links[$link_type][$type]['card']) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				return '"'.$this->getTypeInfo($major->type(), 'name').'" Assets can only have '.((int) $allowed_links[$link_type][$type]['card']).' "'.link_type_name($link_type).'" link(s) to a "'.$type.'"';
			}
		}

		// if we get this far all is OK
		return true;

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
		if (!assert_valid_assetid($to_parentid, '', false, false)) {
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

		$old_parent = &$this->getAsset($link['majorid'], $link['major_type_code']);
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
		$new_parent = &$this->getAsset($to_parentid, '', true);
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
		$minor = &$this->getAsset($link['minorid'], $link['minor_type_code']);
		if (!$minor->id) {
			trigger_localised_error('SYS0222', E_USER_WARNING, $link['minorid']);
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			return 0;
		}

		// are we allowed to move this link?
		if (true !== ($error_msg = $new_parent->canMoveLink($minor, $old_parent, $link_type))) {
			trigger_error($error_msg, E_USER_WARNING);
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			$this->forgetAsset($minor);
			return 0;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// now create the new link, it's non-exclusive + non-dependent, skip canCreateLink() checks
		$new_linkid = $new_parent->createLink($minor, $link_type, '', $to_parent_pos, '0', '0', true);
		if (!$new_linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			$this->forgetAsset($minor);
			return 0;
		}

		// let's try and delete the old link
		$deleted = $old_parent->deleteLink($linkid, true);
		if (!$deleted) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->forgetAsset($old_parent);
			$this->forgetAsset($new_parent);
			$this->forgetAsset($minor);
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

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
		return $this->updateLink($linkid, null, null, $sort_order);

	}//end moveLinkPos()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string $value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=null, $value=null, $sort_order=null)
	{
		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_localised_error('SYS0139', E_USER_WARNING, $linkid);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$set_clauses = Array();

		$link_type_changed	= (!is_null($link_type)		&& $link['link_type']	!= $link_type	);
		$value_changed		= (!is_null($value)			&& $link['value']		!= $value		);
		$sort_order_changed	= (!is_null($sort_order)	&& $link['sort_order']	!= $sort_order	);

		$link_type  = (int) $link_type;
		$sort_order = (int) $sort_order;

		$major = &$this->getAsset((int) $link['majorid'], $link['major_type_code']);
		$minor = &$this->getAsset((int) $link['minorid'], $link['minor_type_code']);

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		if ($link_type_changed) {

			// We need to make sure that we aren't going to have to be stuffing
			// about with tree to get the update working
			// so because the tree only contains significant links if the
			// significant state has changed... barf
			$current_is_sig = (bool) ((int) $link['link_type'] & SQ_SC_LINK_SIGNIFICANT);
			$new_is_sig     = (bool) ($link_type & SQ_SC_LINK_SIGNIFICANT);
			if ($current_is_sig !== $new_is_sig) {
				trigger_localised_error('SYS0256', E_USER_WARNING, $linkid);
				$ms->abortLog();
				return false;
			}

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
				if (($err_msg = $this->canLinkToType($major, $minor->type(), $link_type)) !== true) {
					trigger_localised_error('SYS0255', E_USER_WARNING, $linkid, $err_msg);
					$ms->abortLog();
					return false;
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
					return false;
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
			return true;
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

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


		if ($link_type_changed) {
			// if this is a web path link or if the old link type was,
			// then we need to inform the asset to update it's lookups
			if (($link_type & SQ_SC_LINK_WEB_PATHS) || ((int) $link['link_type'] & SQ_SC_LINK_WEB_PATHS)) {
				if (!$minor->updateLookups()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$ms->abortLog();
					return false;
				}
			}
		}

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

		}//end if sort order changed

		// tell, the asset it has updated
		if (!$major->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$ms->abortLog();
			return false;
		}
		if (!$minor->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$ms->abortLog();
			return false;
		}

		$ms->closeLog();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=false)
	{
		if ($linkid == 0) return false;

		// first, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_localised_error('SYS0137', E_USER_WARNING, $linkid);
			return false;
		}

		$major = &$this->getAsset($link['majorid'], $link['major_type_code']);
		$minor = &$this->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($major) || is_null($minor)) {
			return false;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// to delete a link we need to lock the old parent, but we need to make sure
			// we dont unlock it if it was already locked
			$lock_info = @$this->getLockInfo($major->id, 'links');
			$parent_was_locked = !empty($lock_info);
			if (!$this->acquireLock($major->id, 'links')) {
				trigger_localised_error('SYS0126', E_USER_WARNING, $major->name);
				return 0;
			}
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {
			// check if we are allowed to delete this link
			if (($err_msg = $major->canDeleteLink($linkid)) !== true) {
				trigger_localised_error('SYS0302', E_USER_WARNING, $err_msg);
				return false;
			}
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$minor = &$this->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($minor)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if this is a significant link
		if ($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) {

			$num_other_links = $this->countLinks($minor->id, 'minor', SQ_SC_LINK_SIGNIFICANT, '', true, $linkid);

			// we create a new link to the trash if this is the last significant link
			// being deleted and if we are not purging the trash. We check the moving
			// flag so that if we are the last significant link, we dont create a link
			// to the trash as the move operation will re-create the link elsewhere
			if (!$moving && !$num_other_links && !$GLOBALS['SQ_PURGING_TRASH']) {

				if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LINK_INTEGRITY)) {

					// some assets may not be able to have their last significant link
					// deleted, so lets check first before going ahead
					if (!$minor->canDelete()) {
						trigger_localised_error('SYS0069', E_USER_WARNING, $minor->name);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

					$trash_folder = &$this->getSystemAsset('trash_folder');
					if (is_null($trash_folder)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

					// this asset is destined for the trash, so we need to set its permissions to
					// whatever it is inheriting right now so it keeps them while in the trash
					$perms = Array(SQ_PERMISSION_ADMIN, SQ_PERMISSION_WRITE, SQ_PERMISSION_READ);
					foreach ($perms as $perm) {
						$all_permissions = $this->getPermission($minor->id, $perm, null, false, false, true);
						foreach ($all_permissions as $userid => $granted) {
							// we are deliberatly not checking for the return value here
							// because if for some reason the permission can't be set that is really just
							// bad luck
							@$this->setPermission($minor->id, $userid, $perm, $granted);
						}
					}

					if (!$trash_folder->createLink($minor, SQ_LINK_TYPE_2)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
				}
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

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// we can delete all the links under these nodes because it will be a clean start
			// when we insert into the gap's we create below

			$sub_where = '	pt.linkid	= '.$db->quoteSmart($linkid).'
						AND	ct.treeid	LIKE pt.treeid || '.$db->quoteSmart('%').'
						AND	ct.treeid	> pt.treeid';

			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'pt');
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'ct');

			$sql = 'DELETE FROM
						sq_ast_lnk_tree
					WHERE
						treeid in
						(
							SELECT
								ct.treeid
							FROM
								'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt, '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct
							'.$sub_where.'
						)';

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// we are going to set the treeid nodes that this link is associated
			// with to zero so that we can find it as a gap when we createLink() later on

			$sql = 'UPDATE
						sq_ast_lnk_tree
					SET
						linkid		= '.$db->quoteSmart('0').',
						num_kids	= '.$db->quoteSmart('0').'
					WHERE
						linkid = '.$db->quoteSmart($linkid);

			$result = $db->query($sql);
			assert_valid_db_result($result);

		}//end if significant link

		// move 'em up, higher

		$sql = 'UPDATE
					sq_ast_lnk
				SET
					sort_order = sort_order - 1
				WHERE
						majorid		= '.$db->quoteSmart($major->id).'
					AND	sort_order	> '.$db->quoteSmart($link['sort_order']);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'DELETE FROM
					sq_ast_lnk
				WHERE
						linkid	= '.$db->quoteSmart($linkid).'
					AND	majorid	= '.$db->quoteSmart($major->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// tell the assets that they have been updated
		if (!$major->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$minor->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			// release locks depending on if the parent was locked before the link
			if (!$parent_was_locked) {
				if (!$this->releaseLock($major->id, 'links')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'major_name'	=> $major->name,
						'minor_name'	=> $minor->name,
					);
		$message = $ms->newMessage(Array(), 'asset.linking.delete', $msg_reps);
		$message->parameters['majorid'] = $major->id;
		$message->parameters['minorid'] = $minor->id;
		$message->parameters['linkid']  = $linkid;
		$message->send();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($major, 'DeleteLink', Array('linkid' => $linkid));

		return true;

	}//end deleteAssetLink()


	/**
	* See whether we COULD send the specified asset to the trash IF we got the locks on its parents
	*
	* @param int	$assetid	The asset we want to trash
	*
	* @return bool|array	TRUE if we can trash, FALSE if assetid=0, Array(linkid=>problem) otherwise
	* @access public
	*/
	function couldTrashAsset($assetid)
	{
		if (!$assetid) return false;
		// We can trash the asset if we can delete all the parent links
		$msgs = Array();
		$parent_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', true, 'minor');
		foreach ($parent_links as $id => $link_details) {
			$asset =& $this->getAsset($link_details['majorid']);
			if (true !== ($msg = $asset->couldDeleteLink($link_details['linkid']))) {
				$msgs[$link_details['linkid']] = $msg;
			}
		}
		return empty($msgs) ? true : $msgs;

	}//end couldTrashAsset()


	/**
	* Send the specified asset directly to the trash, where possible
	*
	* This deletes its links to all its current parents, and creates a link for it in the trash folder
	*
	* @param int	$assetid	the asset to trash
	*
	* @return boolean
	* @access public
	*/
	function trashAsset($assetid)
	{
		// trash requests are charged at $4.95 a minute
		// please ask your parents for permission before trashing
		$parent_links = $this->getLinks($assetid, SQ_SC_LINK_ALL, '', true, 'minor');
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
			if (true !== ($msg = $asset->canDeleteLink($link_details['linkid']))) {
				trigger_localised_error('SYS0120', E_USER_WARNING, $assetid, $link_details['majorid'], $msg);
				foreach ($locked as $aid) {
					$this->releaseLock($aid, 'links');
				}
				return false;
			}
		}

		foreach ($parent_links as $link_details) {
			// deleteAssetLink will create the trash link when no other links remain
			if (!$this->deleteAssetLink($link_details['linkid'])) {
				trigger_localised_error('SYS0152', $link_details['linkid']);
				foreach ($locked as $aid) {
					$this->releaseLock($aid, 'links');
				}
				return false;
			}
		}
		foreach ($locked as $aid) {
			$this->releaseLock($aid, 'links');
		}
		return true;

	}//end trashAsset()


//--        PERMISSIONS        --//


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : null = all, true = granted, false = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is false)
	* @param boolean	$all_info		when false fn just returns an array of userids for
	*									those that have permission
	*									When true fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are false)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=null, $and_greater=true, $expand_groups=false, $all_info=false)
	{
		$permission = (int) $permission;
		if (!is_null($granted)) $granted = (bool) $granted;

		if (($and_greater || $expand_groups) && $all_info) {
			trigger_localised_error('SYS0273', E_USER_NOTICE, __CLASS__, __FUNCTION__);
			$all_info = false;
		}

		if (!isset($this->_tmp['permission_cache'])) {
			$this->_tmp['permission_cache'] = Array();
		}
		if (!isset($this->_tmp['permission_cache'][$assetid])) {
			$this->_tmp['permission_cache'][$assetid] = Array();
		}

		// return cached version if we can
		$storage_name =(($and_greater) ? 'effective_' : '').'permission_'.$permission;
		if (!isset($this->_tmp['permission_cache'][$assetid][$storage_name])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = '  SELECT DISTINCT assetid, userid, granted
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_perm ';
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

			if ($granted === false && $data['granted'] != '0') {
				continue;
			} else if ($granted === true  && $data['granted'] != '1') {
				continue;
			}

			// return all info
			if ($all_info) {
				$ret_val[$data['userid']] = $data['granted'];

			// else we just want user ids
			} else {

				// if we are expanding user groups and we aren't public access
				if ($expand_groups && $data['userid']) {
					$user = &$this->getAsset($data['userid']);
					if (!is_null($user)) {
						if (is_a($user, 'user_group')) {
							$ret_val = array_merge($ret_val, array_keys($this->getChildren($user->id, Array('user'), false)));
						} else {
							$ret_val[] = $user->id;
						}
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
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted)
	{
		$permission = (int)  $permission;
		$granted    = (bool) $granted;

		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) return false;
		if (!$asset->adminAccess('permissions')) {
			trigger_localised_error('SYS0111', E_USER_WARNING, $asset->name);
			return false;
		}

		// check if this permission is already set
		$current = $this->getPermission($assetid, $permission, $granted, false);
		if (in_array($userid, array_values($current))) {
			return true;
		}

		// if we dont have an userid, we are granting public access
		if (!empty($userid)) {
			// check that the passed userid is a user or user_group
			$user = &$this->getAsset($userid, '', true);
			if (!$user->id) {
				trigger_localised_error('SYS0112', E_USER_WARNING);
				return false;
			} else if (!is_a($user, 'user') && !is_a($user, 'user_group')) {
				trigger_localised_error('SYS0113', E_USER_WARNING, $user->type());
				return false;
			}
			$user_name = $user->name;
		} else {
			$user_name = 'General Public';
		}

		// check if the reverse access of this permission is set
		$current = $this->getPermission($assetid, $permission, !$granted, false);
		if (in_array($userid, array_values($current))) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$perm_name = permission_type_name($permission);

			$new_access     = ($granted) ? 'grant'   : 'revoke';
			$current_access = ($granted) ? 'revoked' : 'granted';

			trigger_localised_error('SYS0123', E_USER_WARNING, $new_access, $perm_name, $user_name, $asset->name, $current_access);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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

		$result = $db->query($sql);
		assert_valid_db_result($result);

		if (!$asset->permissionsUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// clear the permission cache
		if (isset($this->_tmp['permission_cache'][$assetid])) {
			unset($this->_tmp['permission_cache'][$assetid]);
		}

		// log the action
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'perm_name'		=> permission_type_name($permission),
						'asset_name'	=> $asset->name,
						'user_name'		=> $user_name,
					);
		$message = $ms->newMessage(Array(), 'asset.permissions.'.($granted ? 'grant' : 'deny'), $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$message->send();

		return true;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$assetid		the assetid for asset whose permissions to delete
	* @param int	$userid			the id of the user or user_group to remove the permission from
	* @param string	$permission		the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $userid, $permission)
	{
		$permission = (int) $permission;

		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) return false;
		if (!$asset->adminAccess('permissions')) {
			trigger_localised_error('SYS0104', E_USER_WARNING, $asset->name);
			return false;
		}

		// if we dont have an userid, we are deleting public access
		if (!empty($userid)) {
			// if the permission is in the database then we know that it must have
			// been set up correctly, and if an user or LDAP user's asset has been deleted then we
			// know we want it out. So we'll use getAssetInfo() instead which does not throw an
			// assertion, just instead just gives an empty array
			$user_info = $this->getAssetInfo(Array($userid), 'user', false);

			// check that the passed userid is a user or user_group
			if (empty($user_info)) {
				$user_name = 'Unknown User';
			} else {
				$user_name = $user_info[$userid]['name'];
			}
		} else {
			$user_name = 'General Public';
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// clear the permission cache
		if (isset($this->_tmp['permission_cache'][$assetid])) {
			unset($this->_tmp['permission_cache'][$assetid]);
		}

		// log the action
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		$msg_reps = Array(
						'perm_name'		=> permission_type_name($permission),
						'asset_name'	=> $asset->name,
						'user_name'		=> $user_name,
					);
		$message = $ms->newMessage(Array(), 'asset.permissions.delete', $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$message->send();

		return true;

	}//end deletePermission()


//--        URL's & HREF's        --//


	/**
	* Returns a reference to the asset that has the passed url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol		the protocol to match -> null means it is ignored
	* @param string		$url			the urls to check for -> null defaults it to current url
	* @param boolean	$exact_match	when true only returns an asset if there is an exact
	*									match on the url,
	*									if false searches back along the URL path for the
	*									asset that matches
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object Asset or null
	* @access public
	*/
	function &getAssetFromURL($protocol=null, $url=null, $exact_match=true, $mute_errors=false)
	{
		if (is_null($url)) $url = current_url(false, true);
		if (!is_string($url)) {
			trigger_localised_error('SYS0189', E_USER_WARNING);
			$null = null;
			return $null;
		}

		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') {
			$protocol = 'http';
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
			$null = null; // 'cause we are meant to return by reference
			return $null;

		} else {
			return $this->getAsset($row['assetid'], '', $mute_errors);
		}//end if

	}//end getAssetFromURL()


	/**
	* Returns the information from the sq_asset_url table for the root url closest to the passed URL
	* i.e. returns the info for the site closest to this url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string	$url	the urls to check for (excluding protocol) -> null defaults it to current url
	*
	* @return Array
	* @access public
	*/
	function getRootURL($url=null)
	{
		if (is_null($url)) $url = current_url(false, true);
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
	function getLineageFromURL($protocol=null, $url=null)
	{
		if (is_null($url)) $url = current_url(false, true);
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') {
			$protocol = 'http';
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$non_standard_url = false;
		// get the root(site) url for this url
		$root_url = $this->getRootURL($url);
		if (empty($root_url)) {
			$found = false;
			$non_standard_url = true;
			//We want to check if this is an asset (file) with a publicly served url
			$fancy_asset =& $this->getAssetFromURL($protocol, $url);
			if (empty($fancy_asset)) {
				$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
				trigger_localised_error('SYS0247', E_USER_WARNING, $fancy_url);
				return Array();
			}
			$lineages = $this->getLinkLineages($fancy_asset);

			// Determine which site we are in, and find one of our lineages that lies in that site
			$all_sites = $this->getParents($fancy_asset->id, 'site');
			$valid_sites = Array();
			foreach ($all_sites as $id => $type_code) {
				$current_site =& $this->getAsset($id);
				$site_url = $original_site_url = $current_site->getUrl();
				$original_site_url = $site_url = str_replace(Array('http://', 'https://'), '', $site_url);
				$valid = false;
				if (strpos($url, $site_url) !== false) {
					$valid = true;
				} else {
					while (strrpos($site_url, '/') !== false) {
						$site_url = substr($site_url, 0, strrpos($site_url, '/'));
						if (strpos($url, $site_url) !== false) {
							$valid = true;
							break;
						}
					}
				}
				//We found one, add it to the array
				if ($valid) $valid_sites[$id] = $original_site_url;
			}

			$root_id = 0;
			if (!empty($valid_sites)) {
				if (count($valid_sites) == 1) {
					//Only 1 valid site, lets go with it
					$root_id = current(array_keys($valid_sites));
				} else {
					//More than one site, we need more filtering

					//Go on current url?
					$current_site = $this->getRootUrl(current_url(false, true));
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

			//Lets use our parent's url to get our lineage
			$parent_url = str_replace(Array('http://', 'https://'), '', $parent_url);

			if (empty($parent_url)) {
				return Array();
			}

			//Now lets set the url to our parents url, and continue with the rest of the function as normal
			$root_url = $this->getRootURL($parent_url);

			$url = $parent_url;

			if (empty($root_url)) {
				$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
				trigger_localised_error('SYS0247', E_USER_WARNING, $fancy_url);
				return Array();
			}
		}// end empty $root_url foreach

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

			for (reset($result); null !== ($key = key($result)); next($result)) {
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
			$protocol_select = $db->quote($protocol).' as protocol';
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

		$url_asset = &$this->getAssetFromURL($protocol, $url);
		$id_parts = explode(':', $url_asset->id);
		if (isset($id_parts[1])) {

			$real_assetid = $id_parts[0];
			$asset = &$this->getAsset($real_assetid);
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
				$site = &$this->getAsset($result[0]['assetid'], $result[0]['type_code']);
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

		return $result;

	}//end getLineageFromURL()


	/**
	* Return the design for the passed URL
	*
	* @param string		$url			the url to get the design for
	* @param string		$design_name	the code for the design (eg. design::system::frontend)
	*
	* @return array(designid, type_code)
	* @access public
	*/
	function getDesignFromURL($url, $design_name='design::system::frontend')
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
	* Return the lookup value for the passed URL
	*
	* @param string		$url		the url to get the design for
	* @param string		$value_name	the code for the design (eg. design::system::frontend)
	*
	* @return string	the value
	* @access public
	*/
	function getValueFromURL($url, $value_name)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
	* Returns the URL for an asset, closest to the $base_url
	*
	* @param int|array(int)		$assetids			the id of the asset you want the url for or an array
	*												for a whole bunch of them
	* @param string				$base_url			the url that we are want to match closest to
	* @param boolean			$ignore_rollback	ignore rollback checks
	*
	* @return mixed returns string if $assetids is an int and an array(int => string) if $assetids is an array
	* @access public
	*/
	function getAssetURL($assetids, $base_url=null, $ignore_rollback=false)
	{
		if (!isset($this->_tmp['urls'])) {
			$this->_tmp['urls'] = Array();
		}

		$return_array = true;
		$in_assetid = 0;
		if (!is_array($assetids)) {
			if (empty($assetids)) return '';
			$in_assetid = $assetids;
			$assetids = Array($assetids);
			$return_array = false;
		} else {
			if (empty($assetids)) return Array();
		}

		$return_urls = Array();

		if (!$ignore_rollback && SQ_ROLLBACK_VIEW) {
			if ($return_array) {
				foreach ($assetids as $assetid) {
					$asset = &$this->getAsset($assetid);
					$return_urls[$assetid] = $asset->getURL($base_url, true);
				}
				return $return_urls;
			} else {
				$asset = &$this->getAsset($assetids[0]);
				return $asset->getURL($base_url, true);
			}
		}

		if (is_null($base_url)) {
			$base_url = current_url(false, true);
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$the_rest = array_diff($assetids, array_keys($return_urls));

		if (!empty($the_rest)) {
			$assetid_cond_array = $the_rest;
			for (reset($assetid_cond_array); NULL !== ($k = key($assetid_cond_array)); next($assetid_cond_array)) {
				$assetid_cond_array[$k] = $db->quote($assetid_cond_array[$k]);
			}
			$assetid_cond = 'IN ('.implode(', ', $assetid_cond_array).')';

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
			ORDER BY weighting DESC, l.url';

			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
			assert_valid_db_result($result);

			foreach ($the_rest as $assetid) {
				if (!isset($result[$assetid])) {
					$this->_tmp['urls'][$assetid][$base_url] = '';
				} else {
					$this->_tmp['urls'][$assetid][$base_url] = $result[$assetid][0]['protocol'].'://'.$result[$assetid][0]['url'];
				}
				$return_urls[$assetid] = $this->_tmp['urls'][$assetid][$base_url];
			}
		}

		if ($return_array) {
			return $return_urls;
		} else {
			return $return_urls[$in_assetid];
		}

	}//end getAssetURL()


	/**
	* Returns the href for an asset closest to the $base_url, but relative to the current url
	*
	* @param int|array(int)		$assetids			the id of the asset you want the href for or an array
	*												for a whole bunch of them
	* @param string				$base_url			the url that we are want to match closest to
	* @param boolean			$ignore_rollback	ignore rollback checks
	*
	* @return mixed returns string if $assetids is an in and an array(int => string) if $assetids is an array
	* @access public
	*/
	function getAssetHref($assetids, $base_url=null, $ignore_rollback=false)
	{
		$return_array = true;
		$in_assetid = 0;
		if (!is_array($assetids)) {
			if (empty($assetids)) return '';
			$in_assetid = $assetids;
			$assetids = Array($assetids);
			$return_array = false;
		} else {
			if (empty($assetids)) return Array();
		}

		$return_hrefs = Array();

		if (!$ignore_rollback && SQ_ROLLBACK_VIEW) {
			if ($return_array) {
				foreach ($assetids as $assetid) {
					$asset = &$this->getAsset($assetid);
					$return_hrefs[$assetid] = $asset->getHref($base_url, true);
				}
				return $return_hrefs;
			} else {
				$asset = &$this->getAsset($assetids[0]);
				return $asset->getHref($base_url, true);
			}
		}

		if (is_null($base_url)) $base_url = current_url(false);

		if (is_null($base_url)) $base_url = current_url(false);
		$base_url = strip_url($base_url, true);

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
	* @access public
	* @return void
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
	* @param array(mixed)	$asset_info	an array of asset info in the form assetid => screen code name
	*
	* @return array(int => string) returns an array of assetid and hrefs to the wanted screen
	* @access public
	*/
	function getAssetBackendHref($asset_info)
	{
		if (empty($asset_info)) return Array();

		$current_url_complete = substr(current_url(), -1) == '/';
		$return_hrefs = Array();
		foreach ($asset_info as $assetid => $screen) {
			$href = '';
			if (SQ_IN_LIMBO) {
				$href = sq_web_path('edit').'/?asset_ei_screen='.$screen;
			} else {
				$href = sq_web_path('admin').'/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.$assetid.'&asset_ei_screen='.$screen;
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
	* @param object Asset	$major					the major asset, whose children we are to check
	* @param array			$paths					array of web paths to check
	* @param int			$ignore_assetid			ignore this assetid when checking
	* @param boolean		$return_alternatives	return all paths but replace the bad paths
	*												with a valid alternative
	*
	* @return array(string)
	* @access protected
	*/
	function webPathsInUse(&$major, $paths, $ignore_assetid=0, $return_alternatives=false)
	{
		if (empty($paths)) return Array();
		$return_paths = Array();

		// db_extras_bitand
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

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
			$where = 'WHERE l.majorid = '.$major->id.' ';
			if ($ignore_assetid) {
				$where .= 'AND l.minorid <> '.$ignore_assetid.' ';
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
		}

		return $return_paths;

	}//end webPathsInUse()


//--        PAINTING        --//


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @param object Backend &$backend
	*
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
	* 											(eg 'Page', 'File', etc)
	* 											if an array returns link if matches any
	* 											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
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
	* 											Specify that links should be returned for assets the user has read,
	* 											write, or admin access for. Leave NULL to ignore permissions.
	*
	* @return array
	* @access public
	*/
	function generateGetLinksQuery($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null, $dependant=null, $exclusive=null, $sort_by=null, $access=null)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		assert_valid_assetid($assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$select = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code as '.$other_side.'_type_code, l.sort_order, l.is_dependant, l.is_exclusive';
		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l,
						'.SQ_TABLE_RUNNING_PREFIX.'ast a';

		$where  = 'l.'.$other_side.'id = a.assetid
				   AND l.'.$side_of_link.'id = '.$db->quote($assetid).'
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
			$userid_cond = '';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$from .= ', '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p';
				$where .= ' AND p.assetid = a.assetid';
				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$userids[] = $this->getSystemAssetid('public_user');
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote($userids[$i]);
				}
				$userid_cond = ' AND p.userid  IN ('.implode(',', $userids).')';
				$where .= $userid_cond;
				$having .= 'HAVING MIN(p.granted) <> 0';
			}

			$group_by = 'GROUP BY l.linkid, l.'.$other_side.'id, l.value, l.link_type,
								  a.type_code, l.sort_order, l.is_dependant, l.is_exclusive';
		}

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); null !== ($i = key($type_code)); next($type_code)) {
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

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		if (is_null($sort_by)) {
			// order by sort order (by default)
			$order_by = 'ORDER BY l.sort_order, a.type_code';
		} else {
			// order by a specific field in the asset table
			$order_by = 'ORDER BY a.'.$sort_by;
			if ($sort_by != 'type_code') {
				$select .= ', a.'.$sort_by.' AS sort_value';
				if (!empty($group_by)) $group_by .= ', a.'.$sort_by;
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
	* @param object Asset	&$asset				the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	* @param boolean		$inc_type_code		whether to include the type code in the SELECT or not
	*
	* @return array		returns an array of parts of the SQL statment
	* @access public
	* @see getChildren()
	*/
	function generateGetChildrenQuery(&$asset, $type_code='', $strict_type_code=true, $sort_by=null, $access=null, $inc_type_code=true)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if we are the root folder then we just need to select from the asset table
		if (get_class($asset) == 'root_folder') {
			$select = 'SELECT a.assetid'.(($inc_type_code) ? ', a.type_code' : '');
			$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'a');
			if (!empty($where) && (!empty($type_code) || !is_null($access))) {
				$where .= ' AND ';
			}

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

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			if (!empty($type_code) || !is_null($access)) {
				$where .= ' AND ';
			}

		}//end if assetid == 1

		$group_by = '';
		if (!is_null($access)) {
			$from .= ' INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON a.assetid = p.assetid';
			$where .= 'p.assetid = a.assetid';
			$userid_cond = '';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$userids[] = $this->getSystemAssetid('public_user');
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote($userids[$i]);
				}
				$userid_cond = ' AND p.userid  IN ('.implode(',', $userids).')';
			}
			$where .= $userid_cond.' AND p.granted >= '.$db->quote($access);
			if (!empty($type_code)) $where .= ' AND ';
		}


		if (!empty($type_code)) {

			if (empty($where)) $where = ' WHERE ';

			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); null !== ($i = key($type_code)); next($type_code)) {
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
			$order_by = 'ORDER BY a.'.$sort_by;
			$select .= ', a.'.$sort_by.' AS sort_value';
		}

		return Array(
				'select'	=> $select,
				'from'		=> $from,
				'where'		=> $where,
				'order_by'	=> $order_by,
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
	*
	* @return array(int)
	* @access public
	* @see getParents()
	*/
	function generateGetParentsQuery($assetid, $type_code='', $strict_type_code=true, $sort_by=null, $access=null)
	{
		assert_valid_assetid($assetid);

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetid = (string) $assetid;

		$in_sql = SQ_TABLE_RUNNING_PREFIX.'get_parent_treeids('.$db->quoteSmart($assetid).', '.$db->quoteSmart(SQ_CONF_ASSET_TREE_SIZE).')';

		if ($db->phptype == 'oci8') {
			$in_sql = 'SELECT * FROM table('.$in_sql.')';
			$result = $db->getCol($in_sql);
			assert_valid_db_result($result);

			if (empty($result)) return Array();

			for (reset($result); null !== ($key = key($result)); next($result)) {
				$result[$key] = $db->quoteSmart($result[$key]);
			}

			$in_sql = implode(',', $result);
		} else {
			$in_sql = 'SELECT * FROM '.$in_sql;
		}

		$select = 'SELECT DISTINCT(l.majorid), a.type_code, t.treeid';

		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t ON l.linkid = t.linkid
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON l.majorid = a.assetid';
		$where  ='t.treeid IN (
								'.$in_sql.'
							)';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		if (!empty($type_code)) {
			$type_code_cond = '';
			if (is_array($type_code)) {
				for (reset($type_code); null !== ($i = key($type_code)); next($type_code)) {
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

		// order by a field if required
		$order_by = '';
		if (!is_null($sort_by)) {
			$order_by = 'ORDER BY a.'.$sort_by;
			$select .= ', a.'.$sort_by.' AS sort_value';
		} else {
			$order_by = 'ORDER BY t.treeid';
		}

		$group_by = '';
		if (!is_null($access)) {
			$from .= ','.SQ_TABLE_RUNNING_PREFIX.'ast_perm p';
			$where .= ' AND a.assetid = p.assetid';
			$userid_cond = '';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = array_keys($this->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
				$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$userids[] = $this->getSystemAssetid('public_user');
				for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote($userids[$i]);
				}
				$userid_cond = ' AND p.userid  IN ('.implode(',', $userids).')';
			}
			$where .= $userid_cond.' AND p.granted >= '.$db->quote($access);
		}

		return Array(
				'select'	=> $select,
				'from'		=> $from,
				'where'		=> $where,
				'order_by'	=> $order_by,
			   );

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
