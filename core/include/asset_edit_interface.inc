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

		$asset_dir = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getInfo($this->_type_code, 'dir');
		$class_name = $this->_type_code.'_edit_fns';
		require_once $asset_dir.'/'.$class_name.'.inc';
		$this->_edit_fns = new $class_name();

		$this->_dirs = Array($asset_dir);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->_type_code, false);
		foreach($parents as $parent_type) {
			$this->_dirs[] = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getInfo($parent_type, 'dir');
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
	* Paint's out the interface for the currently selected frame
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$creating	whether we are creating or editing
	*
	* @access  public
	*/
	function paint(&$owner, &$o, $creating)
	{
		// if we dont have read access, piss off
		if ($owner->id && !$owner->readAccess()) {
			include SQ_INCLUDE_PATH.'/login.inc';
			print_login('Login', 'You do not have permission to access <i>'.$owner->name.'</i>');
			return;
		}

		// do we have write access?
		$write_access = $owner->writeAccess();

		// if there are no screens or tabs, piss off
		if (!count($this->_screens) && empty($owner->_tabs)) return;

		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';

		// add all the screens to the combo, but only if we aren't creating the object
		if (!$creating) {
			for(reset($this->_screens); null !== ($code = key($this->_screens)); next($this->_screens)) {
				$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
			}// end for
		}

		$create_sections = Array();
		$active_screen = (empty($_GET['asset_ei_screen'])) ? $this->_default_screen : $_GET['asset_ei_screen'];

		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_screen) = $this->_getCreateSections();
			if (!count($create_sections)) return;
		}// end if creating

		$active_screen = (empty($_GET['asset_ei_screen'])) ? $this->_default_screen : $_GET['asset_ei_screen'];

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('asset_ei_screen', $active_screen);

		// add tabs for the asset if not creating
		if (!$creating) {
			foreach ($owner->_tabs as $code_name => $tab_data) {
				$tab_data['url'] = $current_location.'asset_ei_screen='.rawurlencode($code_name);
				$o->addTab($code_name, $tab_data);
			}
		}

		//check the active screen for special system defined tabs
		switch ($active_screen) {
			case 'permissions' :
				$o->setHeading($GLOBALS['SQ_SYSTEM']->am->getInfo($this->_type_code, 'name').' - '.$owner->_tabs[$active_screen]['name'], $owner->_tabs[$active_screen]['icon']);
				$o->setCurrentTab('permissions');
				$this->paintPermissions($owner, $o);
				break;
			case 'workflow' :
				$o->setHeading($GLOBALS['SQ_SYSTEM']->am->getInfo($this->_type_code, 'name').' - '.$owner->_tabs[$active_screen]['name'], $owner->_tabs[$active_screen]['icon']);
				$o->setCurrentTab('workflow');
				$this->paintWorkflow($owner, $o);
				break;
			default :
				// must be a normal screen
				if (empty($this->_screens[$active_screen])) {
					trigger_error('Active Screen ("'.$active_screen.'") unknown', E_USER_WARNING);
					return;
				}

				// try and find the file
				if(!($file = $this->_findFile('screen_'.$active_screen))) return;

				$o->setHeading($GLOBALS['SQ_SYSTEM']->am->getInfo($this->_type_code, 'name').' - '.$this->_screens[$active_screen]['name'], $this->_screens[$active_screen]['icon']);

				$input = new XML_Tree($file);
				$root  = &$input->getTreeFromFile();
				if (PEAR::isError($root)) {
					trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
					return;
				}

				// prefix for all the form vars
				$prefix = $owner->type().'_'.$owner->id;

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
								$o->note($field->children[1]->content);
							}

						}// end for fields

					}// end if section uses function call

				}// end for
				break;
		}//end switch

		
		if ($owner->writeAccess()) {
			$o->openSection();
			$o->commitButton('Commit');
		}

	}// end paint();


	/**
	* Processes the results of the form posting, returns an array of the attribute names that
	* where processed
	*
	* @param object Asset	$owner		the asset whose interface we are painting
	* @param boolean		$creating	whether we are creating or editing
	*
	* @return Array()
	* @access public
	*/
	function process(&$owner, $creating)
	{
		// if we dont have read access, piss off
		if ($owner->id && !$owner->writeAccess()) {
			include SQ_INCLUDE_PATH.'/login.inc';
			print_login('Login', 'You do not have permission to edit <i>'.$owner->name.'</i>');
			return;
		}

		// if there are no screens or tabs, piss off
		if (!count($this->_screens) && empty($owner->_tabs)) return;

		$processed = Array();
		$create_sections = Array();
		$active_screen = (empty($_GET['asset_ei_screen'])) ? $this->_default_screen : $_GET['asset_ei_screen'];

		// check the active screen for special system defined tabs
		switch ($active_screen) {
			case 'permissions' :
				$this->processPermissions($owner);
				break;
			case 'workflow' :
				$this->processWorkflow($owner);
				break;
			default :
				// must be a normal screen

				// if we are creating open the create file to see which fields we are supposed to print out
				if ($creating) {
					list($create_sections, $active_screen) = $this->_getCreateSections();
					if (!count($create_sections)) return $processed;
				}// end if create

				if (empty($_GET['asset_ei_screen']) || empty($this->_screens[$_GET['asset_ei_screen']])) {
					trigger_error('Active Screen unknown unable to process', E_USER_WARNING);
					return $processed;
				}

				$file = '';
				for($i = 0; $i < count($this->_dirs); $i++) {
					if (file_exists($this->_dirs[$i].'/edit_interface_screen_'.$active_screen.'.xml')) {
						$file = $this->_dirs[$i].'/edit_interface_screen_'.$active_screen.'.xml';
						break;
					}
				}

				if(!$file) {
					trigger_error('edit_interface_screens.xml not found for Asset Type "'.$this->_type_code.'"', E_USER_WARNING);
					return $processed;
				}

				$input = new XML_Tree($file);
				$root  = &$input->getTreeFromFile();
				if (PEAR::isError($root)) {
					trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
					return $processed;
				}

				// prefix for all the form vars
				$prefix = $owner->type().'_'.$owner->id;
				// Loop over the sections
				for($i = 0; $i < count($root->children); $i++) {
					$section = &$root->children[$i];
					// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
					if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;

					if (!($num_fields = count($section->children))) continue;
					if ($section->children[0]->name == 'function_call') {

						$process_node = &$section->children[0]->children[1];

						// if there is nothing declared there is nothing for us to do
						if (empty($process_node->attributes['name'])) {
							continue;
						// There is something declared but the bloody thing doesn't exist - inform the Idiot
						} elseif (!method_exists($this->_edit_fns, $process_node->attributes['name'])) {
							trigger_error('Function not found for processing section "'.$section->attributes['name'].'"', E_USER_WARNING);
						} else {
							$this->_edit_fns->{$process_node->attributes['name']}($owner, $prefix);
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

								$attr->process($prefix);
								if ($attr->processed) $processed[] = $attr->name;

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
									$this->_edit_fns->{$process_node->attributes['name']}($owner, $prefix);
								}

							}// end if

						}// end for fields

					}// end if section funcion call

				}// end for
				break;

		}//end switch
		
		return $processed;

	}// end process();


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
		$prefix = $owner->type().'_'.$owner->id;
		$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

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
			if ($owner->writeAccess()) {
				check_box($prefix."_public[$perm]", 1, (($public) ? true : false));
				hidden_field($prefix."_old_public[$perm]", (($public) ? '1' : '0'));
				echo "&nbsp;Give $title access to all users?";
			} else {
				echo (($public) ? 'Yes' : 'No');
			}

			$o->openField('Inherited');
			ob_start();
			?>
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<tr>
					<td bgcolor="#DDDDDD" class="sq-form-field">&nbsp;</td>
					<?php
					if ($owner->writeAccess()) {
						?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-form-field"><b>Revoke ?</b></td><?php
					}
					?>
				</tr>
			<?php

			foreach ($inherits + $users as $userid => $locs) {
				if (isset($revokes[$userid])) continue;
				if ($userid) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
					if (!$asset->id || (!is_a($asset, 'user') && !is_a($asset, 'user_group'))) continue;
				}

				$loc_string = '';
				foreach ($locs as $locid => $extras) {
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
					if ($owner->writeAccess()) {
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
					if ($owner->writeAccess()) {
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
				if (in_array($owner->id, array_keys($locs))) {
					?>
					<tr>
						<td bgcolor="#F0F0F0" class="sq-form-field"><?php 
						if ($userid) echo $asset->name.' ['.$asset->type()." : $userid]";
						else echo 'Public access';
						?></td>
					<?php
					if ($owner->writeAccess()) {
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
					if ($owner->writeAccess()) {
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
					if ($owner->writeAccess()) {
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
			

			if ($owner->writeAccess()) {
				$o->openField('New?');
				asset_finder($prefix.'_new_'.strtolower($title), 0, Array('user' => 'D', 'user_group' => 'D'));
			}
		}
		
	}//end paintPermissions()


	/**
	* Processes the interface for managing permissions
	*
	* @param object	Asset	$owner	the asset whose interface we are painting
	*
	* @access public
	*/
	function processPermissions(&$owner)
	{
		// need to have write access to do any of this
		if ($owner->writeAccess()) {
			$prefix = $owner->type().'_'.$owner->id;
			$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);

			foreach ($perms as $title => $perm) {
				// we need to check for write access each time because
				// we may actually be revoking write access and then
				// trying to put it back
				if (!$owner->writeAccess()) continue;

				// grant new access
				$user = $_POST[$prefix.'_new_'.strtolower($title)];
				if (!empty($user)) $owner->grantPermission($user, $perm);

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
				if (isset($_POST[$prefix.'_public'][$perm])) {
					if($_POST[$prefix.'_old_public'][$perm] === '0') {
						$owner->grantPermission('0', $perm);
					}
				} else if ($_POST[$prefix.'_old_public'][$perm] === '1') {
					$owner->revokePermission('0', $perm);
				}
			}
		}

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
		$prefix = $owner->type().'_'.$owner->id.'_permissions';

		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($owner, true, true, true);

		foreach ($wfm->_steps as $stepid => $step_data) {
			$o->openSection("Step $stepid");
			
			if ($owner->writeAccess()) {
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

			if (!$owner->writeAccess()) {
				echo '<i>'.$wfm->_valid_step_logic[$step_data['logic']].'</i>';
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
					if ($owner->writeAccess()) {
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
				if (!$owner->writeAccess()) {
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

				if ($owner->writeAccess()) {
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

			if ($owner->writeAccess()) {
				$o->openField('New Condition?');
				asset_finder($prefix.'_steps['.$stepid.'][new_condition]', 0, Array('user' => 'D', 'user_group' => 'D'));
			}
		}

		if ($owner->writeAccess()) {
			$o->openSection("Create New Step");
			$o->openField('');
			check_box($prefix.'_create_step');
			echo '&nbsp; Create a new empty step?';
		}
		
		//$wfm->start();
		//bam($wfm->whoCanPublish());

		//bam($wfm->_current_workflows);
		//$wfm->recordPublish('49');
		//$wfm->recordPublish('4');
		//$wfm->recordPublish('16');
		//$wfm->recordPublish('4');
		//bam($wfm->_current_workflows);

		//bam($wfm->whoCanPublish());
		//$wfm->save();

	}//end paintWorkflow()


	/**
	* Processes the interface for managing workflow
	*
	* @param object	Asset	$owner	the asset whose interface we are painting
	*
	* @access public
	*/
	function processWorkflow(&$owner)
	{
		$prefix = $owner->type().'_'.$owner->id.'_permissions';

		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($owner, true, false, false);

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

		$wfm->save();

	}//end processWorkflow()



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
	* @return Array
	* @access private
	*/
	function _getCreateSections()
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
					$process_node = &$field->children[1]->children[0];
					$create_sections[$section->attributes['name']][] = $process_node->attributes['name'];
				}// end if

			}// end for

		}// end for

		return Array($create_sections, $root->attributes['screen']);

	}// end _getCreateSections()

}//end class


?>
