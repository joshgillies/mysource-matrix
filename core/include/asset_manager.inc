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
* $Id: asset_manager.inc,v 1.164.2.1 2004/02/18 11:39:06 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Asset_Manager
*
* Purpose
*    Holds information on installed packages, as represented by the DB
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* this array holds the references to the different types of objects in the system
	* so that there will only ever (with luck :) be one instance of an Asset
	*
	* @var Array(Asset)
	*/
	var $_assets = Array();


	/**
	* Constructor
	*
	*/
	function Asset_Manager()
	{
		$this->MySource_Object();
		if (empty($GLOBALS['SQ_INSTALL'])) $this->_loadAssetTypes();

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

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT type_code, version, name, instantiable, allowed_access, parent_type, dir, customisation
				FROM '.SQ_TABLE_PREFIX.'asset_type';
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		while (null !== ($row = $result->fetchRow())) {
			$this->_asset_types[$row['type_code']] = $row;
		}//end while

		$result->free();

	}//end _loadAssetTypes()


	/**
	* Refreshes the passed asset types data in the assets array
	* Also updates the sq_asset_type_inherited table so as to have all
	* links pointing correctly
	*
	* @param string	$type_code	the code name for the asset type that you want to refresh
	*
	* @access public
	*/
	function refreshAssetType($type_code)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT type_code, version, name, instantiable, allowed_access, parent_type, dir, customisation
				FROM '.SQ_TABLE_PREFIX.'asset_type
				WHERE type_code = '.$db->quote($type_code);
		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (empty($result)) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return;
		} else {
			$this->_asset_types[$result['type_code']] = $result;
		}

		$parents = Array($type_code);
		$tmp_type_code = $type_code;
		while($this->_asset_types[$tmp_type_code]['parent_type'] != 'asset') {
			// this should NEVER happen, if it does DIE
			if (!isset($this->_asset_types[$this->_asset_types[$tmp_type_code]['parent_type']])) {
				trigger_error('Asset "'.$this->_asset_types[$tmp_type_code]['parent_type'].'" is not installed on the system,<br/>'
							 .'therefore we cannot get the parent type of Asset "'.$tmp_type_code.'".<br/>'
							 .'Check that you got the Asset_Manager by reference', E_USER_ERROR);
			}
			$tmp_type_code = $this->_asset_types[$tmp_type_code]['parent_type'];
			$parents[] = $tmp_type_code;
		}
		$parents[] = 'asset';

		$sql = 'SELECT inherited_type_code
				FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
				WHERE type_code = '.$db->quote($type_code);
		$db_parents = $db->getCol($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$inserts = array_diff($parents, $db_parents);
		$deletes = array_diff($db_parents, $parents);
		$updates = array_intersect($parents, $db_parents);

		$type_code_level = (int) $this->getTypeInfo($type_code, 'level');

		foreach($inserts as $inherited_type_code) {
			$inherited_type_code_level = ($inherited_type_code == 'asset') ? 0 : (int) $this->getTypeInfo($inherited_type_code, 'level');
			$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'asset_type_inherited
					(inherited_type_code, type_code, inherited_type_code_level, type_code_level)
					VALUES
					('.$db->quote($inherited_type_code).', '.$db->quote($type_code).',
					'.$db->quote($inherited_type_code_level).', '.$db->quote($type_code_level).')';
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		foreach($deletes as $inherited_type_code) {
			$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
					WHERE inherited_type_code = '.$db->quote($inherited_type_code).'
					  AND type_code = '.$db->quote($type_code);
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		foreach($updates as $inherited_type_code) {
			$inherited_type_code_level = ($inherited_type_code == 'asset') ? 0 : (int) $this->getTypeInfo($inherited_type_code, 'level');
			$sql = 'UPDATE '.SQ_TABLE_PREFIX.'asset_type_inherited
					SET inherited_type_code_level = '.$db->quote($inherited_type_code_level).',
						type_code_level = '.$db->quote($type_code_level).'
					WHERE inherited_type_code = '.$db->quote($inherited_type_code).'
					  AND type_code = '.$db->quote($type_code);
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

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
	* @param Array|string	$type_code	the code name (or array of code names) for the asset that you want to check
	* @param string			$field		if exists, returns this information from the array for this specific field
	*
	* @return mixed
	* @access public
	*/
	function getTypeInfo($type_code, $field='')
	{
		$do_query = false;

		// multiple typecodes
		if (is_array($type_code)) {

			if (empty($type_code)) return Array();

			// we want to create a query with only the type_codes that
			// we do not have cached, but keep the type_code array intact for
			// looping through later
			$tmp = $type_code;

			// if there are multiple type_codes we need to select the type_code as well
			// as the field from the database so we know what field belongs to what typecode
			$columns = 'type_code, '.$field;

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			for($i = 0; $i < count($type_code); $i++) {
				if (!isset($this->_asset_types[$type_code[$i]])) {
					trigger_error('Asset "'.$type_code[$i].'" is not installed on the system', E_USER_WARNING);
					unset($type_code[$i]);
					unset($tmp[$i]);
					continue;
				} else {
					if ($field && !isset($this->_asset_types[$type_code[$i]][$field])) {
						$do_query = true;
						$tmp[$i] = $db->quote($type_code[$i]);
					} else {
						unset($tmp[$i]);
					}
				}
			}// end for
			$type_code_cond = 'IN ('.implode(', ', $tmp).')';

		// just a single typecode
		} else {
			$columns = $field;

			if (!isset($this->_asset_types[$type_code])) {
				trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
				return ($field) ? null : Array();
			}
			if ($field && !isset($this->_asset_types[$type_code][$field])) {
				$do_query = true;
				$db = &$GLOBALS['SQ_SYSTEM']->db;
				$type_code_cond = '= '.$db->quote($type_code);
			}
		}// end if

		if ($do_query) {

			$sql = 'SELECT '.$columns.'
					FROM '.SQ_TABLE_PREFIX.'asset_type
					WHERE type_code '.$type_code_cond;
			$result = $db->getAll($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			if (is_array($type_code)) {
				for ($i = 0; $i < count($result); $i++) {
					$this->_asset_types[$result[$i]['type_code']][$field] = $result[$i][$field];
				}
			} else {
				$this->_asset_types[$type_code][$field] = $result[0][$field];
			}

		}// end if

		if (is_array($type_code)) {
			$type_codes = Array();
			foreach ($type_code as $code) {
				$type_codes[$code] = ($field) ? $this->_asset_types[$code][$field] : $this->_asset_types[$code];
			}
			return $type_codes;
		}
		return ($field) ? $this->_asset_types[$type_code][$field] : $this->_asset_types[$type_code];

	}//end getTypeInfo()

	/**
	* Retrieves the default component weightings for an asset type
	*/
	function getTypeDefaultWeightings($type_code)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$default_weightings = Array();

		if (is_array($type_code)) {
			$type_codes = $type_code;
			// load the weightings
			$sql = 'SELECT type_code, dw.component, dw.weighting, dw.description
					FROM '.
						SQ_TABLE_PREFIX. 'asset_type at JOIN '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default dw
							ON at.type_code = dw.type_code
					WHERE
						at.type_code IN (';
			$first = true;
			foreach ($type_codes as $type_code) {
				if (!$first)
					$sql .= ", ";

				$first = false;

				$sql .= $db->quote($type_code);
			}
			$sql .= ')';

			$results = $db->getAll($sql);

			$default_weightings = Array();
			foreach ($results as $result) {
				$default_weightings[$result['type_code']] = Array();
				$default_weightings[$result['type_code']][$result['component']] =
					Array('default' => 1, 'weighting' => $result['weighting'], 'description' => $result['description']);
			}
		} else {
			if ($type_code != '') {
				// load the weightings for this type code only
				$sql = 'SELECT dw.component, dw.weighting, dw.description
						FROM '.
							SQ_TABLE_PREFIX. 'asset_type at JOIN '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default dw
								ON at.type_code = dw.type_code
						WHERE
							at.type_code = '. $db->quote($type_code);

				$results = $db->getAll($sql);
				foreach ($results as $result) {
					$default_weightings[$result['component']] =
						Array('default' => 1, 'weighting' => $result['weighting'], 'description' => $result['description']);
				}

			}
		}

		return $default_weightings;
	}//end getTypeDefaultWeightings()


	/**
	* Returns an array representing the class hierarchy for the asset types, either for whole system
	* or for under the the passed asset type
	*
	* @param string		$base_type_code			the asset type's hierarchy to return
	* @param boolean	$ignore_allowed_access
	*
	* @return array
	* @access public
	*/
	function getAssetTypeHierarchy($base_type_code='', $allowed_access=false)
	{
		if (empty($base_type_code)) $base_type_code = 'asset';

		$type_codes = Array();
		$allowed_access = trim($allowed_access);
		if (!empty($ignore_allowed_access)) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT type_code
					FROM '.SQ_TABLE_PREFIX.'asset_type
					WHERE allowed_access = '.$db->quote($allowed_access);
			$type_codes = $db->getCol($sql);
			if (DB::isError($type_codes)) trigger_error($type_codes->getMessage().'<br/>'.$type_codes->getUserInfo(), E_USER_ERROR);
		} else {
			$type_codes = array_keys($this->_asset_types);
		}

		$offspring = Array();
		foreach($type_codes as $type_code) {
			$parent = $this->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}//end for

		$hier = $this->_recurseGetAssetTypeHierarchy($offspring, $base_type_code);

		return $hier;

	}//end getAssetTypeHierarchy()

	function _recurseGetAssetTypeHierarchy($offspring, $base_type_code)
	{
		if (empty($offspring[$base_type_code])) return Array();
		$arr = Array();
		for($i = 0; $i < count($offspring[$base_type_code]); $i++) {
			$type = $offspring[$base_type_code][$i];
			$arr[$type] = Array('name' => $this->_asset_types[$type]['name'],
								'subs' => $this->_recurseGetAssetTypeHierarchy($offspring, $type)
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
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where))
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT a.assetid, at.inherited_type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a INNER JOIN '.SQ_TABLE_PREFIX.'asset_type_inherited at ON a.type_code = at.type_code
				'.$where.' ORDER BY a.assetid';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

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
		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system, unable to include it\'s Source File', E_USER_ERROR);
		}

		require_once SQ_SYSTEM_ROOT.'/'.$this->_asset_types[$type_code]['dir'].'/'.$type_code.'.inc';

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
		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset Type "'.$type_code.'" is not installed on the system', E_USER_ERROR);
			return Array();
		}
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$result = $db->getCol('SELECT inherited_type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE type_code = '.$db->quote($type_code).'
								  AND inherited_type_code <> '.$db->quote($type_code).'
								'.(($include_asset) ? '' : ' AND inherited_type_code <> '.$db->quote('asset')).'
								ORDER BY inherited_type_code_level DESC'
								);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}
		return $result;

	}//end getTypeAncestors()


	/**
	* Returns an array of all assets types that are descendants for the passed asset
	* in the order Array ('type_code_sub_class_one', 'type_code_sub_class_two', ..., 'type_code_sub_class_one_sub_class_one', ...)
	*
	* @param string		$type_code		the code name for the asset that you want to check
	*
	* @access public
	*/
	function getTypeDescendants($type_code)
	{
		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset Type "'.$type_code.'" is not installed on the system', E_USER_ERROR);
			return Array();
		}
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$result = $db->getCol('SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code = '.$db->quote($type_code).'
								  AND type_code <> '.$db->quote($type_code).'
								ORDER BY type_code_level ASC'
								);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}
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

	}//end _validateAttributeType()


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
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

		$sql = 'SELECT '.implode(', ', $details).' FROM '.SQ_TABLE_PREFIX.'asset_attribute
				WHERE type_code = '.$db->quote($type_code);

		if (count($details) > 1) $result = $db->getAssoc($sql);
		else $result = $db->getCol($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		return $result;

	}//end getAssetTypeAttributes()


//--        ASSETS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the asset id to be loaded
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
		if (empty($assetid)) {
			$null = null; // do this because of return by reference
			return $null;
		}

		if (!isset($this->_assets[$assetid]) || !is_object($this->_assets[$assetid]['object'])) {

			$this->_assets[$assetid] = Array();
			$this->_assets[$assetid]['object'] = null;
			$this->_assets[$assetid]['count']  = 0;


			// check if we are getting links for a shadow asset, and palm the request off to the
			// handler of the shadow asset if we are
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {

				$real_assetid = $id_parts[0];
				$asset = &$this->getAsset($real_assetid, '', $mute_errors);
				$this->_assets[$assetid]['object'] = &$asset->getAsset($id_parts[1]);

			} else {

				if (empty($type_code)) {
					$where = 'assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($assetid);
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
					$type_code = $GLOBALS['SQ_SYSTEM']->db->getOne('SELECT type_code FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where);
					if (DB::isError($type_code) || empty($type_code)) {
						if (!$mute_errors) trigger_error('Asset #'.$assetid.' does not exist', E_USER_WARNING);
						return $this->_assets[$assetid]['object'];
					}
				}//end if

				if (!isset($this->_asset_types[$type_code])) {
					if (!$mute_errors) trigger_error('Asset Type "'.$type_code.'" is not installed on this system', E_USER_WARNING);
				} else {
					$this->includeAsset($type_code);
					$this->_assets[$assetid]['object'] = new $type_code($assetid);
					// make sure of the asset is correct
					if (empty($this->_assets[$assetid]['object']->id)) $this->_assets[$assetid]['object'] = null;

				}//end if

			}//end if

		}//end if

		$this->_assets[$assetid]['count']++;
		return $this->_assets[$assetid]['object'];

	}//end getAsset()


	/**
	* Registers the passed object in the store, for others to get
	* Used when a new record has just been "create()d"
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
		if (!is_object($obj) || !is_a($obj, 'asset') || !$obj->id) return;

		if (isset($this->_assets[$obj->id])) unset($this->_assets[$obj->id]);
		$this->_assets[$obj->id]['object'] = &$obj;
		$this->_assets[$obj->id]['count']  = 1;

	}//end rememberAsset();


	/**
	* Unregisters the passed object from the store
	*
	* @param object Asset	&$obj	the asset to be unregistered
	*
	* @see rememberAsset()
	* @access public
	* @return void
	*/
	function forgetAsset(&$obj)
	{
		// wrong class or no id, cya
		if (!is_object($obj) || !is_a($obj, 'asset') || !$obj->id) return;
		if (isset($this->_assets[$obj->id])) {
			if ($this->_assets[$obj->id]['count'] > 1) $this->_assets[$obj->id]['count']--;
			else unset($this->_assets[$obj->id]);
		}

	}//end forgetAsset();


	/**
	* Returns a reference to a system asset
	*
	* @param string	$name	the name of the system asset
	*						'root_folder', 'trash_folder', 'system_user_group',
	*						'root_user', 'designs_folder', 'login_design', 'cron_manager'
	*
	* @access public
	*/
	function &getSystemAsset($name)
	{
		static $system_assets = null;
		if (is_null($system_assets)) {
			// include the main system config file
			require SQ_DATA_PATH.'/private/conf/system_assets.inc';
		}
		if (isset($system_assets[$name])) {
			return $this->getAsset($system_assets[$name], $name);
		} else {
			trigger_error('System Asset Name "'.$name.'" not known', E_USER_ERROR);
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
		if ($asset->id <= SQ_NUM_RESERVED_ASSETIDS) {
			// this asset is a direct system asset
			// so we can not purge this asset
			return false;
		}

		// if we get to here, it is not one of the
		// system assets, but it may be dependantly linked to one
		// so lets check for that
		$parents = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', null, 'minor', '', 1);
		for($i = 0; $i < count($parents); $i++) if ($parents[$i]['majorid'] <= SQ_NUM_RESERVED_ASSETIDS) return false;

		return true;

	}//end canPurgeAsset()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* @param array(int)		$assetids			an array of assets to limit
	* @param array(string)	$type_code			an array of types codes to limit the results to
	* @param boolean		$strict_type_code	ensure returned assets are of the passed type codes
	*
	* @return Array( int => Array() )
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=true)
	{
		if (empty($assetids)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$where = '';

		for($i = 0; $i < count($assetids); $i++) $assetids[$i] = $db->quote($assetids[$i]);
		$where .= 'a.assetid IN ('.implode(', ', $assetids).')';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where))
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a '.$where.' ORDER BY a.assetid';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$return_result = Array();
		foreach ($result as $assetid => $row) {
			foreach ($row as $name => $value) {
				$return_result[$row['assetid']][$name] = $value;
			}
		}

		return $return_result;

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
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @access public
	*/
	function getTypeAssetids($type_code, $strict=true, $include_type=false, $mute_errors=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if ($strict) {
			$where = 'type_code = '.$db->quote($type_code);
		} else {
			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$where = 'type_code IN (~SQ0~)';
			$subs = Array( 'SELECT type_code
							FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
							WHERE inherited_type_code = '.$db->quote($type_code));
			$where = db_extras_subquery($db, $where, $subs);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT assetid'.(($include_type) ? ', type_code' : '').'
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;

		$result = ($include_type) ? $db->getAssoc($sql) : $db->getCol($sql);
		if (DB::isError($result)) {
			if (!$mute_errors) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		return $result;

	}//end getTypeAssetids()


//--        LOCKING        --//


	/**
	* Acquires a lock on an asset
	*
	* @param int		$assetid		the assetid of the asset that we want to lock
	* @param string		$lock_type		any valid lock type for the passed asset's type,
	*									plus 'all' which will attempt to acquire all locks that this asset has
	* @param int		$source_assetid	the id of the asset that started this locking process
	* @param boolean	$force			attempt to forcibly acquire (if allowed)
	* @param int|NULL	$expires		when the lock expires (timestamp or NULL)
	*									If expires is set to NULL, the lock will be acquired indefinitely (ie. it will never expire).
	*
	* @return int		zero on failue, non-zero on success (1 on success and all lock types reqeusted had to be acquired,
	*					2 on any lock was updated [ie at least one lock type was already held])
	* @access public
	*/
	function acquireLock($assetid, $lock_type, $source_assetid=0, $force=false, $expires=0)
	{
		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) return false;

		$current_locks = $this->getLockInfo($assetid, $lock_type, true, true);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$success = true;

		$can_force = ($force) ? $asset->canForceablyAcquireLock($lock_type) : false;
		$lock_updated = false;

		foreach($current_locks as $lock_type => $lock) {

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
			if(!empty($lock) && $can_force) {

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
				foreach($lock['chained_assets'] as $row) $locked_assetids[] = $row['assetid'];

				foreach ($locked_assetids as $locked_assetid) {
					$locked_asset = &$this->getAsset($locked_assetid);

					// create a new message object and populate it, before adding to
					// the message queue in the internal messaging system to sending later
					$log_body = 'User "'.$GLOBALS['SQ_SYSTEM']->user->name.'" has forceably acquired the lock on '.$this->getTypeInfo($locked_asset->type(), 'name').' "'.$locked_asset->name.'", formally held by user "'.$user->name.'".';
					$log = $ms->newMessage(Array(), 'Asset Lock Forceably Acquired', $log_body, 'asset.locking.forced');
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

			}// end if

			// is this asset already locked
			if (!empty($lock)) {
				$user = &$this->getAsset($lock['userid']);
				trigger_error('Cannot acquire "'.$lock_type.'" lock on "'.$asset->name.'", lock already held by "'.$user->name.'"', E_USER_NOTICE);
				$success = false;
				break;
			}

			$lockid = 'asset.'.$assetid.'.'.$lock_type;
			$source_lockid = ($source_assetid) ? 'asset.'.$source_assetid.'.'.$lock_type : '';
			if (true !== ($err_msg = $GLOBALS['SQ_SYSTEM']->acquireLock($lockid, $source_lockid, $expires))) {
				trigger_error('Cannot acquire "'.$lock_type.'" lock on "'.$asset->name.'", '.$err_msg, E_USER_NOTICE);
				$success = false;
				break;
			}

		}// end foreach

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
		$current_locks = $this->getLockInfo($assetid, $lock_type, true, false);
		if (empty($current_locks)) return true;

		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) return false;

		$success = true;

		$can_force = $asset->canForceablyAcquireLock($lock_type);

		foreach($current_locks as $lock_type => $lock) {
			if (empty($lock)) continue;

			// is this asset already locked by someone else and we can't forceably acquire it, piss off
			if ((int) $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserid() && !$can_force) {
				$user = &$this->getAsset($lock['userid']);
				trigger_error('You cannot release a lock that you do not own, the "'.$lock_type.'" lock on "'.$asset->name.'" is held by "'.$user->name.'"', E_USER_WARNING);
				$success = false;
				continue;
			}

			if (true !== ($err_msg = $GLOBALS['SQ_SYSTEM']->releaseLock('asset.'.$assetid.'.'.$lock_type))) {
				trigger_error('Cannot release "'.$lock_type.'" lock on "'.$asset->name.'", '.$err_msg, E_USER_NOTICE);
				$success = false;
				continue;
			}

		}// end foreach

		return $success;

	}//end releaseLock()


	/**
	* Updates the expiry date of the lock on this asset
	*
	* If the current lock is being held indefinitely (ie. expires = NULL), the lock
	* expiry time will not be updated but the function will return true. You must release
	* an indefinite lock before updating its expiry date.
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

		foreach($current_locks as $lock_type => $lock) {

			if (true !== ($err_msg = $GLOBALS['SQ_SYSTEM']->updateLock('asset.'.$assetid.'.'.$lock_type, $expires))) {
				trigger_error('Cannot update lock on "'.$asset->name.'", '.$err_msg, E_USER_NOTICE);
				$success = false;
				break;
			}

		}// end foreach

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
		foreach($lock_types as $lock_type) {
			$lock = $GLOBALS['SQ_SYSTEM']->getLockInfo('asset.'.$assetid.'.'.$lock_type, $full_chain, $check_expires);
			if (!empty($lock)) {
				$lock['lock_type']      = $lock_type;
				$lock['source_assetid'] = (int) preg_replace('/^asset\.([0-9]+)\..*$/',      '\1', $lock['source_lockid']);
				if ($full_chain) {
					for(reset($lock['chained_assets']); NULL !== ($k = key($lock['chained_assets'])); next($lock['chained_assets'])) {
						$lock['chained_assets'][$k]['assetid'] = (int) preg_replace('/^asset\.([0-9]+)\..*$/', '\1', $lock['chained_assets'][$k]['lockid']);
						$lock['chained_assets'][$k]['source_assetid'] = (int) preg_replace('/^asset\.([0-9]+)\..*$/', '\1', $lock['chained_assets'][$k]['source_lockid']);
					}
				}// endif
			}// end if
			$locks[$lock_type] = $lock;
			unset($lock);
		}// end foreach


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
			if (is_null($asset)) return Array();
			$this->_tmp['lock_types'][$assetid] = $asset->lockTypes();
		}// end if

		if ($lock_type != 'all') {
			if (!isset($this->_tmp['lock_types'][$assetid][$lock_type])) return Array();
			$bits = bit_elements($this->_tmp['lock_types'][$assetid][$lock_type]);
			$lock_types = Array();
			foreach($bits as $bit) {
				if (false !== ($k = array_search($bit, $this->_tmp['lock_types'][$assetid]))) {
					$lock_types[] = $k;
				}
			}
			return $lock_types;
		} else {
			$lock_types = Array();
			foreach($this->_tmp['lock_types'][$assetid] as $lock_type => $bit) {
				if (preg_match('/^0*10*$/', decbin($bit))) {
					$lock_types[] = $lock_type;
				}
			}
			return $lock_types;
		}// end if

	}//end getLockTypes()


//--        LINKING        --//


	/**
	* Returns information for a link that this asset is the major asset in
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
	* @return Array()
	* @access public
	*/
	function getLink($assetid, $link_type, $type_code='', $strict_type_code=true, $value=null, $side_of_link='major', $exclusive=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code as '.$other_side.'_type_code, l.sort_order, l.dependant, l.exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.'.$other_side.'id  = a.assetid
				';
		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid).'
				  AND l.link_type = '.$db->quote($link_type);
		if (!is_null($exclusive)) $where .= ' AND l.exclusive = '.$db->quote(($exclusive) ? '1' : '0');

		if ($type_code) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for(reset($type_code); null !== ($i = key($type_code)); next($type_code)) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where))
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			}
		}

		if (!is_null($value)) $where .= '  AND l.value = '.$db->quote($value);

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.' ORDER BY l.sort_order';

		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		return $result;

	}//end getLink()


	/**
	* Returns the link information for a specific link that this asset is the major asset in
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
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT  l.linkid, l.value, l.link_type, l.sort_order, l.dependant, l.exclusive, ';
		$where = 'l.linkid = '.$db->quote((int)$linkid);
		if ($assetid > 0) {
			$sql .= 'l.'.$other_side.'id, a.type_code as '.$other_side.'_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON  l.'.$other_side.'id = a.assetid';
			$where .= ' AND l.'.$side_of_link.'id = '.$db->quote($assetid);

		} else {
			$sql .= 'l.majorid, a.type_code as major_type_code, l.minorid, a2.type_code as minor_type_code
					 FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON  l.majorid = a.assetid
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a2 ON  l.minorid = a2.assetid';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a2');

		}// end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$result = $db->getRow($sql.$where);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinkById()


	/**
	* Returns information for a link or links that is/are between this asset and the passed
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$other_assetid		the asset of the other $side_of_link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$force_array		force the results to return and array of links
	*										even if there is only one link
	*
	* @return Array()	array of link info if only one link and force_array=false,
	*                   array of arrays of link info otherwise
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=null, $value=null, $side_of_link='major', $force_array=false)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$assetid) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type,
				       a.type_code as '.$other_side.'_type_code, l.sort_order, l.dependant, l.exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.'.$other_side.'id  = a.assetid
				';

		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid).'
				  AND l.'.$other_side.'id  = '.$db->quote($other_assetid);
		if (!is_null($link_types)) $where .= '  AND (l.link_type & '.$db->quote($link_types).') > 0 ';
		if (!is_null($value))     $where .= '  AND l.value = '.$db->quote($value);

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.'
				ORDER BY l.sort_order';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		if (!$force_array && count($result) == 1) return $result[0];
		else return $result;

	}//end getLinkByAsset()


	/**
	* Return all links that this asset has to other assets
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
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null, $dependant=null, $exclusive=null, $sort_by=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);
		}

		if (!$assetid) return Array();


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
			if (is_a($asset, 'bridge')) {
				$links = $asset->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by);
				$this->forgetAsset($asset);
				return $links;
			}
			$this->forgetAsset($asset);
			unset($asset);
		}


		// we couldnt palm the request off, so we better do it ourselves
		$query = $this->generateGetLinksQuery($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $value, $dependant, $exclusive, $sort_by);
		if (empty($query)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$result = $db->getAll(implode(' ', $query));
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinks()


	/**
	* For a given asset, get the lineages from the Root folder down to it
	*
	* Also returns basic information about the last link (linkid and link type)
	*
	* @param object	Asset	&$asset		the asset whose links lineages we are getting
	*
	* @return boolean
	* @access public
	*/
	function getLinkLineages(&$asset)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = Array();
		$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, '', true, 'minor');

		foreach ($asset_links as $link) $assetids[$link['majorid']] = Array();

		if (empty($assetids)) {
			$result = Array();
		} else {

			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
			$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.name, a.assetid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ON cl.linkid = ct.linkid,
						 '.SQ_TABLE_RUNNING_PREFIX.'asset_link pl
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt ON pl.linkid = pt.linkid
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON a.assetid = pl.minorid
					';
			$where = 'cl.minorid IN ('.implode(', ', array_keys($assetids)).')
					  AND ct.treeid LIKE '.$concat.'
					  AND pt.treeid <= ct.treeid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$sql .= $where.'
					ORDER BY cl.linkid, ct.treeid, pt.treeid';


			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}
		}

		foreach ($asset_links as $link_data) $link_info[$link_data['majorid']][] = $link_data;

		$link_lineages = Array();
		foreach ($result as $treeid => $link_tree) {

			// work out some values we are going to need
			$parent_data = $link_tree[count($link_tree) -1];
			$majorid = $parent_data['assetid'];

			foreach ($link_info[$majorid] as $link_data) {

				$link_type = $link_data['link_type'];
				$linkid = $link_data['linkid'];

				$asset_lineage = Array();
				foreach ($link_tree as $tree_data) {
					if ($tree_data['assetid'] == $majorid) continue;
					$asset_lineage[] = $tree_data['name'];
				}
				$asset_lineage[] = $parent_data['name'];
				if (isset($output_lineages[$asset_lineage]) && !$can_edit) continue;

				$link_lineages[$linkid] = Array('linkid'    => $linkid,
												'link_type' => $link_type,
												'lineage'   => $asset_lineage,
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
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true)
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


		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT DISTINCT pl.minorid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ON cl.linkid = ct.linkid,
					 '.SQ_TABLE_RUNNING_PREFIX.'asset_link pl INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt ON pl.linkid = pt.linkid ';

		$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');

		$where = 'cl.minorid = '.$db->quote($assetid).'
				  AND ct.treeid LIKE '.$concat.'
				  AND pt.treeid < ct.treeid';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for(reset($type_code); null !== ($i = key($type_code)); next($type_code)) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND pa.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND pa.type_code IN (~SQ0~)';
				$sub = 'SELECT type_code
						FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
						WHERE inherited_type_code '.$type_code_cond;
				$where = db_extras_subquery($db, $where, Array($sub));
			}
		}// end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		if (!empty($type_code)) {
			$sql .= 'INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset pa ON pl.minorid = pa.assetid ';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pa');
		}

		$parents = $db->getCol($sql.$where);
		if (DB::isError($parents)) {
			trigger_error($parents->getMessage().'<br/>'.$parents->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* The query generated to get the children is actually contructed in the Asset Manager's
	* generateGetChildrenQuery function.
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see generateGetChildrenQuery()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
	{
		// check if we are getting children for a shadow asset, and palm the request off to the
		// handler of the shadow asset if we are
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$real_assetid = $id_parts[0];
			$asset = &$this->getAsset($real_assetid);
			$children = $asset->getChildren($id_parts[1], $type_code, $strict_type_code, $sort_by);
			$this->forgetAsset($asset);
			return $children;
		}


		// we are not getting children for a shadow asset, so if we are getting children of
		// an asset that handles shadow assets, palm the request off to it
		$asset = &$this->getAsset($assetid);
		if (is_a($asset, 'bridge')) {
			$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $sort_by);
			$this->forgetAsset($asset);
			return $children;
		}
		$this->forgetAsset($asset);
		unset($asset);


		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$query = $this->generateGetChildrenQuery($assetid, $type_code, $strict_type_code, $sort_by);
		if (empty($query)) return Array();

		$result = $db->getAssoc(implode(' ', $query));
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		if (!is_null($sort_by)) {
			$children = Array();
			foreach ($result as $assetid => $asset_info) $children[$assetid] = $asset_info['type_code'];
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
	* @return Array(int => string)	an array of assetids and their type code
	* @access public
	*/
	function getDependantChildren($assetid, $type_code='', $strict_type_code=true)
	{
		$children = Array();

		$asset = &$this->getAsset($assetid);
		if (!is_null($asset)) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'major', null, 1);
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				if (!$dependant_links[$k]['dependant']) continue;
				$children[$dependant_links[$k]['minorid']] = $dependant_links[$k]['minor_type_code'];
				$children = $children + $this->getDependantChildren($dependant_links[$k]['minorid'],$type_code, $strict_type_code);
			}

			$this->forgetAsset($assetid);
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
	* @return Array	returns an array of assetids
	* @access public
	*/
	function getDependantParents($assetid, $type_code='', $strict_type_code=true)
	{
		$parents = Array();

		$asset = &$this->getAsset($assetid);
		if (!is_null($asset)) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$dependant_links = $this->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, $type_code, $strict_type_code, 'minor', null, 1);
			for (reset($dependant_links); NULL !== ($k = key($dependant_links)); next($dependant_links)) {
				$parentid = $dependant_links[$k]['majorid'];
				$parents[] = $parentid;
				$parents = array_merge($parents, $this->getDependantParents($parentid, $type_code, $strict_type_code));
			}

			$this->forgetAsset($assetid);
		}

		unset($asset);
		return $parents;

	}//end getDependantParents()


	/**
	* Return an array of asset ids in the order that they propogate out from the passed in assetIDs
	*
	* The array returned for each asset is in the following format:
	* <PRE>
	* Array('assetid'    => '155',
	*       'majorid'    => '151',
	*       'type_code'  => 'page_standard',
	*       'status'     => '2',
	*       'name'       => '(( page2 ))',
	*       'short_name' => '(( page2 ))',
	*       'path'       => 'page2',
	*       'sort_order' => '0'
	* )
	* </PRE>
	*
	* @param int|Array(int)	$majorids		the majorids to traverse the tree from
	* @param integer		$levels			the number of levels to traverse (if null, traverse all levels)
	* @param Array			$exclude_list	an array of assetid to exclude from the tree
	* @param integer		$link_type		the type of wanted links
	*
	* @access public
	* @return Array()
	*/
	function  getAssetTree($majorids, $levels=null, $exclude_list=Array(), $link_type=SQ_SC_LINK_FRONTEND_NAV)
	{
		$tree_data = Array();
		if (!is_array($majorids)) $majorids = Array($majorids);
		$this->_getAssetTree($majorids, $tree_data, $levels, $exclude_list, $link_type);
		return $tree_data;

	}//end getAssetTree()


	/**
	* does the work for getAssetTree
	*
	* @param int|Array(int)	$majorids		the majorids to traverse the tree from
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
		// Stuff that is only needed once per design file
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$assetids  = Array();
		$majorids_str = '';
		$exclude_str = implode(',', $exclude_list);
		foreach($majorids as $id) {
			if (empty($tree_data[$id])) {
				$majorids_str .= $id.',';
			} else {
				foreach($tree_data[$id] as $row) {
					$assetids[] = (int) $row['assetid'];
				}
			}
		}

		if (empty($majorids_str)) return $assetids;
		static $USERIDS_COND = null;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if (is_null($USERIDS_COND)) {
			// if they are logged in add their parents to the list
			if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
				if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					$USERIDS_COND = '';
				} else {
					$userids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
					array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
					for(reset($userids); null !== ($i = key($userids)); next($userids)) $userids[$i] = $db->quote($userids[$i]);
					$USERIDS_COND = 'AND p.userid  IN ('.implode(',', $userids).')';
				}
			} else {
				// public users
				$USERIDS_COND = 'AND p.userid = 0';
			}

			if (!empty($USERIDS_COND)) {
				$USERIDS_COND .= '
					GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order
					HAVING MIN(p.access) = 1';
			}
		}

		$sql    = 'SELECT DISTINCT a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order
				   FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON a.assetid = l.minorid
					 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_path pt ON a.assetid = pt.assetid ';
		if (!empty($USERIDS_COND)) {
		$sql   .= '
				INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_permission p ON a.assetid = p.assetid ';
		}
		$where  = 'l.majorid IN ('.substr($majorids_str, 0, -1).')
					 AND (l.link_type & '.$db->quote($link_type).') > 0 ';
		if (!empty($exclude_list)) {
			$where .= 'AND a.assetid NOT IN ('.$exclude_str.')';
		}
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		if (!empty($USERIDS_COND)) {
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'p');
		}
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where .= ' '.$USERIDS_COND .'
				   ORDER BY l.majorid, l.sort_order';

		$result = $db->query($sql.$where);

		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		$assetids = Array();
		while (DB_OK === $result->fetchInto($row)) {
			if (!($row['status'] & (SQ_STATUS_LIVE | SQ_STATUS_LIVE_APPROVAL))) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($row['assetid']);
				$read_access = $asset->readAccess();

				$row['name'] = $asset->name;
				$row['short_name'] = $asset->short_name;

				if ($row['status'] & SQ_SC_STATUS_NOT_LIVE) {
					// somewhere between under construction and live so we show this by altering the name
					$row['name']       = '(( '.$row['name'].' ))';
					$row['short_name'] = '(( '.$row['short_name'].' ))';
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
				if (!$read_access) continue;
			}
			$assetids[] = (int) $row['assetid'];
			if (!isset($tree_data[$row['majorid']])) $tree_data[$row['majorid']] = Array();
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

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'l.minorid = '.$db->quote($assetid).'
				  AND t.treeid '.(($exclusively) ? 'NOT ' : '').'LIKE (~SQ0~)';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid=l.linkid
				'.$where;

		// we need to work out the treeid of the trash
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(t.treeid, '.$db->quote('%').')' : '(t.treeid || '.$db->quote('%').')';
		$sub_where = 'l.minorid = '.$db->quote($trash->id);
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
		$subs = Array( 'SELECT '.$concat.'
						FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
						'.$sub_where.' LIMIT 1');

		$sql = db_extras_subquery($db, $sql, $subs);
		if (DB::isError($sql)) {
			trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			return false;
		}

		$result = $db->getOne($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if ($exclusively) return ($result == 0);
		else return ($result > 0);

	}//end assetInTrash()


	/**
	* Return the number of links that this asset has as either a major or minor party
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
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$assetid) return 0;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$extra_table = '';
		$where = 'l.'.$side_of_link.'id = '.$db->quote($assetid);
		if ($link_types) {
			$where .= ' AND (link_type & '.$db->quote($link_types).') > 0';
		}
		if ($type_code) {
			$extra_table .= ', '.SQ_TABLE_RUNNING_PREFIX.'asset a';
			$where .= ' AND l.minorid = a.assetid ';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where)) {
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
				}
			}// end if
		}// end if
		if ($ignore_linkid) {
			$where .= ' AND l.linkid <> '.$db->quote($ignore_linkid);
		}


		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l '.$extra_table.'
				'.$where;
		$result = $db->getOne($sql);

		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end countLinks()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	*
	* @param object Asset	&$major		the major asset that we are linking from
	* @param object Asset	&$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param string			$value		the value that is to be associated with this link
	* @param string			$sort_order	the position in the links list that this link should take,
	*									if null or less than zero places at end of list
	* @param string			$dependant	'0' / '1' on whether the this asset is dependant on the
	*									asset that will be linked by this new link
	* @param string			$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*									to the asset that will be linked by this new link
	*
	* @return int the linkid of the created link
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0')
	{
		if (!$major->id) return 0;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
			return 0;
		}

		// prepare the link - do the minor first because the major gets the final say in linking
		$minor->prepareLink($major, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
		$major->prepareLink($minor, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);

		$link_type = (int) $link_type;

		if (!($link_type & SQ_SC_LINK_SIGNIFICANT) && $dependant) {
			trigger_error('In order for a link to be dependant it must also be a significant link', E_USER_WARNING);
			return 0;
		}

		// First, we should check that we don't already have a link of this type
		$current_links = $this->getLinks($major->id, $link_type, $minor->type());
		$num_curr_links = count($current_links);
		for($i = 0; $i < $num_curr_links; $i++) {
			// it already exists, so we don't need to do anything
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				// if we are trashing the asset and it is already in the trash, just
				// pretend that we created the link
				$trash = &$this->getSystemAsset('trash_folder');
				if ($trash->id == $major->id) return $current_links[$i]['linkid'];

				trigger_error('Link Already Exists', E_USER_WARNING);
				return $current_links[$i]['linkid'];
			}
		}//end for

		// check if we are allowed to link to these type of assets
		if (($err_msg = $major->canCreateLink($minor, $link_type, $exclusive)) !== true) {
			trigger_error($err_msg, E_USER_WARNING);
			return 0;
		}

		// to create a link we need to lock the new parent, but we need to make sure
		// we dont unlock it if it was already locked
		$lock_info = @$this->getLockInfo($major->id, 'links');
		$parent_was_locked = !empty($lock_info);
		if (!$this->acquireLock($major->id, 'links')) {
			trigger_error('Could not create new asset link - "links" lock not acquired on major asset "'.$major->name.'"', E_USER_WARNING);
			return 0;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if this link is a significant link,
		// then that we aren't moving under ourselves
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {

			//// Check we aren't moving the minor asset under itself ////
			// get all the links where the minor asset is a parent
			// and check to see if our id is under it

			// so much for SQL standards...
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : '(pt.treeid || '.$db->quote('%').')';
			$where = 'ct.treeid LIKE '.$concat.'
					  AND ct.treeid >= pt.treeid
					  AND pt.linkid IN (~SQ0~)
					  AND cl.minorid = '.$db->quote($major->id);

			$sub_where = 'minorid = '.$db->quote($minor->id);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where);
			$subs = Array('SELECT linkid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link '.$sub_where);

			$where = db_extras_subquery($db, $where, $subs);
			if (DB::isError($where)) {
				trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_WARNING);
				return 0;
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');

			$sql = 'SELECT COUNT(DISTINCT ct.linkid)
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt,
						'.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl ON ct.linkid = cl.linkid '.$where;
			$moving_under = $db->getOne($sql);
			if (DB::isError($moving_under)) {
				trigger_error($moving_under->getMessage().'<br/>'.$moving_under->getUserInfo(), E_USER_WARNING);
				return 0;
			} else if ($moving_under) {
				trigger_error('You are not allowed to move asset "'.$minor->name.'" (ID : #'.$minor->id.') under  "'.$major->name.'" (ID : #'.$major->id.') because this will cause the asset to be nested under itself', E_USER_WARNING);
				return 0;
			}// end if

		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// make sure the sort order is in a valid range
		$sql   = 'SELECT COUNT(*) as count, MAX(sort_order) as max
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link ';
		$where = 'majorid = '.$db->quote($major->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$row = $db->getRow($sql.$where);
		if (DB::isError($row)) {
			trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		$max = ($row['count'] > 0) ? (int) $row['max'] + 1 : 0;
		if (is_null($sort_order) || (int) $sort_order > (int) $max || (int) $sort_order < 0) $sort_order = (int) $max;

		// sequences are cool, see the PEAR::DB docs for more info
		$linkid = $db->nextId('sq_sequence_asset_link');
		if (DB::isError($linkid)) {
			trigger_error($linkid->getMessage().'<br/>'.$linkid->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// if this is a significant link, then do stuff with the tree
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			//// UPDATE THE TREE TABLE ////

			// Get one of the treeids that we have
			$sql   = 'SELECT t.treeid
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid ';
			$where = 'l.minorid = '.$db->quote($major->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql .= $where.' LIMIT 1';

			$existing_treeid = $db->getOne($sql);
			if (DB::isError($existing_treeid)) {
				trigger_error($existing_treeid->getMessage().'<br/>'.$existing_treeid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// if we aren't linked anywhere we can't be linked to, sorry
			// NOTE: exception to rule is root folder
			if ((string) $existing_treeid == '' && $major->id != 1) {
				trigger_error('Unable to create link, Asset "'.$major->name.'" (Asset #'.$major->id.') is not linked to anything itself', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// OK, what we are going to get a treeid and then do a "INSERT INTO ... SELECT FROM"
			// into the tree table of all the links that are under the minor asset
			$sql   = 'SELECT t.treeid, t.num_immediate_kids
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid ';
			$where = 'l.minorid = '.$db->quote($minor->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql .= $where.' LIMIT 1';

			$minor_tree = $db->getRow($sql);
			if (DB::isError($minor_tree)) {
				trigger_error($minor_tree->getMessage().'<br/>'.$minor_tree->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			$existing_treeid = (string) $existing_treeid;
			if (empty($minor_tree)) {
				$minor_tree = Array('treeid' => '', 'num_immediate_kids' => 0);
			} else {
				$minor_tree['treeid'] = (string) $minor_tree['treeid'];
				$minor_tree['num_immediate_kids'] = (int) $minor_tree['num_immediate_kids'];
			}

			// when links are deleted their entries in the tree table are set to have a linkid of zero
			// (see deleteLink()) so we can attempt to find unused treeid's that are forming gaps
			$sql   = 'SELECT ct.treeid
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ';
			$where = 'ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
					  AND ct.treeid > '.$db->quote($existing_treeid).'
					  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE).'
					  AND ct.linkid = 0';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$sql .= $where.' LIMIT 1';

			$free_childid = $db->getOne($sql);
			if (DB::isError($free_childid)) {
				trigger_error($free_childid->getMessage().'<br/>'.$free_childid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// No free children ? generate one from the max value
			if ((string) $free_childid == '') {
				$sql   = 'SELECT MAX(ct.treeid) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ';
				$where = 'ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
						  AND ct.treeid > '.$db->quote($existing_treeid).'
						  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE);
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');

				$free_childid = $db->getOne($sql.$where);
				if (DB::isError($free_childid)) {
					trigger_error($free_childid->getMessage().'<br/>'.$free_childid->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

				// No max ? we must be the first
				if ((string) $free_childid == '') {
					if (($free_childid = asset_link_treeid_convert(0, true)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}

				// we found a max so increment it
				} else {
					// get only the child part
					$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

					if (($child_num    = asset_link_treeid_convert($free_childid, false)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
					if (($free_childid = asset_link_treeid_convert($child_num + 1, true)) === false)  {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
				}// end if

			// else there was a free entry
			} else {

				// get only the child part
				$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

				// remove the current zeroed entries in the tree table,
				// as we will be inserting over the top of them

				// because the root folder is not a minor party in any links we have
				// to something special for it
				$where_cond = '';
				if ($major->id == 1) {
					$where_cond = 'treeid = '.$db->quote($free_childid);
				} else {
					$where_cond = 'treeid IN (~SQ0~)';

					$concat = ($db->phptype == 'mysql')
								? 'CONCAT(t.treeid, '.$db->quote($free_childid).')'
								: 't.treeid || '.$db->quote($free_childid);
					$sub_where = 'l.minorid = '.$db->quote($major->id);
					$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
					$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
					$subs = Array(' SELECT '.$concat.'
									FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid '.$sub_where);
					$where_cond = db_extras_subquery($db, $where_cond, $subs);
					if (DB::isError($where_cond)) {
						trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}

				}

				if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link_tree', $where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '') {
				$values = Array('treeid'             => $free_childid,
								'linkid'             => $linkid,
								'num_immediate_kids' => $minor_tree['num_immediate_kids']
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_link_tree', $values)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			// we have existing tree entries, do a insert..select to create entries for them all
			} else {
				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(t.treeid, '.$db->quote($free_childid).')'
							: '(t.treeid || '.$db->quote($free_childid).')';

				$where = 'l.minorid = '.$db->quote($major->id);
				// note that we dont prep this query for rollback because its getting run
				// by the rollback query functions already
				$select_from = SQ_TABLE_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_PREFIX.'asset_link l ON t.linkid = l.linkid';
				$values = Array('treeid'             => $concat,
								'linkid'             => $db->quote($linkid),
								'num_immediate_kids' => $db->quote($minor_tree['num_immediate_kids'])
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsertSelect('asset_link_tree', $select_from, $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}// end if

			// Update the parents to tell them that they are have one kid more
			$where_cond = 'treeid IN (~SQ0~)';
			$sub_where = 't.linkid = '.$db->quote($linkid);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
			$subs = Array(' SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t '.$sub_where);

			$where_cond = db_extras_subquery($db, $where_cond, $subs);
			if (DB::isError($where_cond)) {
				trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			$values = Array('num_immediate_kids' => 'num_immediate_kids + 1');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// if this minor has already been linked so do a select into
			if ($minor_tree['treeid'] != '') {

				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(pt.treeid, '.$db->quote($free_childid).', SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).'))'
							: 'pt.treeid || '.$db->quote($free_childid).' || SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).')';

				$where = 'pl.minorid = '.$db->quote($major->id).'
						  AND ct.treeid LIKE '.$db->quote($minor_tree['treeid'].'%').'
						  AND ct.treeid > '.$db->quote($minor_tree['treeid']);
				// note that we dont prep this query for rollback because its getting run
				// by the rollback query functions already
				$select_from = SQ_TABLE_PREFIX.'asset_link_tree pt INNER JOIN '.SQ_TABLE_PREFIX.'asset_link pl ON pt.linkid = pl.linkid,
								'.SQ_TABLE_PREFIX.'asset_link_tree ct';
				$values = Array('treeid'             => $concat,
								'linkid'             => 'ct.linkid',
								'num_immediate_kids' => 'ct.num_immediate_kids'
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsertSelect('asset_link_tree', $select_from, $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

		}// end if significant link

		// move the sort_order for all the other assets up one
		$where_cond = ' majorid         = '.$db->quote($major->id).'
						AND sort_order >= '.$db->quote($sort_order);
		$values = Array('sort_order' => 'sort_order + 1');
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// and finally insert the link
		$values = Array('linkid'         => $linkid,
						'majorid'        => $major->id,
						'minorid'        => $minor->id,
						'link_type'      => $link_type,
						'value'          => $value,
						'sort_order'     => $sort_order,
						'dependant'      => (($dependant) ? '1' : '0'),
						'exclusive'      => (($exclusive) ? '1' : '0'),
						'updated'        => ts_iso8601(time()),
						'updated_userid' => $GLOBALS['SQ_SYSTEM']->currentUserId()
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_link', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// tell, the asset it has updated
		if (!$major->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}
		if (!$minor->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// release locks depending on if the parent was locked before the link
		if (!$parent_was_locked) {
			if (!$this->releaseLock($major->id, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}
		}

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message_body = '"'.$minor->name.'" was successfully linked to "'.$major->name.'"';
		$message = $ms->newMessage(Array(), 'Asset Link Created', $message_body, 'asset.linking');
		$message->parameters['majorid'] = $major->id;
		$message->parameters['minorid'] = $minor->id;
		$message->parameters['linkid']  = $linkid;
		$message->send();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($major, 'CreateLink', Array('linkid' => $linkid));
		return $linkid;

	}//end createAssetLink()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object Asset	$major		the major asset that we are linking to
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param int			$exclusive	the exclusive status of the link (gets passed to canLinkToType)
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
		if (empty($GLOBALS['SQ_INSTALL']) && !$major->writeAccess('')) {
			return 'Cannot create link from '.$major->name.' (# '.$major->id.') to '.$minor->name.' (# '.$minor->id.'), permission denied (no write access to "'.$major->name.'")';
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
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link ';
			$where = 'minorid       = '.$db->quote($minor->id).'
						AND (link_type & '.SQ_SC_LINK_SIGNIFICANT.') > 0';
			if (!$exclusive) $where .= ' AND exclusive = '.$db->quote('1');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where;

			$current_majorid = $db->getOne($sql);
			if (DB::isError($current_majorid)) {
				trigger_error($current_majorid->getMessage().'<br/>'.$current_majorid->getUserInfo(), E_USER_WARNING);
				return 'Database Error';
			}

			if ($current_majorid) {
				$current_major = &$this->getAsset($current_majorid);
				return 'Asset "'.$minor->name.'" (Asset #'.$minor->id.') cannot be linked to "'.$major->name.'" (Asset #'.$major->id.') it is exclusively linked to Asset #'.$current_majorid.' ("'.$current_major->name.'")';
			}

		}// end if

		return true;

	}// end canCreateLink()


	/**
	* Checks whether a link of a certain type can be made to this asset
	* Returns TRUE if link allowed and a string containing the error if it is not
	*
	* @param object Asset	$major			the major asset that we are linking to
	* @param string			$type_code		the type code that we want to check
	* @param string			$link_type		the type of link to check
	* @param int			$ignore_linkid	if it gets down to the checking of the cardinality,
	*										ignore the link represented by this link id
	* @param int			$exclusive		the exclusive status of the link
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canLinkToType(&$major, $type_code, $link_type, $ignore_linkid=0, $exclusive=0)
	{
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
		for($i = 0; $i < count($types); $i++) {
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
		if ($allowed_links[$link_type][$type]['card'] != 'M') {
			$num_curr_links = $this->countLinks($major->id, 'major', $link_type, $type, true, $ignore_linkid);
			//  and we already have our quota of links, bugger off
			if ($num_curr_links >= (int) $allowed_links[$link_type][$type]['card']) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				return '"'.$this->getTypeInfo($major->type(), 'name').'" Assets can only have '.((int) $allowed_links[$link_type][$type]['card']).' "'.link_type_name($link_type).'" link(s) to a "'.$type.'"';
			}
		}//end if

		// if we get this far all is OK
		return true;

	}// canLinkToType()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid)
	{
		// first, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		$major = &$this->getAsset($link['majorid'], $link['major_type_code']);
		$minor = &$this->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($major) || is_null($minor)) {
			return false;
		}

		// to delete a link we need to lock the old parent, but we need to make sure
		// we dont unlock it if it was already locked
		$lock_info = @$this->getLockInfo($major->id, 'links');
		$parent_was_locked = !empty($lock_info);
		if (!$this->acquireLock($major->id, 'links')) {
			trigger_error('Could not delete asset link - "links" lock not acquired on major asset "'.$major->name.'"', E_USER_WARNING);
			return 0;
		}

		// check if we are allowed to delete this link
		if (($err_msg = $major->canDeleteLink($linkid)) !== true) {
			trigger_error($err_msg, E_USER_WARNING);
			return false;
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
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
			// being deleted and if we are not purging the trash
			if (!$num_other_links && !$GLOBALS['SQ_PURGING_TRASH']) {
				// some assets may not be able to have their last significant link
				// deleted, so lets check first before going ahead
				if (!$minor->canDelete()) {
					trigger_error($minor->name.' can not have its last significant link removed' , E_USER_WARNING);
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
					foreach($all_permissions as $userid => $access) {
						// we are deliberatly not checking for the return value here
						// because if for some reason the permission can't be set that is really just
						// bad luck
						@$this->setPermission($minor->id, $userid, $perm, $access);
					}
				}

				if (!$trash_folder->createLink($minor, SQ_LINK_TYPE_2)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			//// UPDATE THE TREE ////

			// Update the parents to tell them that they are going to be one kid less
			$where_cond_string = 'treeid in (~SQ0~)';
			$sub_where = 't.linkid = '.$db->quote($linkid);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
			$subs = Array(' SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t '.$sub_where);

			$where_cond = db_extras_subquery($db, $where_cond_string, $subs);
			if (DB::isError($where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($sql->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// add a rollback entry for the tree
			$values = Array('num_immediate_kids' => 'num_immediate_kids - 1');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// We can delete all the links under these nodes because it will be a clean start
			// when we insert into the gap's we create below
			$where_cond_string = 'treeid in (~SQ0~)';
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
			$sub_where = 'pt.linkid = '.$db->quote($linkid).'
						  AND ct.treeid LIKE '.$concat.'
						  AND ct.treeid > pt.treeid';
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'pt');
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'ct');
			$subs = Array('SELECT ct.treeid
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt, '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct
							'.$sub_where);

			$where_cond = db_extras_subquery($db, $where_cond_string, $subs);
			if (DB::isError($where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
				return false;
			}

			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link_tree', $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// We are going to set the treeid nodes that this link is associated
			// with to zero so that we can find it as a gap when we createLink() later on
			$where = 'linkid = '.$db->quote($linkid);
			$values = Array('linkid' => $db->quote('0'),
							'num_immediate_kids' => $db->quote('0')
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

		}// end if significant link

		// move 'em up, higher
		$where_cond = ' majorid        = '.$db->quote($major->id).'
						AND sort_order > '.$db->quote($link['sort_order']);
		$values = Array('sort_order' => 'sort_order - 1');
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$where_cond = ' linkid  = '.$db->quote($linkid).'
						AND majorid = '.$db->quote($major->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// tell, the asset it has updated
		if (!$major->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$minor->linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// release locks depending on if the parent was locked before the link
		if (!$parent_was_locked) {
			if (!$this->releaseLock($major->id, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message_body = '"'.$minor->name.'" was successfully unlinked from "'.$major->name.'"';
		$message = $ms->newMessage(Array(), 'Asset Link Deleted', $message_body, 'asset.linking');
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
	* Determine if we can delete the passed link
	*
	* @param object Asset	$major	the major asset that we are linking to
	* @param int			$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink(&$major, $linkid)
	{
		// if you dont have write access to the parent, you can't delete any links
		if (!$major->writeAccess('links')) {
			return 'Cannot delete link, permission denied';
		}

		return true;

	}//end canDeleteLink()


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
		// first, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to move link', E_USER_WARNING);
			return 0;
		}

		$old_parent = &$this->getAsset($link['majorid'], $link['major_type_code']);
		if (is_null($old_parent)) {
			trigger_error('Unable to Move Asset, old parent (Asset #"'.$link['majorid'].'") not found', E_USER_WARNING);
			return 0;
		}

		// now check that the new parent exists
		$new_parent = &$this->getAsset($to_parentid);
		if (is_null($new_parent)) {
			trigger_error('Unable to Move Asset, new parent (Asset #"'.$to_parentid.'") not found', E_USER_WARNING);
			return 0;
		}

		// cant link to the new parent if it is in the trash
		if ($this->assetInTrash($new_parent->id, true)) {
			trigger_error('Unable to Move Asset, new parent (Asset #"'.$new_parent->id.'") is in the trash');
			return 0;
		}

		// if you dont have write access to this asset you cant delete any links
		if (!$old_parent->writeAccess('')) {
			trigger_error('Unable to Move Asset, permission denied to old parent "'.$old_parent->name.'"', E_USER_WARNING);
			return 0;
		}

		// if you dont have write access to the new parent asset you cant create any links
		if (!$new_parent->writeAccess('')) {
			trigger_error('Unable to Move Asset, permission denied to new parent "'.$new_parent->name.'"', E_USER_WARNING);
			return 0;
		}

		// this is the asset we are moving
		$minor = &$this->getAsset($link['minorid'], $link['minor_type_code']);
		if (!$minor->id) {
			trigger_error('Unable to Move Asset, minor (Asset #"'.$link['minorid'].'") not found', E_USER_WARNING);
			return 0;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$new_linkid = $new_parent->createLink($minor, $link_type, '', $to_parent_pos);
		if (!$new_linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// Now let's try and delete the old link
		$deleted = $old_parent->deleteLink($linkid);
		if (!$deleted) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $new_linkid;

	}//end moveLink()


	/**
	* Move a link from one position in the assets links order to another
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
		return $this->_updateLink($linkid, null, null, $sort_order);

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
	function _updateLink($linkid, $link_type=null, $value=null, $sort_order=null)
	{
		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to update link', E_USER_WARNING);
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
				trigger_error('Unable to update link #'.$linkid.' , you cannot change a link from being a significant link to a non-significant link or vice versa', E_USER_WARNING);
				$ms->abortLog();
				return false;
			}

			if (($err_msg = $this->canLinkToType($major, $minor->type(), $link_type)) !== true) {
				trigger_error('Unable to update link #'.$linkid.' ('.$err_msg.')', E_USER_WARNING);
				$ms->abortLog();
				return false;
			}

			// check for web path conflicts with the new parents children
			// if the old link is not a web path link but the new one is
			if (!($link['link_type'] & SQ_SC_LINK_WEB_PATHS) && ($link_type & SQ_SC_LINK_WEB_PATHS)) {
				$paths     = $minor->getWebPaths();
				$bad_paths = $this->webPathsInUse($major, $paths, $minor->id);
				if (!empty($bad_paths)) {
					trigger_error('Cannot update link #'.$linkid.', path(s) "'.implode('", "', $bad_paths).'" already in use by one of the chlidren of "'.$major->name.'"', U_USER_WARNING);
					$ms->abortLog();
					return false;
				}
			}

			$set_clauses['link_type'] = $db->quote($link_type);

			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$message_body = 'The type of link #'.$linkid.' (between "'.$major->name.'" and "'.$minor->name.'") was successfully changed from '.link_type_name($link['link_type']).' to '.link_type_name($link_type);
			$message = $ms->newMessage(Array(), 'Asset Link Type Updated', $message_body, 'asset.linking.type');
			$message->parameters['majorid']  = $major->id;
			$message->parameters['minorid']  = $minor->id;
			$message->parameters['linkid']   = $linkid;
			$ms->logMessage($message);

		}// end if link_type_changed

		if ($value_changed) {

			$set_clauses['value'] = $db->quote($value);

			$message_body = 'The value on link #'.$linkid.' (between "'.$major->name.'" and "'.$minor->name.'") was successfully changed from '.$link['value'].' to '.$value;
			$message = $ms->newMessage(Array(), 'Asset Link Value Updated', $message_body, 'asset.linking.value');
			$message->parameters['majorid']   = $major->id;
			$message->parameters['minorid']   = $minor->id;
			$message->parameters['linkid']    = $linkid;
			$ms->logMessage($message);

		}// end if value_changed

		if ($sort_order_changed) {
			$where = 'majorid = '.$db->quote($major->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT COUNT(*) AS count, MAX(sort_order) AS max
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link '.$where;
			$row = $db->getRow($sql);
			if (DB::isError($row)) {
				trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
				$ms->abortLog();
				return false;
			}
			$max = ($row['count'] > 0) ? (int) $row['max'] : 0;
			if ($sort_order > $max || $sort_order < 0) $sort_order = $max;

			$set_clauses['sort_order'] = $db->quote($sort_order);

			$message_body = 'The sort order of link #'.$linkid.' (between "'.$major->name.'" and "'.$minor->name.'") was successfully changed from '.$link['sort_order'].' to '.$sort_order;
			$message = $ms->newMessage(Array(), 'Asset Link Sort Order Updated', $message_body, 'asset.linking.order');
			$message->parameters['majorid']   = $major->id;
			$message->parameters['minorid']   = $minor->id;
			$message->parameters['linkid']    = $linkid;
			$ms->logMessage($message);
		}

		// dickhead check
		if (empty($set_clauses)) {
			$ms->abortLog();
			return true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$set_clauses['updated_userid']  = $db->quote($GLOBALS['SQ_SYSTEM']->currentUserId());
		$set_clauses['updated'] = $db->quote(ts_iso8601(time()));
		$where = '  linkid      = '.$db->quote($linkid).'
					AND majorid = '.$db->quote($major->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $set_clauses, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$ms->abortLog();
			return false;
		}

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
				$where = '  majorid = '.$db->quote($major->id).'
							AND linkid <> '.$db->quote($linkid).'
							AND sort_order >= '.$db->quote($sort_order).'
							AND sort_order <= '.$db->quote($link['sort_order']);
				$values = Array('sort_order' => 'sort_order + 1');
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$ms->abortLog();
					return false;
				}

			} else {
				$where = '  majorid = '.$db->quote($major->id).'
							AND linkid <> '.$db->quote($linkid).'
							AND sort_order >= '.$db->quote($link['sort_order']).'
							AND sort_order <= '.$db->quote($sort_order);
				$values = Array('sort_order' => 'sort_order - 1');
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$ms->abortLog();
					return false;
				}
			}

		}// end if sort order changed

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

	}//end _updateLink()


//--        PERMISSIONS        --//


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$access			Type of Access : null = all, true = granted, false = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is false)
	* @param boolean	$all_info		when false fn just returns an array of userids for
	*									those that have permission
	*									When true fn returns all info about the permission in the form of
	*									Array(userid => access)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are false)
	*
	* @return Array()
	* @access public
	*/
	function getPermission($assetid, $permission, $access=null, $and_greater=true, $expand_groups=false, $all_info=false)
	{
		$permission = (int) $permission;
		// can't be and_greater with workflow
		if ($permission == SQ_PERMISSION_WORKFLOW) $and_greater = false;
		if (!is_null($access)) $access = (bool) $access;

		if (($and_greater || $expand_groups) && $all_info) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() - You cannot have either and_greater or expand_groups arguments set to true if you want to have all_info set to true', E_USER_NOTICE);
			$all_info = false;
		}

		if (!isset($this->_tmp['permission_cache'])) $this->_tmp['permission_cache'] = Array();
		if (!isset($this->_tmp['permission_cache'][$assetid])) $this->_tmp['permission_cache'][$assetid] = Array();

		// return cached version if we can
		$storage_name =(($and_greater) ? 'effective_' : '').'permission_'.$permission;
		if (!isset($this->_tmp['permission_cache'][$assetid][$storage_name])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = '  SELECT DISTINCT assetid, userid, access
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_permission ';
			$where = 'assetid = '.$db->quote($assetid).'
					  AND permission '.(($and_greater) ? '>= ' : '= ').$db->quote($permission);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			// cache the result for next time
			$this->_tmp['permission_cache'][$assetid][$storage_name] = $result;

		}// end if

		$ret_val = Array();
		foreach($this->_tmp['permission_cache'][$assetid][$storage_name] as $data) {

			if      ($access === false && $data['access'] != '0') continue;
			else if ($access === true  && $data['access'] != '1') continue;

			// return all info
			if ($all_info) {
				$ret_val[$data['userid']] = $data['access'];

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

			}// end if all info

		}// end foreach

		if ($all_info) return $ret_val;
		else           return array_unique($ret_val);

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$access		whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $access)
	{
		$permission = (int)  $permission;
		$access     = (bool) $access;

		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) return false;
		if (!$asset->adminAccess('permissions')) {
			trigger_error('Cannot set permission for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		// check if this permission is already set
		$current = $this->getPermission($assetid, $permission, $access, false);
		if (in_array($userid, array_values($current))) return true;

		// if we dont have an userid, we are granting public access
		if ($permission != SQ_PERMISSION_WORKFLOW && !empty($userid)) {
			// check that the passed userid is a user or user_group
			$user = &$this->getAsset($userid, '', true);
			if (!$user->id) {
				trigger_error('Cannot set permission for an asset that does not exist', E_USER_WARNING);
				return false;
			} else if (!is_a($user, 'user') && !is_a($user, 'user_group')) {
				trigger_error('Cannot set permission for asset type "'.$user->type().'", not a user or user group', E_USER_WARNING);
				return false;
			}
			$user_name = $user->name;
		} else if ($permission == SQ_PERMISSION_WORKFLOW) {
			$user_name = 'Asset Workflow';
		} else {
			$user_name = 'General Public';
		}

		// check if the reverse access of this permission is set
		$current = $this->getPermission($assetid, $permission, !$access, false);
		if (in_array($userid, array_values($current))) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$perm_name = permission_type_name($permission);

			$new_access     = ($access) ? 'grant'   : 'revoke';
			$current_access = ($access) ? 'revoked' : 'granted';

			trigger_error('Could not '.$new_access.' '.$perm_name.' access for "'.$user_name.'" on "'.$asset->name.'" because this asset already has '.$current_access.' '.$perm_name.' access to "'.$user_name.'"', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the permission into the database
		$values = Array('assetid'      => $assetid,
						'userid'       => $userid,
						'permission'   => $permission,
						'access'       => (($access) ? '1' : '0')
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_permission', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$asset->permissionsUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// clear the permission cache
		if (isset($this->_tmp['permission_cache'][$assetid])) unset($this->_tmp['permission_cache'][$assetid]);

		// log the action
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message_body = permission_type_name($permission).' permission has been '.(($access) ? 'granted' : 'denied').' on asset "'.$asset->name.'" for "'.$user_name.'"';
		$message = $ms->newMessage(Array(), 'Asset Permission Set', $message_body, 'asset.permissions');
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
		$assetid    = (int) $assetid;
		$userid     = (int) $userid;
		$permission = (int) $permission;

		$asset = &$this->getAsset($assetid);
		if (is_null($asset)) return false;
		if (!$asset->adminAccess('permissions')) {
			trigger_error('Cannot delete permission for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		// if we dont have an userid, we are deleting public access
		if ($permission != SQ_PERMISSION_WORKFLOW && !empty($userid)) {
			// check that the passed userid is a user or user_group
			$user = &$this->getAsset($userid, '', true);
			if (!$user->id) {
				trigger_error('Cannot delete permission for an asset that does not exist', E_USER_WARNING);
				return false;
			} else if (!is_a($user, 'user') && !is_a($user, 'user_group')) {
				trigger_error('Cannot delete permission for asset type "'.$user->type().'", not a user or user group', E_USER_WARNING);
				return false;
			}
			$user_name = $user->name;
		} else if ($permission == SQ_PERMISSION_WORKFLOW) {
			$user_name = 'Asset Workflow';
		} else {
			$user_name = 'General Public';
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete and add rollback entries
		$where = 'userid         = '.$db->quote($userid).'
				  AND permission = '.$db->quote($permission).'
				  AND assetid    = '.$db->quote($assetid);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_permission', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$asset->permissionsUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// clear the permission cache
		if (isset($this->_tmp['permission_cache'][$assetid])) unset($this->_tmp['permission_cache'][$assetid]);

		// log the action
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message_body = permission_type_name($permission).' permission has been deleted on asset "'.$asset->name.'" for "'.$user_name.'"';
		$message = $ms->newMessage(Array(), 'Asset Permission Deleted', $message_body, 'asset.permissions');
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
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') $protocol = 'http';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$url_condition = '';
		if ($exact_match) {
			$url_condition = ' = '.$db->quote($url);

		} else {
			$sections = explode('/', $url);
			$option  = '';
			$url_condition = ' IN (';
			foreach($sections as $piece) {
				$url_condition .= (($option) ? ',' : '').$db->quote($option.$piece);
				$option .= $piece.'/';
			}
			$url_condition .= ') ';
		}// end if

		$where = 'l.url '.$url_condition;

		if (!is_null($protocol)) {
			$where .= ' AND l.'.$protocol.' = '.$db->quote(1);
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT l.url, l.assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
				'.$where.'
				ORDER BY CHARACTER_LENGTH(l.url) DESC
				LIMIT 1';

		$row = $db->getRow($sql);
		if (DB::isError($row)) trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_ERROR);

		// URL not found
		if (empty($row)) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			if (!$mute_errors) trigger_error('Unable to find an Asset for the URL "'.$fancy_url.'"', E_USER_WARNING);
			$null = null; // 'cause we are meant to return by reference
			return $null;

		} else {
			return $this->getAsset($row['assetid'], '', $mute_errors);
		}// end if

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

		if (is_null($url)) $url = current_url(false);
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// so much for SQL standards...
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(url, '.$db->quote('%').')' : '(url || '.$db->quote('%').')';
		$where = $db->quote($url).' LIKE '.$concat;
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT urlid, assetid, url, http, https
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_url
				'.$where. '
				ORDER BY url DESC
				LIMIT 1';
		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getRootURL()

	/**
	* Returns an array of assetid's + info in the order that they propogate out from the site url
	*
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string    $protocol    the protocol to match -> null means it is ignored
	* @param string    $url        the urls to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($protocol=null, $url=null)
	{
		if (is_null($url)) $url = current_url(false, true);
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') $protocol = 'http';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get the root(site) url for this url
		$root_url = $this->getRootURL($url);
		if (empty($root_url)) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			trigger_error('Unable to find an lineage for the URL "'.$fancy_url.'", no root url was found', E_USER_WARNING);
			return Array();
		}

		//// See Mantis #0000128 for a reason on why the trailing slash is added to the URL and before the '%' ////
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(l.url, '.$db->quote('/%').')' : '(l.url || '.$db->quote('/%').')';
		$where = 'l.root_urlid = '.$db->quote((int) $root_url['urlid']).'
				AND '.$db->quote($url.'/').' LIKE '.$concat;

		if (!is_null($protocol)) {
			$where .= ' AND l.'.$protocol.' = '.$db->quote('1');
			$protocol_select = $db->quote($protocol).' as protocol';
		} else {
			$curr_proto = current_protocol();
			$protocol_select = 'CASE WHEN l.'.$curr_proto.' = '.$db->quote('1').' THEN '.$db->quote($curr_proto).' ELSE '.$db->quote(($curr_proto == 'http') ? 'https' : 'http').' END as protocol';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql = 'SELECT l.url, l.assetid, a.name, a.short_name, a.type_code, '.$protocol_select.', MIN(al.link_type) AS link_type
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
				  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.assetid = a.assetid
				  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link al ON a.assetid = al.minorid
				'.$where.'
				GROUP BY l.url, l.assetid, a.name, a.short_name, a.type_code, protocol
				ORDER BY l.url ASC';

		$result = $db->getAll($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		// No urls found or we only found some of the parents, but not the passed url
		if (empty($result) || $result[count($result) - 1]['url'] != $url) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			trigger_error('Unable to find an lineage for the URL "'.$fancy_url.'"', E_USER_WARNING);
			return Array();

		} else {

			// if there is only one entry then this is the site root, so for completeness
			// we will manually add in the index page to the index
			if (count($result) == 1) {
				$site = &$this->getAsset($result[0]['assetid'], $result[0]['type_code']);
				if (is_null($site)) {
					return Array();
				}
				$index_page = $site->getSpecialPage('index');
				if (!is_null($index_page)) {
					$paths = $index_page->getWebPaths();
					$result[] = Array (
										'url'        => $result[0]['url'].((empty($paths)) ? '' : '/'.$paths[0]),
										'assetid'    => $index_page->id,
										'name'       => $index_page->name,
										'short_name' => $index_page->short_name,
										'type_code'  => $index_page->type(),
										'link_type'  => SQ_LINK_TYPE_3,
										'protocol'   => current_protocol(),
										);
				}// end if
			}// end if

			return $result;
		}// end if

	}//end getLineageFromURL()


	/**
	* Returns the URL for an asset, closest to the $base_url
	*
	* @param int | array(int)	$assetids			the id of the asset you want the url for or an array
	*												for a whole bunch of them
	* @param string				$base_url			the url that we are want to match closest to
	* @param boolean			$ignore_rollback	ignore rollback checks
	*
	* @return mixed returns string if $assetids is an int and an array(int => string) if $assetids is an array
	* @access public
	*/
	function getAssetURL($assetids, $base_url=null, $ignore_rollback=false)
	{
		if (!isset($this->_tmp['urls'])) $this->_tmp['urls'] = Array();

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

		if (is_null($base_url)) $base_url = current_url(false);

		if (!preg_match('/^(([a-z]+):\/\/)?(.*)$/', strip_url($base_url), $matches)) return ($return_array) ? Array() : '';
		$protocol = ($matches[2] == '') ? current_protocol() : (($matches[2] == 'https') ? 'https' : 'http');
		$base_url = $matches[3];

		// work out if any of these are cached
		foreach ($assetids as $assetid) {
			if (!isset($this->_tmp['urls'][$assetid])) $this->_tmp['urls'][$assetid] = Array();

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
			// so much for SQL standards...
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(l.url, '.$db->quote('/').')' : 'l.url || '.$db->quote('/');
			foreach($url_bits as $i => $bit) {
				$str .= $bit.'/';
				$sql .= ' +
				(CASE WHEN SUBSTRING('.$concat.' FROM 1 FOR '.strlen($str).') = '.$db->quote($str).' THEN 1 ELSE 0 END)';
			}

			$sql .= '
			) as weighting
			FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
			'.$where.'
			ORDER BY weighting DESC, l.url';

			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return '';
			}
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
	* @param int | array(int)	$assetids			the id of the asset you want the href for or an array
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
			if (!isset($this->_tmp['hrefs'][$assetid])) $this->_tmp['hrefs'][$assetid] = Array();

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

		$return_hrefs = Array();
		foreach ($asset_info as $assetid => $screen) {
			$return_hrefs[$assetid] = sq_web_path('admin').'/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.$assetid.'&asset_ei_screen='.$screen;
		}
		return $return_hrefs;

	}// end getAssetBackendHref()


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
	* @return Array(string)
	* @access protected
	*/
	function webPathsInUse(&$major, $paths, $ignore_assetid=0, $return_alternatives=false)
	{
		if (empty($paths)) return Array();
		$return_paths = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all our nav children
		$childids = Array();
		$my_kids = $this->getLinks($major->id, SQ_SC_LINK_WEB_PATHS, '', null, 'major');
		for($j = 0; $j < count($my_kids); $j++) {
			if ($my_kids[$j]['minorid'] == $ignore_assetid) continue;
			$childids[] = $db->quote($my_kids[$j]['minorid']);
		}

		$child_paths = Array();

		if (count($childids)) {
			$sql = 'SELECT path
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_path';
			$where = 'assetid IN ('.implode(',', $childids).')';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$child_paths = $db->getCol($sql.$where);
			if (DB::isError($child_paths)) {
				trigger_error($child_paths->getMessage().'<br/>'.$child_paths->getUserInfo(), E_USER_ERROR);
				return false;
			}
		}

		foreach ($paths as $path) {
			if (in_array($path, $child_paths)) {
				if ($return_alternatives) {
					// find a good path to use
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$new_path = $path;
					while (in_array($new_path, $child_paths)) {
						// increment the path using numerials
						$new_path = increment_filename($new_path);
					}
					$path = $new_path;
				}
				$return_paths[] = $path;
			} else if ($return_alternatives) {
				$return_paths[] = $path;
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



	function paintLimbo(&$backend)
	{
		require_once SQ_INCLUDE_PATH.'/asset_manager_edit_fns.inc';
		$edit_fns =& new Asset_Manager_Edit_Fns($this, $backend);
		$edit_fns->paintLimbo();

	}//end paintLimbo()


//--        QUERY GENERATORS        --//


	/**
	* Return all links that this asset has to other assets
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
	*
	* @return Array()
	* @access public
	*/
	function generateGetLinksQuery($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null, $dependant=null, $exclusive=null, $sort_by=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$assetid) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$select = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code as '.$other_side.'_type_code, l.sort_order, l.dependant, l.exclusive';
		$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l, '.SQ_TABLE_RUNNING_PREFIX.'asset a';
		$where = 'l.'.$other_side.'id = a.assetid
				  AND l.'.$side_of_link.'id = '.$db->quote($assetid).'
				  AND (l.link_type & '.$db->quote($link_types).') > 0 ';
		if (!is_null($value))		$where .= ' AND l.value     = '.$db->quote($value);
		if (!is_null($dependant))	$where .= ' AND l.dependant = '.$db->quote(($dependant) ? '1' : '0');
		if (!is_null($exclusive))	$where .= ' AND l.exclusive = '.$db->quote(($exclusive) ? '1' : '0');

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for(reset($type_code); null !== ($i = key($type_code)); next($type_code)) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where))
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		if (is_null($sort_by)) {
			// order by sort order (by default)
			$order_by = 'ORDER BY l.sort_order, a.type_code';
		} else {
			// order by a specific field in the asset table
			$order_by = 'ORDER BY a.'.$sort_by;
			if ($sort_by != 'type_code') $select .= ', a.'.$sort_by.' AS sort_value';
		}

		return Array('select'   => $select,
					 'from'     => $from,
					 'where'    => $where,
					 'order_by' => $order_by,
					);

	}//end generateGetLinksQuery()


	/**
	* Generate the parts of the query needed to get children of the passwed assetid
	*
	* The return value is array consisting of a select statement, a from statement, a where clause
	* (which may be blank is we are getting children of the root node) and an order by clause
	* (which may be blank if we are not sorting). The return array is structed so you can implode
	* it using ' ' as glue to form the full query.
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return Array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see getChildren()
	*/
	function generateGetChildrenQuery($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if we are the root folder then we just need to select from the asset table
		if ($assetid == 1) {
			$select = 'SELECT a.assetid, a.type_code';
			$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'a');
			if (!empty($where) && !empty($type_code)) $where .= ' AND ';

		// OK, normal asset, let's find a reference to them in the tree and go from there
		} else {
			$where = 'l.minorid = '.$db->quote($assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
					'.$where.' LIMIT 1';

			$treeid = $db->getOne($sql);
			if (DB::isError($treeid)) {
				trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			}

			$select = 'SELECT DISTINCT l.minorid, a.type_code';
			$from   = 'FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
					         INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
					         INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.minorid = a.assetid
					';
			$where = 't.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid);

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			if (!empty($type_code)) $where .= ' AND ';

		}// end if assetid == 1

		if (!empty($type_code)) {

			if (empty($where)) $where = ' WHERE ';

			$type_code_cond = '';
			if (is_array($type_code)) {
				for(reset($type_code); null !== ($i = key($type_code)); next($type_code)) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= 'a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= 'a.type_code IN (~SQ0~)';
				$sub = 'SELECT type_code
						FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
						WHERE inherited_type_code '.$type_code_cond;
				$where = db_extras_subquery($db, $where, Array($sub));
			}
		}// end if

		// order by a field if required
		$order_by = '';
		if (!is_null($sort_by)) {
			$order_by = 'ORDER BY a.'.$sort_by;
			$select .= ', a.'.$sort_by.' AS sort_value';
		}

		return Array('select'   => $select,
					 'from'     => $from,
					 'where'    => $where,
					 'order_by' => $order_by,
					);

	}//end generateGetChildrenQuery()


}//end class

?>
