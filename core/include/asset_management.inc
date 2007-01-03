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
* $Id: asset_management.inc,v 1.78 2007/01/03 03:21:19 arailean Exp $
*
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
* @version $Revision: 1.78 $
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
	* Array(
	*   var_name => Array(
	*                 'added'        => '0.0.1',
	*                 'removed'      => '9.9.0',
	*                 'type'         => 'text|...|...',
	*                 'default'      => '',
	*                 ['uniq'        => true|false],
	*                 ['description' => 'description'],
	*                 ['update_fn'   => 'update_var_name'],
	*               )
	* )
	*
	* NOTE: should only ever be set in a sub-class
	*/
	var $vars = Array();

	/**
	* @var array
	* Holds all the searchable components for this assets.
	* Array (
	*   component_name => Array(
	*                       'description'       => 'description',
	*                       'default_weighting' => '1',
	*                     )
	* )
	*
	* NOTE: should only ever be set in a sub-class
	*/
	var $components = Array();


	/**
	* Constructor
	*
	*/
	function Asset_Management(&$pm)
	{

		if (get_parent_class($this) != __CLASS__) {
			trigger_localised_error('SYS0274', E_USER_ERROR, get_class($this));
		}


		$this->_pm = &$pm;
		$this->_am = &$GLOBALS['SQ_SYSTEM']->am;
		$this->_db = &$GLOBALS['SQ_SYSTEM']->db;

	}//end constructor


	/**
	* Returns the asset type that this management class is working for
	*
	* @return string
	* @access public
	*/
	function getAssetType()
	{
		$class = get_class($this);
		return substr($class, 0, strlen($class) - 11);

	}//end getAssetType()


	/**
	* This gets run to intall/upgrade the asset that this management class represents
	*
	* @return boolean
	* @access public
	*/
	function update()
	{
		$type_code = $this->getAssetType();

		// OK, first thing we are going to do is check that the parent type of this asset is the same between the
		// asset.xml, actual asset class and the edit fns class

		$asset_dir = SQ_SYSTEM_ROOT.'/'.$this->_pm->getTypeInfo($type_code,'dir');
		require_once $asset_dir.'/'.$type_code.'.inc';
		require_once $asset_dir.'/'.$type_code.'_edit_fns.inc';

		$asset_xml_parent_type = $this->_pm->getTypeInfo($type_code, 'parent_type');
		$asset_parent_type = get_parent_class($type_code);
		$asset_edit_fns_parent_type = preg_replace('/_edit_fns$/', '', get_parent_class($type_code.'_edit_fns'));

		if ($asset_xml_parent_type != $asset_parent_type || $asset_xml_parent_type != $asset_edit_fns_parent_type) {
			trigger_localised_error('SYS0254', E_USER_WARNING, $type_code, $type_code, $type_code);
			return false;
		}

		// if the parent asset type hasn't been installed, abort
		if ($asset_xml_parent_type != 'asset' && !$this->_am->installed($asset_xml_parent_type)) {
			trigger_localised_error('SYS0253', E_USER_WARNING, $type_code, $asset_xml_parent_type);
			return false;
		}

		// if the asset manager knows about it - it is installed, so upgrade...
		if ($this->_am->installed($type_code)) {
			if (!$this->_upgrade($this->_am->getTypeInfo($type_code, 'version'))) {
				return false;
			}
		} else {
			if (!$this->_install()) return false;
		}

		if (!$this->_updateFiles()) return false;

		// save a copy of an icon we can use to represent this asset
		// if no icon is directly available, the parents icon will be used
		$from_icon_path = SQ_SYSTEM_ROOT.'/'.$this->_pm->getTypeInfo($type_code,'dir').'/icon.png';
		$to_icon_path   = SQ_DATA_PATH.'/public/asset_types/'.$type_code.'/icon.png';
		if (!is_file($from_icon_path)) {
			// copy the icon from our parent
			$parent_type = $this->_pm->getTypeInfo($type_code, 'parent_type');
			if ($parent_type == 'asset') {
				// we are a top level asset so we need to copy our icon from the include dir
				$from_icon_path = SQ_INCLUDE_PATH.'/asset_edit/icon.png';
			} else {
				// our parent should already have gone through all this and have
				// an icon we can borrow
				$from_icon_path = SQ_DATA_PATH.'/public/asset_types/'.$parent_type.'/icon.png';
			}
		}

		if (!is_file($from_icon_path)) {
			trigger_localised_error('CORE0068', E_USER_WARNING, $type_code);
		} else {
			if (!copy_file($from_icon_path, $to_icon_path)) {
				return false;
			}
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
	* Installs this asset
	*
	* @return boolean
	* @access public
	*/
	function _install()
	{
		$type_code = $this->getAssetType();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// get how deep we are in the heirarchy
		$level = count($this->_pm->getTypeAncestors($type_code));
		$info = $this->_pm->getTypeInfo($type_code);

		$sql = 'INSERT INTO sq_ast_typ
				(type_code, version, name, description, instantiable, allowed_access, parent_type, lvl, dir, customisation)
				VALUES
				('.$this->_db->quote($type_code).', '.$this->_db->quote($info['version']).',
				'.$this->_db->quote($info['name']).', '.$this->_db->quote($info['description']).',
				'.$this->_db->quote($info['instantiable']).', '.$this->_db->quote($info['allowed_access']).',
				'.$this->_db->quote($info['parent_type']).', '.$this->_db->quote($level).',
				'.$this->_db->quote($info['dir']).', 0)';
		$result = $this->_db->query($sql);
		assert_valid_db_result($result);

		// now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($type_code);

		// now add the attributes
		$all_vars = $this->_getAllVars();
		for (reset($all_vars); null !== ($k = key($all_vars)); next($all_vars)) {
			$name =& $all_vars[$k];
			if (!$this->_addAttribute($name)) return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _install()


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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// get how deep we are in the heirarchy
		$level = count($this->_pm->getTypeAncestors($this->getAssetType()));
		$info = $this->_pm->getTypeInfo($this->getAssetType());

		$sql = 'UPDATE sq_ast_typ
				SET version         = '.$this->_db->quote($info['version']).',
					name            = '.$this->_db->quote($info['name']).',
					description     = '.$this->_db->quote($info['description']).',
					instantiable    = '.$this->_db->quote($info['instantiable']).',
					allowed_access  = '.$this->_db->quote($info['allowed_access']).',
					parent_type     = '.$this->_db->quote($info['parent_type']).',
					lvl             = '.$this->_db->quote($level).',
					dir             = '.$this->_db->quote($info['dir']).'
				WHERE type_code = '.$this->_db->quote($this->getAssetType());

		$result = $this->_db->query($sql);
		if (!assert_valid_db_result($result, '', false, false)) {
			return false;
		}

		// now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($this->getAssetType());

		// now update the attributes
		$sql = 'SELECT name
				FROM sq_ast_attr
				WHERE type_code = '.$this->_db->quote($this->getAssetType());
		$current_vars = $this->_db->getCol($sql);

		$all_vars = $this->_getAllVars();

		foreach ($all_vars as $name) {
			// if we already have this attribute installed, then let's update it
			if (in_array($name, $current_vars)) {
				if (!$this->_updateAttribute($name, $current_version)) {
					return false;
				}
			} else {
				if (!$this->_addAttribute($name)) return false;
			}
		}//end for

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _upgrade()


	/**
	* Returns all the variable names that are available for this asset type
	* - any inherited vars and those defined in this management class
	*
	* @return array(string)
	* @access public
	*/
	function _getAllVars()
	{
		$sql = 'SELECT DISTINCT atr.name
				FROM sq_ast_typ_inhd i, sq_ast_attr atr
				WHERE i.type_code = '.$this->_db->quote($this->getAssetType()).'
				  AND i.inhd_type_code <> '.$this->_db->quote($this->getAssetType()).'
				  AND i.inhd_type_code = atr.type_code';

		$parent_vars = $this->_db->getCol($sql);
		assert_valid_db_result($parent_vars);

		// array_values() to get proper indexed array
		$vars = array_values(array_unique(array_merge($parent_vars, array_keys($this->vars))));
		return $vars;

	}//end _getAllVars()


	/**
	* Returns all files this asset wants installed/updated into the data directory
	*
	* @return array(string)
	* @access private
	*/
	function _getFileList()
	{
		return Array();

	}//end _getFileList()


	/**
	* returns a list of events that this asset is listening for
	*
	* @access public
	* @return array
	*/
	function getEventList()
	{
		return Array();

	}//end getEventList()


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

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// See if any other our parents have an attribute with this name
		$sql = 'SELECT COUNT(*) as count, MIN(at.lvl) as min, MAX(at.lvl) as max
				FROM sq_ast_typ_inhd i, sq_ast_attr atr, sq_ast_typ at
				WHERE i.type_code = '.$this->_db->quote($type_code).'
				  AND i.inhd_type_code <> '.$this->_db->quote($type_code).'
				  AND i.inhd_type_code = atr.type_code
				  AND atr.name = '.$this->_db->quote($name).'
				  AND atr.type_code = at.type_code';

		$parent_info = $this->_db->getRow($sql);
		if (!assert_valid_db_result($parent_info, '', false, false)) {
			return false;
		}

		// if none of our parent types have an attribute with this name
		if (empty($parent_info['count'])) {

			// check children to see if any of them have an attribute with this name
			$sql = 'SELECT a.type_code, a.type, a.attrid
					FROM sq_ast_typ_inhd i, sq_ast_attr a
					WHERE i.inhd_type_code = '.$this->_db->quote($type_code).'
					  AND i.type_code <> '.$this->_db->quote($type_code).'
					  AND i.type_code = a.type_code
					  AND a.name = '.$this->_db->quote($name);
			$children = $this->_db->getAll($sql);

			if (!assert_valid_db_result($children, '', false, false)) {
				return false;
			}

			// if  children have this attribute, we'll try to change the owner type code to us
			if (!empty($children)) {
				echo '*** Attribute "'.$name.'" is being moved from child types:'."\n";
				foreach ($children as $child) {
					// if attribute types don't match - cannot upgrade - fail
					if ($child['type'] != $this->vars[$name]['type']) {
						trigger_localised_error('SYS0233', E_USER_WARNING, $name, $type_code, implode('","', $children));
						return false;
					}

					echo '* '.$child['type_code']."\n";
					$temp_attr_ids[] = $this->_db->quoteSmart($child['attrid']);
				}

				// now, let's mark the attribute as being owned by this asset type
				$sql = 'UPDATE sq_ast_attr
						SET owning_type_code = '.$this->_db->quoteSmart($type_code).'
						WHERE attrid IN ('.implode(',',$temp_attr_ids).')';

				$result = $this->_db->query($sql);
				assert_valid_db_result($result);

				echo '*** Done'."\n\n";

			}

			$attr_info['owning_type_code']       = $type_code;
			$attr_info['type']                   = $this->vars[$name]['type'];
			$attr_info['default_type_code']      = $type_code;
			$attr_info['default_val']            = $this->vars[$name]['default'];
			$attr_info['uniq']                   = empty($this->vars[$name]['uniq']) ? 0 : 1;
			$attr_info['is_admin']               = isset($this->vars[$name]['is_admin']) ? (int) $this->vars[$name]['is_admin'] : 1;
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
			$sql = 'SELECT	atr.type_code, atr.owning_type_code, atr.type, atr.uniq, atr.parameters_val,
							atr.parameters_type_code, atr.default_type_code, atr.default_val, atr.description, atr.is_admin
					FROM sq_ast_typ_inhd i, sq_ast_attr atr, sq_ast_typ at
					WHERE i.type_code = '.$this->_db->quote($type_code).'
					  AND i.inhd_type_code <> '.$this->_db->quote($type_code).'
					  AND i.inhd_type_code = atr.type_code
					  AND atr.name = '.$this->_db->quote($name).'
					  AND atr.type_code = at.type_code
					  AND at.lvl = ';

			$min_sql = $sql.$this->_db->quote($parent_info['min']);
			$attr_owner = $this->_db->getRow($min_sql);
			if (!assert_valid_db_result($attr_owner, '', false, false)) {
				return false;
			}

			$attr_info['owning_type_code']  = $attr_owner['owning_type_code'];
			$attr_info['type']              = $attr_owner['type'];
			$attr_info['uniq']              = $attr_owner['uniq'];
			$attr_info['is_admin']          = (int) $attr_owner['is_admin'];
			$attr_info['description']       = $attr_owner['description'];

			// if we have set our own default value, then we are overriding the default
			if (isset($this->vars[$name]['default'])) {
				$attr_info['default_type_code'] = $type_code;
				$attr_info['default_val']     = $this->vars[$name]['default'];
			} else {
				// else we want to inherit the default value
				$max_sql = $sql.$this->_db->quote($parent_info['max']);
				$attr_default = $this->_db->getRow($max_sql);
				if (!assert_valid_db_result($attr_default, '', false, false)) {
					return false;
				}

				$attr_info['default_type_code'] = $attr_default['default_type_code'];
				$attr_info['default_val']     = $attr_default['default_val'];
			}

			// if we have set our own parameters value, then we are overriding the default
			if (isset($this->vars[$name]['parameters'])) {
				$attr_info['parameters_type_code'] = $type_code;
				$attr_info['parameters']           = $this->vars[$name]['parameters'];
			} else {
				// else we want to inherit the default value
				$max_sql = $sql.$this->_db->quote($parent_info['max']);
				$attr_default = $this->_db->getRow($max_sql);
				if (!assert_valid_db_result($attr_default, '', false, false)) {
					return false;
				}

				$attr_info['parameters_type_code'] = $attr_default['parameters_type_code'];
				$attr_info['parameters']           = unserialize($attr_default['parameters_val']);
			}

		}//end if parents using this name

		// make sure this is a valid attribute type
		if (!$this->_am->validAttributeType($attr_info['type'])) {
			trigger_localised_error('SYS0096', E_USER_WARNING, $attr_info['type'], $this->getAssetType());
			return false;
		}//end if

		if (!$this->_validateAttributeDefault($attr_info['default_val'], $attr_info['type'], $attr_info['parameters'])) {
			trigger_localised_error('SYS0130', E_USER_WARNING, $name, $this->getAssetType());
			return false;
		}

		$attributeid = $this->_db->nextId('sq_ast_attr');
		if (!assert_valid_db_result($attributeid, '', false, false)) {
			return false;
		}

		// OK, all seems fine, let's add the bloody thing
		$sql = 'INSERT INTO sq_ast_attr
				(attrid, type_code, owning_type_code, name, type, uniq, parameters_type_code, default_type_code, default_val, parameters_val, is_admin, description)
				VALUES
				('.$this->_db->quote($attributeid).',
				'.$this->_db->quote($type_code).',
				'.$this->_db->quote($attr_info['owning_type_code']).',
				'.$this->_db->quote($name).',
				'.$this->_db->quote($attr_info['type']).',
				'.$this->_db->quote($attr_info['uniq']).',
				'.$this->_db->quote($attr_info['parameters_type_code']).',
				'.$this->_db->quote($attr_info['default_type_code']).',
				'.$this->_db->quote($attr_info['default_val']).',
				'.$this->_db->quote(serialize($attr_info['parameters'])).',
				'.$this->_db->quote($attr_info['is_admin']).',
				'.$this->_db->quote($attr_info['description']).')';

		$result = $this->_db->query($sql);
		if (!assert_valid_db_result($result, '', false, false)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _addAttribute()


	/**
	* Removes an attribute from the set of attributes owned by this asset
	*
	* This asset type must be the attribute's owning type.  If we have the
	* attribute via inheritance or whatever, deletion is not allowed.
	*
	* @param string $name				the attribute to delete
	*
	* @return boolean					whether the delete was successful
	* @access private
	*/
	function _deleteAttribute($name)
	{
		$type_code = $this->getAssetType();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// find the attributeid
		$sql = 'SELECT attrid FROM sq_ast_attr WHERE owning_type_code = '.$this->_db->quote($type_code).' AND name='.$this->_db->quote($name);
		$attributeid = $this->_db->getOne($sql);

		assert_valid_db_result($attributeid);
		if (is_null($attributeid)) {
			trigger_localised_error('SYS0050', E_USER_WARNING, $typecode, $name);
			return false;
		}

		// delete the attribute values
		$sql = 'DELETE FROM sq_ast_attr_val WHERE attrid='.$this->_db->quote($attributeid);
		$result = $this->_db->query($sql);
		assert_valid_db_result($result);

		// delete the attribute
		$sql = 'DELETE FROM sq_ast_attr WHERE attrid='.$this->_db->quote($attributeid);
		$result = $this->_db->query($sql);
		assert_valid_db_result($result);

		// get rid of it in our array
		if (isset($this->vars[$name])) {
			unset($this->vars[$name]);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _deleteAttribute()


	/**
	* Updates an attribute in this asset's set
	*
	* @param string	$name				the name of the attribute to update
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access protected
	*/
	function _updateAttribute($name, $current_version)
	{
		$type_code = $this->getAssetType();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// if they have defined an update fn for this, then we don't need to do anything
		if (!empty($this->vars[$name]['update_fn'])) {
			$fn = $this->vars[$name]['update_fn'];
			if (!$this->$fn($current_version)) return false;

		} else {
			// load this attribute info
			$sql = 'SELECT attrid, type_code, owning_type_code, name, type, uniq, parameters_val, parameters_type_code, default_type_code, default_val
					FROM sq_ast_attr
					WHERE type_code = '.$this->_db->quoteSmart($type_code).'
					  AND name = '.$this->_db->quoteSmart($name);

			$curr_attr = $this->_db->getRow($sql);
			if (!assert_valid_db_result($curr_attr, '', false, false)) {
				return false;
			}

			// determine the parent of the asset being upgraded.
			if (!isset($this->_tmp['parent_asset_type'])) {
				$parents = $this->_pm->getTypeAncestors($type_code);
				$this->_tmp['parent_asset_type'] = array_shift($parents);
			}
			$parent = $this->_tmp['parent_asset_type'];

			// see if there is a parent type with the same name
			$sql = 'SELECT attrid, owning_type_code
					FROM sq_ast_attr
					WHERE type_code = '.$this->_db->quoteSmart($parent).'
					  AND name = '.$this->_db->quoteSmart($name);

			$parent_check_data = $this->_db->getRow($sql);
			if (!assert_valid_db_result($parent_check_data, '', false, false)) {
				return false;
			}

			// fix owning type code if:
			// - there is a parent with this attribute and our owning_type_code is not the same as theirs
			// - there is NO parent with this attribute and our owning_type_code is not the same as OUR type_code
			$owning_type_code = null;
			if (!empty($parent_check_data) && ($parent_check_data['owning_type_code'] != $curr_attr['owning_type_code'])) {
				$owning_type_code = $parent_check_data['owning_type_code'];
			} else if (empty($parent_check_data) && ($curr_attr['owning_type_code'] != $type_code)) {
				$owning_type_code = $type_code;
			}

			if (!empty($owning_type_code)) {
				echo '* Fixing invalid owner definition of attribute "'.$name.'"'."\n";

				$sql = 'UPDATE sq_ast_attr
						SET owning_type_code = '.$this->_db->quoteSmart($owning_type_code).'
						WHERE type_code = '.$this->_db->quoteSmart($type_code).' AND name = '.$this->_db->quoteSmart($name);

				$result = $this->_db->query($sql);
				if (!assert_valid_db_result($result, '', false, false)) {
					return false;
				}

				$curr_attr['owning_type_code'] = $owning_type_code;
			}
			// end fixing owning type code

			// if this asset type owns this var, we can change it how ever we want
			if ($curr_attr['owning_type_code'] == $type_code) {
				if (!$this->_updateOwnedAttribute($name, $curr_attr)) {
					return false;
				}

			// if we have set the params or default value in the vars
			} else {

				// we have removed our customisation of parameters
				// so get our parent's parameters and set to that
				$sql = 'SELECT owning_type_code, parameters_type_code, parameters_val, default_type_code, default_val
						FROM sq_ast_attr
						WHERE type_code = '.$this->_db->quoteSmart($parent).'
						  AND name = '.$this->_db->quoteSmart($name);

				$parent_attr = $this->_db->getRow($sql);
				if (!assert_valid_db_result($parent_attr, '', false, false)) {
					return false;
				}


				//// PARAMETERS ////

				// if this asset type has overidden, use it's values
				if (isset($this->vars[$name]['parameters'])) {
					$parameters_value     = serialize($this->vars[$name]['parameters']);
					$parameters_type_code = $type_code;

				// otherwise reset to inherit from parents
				} else if (!empty($parent_attr)) {
					$parameters_value     = $parent_attr['parameters_val'];
					$parameters_type_code = $parent_attr['parameters_type_code'];

				// no parent ? set to blank for ourselves
				} else {
					$parameters_value     = serialize(Array());
					$parameters_type_code = $type_code;
				}

				if ($parameters_type_code != $curr_attr['parameters_type_code'] || $parameters_value != $curr_attr['parameters_val']) {
					if (!$this->_updateCustomAttribute($curr_attr['attrid'], $name, 'parameters', $curr_attr['parameters_type_code'], $parameters_type_code, $parameters_value)) {
						return false;
					}
				}


				//// DEFAULT VALUES ////

				// if this asset type has overidden, use it's values
				if (isset($this->vars[$name]['default'])) {
					$default_value		= $this->vars[$name]['default'];
					$default_type_code	= $type_code;

					$parameters = unserialize($parameters_value);
					if (!$this->_validateAttributeDefault($default_value, $curr_attr['type'], $parameters)) {
						trigger_localised_error('SYS0130', E_USER_WARNING, $name, $this->getAssetType());
						return false;
					}

				// otherwise reset to inherit from parents
				} else if (!empty($parent_attr)) {
					$default_value		= $parent_attr['default_val'];
					$default_type_code	= $parent_attr['default_type_code'];

				// no parent ? set to blank for ourselves
				} else {
					$default_value		= '';
					$default_type_code	= $type_code;
				}

				if ($default_type_code != $curr_attr['default_type_code'] || $default_value != $curr_attr['default_val']) {
					if (!$this->_updateCustomAttribute($curr_attr['attrid'], $name, 'default', $curr_attr['default_type_code'], $default_type_code, $default_value)) {
						return false;
					}
				}

			} // end if

		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$type_code = $this->getAssetType();

		// make sure this is a valid attribute type
		if (!$this->_am->validAttributeType($this->vars[$name]['type'])) {
			trigger_localised_error('SYS0096', E_USER_WARNING, $this->vars[$name]['type'], $this->getAssetType());
			return false;
		}//end if

		$description = empty($this->vars[$name]['description']) ? '' : $this->vars[$name]['description'];

		// update all the attributes that we own
		$sql = 'UPDATE sq_ast_attr
				SET type        = '.$this->_db->quote($this->vars[$name]['type']).',
					uniq        = '.$this->_db->quote(empty($this->vars[$name]['uniq']) ? 0 : 1).',
					is_admin    = '.$this->_db->quote(isset($this->vars[$name]['is_admin']) ? (int) $this->vars[$name]['is_admin'] : 1).',
					description = '.$this->_db->quote($description).'
				WHERE owning_type_code = '.$this->_db->quote($type_code).'
				  AND name = '.$this->_db->quote($name);

		$result = $this->_db->query($sql);
		if (!assert_valid_db_result($result, '', false, false)) {
			return false;
		}

		if (!$this->_validateAttributeDefault($this->vars[$name]['default'], $this->vars[$name]['type'], (empty($this->vars[$name]['parameters']) ? Array() : $this->vars[$name]['parameters']))) {
			trigger_localised_error('SYS0130', E_USER_WARNING, $name, $this->getAssetType());
			return false;
		}

		// update all the attributes that are still using our default
		$sql = 'UPDATE sq_ast_attr
				SET default_val = '.$this->_db->quote($this->vars[$name]['default']).'
				WHERE default_type_code = '.$this->_db->quote($type_code).'
				  AND name = '.$this->_db->quote($name);

		$result = $this->_db->query($sql);
		if (!assert_valid_db_result($result, '', false, false)) {
			return false;
		}

		if (!empty($this->vars[$name]['parameters'])) {
			// update all the attributes that are still using our parameters
			$sql = 'UPDATE sq_ast_attr
					SET parameters_val = '.$this->_db->quote(serialize($this->vars[$name]['parameters'])).'
					WHERE parameters_type_code = '.$this->_db->quote($type_code).'
					  AND name = '.$this->_db->quote($name);

			$result = $this->_db->query($sql);
			if (!assert_valid_db_result($result, '', false, false)) {
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// OK, the reason that we are setting the default type code for the attributes
		// is so that if this is the first time that we are setting the default value
		// then all the asset types below us that were inheriting the default value from the
		// the same asset type as we used to be will be converted to using our default value
		$sql = 'UPDATE sq_ast_attr
				SET '.$field.'_type_code = '.$this->_db->quote($new_type_code).',
					'.$field.'_val     = '.$this->_db->quote($new_value).'
				WHERE attrid = '.$this->_db->quote($attributeid).'
				   OR attrid IN (
								SELECT atr.attrid
								FROM sq_ast_typ_inhd i, sq_ast_attr atr
								WHERE i.inhd_type_code = '.$this->_db->quote($new_type_code).'
								  AND i.type_code <> '.$this->_db->quote($new_type_code).'
								  AND i.type_code = atr.type_code
								  AND atr.'.$field.'_type_code = '.$this->_db->quote($old_type_code).'
								  AND atr.name = '.$this->_db->quote($name).'
								  )';

		$result = $this->_db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _updateCustomAttribute()


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
		require_once SQ_ATTRIBUTES_PATH.'/'.$attr_type.'/'.$attr_type.'.inc';
		$class = 'Asset_Attribute_'.$attr_type;
		$attr =& new $class();
		foreach ($parameters as $k => $v) {
			$attr->_params[$k] = $v;
		}
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
		if (!isset($GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'])) {
			$GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'] = Array();
		}

		if (!isset($GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file])) {

			$xml_array =& new XML_Array(Asset_Management::getAssetArrayPaths());

			$info = $xml_array->getArrayFromFile($file);
			if (PEAR::isError($info)) {
				trigger_error($info->error_message_prefix.$info->message.'<br/>In File:'.$file, E_USER_WARNING); //ast-val-db fix
				return Array();
			}

			$xml_array->close();

			$GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file] = Array(
																	'type_code'			=> strtolower($info['type_code']['value']),
																	'package'			=> $info['package']['value'],
																	'name'				=> $info['name']['value'],
																	'version'			=> $info['version']['value'],
																	'description'		=> $info['description']['value'],
																	'instantiable'		=> ($info['instantiable']['value'] == 'yes') ? 1 : 0,
																	'allowed_access'	=> $info['allowed_access']['value'],
																	'parent_type'		=> $info['parent_type']['value'],
																	'flash_menu_path'	=> (isset($info['flash_menu_path']['value'])) ? $info['flash_menu_path']['value'] : '',
																);

		}//end if

		return $GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file];

	}//end getAssetInfo()


	/**
	* Used by the XML_Array()
	*
	* @see XML_Array::XML_Array()
	*
	* @return array
	* @access public
	*/
	function getAssetArrayPaths()
	{
		return Array('asset_info/requirements/requirement');

	}//end getAssetArrayPaths()


}//end class

?>
