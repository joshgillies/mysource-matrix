<?php

/**
* Custom_Asset_Type
*
* Purpose
*
*    Looks after the manipulation of custom assets (creation, editing attribute types etc)
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Custom_Asset_Type extends Resolve_Object
{

	/**
	* The type code for the asset type that we are currently representing
	*
	* @var string
	*/
	var $type_code;

	/**
	* @var object Package_Manager
	*/
	var $_pm;
	
	/**
	* @var object Asset_Manager
	*/
	var $_am;

	/**
	* @var object DB
	*/
	var $_db;

	/**
	* The path from the system root to where the custom asset types classes are stored
	*
	* @var string
	*/
	var $_inc_path;

	/**
	* Constructor
	*
	* @param string    $type_code      the type_code for the custom asset that we are managing
	*
	*/
	function Custom_Asset_Type($type_code='') 
	{
		$this->Resolve_Object();
		$this->_am = &$GLOBALS['SQ_RESOLVE']->am;
		$this->_db = &$GLOBALS['SQ_RESOLVE']->getDb();

		$this->_inc_path = preg_replace('/^' .str_replace('/', '\\/', SQ_SYSTEM_ROOT).'\//', '', SQ_DATA_PATH.'/custom_asset_types');

		if ($type_code) {
			$this->load($type_code);
		}
	}

	/**
	* Create's a custom asset
	*
	* @return boolean
	* @access public
	*/
	function create($type_code, $parent_type) 
	{

		# get how deep we are in the heirarchy
		$level = count($this->_am->getParentList($parent_type)) + 1;

		$name = ucwords(str_replace('_', ' ', $type_code));

		$sql = 'INSERT INTO sq_asset_type
				(type_code, version, name, description, instantiable, parent_type, level, dir, customisation)
				VALUES
				('.$this->_db->quote($type_code).', '.$this->_db->quote('0.1').', 
				'.$this->_db->quote($name).', '.$this->_db->quote('').',
				 1, '.$this->_db->quote($parent_type).',
				'.$this->_db->quote($level).', '.$this->_db->quote($this->_inc_path).', 1)';
		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		# now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($type_code);

		$this->load($type_code);

		return true;

	}#end create()

	/*
	* Load the custom asset represented by type_code
	*
	* @param string    $type_code
	*
	* @access public
	*/
	function load($type_code)
	{

		# OK, the first thing to do is check we actually exist :)

		$sql = 'SELECT assetid, type_code, last_updated, last_userid
				FROM sq_asset
				WHERE assetid = '.$db->quote($assetid);
		$result = $db->getRow($sql, NULL, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) {
			trigger_error('Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return;
		}

		list($this->id, $type_code, $this->last_updated, $this->last_userid) = $result;
		unset($result);

		# make sure the asset we are loading is of the same type as our class
		if ($type_code != get_class($this)) {
			trigger_error('Asset #'.$assetid.' is not a '.get_class($this), E_USER_WARNING);
			$this->id   = null;
			$this->last_updated = null;
			$this->last_userid  = null;
			return;
		}

		$this->_loadVars();

	}#end load()


	/*
	* Loads the vars for this asset into the vars array
	*
	* @access public
	*/
	function _loadVars() {

		if (!$this->id) return;

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		# Right, now we need to get any values that this asset has customised
		$this->vars = Array();

		$sql = 'SELECT a.attributeid, a.name, v.value
				FROM sq_asset_attribute a, sq_asset_attribute_value v
				WHERE a.attributeid = v.attributeid
				  AND v.assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);


		$vars_str = '';
		while (NULL !== ($row = $result->fetchRow())) {
			$vars_str .= (($vars_str) ? ',' : '').$db->quote($row['name']);
			$this->vars[$row['name']] = Array('attributeid' => $row['attributeid'], 'value' => $row['value']);
		}#end while
		$result->free();

		$parents = $this->getParentList();
		$parents[] = get_class($this);
		# OK, because of the way things work we now need to get any vars that this asset has
		# or has inherited but has not customised - ie the default values
		# NOTE: we specifically don't get defaults for vars we have customised
		$parents_str = '';
		foreach($parents as $i) $parents_str .= (($parents_str) ? ',' : '').$db->quote($i);
		$sql = 'SELECT a.attributeid, a.name, d.value
				FROM sq_asset_attribute a, sq_asset_attribute_default d, sq_asset_type t
				WHERE a.attributeid = d.attributeid
				  AND d.type_code = t.type_code '
				.(($parents_str) ? ' AND t.type_code IN ('.$parents_str.') ' : '')
				.(($vars_str)    ? ' AND a.name  NOT IN ('.$vars_str.') '    : '')
				.'ORDER BY a.attributeid, t.level';

		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		while (NULL !== ($row = $result->fetchRow())) {
			$this->vars[$row['name']] = Array('attributeid' => $row['attributeid'], 'value' => $row['value']);
		}#end while
		$result->free();



}#end class
?>
