<?php

/**
* Asset_Manager
*
* Purpose
*    Holds information on installed packages, as represented by the DB
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Manager extends Resolve_Object
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
		$this->Resolve_Object();
		$this->_loadAssetTypes();
	}


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
				FROM sq_asset_type';
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		while (null !== ($row = $result->fetchRow())) {
			$this->_asset_types[$row['type_code']] = $row;
		}//end while

		$result->free();
	}

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
				FROM sq_asset_type
				WHERE type_code = '.$db->quote($type_code);
		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (empty($result)) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return;
		} else {
			$this->_asset_types[$result['type_code']] = $result;
		}

		$parents = Array();
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
				FROM sq_asset_type_inherited
				WHERE type_code = '.$db->quote($type_code);
		$db_parents = $db->getCol($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$inserts = array_diff($parents, $db_parents);
		$deletes = array_diff($db_parents, $parents);

		foreach($inserts as $inherited_type_code) {
			$sql = 'INSERT INTO sq_asset_type_inherited (inherited_type_code, type_code)
					VALUES ('.$db->quote($inherited_type_code).', '.$db->quote($type_code).')';
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		foreach($deletes as $inherited_type_code) {
			$sql = 'DELETE FROM sq_asset_type_inherited
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
	function installed($type_code) {
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
	* @param string	$type_code	the code name for the asset that you want to check
	* @param string	$field		if exists, returns this information from the array for this specific field
	*
	* @return mixed
	* @access public
	*/
	function getTypeInfo($type_code, $field='')
	{

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return ($field) ? null : Array();
		}

		// because we don't get the load all the information about a type, if we don't know about a field
		// get it from the DB
		if ($field && !isset($this->_asset_types[$type_code][$field])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT '.$field.'
					FROM sq_asset_type
					WHERE type_code = '.$db->quote($type_code);
			$result = $db->getOne($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

			$this->_asset_types[$type_code][$field] = $result;

		}// end if

		return ($field) ? $this->_asset_types[$type_code][$field] : $this->_asset_types[$type_code];

	}//end getTypeInfo()


	/**
	* Returns an array representing the class hierarchy for the asset types, either for whole system
	* or for under the the passed asset type
	*
	* @param string	$base_type_code		the asset type's hierarchy to return
	* @param bool	$ignore_allowed_access
	*
	* @return array
	* @access public
	*/
	function getAssetTypeHierarchy($base_type_code='asset', $allowed_access='')
	{

		if (empty($base_type_code)) $base_type_code = 'asset';

		$type_codes = Array();
		$allowed_access = trim($allowed_access);
		if (!empty($ignore_allowed_access)) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT type_code
					FROM sq_asset_type
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

	}// end getAssetTypeHierarchy()

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
	}


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
		$sql = 'SELECT a.assetid, at.inherited_type_code FROM sq_asset a INNER JOIN sq_asset_type_inherited at ON a.type_code = at.type_code WHERE ';

		for($i = 0; $i < count($assetids); $i++) $assetids[$i] = $db->quote($assetids[$i]);
		$sql .= 'a.assetid IN ('.implode(', ', $assetids).')';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$sql .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND (a.type_code '.$type_code_cond.' OR a.type_code IN (~SQ0~))';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$sql = db_extras_subquery($db, $sql, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$sql .= ' ORDER BY a.assetid';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		return $result;
	}//end getAssetTypeInfo()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* @param array(int)		$assetids			an array of assets to limit
	* @param array(string)	$type_code			an array of types codes to limit the results to
	* @param boolean		$strict_type_code	ensure returned assets are of the passed type codes
	*
	* @return mixed
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=true)
	{
		if (empty($assetids)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT * FROM sq_asset a WHERE ';

		for($i = 0; $i < count($assetids); $i++) $assetids[$i] = $db->quote($assetids[$i]);
		$sql .= 'a.assetid IN ('.implode(', ', $assetids).')';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$sql .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND (a.type_code '.$type_code_cond.' OR a.type_code IN (~SQ0~))';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$sql = db_extras_subquery($db, $sql, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$sql .= ' ORDER BY a.assetid';

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
	}// end _validateAttributeType()


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
								FROM sq_asset_type_inherited
								WHERE type_code = '.$db->quote($type_code)
								.(($include_asset) ? '' : ' AND inherited_type_code != '.$db->quote('asset'))
								);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}
		return $result;
	}//end getTypeAncestors()

	/**
	* Returns an array of all assets types that are descendants for the passed asset
	*
	* @param string		$type_code		the code name for the asset that you want to check
	* @param boolean	$include_asset	when true adds the "Asset" type to the parent list,
	*									even though it's an unistantiable object
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
								FROM sq_asset_type_inherited
								WHERE inherited_type_code = '.$db->quote($type_code));
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}
		return $result;
	}//end getTypeDescendants()

	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc) if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code or $type_code and any of it's sub-classes
	*
	* @access public
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
		$sql = 'SELECT DISTINCT pl.minorid
				FROM sq_asset_link cl INNER JOIN sq_asset_link_tree ct ON cl.linkid = ct.linkid,
					 sq_asset_link pl INNER JOIN sq_asset_link_tree pt ON pl.linkid = pt.linkid ';
		if (!empty($type_code)) $sql .= 'INNER JOIN sq_asset pa ON pl.minorid = pa.assetid ';
		$sql .= 'WHERE cl.minorid = '.$db->quote($assetid).'
				  AND ct.treeid LIKE '.$concat.'
				  AND pt.treeid < ct.treeid';

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$sql .= ' AND pa.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND (pa.type_code '.$type_code_cond.' OR pa.type_code IN (~SQ0~))';
				$sub = 'SELECT type_code
						FROM sq_asset_type_inherited
						WHERE inherited_type_code '.$type_code_cond;
				$sql = db_extras_subquery($db, $sql, Array($sub));
			}
		}// end if

#pre_echo($sql);

		$parents = $db->getCol($sql);
		if (DB::isError($parents)) {
			trigger_error($parents->getMessage().'<br/>'.$parents->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		return $parents;

	}// end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc) if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code or $type_code and any of it's sub-classes
	*
	* @access public
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if we are the root folder then we just need to select from the asset table
		if ($assetid == 1) {
			$sql = 'SELECT a.assetid
					FROM sq_asset a
					'.((!empty($type_code)) ? 'WHERE ' : '');

		// OK, normal asset, let's find a reference to them in the tree and go from there
		} else {
			$sql = 'SELECT t.treeid
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($assetid).'
					LIMIT 1';

			$treeid = $db->getOne($sql);
			if (DB::isError($treeid)) {
				trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			}

			$sql = 'SELECT DISTINCT l.minorid
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					';
			if (!empty($type_code)) $sql .= ' INNER JOIN sq_asset a ON l.minorid = a.assetid';
			$sql .= '
					WHERE t.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid).'
					'.((!empty($type_code)) ? '  AND ' : '');

		}// end if assetid == 1

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$sql .= 'a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= '(a.type_code '.$type_code_cond.' OR a.type_code IN (~SQ0~))';
				$sub = 'SELECT type_code
						FROM sq_asset_type_inherited
						WHERE inherited_type_code '.$type_code_cond;
				$sql = db_extras_subquery($db, $sql, Array($sub));
			}
		}// end if

		$children = $db->getCol($sql);
		if (DB::isError($children)) {
			trigger_error($children->getMessage().'<br/>'.$children->getUserInfo(), E_USER_ERROR);
		}

		return $children;

	}// end getChildren()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the asset id to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @access public
	*/
	function &getAsset($assetid, $type_code='', $mute_errors=false)
	{

		if (!isset($this->_assets[$assetid]) || !is_object($this->_assets[$assetid])) {

			if (empty($type_code)) {
				$type_code = $GLOBALS['SQ_SYSTEM']->db->getOne('SELECT type_code FROM sq_asset WHERE assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($assetid));
				if (empty($type_code)) {
					if (!$mute_errors) trigger_error('Asset #'.$assetid.' does not exist', E_USER_WARNING);
					$this->_assets[$assetid] = null; // do this because of return by reference
					return $this->_assets[$assetid];
				}
			}//end if

			if (!isset($this->_asset_types[$type_code])) {
				if (!$mute_errors) trigger_error('Asset Type "'.$type_code.'" is not installed on this system', E_USER_WARNING);
				$this->_assets[$assetid] = null;
			} else {
				$this->includeAsset($type_code);
				$this->_assets[$assetid] = new $type_code($assetid);
				// make sure of the asset is correct
				if (empty($this->_assets[$assetid]->id)) $this->_assets[$assetid] = null;

			}//end if

		}//end if

		return $this->_assets[$assetid];

	}//end getAsset()


	/**
	* Registers the passed object in the store, for others to get
	* Used when a new record has just been "create()d"
	*
	* @param object Asset	&$obj	the asset to be registered
	*
	* @access public
	*/
	function registerAsset(&$obj)
	{
		// wrong class or no id, fuck off
		if (!is_object($obj) || is_a($obj, 'Asset') || !$obj->id) return;

		if (isset($this->_assets[$obj->id])) unset($this->_assets[$obj->id]);
		$this->_assets[$obj->id] = &$obj;

	}//end registerAsset();


	/**
	* Unregisters the passed object from the store
	*
	* @param object Asset	&$obj	the asset to be unregistered
	*
	* @access public
	*/
	function unRegisterAsset(&$obj)
	{
		// wrong class or no id, fuck off
		if (!is_object($obj) || is_a($obj, 'Asset') || !$obj->id) return;
		if (isset($this->_assets[$obj->id])) unset($this->_assets[$obj->id]);

	}//end unRegisterAsset();


	/**
	* Returns a reference to a system asset (ie Root Folder, Root User or Trash Folder)
	*
	* @param string	$name	the name of the system asset ('root_folder', 'root_user', 'trash_folder')
	*
	* @access public
	*/
	function &getSystemAsset($name)
	{
		switch(strtolower($name)) {
			case 'root_folder' :
				return $this->getAsset(1, 'root_folder');
			case 'trash_folder' :
				return $this->getAsset(2, 'trash_folder');
			case 'system_user_group' :
				return $this->getAsset(3, 'system_user_group');
			case 'root_user' :
				return $this->getAsset(4, 'root_user');
			case 'designs_folder' :
				return $this->getAsset(7, 'designs_folder');
			case 'login_design' :
				return $this->getAsset(8, 'login_design');
			default :
				trigger_error('System Asset Name "'.$name.'" not known', E_USER_ERROR);
		}

	}//end getSystemAsset()

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

		$sql = 'SELECT assetid'.(($include_type) ? ', type_code' : '').'
				FROM sq_asset
				WHERE type_code = '.$db->quote($type_code);

		if (!$strict) {
			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$sql .= '   OR type_code IN (~SQ0~)';
			$subs = Array( 'SELECT type_code
							FROM sq_asset_type_inherited
							WHERE inherited_type_code = '.$db->quote($type_code));
			$sql = db_extras_subquery($db, $sql, $subs);
		}

		$result = ($include_type) ? $db->getAssoc($sql) : $db->getCol($sql);
		if (DB::isError($result)) {
			if (!$mute_errors) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		return $result;

	}//end getTypeAssetids()


	/**
	* Returns a reference to the asset that has the passed url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol		the protocol to match -> null means it is ignored
	* @param string		$url			the urls to check for -> null defaults it to current url
	* @param boolean	$exact_match	when true only returns an asset if there is an exact match on the url,
	*									if false searches back along the URL path for the asset that matches
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because you can't
	*									use the '@' operator when returning by reference
	*
	* @return object Asset | null
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

		$extra_table = '';
		$extra_where = '';
		if (!is_null($protocol)) {
			$extra_table = ' INNER JOIN sq_asset_url u ON l.root_urlid = u.urlid';
			$extra_where = ' AND u.'.$protocol.' = '.$db->quote(1);
		}

		$sql = 'SELECT l.url, l.assetid
				FROM sq_asset_lookup l '.$extra_table.'
				WHERE l.url '.$url_condition.'
				'.$extra_where.'
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
	* @param string		$url		the urls to check for (excluding protocol) -> null defaults it to current url
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
		$sql = 'SELECT urlid, assetid, url, http, https
				FROM sq_asset_url
				WHERE '.$db->quote($url).' LIKE '.$concat.'
				ORDER BY url DESC
				LIMIT 1';
		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getRootURL()


	/**
	* Returns an array of assetid's in the order that they propogate out from the site url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol	the protocol to match -> null means it is ignored
	* @param string		$url		the urls to check for -> null defaults it to current url
	* @param string		$name_type	the type of name to return for each asset
	*
	* @return Array
	* @access public
	*/
	function getLineageFromURL($protocol=null, $url=null, $name_type='name')
	{

		if (is_null($url)) $url = current_url(false);
		// remove any trailing slashes and double slashes
		$url = strip_url($url);

		if (!is_null($protocol) && $protocol != 'https') $protocol = 'http';
		if ($name_type != 'short_name') $name_type = 'name';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get the root(site) url for this url
		$root_url = $this->getRootURL($url);
		if (empty($root_url)) {
			trigger_error('Unable to find an lineage for the URL "'.$fancy_url.'", no root url was found', E_USER_WARNING);
			return Array();
		}

		$extra_table = '';
		$extra_where = '';
		if (!is_null($protocol)) {
			$extra_table = ', sq_asset_url u';
			$extra_where = ' AND l.root_urlid = u.urlid AND u.'.$protocol.' = '.$db->quote(1);
		}

		$concat = ($db->phptype == 'mysql') ? 'CONCAT(l.url, '.$db->quote('%').')' : '(l.url || '.$db->quote('%').')';
		$sql = 'SELECT l.url, l.assetid, a.'.$name_type.', a.type_code
				FROM sq_asset_lookup l INNER JOIN sq_asset a ON l.assetid = a.assetid
				'.$extra_table.'
				WHERE l.root_urlid = '.$db->quote((int) $root_url['urlid']).'
				  AND '.$db->quote($url).' LIKE '.$concat.'
				'.$extra_where.'
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
					$result[] = Array ('url'		=> $result[0]['url'],
										'assetid'	=> $index_page->id,
										$name_type	=> $index_page->$name_type,
										'type_code'	=> $index_page->type()
									);
				}// end if
			}// end if

			return $result;
		}// end if

	}//end getLineageFromURL()


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


	/**
	* Returns the URL for an asset, closest to the $base_url
	*
	* @param int	$assetid	the id of the asset you want the url for
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getAssetURL($assetid, $base_url=null)
	{
		if (is_null($base_url)) $base_url = current_url(false);
		if (!isset($this->_tmp['urls'][$assetid])) $this->_tmp['urls'][$assetid] = Array();

		$base_url = strip_url($base_url, true);

		if (!isset($this->_tmp['urls'][$assetid][$base_url])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$url_bits = explode('/', preg_replace('/\\/+$/', '', $base_url));

			$sql = 'SELECT l.url, u.http, (';
			$str = '';
			// so much for SQL standards...
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(l.url, '.$db->quote('/').')' : 'l.url || '.$db->quote('/');
			foreach($url_bits as $i => $bit) {
				$str .= $bit.'/';
				$sql .= (($i) ? '+' : '').'
				(CASE WHEN SUBSTRING('.$concat.' FROM 1 FOR '.strlen($str).') = '.$db->quote($str).' THEN 1 ELSE 0 END)';
			}

			$sql .= '
			) as weighting
			FROM sq_asset_lookup l INNER JOIN sq_asset_url u ON l.root_urlid = u.urlid
			WHERE l.assetid = '.$db->quote($assetid).'
			ORDER BY weighting DESC, l.url
			LIMIT 1';
			$result = $db->getRow($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return '';
			}

			if (empty($result)) {
				$this->_tmp['urls'][$assetid][$base_url] = '';
			} else {
				$this->_tmp['urls'][$assetid][$base_url] = (($result['http']) ? 'http' : 'https').'://'.$result['url'];
			}

		}// end if !isset

		return $this->_tmp['urls'][$assetid][$base_url];

	}//end getAssetURL()


	/**
	* Returns the href for an asset closest to the $base_url, but relative to the current url
	*
	* @param int	$assetid	the id of the asset you want the href for
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getAssetHref($assetid, $base_url=null)
	{
		if (is_null($base_url)) $base_url = current_url(false);
		if (!isset($this->_tmp['hrefs'][$assetid])) $this->_tmp['hrefs'][$assetid] = Array();

		$base_url = strip_url($base_url, true);
		if (!isset($this->_tmp['hrefs'][$assetid][$base_url])) {
			if ($url = $this->getAssetURL($assetid, $base_url)) {
				require_once SQ_FUDGE_PATH.'/general/www.inc';
				// deliberatley don't strip url here because if there is any trailing slashes then 
				// relative_href will work to accomodate them
				$this->_tmp['hrefs'][$assetid][$base_url] = relative_href(current_url(), $url);
			} else {
				$this->_tmp['hrefs'][$assetid][$base_url] = '';
			}
		}// end if !isset
		return $this->_tmp['hrefs'][$assetid][$base_url];
	}//end getAssetHref()

}//end class
?>
