<?php

require_once SQ_LIB_PATH.'/xml_array/xml_array.inc';

/**
* Asset_Management
*
* Purpose
*
*    Looks after the installing and upgrading of an asset
*
*
*    !IMPORTANT! Only to be used by the Package_Manager
*    
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Asset_Management extends Resolve_Object
{

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
	* Holds all the vars used for this Asset in the form
	* Array(var_name => Array(
	*						'added'     => '0.0.1',
	*						'removed'   => '9.9.0',
	*						'type'      => 'text|...|...',
	*						'default'   => '',
	*						'update_fn' => 'update_var_name'
	*						)
	*/
	var $vars;

	/**
	* Constructor
	*
	*/
	function Asset_Management(&$pm)
	{
		$this->_pm = &$pm;
		$this->_am = &$GLOBALS['SQ_RESOLVE']->am;
		$this->_db = &$GLOBALS['SQ_RESOLVE']->getDb();

		$this->vars = Array();

	}


	/**
	* This get's run to intall/upgrade the asset that this management class represents
	*
	* @access public
	*/
	function update() {

		# if the asset manager knows about it it is installed, so upgrade...
		if ($this->_am->installed($this->getAssetType())) {
			return $this->_upgrade($this->_am->getInfo($this->getAssetType(), 'version'));
		} else {
			return $this->_install();
		}

	}#end update()

	/**
	* Install
	*
	* @access public
	*/
	function _install() 
	{

		$type_code = preg_replace('/[^a-z_]/i', '', $this->getAssetType());

		# get how deep we are in the heirarchy
		$level = count($this->_pm->getParentList($type_code));

		$info = $this->_pm->getInfo($type_code);
		$sql = 'INSERT INTO sq_asset_type
				(type_code, version, name, description, instantiable, parent_type, level, dir, customisation)
				VALUES
				('.$this->_db->quote($type_code).', '.$this->_db->quote($info['version']).', 
				'.$this->_db->quote($info['name']).', '.$this->_db->quote($info['description']).',
				'.$this->_db->quote($info['instantiable']).', '.$this->_db->quote($info['parent_type']).',
				'.$this->_db->quote($level).', '.$this->_db->quote($info['dir']).', 0)';
//		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		# now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($type_code);

		return $this->_updateVars();

	}#end install()


	/**
	* Upgrade
	*
	* @access public
	*/
	function _upgrade($current_version) {

		# get how deep we are in the heirarchy
		$level = count($this->_pm->getParentList($this->getAssetType()));

		$info = $this->_pm->getInfo($this->getAssetType());
		$sql = 'UPDATE sq_asset_type
				SET version = '.$this->_db->quote($info['version']).',
					name    = '.$this->_db->quote($info['name']).',
					description  = '.$this->_db->quote($info['description']).',
					instantiable = '.$this->_db->quote($info['instantiable']).',
					parent_type  = '.$this->_db->quote($info['parent_type']).',
					level   = '.$this->_db->quote($level).',
					dir     = '.$this->_db->quote($info['dir']).'
				WHERE type_code = '.$this->_db->quote($this->getAssetType());

//		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		# now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($this->getAssetType());

		return $this->_updateVars();

	}#end upgrade()

	/*
	* Returns the asset type that this management class is working for
	*
	* @access public
	*/
	function getAssetType() {

		$class = get_class($this);
		return substr($class, 0, strlen($class) - 11);

	}#end getAssetType()


	/*
	* Adds the passed attribute to this asset's list
	*
	* @access public
	*/
	function _updateVars() {

		$sql = 'SELECT name, attributeid, type
				FROM sq_asset_attribute
				WHERE type_code = '.$this->_db->quote($this->getAssetType());
		
		$current_vars = $this->_db->getAssoc($sql, false, array(), DB_FETCHMODE_ASSOC); # need all this because of DB API

		pre_echo('Current Vars:');
		pre_echo($current_vars);

		for(reset($this->vars); NULL !== ($name = key($this->vars)); next($this->vars)) {

			# if this var was never added to this asset, then we are setting the default
			# for something further upstream
			if (empty($this->vars[$name]['added'])) {
				if (!$this->_addSetDefault($name, $this->vars[$name]['default'])){
					return false;
				}
			# if this var was never added to this asset, then we are setting the default
			# for something further upstream
			} elseif (empty($current_vars[$name])) {
				if (!$this->_addAttribute($name)){
					return false;
				}
			} else {
				if (!$this->_updateAttribute($name)){
					return false;
				}
			}#end if

		}#end for

		return true;

	}#end _updateVars()


	/*
	* Adds an attribute to this asset's set
	*
	* @param string $name  the name of the attribute to add
	*
	* @return boolean
	* @access private
	*/
	function _addAttribute($name) {

		$type_code = $this->getAssetType();

		# the first this to do is checks to see if the passed attribute name is used by any 
		# of this asset types parents or of its children - we don't want name clashes :)

		# Check Parent's
		$sql = 'SELECT a.type_code
				FROM sq_asset_type_inherited i, sq_asset_attribute a
				WHERE i.type_code = '.$this->_db->quote($type_code).'
				  AND i.inherited_type_code = a.type_code
				  AND a.name = '.$this->_db->quote($name);

//		pre_echo($sql);

		$parents = $this->_db->getCol($sql);
		if (DB::isError($parents)) {
			trigger_error($parents->getMessage().'<br>'.$parents->getUserInfo(), E_USER_WARNING);
			return false;
		}
		if (count($parents)) {
			trigger_error('Unable to add var "'.$name.'" to Asset "'.$type_code.'" because this variable name is already used by it\'s ancestor "'.implode('","', $parents).'"' , E_USER_WARNING);
			return false;
		}

		# Check Children
		$sql = 'SELECT a.type_code
				FROM sq_asset_type_inherited i, sq_asset_attribute a
				WHERE i.inherited_type_code = '.$this->_db->quote($type_code).'
				  AND i.type_code = a.type_code
				  AND a.name = '.$this->_db->quote($name);
//		pre_echo($sql);

		$children = $this->_db->getCol($sql);
		if (DB::isError($children)) {
			trigger_error($children->getMessage().'<br>'.$children->getUserInfo(), E_USER_WARNING);
			return false;
		}
		if (count($children)) {
			trigger_error('Unable to add var "'.$name.'" to Asset "'.$type_code.'" because variable name is already used by it\'s children "'.implode('","', $children).'"' , E_USER_WARNING);
			return false;
		}


		trigger_error('Need to have check to make sure that the variable type is valid' , E_USER_WARNING);


		$attributeid = $this->_db->nextid('sq_sequence_asset_attribute');

		# OK, all seems fine, let's add the bloody thing
		$sql = 'INSERT INTO sq_asset_attribute 
				(attributeid, type_code, name, type)
				VALUES
				('.$this->_db->quote($attributeid).', '.$this->_db->quote($type_code).', '.$this->_db->quote($name).', '.$this->_db->quote($this->vars[$name]['type']).')';

		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return $this->_setAttributeDefault($name, $this->vars[$name]['default']);

	}#end _addAttribute()


	/*
	* Updates an attribute in this asset's set
	*
	* @param string $name  the name of the attribute to update
	*
	* @return boolean
	* @access private
	*/
	function _updateAttribute($name) {

		$type_code = $this->getAssetType();

		trigger_error('Need to have check to make sure that the variable type is valid' , E_USER_WARNING);


		if (!empty($this->vars[$name]['update_fn'])) {

			$fn = $this->vars[$name]['update_fn'];
			return $this->$fn($name);

		} else {

			# OK, all seems fine, let's add the bloody thing
			$sql = 'UPDATE sq_asset_attribute 
					SET type = '.$this->_db->quote($this->vars[$name]['type']).'
					WHERE type_code = '.$this->_db->quote($type_code).'
					  AND name = '.$this->_db->quote($name);

//			pre_echo($sql);
			$result = $this->_db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			return $this->_setAttributeDefault($name, $this->vars[$name]['default']);

		}

	}#end _updateAttribute()

	/*
	* Set's the attributes default to the 
	*
	* @param string $name  the name of the attribute to add
	*
	* @return boolean
	* @access private
	*/
	function _setAttributeDefault($name, $value) {

		$type_code = $this->getAssetType();

		$sql = 'SELECT a.attributeid
				FROM sq_asset_attribute a
				WHERE a.type_code = '.$this->_db->quote($type_code).'
				  AND a.name = '.$this->_db->quote($name);

//		pre_echo($sql);
		$attributeid = $this->_db->getOne($sql);
		if (DB::isError($attributeid)) {
			trigger_error($attributeid->getMessage().'<br>'.$attributeid->getUserInfo(), E_USER_WARNING);
			return false;
		}
		if (empty($attributeid)) {
			# see if they are in one of our parents
			$sql = 'SELECT a.attributeid
					FROM sq_asset_type_inherited i, sq_asset_attribute a
					WHERE i.type_code = '.$this->_db->quote($type_code).'
					  AND i.inherited_type_code = a.type_code
					  AND a.name = '.$this->_db->quote($name);

//			pre_echo($sql);
			$attributeid = $this->_db->getOne($sql);
			if (DB::isError($attributeid)) {
				trigger_error($attributeid->getMessage().'<br>'.$attributeid->getUserInfo(), E_USER_WARNING);
				return false;
			}

			if (empty($attributeid)) {
				trigger_error('Attribute "'.$name.'" not found for Asset"'.$type_code.'" or any of it\'s parents', E_USER_WARNING);
				return false;
			}

		}#end if



		# delete previous entry, if any
		$sql = 'DELETE FROM sq_asset_attribute_default
				WHERE attributeid = '.$this->_db->quote($attributeid).'
				  AND type_code   = '.$this->_db->quote($type_code);
//		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		# now insert new entry
		$sql = 'INSERT INTO sq_asset_attribute_default
				(attributeid, type_code, value)
				VALUES
				('.$this->_db->quote($attributeid).', '.$this->_db->quote($type_code).', '.$this->_db->quote($value).')';

//		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

//		pre_echo('RETURN TRUE');

		return true;

	}#end _setAttributeDefault()


	/*
	* Returns the information from the passed asset.xml
	* This function can be called without creating an instance of the class
	* ie $info = Asset_Management::getAssetInfo($file);
	* 
	* @param string $file  the asset.xml file path
	*
	* @access public
	*/
	function getAssetInfo($file)
	{

		# OK, now because this can be called without the need to have an instance of the class
		# we need to store our tmp stuff in a global array
		if (!isset($GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'])) $GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'] = Array();

		if (!isset($GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file])) {

			$xml_array = new XML_Array(Asset_Management::getAssetArrayPaths());

			$info = $xml_array->getArrayFromFile($file);
			if (PEAR::isError($info)) {
				trigger_error($info->error_message_prefix.$info->message.'<br>In File:'.$file, E_USER_WARNING);
				return Array();
			}

			$xml_array->close();

			$GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file] = Array(
																	'type_code'    => strtolower($info['type_code']['value']),
																	'package'      => $info['package']['value'],
																	'name'         => $info['name']['value'],
																	'version'      => $info['version']['value'],
																	'description'  => $info['description']['value'],
																	'instantiable' => ($info['instantiable']['value'] == 'yes'),
																	'parent_type'  => $info['parent_type']['value']
																);

		}#end if
		
		return $GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file];

	}#end getAssetInfo()

	/*
	* Used by the XML_Array()
	* 
	* @see XML_Array::XML_Array()
	*
	* @access public
	*/
	function getAssetArrayPaths()
	{
		return Array('asset_info/requirements/requirement');
	}


}#end class
?>
