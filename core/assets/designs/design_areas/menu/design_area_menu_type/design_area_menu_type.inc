<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_menu_type.inc,v 1.30 2003/09/26 05:26:25 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Menu_Type
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Type extends Design_Area
{

	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array('design_area_menu_type' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name	the name of the attribute
	* @param string		$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		// if they are trying to set a page position dependent setting
		if (substr($name, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			// where setting type is either 'normal', 'heirarchy' or 'current'
			$split = explode('.', $name);
			if (count($split) != 3) {
				trigger_error('Setting "'.$name.'" unable to be set, three elements in name not found', E_USER_WARNING);
				return false;
			}
			switch($split[2]) {
				case 'normal'    :
				case 'hierarchy' :
				case 'current'   :
					$new_value = $this->vars['settings']['value'];
					if(!is_array($new_value)) $new_value = Array();
					if(!isset($new_value[$split[1]])) $new_value[$split[1]] = Array();
					$new_value[$split[1]][$split[2]] = $value;
					$name  = 'settings';
					$value = $new_value;
					break;

				default :
					trigger_error('Settings Type "'.$split[2].'" not known', E_USER_WARNING);
					return false;
			}// end switch

		// if they are trying to set a level, so some validation
		} else if ($name == 'level') {
			switch(strtolower($value)) {
				case 'top';
					$value = 0;
					break;
				case 'sub';
					$value = 1;
					break;
				default :
					$value = max(0, (int) $value);
			}// end switch

		}// end if


		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name
	*
	* @return mixed
	* @access public
	*/
	function attr($name)
	{
		// if they are trying to set a page position dependent setting
		if (substr($name, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			$split = explode('.', $name);
			if (count($split) == 3) {
				$settings = $this->attr('settings'); // do this so that any var references will be taken into account
				if (isset($settings[$split[1]][$split[2]])) {
					return $settings[$split[1]][$split[2]];
				}
			}// end if
		}// end if

		// if we hit this, just send it upwards
		return parent::attr($name);

	}//end attr()


	/**
	* Returns an array of Array(id_name => Array(var)) for all var references that are needed by this design area
	*
	* @access public
	*/
	function getVarReferences()
	{
		$var_refs = parent::getVarReferences();
		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_var_refs = $sub_menu->getVarReferences();
			for(reset($sub_var_refs); null !== ($id_name = key($sub_var_refs)); next($sub_var_refs)) {
				if (!isset($var_refs[$id_name])) $var_refs[$id_name] = Array();
				foreach($sub_var_refs[$id_name] as $var) {
					if (in_array($var, $var_refs[$id_name])) continue;
					$var_refs[$id_name][] = $var;
				}// end foreach
			}// end foreach
		}// end if

		return $var_refs;

	}//end getVarReferences()


	/**
	* Set's the values for the var references values (registered in $this->var_references)
	*
	* @param array $var_ref_values	array of values - Array(id_name => Array(var => [value]))
	*
	* @access public
	*/
	function setVarReferenceValues($var_ref_values)
	{
		$settings = $this->attr('settings');
		$settings_changed = false;
		foreach($this->vars['var_references']['value'] as $var_name => $info) {
			// if this is a setting and it is available in the var_ref_values
			if (substr($var_name, 0, 9) == 'settings.' && isset($var_ref_values[$info['id_name']][$info['var']])) {
				// The name is in the form 'settings.[var_name].[setting type]'
				$split = explode('.', $var_name);
				if (count($split) == 3) {
					if (!isset($settings[$split[1]])) $settings[$split[1]] = Array();
					$settings[$split[1]][$split[2]] = $var_ref_values[$info['id_name']][$info['var']];
					$settings_changed = true;
				}// end if
			}// end if
		}// end foreach

		if ($settings_changed) {
			$this->_tmp['var_reference_values']['settings'] = $settings;
		}

		// send our custom array to the parent fn
		parent::setVarReferenceValues($var_ref_values);
		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			// now send the original array to the sub
			$sub_menu->setVarReferenceValues($var_ref_values);
		}// end if

	}//end setVarReferenceValues()


	/**
	* Returns the link to the sub menu for this menu (if any)
	*
	* @return Array()
	* @access public
	*/
	function getSubMenuLink()
	{
		if (!isset($this->_tmp['sub_menu_link'])) {
			$this->_tmp['sub_menu_link'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'design_area_menu_type', false, 'sub_menu');
		}
		return $this->_tmp['sub_menu_link'];

	}//end getSubMenuLink()


	/**
	* Returns the sub menu for this menu (if any)
	*
	* @return mixed Design_Area_Menu_Type or null
	* @access public
	*/
	function &getSubMenu()
	{
		$link = $this->getSubMenuLink();
		if (empty($link)) {
			$null = null;
			return $null;
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code'], true);
		}

	}//end getSubMenu()


	/**
	* Returns the parent menu for this menu (assuming it is a sub menu)
	*
	* @return mixed Design_Area_Menu_Type or null
	* @access public
	*/
	function &getParentMenu()
	{
		if (!isset($this->_tmp['parent_menu_link'])) {
			$this->_tmp['parent_menu_link'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'design_area_menu_type', false, 'sub_menu', 'minor');
		}
		if (empty($this->_tmp['parent_menu_link'])) {
			$null = null;
			return $null;
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['parent_menu_link']['majorid'], $this->_tmp['parent_menu_link']['major_type_code'], true);
		}

	}//end getParentMenu()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
	*
	* @return boolean
	* @access private
	*/
	function _linksUpdated()
	{
		if (!parent::_linksUpdated()) return false;
		unset($this->_tmp['sub_menu_link']);
		unset($this->_tmp['parent_menu_link']);
		return true;

	}//end _linksUpdated()


	/**
	* Returns whether the passed setting exists or not
	*
	* @return boolean
	* @access public
	*/
	function settingExists($name)
	{
		$settings = $this->attr('settings'); // do like this to take into account var refs
		return !empty($settings[$name]);

	}//end settingExists()


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		echo '<', '?php
		';
		$this->printStaticInitOutput($design);

		$parent_menu = &$this->getParentMenu();
		// if we aren't a sub menu
		if (is_null($parent_menu)) {
			$this->printInitOutput($design);
			echo '
		if (!empty($PAGE_LINEAGE) && !empty($PAGE_LINEAGE[$'.$this->attr('id_name').'_level])) {
			';
			$this->printDataCollection($design);
			$parent_assetid_name = '$PAGE_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']';
		} else {
			$parent_assetid_name = '$'.$parent_menu->attr('id_name').'_row[\'assetid\']';
		}// end if

		echo '
			$'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), true).';
			if (!empty($GLOBALS[\'MENU_DATA\']['.$parent_assetid_name.'])) {
		?', '>';
		if (!parent::printArea($design)) return false;
		echo '<', '?php
			}// end if subs '.$this->attr('id_name').'
		';
		// if we aren't a sub menu
		if (is_null($parent_menu)) {
			echo '
		}// end if found in PAGE_LINEAGE
			';
		}// end if

		echo '
		?', '>';

		return true;

	}//end printArea()


	/**
	* Performs any outputting needed to by all design areas of this menu type
	* NOTE: assumes PHP tags are open
	*
	* @param object Design	$design	the design for which we are currently printing
	*
	* @access public
	*/
	function printStaticInitOutput(&$design)
	{
		// Stuff that is only needed once per design file
		if (empty($GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$design->id])) {
			echo '
			$db = &$GLOBALS[\'SQ_SYSTEM\']->db;
			$PAGE_LINEAGE = $am->getLineageFromURL(null, null, \'short_name\');

			require_once SQ_FUDGE_PATH.\'/db_extras/db_extras.inc\';

			$GLOBALS[\'MENU_DATA\'] = Array();

			function menu_get_assets($majorids)
			{
				$assetids = Array();
				$majorids_str = \'\';
				foreach($majorids as $id) {
					if (empty($GLOBALS[\'MENU_DATA\'][$id])) {
						$majorids_str .= $id.\',\';
					} else {
						foreach($GLOBALS[\'MENU_DATA\'][$id] as $row) {
							$assetids[] = (int) $row[\'assetid\'];
						}
					}
				}
				
				if (empty($majorids_str)) return $assetids;

				static $USERIDS_COND = null;

				$db = &$GLOBALS[\'SQ_SYSTEM\']->db;

				if (is_null($USERIDS_COND)) {
					// if they are logged in add their parents to the list
					if ($GLOBALS[\'SQ_SYSTEM\']->currentUserId()) {
						if ($GLOBALS[\'SQ_SYSTEM\']->userRoot() || $GLOBALS[\'SQ_SYSTEM\']->userSystemAdmin()) {
							$USERIDS_COND = \'\';
						} else {
							$userids = $GLOBALS[\'SQ_SYSTEM\']->am->getParents($GLOBALS[\'SQ_SYSTEM\']->user->id, \'user_group\', false);
							array_push($userids, 0, (int) $GLOBALS[\'SQ_SYSTEM\']->user->id);
							$USERIDS_COND = \'AND p.userid  IN (\'.implode(\',\', $userids).\')\';
						}
					} else {
						// public users
						$USERIDS_COND = \'AND p.userid = 0\';
					}

					if (!empty($USERIDS_COND)) {
						$USERIDS_COND .= \'
										  GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path
										  HAVING MIN(p.access) = 1\';
					}
				}

				$sql    = \'SELECT DISTINCT a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order
						   FROM \'.SQ_TABLE_RUNNING_PREFIX.\'asset a
						     INNER JOIN \'.SQ_TABLE_RUNNING_PREFIX.\'asset_link l ON a.assetid = l.minorid
						     INNER JOIN \'.SQ_TABLE_RUNNING_PREFIX.\'asset_path pt ON a.assetid = pt.assetid \';
				if (!empty($USERIDS_COND)) {
				$sql   .= \'
						     INNER JOIN \'.SQ_TABLE_RUNNING_PREFIX.\'asset_permission p ON a.assetid = p.assetid \';
				}
				$where  = \'l.majorid IN (\'.substr($majorids_str, 0, -1).\')
						     AND (l.link_type & \'.$db->quote(SQ_SC_LINK_FRONTEND_NAV).\') > 0 \';
				$where  = $GLOBALS[\'SQ_SYSTEM\']->constructRollbackWhereClause($where, \'a\');
				$where  = $GLOBALS[\'SQ_SYSTEM\']->constructRollbackWhereClause($where, \'l\');
				if (!empty($USERIDS_COND)) {
					$where  = $GLOBALS[\'SQ_SYSTEM\']->constructRollbackWhereClause($where, \'p\');
				}
				$where  = $GLOBALS[\'SQ_SYSTEM\']->constructRollbackWhereClause($where, \'pt\');
				$where .= \' \'.$USERIDS_COND .\'
						   ORDER BY l.majorid, l.sort_order\';

				$result = $db->query($sql.$where);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().\'<br/>\'.$result->getUserInfo(), E_USER_ERROR);
				}

				$assetids = Array();
				while (DB_OK === $result->fetchInto($row)) {
					if (!($row[\'status\'] & (SQ_STATUS_LIVE | SQ_STATUS_LIVE_APPROVAL))) {
						$menu_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset($row[\'assetid\']);
						$read_access = $menu_asset->readAccess();

						$row[\'name\'] = $menu_asset->name;
						$row[\'short_name\'] = $menu_asset->short_name;

						if ($row[\'status\'] & SQ_SC_STATUS_NOT_LIVE) {
							// somewhere between under construction and live so we show this by altering the name
							$row[\'name\']       = \'(( \'.$row[\'name\'].\' ))\';
							$row[\'short_name\'] = \'(( \'.$row[\'short_name\'].\' ))\';
						}

						$GLOBALS[\'SQ_SYSTEM\']->am->forgetAsset($menu_asset);
						unset($menu_asset);
						if (!$read_access) continue;
					}
					$assetids[] = (int) $row[\'assetid\'];
					if (!isset($GLOBALS[\'MENU_DATA\'][$row[\'majorid\']])) $GLOBALS[\'MENU_DATA\'][$row[\'majorid\']] = Array();
					$GLOBALS[\'MENU_DATA\'][$row[\'majorid\']][$row[\'assetid\']] = $row;

				}
				$result->free();

				return $assetids;

			}// end menu_get_assets()

			';
			$GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$design->id] = true;
		}

		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_menu->printStaticInitOutput($design);
		}

	}//end printStaticInitOutput()


	/**
	* Performs any outputting needed to intialise this instance of the menu type
	*
	* @param object Design	$design	the design for which we are currently printing
	*
	* @access public
	*/
	function printInitOutput(&$design)
	{

		echo '
		$'.$this->attr('id_name').'_level = '.((int) $this->attr('level')).';';

		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_menu->printInitOutput($design);
		}

	}//end printInitOutput()


	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @access public
	*/
	function printDataCollection(&$design)
	{
		//// SHOULD BE OVERRIDDEN ////

		// the _assetids array holds all assetids that the sub level (if any) will printing kids of
		// if the show_subs is only on_current the asset for the kids level in the page lineage is used
		echo '
		$'.$this->attr('id_name').'_assetids = Array(); // menu_get_assets(majorids);
		$'.$this->attr('id_name').'_urls     = $am->getAssetHref($'.$this->attr('id_name').'_assetids);

		';

		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_menu->printDataCollection($design);
		}

	}//end printDataCollection()


	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		// if they are trying to set a page position dependent setting
		if (substr($var, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			list(,$setting_name) = explode('.', $var, 2);
			if ($setting_name && $this->settingExists($setting_name)) {

				// we got php code ? OK, we need to use a switch statement
				if ($this->_settingHasPHPCode($setting_name)) {
					$settings = $this->attr('settings');
					echo '<', '?php switch($'.$this->attr('id_name').'_current_setting){ ';
					foreach($settings[$setting_name] as $case => $value) {
						echo 'case \''.addslashes($case).'\' : ?','>', $value, '<','?php break; ';
					}
					echo '}// end switch ?', '>';

				// no php code ? cool, we can read from the array
				} else {
					echo '<', '?php echo $'.$this->attr('id_name').'_settings[\''.$setting_name.'\'][$'.$this->attr('id_name').'_current_setting]; ?', '>';

				}// end if


			}// end if
		} else {

			switch($var) {
				case 'assetid' :
					echo '<', '?php echo $'.$this->attr('id_name').'_row[\'assetid\']; ?', '>';
					break;

				case 'asset_link' :
					echo '<', '?php echo $'.$this->attr('id_name').'_urls[$'.$this->attr('id_name').'_row[\'assetid\']]; ?', '>';
					break;

				case 'asset_short_name' :
					echo '<', '?php echo ', $this->_escapeVar($value_prefix.'$'.$this->attr('id_name').'_row[\'short_name\']'.$value_suffix, $args), '; ?', '>';
					break;

				case 'asset_name' :
					echo '<', '?php echo ', $this->_escapeVar($value_prefix.'$'.$this->attr('id_name').'_row[\'name\']'.$value_suffix, $args), '; ?', '>';
					break;

				default :
					parent::printVar($var, $args);
			}// end switch
		}// end if

	}//end printVar()


	/*
	* Returns the settings array that will be need to be var_exported
	*
	* @access public
	*/
	function _settingHasPHPCode($setting_name)
	{
		$settings = $this->attr('settings');
		// OK we need out find out if any of the values for this setting have PHP code that needs to be
		$has_php_code = false;
		foreach($settings[$setting_name] as $case => $value) {
			// we know that this will be the long format because only our design areas have put this in
			if (strpos($value, '<'.'?php') !== false) return true;
		}

	}//end _settingHasPHPCode()

	/*
	* Returns the settings array that will be need to be var_exported
	*
	* @access public
	*/
	function _getSettingsArray()
	{
		$settings = $this->attr('settings');
		// OK we need out find out if any of the values for this setting have PHP code that needs to be
		$names = array_keys($settings);
		foreach($names as $setting_name) {
			// we don't want any of the settings that need to be processed
			if ($this->_settingHasPHPCode($setting_name)) {
				unset($settings[$setting_name]);
			}
		}// end if

		return $settings;

	}//end _getSettingsArray()


}//end class

?>