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
	}//end getInfo()


	/**
	* Returns an array of asset information or a specific piece of information
	*
	* @param string	$type_code	the code name for the asset that you want to check
	* @param string	$field		if exists, returns this information from the array for this specific field
	*
	* @return mixed
	* @access public
	*/
	function getInfo($type_code, $field='')
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

	}//end getInfo()


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
	* @param int	$assetid	the id of the asset to get its parents for
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
			if ($strict_type_code) {
				if (is_array($type_code)) {
					$types = '';
					foreach ($type_code as $type) $types .= $db->quote($type).', ';
					$sql .= ' AND pa.type_code IN ('.trim($types, ', ').')';
				} else {
					$sql .= ' AND pa.type_code = '.$db->quote($type_code);
				}
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND (pa.type_code = '.$db->quote($type_code).' OR pa.type_code IN (~SQ0~))';
				$sub = 'SELECT type_code
								FROM sq_asset_type_inherited ';
				if (is_array($type_code)) {
					$types = '';
					foreach ($type_code as $type) $types .= $db->quote($type).', ';
					$sub .= ' WHERE inherited_type_code IN ('.trim($types, ', ').')';
				} else {
					$sub .= ' WHERE inherited_type_code = '.$db->quote($type_code);
				}
				$sql = db_extras_subquery($db, $sql, Array($sub));
			}
		}// end if

		$parents = $db->getCol($sql);

		if (DB::isError($parents)) {
			trigger_error($parents->getMessage().'<br/>'.$parents->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		return $parents;
	}


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
				return $this->getAsset(1);
			case 'trash_folder' :
				return $this->getAsset(2);
			case 'system_user_group' :
				return $this->getAsset(3);
			case 'root_user' :
				return $this->getAsset(4);
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

		if (is_null($url)) $url = $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'];
		// remove any trailing slashes
		$url = preg_replace('/\/+$/', '', $url);

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
	* Returns an array of assetid's in the order that they propogate out from the site url
	* If no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol	the protocol to match -> null means it is ignored
	* @param string		$url		the urls to check for -> null defaults it to current url
	*
	* @return Array
	* @access public
	*/
	function getLineageFromURL($protocol=null, $url=null)
	{

		if (is_null($url)) $url = $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'];
		// remove any trailing slashes
		$url = preg_replace('/\/+$/', '', $url);

		if (!is_null($protocol) && $protocol != 'https') $protocol = 'http';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sections = explode('/', $url);
		$option  = '';
		$url_condition = ' IN (';
		foreach($sections as $piece) {
			$url_condition .= (($option) ? ',' : '').$db->quote($option.$piece);
			$option .= $piece.'/';
		}
		$url_condition .= ') ';

		$extra_table = '';
		$extra_where = '';
		if (!is_null($protocol)) {
			$extra_table = ', sq_asset_url u';
			$extra_where = ' AND l.root_urlid = u.urlid AND u.'.$protocol.' = '.$db->quote(1);
		}

		$sql = 'SELECT l.url, l.assetid, a.name, a.type_code
				FROM sq_asset_lookup l, sq_asset a '.$extra_table.'
				WHERE l.url '.$url_condition.'
				AND l.assetid = a.assetid
				'.$extra_where.'
				ORDER BY CHARACTER_LENGTH(l.url) ASC';
		$result = $db->getAll($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		// No urls found or we only found some of the parents, but not the passed url
		if (empty($result) || $result[count($result) - 1]['url'] != $url) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			trigger_error('Unable to find an lineage for the URL "'.$fancy_url.'"', E_USER_WARNING);
			return null;

		} else {
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

		$o  = &$backend->out;

		if (!isset($_GET['am_section'])) $_GET['am_section'] = '';
		$o->addFormActionGetVar('am_section', $_GET['am_section']);
		$o->addHiddenField('am_form_submitted', '1');

		switch($_GET['am_section']) {
			case 'add_asset' :
				$parent = &$this->getAsset($_GET['parent_assetid']);
				if (is_null($parent)) {
					trigger_error('Parent Asset #'.$_GET['parent_assetid'].' not found', E_USER_WARNING);
					break;
				}

				// make sure that we are allowed to link this type to the parent
				if (($err_msg = $parent->canLinkToType($_GET['type_code'], $_GET['link_type'])) !== true) {
					trigger_error($err_msg, E_USER_WARNING);
					break;
				}

				$o->addFormActionGetVar('parent_assetid', $_GET['parent_assetid']);
				$o->addFormActionGetVar('pos',            $_GET['pos']);
				$o->addFormActionGetVar('type_code',      $_GET['type_code']);
				$o->addFormActionGetVar('link_type',      $_GET['link_type']);

				$this->includeAsset($_GET['type_code']);
				$asset = new $_GET['type_code']();

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				if (!empty($_POST['am_form_submitted'])) {

					// start the transaction to create the asset and initial link
					$link = Array('asset'     => &$parent,
								  'link_type' => $_GET['link_type'],
								  'value'     => '',
								  'position'  => $_GET['pos']);

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$success = $asset->processBackend($link);

					if ($success) {

						$o->addMessage(SQ_BO_MSG_NOTICE, 'New '.$this->_asset_types[$_GET['type_code']]['name'].' "'.$asset->name.'" created');
						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" successfully linked to "'.$parent->name.'"');
						$this->registerAsset($asset);
						$o->addFormActionGetVar('am_section', 'edit_asset');
						$o->addFormActionGetVar('assetid', $asset->id);
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

					} else {
						// the asset was not created fully or linking failed
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
				}

				$asset->paintBackend($backend);

				break;

			case 'edit_asset' :

				$o->addFormActionGetVar('assetid', $_GET['assetid']);

				$asset = &$this->getAsset($_GET['assetid']);
				if (is_null($asset)) {
					trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
					break;
				}

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				if (!empty($_POST['am_form_submitted']) && $asset->processBackend(Array())) {
					$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" Updated');
				}// end if

				$asset->paintBackend($backend);
				break;

			case 'duplicate' :

				$o->addFormActionGetVar('assetid', $_GET['assetid']);
				$asset = &$this->getAsset($_GET['assetid']);
				if (is_null($asset)) {
					trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
					break;
				}

				require_once SQ_LIB_PATH.'/progress_bar/progress_bar.inc';
				$pb = new Progress_Bar($o->getCurrentLocation(), $this, '_dupeInit', '_dupeInitPaint', '_dupeExec', '_dupeDone');

				pre_echo($_GET);
				pre_echo($_POST);
				pre_echo($GLOBALS['SQ_SYSTEM']->datetime());
				$pb->run();

				if ($pb->inPopup()) {
					$o->openRaw();
				} else {
					$o->setHeading('Duplicate : '.$asset->name);
					$o->openSection('Options');
					$o->openField('');
				}

				$pb->paint();

				if ($pb->inPopup()) {
					$o->closeRaw();
				} else {
					$o->closeSection();
					$o->commitButton();
				}
				break;


			default   :
				$this->_printAssetList($o);

		}//end switch

	}//end paintBackend();


	/**
	*/
	function _dupeInit(&$pb)
	{
		$vars = $pb->getRunningVars();
		pre_echo("VARS : ");
		pre_echo($vars);
		if (empty($vars)) {
			$vars = Array(	'assetid'			=> $_GET['assetid'],
							'to_parent_assetid' => $_GET['to_parent_assetid'],
							'to_parent_pos'		=> $_GET['to_parent_pos']
						);
		}
		pre_echo("VARS : ");
		pre_echo($vars);
		$vars['test'] = empty($vars['test']) ? 1 : ($vars['test'] + 1);
		pre_echo("TEST : ".$vars['test']);

		$pb->setRunningVars($vars);

		#return ($vars['test'] >= 3);
		return true;

	}

	/**
	*/
	function _dupeInitPaint(&$pb)
	{
		echo "SUBMIT AGAIN";
	}

	/**
	*/
	function _dupeExec(&$pb)
	{
		$vars = $pb->getRunningVars();
		$vars['run_num'] = empty($vars['run_num']) ? 1 : ($vars['run_num'] + 1);

		pre_echo("RUN NUM : ".$vars['run_num']);

		sleep(1);

		$pb->setTitle('Run Number : '.$vars['run_num']);
		$pb->setStatusMsg('Running'.str_repeat('.', $vars['run_num']));
		$pb->setPercent($vars['run_num'] / 5);
		$pb->setRunningVars($vars);
	}

	/**
	*/
	function _dupeDone(&$pb)
	{
		$vars = $pb->getRunningVars();
		pre_echo("ALL DONE");
		pre_echo($vars);
		$pb->setTitle('Finished');
		$pb->setStatusMsg('Finished');
	}



	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @param object Backend_Outputter $o
	* @param object DB $db
	* @param string $parent  the parent asset type that to have it's kids printed
	*
	* @access public
	*/
	function _printAssetList(&$o)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		/*
		if (!empty($_REQUEST['am_action'])) {

			require_once SQ_INCLUDE_PATH.'/custom_asset_type.inc';

			$o->addFormActionGetVar('am_action', 'edit');

			pre_echo($_POST);
			switch ($_REQUEST['am_action']) {
				case 'create_custom' :
					$custom = new Custom_Asset_Type();
					if ($custom->create($_REQUEST['am_new_type_code'], $_REQUEST['am_type_code'])) {
						$o->addFormActionGetVar('am_type_code', $_REQUEST['am_new_type_code']);
						$custom->paintBackend($backend);
						return;
					}
					break;
				case 'edit' :
					$o->addFormActionGetVar('am_type_code', $_REQUEST['am_type_code']);
					$custom = new Custom_Asset_Type($_REQUEST['am_type_code']);
					$custom->paintBackend($backend);
					return;
					break;
			}
		}//end if
		*/

		//$o->addCrummingLink('Asset Types', $o->getCurrentLocation());
		$o->setHeading('Assets Types', 'create');
		$o->addHiddenField('am_action');
		$o->addHiddenField('am_new_type_code');

		$o->openSection('Existing Assets');
		$o->openField('', 'new_line');


		$offspring = Array();
		for (reset($this->_asset_types); null !== ($type_code = key($this->_asset_types)); next($this->_asset_types)) {
			$parent = $this->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}//end for

	?>
		<script language="JavaScript" type="text/javascript">
		<!--

			var current_asset_name = '';

			function asset_checked(asset_name, customisation) {
				current_asset_name = asset_name;

				var button = get_element('edit_button');
				if (customisation) {
					button.value = 'Edit ' + asset_name;
				} else {
					button.value = '- - - - - - - - - - - - -';
				}
			}

			function create_custom() {

				if (get_element_value('am_type_code')) {

					var type_codes = new Array("<?php echo implode('", "', array_keys($this->_asset_types)); ?>");
					var new_type_code = '';
					var prompt_str = '';
					do {

						if (new_type_code == '') {
							prompt_str = 'Please Enter a unique Asset Type Code for your new customisation';
						} else {
							prompt_str = '"' + new_type_code + '" is already in use\nPlease enter a unique Asset Type Code';
						}

						new_type_code = prompt(prompt_str, new_type_code);
						// they hit cancel
						if (new_type_code == null) return false;

						// make sure it's in a proper format
						new_type_code = new_type_code.toLowerCase();
						new_type_code = new_type_code.replace(/ /g, '_');
						new_type_code = new_type_code.replace(/[^a-z_]/g, '');
						new_type_code = new_type_code.replace(/_+/g, '_');

					} while (array_search(type_codes, new_type_code) != null)

					set_hidden_field('am_new_type_code', new_type_code);
					set_hidden_field('am_action', 'create_custom');
					return true;
				}
				return false;
			}


		//-->
		</script>
		<table cellpadding="0" cellspacing="0" border="0">
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					&nbsp;Asset
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				$this->_recursePrintAssetList($o, $offspring, 'asset');
			?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				submit_button('create_button', 'Create Custom Asset', 'return create_custom();');
				submit_button('edit_button', '- - - - - - - - - - - - -', 'if (this.value.substr(0, 4) == \'Edit\') { set_hidden_field(\'am_action\', \'edit\'); return true; } else { return false };');
			?>
				</td>
			</tr>
		</table>
	<?php

	}// end printAssetList()

	function _recursePrintAssetList(&$o, &$offspring, $parent)
	{

	?>
		<table cellpadding="0" cellspacing="0" border="0">
	<?php
		$num_kids = count($offspring[$parent]);
		for ($i = 0; $i < $num_kids; $i++) {

			$type_code = $offspring[$parent][$i];
			$end = ($i == $num_kids - 1);
//			$bg_img = 'images/page_'.(($page[effective_visible]) ? '' : 'in').'visible_'.page::get_status_colour($page[effective_status]).'.gif';

			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?>
			<tr>
				<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					<?php radio_button('am_type_code', $type_code, false, 'asset_checked(\''.addslashes($this->_asset_types[$type_code]['name']).'\', '.(($this->_asset_types[$type_code]['customisation']) ? 'true' : 'false').');'); ?>
					<span class="sq-backend-<?php echo ($this->_asset_types[$type_code]['customisation']) ? 'custom-' : ''?>asset"><?php echo $this->_asset_types[$type_code]['name'].' ('.$type_code.')';?></span>
				</td>
			</tr>
		<?php
			if (!empty($offspring[$type_code])) {
			?>
				<tr>
					<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
					<td class="sq-backend-data">
				<?php
					$this->_recursePrintAssetList($o, $offspring, $type_code);
				?>
					</td>
				</tr>
			<?php
			}//end if

		}//end for
	?>
		</table>
	<?php

	}//end _printAssetList()

}//end class
?>
