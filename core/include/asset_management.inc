<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: asset_management.inc,v 1.90 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

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
* @version $Revision: 1.90 $
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

		if ((get_class($this) !== __CLASS__) && (get_parent_class($this) !== __CLASS__)) {
			trigger_localised_error('SYS0274', sprintf(translate('Unable to Construct "%s" - all Asset Management classes MUST inherit directly from the Asset_Management class'), get_class($this)), E_USER_ERROR);
		}


		$this->_pm = $pm;
		$this->_am = $GLOBALS['SQ_SYSTEM']->am;
		$this->_db = MatrixDAL::getDb();

	}//end constructor


	/**
	* Returns the asset type that this management class is working for
	*
	* Note that as of PHP 5.x, return values from get_class() are in mixed case.
	* This makes sure asset types are returned lower case, as in other parts of
	* Matrix.
	*
	* @return string
	* @access public
	*/
	function getAssetType()
	{
		$class = get_class($this);
		return strtolower(substr($class, 0, strlen($class) - 11));

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
		$asset_parent_type = strtolower(get_parent_class($type_code));
		$asset_edit_fns_parent_type = preg_replace('/_edit_fns$/', '', strtolower(get_parent_class($type_code.'_edit_fns')));

		if (($asset_xml_parent_type != $asset_parent_type) || ($asset_xml_parent_type != $asset_edit_fns_parent_type)) {
			trigger_localised_error('SYS0254', sprintf(translate('Unable to update "%1$s". Parent is different between asset.xml, %2$s.inc and %3$s_edit_fns.inc'), $type_code, $type_code, $type_code), E_USER_WARNING);
			return false;
		}

		// if the parent asset type hasn't been installed, abort
		if ($asset_xml_parent_type != 'asset' && !$this->_am->installed($asset_xml_parent_type)) {
			trigger_localised_error('SYS0253', sprintf(translate('Unable to update "%1$s". Parent "%2$s" is not installed'), $type_code, $asset_xml_parent_type), E_USER_WARNING);
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

		$this->_installDALQueries();

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
			trigger_localised_error('CORE0068', sprintf(translate('No icon file found for asset type "%s" or its ancestors.'), $type_code), E_USER_WARNING);
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

		$db = MatrixDAL::getDb();

		$sql = 'INSERT INTO sq_ast_typ
				(type_code, version, name, description, instantiable, allowed_access, parent_type, lvl, dir, customisation)
				VALUES
				(:type_code, :version, :name, :description, :instantiable, :allowed_access, :parent_type, :lvl, :dir, :customisation)';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code',      $type_code,              PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'version',        $info['version'],        PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'name',           $info['name'],           PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'description',    $info['description'],    PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'instantiable',   $info['instantiable'],   PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'allowed_access', $info['allowed_access'], PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'parent_type',    $info['parent_type'],    PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'lvl',            $level,                  PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'dir',            $info['dir'],            PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'customisation',  0,                       PDO::PARAM_INT);
			$result = MatrixDAL::execPdoQuery($query);
		} catch (DALException $e) {
			throw new Exception('Unable to add information for new asset type "'.$type_code.'" due to database error: '.$e->getMessage());
		}

		// now inform the asset manager that this asset type is available for use
		$this->_am->refreshAssetType($type_code);

		// now add the attributes
		$all_vars = $this->_getAllVars();

		// In case a package is being re-installed, be mindful of existing attributes.
		$existing_attributes = $this->_getExistingAttributes($type_code);

		for (reset($all_vars); null !== ($k = key($all_vars)); next($all_vars)) {
			$name = $all_vars[$k];
			if (in_array($name, $existing_attributes)) {
				continue;
			}

			if (!$this->_addAttribute($name)) {
				return false;
			}
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

		$db = MatrixDAL::getDb();
		$asset_type = $this->getAssetType();

		// get how deep we are in the heirarchy
		$level = count($this->_pm->getTypeAncestors($asset_type));
		$info = $this->_pm->getTypeInfo($asset_type);

		$sql = 'UPDATE sq_ast_typ
				SET
					version         = :version,
					name            = :name,
					description     = :description,
					instantiable    = :instantiable,
					allowed_access  = :allowed_access,
					parent_type     = :parent_type,
					lvl             = :lvl,
					dir             = :dir
				WHERE
					type_code = :type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'version',        $info['version'],        PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'name',           $info['name'],           PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'description',    $info['description'],    PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'instantiable',   $info['instantiable'],   PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'allowed_access', $info['allowed_access'], PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'parent_type',    $info['parent_type'],    PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'lvl',            $level,                  PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'dir',            $info['dir'],            PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'type_code',      $asset_type,             PDO::PARAM_STR);
			DAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update type details while upgrading asset type "'.$asset_type.'" to version '.$current_version.', due to database error: ', $e->getMessage());
		}

		// now inform the asset manager that this asset is available for use
		$this->_am->refreshAssetType($asset_type);

		// now update the attributes
		$sql = 'SELECT name
				FROM sq_ast_attr
				WHERE type_code = :type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $asset_type);
			$current_vars = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get current attributes while upgrading asset type "'.$asset_type.'" to version '.$current_version.', due to database error: ', $e->getMessage());
		}

		$all_vars = $this->_getAllVars();

		foreach ($all_vars as $name) {
			// if we already have this attribute installed, then let's update it
			if (in_array($name, $current_vars)) {
				if (!$this->_updateAttribute($name, $current_version)) {
					return false;
				}
			} else {
				if (!$this->_addAttribute($name)) return FALSE;
			}
		}//end for

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _upgrade()


	/**
	 * Get a list of existing attributes for a particular type code.
	 * Only returns the name of existing attributes (if any).
	 *
	 * @param string $type_code The type code to get attributes for.
	 *
	 * @return array
	 */
	function _getExistingAttributes($type_code)
	{
		$db = MatrixDAL::getDb();

		$type_code = $this->getAssetType();
		$sql = 'SELECT atr.name
				FROM sq_ast_attr atr
				  WHERE atr.type_code = :type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
			$existing_attributes = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get attribute list for type code "'.$type_code.'" due to database error: '.$e->getMessage());
		}

		return $existing_attributes;

	}//end _getExistingAttributes()


	/**
	* Returns all the variable names that are available for this asset type
	* - any inherited vars and those defined in this management class
	*
	* @return array(string)
	* @access public
	*/
	function _getAllVars()
	{
		$db = MatrixDAL::getDb();

		$type_code = $this->getAssetType();
		$sql = 'SELECT DISTINCT atr.name
				FROM sq_ast_typ_inhd i, sq_ast_attr atr
				WHERE i.type_code = :type_code_1
				  AND i.inhd_type_code <> :type_code_2
				  AND i.inhd_type_code = atr.type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code_1', $type_code, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'type_code_2', $type_code, PDO::PARAM_STR);
			$parent_vars = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get attribute list for asset type "'.$asset_type.'" due to database error: '.$e->getMessage());
		}

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
		$db = MatrixDAL::getDb();

		// See if any other our parents have an attribute with this name
		$sql = 'SELECT COUNT(*) as count, MIN(at.lvl) as min, MAX(at.lvl) as max
				FROM sq_ast_typ_inhd i, sq_ast_attr atr, sq_ast_typ at
				WHERE i.type_code = :type_code
				  AND i.inhd_type_code <> :inhd_type_code
				  AND i.inhd_type_code = atr.type_code
				  AND atr.name = :name
				  AND atr.type_code = at.type_code';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code',      $type_code, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $type_code, PDO::PARAM_STR);	// not needed for baked DAL?
			MatrixDAL::bindValueToPdo($query, 'name',           $name,      PDO::PARAM_STR);
			$result = MatrixDAL::executePdoAll($query);
			$parent_info = $result[0];
			unset($result);
		} catch (Exception $e) {
			echo 'Unable to search for matching parent attribute while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage();
			throw new Exception('Unable to search for matching parent attribute while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
		}

		// if none of our parent types have an attribute with this name
		if (empty($parent_info['count'])) {

			// check children to see if any of them have an attribute with this name
			$sql = 'SELECT a.type_code, a.type, a.attrid
					FROM sq_ast_typ_inhd i, sq_ast_attr a
					WHERE i.inhd_type_code = :type_code
					  AND i.type_code <> :inhd_type_code
					  AND i.type_code = a.type_code
					  AND a.name = :name';
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code',      $type_code, PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $type_code, PDO::PARAM_STR);	// not needed for baked DAL?
				MatrixDAL::bindValueToPdo($query, 'name',           $name,      PDO::PARAM_STR);
				$children = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Unable to search for matching child attribute while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
			}

			// if  children have this attribute, we'll try to change the owner type code to us
			if (!empty($children)) {
				echo '*** Attribute "'.$name.'" is being moved from child types:'."\n";
				foreach ($children as $child) {
					// if attribute types don't match - cannot upgrade - fail
					if ($child['type'] != $this->vars[$name]['type']) {
						trigger_localised_error('SYS0233', sprintf(translate('Unable to add attribute "%1$s" to Asset "%2$s" because variable name is already used by it\'s children "%3$s"', $name, $type_code, implode('","'), $children)), E_USER_WARNING);
						return false;
					}

					echo '* '.$child['type_code']."\n";
					$temp_attr_ids[] = MatrixDAL::quote($child['attrid']);
				}
				$temp_attr_ids = implode(', ', $temp_attr_ids);

				// now, let's mark the attribute as being owned by this asset type
				$sql = 'UPDATE sq_ast_attr
						SET owning_type_code = :type_code
						WHERE attrid IN ('.$temp_attr_ids.')';
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
					DAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update attribute\'s owning type while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
				}

				echo '*** Done'."\n\n";

			}

			$attr_info['owning_type_code']       = $type_code;
			$attr_info['type']                   = $this->vars[$name]['type'];
			$attr_info['default_type_code']      = $type_code;
			$attr_info['default_val']            = $this->vars[$name]['default'];
			$attr_info['uniq']                   = empty($this->vars[$name]['uniq']) ? 0 : 1;
			$attr_info['is_admin']               = isset($this->vars[$name]['is_admin']) ? (int) $this->vars[$name]['is_admin'] : 1;
			$attr_info['is_contextable']         = isset($this->vars[$name]['is_contextable']) ? (int) $this->vars[$name]['is_contextable'] : 0;
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
							atr.parameters_type_code, atr.default_type_code, atr.default_val, atr.description, atr.is_admin, atr.is_contextable
					FROM sq_ast_typ_inhd i, sq_ast_attr atr, sq_ast_typ at
					WHERE i.type_code = :type_code
					  AND i.inhd_type_code <> :inhd_type_code
					  AND i.inhd_type_code = atr.type_code
					  AND atr.name = :name
					  AND atr.type_code = at.type_code
					  AND at.lvl = ';

			$min_sql = $sql.':min';

			try {
				$query = MatrixDAL::preparePdoQuery($min_sql);
				MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $type_code, PDO::PARAM_STR);	// not needed for baked DAL?
				MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'min',       $parent_info['min'], PDO::PARAM_INT);
				$result = MatrixDAL::executePdoAll($query);
				$attr_owner = $result[0];
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to search for matching parent attribute while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
			}

			$attr_info['owning_type_code']  = $attr_owner['owning_type_code'];
			$attr_info['type']              = $attr_owner['type'];
			$attr_info['uniq']              = $attr_owner['uniq'];
			$attr_info['is_admin']          = (int) $attr_owner['is_admin'];
			$attr_info['is_contextable']    = (int) $attr_owner['is_contextable'];
			$attr_info['description']       = $attr_owner['description'];

			// if we have set our own default value, then we are overriding the default
			if (isset($this->vars[$name]['default'])) {
				$attr_info['default_type_code'] = $type_code;
				$attr_info['default_val']     = $this->vars[$name]['default'];
			} else {
				// else we want to inherit the default value
				$max_sql = $sql.':max';
				try {
					$query = MatrixDAL::preparePdoQuery($max_sql);
					MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $type_code, PDO::PARAM_STR);	// not needed for baked DAL?
					MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'max',       $parent_info['max'], PDO::PARAM_INT);
					$result = MatrixDAL::executePdoAll($query);
					$attr_default = $result[0];
					unset($result);
				} catch (Exception $e) {
					throw new Exception('Unable to search for matching parent attribute while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
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
				$max_sql = $sql.':max';
				try {
					$query = MatrixDAL::preparePdoQuery($max_sql);
					MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'inhd_type_code', $type_code, PDO::PARAM_STR);	// not needed for baked DAL?
					MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'max',       $parent_info['max'], PDO::PARAM_INT);
					$result = MatrixDAL::executePdoAll($query);
					$attr_default = $result[0];
					unset($result);
				} catch (Exception $e) {
					throw new Exception('Unable to search for matching parent attribute while adding attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
				}

				$attr_info['parameters_type_code'] = $attr_default['parameters_type_code'];
				$attr_info['parameters']           = unserialize($attr_default['parameters_val']);
			}

		}//end if parents using this name

		// make sure this is a valid attribute type
		if (!$this->_am->validAttributeType($attr_info['type'])) {
			trigger_localised_error('SYS0096', sprintf(translate('Attribute Type "%1$s" is not known (for the "%2$s" Asset Type) \''), $attr_info['type'], $this->getAssetType()), E_USER_WARNING);
			return false;
		}//end if

		if (!$this->_validateAttributeDefault($attr_info['default_val'], $attr_info['type'], $attr_info['parameters'])) {
			trigger_localised_error('SYS0130', sprintf(translate('Default value of attribute "%1$s" is not valid for Asset Type "%2$s'), $name, $this->getAssetType()), E_USER_WARNING);
			return false;
		}

		// OK, all seems fine, let's add the attribute
		try {
				$bind_vars = Array(
								'type_code'				=> $type_code,
								'owning_type_code'		=> $attr_info['owning_type_code'],
								'name'					=> $name,
								'type'					=> $attr_info['type'],
								'uniq' 					=> $attr_info['uniq'],
								'parameters_type_code'	=> $attr_info['parameters_type_code'],
								'default_type_code'		=> $attr_info['default_type_code'],
								'default_val'			=> $attr_info['default_val'],
								'parameters_val'		=> serialize($attr_info['parameters']),
								'is_admin'				=> $attr_info['is_admin'] ? 1 : 0,
								'is_contextable'		=> $attr_info['is_contextable'] ? 1 : 0,
								'description'			=> $attr_info['description'],
							 );
				MatrixDAL::executeQuery('core', 'insertAssetAttribute', $bind_vars);
		} catch (DALException $e) {
			throw new Exception('Unable to add attribute "'.$name.'" to asset type "'.$type_code.'", due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

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
		$sql = 'SELECT attrid FROM sq_ast_attr WHERE owning_type_code = :type_code AND name = :name';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'type_code', $type_code);
		MatrixDAL::bindValueToPdo($query, 'name',      $name);
		$attributeid = MatrixDAL::executePdoOne($query);

		if (is_null($attributeid)) {
			trigger_localised_error('SYS0050', sprintf(translate('Asset type %1$s has no attribute called %2$s'), $typecode, $name), E_USER_WARNING);
			return false;
		}

		// delete the attribute values
		$sql = 'DELETE FROM sq_ast_attr_val WHERE attrid = :attrid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'attrid', $attributeid);
		MatrixDAL::execPdoQuery($query);

		// delete the attribute
		$sql = 'DELETE FROM sq_ast_attr WHERE attrid = :attrid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'attrid', $attributeid);
		MatrixDAL::execPdoQuery($query);

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
			$sql = 'SELECT
						attrid, type_code, owning_type_code, name, type, uniq, parameters_val, parameters_type_code, default_type_code, default_val
					FROM
						sq_ast_attr
					WHERE
						type_code = :type_code
						AND name = :name';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
				$result = MatrixDAL::executePdoAll($query);
				$curr_attr = $result[0];
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to load attribute info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
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
					WHERE type_code = :type_code
					AND name = :name';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
				$result = MatrixDAL::executePdoAll($query);
				$parent_check_data = $result[0];
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to load parent type attribute info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
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
						SET owning_type_code = :owning_type_code
						WHERE type_code = :type_code AND name = :name';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'owning_type_code', $owning_type_code, PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
					DAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update owning type code info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
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
						WHERE type_code = :type_code
						  AND name = :name';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'type_code', $type_code, PDO::PARAM_STR);
					MatrixDAL::bindValueToPdo($query, 'name',      $name,      PDO::PARAM_STR);
					$result = MatrixDAL::executePdoAll($query);
					$parent_attr = $result[0];
					unset($result);
				} catch (Exception $e) {
					throw new Exception('Unable to load parent type attribute info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
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
						trigger_localised_error('SYS0130', sprintf(translate('Default value of attribute "%1$s" is not valid for Asset Type "%2$s'), $name, $this->getAssetType()), E_USER_WARNING);
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
		$db = MatrixDAL::getDb();

		$type_code = $this->getAssetType();

		// make sure this is a valid attribute type
		if (!$this->_am->validAttributeType($this->vars[$name]['type'])) {
			trigger_localised_error('SYS0096', sprintf(translate('Attribute Type "%1$s" is not known (for the "%2$s" Asset Type) \''), $this->vars[$name]['type'], $this->getAssetType()), E_USER_WARNING);
			return false;
		}//end if

		$description = empty($this->vars[$name]['description']) ? '' : $this->vars[$name]['description'];

		// update all the attributes that we own
		$sql = 'UPDATE sq_ast_attr
				SET type        = :type,
					uniq        = :uniq,
					is_admin    = :is_admin,
					is_contextable = :is_contextable,
					description = :description
				WHERE owning_type_code = :owning_type_code
				  AND name = :name';


		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type',             $this->vars[$name]['type'], PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'uniq',             empty($this->vars[$name]['uniq']) ? 0 : 1, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'is_admin',         isset($this->vars[$name]['is_admin']) ? (int) $this->vars[$name]['is_admin'] : 1, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'is_contextable',   isset($this->vars[$name]['is_contextable']) ? (int) $this->vars[$name]['is_contextable'] : 0, PDO::PARAM_INT);
			MatrixDAL::bindValueToPdo($query, 'description',      $description, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'owning_type_code', $type_code,   PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'name',             $name,        PDO::PARAM_STR);
			DAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update owned attribute info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
		}

		if (!$this->_validateAttributeDefault($this->vars[$name]['default'], $this->vars[$name]['type'], (empty($this->vars[$name]['parameters']) ? Array() : $this->vars[$name]['parameters']))) {
			trigger_localised_error('SYS0130', sprintf(translate('Default value of attribute "%1$s" is not valid for Asset Type "%2$s'), $name, $this->getAssetType()), E_USER_WARNING);
			return false;
		}

		// update all the attributes that are still using our default
		$sql = 'UPDATE sq_ast_attr
				SET default_val = :val
				WHERE default_type_code = :type_code
				  AND name = :name';


		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'val',         $this->vars[$name]['default'], PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code,   PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'name',             $name,        PDO::PARAM_STR);
			DAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update attribute defaults for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
		}

		if (!empty($this->vars[$name]['parameters'])) {
			// update all the attributes that are still using our parameters
			$sql = 'UPDATE sq_ast_attr
					SET parameters_val = :val
					WHERE parameters_type_code = :type_code
					  AND name = :name';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'val',         serialize($this->vars[$name]['parameters']), PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'type_code', $type_code,   PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'name',             $name,        PDO::PARAM_STR);
				DAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update attribute parameter info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
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
		$db = MatrixDAL::getDb();

		// OK, the reason that we are setting the default type code for the attributes
		// is so that if this is the first time that we are setting the default value
		// then all the asset types below us that were inheriting the default value from the
		// the same asset type as we used to be will be converted to using our default value
		$sql = 'UPDATE sq_ast_attr
				SET '.$field.'_type_code = :new_type_code,
					'.$field.'_val     = :value
				WHERE attrid = :attrid
				   OR attrid IN (
								SELECT atr.attrid
								FROM sq_ast_typ_inhd i, sq_ast_attr atr
								WHERE i.inhd_type_code = :new_type_code_1
								  AND i.type_code <> :new_type_code_2
								  AND i.type_code = atr.type_code
								  AND atr.'.$field.'_type_code = :old_type_code
								  AND atr.name = :name
								  )';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'new_type_code',   $new_type_code, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'new_type_code_1',   $new_type_code, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'new_type_code_2',   $new_type_code, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'old_type_code', $old_type_code,   PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'name',             $name,        PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'value',             $new_value,        PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'attrid',             $attributeid,        PDO::PARAM_STR);
			DAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update custom attribute value info for attribute "'.$name.'" of asset type "'.$type_code.'", due to database error: '.$e->getMessage());
		}

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
		$attr = new $class();
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

			try {
				$info = simplexml_load_string(file_get_contents($file));
			} catch (Exception $e) {
				throw new Exception('Could not parse asset definition file "'.$file.'": '.$e->getMessage());
			}

			$GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file] = Array(
																	'type_code'			=> strtolower((string)$info->type_code),
																	'package'			=> (string)$info->package,
																	'name'				=> (string)$info->name,
																	'version'			=> (string)$info->version,
																	'description'		=> (string)$info->description,
																	'instantiable'		=> ((string)$info->instantiable === 'yes') ? 1 : 0,
																	'allowed_access'	=> (string)$info->allowed_access,
																	'parent_type'		=> (string)$info->parent_type,
																	'flash_menu_path'	=> (isset($info->flash_menu_path)) ? (string)$info->flash_menu_path : '',
																);

		}//end if

		return $GLOBALS['SQ_ASSET_MANAGEMENT_ASSET_INFO'][$file];

	}//end getAssetInfo()


	/**
	* Generates the baked Database Abstraction Layer (DAL) queries for the asset
	*
	* @return boolean
	* @access public
	*/
	function _installDALQueries()
	{
		$name = $this->getAssetType();
		$old_path = ini_get('include_path');
		ini_set('include_path', SQ_LIB_PATH);
		require_once SQ_LIB_PATH.'/MatrixDAL/MatrixDALBaker.inc';
		$queries_found = MatrixDALBaker::addAssetTypeQueries($name);
		if ($queries_found) {
			MatrixDALBaker::bakeQueriesFile($name);
		}
		ini_set('include_path', $old_path);

	}//end _installDALQueries()


}//end class

?>
