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
		if (!$GLOBALS['SQ_INSTALL']) $this->_loadAssetTypes();

	}//end constructor


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
		$sql = 'SELECT a.assetid, at.inherited_type_code 
				FROM sq_asset a INNER JOIN sq_asset_type_inherited at ON a.type_code = at.type_code 
				WHERE ';

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
				$sql .= ' AND a.type_code IN (~SQ0~)';
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
				$sql .= ' AND a.type_code IN (~SQ0~)';
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
								WHERE type_code = '.$db->quote($type_code).'
								  AND inherited_type_code <> '.$db->quote($type_code).'
								'.(($include_asset) ? '' : ' AND inherited_type_code <> '.$db->quote('asset'))
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
								WHERE inherited_type_code = '.$db->quote($type_code).'
								  AND type_code <> '.$db->quote($type_code)
								);
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
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc) 
	*											if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code 
	*											or $type_code and any of it's sub-classes
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
				$sql .= ' AND pa.type_code IN (~SQ0~)';
				$sub = 'SELECT type_code
						FROM sq_asset_type_inherited
						WHERE inherited_type_code '.$type_code_cond;
				$sql = db_extras_subquery($db, $sql, Array($sub));
			}
		}// end if

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
	* @param string|array	$type_code			the type of asset that is linked 
	*											(eg 'Page', 'File', etc) 
	*											if an array returns link if matches any 
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code 
	*											or $type_code and any of it's sub-classes
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
				$sql .= 'a.type_code IN (~SQ0~)';
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
	* Returns true if the passed assetid is in the trash, false otherwise
	* Note that if the passed assetid is the id of the trash folder, false will be returned
	*
	* @param int		$assetid		the asset id to check the trash for
	* @param boolean	$exclusively	is the asset ONLY in the trash
	*									(ie. does it have no other significant links
	*									outside the trash?)
	*
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

		$sql = 'SELECT COUNT(*)
				FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid=l.linkid
				WHERE l.minorid = '.$db->quote($assetid).' 
				  AND t.treeid '.(($exclusively) ? 'NOT ' : '').'LIKE (~SQ0~)';

		// we need to work out the treeid of the trash
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(t.treeid, '.$db->quote('%').')' : '(t.treeid || '.$db->quote('%').')';
		$subs = Array( 'SELECT '.$concat.'
						FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
						WHERE l.minorid = '.$db->quote($trash->id).'
						LIMIT 1');

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
			case 'cron_manager' :
				return $this->getAsset(10, 'cron_manager');
			default :
				trigger_error('System Asset Name "'.$name.'" not known', E_USER_ERROR);
		}

	}//end getSystemAsset()


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
		$parents = $asset->getLinks(SQ_SC_LINK_SIGNIFICANT, '', null, 'minor', '', 1);
		for($i = 0; $i < count($parents); $i++) if ($parents[$i]['majorid'] <= SQ_NUM_RESERVED_ASSETIDS) return false;
		
		return true;

	}//end canPurgeAsset()


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
				WHERE ';

		if ($strict) {
			$sql .= 'type_code = '.$db->quote($type_code);
		} else {
			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$sql .= 'type_code IN (~SQ0~)';
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
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the urls to check for -> null defaults it to current url
	* @param string	$name_type	the type of name to return for each asset
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
					$paths = $index_page->getWebPaths();
					$result[] = Array (
										'url'		=> $result[0]['url'].((empty($paths)) ? '' : '/'.$paths[0]),
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
	* @param int | array(int)	$assetids	the id of the asset you want the url for or an array
	*										for a whole bunch of them
	* @param string				$base_url	the url that we are want to match closest to
	*
	* @return mixed returns string if $assetids is an in and an array(int => string) if $assetids is an array
	* @access public
	*/
	function getAssetURL($assetids, $base_url=null)
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

		if (is_null($base_url)) $base_url = current_url(false);
		$base_url = strip_url($base_url, true);

		$return_urls = Array();

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
		$assetid_cond_array = $the_rest;
		for($i = 0; $i < count($the_rest); $i++) $assetid_cond_array[$i] = $db->quote($assetid_cond_array[$i]);
		$assetid_cond = 'IN ('.implode(', ', $assetid_cond_array).')';

		$url_bits = explode('/', preg_replace('/\\/+$/', '', $base_url));

		$sql = 'SELECT l.assetid, l.url, u.http, (';
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
		WHERE l.assetid '.$assetid_cond.'
		ORDER BY weighting DESC, l.url';
		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return '';
		}

		foreach ($the_rest as $assetid) {
			if (!isset($result[$assetid])) {
				$this->_tmp['urls'][$assetid][$base_url] = '';
			} else {
				$this->_tmp['urls'][$assetid][$base_url] = (($result[$assetid][0][1]) ? 'http' : 'https').'://'.$result[$assetid][0][0];
			}
			$return_urls[$assetid] = $this->_tmp['urls'][$assetid][$base_url];
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
	* @param int | array(int)	$assetids	the id of the asset you want the href for or an array
	*										for a whole bunch of them
	* @param string				$base_url	the url that we are want to match closest to
	*
	* @return mixed returns string if $assetids is an in and an array(int => string) if $assetids is an array
	* @access public
	*/
	function getAssetHref($assetids, $base_url=null)
	{
		if (is_null($base_url)) $base_url = current_url(false);

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

		if (is_null($base_url)) $base_url = current_url(false);
		$base_url = strip_url($base_url, true);

		$return_hrefs = Array();

		// work out if any of these are cached
		foreach ($assetids as $assetid) {
			if (!isset($this->_tmp['hrefs'][$assetid])) $this->_tmp['hrefs'][$assetid] = Array();

			if (isset($this->_tmp['hrefs'][$assetid][$base_url])) {
				// use the cached version
				$return_hrefs[$assetid] = $this->_tmp['hrefs'][$assetid][$base_url];
			}
		}

		$the_rest = array_diff($assetids, array_keys($return_hrefs));
		$urls = $this->getAssetURL($the_rest, $base_url);

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
	* @param array(mixed) $assetinfo an array of asset info in the form assetid => screen code name
	*
	* @return array(int => string) returns an array of assetid and hrefs to the wanted screen
	* @access public
	*/
	function getAssetBackendHref($asset_info)
	{
		if (empty($asset_info)) return Array();

		$return_hrefs = Array();
		foreach ($asset_info as $assetid => $screen) {
			$return_hrefs[$assetid] = sq_web_path('edit').'/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.$assetid.'&asset_ei_screen='.$screen;
		}
		return $return_hrefs;

	} // end getAssetBackendHref()

}//end class

?>