<?php

require_once 'XML/Tree.php';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Interface
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Edit_Interface extends Resolve_Object
{

	/**
	* The asset type that we are creating the interface for
	* @var string
	*/
	var $_type_code = '';

	/**
	* The Edit Fns object for dealing with any complex attribute painting and processing
	* @var object Asset_Edit_Fns
	*/
	var $_edit_fns = null;

	/**
	* An array of dirs to check for certain files for this asset type
	* @var Array()
	*/
	var $_dirs = Array();

	/**
	* An array of screen code names and their display names
	* @var Array()
	*/
	var $_screens = Array();

	/**
	* The code name for default screen for the interface - used when no existing screen is specified
	* @var string
	*/
	var $_default_screen = '';


	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Asset_Edit_Interface($type_code)
	{
		$this->_type_code = $type_code;

		$asset_dir = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'dir');
		$class_name = $this->_type_code.'_edit_fns';
		require_once $asset_dir.'/'.$class_name.'.inc';
		$this->_edit_fns = new $class_name();

		// check that the edit functions have a details tab
		// or else we cant even create assets of this type
		// let alone edit them
		if (!isset($this->_edit_fns->static_screens['details'])) {
			trigger_error("Asset Edit Functions for '$type_code' do not define a 'details' tab", E_USER_ERROR);
			die();
		}

		$this->_dirs = Array($asset_dir);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->_type_code, false);
		foreach($parents as $parent_type) {
			$this->_dirs[] = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($parent_type, 'dir');
		}

		// add the default for Asset
		$this->_dirs[] = SQ_INCLUDE_PATH.'/asset_edit';

		if(!($file = $this->_findFile('screens'))) return;

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		$this->_screens = Array();
		for($i = 0; $i < count($root->children); $i++) {
			$code_name = strtolower(preg_replace('/[^a-z_]/', '', $root->children[$i]->attributes['code_name']));
			if (!$code_name) continue;
			$this->_screens[$code_name] = Array(
												'name' => $root->children[$i]->attributes['display_name'],
												'icon' => $root->children[$i]->attributes['icon']
											);

			$force_unlock = ((isset($root->children[$i]->attributes['force_unlock'])) ? $root->children[$i]->attributes['force_unlock'] : '1');
			if ($force_unlock == 0) $this->_screens[$code_name]['force_unlock'] = false;
			else $this->_screens[$code_name]['force_unlock'] = true;
			
			if (!empty($root->children[$i]->attributes['default'])) $this->_default_screen = $code_name;
		}// end for

	}// end constructor;


	/**
	* Returns the screens array
	*
	* @access public
	*/
	function getScreens()
	{
		return $this->_screens;
	}


	/**
	* Returns the prefix that will be used by paint() and process()
	*
	* @param object	Asset	$owner	the asset whose interface we are painting
	*
	* @access public
	*/
	function getPrefix(&$owner)
	{
		return $owner->type().'_'.$owner->id;
	}


	/**
	* Paint's out the interface for the currently selected frame
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$creating	whether we are creating or editing
	*
	* @access public
	*/
	function paint(&$owner, &$o, $creating)
	{
		// if we dont have read access, piss off
		if ($owner->id && !$owner->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$owner->name.'</i>');
			return;
		}

		// if there are no screens or static screens, piss off
		if (!count($this->_screens) && empty($this->_edit_fns->static_screens)) return;

		// do we have write access?
		$write_access = $owner->writeAccess();
		$full_write_access = $owner->writeAccess(Array(), false);

		// do we unlock this asset when committing?
		$force_unlock_on_commit = true;

		if (!$creating) {
			// lets tell the user if the asset is locked for editing
			// or if they can lock it to edit it
			$o->openSection('Locking / Editing');
			$o->openField('&nbsp;');
			$lock = $owner->getLockInfo();
			if (!empty($lock)) {
				// this asset is currently locked
				// so display message to the user
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
				$editing = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_asset']);

				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$expires_in = easy_time_total(($lock['expires'] - time()), true);
				if (!$expires_in) $expires_in = '1 second';

				echo '<p class="sq-form-field">"'.$owner->name.'" is currently locked for editing by user "'.$user->name.'" at "'.$editing->name.'". The lock is due to expire in '.$expires_in.'</p>';

				if ($GLOBALS['SQ_SYSTEM']->currentUser($user)) {
					submit_button('sq_manual_release_lock', 'Release Lock');
				}
				
				// does the current user has a high
				// enough level of access to forceably aquire the lock
				$can_aquire_lock = $owner->canForceablyAquireLock();
				
				if ($can_aquire_lock) {
					echo '<p class="sq-form-field">You can forceably aquire this lock, which will remove the lock from "'.$user->name.'" and give the lock to yourself. Any changes currently being made by "'.$user->name.'" will be lost if this lock is forceably aquired.</p>';

					submit_button('aquire_lock', 'Aquire Lock');
				}
			} else if ($full_write_access) {
				$write_access = false;
				submit_button('sq_aquire_lock', 'Aquire Lock');
			} else {
				echo 'You do not have access to edit "'.$owner->name.'".';
			}
		}

		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';

		// add all the screens to the combo, but only if we aren't creating the object
		if (!$creating) {
			for(reset($this->_screens); null !== ($code = key($this->_screens)); next($this->_screens)) {
				$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
			}
		}

		// add static screens for the asset if not creating
		if (!$creating) {
			foreach ($this->_edit_fns->static_screens as $code_name => $data) {
				$url = $current_location.'asset_ei_screen='.rawurlencode($code_name);
				$o->addStaticScreen($url, $data['name']);
			}
		}

		$create_sections = Array();
		$active_screen = (empty($_GET['asset_ei_screen'])) ? 'details' : $_GET['asset_ei_screen'];

		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_screen) = $this->_getCreateSections(true);
			if (!count($create_sections)) return;
		}

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('asset_ei_screen', $active_screen);

		// check the active screen for special system defined static screens
		$file = null;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {
			$o->setHeading($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name'));
			
			// if we are not creating, paint the system defined bit of this tab
			if (!$creating) {
				$paint_function = 'paint'.ucfirst($active_screen);
				$this->$paint_function($owner, $o);
			}

			$force_unlock_on_commit = $this->_edit_fns->static_screens[$active_screen]['force_unlock'];

			// try and find the file to print user defined section of the tab
			$file = @$this->_findFile('screen_'.$active_screen);

		} else {

			// must be a normal screen
			if (empty($this->_screens[$active_screen])) {
				trigger_error('Active Screen ("'.$active_screen.'") unknown', E_USER_WARNING);
				return;
			}

			// try and find the file with the screen details
			if(!($file = $this->_findFile('screen_'.$active_screen))) return;

			$force_unlock_on_commit = $this->_screens[$active_screen]['force_unlock'];

			$o->setHeading($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name').' - '.$this->_screens[$active_screen]['name'], $this->_screens[$active_screen]['icon']);
		}// end if

		if ($file) {
			$input = new XML_Tree($file);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
				return;
			}

			// prefix for all the form vars
			$prefix = $this->getPrefix($owner);

			// Loop over the sections
			for($i = 0; $i < count($root->children); $i++) {
				$section = &$root->children[$i];
				// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
				if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;

				$o->openSection($section->attributes['name']);

				if (!($num_fields = count($section->children))) continue;
				if ($section->children[0]->name == 'function_call') {

					$paint_node = &$section->children[0]->children[0];

					if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
						$o->openField('');
						echo 'Function not found for painting section "', $section->attributes['name'], '"';
					} else {
						$this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix);
					}

				// they must just be normal fields
				} else {

					// Loop over the fields
					for($j = 0; $j < $num_fields; $j++) {
						$field = &$section->children[$j];

						if ($field->children[0]->name != 'display_name') {
							trigger_error('Display Name for "'.$field->attributes['attribute'].'" not found', E_USER_WARNING);
							continue;
						}

						if (!isset($field->attributes['format'])) $field->attributes['format'] = '';

						if (!empty($field->attributes['attribute'])) {
							// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
							if ($field->name != 'field' || ($creating && (empty($field->attributes['attribute']) || !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']])))) continue;

							$o->openField($field->children[0]->content, $field->attributes['format']);

							$attr = &$owner->getAttribute($field->attributes['attribute']);
							if (is_null($attr)) continue;
							if (!$attr->setEditParams($field->children[2])) continue;
							$attr->paint($prefix, !$write_access);

						} else if ($field->children[2]->name == 'function_call') {

							$paint_node = &$field->children[2]->children[0];
							// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
							if ($field->name != 'field' || ($creating && !in_array($paint_node->attributes['name'], $create_sections[$section->attributes['name']]))) continue;

							$o->openField($field->children[0]->content, $field->attributes['format']);

							if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
								echo 'Function not found for painting';
							} else {
								$this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix);
							}

						}// end if

						if ($field->children[1]->name == 'note' && $field->children[1]->content) {
							echo '<br/>&nbsp;';
							$o->note($field->children[1]->content);
						}

					}// end for fields

				}// end if section uses function call

			}// end for

		}// end if $file


		$statii = Array();
		if (!$write_access) $statii = $owner->getAvailableStatii();

		// if the user has write access or has the ability
		// to alter the status of the asset, show the commit button
		if ($write_access || !empty($statii)) {
			$o->openSection();
			$o->commitButton('Commit', $force_unlock_on_commit);
		}

	}// end paint();


	/**
	* Processes the results of the form posting, returns true anything was processed/changed
	*
	* @param object Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$creating	whether we are creating or editing
	*
	* @return boolean
	* @access public
	*/
	function process(&$owner, &$o, $creating)
	{
		// if there are no screens or static screens, piss off
		if (!count($this->_screens) && empty($this->_edit_fns->static_screens)) return false;

		$ret_val = false;
		$create_sections = Array();
		$active_screen = (empty($_GET['asset_ei_screen'])) ? 'details' : $_GET['asset_ei_screen'];

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_screen) = $this->_getCreateSections(false);
			if (!count($create_sections)) return false;
		}

		// check the active screen for special system defined static screens
		$file = null;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {

			// process the system defined bit of this tab if we
			// are not creating the asset
			if (!$creating) {
				$process_function = 'process'.ucfirst($active_screen);
				if ($this->$process_function($owner, $o)) $ret_val = true;
			}

			// if we dont have write access, dont process the rest of the screen
			if ($owner->id && !$owner->writeAccess()) return $ret_val;

			// try and find the file to print user defined
			$file = @$this->_findFile('screen_'.$active_screen);

		} else {

			// if we dont have write access, piss off
			if ($owner->id && !$owner->writeAccess(Array(), false)) {
				$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit <i>'.$owner->name.'</i>');
				return false;
			}

			// must be a normal screen
			if (empty($_GET['asset_ei_screen']) || empty($this->_screens[$_GET['asset_ei_screen']])) {
				trigger_error('Active Screen unknown unable to process', E_USER_WARNING);
				return false;
			}

			// try and find the file to print user defined
			$file = $this->_findFile('screen_'.$active_screen);
			if(!$file) return false;

		}// end if

		if ($file && $owner->writeAccess()) {
			$input = new XML_Tree($file);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// prefix for all the form vars
			$prefix = $this->getPrefix($owner);
			
			// Loop over the sections
			for($i = 0; $i < count($root->children); $i++) {
				$section = &$root->children[$i];
				// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
				if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;
				if (!($num_fields = count($section->children))) continue;

				// if this is section only has function call, then process accordingly
				if ($section->children[0]->name == 'function_call') {

					$process_node = &$section->children[0]->children[1];

					// if there is nothing declared there is nothing for us to do
					if (empty($process_node->attributes['name'])) {
						continue;
					// There is something declared but the bloody thing doesn't exist - inform the Idiot
					} elseif (!method_exists($this->_edit_fns, $process_node->attributes['name'])) {
						trigger_error('Function not found for processing section "'.$section->attributes['name'].'"', E_USER_WARNING);
					} else {
						if ($this->_edit_fns->{$process_node->attributes['name']}($owner, $o, $prefix)) $ret_val = true;
					}

				// they must just be normal fields
				} else {
					// Loop over the fields
					for($j = 0; $j < $num_fields; $j++) {
						$field = &$section->children[$j];

						if (!empty($field->attributes['attribute'])) {
							// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
							if ($field->name != 'field' || ($creating && (empty($field->attributes['attribute']) || !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']])))) continue;

							$attr = &$owner->getAttribute($field->attributes['attribute']);
							if (is_null($attr)) continue;
							if (!$attr->setEditParams($field->children[2])) continue;

							$revert_attr = $attr;
							$attr->process($prefix);
							if ($attr->processed && $owner->setAttrValue($attr->name, $attr->value)) {
								$ret_val = true;
							} else {
								// processing failed so revert to old version
								$attr = $revert_attr;
							}
							unset($revert_attr);

						} else if ($field->children[2]->name == 'function_call') {

							$process_node = &$field->children[2]->children[1];

							// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
							if ($field->name != 'field' || ($creating && !in_array($process_node->attributes['name'], $create_sections[$section->attributes['name']]))) continue;

							// if there is nothing declared there is nothing for use to do
							if (empty($process_node->attributes['name'])) {
								continue;
							// There is something declared but the bloody thing doesn't exist - inform the Idiot
							} elseif (!method_exists($this->_edit_fns, $process_node->attributes['name'])) {
								trigger_error('Function not found for processing field "'.$section->attributes['name'].'/'.$field->children[0]->content.'"', E_USER_WARNING);
							} else {
								if ($this->_edit_fns->{$process_node->attributes['name']}($owner, $o, $prefix)) $ret_val = true;
							}

						}// end if

					}// end for fields

				}// end if section funcion call

			}// end for

		}// end if $file

		return $ret_val;

	}// end process();


	/**
	* Paints the interface for editing details
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @access public
	*/
	function paintDetails(&$owner, &$o)
	{
		if(!($file = $this->_findFile('static_screen_details'))) return;

		$prefix = $this->getPrefix($owner);

		if ($file) {
			$input = new XML_Tree($file);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
				return;
			}

			for($i = 0; $i < count($root->children); $i++) {
				$section = &$root->children[$i];
				
				$o->openSection($section->attributes['name']);

				if (!($num_fields = count($section->children))) continue;
				for($j = 0; $j < $num_fields; $j++) {
					$field = &$section->children[$j];

					switch (strtolower($field->attributes['code'])) {
						case 'current' :
							if ($field->children[0]->name != 'display_name') $display_name = 'Current';
							else $display_name = $field->children[0]->content;
							$o->openField($display_name);
							echo '<i>'.$owner->getStatusDescription().'</i>';
							break;
						case 'change' :
							$statii = $owner->getAvailableStatii();

							if (!empty($statii)) {
								$statii = array_reverse($statii, true);
								$statii[0] = '';
								$statii = array_reverse($statii, true);
								if ($field->children[0]->name != 'display_name') $display_name = 'Change ?';
								else $display_name = $field->children[0]->content;
								$o->openField($display_name);
								combo_box($prefix.'_change_status', $statii, false, $owner->status);
							}
							break;
						default :
							continue;
							break;
					}

					if ($field->children[1]->name == 'note' && $field->children[1]->content) {
						$o->note($field->children[1]->content);
					}
				}
			}
		}

	}//end paintDetails()


	/**
	* Processes the interface for editing details
	* returns true if there is a change
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function processDetails(&$owner, &$o)
	{
		$prefix = $this->getPrefix($owner);

		if (isset($_POST[$prefix.'_change_status']) && $_POST[$prefix.'_change_status'] > 0) {
			return $owner->processStatusChange($_POST[$prefix.'_change_status']);
		} else {
			return false;
		}
	}//end processDetails()


	/**
	* Paints the interface for managing permissions
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	*/
	function paintPermissions(&$owner, &$o)
	{
		$prefix = $this->getPrefix($owner);
		$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

		$admin_access = $owner->adminAccess();

		foreach ($perms as $title => $perm) {
			$inherited = false;
			$direct    = false;
			$revoked   = false;

			$o->openSection($title.' Access');
			$users    = $owner->getPermission($perm, false);
			$inherits = $owner->getInheritedPermission($perm, array_keys($users));
			$revokes  = $owner->getRevokedPermission($perm);

			$o->openField('Public');
			$public = false;
			foreach ($users as $userid => $locs) {
				if ($userid) continue;
				if (in_array($owner->id, array_keys($locs))) $public = true;
			}
			if ($admin_access) {
				check_box($prefix."_public[$perm]", 1, (($public) ? true : false));
				hidden_field($prefix."_old_public[$perm]", (($public) ? '1' : '0'));
				echo "&nbsp;Give $title access to all users?";
			} else {
				echo (($public) ? 'Yes' : 'No');
			}

			// this is the full list of inherited permissions
			$full_inherits = $inherits;
			foreach ($users as $userid => $locs) {
				foreach ($locs as $locid => $treeid) {
					$full_inherits[$userid][] = $locid;
					$full_inherits[$userid] = array_unique($full_inherits[$userid]);
				}
			}

			$o->openField('Inherited');
			ob_start();
			?>
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<tr>
					<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Revoke ?</b></td><?php
					}
					?>
				</tr>
			<?php

			foreach ($full_inherits as $userid => $locs) {
				if (isset($revokes[$userid])) continue;
				if ($userid) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
					if (!$asset->id || (!is_a($asset, 'user') && !is_a($asset, 'user_group'))) continue;
				}

				$loc_string = '';
				foreach ($locs as $locid) {
					if ($locid == $owner->id) continue;
					$loc = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locid);
					if (!$loc->id) continue;
					$loc_string .= $loc->name.', ';
				}
				$loc_string = trim($loc_string, ', ');

				if (!empty($loc_string)) {
					?>
					<tr>
						<td bgcolor="#F0F0F0" class="sq-form-field"><?php 
						if ($userid) echo $asset->name.' ['.$asset->type()." : $userid] was set at $loc_string";
						else echo "Public access was set at $loc_string";
						?></td><?php
					if ($admin_access) {
						?><td bgcolor="#F0F0F0" align="center" width="100" class="sq-form-field"><?php
						check_box($prefix.'_revoke_'.strtolower($title)."[$userid]")
						?></td><?php
					}
					?></tr><?php
					$inherited = true;
				}
			}

			if (!$inherited) {
				ob_end_clean();
				echo 'None';
			} else {
				ob_end_flush();
				?></table><?php
			}

			$o->openField('Revoked');
			ob_start();
			?>
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<tr>
					<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Reinstate ?</b></td><?php
					}
					?>
				</tr>
			<?php
			foreach ($revokes as $userid => $locs) {
				if ($userid) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
					if (!$asset->id || (!is_a($asset, 'user') && !is_a($asset, 'user_group'))) continue;
				}
				if (in_array($owner->id, $locs)) {
					?>
					<tr>
						<td bgcolor="#F0F0F0" class="sq-form-field"><?php 
						if ($userid) echo $asset->name.' ['.$asset->type()." : $userid]";
						else echo 'Public access';
						?></td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#F0F0F0" align="center" width="100" class="sq-form-field"><?php
						check_box($prefix.'_unrevoke_'.strtolower($title)."[$userid]")
						?></td><?php
					}
					?></tr><?php
					$revoked = true;
				}
			}

			if (!$revoked) {
				ob_end_clean();
				echo 'None';
			} else {
				ob_end_flush();
				?></table><?php
			}

			$o->openField('Direct');
			ob_start();
			?>
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<tr>
					<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Delete ?</b></td><?php
					}
					?>
				</tr>
			<?php
			foreach ($users as $userid => $locs) {
				if (!$userid) continue;
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
				if (!$asset->id || (!is_a($asset, 'user') && !is_a($asset, 'user_group'))) continue;
				if (in_array($owner->id, array_keys($locs))) {
					?>
					<tr>
						<td bgcolor="#F0F0F0" class="sq-form-field"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $userid]";?></td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#F0F0F0" align="center" width="100" class="sq-form-field"><?php
						check_box($prefix.'_delete_'.strtolower($title)."[$userid]")
						?></td><?php
					}
					?></tr><?php
					$direct = true;
				}
			}

			if (!$direct) {
				ob_end_clean();
				echo 'None';
			} else {
				ob_end_flush();
				?></table><?php
			}

			if ($admin_access) {
				$o->openField('New?');
				asset_finder($prefix.'_new_'.strtolower($title), 0, Array('user' => 'D', 'user_group' => 'D'));
			}
		}
		
	}//end paintPermissions()


	/**
	* Processes the interface for managing permissions
	* returns true if there is a change
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function processPermissions(&$owner, &$o)
	{
		// if we dont have admin access, piss off
		if ($owner->id && !$owner->adminAccess(Array(), false)) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the permissions for <i>'.$owner->name.'</i>');
			return false;
		}

		// if we dont have effective admin access, dont process anything
		if (!$owner->adminAccess()) return false;

		$prefix = $this->getPrefix($owner);
		$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);

		foreach ($perms as $title => $perm) {
			// we need to check for write access each time because
			// we may actually be revoking write access and then
			// trying to put it back
			if (!$owner->adminAccess(Array(), false)) continue;

			// grant new access
			if (isset($_POST[$prefix.'_new_'.strtolower($title)])) {
				$user = $_POST[$prefix.'_new_'.strtolower($title)];
				if (!empty($user)) $owner->grantPermission($user, $perm);
			}

			// delete direct access
			if (isset($_POST[$prefix.'_delete_'.strtolower($title)])) {
				foreach ($_POST[$prefix.'_delete_'.strtolower($title)] as $userid => $on) {
					$owner->revokePermission($userid, $perm);
				}
			}

			// unrevoke inherited permissions
			if (isset($_POST[$prefix.'_unrevoke_'.strtolower($title)])) {
				foreach ($_POST[$prefix.'_unrevoke_'.strtolower($title)] as $userid => $on) {
					$owner->reinstateRevokedPermission($userid, $perm);
				}
			}

			// revoke inherited permissions
			if (isset($_POST[$prefix.'_revoke_'.strtolower($title)])) {
				foreach ($_POST[$prefix.'_revoke_'.strtolower($title)] as $userid => $on) {
					$owner->revokeInheritedPermission($userid, $perm);
				}
			}

			// grant/revoke public access?
			if (isset($_POST[$prefix.'_old_public'][$perm])) {
				if (isset($_POST[$prefix.'_public'][$perm])) {
					if($_POST[$prefix.'_old_public'][$perm] === '0') {
						$owner->grantPermission('0', $perm);
					}
				} else if ($_POST[$prefix.'_old_public'][$perm] === '1') {
					$owner->revokePermission('0', $perm);
				}
			}
		}

		return true;

	}//end processPermissions()


	/**
	* Paints the interface for managing workflow
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	*/
	function paintWorkflow(&$owner, &$o)
	{
		$prefix = $this->getPrefix($owner).'_workflow';

		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($owner, true, true, true);

		$inheriting_workflows = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo($wfm->getRunningInheriting()));
		if (!empty($inheriting_workflows)) {
			$o->openSection('Running Workflows Downstream');
			$o->openField('Note');
			echo 'Any changes you make to the workflow will not affect the currently running workflows for the following assets:<ul>';
			foreach ($inheriting_workflows as $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo '<li class="sq-form-field">'.$asset->name.'</li>';
			}
			echo '</ul>';
		}

		$admin_access = false;
		if ($wfm->state == 'running') {
			// cant edit workflow when workflow
			// is currently running
			$admin_access = false;

			$o->openSection('Current Workflow Progress');
			$o->openField('Note');
			echo 'Workflow for this asset is currently in progress. You can not edit the workflow for this asset at this time. The currently running workflows are displayed below followed by the workflow for this asset.';

			foreach ($wfm->current_workflows['complete'] + $wfm->current_workflows['running'] as $workflowid => $workflow) {
				list($assetid, $inheritid) = explode('~', $workflowid);
				$workflow_title = 'Running workflow set at ';
				if ($assetid == $inheritid) {
					$workflow_title .= 'this asset';
				} else {
					$inherit = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inheritid);
					if (!$inherit->id) $workflow_title .= "unknown asset $inheritid";
					else $workflow_title .= $inherit->name;
				}
				$o->openSection($workflow_title);

				foreach ($workflow['steps'] as $stepid => $step_data) {
					$section_title = "Step $stepid";
					if (!$workflow['current_step'] || $workflow['current_step'] > $stepid) {
						$section_title .= ' - COMPLETED';
					}

					$o->openSection($section_title);

					$o->openField('Step Logic');
					if (isset($wfm->_valid_cond_logic[$step_data['logic']])) {
						// a special type of logic statement
						$step_logic = $step_data['logic'];
						$step_logic_num = '';
					} else {
						list($step_logic, $step_logic_num) = explode(' ', $step_data['logic']);
					}

					echo '<i>'.$wfm->_valid_step_logic[$step_logic];
					if ($step_logic_num) echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '')." must be met</i>";

					$o->openField('Conditions');

					$has_conds = false;
					ob_start();
					?>
					<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
						<tr>
							<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
							<td bgcolor="#DDDDDD" class="sq-form-field" width="270"><b>Group Logic</b></td>
							<td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Complete ?</b></td>
						</tr>
					<?php
					foreach ($step_data['conditions'] as $pub => $cond_data) {
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
						if (!$asset->id) continue;
						?>
						<tr>
							<td bgcolor="#F0F0F0" class="sq-form-field"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $pub]";?></td>
							<td bgcolor="#F0F0F0" class="sq-form-field">
						<?php
						if (isset($wfm->_valid_cond_logic[$cond_data['require']])) {
							// a special type of require statement
							$cond_logic = $cond_data['require'];
							$cond_logic_num = '';
						} else {
							list($cond_logic, $cond_logic_num) = explode(' ', $cond_data['require']);
						}

						if (is_a($asset, 'user')) {
							echo '&nbsp;';
						} else {
							echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
							if ($cond_logic_num) echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '')." must approve</i>";
						}

						?></td>

							<td bgcolor="#F0F0F0" align="center" class="sq-form-field">
							<?php
								if (isset($cond_data['complete']) && $cond_data['complete']) {
									echo 'Yes';
								} else {
									// do some extra checking
									if (is_a($asset, 'user')) {
										echo ((in_array($asset->id, $cond_data['published_by'])) ? 'Yes' : '&nbsp;');
									} else {
										if ($cond_logic_num) $user_count = $cond_logic_num;
										else $user_count = count($asset->getChildren(Array('user'), false));
										echo count($cond_data['published_by']).' of '.$user_count;
									}
								}
							?>
							</td>
						</tr>
						
						<?php
						$has_conds = true;
					}

					if (!$has_conds) {
						ob_end_clean();
						echo 'None';
					} else {
						ob_end_flush();
						?></table><?php
					}
				}
			}
		} else {
			$admin_access = $owner->adminAccess();
		}


		//// print the list of inherited workflows ////
		$inherited = false;
		$revoked   = false;

		$o->openSection('Inherited Workflows');
		$users    = $owner->getPermission(SQ_PERMISSION_WORKFLOW, false);
		$inherits = $owner->getInheritedPermission(SQ_PERMISSION_WORKFLOW, array_keys($users));
		$revokes  = $owner->getRevokedPermission(SQ_PERMISSION_WORKFLOW);
		$revokes  = array_keys($revokes);

		// this is the full list of workflows
		$workflows = array_keys($users) + array_keys($inherits);
		
		$o->openField('Inherited');
		ob_start();
		?>
		<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
			<tr>
				<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
				<?php
				if ($admin_access) {
					?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Revoke ?</b></td><?php
				}
				?>
			</tr>
		<?php

		foreach ($workflows as $assetid) {
			if ($assetid == $owner->id) continue;
			if (!empty($revokes) && in_array($assetid, $revokes)) continue;
			
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (!$asset->id) continue;

			?>
			<tr>
				<td bgcolor="#F0F0F0" class="sq-form-field"><?php 
				echo 'Workflow inherited from '.$asset->name.' ['.$asset->type()." : $assetid]";
				?></td><?php
			if ($admin_access) {
				?><td bgcolor="#F0F0F0" align="center" width="100" class="sq-form-field"><?php
				check_box($prefix."_revoke[$assetid]")
				?></td><?php
			}
			?></tr><?php
			$inherited = true;
		}
		

		if (!$inherited) {
			ob_end_clean();
			echo 'None';
		} else {
			ob_end_flush();
			?></table><?php
		}

		$o->openField('Revoked');
		ob_start();
		?>
		<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
			<tr>
				<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
				<?php
				if ($admin_access) {
					?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Reinstate ?</b></td><?php
				}	
				?>
			</tr>
		<?php

		if (!empty($revokes)) {
			foreach ($revokes as $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$asset->id) continue;
				?>
				<tr>
					<td bgcolor="#F0F0F0" class="sq-form-field"><?php 
					echo 'Workflow inherited from '.$asset->name.' ['.$asset->type()." : $assetid]";
					?></td>
				<?php
				if ($admin_access) {
					?><td bgcolor="#F0F0F0" align="center" width="100" class="sq-form-field"><?php
					check_box($prefix."_unrevoke[$assetid]")
					?></td><?php
				}
				?></tr><?php
				$revoked = true;
			}
		}

		if (!$revoked) {
			ob_end_clean();
			echo 'None';
		} else {
			ob_end_flush();
			?></table><?php
		}


		//// now print the direct workflow for this asset ////
		$o->openSection('Workflow set for this asset');
		foreach ($wfm->steps as $stepid => $step_data) {
			$o->openSection("Step $stepid");
			
			if ($admin_access) {
				$o->openField('Delete?');
				check_box($prefix.'_steps['.$stepid.'][delete]');
				echo '&nbsp; Delete this step and all its conditions?';
			}

			$o->openField('Step Logic');
			if (isset($wfm->_valid_cond_logic[$step_data['logic']])) {
				// a special type of logic statement
				$step_logic = $step_data['logic'];
				$step_logic_num = '';
			} else {
				list($step_logic, $step_logic_num) = explode(' ', $step_data['logic']);
			}

			if (!$admin_access) {
				echo '<i>'.$wfm->_valid_step_logic[$step_logic];
				if ($step_logic_num) echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '')." must be met</i>";
			} else {
				combo_box($prefix.'_steps['.$stepid.'][logic]', $wfm->_valid_step_logic, false, $step_logic);
				echo '&nbsp;';
				text_box($prefix.'_steps['.$stepid.'][logic_num]', $step_logic_num, 4);
				echo ' condition(s) must be met in this step';
			}

			$o->openField('Conditions');

			$has_conds = false;
			ob_start();
			?>
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<tr>
					<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
					<td bgcolor="#DDDDDD" class="sq-form-field" width="270"><b>Group Logic</b></td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#DDDDDD" align="center" width="70" class="sq-form-field"><b>Delete ?</b></td><?php
					}
					?>
				</tr>
			<?php
			foreach ($step_data['conditions'] as $pub => $cond_data) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
				if (!$asset->id) continue;
				?>
				<tr>
					<td bgcolor="#F0F0F0" class="sq-form-field"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $pub]";?></td>
					<td bgcolor="#F0F0F0" class="sq-form-field">
				<?php
				if (isset($wfm->_valid_cond_logic[$cond_data['require']])) {
					// a special type of require statement
					$cond_logic = $cond_data['require'];
					$cond_logic_num = '';
				} else {
					list($cond_logic, $cond_logic_num) = explode(' ', $cond_data['require']);
				}
				if (!$admin_access) {
					if (is_a($asset, 'user')) {
						echo '&nbsp;';
					} else {
						echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
						if ($cond_logic_num) echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '')." must approve</i>";
					}
				} else {
					if (is_a($asset, 'user')) {
						hidden_field($prefix.'_steps['.$stepid.'][conditions]['.$pub.'][logic]', '>=');
						hidden_field($prefix.'_steps['.$stepid.'][conditions]['.$pub.'][logic_num]', '1');
					} else {
						combo_box($prefix.'_steps['.$stepid.'][conditions]['.$pub.'][logic]', $wfm->_valid_cond_logic, false, $cond_logic);
						echo '&nbsp;';
						text_box($prefix.'_steps['.$stepid.'][conditions]['.$pub.'][logic_num]', $cond_logic_num, 4);
						echo ' member(s) must approve';
					}
				}
				?></td><?php

				if ($admin_access) {
					?><td bgcolor="#F0F0F0" align="center" class="sq-form-field"><?php
					check_box($prefix.'_steps['.$stepid.'][conditions]['.$pub.'][delete]')
					?></td><?php
				}
				?></tr><?php
				$has_conds = true;
			}

			if (!$has_conds) {
				ob_end_clean();
				echo 'None';
			} else {
				ob_end_flush();
				?></table><?php
			}

			if ($admin_access) {
				$o->openField('New Condition?');
				asset_finder($prefix.'_steps['.$stepid.'][new_condition]', 0, Array('user' => 'D', 'user_group' => 'D'));
			}
		}

		if ($admin_access) {
			$o->openSection("Create New Step");
			$o->openField('');
			check_box($prefix.'_create_step');
			echo '&nbsp; Create a new empty step?';
		}

	}//end paintWorkflow()


	/**
	* Processes the interface for managing workflow
	* returns true if there is a change
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processWorkflow(&$owner, &$o)
	{
		// if we dont have admin access, piss off
		if ($owner->id && !$owner->adminAccess(Array(), false)) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the workflow of <i>'.$owner->name.'</i>');
			return false;
		}

		// if we dont have effective admin access, dont process anything
		if (!$owner->adminAccess()) return false;

		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($owner);
		if ($wfm->state == 'running') return false;

		$prefix = $this->getPrefix($owner).'_workflow';
		
		// unrevoke inherited workflow
		if (isset($_POST[$prefix.'_unrevoke'])) {
			foreach ($_POST[$prefix.'_unrevoke'] as $assetid => $on) {
				$owner->reinstateRevokedPermission($assetid, SQ_PERMISSION_WORKFLOW);
			}
		}

		// revoke inherited workflow
		if (isset($_POST[$prefix.'_revoke'])) {
			foreach ($_POST[$prefix.'_revoke'] as $assetid => $on) {
				$owner->revokeInheritedPermission($assetid, SQ_PERMISSION_WORKFLOW);
			}
		}

		// create a new step?
		if (isset($_POST[$prefix.'_create_step'])) {
			$new_stepid = $wfm->addStep();
		}
	
		if (isset($_POST[$prefix.'_steps'])) {
			$deleted_steps = 0;
			foreach ($_POST[$prefix.'_steps'] as $stepid => $step_data) {
				$effective_stepid = $stepid - $deleted_steps;
				if (isset($step_data['delete'])) {
					$wfm->deleteStep($effective_stepid);
					$deleted_steps++;
					continue;
				}

				if ($step_data['new_condition'] > 0) {
					$wfm->addCondition($effective_stepid, $step_data['new_condition']);
				}

				if ($step_data['logic'] == 'all') $step_data['logic_num'] = '0';
				$wfm->setStepLogic($effective_stepid, $step_data['logic'], $step_data['logic_num']);

				if (isset($step_data['conditions'])) {
					foreach ($step_data['conditions'] as $pub => $cond_data) {
						if (isset($cond_data['delete'])) {
							$wfm->deleteCondition($effective_stepid, $pub);
							continue;
						}

						if ($cond_data['logic'] == 'all') $cond_data['logic_num'] = '0';
						$wfm->setConditionLogic($effective_stepid, $pub, $cond_data['logic'], $cond_data['logic_num']);
					}
				}
			}
		}

		return $wfm->save();

	}//end processWorkflow()


	/**
	* Paints the interface for viewing dependants details
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintDependants(&$owner, &$o)
	{
		$o->openSection('Current Lock Status');
		$o->openField('&nbsp;');

		$lock_message = '';
		$lock = $owner->getLockInfo();
		if (!empty($lock)) {
			// this asset is currently locked
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);

			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			$expires_in = easy_time_total(($lock['expires'] - time()));
			if (!$expires_in) $expires_in = '1 second';

			$lock_message = '<span style="color:red"><b>[LOCKED]</b></span> by "'.$user->name.'". Expires in '.$expires_in.'';
		} else {
			$lock_message = '<span style="color:green"><b>[UNLOCKED]</b></span>';
		}

		?>
		<table cellpadding="0" cellspacing="0" border="0">
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					<?php echo '<b>'.$owner->name.'</b> ('.$owner->getStatusDescription().') :: '.$lock_message; ?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data"><?php $this->_recursePaintDependants($owner, $o); ?></td>
			</tr>
		</table>
		<?php
		
		return true;
	}//end paintLocking()


	/**
	* Paints an asset dependancy tree with stalks and dependant info
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @return void
	* @access public
	*/
	function _recursePaintDependants(&$asset, &$o)
	{
		?>
		<table cellpadding="0" cellspacing="0" border="0">
		<?php
		$dependant_links = $asset->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		$num_kids = count($dependant_links);
		for ($i = 0; $i < $num_kids; $i++) {
			$dep_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_links[$i]['minorid']);
			$end = ($i == $num_kids - 1);
			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';

			$lock_message = '';
			$lock = $dep_asset->getLockInfo();
			if (!empty($lock)) {
				// this asset is currently locked
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
				$editing = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_asset']);

				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$expires_in = easy_time_total(($lock['expires'] - time()));
				if (!$expires_in) $expires_in = '1 second';

				$lock_message = '<span style="color:red"><b>[LOCKED]</b></span> by "'.$user->name.'" at "'.$editing->name.'". Expires in '.$expires_in.'';
			} else {
				$lock_message = '<span style="color:green"><b>[UNLOCKED]</b></span>';
			}

			?>
			<tr>
				<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					<a href="<?php echo $dep_asset->getBackendHref('dependants'); ?>"><b><?php echo $dep_asset->name; ?></b></a>&nbsp;<?php echo '('.$dep_asset->getStatusDescription().') :: '.$lock_message; ?>
				</td>
			</tr>
			<?php
			$child_deps = $dep_asset->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);
			if (!empty($child_deps)) {
				?>
				<tr>
					<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
					<td class="sq-backend-data"><?php $this->_recursePaintDependants($dep_asset, $o); ?></td>
				</tr>
				<?php
			}
		}
		?>
		</table>
		<?php
	}


	/**
	* Processes the interface for viewing depedants
	* Note that this function doesnt do anything
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processDependants(&$owner, &$o)
	{
		return false;
	}//end processLocking()


	/**
	* Paints the interface for viewing links to/from this asset
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintLinking(&$owner, &$o)
	{
		// prefix for all the form vars
		$prefix = $this->getPrefix($owner);

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$o->openSection('Assets linking to "'.$owner->name.'"');
		$o->openField('&nbsp;');
		echo '<i>NOTE: <b>TYPE_1</b> links are displayed in both the frontend and backend navigation but <b>TYPE_2</b> links are only displayed in the backend navigation.</i><br/><br/>';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = Array();
		$asset_links = $owner->getLinks(SQ_LINK_ALL, '', true, 'minor');
		$editable_link_types = Array(SQ_LINK_TYPE_1 => link_type_name(SQ_LINK_TYPE_1),
									 SQ_LINK_TYPE_2 => link_type_name(SQ_LINK_TYPE_2)
									);
		
		foreach ($asset_links as $link) $assetids[$link['majorid']] = Array();

		$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
		$sql = 'SELECT cl.minorid, pt.treeid, a.name
				FROM sq_asset_link cl INNER JOIN sq_asset_link_tree ct ON cl.linkid = ct.linkid,
					 sq_asset_link pl INNER JOIN sq_asset_link_tree pt ON pl.linkid = pt.linkid 
					 INNER JOIN sq_asset a ON a.assetid = pl.minorid ';
		$sql .= 'WHERE cl.minorid IN ('.implode(', ', array_keys($assetids)).')
				  AND ct.treeid LIKE '.$concat.'
				  AND pt.treeid < ct.treeid';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		foreach ($result as $assetid => $treeids) $assetids[$assetid] = $treeids;
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));

		$done_something = false;
		foreach ($asset_links as $link) {
			$majorid = $link['majorid'];
			
			// dont even pretend that the root folder is going to
			// care that it is linking to this asset!!
			if ($majorid == 1) continue;

			$can_edit = false;
			if ($owner->writeAccess() && in_array($link['link_type'], array_keys($editable_link_types))) $can_edit = true;

			$lineage_prefix = '';
			if (!$can_edit) $lineage_prefix = '[ '.link_type_name($link['link_type']).' ] ';
			$lineage_prefix .= 'Root Folder > ';
			$lineage_suffix = '<b>'.$asset_info[$majorid]['name'].'</b><br/>';

			$asset_lineage = $lineage_prefix;
			$last_treeid = '';
			
			foreach ($assetids[$majorid] as $tree_data) {
				if ($tree_data[0] < $last_treeid) {
					// starting a new lineage
					$asset_lineage .= $lineage_suffix;
					echo $asset_lineage;
					$asset_lineage = $lineage_prefix;
				}
				if ($tree_data[1] == '/') continue;
				$last_treeid = $tree_data[0];
				$asset_lineage .= $tree_data[1].' > ';
			}
			$asset_lineage .= $lineage_suffix;
			$done_something = true;
			
			if ($can_edit) {
				combo_box($prefix.'_link_type['.$link['linkid'].']', $editable_link_types, false, $link['link_type']);
				echo '&nbsp;';
			}
			echo $asset_lineage;

		}//end foreach link

		if (!$done_something) echo 'None';
	}//end paintLinking()


	/**
	* Processes the interface for viewing links to/from this asset
	* Note that this function doesnt do anything
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processLinking(&$owner, &$o)
	{
		// if we dont have admin access, piss off
		if ($owner->id && !$owner->writeAccess(Array(), false)) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the links of <i>'.$owner->name.'</i>');
			return false;
		}

		// if we dont have effective write access, dont process anything
		if (!$owner->writeAccess()) return false;

		// prefix for all the form vars
		$prefix = $this->getPrefix($owner);

		$done_something = false;
		$asset_links = $owner->getLinks(SQ_LINK_ALL, '', true, 'minor');

		foreach ($asset_links as $link) {
			if (!isset($_POST[$prefix.'_link_type'][$link['linkid']])) continue;
			$new_type = $_POST[$prefix.'_link_type'][$link['linkid']];
			if ($link['link_type'] == $new_type) continue;

			$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid']);
			if (!$parent->id) continue;
			if (!$parent->_updateLink($link['linkid'], $new_type)) continue;

			$done_something = true;
		}

		return $done_something;
	}//end processLinking()


	/**
	* Takes the requested filename and searches back up the directory's to find the matching file
	* closest to the type of the asset that we are creating
	*
	* @param string	$file_name	the file to find
	*
	* @return string
	* @access private
	*/
	function _findFile($file_name)
	{
		$file_name = 'edit_interface_'.$file_name.'.xml';

		$file = '';
		for($i = 0; $i < count($this->_dirs); $i++) {
			if (file_exists($this->_dirs[$i].'/'.$file_name)) {
				$file = $this->_dirs[$i].'/'.$file_name;
				break;
			}
		}

		if(!$file) {
			trigger_error($file_name.' not found for Asset Type "'.$this->_type_code.'"', E_USER_WARNING);
			return false;
		}

		return $file;
	}// end _findFile()


	/**
	* Returns the Sections and fields that are to be used in a create interface
	* and the screen that these belong to
	*
	* @param boolean	$painting	if we are painting or processing
	*
	* @return Array
	* @access private
	*/
	function _getCreateSections($painting)
	{
		$create_sections = Array();

		if (!($create_file = $this->_findFile('create'))) return $create_sections;

		$input = new XML_Tree($create_file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return $create_sections;
		}

		// Loop over the sections
		for($i = 0; $i < count($root->children); $i++) {
			$section = &$root->children[$i];
			if ($section->name != 'section') continue;

			$create_sections[$section->attributes['name']] = Array();
			// Loop over the fields
			for($j = 0; $j < count($section->children); $j++) {
				$field = &$section->children[$j];
				if ($field->name != 'field') continue;
				
				if (!empty($field->attributes['attribute'])) {
					$create_sections[$section->attributes['name']][] = $field->attributes['attribute'];
				} else if ($field->children[1]->name == 'function_call') {
					$fn_node = &$field->children[1]->children[(($painting) ? 0 : 1)];
					$create_sections[$section->attributes['name']][] = $fn_node->attributes['name'];
				}// end if

			}// end for

		}// end for

		return Array($create_sections, $root->attributes['screen']);
	}// end _getCreateSections()

}//end class


?>
