<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_management.inc,v 1.44 2003/10/21 08:03:14 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


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
* @package MySource_Matrix
*/
class Asset_Management extends MySource_Object
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
	*						'added'        => '0.0.1',
	*						'removed'      => '9.9.0',
	*						'type'         => 'text|...|...',
	*						'default'      => '',
	*						['uniq'        => true|false,]
	*						['description' => 'description',]
	*						['update_fn'   => 'update_var_name',]
	*						)
	*		)
	*/
	var $vars;


	/**
	* @var array
	* Holds all the searchable components for this assets. 
	* Array (component_name => Array(
	*				'description'		=> 'description',
	*				'default_weighting' => '1'
	*			)
	* )
	*/
	var $components;

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
		$this->components = Array(
			'name' => Array (
				'description'		=> 'Name',
				'default_weighting'	=> '-1',
			), 	
		);

	}//end constructor


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
			if (!$this->_upgrade($this->_am->getTypeInfo($this->getAssetType(), 'version'))) return false;
		} else {
			if (!$this->_install()) return false;
		}
		if (!$this->_updateFiles()) return false;

		// save a copy of an icon we can use to represent this asset
		// if no icon is directly available, the parents icon will be used
		$from_icon_path = SQ_SYSTEM_ROOT.'/'.$this->_pm->getTypeInfo($this->getAssetType(),'dir').'/icon.png';
		$to_icon_path   = SQ_DATA_PATH.'/public/asset_types/'.$this->getAssetType().'/icon.png';
		if (!is_file($from_icon_path)) {
			// copy the icon from our parent
			$parent_type = $this->_pm->getTypeInfo($this->getAssetType(), 'parent_type');
			if ($parent_type == 'asset') {
				// we are a top level asset so we need to copy our icon from the include dir
				$from_icon_path = SQ_INCLUDE_PATH.'/asset_edit/icon.png';
			} else {
				// our parent should already have gone through all this and have
				// an icon we can... 'borrow' <evil_laugh> mwahhaha </evil_laugh>
				$from_icon_path = SQ_DATA_PATH.'/public/asset_types/'.$parent_type.'/icon.png';
			}
		}

		if (!is_file($from_icon_path)) {
			trigger_error('No icon file found for asset type "'.$this->getAssetType().'" or its ancestors.', E_USER_WARNING);
		} else {
			if (!copy_file($from_icon_path, $to_icon_path)) return false;
		}

		return true;

	}//end update()


	/**
	* Copy files used by the asset to the data directory
	*
	* @return boolean
	* @access private
	*/
	function _updateFiles()
	{
		// get a list of files to copy
		$file_list = $this->_getFileList();
		if (empty($file_list)) return true;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$to_dir_path = SQ_DATA_PATH.'/public/asset_types/'.$this->getAssetType();
		$from_dir_path = SQ_SYSTEM_ROOT.'/'.$this->_pm->getTypeInfo($this->getAssetType(),'dir');

		// create the directory to store these files if it doesnt exist
		// NOTE: The create_direcotyr function of file_system.inc handles this for us
		create_directory($to_dir_path);

		// for each file, copy it over
		for (reset($file_list); null !== ($k = key($file_list)); next($file_list)) {
			$file = $file_list[$k];
			$from = $from_dir_path.'/'.$file;
			$to = $to_dir_path.'/'.$file;

			// the copy_file function of file_system.inc will create the
			// directory strucutre before copying
			if (!copy_file($from, $to)) return false;
		}

		return true;

	}//end _updateFiles()


	/**
	* Install's this asset
	*
	* @return boolean
	* @access public
	*/
	function _install()
	{
		$type_code = $this->getAssetType();

		// get how deep we are in the heirarchy
		$level = count($this->_pm->getTypeAncestors($type_code));
		$info = $this->_pm->getTypeInfo($type_code);

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'asset_type
				(type_code, version, name, description, instantiable, allowed_access, parent_type, level, dir, customisation)
				VALUES
				('.$this->_db->quote($type_code).', '.$this->_db->quote($info['version']).',
				'.$this->_db->quote($info['name']).', '.$this->_db->quote($info['description']).',
				'.$this->_db->quote($info['instantiable']).', '.$this->_db->quote($info['allowed_access']).',
				'.$this->_db->quote($info['parent_type']).', '.$this->_db->quote($level).',
				'.$this->_db->quote($info['dir']).', 0)';
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
			$name =& $all_vars[$k];
			if (!$this->_addAttribute($name)) return false;
		}
		
		// now add the component weightings
		$this->_setComponentWeightings($this->components);

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
		$level = count($this->_pm->getTypeAncestors($this->getAssetType()));
		$info = $this->_pm->getTypeInfo($this->getAssetType());

		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'asset_type
				SET version         = '.$this->_db->quote($info['version']).',
					name            = '.$this->_db->quote($info['name']).',
					description     = '.$this->_db->quote($info['description']).',
					instantiable    = '.$this->_db->quote($info['instantiable']).',
					allowed_access  = '.$this->_db->quote($info['allowed_access']).',
					parent_type     = '.$this->_db->quote($info['parent_type']).',
					level           = '.$this->_db->quote($level).',
					dir             = '.$this->_db->quote($info['dir']).'
				WHERE type_code = '.$this->_db->quote($this->getAssetType());

		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($this->getAssetType());

		// now update the attributes
		$sql = 'SELECT name
				FROM '.SQ_TABLE_PREFIX.'asset_attribute
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

		// now add the component weightings
		$this->_setComponentWeightings($this->components);

		return true;

	}//end upgrade()


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
				FROM '.SQ_TABLE_PREFIX.'asset_type_inherited i, '.SQ_TABLE_PREFIX.'asset_attribute atr
				WHERE i.type_code = '.$this->_db->quote($this->getAssetType()).'
				  AND i.inherited_type_code <> '.$this->_db->quote($this->getAssetType()).'
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
	function _getFileList()
	{
		return Array();

	}//end _getFileList()


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
				FROM '.SQ_TABLE_PREFIX.'asset_type_inherited i, '.SQ_TABLE_PREFIX.'asset_attribute atr, '.SQ_TABLE_PREFIX.'asset_type at
				WHERE i.type_code = '.$this->_db->quote($type_code).'
				  AND i.inherited_type_code <> '.$this->_db->quote($type_code).'
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
					FROM '.SQ_TABLE_PREFIX.'asset_type_inherited i, '.SQ_TABLE_PREFIX.'asset_attribute a
					WHERE i.inherited_type_code = '.$this->_db->quote($type_code).'
					  AND i.type_code <> '.$this->_db->quote($type_code).'
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

			$attr_info['owning_type_code']       = $type_code;
			$attr_info['type']                   = $this->vars[$name]['type'];
			$attr_info['default_type_code']      = $type_code;
			$attr_info['default_value']          = $this->vars[$name]['default'];
			$attr_info['uniq']                   = empty($this->vars[$name]['uniq']) ? 0 : 1;
			$attr_info['parameters_type_code']   = $type_code;

			if (isset($this->vars[$name]['parameters'])) {
				$attr_info['parameters'] = $this->vars[$name]['parameters'];
			} else {
				$attr_info['parameters'] = Array();
			}

			$attr_info['description'] = empty($this->vars[$name]['description']) ? '' : $this->vars[$name]['description'];


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
			$sql = 'SELECT	atr.type_code, atr.owning_type_code, atr.type, atr.uniq, atr.parameters_value,
							atr.parameters_type_code, atr.default_type_code, atr.default_value, atr.description
					FROM '.SQ_TABLE_PREFIX.'asset_type_inherited i, '.SQ_TABLE_PREFIX.'asset_attribute atr, '.SQ_TABLE_PREFIX.'asset_type at
					WHERE i.type_code = '.$this->_db->quote($type_code).'
					  AND i.inherited_type_code <> '.$this->_db->quote($type_code).'
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
			$attr_info['uniq']              = $attr_owner['uniq'];
			$attr_info['description']       = $attr_owner['description'];

			// if we have set our own default value, then we are overriding the default
			if (isset($this->vars[$name]['default'])) {
				$attr_info['default_type_code'] = $type_code;
				$attr_info['default_value']     = $this->vars[$name]['default'];
			} else {
				// else we want to inherit the default value
				$max_sql = $sql.$this->_db->quote($parent_info['max']);
				$attr_default = $this->_db->getRow($max_sql);
				if (DB::isError($attr_default)) {
					trigger_error($attr_default->getMessage().'<br/>'.$attr_default->getUserInfo(), E_USER_WARNING);
					return false;
				}

				$attr_info['default_type_code'] = $attr_default['default_type_code'];
				$attr_info['default_value']     = $attr_default['default_value'];
			}

			// if we have set our own parameters value, then we are overriding the default
			if (isset($this->vars[$name]['parameters'])) {
				$attr_info['parameters_type_code'] = $type_code;
				$attr_info['parameters']           = $this->vars[$name]['parameters'];
			} else {
				// else we want to inherit the default value
				$max_sql = $sql.$this->_db->quote($parent_info['max']);
				$attr_default = $this->_db->getRow($max_sql);
				if (DB::isError($attr_default)) {
					trigger_error($attr_default->getMessage().'<br/>'.$attr_default->getUserInfo(), E_USER_WARNING);
					return false;
				}

				$attr_info['parameters_type_code'] = $attr_default['parameters_type_code'];
				$attr_info['parameters']           = unserialize($attr_default['parameters_value']);
			}

		}// end if parents using this name

		// make sure this is a valid attribute type
		if (!$this->_am->validAttributeType($attr_info['type'])) {
			trigger_error('Attribute Type "'.$attr_info['type'].'" is not known (for the "'.$this->getAssetType().'" Asset Type) ' , E_USER_WARNING);
			return false;
		}// end if

		if (!$this->_validateAttributeDefault($attr_info['default_value'], $attr_info['type'], $attr_info['parameters'])) {
			trigger_error('Default value of attribute "'.$name.'" is not valid for Asset Type "'.$this->getAssetType().'"' , E_USER_WARNING);
			return false;
		}

		$attributeid = $this->_db->nextId('sq_sequence_asset_attribute');
		if (DB::isError($attributeid)) {
			trigger_error($attributeid->getMessage().'<br/>'.$attributeid->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// OK, all seems fine, let's add the bloody thing
		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'asset_attribute
				(attributeid, type_code, owning_type_code, name, type, uniq, parameters_type_code, default_type_code, default_value, parameters_value, description)
				VALUES
				('.$this->_db->quote($attributeid).',
				'.$this->_db->quote($type_code).',
				'.$this->_db->quote($attr_info['owning_type_code']).',
				'.$this->_db->quote($name).',
				'.$this->_db->quote($attr_info['type']).',
				'.$this->_db->quote($attr_info['uniq']).',
				'.$this->_db->quote($attr_info['parameters_type_code']).',
				'.$this->_db->quote($attr_info['default_type_code']).',
				'.$this->_db->quote($attr_info['default_value']).',
				'.$this->_db->quote(serialize($attr_info['parameters'])).',
				'.$this->_db->quote($attr_info['description']).')';

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

		// if they have defined an update fn for this, then we don't need to do anything
		if (!empty($this->vars[$name]['update_fn'])) {
			$fn = $this->vars[$name]['update_fn'];
			if (!$this->$fn($current_version)) return false;

		} else {
			// Find the parents with an attribute of this name
			$sql = 'SELECT attributeid, type_code, owning_type_code, name, type, uniq, parameters_value, parameters_type_code, default_type_code, default_value
					FROM '.SQ_TABLE_PREFIX.'asset_attribute
					WHERE type_code = '.$this->_db->quote($type_code).'
					  AND name = '.$this->_db->quote($name);

			$curr_attr = $this->_db->getRow($sql);
			if (DB::isError($curr_attr)) {
				trigger_error($curr_attr->getMessage().'<br/>'.$curr_attr->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// if this asset type owns this var, we can change it how ever we want
			if ($curr_attr['owning_type_code'] == $type_code) {
				if (!$this->_updateOwnedAttribute($name, $curr_attr)) return false;

			// if we have set the params or default value in the vars
			} else {

				// we have removed our customisation of parameters
				// so get our parents parameters and set to that
				$parents = $this->_pm->getTypeAncestors($type_code);
				$parent = array_shift($parents);

				$sql = 'SELECT parameters_type_code, parameters_value, default_type_code, default_value
						FROM '.SQ_TABLE_PREFIX.'asset_attribute
						WHERE type_code = '.$this->_db->quote($parent).'
						  AND name = '.$this->_db->quote($name);

				$parent_attr = $this->_db->getRow($sql);
				if (DB::isError($parent_attr)) {
					trigger_error($parent_attr->getMessage().'<br/>'.$parent_attr->getUserInfo(), E_USER_WARNING);
					return false;
				}

				//// PARAMETERS ////

				// if this asset type has overidden, use it's values
				if (isset($this->vars[$name]['parameters'])) {
					$parameters_value		= serialize($this->vars[$name]['parameters']);
					$parameters_type_code	= $type_code;

				// otherwise reset to inherit from parents
				} else if (!empty($parent_attr)) {
					$parameters_value		= $parent_attr['parameters_value'];
					$parameters_type_code	= $parent_attr['parameters_type_code'];

				// no parent ? set to blank for ourselves
				} else {
					$parameters_value		= serialize(Array());
					$parameters_type_code	= $type_code;
				}

				if ($parameters_type_code != $curr_attr['parameters_type_code'] || $parameters_value != $curr_attr['parameters_value']) {
					if (!$this->_updateCustomAttribute($curr_attr['attributeid'], $name, 'parameters', $curr_attr['parameters_type_code'], $parameters_type_code, $parameters_value)) return false;
				}


				//// DEFAULT VALUES ////

				// if this asset type has overidden, use it's values
				if (isset($this->vars[$name]['default'])) {
					$default_value		= $this->vars[$name]['default'];
					$default_type_code	= $type_code;

					$parameters = unserialize($parameters_value);
					if (!$this->_validateAttributeDefault($default_value, $curr_attr['type'], $parameters)) {
						trigger_error('Default value of attribute "'.$name.'" is not valid for Asset Type "'.$this->getAssetType().'"' , E_USER_WARNING);
						return false;
					}

				// otherwise reset to inherit from parents
				} else if (!empty($parent_attr)) {
					$default_value		= $parent_attr['default_value'];
					$default_type_code	= $parent_attr['default_type_code'];

				// no parent ? set to blank for ourselves
				} else {
					$default_value		= '';
					$default_type_code	= $type_code;
				}

				if ($default_type_code != $curr_attr['default_type_code'] || $default_value != $curr_attr['default_value']) {
					if (!$this->_updateCustomAttribute($curr_attr['attributeid'], $name, 'default', $curr_attr['default_type_code'], $default_type_code, $default_value)) return false;
				}

			} // end if

		}// end if

		return true;

	}//end _updateAttribute()


	/**
	* Updates an attribute that this asset owns (ie the first time it is defined is by this asset)
	*
	* @param string	$name	the name of the attribute to update
	* @param string	$curr_attr	the info for the attribute as it stands at the moment
	*
	* @return boolean
	* @access private
	*/
	function _updateOwnedAttribute($name, $curr_attr)
	{
		$type_code = $this->getAssetType();

		// make sure this is a valid attribute type
		if (!$this->_am->validAttributeType($this->vars[$name]['type'])) {
			trigger_error('Attribute Type "'.$this->vars[$name]['type'].'" is not known (for the "'.$this->getAssetType().'" Asset Type) ' , E_USER_WARNING);
			return false;
		}// end if

		$description = empty($this->vars[$name]['description']) ? '' : $this->vars[$name]['description'];

		// update all the attributes that we own
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'asset_attribute
				SET type        = '.$this->_db->quote($this->vars[$name]['type']).',
					uniq        = '.$this->_db->quote(empty($this->vars[$name]['uniq']) ? 0 : 1).',
					description = '.$this->_db->quote($description).'
				WHERE owning_type_code = '.$this->_db->quote($type_code).'
				  AND name = '.$this->_db->quote($name);

		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (!$this->_validateAttributeDefault($this->vars[$name]['default'], $this->vars[$name]['type'], (empty($this->vars[$name]['parameters']) ? Array() : $this->vars[$name]['parameters']))) {
			trigger_error('Default value of attribute "'.$name.'" is not valid for Asset Type "'.$this->getAssetType().'"' , E_USER_WARNING);
			return false;
		}

		// update all the attributes that are still using our default
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'asset_attribute
				SET default_value = '.$this->_db->quote($this->vars[$name]['default']).'
				WHERE default_type_code = '.$this->_db->quote($type_code).'
				  AND name = '.$this->_db->quote($name);

		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (!empty($this->vars[$name]['parameters'])) {
			// update all the attributes that are still using our parameters
			$sql = 'UPDATE '.SQ_TABLE_PREFIX.'asset_attribute
					SET parameters_value = '.$this->_db->quote(serialize($this->vars[$name]['parameters'])).'
					WHERE parameters_type_code = '.$this->_db->quote($type_code).'
					  AND name = '.$this->_db->quote($name);

			$result = $this->_db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
		}

		return true;

	}//end _updateOwnedAttribute()


	/**
	* Updates an attribute that this asset doesn't own but has set the default value on
	*
	* @param string	$name		the name of the attribute to update
	* @param string	$curr_attr	the info for the attribute as it stands at the moment
	* @param string	$field		the part of the attribute we are updating (ie default | param)
	*
	* @return boolean
	* @access private
	*/
	function _updateCustomAttribute($attributeid, $name, $field, $old_type_code, $new_type_code, $new_value)
	{
		// OK, the reason that we are setting the default type code for the attributes
		// is so that if this is the first time that we are setting the default value
		// then all the asset types below us that were inheriting the default value from the
		// the same asset type as we used to be will be converted to using our default value
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'asset_attribute
				SET '.$field.'_type_code = '.$this->_db->quote($new_type_code).',
					'.$field.'_value     = '.$this->_db->quote($new_value).'
				WHERE attributeid = '.$this->_db->quote($attributeid).'
				   OR attributeid IN (~SQ0~)';

		$subs = Array( 'SELECT atr.attributeid
						FROM '.SQ_TABLE_PREFIX.'asset_type_inherited i, '.SQ_TABLE_PREFIX.'asset_attribute atr
						WHERE i.inherited_type_code = '.$this->_db->quote($new_type_code).'
						  AND i.type_code <> '.$this->_db->quote($new_type_code).'
						  AND i.type_code = atr.type_code
						  AND atr.'.$field.'_type_code = '.$this->_db->quote($old_type_code).'
						  AND atr.name = '.$this->_db->quote($name));

		$sql = db_extras_subquery($this->_db, $sql, $subs);
		if (DB::isError($sql)) {
			trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end _updateCustomAttribute()


	/**
	*  Sets the default weightings for this asset type's components
	* 
	* @param array The component weightings.
	* 
	* @access private
	* @return void
	*/
	function _setComponentWeightings($component_weightings)
	{
		$type_code = $this->getAssetType();
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// clear all weightings for this type_code
		$sql = 'DELETE FROM '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default 
				WHERE type_code = ' . $db->quote($type_code) ;

		$db->query($sql);
		foreach ($component_weightings as $component => $weighting) {
			if (strlen($component) == 0) continue;

			$sql = 'INSERT INTO '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
					(type_code, component, description, weighting)
					VALUES (' . 
						$db->quote($type_code)					. ', ' .
						$db->quote($component)					. ', ' .
						$db->quote($weighting['description'])	. ', ' .
						$db->quote($weighting['default_weighting']) . 
					')';
			$db->query($sql);

		}
	} // end _setComponentWeighting()

	/**
	* Checks an attributes default value to make sure that it is valid
	* Accepts the $default by reference so that it can be changed if need be
	*
	*
	* @param mixed	$default		the proposed default value
	* @param string	$attr_type		the type of asset represented by the id
	* @param array	$parameters		the params for this attribute
	*
	* @return boolean
	* @access private
	*/
	function _validateAttributeDefault(&$default, $attr_type, $parameters)
	{
		// check that the default value is valid
		require_once(SQ_ATTRIBUTES_PATH.'/'.$attr_type.'/'.$attr_type.'.inc');
		$class = 'Asset_Attribute_'.$attr_type;
		$attr = new $class();
		$attr->_params = $parameters;
		return $attr->validateValue($default);

	}//end _validateAttributeDefault()


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

			$GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file] = Array(
																	'type_code'       => strtolower($info['type_code']['value']),
																	'package'         => $info['package']['value'],
																	'name'            => $info['name']['value'],
																	'version'         => $info['version']['value'],
																	'description'     => $info['description']['value'],
																	'instantiable'    => ($info['instantiable']['value'] == 'yes') ? 1 : 0,
																	'allowed_access'  => $info['allowed_access']['value'],
																	'parent_type'     => $info['parent_type']['value'],
																	'flash_menu_path' => (isset($info['flash_menu_path']['value'])) ? $info['flash_menu_path']['value'] : '',
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
		return Array('asset_info/requirements/requirement');

	}//end getAssetArrayPaths()


}//end class

?>