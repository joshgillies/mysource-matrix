<?php

require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
require_once SQ_LIB_PATH.'/xml_array/xml_array.inc';

/**
* Asset_Management
*
* Purpose
*    Looks after the installing and upgrading of an asset
*
*    !IMPORTANT! Only to be used by the Package_Manager
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
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
	* @var array
	* Holds all the vars used for this Asset in the form
	* Array(var_name => Array(
	*						'added'     => '0.0.1',
	*						'removed'   => '9.9.0',
	*						'type'      => 'text|...|...',
	*						'default'   => '',
	*						'update_fn' => 'update_var_name'
	*						)
	*		)
	*/
	var $vars;


	/**
	* Constructor
	*
	*/
	function Asset_Management(&$pm)
	{
		$this->_pm = &$pm;
		$this->_am = &$GLOBALS['SQ_SYSTEM']->am;
		$this->_db = &$GLOBALS['SQ_SYSTEM']->db;

		$this->vars = Array();
	}


	/**
	* Returns the asset type that this management class is working for
	*
	* @access public
	*/
	function getAssetType()
	{

		$class = get_class($this);
		return substr($class, 0, strlen($class) - 11);

	}//end getAssetType()


	/**
	* This get's run to intall/upgrade the asset that this management class represents
	*
	* @return boolean
	* @access public
	*/
	function update()
	{

		// if the asset manager knows about it it is installed, so upgrade...
		if ($this->_am->installed($this->getAssetType())) {
			if (!$this->_upgrade($this->_am->getInfo($this->getAssetType(), 'version'))) return false;
		} else {
			if (!$this->_install()) return false;
		}
		if (!$this->_updateFiles()) return false;
		return true;


	}//end update()


	/**
	* Copy files used by the asset to the data directory
	*
	* @return boolean
	* @access private
	*/
	function _updateFiles() {
		
		# get a list of files to copy
		$file_list = $this->_getFileList();
		if (empty($file_list)) return true;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		
		$to_dir_path = SQ_DATA_PATH.'/unrestricted/'.$this->getAssetType().'/files';
		$from_dir_path = SQ_SYSTEM_ROOT.'/'.$this->_pm->getInfo($this->getAssetType(),'dir');
		
		# create the directory to store these files if it doesnt exist
		# NOTE: The create_direcotyr function of file_system.inc handles this for us
		create_directory($to_dir_path);
		
		# for each file, copy it over
		for (reset($file_list); null !== ($k = key($file_list)); next($file_list)) {
			$file = $file_list[$k];
			$from = $from_dir_path.'/'.$file;
			$to = $to_dir_path.'/'.$file;

			# the copy_file function of file_system.inc will create the
			# directory strucutre before copying
			if (!copy_file($from, $to)) return false;
		}

		return true;
	}


	/**
	* Install
	*
	* @access public
	*/
	function _install()
	{

		$type_code   = $this->getAssetType();

		// get how deep we are in the heirarchy
		$level = count($this->_pm->getParentList($type_code));
		$info = $this->_pm->getInfo($type_code);
		$editing_options = $this->_getEditingOptions();
		if (is_null($editing_options)) return false;

		$sql = 'INSERT INTO sq_asset_type
				(type_code, version, name, description, instantiable, system_only, parent_type, level, dir, customisation, editing_options)
				VALUES
				('.$this->_db->quote($type_code).', '.$this->_db->quote($info['version']).',
				'.$this->_db->quote($info['name']).', '.$this->_db->quote($info['description']).',
				'.$this->_db->quote($info['instantiable']).', '.$this->_db->quote($info['system_only']).',
				'.$this->_db->quote($info['parent_type']).', '.$this->_db->quote($level).',
				'.$this->_db->quote($info['dir']).', 0, '.$this->_db->quote(serialize($editing_options)).')';
#		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($type_code);

		// now add the attributes
		$all_vars = $this->_getAllVars();
		for (reset($all_vars); null !== ($k = key($all_vars)); next($all_vars)) {
			$name = $all_vars[$k];
			if (!$this->_addAttribute($name)) return false;
		}

		return true;

	}//end install()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{

		// get how deep we are in the heirarchy
		$level = count($this->_pm->getParentList($this->getAssetType()));
		$info = $this->_pm->getInfo($this->getAssetType());
		$editing_options = $this->_getEditingOptions();
		if (is_null($editing_options)) return false;

		$sql = 'UPDATE sq_asset_type
				SET version         = '.$this->_db->quote($info['version']).',
					name            = '.$this->_db->quote($info['name']).',
					description     = '.$this->_db->quote($info['description']).',
					instantiable    = '.$this->_db->quote($info['instantiable']).',
					system_only     = '.$this->_db->quote($info['system_only']).',
					parent_type     = '.$this->_db->quote($info['parent_type']).',
					level           = '.$this->_db->quote($level).',
					dir             = '.$this->_db->quote($info['dir']).',
					editing_options = '.$this->_db->quote(serialize($editing_options)).'
				WHERE type_code = '.$this->_db->quote($this->getAssetType());

#		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($this->getAssetType());

		// now update the attributes
		$sql = 'SELECT name
				FROM sq_asset_attribute
				WHERE type_code = '.$this->_db->quote($this->getAssetType());
		$current_vars = $this->_db->getCol($sql);

		$all_vars = $this->_getAllVars();
		for (reset($all_vars); null !== ($k = key($all_vars)); next($all_vars)) {
			$name = $all_vars[$k];

			// if we already have this attribute installed, then let's update it
			if (in_array($name, $current_vars)) {
				if (!$this->_updateAttribute($name, $current_version)) return false;
			} else {
				if (!$this->_addAttribute($name)) return false;
			}
		}//end for

		return true;

	}//end upgrade()


	/**
	* Returns an array of editing options for this asset type
	*
	* @return Array
	* @access private
	*/
	function _getEditingOptions()
	{

		$parents = $this->_pm->getParentList($this->getAssetType());
		// add this type to the front
		array_unshift($parents, $this->getAssetType());

		// remove "asset"
		array_pop($parents);

		$editing_options = Array();

		while(null !== ($type_code = array_pop($parents))) {
			$info = @$this->_pm->getInfo($type_code, 'editing_options');
			// if it ain't in this package, it's has to be installed, get the info from the asset manager
			if (is_null($info)) {
				$tmp = $info['editing_options'];
				$info = @$GLOBALS['SQ_SYSTEM']->am->getInfo($type_code, 'editing_options');
				if (is_null($info)) return null;
			}
			for (reset($info); null !== ($k = key($info)); next($info)) {
				$v = $editing_options[$k];
				$editing_options[$k] = $v;
			}
		}

		for (reset($editing_options); null !== ($k = key($editing_options)); next($editing_options)) {
			$v = $editing_options[$k];
			if (empty($v)) unset($editing_options[$k]);
		}

		return $editing_options;

	}//end _getEditingOptions()


	/**
	* Returns all the variable names that are available for this asset type
	* - any inherited vars and those defined in this management class
	*
	* @return Array(string)
	* @access public
	*/
	function _getAllVars()
	{

		$sql = 'SELECT DISTINCT atr.name
				FROM sq_asset_type_inherited i, sq_asset_attribute atr
				WHERE i.type_code = '.$this->_db->quote($this->getAssetType()).'
				  AND i.inherited_type_code = atr.type_code';

		$parent_vars = $this->_db->getCol($sql);
		// array_values() to get proper indexed array
		$vars = array_values(array_unique(array_merge($parent_vars, array_keys($this->vars))));
		return $vars;

	}// end _getAllVars()


	/**
	* Returns all files this asset wants installed/updated into the data directory
	*
	* @return Array(string)
	* @access private
	*/
	function _getFileList() {
		return Array();
	}


	/**
	* Adds an attribute to this asset's collection
	*
	* @param string $name  the name of the attribute to add
	*
	* @return boolean
	* @access private
	*/
	function _addAttribute($name)
	{

		$type_code = $this->getAssetType();

		$attr_info = Array();

		// See if any other our parents have an attribute with this name
		$sql = 'SELECT COUNT(*) as count, MIN(at.level) as min, MAX(at.level) as max
				FROM sq_asset_type_inherited i, sq_asset_attribute atr, sq_asset_type at
				WHERE i.type_code = '.$this->_db->quote($type_code).'
				  AND i.inherited_type_code = atr.type_code
				  AND atr.name = '.$this->_db->quote($name).'
				  AND atr.type_code = at.type_code';

		$parent_info = $this->_db->getRow($sql);
		if (DB::isError($parent_info)) {
			trigger_error($parent_info->getMessage().'<br/>'.$parent_info->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// if none of our parent types have an attribute with this name
		if (empty($parent_info['count'])) {

			// Check Children to see if any of them have an attribute with this name
			$sql = 'SELECT a.type_code
					FROM sq_asset_type_inherited i, sq_asset_attribute a
					WHERE i.inherited_type_code = '.$this->_db->quote($type_code).'
					  AND i.type_code = a.type_code
					  AND a.name = '.$this->_db->quote($name);
			$children = $this->_db->getCol($sql);
			if (DB::isError($children)) {
				trigger_error($children->getMessage().'<br/>'.$children->getUserInfo(), E_USER_WARNING);
				return false;
			}
			if (count($children)) {
				trigger_error('Unable to add attribute "'.$name.'" to Asset "'.$type_code.'" because variable name is already used by it\'s children "'.implode('","', $children).'"' , E_USER_WARNING);
				return false;
			}

			$attr_info['owning_type_code']  = $type_code;
			$attr_info['type']              = $this->vars[$name]['type'];
			$attr_info['default_type_code'] = $type_code;
			$attr_info['default_value']     = $this->vars[$name]['default'];

		// else one of our parents has defined the an attribute of this type before
		// so we are really just overriding the default value for this asset type
		} else {

			// OK, what's happening is that the asset type that is represented by the min level
			// is the owner of the attribute, so from there we get all the attribute information
			// except for the default value, because this might have been overridden by an asset type
			// further down the line, this is where the max level come in. The Max Level is really our parent
			// asset type and from here we get the current default value and the what that default value
			// was set by (the default_type_code). From these values we can se-up the attribute correctly

			// Find the parents with an attribute of this name
			$sql = 'SELECT atr.type_code, atr.owning_type_code, atr.type, atr.default_type_code, atr.default_value
					FROM sq_asset_type_inherited i, sq_asset_attribute atr, sq_asset_type at
					WHERE i.type_code = '.$this->_db->quote($type_code).'
					  AND i.inherited_type_code = atr.type_code
					  AND atr.name = '.$this->_db->quote($name).'
					  AND atr.type_code = at.type_code
					  AND at.level = ';

			$min_sql = $sql.$this->_db->quote($parent_info['min']);
			$attr_owner = $this->_db->getRow($min_sql);
			if (DB::isError($attr_owner)) {
				trigger_error($attr_owner->getMessage().'<br/>'.$attr_owner->getUserInfo(), E_USER_WARNING);
				return false;
			}

			$attr_info['owning_type_code']  = $attr_owner['owning_type_code'];
			$attr_info['type']              = $attr_owner['type'];
			// if we have set our own default value, then we are overriding the default
			if (isset($this->vars[$name]['default'])) {
				$attr_info['default_type_code'] = $type_code;
				$attr_info['default_value']     = $this->vars[$name]['default'];

			// else we want to inherit the default value
			} else {

				$max_sql = $sql.$this->_db->quote($parent_info['max']);
				$attr_default = $this->_db->getRow($max_sql);
				if (DB::isError($attr_default)) {
					trigger_error($attr_default->getMessage().'<br/>'.$attr_default->getUserInfo(), E_USER_WARNING);
					return false;
				}

				$attr_info['default_type_code'] = $attr_default['default_type_code'];
				$attr_info['default_value']     = $attr_default['default_value'];

			}

		}// end if parents using this name

		trigger_error('Need to have check to make sure that the variable type is valid' , E_USER_NOTICE);

		$attributeid = $this->_db->nextId('sq_sequence_asset_attribute');
		if (DB::isError($attributeid)) {
			trigger_error($attributeid->getMessage().'<br/>'.$attributeid->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$attr_info['description'] = empty($this->vars[$name]['description']) ? '' : $this->vars[$name]['description'];

		// OK, all seems fine, let's add the bloody thing
		$sql = 'INSERT INTO sq_asset_attribute
				(attributeid, type_code, owning_type_code, name, type, default_type_code, default_value, description)
				VALUES
				('.$this->_db->quote($attributeid).',
				'.$this->_db->quote($type_code).',
				'.$this->_db->quote($attr_info['owning_type_code']).',
				'.$this->_db->quote($name).',
				'.$this->_db->quote($attr_info['type']).',
				'.$this->_db->quote($attr_info['default_type_code']).',
				'.$this->_db->quote($attr_info['default_value']).',
				'.$this->_db->quote($attr_info['description']).')';

#		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end _addAttribute()


	/**
	* Updates an attribute in this asset's set
	*
	* @param string	$name				the name of the attribute to update
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access private
	*/
	function _updateAttribute($name, $current_version)
	{

		$type_code = $this->getAssetType();

		trigger_error('Need to have check to make sure that the variable type is valid' , E_USER_NOTICE);

		// if they have defined an update fn for this, then we don't need to do anything
		if (!empty($this->vars[$name]['update_fn'])) {
			$fn = $this->vars[$name]['update_fn'];
			return $this->$fn($current_version);

		} else {
			// Find the parents with an attribute of this name
			$sql = 'SELECT attributeid, type_code, owning_type_code, name, type, parameters, default_type_code, default_value
					FROM sq_asset_attribute
					WHERE type_code = '.$this->_db->quote($type_code).'
					  AND name = '.$this->_db->quote($name);

#			pre_echo($sql);

			$curr_attr = $this->_db->getRow($sql);
			if (DB::isError($curr_attr)) {
				trigger_error($curr_attr->getMessage().'<br/>'.$curr_attr->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// if this asset type owns this var, we can change it how ever we want
			if ($curr_attr['owning_type_code'] == $type_code) {
				return $this->_updateOwnedAttribute($name);

			// if we have set the default value in the vars
			} else if (isset($this->vars[$name]['default'])) {
				return $this->_updateDefaultAttribute($name, $curr_attr);

			}// end if

			// otherwise there ain't anything for us to do
			return true;

		}// end if

	}//end _updateAttribute()


	/**
	* Updates an attribute that this asset owns (ie the first time it is defined is by this asset)
	*
	* @param string	$name	the name of the attribute to update
	*
	* @return boolean
	* @access private
	*/
	function _updateOwnedAttribute($name)
	{

		$type_code = $this->getAssetType();

		trigger_error('Need to have check to make sure that the variable type is valid' , E_USER_NOTICE);

		$description = empty($this->vars[$name]['description']) ? '' : $this->vars[$name]['description'];

		// update all the attributes that we own
		$sql = 'UPDATE sq_asset_attribute
				SET type        = '.$this->_db->quote($this->vars[$name]['type']).',
					description = '.$this->_db->quote($description).'
				WHERE owning_type_code = '.$this->_db->quote($type_code).'
				  AND name = '.$this->_db->quote($name);

#		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}


		// update all the attributes that are still using our default
		$sql = 'UPDATE sq_asset_attribute
				SET default_value = '.$this->_db->quote($this->vars[$name]['default']).'
				WHERE default_type_code = '.$this->_db->quote($type_code).'
				  AND name = '.$this->_db->quote($name);

#		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end _updateOwnedAttribute()


	/**
	* Updates an attribute that this asset doesn't own but has set the default value on
	*
	* @param string	$name		the name of the attribute to update
	* @param string	$curr_attr	the info for the attribute as it stands at the moment
	*
	* @return boolean
	* @access private
	*/
	function _updateDefaultAttribute($name, $curr_attr)
	{
		$type_code = $this->getAssetType();


		// OK, the reason that we are setting the default type code for the attributes
		// is so that if this is the first time that we are setting the default value
		// then all the asset types below us that were inheriting the default value from the
		// the same asset type as we used to be will be converted to using our default value
		$sql = 'UPDATE sq_asset_attribute
				SET default_type_code = '.$this->_db->quote($type_code).',
					default_value     = '.$this->_db->quote($this->vars[$name]['default']).'
				WHERE attributeid = '.$this->_db->quote($curr_attr['attributeid']).'
				   OR attributeid IN (~SQ0~)';

		$subs = Array( 'SELECT atr.attributeid
						FROM sq_asset_type_inherited i, sq_asset_attribute atr
						WHERE i.inherited_type_code = '.$this->_db->quote($type_code).'
						  AND i.type_code = atr.type_code
						  AND atr.default_type_code = '.$this->_db->quote($curr_attr['default_type_code']).'
						  AND atr.name = '.$this->_db->quote($name));

		$sql = db_extras_subquery($this->_db, $sql, $subs);
		if (DB::isError($sql)) {
			trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
			return false;
		}

#		pre_echo($sql);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end _updateDefaultAttribute()


	/**
	* Returns the information from the passed asset.xml
	* This function can be called without creating an instance of the class
	* ie $info = Asset_Management::getAssetInfo($file);
	*
	* @param string	$file	the asset.xml file path
	*
	* @return Array
	* @access public
	*/
	function getAssetInfo($file)
	{

		// OK, now because this can be called without the need to have an instance of the class
		// we need to store our tmp stuff in a global array
		if (!isset($GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'])) $GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'] = Array();

		if (!isset($GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file])) {

			$xml_array = new XML_Array(Asset_Management::getAssetArrayPaths());

			$info = $xml_array->getArrayFromFile($file);
			if (PEAR::isError($info)) {
				trigger_error($info->error_message_prefix.$info->message.'<br/>In File:'.$file, E_USER_WARNING);
				return Array();
			}

			$xml_array->close();

			$editing_options = Array();
			if (isset($info['editing_options']['option']) && is_array($info['editing_options']['option'])) {
				for($i = 0; $i < count($info['editing_options']['option']); $i++) {
					$opt = &$info['editing_options']['option'][$i];
					$editing_options[$opt['@action']] = (empty($opt['value'])) ? '' : $opt['value'];
				}// end for
			}// endif

			$GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file] = Array(
																	'type_code'       => strtolower($info['type_code']['value']),
																	'package'         => $info['package']['value'],
																	'name'            => $info['name']['value'],
																	'version'         => $info['version']['value'],
																	'description'     => $info['description']['value'],
																	'instantiable'    => ($info['instantiable']['value'] == 'yes') ? 1 : 0,
																	'system_only'     => ($info['system_only']['value']  == 'yes') ? 1 : 0,
																	'parent_type'     => $info['parent_type']['value'],
																	'editing_options' => $editing_options
																);

		}//end if

		return $GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file];

	}//end getAssetInfo()


	/**
	* Used by the XML_Array()
	*
	* @see XML_Array::XML_Array()
	*
	* @return Array
	* @access public
	*/
	function getAssetArrayPaths()
	{
		return Array('asset_info/requirements/requirement', 'asset_info/editing_options/option');
	}

}//end class
?>
