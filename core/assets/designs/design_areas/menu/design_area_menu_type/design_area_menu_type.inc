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
* $Id: design_area_menu_type.inc,v 1.117 2013/07/25 23:23:48 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Menu_Type
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.117 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Type extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID of the asset to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array('design_area_menu_type' => Array('card' => 'M', 'exclusive' => FALSE)),
			   );

	}//end _getAllowedLinks()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		// if they are trying to set an asset position dependent setting
		if (substr($name, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			// where setting type is either 'normal', 'heirarchy' or 'current'
			$split = explode('.', $name);
			if (count($split) != 3) {
				trigger_localised_error('CORE0173', sprintf(translate('Setting "%s" unable to be set, three elements in name not found'), $name), E_USER_WARNING);
				return FALSE;
			}
			switch ($split[2]) {
				case 'normal'    :
				case 'hierarchy' :
				case 'current'   :
					$new_value = $this->vars['settings']['value'];
					if (!is_array($new_value)) $new_value = Array();
					if (!isset($new_value[$split[1]])) {
						$new_value[$split[1]] = Array();
					}
					$new_value[$split[1]][$split[2]] = $value;
					$name  = 'settings';
					$value = $new_value;
				break;

				default :
					trigger_localised_error('CORE0174', sprintf(translate('Settings Type "%s" not known'), $split[2]), E_USER_WARNING);
					return FALSE;
			}// end switch

		// if they are trying to set a level, so some validation
		} else if ($name == 'level') {
			switch (strtolower($value)) {
				case 'top':
					$value = 0;
				break;
				case 'sub':
					$value = 1;
				break;
				case 'current':
					$value = -1;
				break;
				case 'children':
					$value = -2;
				break;
				default :
					$value = (int) $value;
			}// end switch
		} else if (($name == 'unwanted_asset_types') && is_scalar($value)) {
			$sub_menu_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'design_area_menu_type', FALSE, 'sub_menu');
			// Update sub menu attributes too
			if (!empty($sub_menu_link)) {
				$smenu = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_menu_link['minorid'], $sub_menu_link['minor_type_code'], TRUE);
				$smenu->setAttrValue('unwanted_asset_types', $value);
				$smenu->saveAttributes();
			}//end foreach

		} else if (($name == 'wanted_asset_types') && is_scalar($value)) {
			$sub_menu_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'design_area_menu_type', FALSE, 'sub_menu');
			// Update sub menu attributes too
			if (!empty($sub_menu_link)) {
				$smenu = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_menu_link['minorid'], $sub_menu_link['minor_type_code'], TRUE);
				$smenu->setAttrValue('wanted_asset_types', $value);
				$smenu->saveAttributes();
			}//end foreach

		}// end if

		if (($name == 'unwanted_asset_types' || $name == 'wanted_asset_types')  && is_scalar($value)) {

			$value = preg_split('/[ ,]+/', (string)$value);
			foreach ($value as $type_code) {
				if (!$GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
					trigger_localised_error('CORE0145', sprintf(translate('Asset Type "%s" is not installed in this system, unable to set "type_codes"'), $type_code), E_USER_WARNING);
					return FALSE;
				}
			}// end foreach
		}


		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name	The name of the attribute to get the value for
	*
	* @return mixed string|NULL
	* @access public
	*/
	public function attr($name)
	{
		// if they are trying to set an asset position dependent setting
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
	* @return array
	* @access public
	*/
	public function getVarReferences()
	{
		$var_refs = parent::getVarReferences();
		$sub_menu = $this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_var_refs = $sub_menu->getVarReferences();
			for (reset($sub_var_refs); NULL !== ($id_name = key($sub_var_refs)); next($sub_var_refs)) {
				if (!isset($var_refs[$id_name])) {
					$var_refs[$id_name] = Array();
				}
				foreach ($sub_var_refs[$id_name] as $var) {
					if (in_array($var, $var_refs[$id_name])) continue;
					$var_refs[$id_name][] = $var;
				}// end foreach
			}// end foreach
		}// end if

		return $var_refs;

	}//end getVarReferences()


	/**
	* Sets the values for the var references values (registered in $this->var_references)
	*
	* @param array	$var_ref_values	array of values - Array(id_name => Array(var => [value]))
	*
	* @return void
	* @access public
	*/
	public function setVarReferenceValues(Array $var_ref_values)
	{
		$settings = $this->attr('settings');
		$settings_changed = FALSE;
		foreach ($this->vars['var_references']['value'] as $var_name => $info) {
			// if this is a setting and it is available in the var_ref_values
			if (substr($var_name, 0, 9) == 'settings.' && isset($var_ref_values[$info['id_name']][$info['var']])) {
				// The name is in the form 'settings.[var_name].[setting type]'
				$split = explode('.', $var_name);
				if (count($split) == 3) {
					if (!isset($settings[$split[1]])) {
						$settings[$split[1]] = Array();
					}
					$settings[$split[1]][$split[2]] = $var_ref_values[$info['id_name']][$info['var']];
					$settings_changed = TRUE;
				}// end if
			}// end if
		}// end foreach

		if ($settings_changed) {
			$this->_tmp['var_reference_values']['settings'] = $settings;
		}

		// send our custom array to the parent fn
		parent::setVarReferenceValues($var_ref_values);
		$sub_menu = $this->getSubMenu();
		if (!is_null($sub_menu)) {
			// now send the original array to the sub
			$sub_menu->setVarReferenceValues($var_ref_values);
		}// end if

	}//end setVarReferenceValues()


	/**
	* Returns the link to the sub menu for this menu (if any)
	*
	* @return array
	* @access public
	*/
	public function getSubMenuLink()
	{
		if (empty($this->_tmp['sub_menu_link']) ||(!isset($this->_tmp['sub_menu_link']))) {
			$this->_tmp['sub_menu_link'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'design_area_menu_type', FALSE, 'sub_menu');
		}
		return $this->_tmp['sub_menu_link'];

	}//end getSubMenuLink()


	/**
	* Returns the sub menu for this menu (if any)
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function &getSubMenu()
	{
		$link = $this->getSubMenuLink();
		if (empty($link)) {
			$null = NULL;
			return $null;
		} else {
			$smenu = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code'], TRUE);
			return $smenu;
		}

	}//end getSubMenu()


	/**
	* Returns the parent menu for this menu (assuming it is a sub menu)
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function &getParentMenu()
	{
		if (!isset($this->_tmp['parent_menu_link'])) {
			$this->_tmp['parent_menu_link'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'design_area_menu_type', FALSE, 'sub_menu', 'minor');
		}
		if (empty($this->_tmp['parent_menu_link'])) {
			$null = NULL;
			return $null;
		} else {
			$pmenu = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_tmp['parent_menu_link']['majorid'], $this->_tmp['parent_menu_link']['major_type_code'], TRUE)    ;
			return $pmenu;
		}

	}//end getParentMenu()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
	*
	* @return void
	* @access private
	*/
	public function linksUpdated()
	{
		parent::linksUpdated();
		unset($this->_tmp['sub_menu_link']);
		unset($this->_tmp['parent_menu_link']);

	}//end linksUpdated()


	/**
	* Returns whether the passed setting exists or not
	*
	* @param string	$name	Setting name
	*
	* @return boolean
	* @access public
	*/
	public function settingExists($name)
	{
		$settings = $this->attr('settings'); // do like this to take into account var refs
		return !empty($settings[$name]);

	}//end settingExists()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* This public version is overridden here only because menus print some code
	* that is shared by all menu DAs and should never be cached
	*
	* @return void
	* @access public
	*/
	public function printInitCode()
	{
		// Print output that is only required once per parse file (shared by all menu DAs)
		echo '<', '?php
		';
		$this->printStaticInitOutput();
		echo '
		?', '>
		';

		parent::printInitCode();

	}//end printInitCode()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		echo '<', '?php
		';
		$parent_menu = $this->getParentMenu();
		// if we aren't a sub menu
		if (is_null($parent_menu)) {
			$this->printInitOutput();
			echo '
		if (!empty($ASSET_LINEAGE) && !empty($ASSET_LINEAGE[$'.$this->attr('id_name').'_level])) {
			';
			$this->printDataCollection();
			$this->_tmp['parent_assetid_name'] = '$ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']';
		} else {
			$this->_tmp['parent_assetid_name'] = '$'.$parent_menu->attr('id_name').'_row[\'assetid\']';
		}// end if

		echo '
			$'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), TRUE).';
		';

		// if we aren't a sub menu
		if (is_null($parent_menu)) {
			echo '
		}// end if found in ASSET_LINEAGE
			';
		}// end if

		echo '
		?', '>';

		return TRUE;

	}//end _printInitCode()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* This function can be overridden (partially) by children
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		echo '
			<?php
			if (isset('.$this->_tmp['parent_assetid_name'].') && !empty($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).']['.$this->_tmp['parent_assetid_name'].'])) {
				?', '>
		';

		parent::_printPaintingCode();

		echo '
				<', '?php
			}// end if subs '.$this->attr('id_name').'
			?>
		';

	}//end _printPaintingCode()


	/**
	* Performs any outputting needed to by all design areas of this menu type
	*
	* NOTE: assumes PHP tags are open
	*
	* @return void
	* @access public
	*/
	public function printStaticInitOutput()
	{
		// Stuff that is only needed once per design file
		if (empty($GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$GLOBALS['DESIGN_BEING_GENERATED']->id])) {
			echo '
			$db = MatrixDAL::getDb();

			require_once SQ_FUDGE_PATH.\'/db_extras/db_extras.inc\';
			require_once SQ_CORE_PACKAGE_PATH.\'/designs/design_areas/menu/design_area_menu_type/menu_get_assets.inc\';

			$GLOBALS[\'MENU_DATA\'] = Array();

			';
			$GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$GLOBALS['DESIGN_BEING_GENERATED']->id] = TRUE;
		}//end if

		$sub_menu = $this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_menu->printStaticInitOutput();
		}

	}//end printStaticInitOutput()


	/**
	* Performs any outputting needed to intialise this instance of the menu type
	*
	* @return void
	* @access public
	*/
	public function printInitOutput()
	{
		$level = (int) $this->attr('level');
		$parent_menu = $this->getParentMenu();
		if (!is_null($parent_menu)) {
			// we are a sub menu
			if ($level == 0) {
				// we dont have a specific level to print (or they want to print the top level - thats wierd)
				// so dynamically get the level to print based on our parent's level at runtime
				$level = NULL;
				echo '
				$'.$this->attr('id_name').'_level = $'.$parent_menu->attr('id_name').'_level + 1;';
			}
		}

		if (!is_null($level)) {
			if ($level < 0) {
				echo '
				$'.$this->attr('id_name').'_level = (count($ASSET_LINEAGE) -2) + '.(-1 * ($level + 1)).';';
			} else {
				echo '
				$'.$this->attr('id_name').'_level = '.$level.';';
			}
		}

		$sub_menu = $this->getSubMenu();
		if (!is_null($sub_menu)) $sub_menu->printInitOutput();

	}//end printInitOutput()


	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @return void
	* @access public
	*/
	public function printDataCollection()
	{
		//// SHOULD BE OVERRIDDEN ////

		// the _assetids array holds all assetids that the sub level (if any) will printing kids of
		// if the show_subs is only on_current the asset for the kids level in the asset lineage is used
		echo '
		$'.$this->attr('id_name').'_assetids = Array(); // menu_get_assets(majorids);
		$'.$this->attr('id_name').'_urls     = $am->getAssetURL($'.$this->attr('id_name').'_assetids);

		';

		$sub_menu = $this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_menu->printDataCollection();
		}

	}//end printDataCollection()


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args, $value_prefix='', $value_suffix='')
	{
		// if they are trying to set an asset position dependent setting
		if (substr($var, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			list(,$setting_name) = explode('.', $var, 2);
			if ($setting_name && $this->settingExists($setting_name)) {

				// we got php code ? OK, we need to use a switch statement
				if ($this->_settingHasPHPCode($setting_name)) {
					$settings = $this->attr('settings');
					echo '<', '?php switch ($'.$this->attr('id_name').'_current_setting){ ';
					foreach ($settings[$setting_name] as $case => $value) {
						echo 'case \''.addslashes($case).'\' : ?','>', $value, '<','?php break; ';
					}
					echo '}// end switch ?', '>';

				// no php code ? cool, we can read from the array
				} else {
					echo '<', '?php echo $'.$this->attr('id_name').'_settings[\''.$setting_name.'\'][$'.$this->attr('id_name').'_current_setting]; ?', '>';

				}// end if


			}// end if
		} else {

			// if we are not in the asset looping section then we need to print the details of
			// our parent asset - if we are not a sub menu there is nothing we can do
			if (empty($this->_tmp['outputting_asset_section'])) {
				$parent_menu = $this->getParentMenu();
				// if we aren't a sub menu, there is nothing we can do
				if (is_null($parent_menu)) {
					$id_name = '';
				} else {
					$id_name = $parent_menu->attr('id_name');
				}
			// otherwise we are in the asset section and we want to print our data
			} else {
				$id_name = $this->attr('id_name');

			}

			switch ($var) {
				case 'assetid' :
					if ($id_name) {
						echo '<', '?php echo $'.$id_name.'_row[\'assetid\']; ?', '>';
					}
				break;

				case 'asset_link' :
					if ($id_name) {
						echo '<', '?php echo $'.$id_name.'_urls[$'.$id_name.'_row[\'assetid\']].((SQ_LIMBO_EDITING) ? \'/\'.SQ_CONF_LIMBO_SUFFIX.\'?SQ_BACKEND_PAGE=main\' : \'\'); ?', '>';
					}
				break;

				case 'asset_short_name' :
					if ($id_name) {
						echo '<', '?php echo ', $this->_escapeVar($value_prefix.'htmlspecialchars($'.$id_name.'_row[\'short_name\'])'.$value_suffix, $args), '; ?', '>';
					}
				break;

				case 'asset_name' :
					if ($id_name) {
						echo '<', '?php echo ', $this->_escapeVar($value_prefix.'htmlspecialchars($'.$id_name.'_row[\'name\'])'.$value_suffix, $args), '; ?', '>';
					}
				break;

				case 'asset_name_linked' :
				case 'asset_short_name_linked' :
					if ($id_name) {

						$name_var = substr($var, 0, -7);

						if ($this->attr('self_link')) {
							echo '<a href="', $this->printVar('asset_link', Array()), '">', $this->printVar($name_var, Array(), $value_prefix, $value_suffix), '</a>';
						} else {
							echo '<', '?php ', "\n";
							echo ' if ($'.$id_name.'_current_setting == \'current\') { ', "\n";
							echo '?', '>', $this->printVar($name_var, Array(), $value_prefix, $value_suffix), '<', '?php ', "\n";
							echo '} else { ', "\n";
							echo '?', '><a href="', $this->printVar('asset_link', Array()), '">', $this->printVar($name_var, Array(), $value_prefix, $value_suffix), '</a><', '?php ', "\n";
							echo '}// end if ?', '>';
						}// end if

					}// end if
				break;

				case 'asset_thumbnail' :
					if ($id_name) {
						echo '<', '?php ';
						echo '$thumbnail = NULL;'."\n";
						echo '$thumbnail_asset = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($'.$id_name.'_row[\'assetid\']);'."\n";
						echo 'if ($thumbnail_asset->useSystemVersion()) {'."\n";
						echo '    $notice_links = unserialize(file_to_string($thumbnail_asset->data_path.\'/.sq_notice_links\'));'."\n";
						echo '    foreach ($notice_links as $link) {'."\n";
						echo '		  if ($link[\'value\'] == \'thumbnail\') {'."\n";
						echo '			  $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
						echo '			  break;'."\n";
						echo '		  }'."\n";
						echo '	  }'."\n";
						echo '} else {'."\n";
						echo '    $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink($'.$id_name.'_row[\'assetid\'], SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
						echo '    if (!empty($link)) {'."\n";
						echo '        $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
						echo '    }'."\n";
						echo '}'."\n";
						echo 'if ($thumbnail) echo $thumbnail->printImageTag(\'\', \'\', TRUE);'."\n";
						echo '?', '>';
					}
				break;

				case 'asset_thumbnail_url' :
					if ($id_name) {
						echo '<', '?php ';
						echo '$thumbnail = NULL;'."\n";
						echo '$thumbnail_asset = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($'.$id_name.'_row[\'assetid\']);'."\n";
						echo 'if ($thumbnail_asset->useSystemVersion()) {'."\n";
						echo '    $notice_links = unserialize(file_to_string($thumbnail_asset->data_path.\'/.sq_notice_links\'));'."\n";
						echo '    foreach ($notice_links as $link) {'."\n";
						echo '		  if ($link[\'value\'] == \'thumbnail\') {'."\n";
						echo '			  $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
						echo '			  break;'."\n";
						echo '		  }'."\n";
						echo '	  }'."\n";
						echo '} else {'."\n";
						echo '    $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink($'.$id_name.'_row[\'assetid\'], SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
						echo '    if (!empty($link)) {'."\n";
						echo '        $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
						echo '    }'."\n";
						echo '}'."\n";
						echo 'if ($thumbnail) echo $thumbnail->getURL($'.$id_name.'_row[\'name\']);'."\n";
						echo '?', '>';
					}
				break;

				case (preg_match('/^asset_/i', $var) ? $var : !$var):
						echo '<', '?php ';
						echo '$keyword = \'',$var, '\';';
						echo '$ROW_ASSET = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($'.$id_name.'_row[\'assetid\']);';
						echo 'if (!is_null($ROW_ASSET)) {';
						echo '$need_brackets = FALSE;';
						if(preg_match('/^asset_attribute_(short_)?name/i', $var)) {
						    echo '$need_brackets = $ROW_ASSET->status & SQ_SC_STATUS_NOT_LIVE;';
						}
						echo '$replacement = '.$this->_escapeVar('$ROW_ASSET->getKeywordReplacement($keyword)', $args).';';
						echo 'if ($replacement == $keyword) {';
						echo '	echo \'\';';
						echo '} else {';
						echo '          echo $need_brackets ? "(( " : "";';
						echo '	echo $replacement;';
						echo '          echo $need_brackets ? " ))" : "";';
						echo '}';
						echo '$GLOBALS[\'SQ_SYSTEM\']->am->forgetAsset($ROW_ASSET);';
						echo '} else {';
						echo '  echo \'\';';
						echo '}';
						echo '?', '>';
				break;

				default :
					parent::printVar($var, $args);
			}//end switch
		}//end else

	}//end printVar()


	/**
	* Returns the settings array that will be need to be var_exported
	*
	* @param string	$setting_name	Setting name
	*
	* @return boolean
	* @access public
	*/
	public function _settingHasPHPCode($setting_name)
	{
		$settings = $this->attr('settings');
		// OK we need out find out if any of the values for this setting have PHP code that needs to be
		$has_php_code = FALSE;
		foreach ($settings[$setting_name] as $case => $value) {
			// we know that this will be the long format because only our design areas have put this in
			if (strpos($value, '<'.'?php') !== FALSE) return TRUE;
		}

	}//end _settingHasPHPCode()


	/**
	* Returns the settings array that will be need to be var_exported
	*
	* @return array
	* @access public
	*/
	public function _getSettingsArray()
	{
		$settings = $this->attr('settings');
		// OK we need out find out if any of the values for this setting have PHP code that needs to be
		$names = array_keys($settings);
		foreach ($names as $setting_name) {
			// we don't want any of the settings that need to be processed
			if ($this->_settingHasPHPCode($setting_name)) {
				unset($settings[$setting_name]);
			}
		}// end if

		return $settings;

	}//end _getSettingsArray()


	/**
	* Get the names of attributes for this DA that cannot be set in the parse file
	*
	* Design Area attributes that are not mentioned in the parse file are normally set to defaults
	* when parsing the design.  Attributes returned by this function are exempted from that process.
	*
	* @return array
	* @access public
	*/
	public function getProtectedAttrs()
	{
		$res = parent::getProtectedAttrs();
		$res[] = 'asset_contents';
		$res[] = 'divider_contents';
		$res[] = 'settings';
		return $res;

	}//end getProtectedAttrs()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$element	An entry from our contents array
	*
	* @return void
	* @access private
	*/
	protected function _printContentItemCode(Array $element)
	{
		switch (array_get_index($element, 'operation')) {
			case 'asset' :
				if ($divider_contents = $this->attr('divider_contents')) {
					ob_start();
						foreach ($divider_contents as $item) {
							$this->_printContentItemCode($item);
						}
						$divider = ob_get_contents();
					ob_end_clean();
				} else {
					$divider = '';
				}

				// should we start with a divider ?
				if ($this->attr('prefix_with_divider')) echo $divider;

				echo '<', '?php

				$'.$this->attr('id_name').'_lineage_assetid = (empty($ASSET_LINEAGE[$'.$this->attr('id_name').'_level + 1])) ? 0 : $ASSET_LINEAGE[$'.$this->attr('id_name').'_level + 1][\'assetid\'];
				$'.$this->attr('id_name').'_lineage_setting = (count($ASSET_LINEAGE) - 1 == $'.$this->attr('id_name').'_level + 1) ? \'current\' : \'hierarchy\';

				';
				$parent_menu = $this->getParentMenu();
				// if we aren't a sub menu
				if (is_null($parent_menu)) {
					$this->_tmp['parent_assetid_name'] = '$ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']';

					echo '
				$'.$this->attr('id_name').'_base_url = (empty($ASSET_LINEAGE[$'.$this->attr('id_name').'_level])) ? \'\' : current_protocol().\'://\'.$ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'url\'];
				$'.$this->attr('id_name').'_urls     = $am->getAssetURL(array_keys($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).'][$ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']]), $'.$this->attr('id_name').'_base_url);
					';

				} else {
					$this->_tmp['parent_assetid_name'] = '$'.$parent_menu->attr('id_name').'_row[\'assetid\']';

					echo '
				$'.$this->attr('id_name').'_base_url = $'.$parent_menu->attr('id_name').'_urls[$'.$parent_menu->attr('id_name').'_row[\'assetid\']];
				$'.$this->attr('id_name').'_urls     = $am->getAssetURL(array_keys($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).'][$'.$parent_menu->attr('id_name').'_row[\'assetid\']]), $'.$this->attr('id_name').'_base_url);
					';

				}// end if

				// if there is a divider to deal with and
				// if there is not divider at the end we these vars
				if ($divider != '' && !$this->attr('suffix_with_divider')) {
					echo '

				$'.$this->attr('id_name').'_i = -1;
				$'.$this->attr('id_name').'_total = count($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).']['.$this->_tmp['parent_assetid_name'].']);
					';
				}

				echo '

				foreach ($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).']['.$this->_tmp['parent_assetid_name'].'] as $'.$this->attr('id_name').'_row) {
					$'.$this->attr('id_name').'_current_setting = ($'.$this->attr('id_name').'_lineage_assetid == $'.$this->attr('id_name').'_row[\'assetid\']) ? $'.$this->attr('id_name').'_lineage_setting : \'normal\';
				?', '>';

				$unwanted_types = $this->attr('unwanted_asset_types');
				$all_type_codes = Array();
				foreach ($unwanted_types as $type) {
					if (!empty($type)) {
						$all_type_codes = array_merge($all_type_codes, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type, TRUE));
					}
				}
				$unwanted_types = $all_type_codes;

				if (!empty($unwanted_types)) {
					$unwanted_types_literal_array = 'Array("'.implode('", "', $unwanted_types).'")';
					// We are sub menu
					echo '<?php  if (in_array($'.$this->attr('id_name').'_row[\'type_code\'], '.$unwanted_types_literal_array.')) continue ;?>';
				}//end if

				$wanted_types = $this->attr('wanted_asset_types');
				$all_type_codes = Array();
				foreach ($wanted_types as $type) {
					if (!empty($type)) {
						$all_type_codes = array_merge($all_type_codes, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type, TRUE));
					}
				}
				$wanted_types = $all_type_codes;

				if (!empty($wanted_types)) {
					$wanted_types_literal_array = 'Array("'.implode('", "', $wanted_types).'")';
					echo '<?php  if (!in_array($'.$this->attr('id_name').'_row[\'type_code\'], '.$wanted_types_literal_array.')) continue ;?>';
				}

				if ($this_contents = $this->attr('asset_contents')) {
					$this->_tmp['outputting_asset_section'] = TRUE;
					foreach ($this_contents as $item) {
						$this->_printContentItemCode($item);
					}
					unset($this->_tmp['outputting_asset_section']);
				}



				// if there is a divider to deal with
				if ($divider != '') {

					// if there is not divider at the end we need to check that we aren't at the end of the list
					if (!$this->attr('suffix_with_divider')) {
						echo '<', '?php
							$'.$this->attr('id_name').'_i++;
							if ($'.$this->attr('id_name').'_i != $'.$this->attr('id_name').'_total - 1) {
							?','>';
					}

					echo $divider;

					// if there is not divider at the end we need to check that we aren't at the end of the list
					if (!$this->attr('suffix_with_divider')) {
						echo '<','?php
							}// end if
						?','>';
					}

				}// end if divider

				echo '<', '?php
				}// end foreach '.$this->attr('id_name').' data
				?', '>';

				return TRUE;

			case 'sub' :
				$sub = $this->getSubMenu();
				if (is_null($sub)) return TRUE;

				// See if this menu has a customisation underneath it
				// If does, use the customised settings in this asset
				$current_designs = $sub->getDesigns();
				foreach ($current_designs as $current_design) {

					if (is_null($current_design)) continue;

					// Find the current customisations
					$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($current_design->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
					$am =& $GLOBALS['SQ_SYSTEM']->am;
					foreach ($customisation_links as $link) {
						$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
						if (is_null($customisation)) continue;

						// Do something
						$custom_das = $customisation->getCustomisedAreas();
						foreach ($custom_das as $custom_da) {
							if ($custom_da['minorid'] != $sub->id) {
								$new_da = $am->getAsset($custom_da['minorid'], $custom_da['minor_type_code']);
								if (is_null($new_da)) continue;
								if ($new_da->name == $sub->name) {
									foreach ($new_da->vars as $name => $details) {
										if (in_array($name, Array('unwanted_asset_types', 'self_link', 'wanted_asset_types'))) {
											$sub->vars[$name] = $new_da->vars[$name];
										}
									}
								}//end if
							}//end if
						}//end foreach
					}//end foreach
				}//end foreach
				return $sub->printAreaCode();

			default :
				return parent::_printContentItemCode($element);
		}//end switch

	}//end _printContentItemCode()

}//end class
?>
