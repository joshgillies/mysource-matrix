<?php

require_once 'XML/Tree.php';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/*
* Used by both paint() and process() to determine the access that the current user
* has - set in _getAccess()
*/
define('SQ_EI_READ', 1);
define('SQ_EI_WRITE', 2);

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
		if (empty($_GET['asset_ei_screen'])) {
			if (isset($_GET['sq_from_frontend']) && !empty($this->_default_screen)) {
				$active_screen = $this->_default_screen;
			} else {
				$active_screen = 'details';
			}
			$_GET['asset_ei_screen'] = $active_screen;
		} else {
			$active_screen = $_GET['asset_ei_screen'];
		}

		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_screen) = $this->_getCreateSections(true);
			if (!count($create_sections)) return;
		}

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('asset_ei_screen', $active_screen);

		// do we have write access?
		$write_access      = $owner->writeAccess();
		$full_write_access = $owner->writeAccess(Array(), false);

		// do we print the commit button (ie. have we printed an editable interface?)
		$print_commit_button = false;

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

				if (SQ_ROLLBACK_VIEW) $now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
				else $now = time();

				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$expires_in = easy_time_total(($lock['expires'] - $now), true);
				if (!$expires_in) $expires_in = '1 second';

				echo '<p class="sq-backend-data">"'.$owner->name.'" is currently locked for editing by user "'.$user->name.'" at "'.$editing->name.'". The lock is due to expire in '.$expires_in.'</p>';

				if ($GLOBALS['SQ_SYSTEM']->currentUser($user)) {

					submit_button('sq_manual_release_lock', 'Release Lock');
				} else {
					// does the current user has a high
					// enough level of access to forceably acquire the lock
					if ($owner->canForceablyAcquireLock()) {
						echo '<p class="sq-backend-data">You can forceably acquire this lock, which will remove the lock from "'.$user->name.'" and give the lock to yourself. Any changes currently being made by "'.$user->name.'" will be lost if this lock is forceably acquired.</p>';

						// they aren't going to do this without a fight though
						// they'll have to go thorough our popup window first
						$popup_url = '?SQ_BACKEND_PAGE=main&backend_section=am&am_section=forceably_acquire_lock&assetid='.$owner->id;

						hidden_field('sq_acquire_by_force', '0');
						normal_button('sq_forceably_acquire_lock', 'Acquire Lock', 'window.open(\''.$popup_url.'\', \'sq_forceably_acquire_lock_popup\', \'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300\');');

					}
				}
			} else if ($full_write_access) {
				$write_access = false;
				hidden_field('sq_acquire_by_force', '0');
				submit_button('sq_acquire_lock', 'Acquire Lock');
			} else {
				echo 'You do not have access to edit "'.$owner->name.'".';
			}

			$o->closeSection();

		}// end if

		if (!$creating) {
			$page_heading = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name').
							' - '.$owner->name.' (version '.$owner->version.')';
		} else {
			$page_heading = 'Create new '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name');
		}

		// check the active screen for special system defined static screens
		$file = null;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {
			$o->setHeading($page_heading);

			// if we are not creating, paint the system defined bit of this tab
			if (!$creating) {
				$paint_function = 'paint'.ucfirst($active_screen);
				$print_commit_button = $this->$paint_function($owner, $o);
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

			$o->setHeading($page_heading.' - '.$this->_screens[$active_screen]['name'], $this->_screens[$active_screen]['icon']);
		}// end if

		if ($file) {
			$input = new XML_Tree($file);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
				return;
			}

			// prefix for all the form vars
			$prefix = $owner->getPrefix();

			// Loop over the sections
			for($i = 0; $i < count($root->children); $i++) {
				$section = &$root->children[$i];
				// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
				if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;

				$section_access = $this->_getAccess($owner, $section);
				if (!($section_access & SQ_EI_READ)) continue;

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

						if ($field->name != 'field') {
							trigger_error('Expected "field" tag in section, found "'.$field->name.'"', E_USER_WARNING);
							continue;
						}

						if ($field->children[0]->name != 'display_name') {
							trigger_error('Display Name for "'.$field->attributes['attribute'].'" not found', E_USER_WARNING);
							continue;
						}

						$field_access = $this->_getAccess($owner, $field, $section_access);
						if (!($field_access & SQ_EI_READ)) continue;

						if (!isset($field->attributes['format'])) $field->attributes['format'] = '';

						if (!empty($field->attributes['attribute'])) {
							// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
							if ($field->name != 'field' || ($creating && (empty($field->attributes['attribute']) || !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']])))) continue;

							$o->openField($field->children[0]->content, $field->attributes['format']);

							$attr = &$owner->getAttribute($field->attributes['attribute']);
							if (is_null($attr)) continue;
							if (!$attr->setEditParams($field->children[2])) continue;
							$attr->paint($prefix, !($field_access & SQ_EI_WRITE));

							if ($field->children[1]->name == 'note' && $field->children[1]->content) {
								$field->children[1]->content = str_replace('%description%', $attr->description, $field->children[1]->content);
							}

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

				$o->closeSection();

			}// end for

		}// end if $file


		$statii = Array();
		if (!$creating && !$write_access && !$print_commit_button && $active_screen == 'details') $statii = $owner->getAvailableStatii();

		// if the user has write access, has the ability to alter the status of the asset,
		// or we have printed an editable interface on a static screen - show the commit button
		if ($write_access || $print_commit_button || !empty($statii)) {
			$o->commitButton('Commit', $force_unlock_on_commit);
		}

	}// end paint()


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

			// save the status for later so we can see if it changed
			$old_status = $owner->status;

			// process the system defined bit of this tab if we
			// are not creating the asset
			if (!$creating) {
				$process_function = 'process'.ucfirst($active_screen);
				if ($this->$process_function($owner, $o)) $ret_val = true;
			}

			if ($old_status != $owner->status) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				// the status changed, so they might want to know
				// how the operation went...
				?>
				<script language="Javascript" type="text/javascript">
					if (confirm("The status of '<?php echo $owner->name; ?>' has changed from <?php echo getStatusDescription($old_status); ?> to <?php echo getStatusDescription($owner->status); ?>. Do you want to view a dependant links report to view the status of the dependant assets below '<?php echo $owner->name; ?>'?")) {
						document.location = '<?php echo $owner->getBackendHref('dependants'); ?>';
					}
				</script>
				<?php
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
			$prefix = $owner->getPrefix();

			// Loop over the sections
			for($i = 0; $i < count($root->children); $i++) {
				$section = &$root->children[$i];
				// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
				if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;
				if (!($num_fields = count($section->children))) continue;

				$section_access = $this->_getAccess($owner, $section);
				if (!($section_access & SQ_EI_READ)) continue;

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

						$field_access = $this->_getAccess($owner, $field, $section_access);
						if (!($field_access & SQ_EI_WRITE)) continue;

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
	* Reads the write_access and read_access attributes of the passed node and returns
	* at bitmask that can be used with the SQ_EI_READ and SQ_EI_WRITE constants
	*
	* @param object Asset			$owner			the asset whose interface we are painting
	* @param object	XML_Tree_Node	$node			the xml node to check for 'write_access' and 'read_access' attrs
	* @param int					$inherit_access	used if an access is not specified
	*
	* @return int
	* @access private
	*/
	function _getAccess(&$owner, $node, $inherit_access=null)
	{
		$access = 0;

		// got to have writeAccess at least (this checks for locks as well)
		if ($owner->writeAccess()) {
			// if there is nothing specified, and if an inherited access was specified we must use that
			if (empty($node->attributes['write_access']) && !is_null($inherit_access)) {
					if ($inherit_access & SQ_EI_WRITE)  $access |= SQ_EI_WRITE;

			// otherwise attempt to find write access
			} else {
				$wa = (empty($node->attributes['write_access'])) ? 'write' : $node->attributes['write_access'];
				switch($wa) {
					case 'root' :
						if ($GLOBALS['SQ_SYSTEM']->userRoot()) $access |= SQ_EI_WRITE;
						break;
					case 'sys_admin' :
						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) $access |= SQ_EI_WRITE;
						break;
					case 'admin' :
						if ($owner->adminAccess()) $access |= SQ_EI_WRITE;
						break;
					case 'write' :
						$access |= SQ_EI_WRITE; // this was checked above
						break;

					case 'none' :
						// meaning that this attribute is not to be written
						break;

				}// end switch
			}// end if
		}// end if

		// if they have write access they have read access by definition
		if ($access & SQ_EI_WRITE) {
			$access |= SQ_EI_READ;
		} else {

			// if there is nothing specified, and if an inherited access was specified we must use that
			if (empty($node->attributes['read_access']) && !is_null($inherit_access)) {
					if ($inherit_access & SQ_EI_READ)  $access |= SQ_EI_READ;

			// otherwise attempt to find read access
			} else {
				$ra = (empty($node->attributes['read_access'])) ? 'read' : $node->attributes['read_access'];
				switch($ra) {
					case 'root' :
						if ($GLOBALS['SQ_SYSTEM']->userRoot()) $access |= SQ_EI_READ;
						break;
					case 'sys_admin' :
						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) $access |= SQ_EI_READ;
						break;
					case 'admin' :
						if ($owner->adminAccess()) $access |= SQ_EI_READ;
						break;
					case 'write' :
						if ($owner->writeAccess()) $access |= SQ_EI_READ;
						break;
					case 'read' :
						if ($owner->readAccess()) $access |= SQ_EI_READ;
						break;

				}// end switch
			}// end if

		}// end if

		return $access;

	}// end _getAccess()


	/**
	* Paints the interface for editing details
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintDetails(&$owner, &$o)
	{
		if(!($file = $this->_findFile('static_screen_details'))) return false;

		$prefix = $owner->getPrefix();

		$print_commit_button = false;

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
					
					///// STATUS CHANGES - NOW AND IN THE FUTURE ////
					
					case 'current' :
						if ($field->children[0]->name != 'display_name') $display_name = 'Current';
						else $display_name = $field->children[0]->content;
						$o->openField($display_name);
						echo '<i>'.$owner->getStatusDescription().'</i>';
					break;

					case 'change' :
						$statii = $owner->getAvailableStatii();

						if (!empty($statii)) {
							$print_commit_button = true;
							$statii = array_reverse($statii, true);
							$statii[0] = '';
							$statii = array_reverse($statii, true);
							if ($field->children[0]->name != 'display_name') $display_name = 'Change ?';
							else $display_name = $field->children[0]->content;
							$o->openField($display_name);
							combo_box($prefix.'_change_status', $statii, false, 0);
						}
					break;

					case 'future_status' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
						$fs_jobs = &Cron_Job_Future_Status::getActiveJobs($owner);
						if (!empty($fs_jobs)) {
							$o->openField('Future Statuses', 'new_line');
							?>
							<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
								<tr>
									<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">
										Status
									</td>
									<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">
										When
									</td>
									<td bgcolor="#DDDDDD" align="center" class="sq-backend-field" style="font-weight: bold;">
										Delete ?
									</td>
								</tr>
							<?php

							for($i = 0; $i < count($fs_jobs); $i++) {
							?>
								<tr>
									<td class="sq-backend-data">
										<?php echo $fs_jobs[$i]->statusName(); ?>
									</td>
									<td class="sq-backend-data">
										<?php echo $fs_jobs[$i]->readableWhen(); ?>
									</td>
									<td align="center" class="sq-backend-data">
								<?php
									if ($fs_jobs[$i]->canDelete()) {
										check_box($prefix.'_remove_fs[]', $fs_jobs[$i]->id);
									} else {
										echo '&nbsp;';
									}// end if
								?>
									</td>
								</tr>
							<?php
							}// end foreach

							?>
							</table>
							<?php

						}// end if

						if ($owner->writeAccess(Array(), false)) {
							$o->openField('New Future Statuses', 'new_line');
							$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
							$fs = new Cron_Job_Future_Status();
							$edit_fns = $fs->getEditFns();
							$edit_fns->paintOneLineBackend($fs, $o, $prefix.'_new_fs');

							$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
							?>
							<div class="">
								The Cron Manager runs every <?php echo $cron_mgr->readableRefreshTime(); ?>.<br>
								The next time that it will run will be <?php echo $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun()); ?>
							</div>
							<?php
							$print_commit_button = true;
						}
					break;

					default :
						continue;
					break;
				}

				if ($field->children[1]->name == 'note' && $field->children[1]->content) {
					$o->note($field->children[1]->content);
				}
			}// end for

			$o->closeSection();

		}// end for

		return $print_commit_button;

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
		$prefix = $owner->getPrefix();

		$is_updated = false;

		///// IMMEDIATE STATUS CHANGES ////

		if (isset($_POST[$prefix.'_change_status']) && (int) $_POST[$prefix.'_change_status'] > 0) {
			if ($owner->processStatusChange($_POST[$prefix.'_change_status'])) {
				$is_updated = true;
			}
		}


		///// FUTURE STATUS CHANGES ////

		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (!is_null($cron_mgr)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

			if (!empty($_POST[$prefix.'_remove_fs'])) {
				$fs_jobs = &Cron_Job_Future_Status::getActiveJobs($owner);
				if (!empty($fs_jobs)) {

					if ($cron_mgr->acquireLock()) {
						for($i = 0; $i < count($fs_jobs); $i++) {
							if (in_array($fs_jobs[$i]->id, $_POST[$prefix.'_remove_fs'])) {
								if ($cron_mgr->removeJob($fs_jobs[$i])) {
									$is_updated = true;
								}
							}// end if
						}// end for

						$cron_mgr->releaseLock();

					} else {
						trigger_error('Unable to acquire the lock on the Cron Manager, unable to remove jobs', E_USER_NOTICE);

					}// end if

				}// end if
			}// end if


			if ($owner->writeAccess(Array(), false)) {

				$fs = new Cron_Job_Future_Status();
				$edit_fns = $fs->getEditFns();
				if ($edit_fns->processOneLineBackend($fs, $o, $prefix.'_new_fs')) {
					if (!is_null($cron_mgr)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
						if ($cron_mgr->addJob($fs, $GLOBALS['SQ_SYSTEM']->user)) {
							if ($fs->acquireLock()) {
								if ($fs->setAssetToUpdate($owner)) {
									$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
									$is_updated = true;
									$fs->releaseLock();
								} else {
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								}
							} else {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							}
						} else {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						}
					}
				}// end if

			}// end if

		}//end if is null cron mgr

		return ($is_updated);

	}//end processDetails()



	/**
	* Paints the interface for editing various settings
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @access public
	* @return boolean
	*/
	function paintSettings(&$owner, &$o)
	{
		if(!($file = $this->_findFile('static_screen_settings'))) return;

		$prefix = $owner->getPrefix();

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		if (empty($root->children)) {
			$o->OpenSection('There are no settings for "'.$owner->name.'"');
			$o->closeSection();
		}

		for($i = 0; $i < count($root->children); $i++) {
			$section = &$root->children[$i];

			$o->openSection($section->attributes['name']);

			if (!($num_fields = count($section->children))) continue;
			for($j = 0; $j < $num_fields; $j++) {
				$field = &$section->children[$j];

				// should we print the note for the field or not
				$print_note = true;

				switch (strtolower($field->attributes['code'])) {
					case 'version_current_version' :
						if ($field->children[0]->name != 'display_name') $display_name = 'Current Version';
						else $display_name = $field->children[0]->content;
						$o->openField($display_name);
						echo $owner->version;
					break;

					case 'version_update_verion' :
						// have to have admin access to update the major version
						if ($owner->adminAccess()) {
							if ($field->children[0]->name != 'display_name') $display_name = 'Update Major Version ?';
							else $display_name = $field->children[0]->content;
							$o->openField($display_name);
							check_box($prefix.'_version_update_version');
						} else {
							$print_note = false;
						}
					break;

					case 'intl_languages' :
						if ($field->children[0]->name != 'display_name') $display_name = 'Languages';
						else $display_name = $field->children[0]->content;
						$o->openSection($display_name);

						require SQ_FUDGE_PATH.'/standards_lists/languages.inc';

						if ($owner->writeAccess()) {
							$standards_lists_languages = array_reverse($standards_lists_languages);
							$standards_lists_languages[''] = '';
							$standards_lists_languages = array_reverse($standards_lists_languages);
							$o->openField('Add Language ?');
							combo_box($prefix.'_add_intl_language', $standards_lists_languages, false, '');
						}

						$o->openField('Current');
						$current_langs = $owner->getLanguages();
						if (!empty($current_langs)) {
							sort($current_langs);
							?>
							<table border="0">
								<?php if ($owner->writeAccess()) {
								?>
								<tr>
									<td bgcolor="#DDDDDD" class="sq-backend-data">&nbsp;</td>
									<td bgcolor="#DDDDDD" class="sq-backend-data"><b>Delete ?</b></td>
								</tr>
								<?php } ?>
							<?php
							foreach ($current_langs as $lang_code) {
								?>
								<tr>
									<td bgcolor="#F0F0F0" class="sq-backend-data"><?php echo $standards_lists_languages[$lang_code]; ?></td>
									<?php if ($owner->writeAccess()) {
									?>
									<td bgcolor="#F0F0F0" class="sq-backend-data" align="center"><?php check_box($prefix.'_rem_intl_language['.$lang_code.']'); ?></td>
									<?php } ?>
								</tr>
								<?php
							}
							?></table><?php
						} else {
							echo '<i>No languages are currently set for "'.$owner->name.'"</i>';
						}
						$o->closeSection();
					break;

					case 'intl_character_set' :
						if ($field->children[0]->name != 'display_name') $display_name = 'Character Set';
						else $display_name = $field->children[0]->content;
						$o->openSection($display_name);
						$o->openField('Current');
						require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';

						if ($owner->writeAccess()) {
							$standards_lists_character_sets = array_reverse($standards_lists_character_sets);
							$standards_lists_character_sets[''] = '** Use System Default **';
							$standards_lists_character_sets = array_reverse($standards_lists_character_sets);
							combo_box($prefix.'_intl_character_set', $standards_lists_character_sets, false, $owner->charset);
						} else {
							if (isset($standards_lists_character_sets[$owner->charset])) {
								echo $standards_lists_character_sets[$owner->charset];
							} elseif ($owner->charset == '') {
								echo '<i>No character set is currently set for "'.$owner->name.'"</i>';
							} else {
								echo '<i>Unkown character set "'.$owner->charset.'" is currently set for "'.$owner->name.'"</i>';
							}
						}
						$o->closeSection();
					break;

					case 'morph_new_type' :
						if ($field->children[0]->name != 'display_name') $display_name = 'New Type';
						else $display_name = $field->children[0]->content;
						$o->openField($display_name);

						if ($owner->adminAccess()) {
							if ($owner->status != SQ_STATUS_UNDER_CONSTRUCTION) {
								// cant morph an asset if it is not under construction
								echo '<i>An asset can only be morphed while Under Construction</i>';
							} else {
								$db = &$GLOBALS['SQ_SYSTEM']->db;
								$am = &$GLOBALS['SQ_SYSTEM']->am;
								$user = &$am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());

								$types = array_merge($am->getTypeAncestors($owner->type()), $am->getTypeDescendants($owner->type()));
								for($i = 0; $i < count($types); $i++) $types[$i] = $db->quote($types[$i]);
								$type_code_cond = 'IN ('.implode(', ', $types).')';

								$user_types = $am->getTypeAncestors($user->type());
								$user_types[] = $user->type();
								for($i = 0; $i < count($user_types); $i++) $user_types[$i] = $db->quote($user_types[$i]);
								$user_type_cond = 'IN ('.implode(', ', $user_types).')';

								$sql = 'SELECT type_code
										FROM '.SQ_TABLE_PREFIX.'asset_type
										WHERE allowed_access '.$user_type_cond.'
										  AND type_code '.$type_code_cond;

								$result = $db->getCol($sql);
								if (DB::isError($result)) {
									trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
									return Array();
								}

								if (empty($result)) {
									echo '<i>No types are available to morph this asset into</i>';
								} else {
									$morph_types = Array('' => '');
									foreach ($result as $type_code) {
										$code_name = str_replace('_', ' ', $type_code);
										$code_name = ucwords($code_name);
										$morph_types[$type_code] = $code_name;
									}
									combo_box($prefix.'_morph_new_type', $morph_types, false, '');
									?>
									<table border="0">
										<tr>
											<td class="sq-backend-data">To change the type of this asset, enter the 15 character string you see below into the box supplied, then press the Commit button</td>
										</tr>
										<tr>
											<td valign="top"><?php security_key(15, 20, 2); ?></td>
										</tr>
									</table>
									<?php
								}
							}
						} else {
							echo '<i>You do not have access to morph this asset</i>';
						}
					break;

					//// FRONTEND AND LOGIN DESIGN OPTIONS ////

					case 'frontend_design' :
					case 'login_design' :
						$design_name = strtolower($field->attributes['code']);
						
						if ($field->children[0]->name != 'display_name') $display_name = ucwords(str_replace('_', ' ', $design_name));
						else $display_name = $field->children[0]->content;
						$o->openSection($display_name);
						$o->openField('Current');

						$design_link = $owner->getLink(SQ_LINK_TYPE_3, 'design', false, $design_name);
						$designid = (empty($design_link)) ? 0 : $design_link['minorid'];

						if (!$owner->writeAccess()) {
							echo '<i>';
							if (!$designid) {
								echo 'None';
							} else {
								$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($designid, '', true);
								echo $design->name.' :: '.$design->id;
							}
							echo '</i>';

						} else {

							$designs = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', false, true);
							$contents = Array('0' => '-- use inherited --');
							foreach($designs as $id => $type_code) {
								$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($id, $type_code, true);
								if (is_null($design)) continue;
								$contents[$design->id] = $design->name.' :: '.$design->id;
							}

							asort($contents);
							combo_box($prefix.'_designs['.$design_name.']', $contents, false, $designid);
						}

						
						if ($field->children[1]->name == 'note' && $field->children[1]->content) {
							$o->note($field->children[1]->content);
						}
						$print_note = false;
						

						$o->closeField();
						$o->openField('Effective');

						$db = &$GLOBALS['SQ_SYSTEM']->db;

						// check for inherited designs
						$sql = 'SELECT l.url, a.name, l.'.$design_name.'id
								FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l 
								INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.'.$design_name.'id = a.assetid ';
						$where = 'l.frontend_designid <> 0 AND l.assetid = '.$db->quote($owner->id);
						$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
						$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

						$inherited_designs = $db->getAll($sql.$where);
						if (DB::isError($inherited_designs)) {
							trigger_error($inherited_designs->getMessage().'<br/>'.$inherited_designs->getUserInfo(), E_USER_WARNING);
						} else {
							if (!empty($inherited_designs)) {
								?>
								<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
									<tr>
										<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">URL</td>
										<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">Effective Design</td>
									</tr>
								<?php
								foreach ($inherited_designs as $design_data) {
									?>
									<tr>
										<td class="sq-backend-data"><?php echo $design_data['url']; ?></td>
										<td class="sq-backend-data"><?php echo $design_data['name'].' :: '.$design_data[$design_name.'id']; ?></td>
									</tr>
									<?php
								}
								?>
								</table>
								<?php
							} else {
								echo 'There are no effective designs';
							}
						}

						$o->closeField();
						$o->closeSection();

					break;

					default :
						continue;
					break;
				}

				if ($print_note && $field->children[1]->name == 'note' && $field->children[1]->content) {
					$o->note($field->children[1]->content);
				}
			}// end for

			$o->closeSection();

		}// end for

		return false;

	}//end paintSettings()


	/**
	* Processes the interface for editing various settings
	* returns true if there is a change
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function processSettings(&$owner, &$o)
	{
		$prefix = $owner->getPrefix();

		//// VERSIONING ////
		if ($owner->adminAccess()) {
			if (isset($_POST[$prefix.'_version_update_version'])) {
				if (!$owner->incrementVersion('major')) {
					trigger_error('Failed updating major version of "'.$owner->name.'"', E_USER_WARNING);
				}
			}
		}


		//// LANGUAGES ////

		$current_langs = $owner->getLanguages();
		$set_languages = Array();

		if (!isset($_POST[$prefix.'_rem_intl_language'])) $_POST[$prefix.'_rem_intl_language'] = Array();

		// add a new language
		if (isset($_POST[$prefix.'_add_intl_language']) && !empty($_POST[$prefix.'_add_intl_language'])) {
			$new_language = $_POST[$prefix.'_add_intl_language'];
			if (!in_array($new_language, $current_langs)) $set_languages[] = $new_language;
		}

		// check for delete languages
		foreach ($current_langs as $lang_code) {
			if (!isset($_POST[$prefix.'_rem_intl_language'][$lang_code])) $set_languages[] = $lang_code;
		}

		$owner->setLanguages($set_languages);


		//// CHARACTER SET ////

		if (isset($_POST[$prefix.'_intl_character_set'])) {
			$new_charset = $_POST[$prefix.'_intl_character_set'];
			$owner->setCharset($new_charset);
		}


		//// MORPHING ////

		if ($owner->adminAccess() && $owner->status == SQ_STATUS_UNDER_CONSTRUCTION) {
			if (isset($_POST[$prefix.'_morph_new_type']) && !empty($_POST[$prefix.'_morph_new_type'])) {
				// check that the security key was entered correctly
				if (validate_security_key()) {
					// errors for morphing will get reported in the morph function
					if ($owner->morph($_POST[$prefix.'_morph_new_type'])) return true;
				} else {
					trigger_error('The security key entered was incorrect. Asset type remains unchanged.', E_USER_WARNING);
				}
			}
		}


		///// FRONTEND AND LOGIN DESIGN CHANGES ////

		if ($owner->writeAccess()) {
			if (isset($_POST[$prefix.'_designs'])) {
				foreach ($_POST[$prefix.'_designs'] as $design_name => $new_designid) {
					
					$design_link = $owner->getLink(SQ_LINK_TYPE_3, 'design', false, $design_name);
					$designid = (empty($design_link)) ? 0 : $design_link['minorid'];

					// if they are the same then there is nothing to do.
					if ($designid != $new_designid) {

						$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

						// if there is an existing entry delete it
						if ($designid) {
							if (!$owner->deleteLink($design_link['linkid'])) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return false;
							}
						}

						if ($new_designid) {
							$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
							if (is_null($design)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return false;
							}
							if (!$owner->createLink($design, SQ_LINK_TYPE_3, $design_name)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return false;
							}
						}

						// fianlly, try updating the lookups
						if (!$owner->_updateLookups()) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return false;
						}

						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
						$is_updated = true;
					}
				}
			}
		}

		return true;

	}//end processSettings()


	/**
	* Paint the interface for managing permissions
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$allow_edit	allow editing of permissions if user has access
	*
	* @access public
	* @return boolean
	*/
	function paintPermissions(&$owner, &$o, $allow_edit=true)
	{
		$prefix = $owner->getPrefix();
		$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

		if ($allow_edit) $admin_access = $owner->adminAccess(Array(), true, false);
		else             $admin_access = false;

		foreach ($perms as $title => $perm) {
			$o->openSection($title.' Access');

			$permissions = $owner->getPermission($perm, null, false, false, true);

			$direct = Array();
			foreach($permissions as $userid => $access) {
				if (!isset($direct[$access])) $direct[$access] = Array();
				$direct[$access][] = $userid;
			}
			ksort($direct);

			if (!empty($direct)) {
				$o->openField('&nbsp;');
				foreach ($direct as $access => $userids) {
				?>
					<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
						<tr>
							<td bgcolor="#DDDDDD" class="sq-backend-data">
								<span style="font-weight: bold; color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Granted') ?></span>
							</td>
							<?php
							if ($admin_access) {
								?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-backend-data" style="font-weight: bold;">Delete ?</td><?php
							}
							?>
						</tr>
					<?php
					foreach($userids as $userid) {
						if ($userid) {
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
							if (!$user->id || (!is_a($user, 'user') && !is_a($user, 'user_group'))) continue;
						}
					?>
						<tr>
							<td bgcolor="#F0F0F0" class="sq-backend-data">
						<?php
							echo (($userid) ? $user->name.' ['.$user->type().' : '.$userid.']' : 'Public Access');
						?>
							</td>
						<?php
							if ($admin_access) {
								?><td bgcolor="#F0F0F0" align="center" width="100" class="sq-backend-data"><?php
								check_box($prefix.'_permissions['.$perm.'][delete]['.$userid.']');
								?></td><?php
							}
						?>
						</tr>
					<?php
					}// end foreach data
				?>
					</table>
					<br>
				<?php
				}//end foreach inherited

			}// end if

			if ($admin_access) {
				$o->openField('New ?', 'new_line');
					// if Public Access isn't set
					if (!isset($direct['0'])) {
						combo_box($prefix.'_permissions['.$perm.'][public_access][access]', Array('' => '', '1' => 'Grant', '0' => 'Deny'), false, '');
						echo ' Public Access<br/>';
					}

					combo_box($prefix.'_permissions['.$perm.'][new][access]', Array('' => '', '1' => 'Grant', '0' => 'Deny'), false, '');
					echo ' Access for ';
					asset_finder($prefix.'_permissions['.$perm.'][new][userid]', 0, Array('user' => 'D', 'user_group' => 'D'));
				$o->closeField();
			}// end if

			$o->closeSection();
		}// end foreach $perms

		if ($admin_access) {
			$kiddies = $owner->getChildren();
			if (count($kiddies)) {
				$o->openSection('Cascade Permission Changes?');
					$o->openField('Note');
						?>
						If you choose to cascade the changes to permissions you make on "<?php echo $owner->name; ?>", the changes will also be made on all assets that fall under "<?php echo $owner->name; ?>" in the various hierarchies in which it exists. This will affect <?php echo count($kiddies); ?> other assets.
						<?php
					$o->closeField();
					$o->openField('&nbsp;');
						check_box($prefix.'_permissions[cascade_changes]', '1', true);
						echo ' Yes, cascade permission changes';
					$o->closeField();
				$o->closeSection();

				$o->openSection('Manually Cascade Permissions?');
					$o->openField('Note');
						?>
						Manually cascading permissions will try to set the current permissions of "<?php echo $owner->name; ?>" on all assets that fall under "<?php echo $owner->name; ?>" in the various hierarchies in which it exists. This will affect <?php echo count($kiddies); ?> other assets.<br/>
						<i>Any additional permissions set on affected assets will <b>not</b> be removed during a manual cascade. Any permissions set on affected assets that conflict with ones set on "<?php echo $owner->name; ?>" will also <b>not</b> be changed.</i><br/>
						<b>NB. If you select to manually cascade, any permission changes you make will also cascade even if you do not select them to. Use the option on its own for more predicatable results.</b>
						<?php
					$o->closeField();
					$o->openField('&nbsp;');
						check_box($prefix.'_permissions[manual_cascade]');
						echo ' Manually cascade permissions';
					$o->closeField();
				$o->closeSection();
			}
		}

		return $admin_access;

	}//end paintPermissions()


	/**
	* Process the interface for managing permissions
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @access public
	* @return boolean TRUE is there is a change, false on error or no change
	*/
	function processPermissions(&$owner, &$o)
	{
		// if we dont have admin access, piss off
		if ($owner->id && !$owner->adminAccess(Array(), false)) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the permissions for <i>'.$owner->name.'</i>');
			return false;
		}

		// if we dont have admin access, dont process anything
		if (!$owner->adminAccess(Array(), true, false)) return false;

		$permission_vars = Array();
		$prefix = $owner->getPrefix();
		if (!isset($_POST[$prefix.'_permissions'])) return false;

		// are we cascading these permission changes to our children?
		$cascade_changes = false;
		if (isset($_POST[$prefix.'_permissions']['cascade_changes']) && $_POST[$prefix.'_permissions']['cascade_changes']) {
			$cascade_changes = true;
		}

		$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);
		foreach ($perms as $title => $perm) {
			// we need to check for write access each time because
			// we may actually be revoking write access and then
			// trying to put it back
			if (!$owner->adminAccess(Array(), true, false)) continue;

			$post_data = $_POST[$prefix.'_permissions'][$perm];

			// grant public access
			if (isset($post_data['public_access']['access']) && $post_data['public_access']['access'] != '') {
				if ($cascade_changes) {
					// cascading public access of some kind, so queue it up for the HIPO
					$permission_vars[$perm][] = Array('access'          => (int)$post_data['public_access']['access'], 
													  'userid'          => 0,
													  'previous_access' => null,
													  );
				} else {
					// lets do this ourselves
					if (!$owner->setPermission(0, $perm, $post_data['public_access']['access'])) return false;
				}
			}

			// grant access to a user
			if (!empty($post_data['new']['userid'])) {
				if (!isset($post_data['new']['access']) || $post_data['new']['access'] == '') {
					trigger_error('You need to select the type of access in order to set a permission for Asset #'.$post_data['new']['userid'], E_USER_NOTICE);
				} else {
					if ($cascade_changes) {
						// cascading access of some kind, so queue it up for the HIPO
						$permission_vars[$perm][] = Array('access'          => (int)$post_data['new']['access'],
														  'userid'          => (int)$post_data['new']['userid'],
														  'previous_access' => null,
														  );
					} else {
						// lets do this ourselves
						if (!$owner->setPermission($post_data['new']['userid'], $perm, $post_data['new']['access'])) return false;

					}
				}
			}

			// revoke [delete] access (grant or deny can be revoked)
			if (!empty($post_data['delete'])) {
				foreach ($post_data['delete'] as $userid => $on) {
					$userid = (int)$userid;
					if ($cascade_changes) {
						$current = $owner->getPermission($perm, null, false, false, true);
						$current_access = $current[$userid];
						$permission_vars[$perm][] = Array('access'          => -1,
														  'userid'          => $userid,
														  'previous_access' => $current_access,
														  );
					} else {
						if (!$owner->deletePermission($userid, $perm)) return false;
					}
				}
			}

			if (isset($_POST[$prefix.'_permissions']['manual_cascade']) && $_POST[$prefix.'_permissions']['manual_cascade']) {
				$current = $owner->getPermission($perm, null, false, false, true);
				foreach ($current as $userid => $access) {
					$permission_vars[$perm][] = Array('access'          => $access,
													  'userid'          => $userid,
													  'previous_access' => null,
													  );
				}
			}

		}// end foreach perms

		if (!empty($permission_vars)) {

			//// START HIPO PROCESSING ////
			
			$vars = Array('assetid' => $owner->id, 'permission_changes' => $permission_vars);
			$codename = 'HIPO_edit_permissions';
			$complete_url = $_SERVER['REQUEST_URI'];

			require_once(SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_edit_permissions.inc');
			$hipo = new HIPO_Job_Edit_Permissions();

			$hipo->setRunningVars($vars);
			$hipo->setOption('on_complete_url', $complete_url);

			if ($hipo->initialise()) {
				$url = $hipo->getProcessURL();
				$o->setRedirect($url);
			}
			
			////  END HIPO PROCESSING  ////

		}//end if we have permission to set

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
		$prefix = $owner->getPrefix().'_workflow';

		require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($owner, true, true, true);

		$inheriting_workflows = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo($wfm->getRunningInheriting()));

		if (!empty($inheriting_workflows)) {
			$o->openSection('Running Workflows Downstream');
			$o->openField('Note');
			echo 'Any changes you make to the workflow will not affect the currently running workflows for the following assets:<ul>';
			foreach ($inheriting_workflows as $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo '<li class="sq-backend-data">'.$asset->name.'</li>';
			}
			echo '</ul>';
			$o->closeSection();
		}// end if

		$admin_access = false;
		if ($wfm->state == 'running') {
			// cant edit workflow when workflow
			// is currently running
			$admin_access = false;

			$o->openSection('Current Workflow Progress');
			$o->openField('Note');
			echo 'Workflow for this asset is currently in progress. You can not edit the workflow for this asset at this time. The currently running workflows are displayed below followed by the workflow for this asset.';
			$o->closeSection();

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
							<td bgcolor="#DDDDDD" class="sq-backend-data">&nbsp;</td>
							<td bgcolor="#DDDDDD" class="sq-backend-data" width="270"><b>Group Logic</b></td>
							<td bgcolor="#DDDDDD" align="center" width="100" class="sq-backend-data"><b>Complete ?</b></td>
						</tr>
					<?php
					foreach ($step_data['conditions'] as $pub => $cond_data) {
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
						if (!$asset->id) continue;
						?>
						<tr>
							<td bgcolor="#F0F0F0" class="sq-backend-data"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $pub]";?></td>
							<td bgcolor="#F0F0F0" class="sq-backend-data">
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

							<td bgcolor="#F0F0F0" align="center" class="sq-backend-data">
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

					$o->closeSection();

				}// end foreach

				$o->closeSection();

			}// end foreach

		} else {
			$admin_access = $owner->adminAccess(Array(), true, false);
		}


		//// print the list of inherited workflows ////

		$inherited = count($wfm->inherited_workflows);
		$revoked   = $owner->getPermission(SQ_PERMISSION_WORKFLOW, false);
		$revokes   = count($revoked);

		if ($inherited || $revokes) {
			$o->openSection('Inherited Workflows');

			if ($inherited) {
				$o->openField('Inherited');
				?>
				<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<?php
				if ($admin_access) {
					?>
					<tr>
						<td bgcolor="#DDDDDD" class="sq-backend-data">&nbsp;</td>
						<td bgcolor="#DDDDDD" align="center" width="100" class="sq-backend-data"><b>Revoke ?</b></td>
					</tr>
					<?php
				}

				foreach ($wfm->inherited_workflows as $assetid => $data) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (is_null($asset)) continue;
					?>
					<tr>
						<td bgcolor="#F0F0F0" class="sq-backend-data">
						<?php
							echo 'Workflow inherited from '.$asset->name.' ['.$asset->type()." : $assetid]";
						?>
						</td>
					<?php
					if ($admin_access) {
						?>
						<td bgcolor="#F0F0F0" align="center" width="100" class="sq-backend-data">
						<?php check_box($prefix."_revoke[$assetid]"); ?>
						</td>
						<?php
					}
					?></tr><?php
				}

				?></table><?php
			}

			if ($revokes) {
				$o->openField('Revoked');
				?>
				<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
					<tr>
						<td bgcolor="#DDDDDD" class="sq-backend-data">&nbsp;</td>
						<?php
						if ($admin_access) {
							?><td bgcolor="#DDDDDD" align="center" width="100" class="sq-backend-data"><b>Reinstate ?</b></td><?php
						}
						?>
					</tr>
				<?php

				foreach ($revoked as $assetid) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (is_null($asset)) continue;
					?>
					<tr>
						<td bgcolor="#F0F0F0" class="sq-backend-data">
						<?php echo 'Workflow inherited from '.$asset->name.' ['.$asset->type().' : '.$assetid.']'; ?>
						</td>
					<?php
					if ($admin_access) {
						?>
						<td bgcolor="#F0F0F0" align="center" width="100" class="sq-backend-data">
						<?php check_box($prefix.'_unrevoke['.$assetid.']'); ?>
						</td>
						<?php
					}
					?></tr><?php

				}// end foreach

				?></table><?php

			}// end if revoked

			$o->closeSection();

		}// end if inherited section


		//// now print the direct workflow for this asset ////
		$o->openSection('Workflow set for this asset');
		$o->closeSection();
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
					<td bgcolor="#DDDDDD" class="sq-backend-data">&nbsp;</td>
					<td bgcolor="#DDDDDD" class="sq-backend-data" width="270"><b>Group Logic</b></td>
					<?php
					if ($admin_access) {
						?><td bgcolor="#DDDDDD" align="center" width="70" class="sq-backend-data"><b>Delete ?</b></td><?php
					}
					?>
				</tr>
			<?php
			foreach ($step_data['conditions'] as $pub => $cond_data) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
				if (!$asset->id) continue;
				?>
				<tr>
					<td bgcolor="#F0F0F0" class="sq-backend-data"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $pub]";?></td>
					<td bgcolor="#F0F0F0" class="sq-backend-data">
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
					?><td bgcolor="#F0F0F0" align="center" class="sq-backend-data"><?php
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

			$o->closeSection();

		}// end for

		if ($admin_access) {
			$o->openSection("Create New Step");
			$o->openField('');
			check_box($prefix.'_create_step');
			echo '&nbsp; Create a new empty step?';
			$o->closeSection();
		}// end for

		return $admin_access;

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

		// if we dont have admin access, dont process anything
		if (!$owner->adminAccess(Array(), true, false)) return false;

		require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($owner);
		if ($wfm->state == 'running') return false;

		$prefix = $owner->getPrefix().'_workflow';

		// unrevoke inherited workflow
		if (isset($_POST[$prefix.'_unrevoke'])) {
			foreach ($_POST[$prefix.'_unrevoke'] as $assetid => $on) {
				if (!$owner->deletePermission($assetid, SQ_PERMISSION_WORKFLOW)) return false;
			}
		}

		// revoke inherited workflow
		if (isset($_POST[$prefix.'_revoke'])) {
			foreach ($_POST[$prefix.'_revoke'] as $assetid => $on) {
				if (!$owner->setPermission($assetid, SQ_PERMISSION_WORKFLOW, '0')) return false;
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
				<td class="sq-backend-data">
					<table cellpadding="0" cellspacing="0" border="0">
						<tr>
							<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="20" border="0" alt="" /></td>
							<td class="sq-backend-data">
								<a href="<?php echo $owner->getBackendHref('dependants'); ?>"><b><?php echo $owner->name; ?></b></a>
							</td>
						</tr>
					</table>
				</td>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" valign="top">
					<?php echo $lock_message; ?>
				</td>
				<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" colspan="3">
					<?php echo $owner->getStatusDescription(); ?>
				</td>
			</tr>
			<?php $this->_recursePaintDependants($owner, $owner, $o); ?>
		</table>
		<table>
			<tr>
				<td class="sq-backend-data"><span style="color:red"><b>*</b></span></td>
				<td class="sq-backend-data">
					The status of the dependant asset is higher than the status of "<?php echo $owner->name; ?>"
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><span style="color:red"><b>**</b></span></td>
				<td class="sq-backend-data">
					The status of the dependant asset is lower than the status of "<?php echo $owner->name; ?>"
				</td>
			</tr>
		</table>
		<?php

		$o->closeSection();

		return false;

	}//end paintLocking()


	/**
	* Paints an asset dependancy tree with stalks and dependant info
	*
	* @param object	Asset				$owner			the asset whose interface we are painting
	* @param object	Asset				$owner			the asset whose dependants we are painting
	* @param object	Backend_Outputter	$o				the outputter class
	* @param int						$levels			how many levels down are we
	* @param int						$depth			how far deep are we in
	* @param boolean					$last_bg		was the last entry printed an end stalk?
	*
	* @return void
	* @access public
	*/
	function _recursePaintDependants(&$owner, &$asset, &$o, $levels=1, $depth=0, $last_bg=false)
	{
		$dependant_links = $asset->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		$num_kids = count($dependant_links);
		for ($i = 0; $i < $num_kids; $i++) {
			$dep_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_links[$i]['minorid']);
			$end = ($i == $num_kids - 1);
			$stalk = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
			$bg = ($end) ? '' : $stalk;

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

			$status_message = '';
			if ($dep_asset->status < $owner->status) {
				$status_message = '<span style="color:red"><b>**</b></span>';
			} else if ($dep_asset->status > $owner->status) {
				$status_message = '<span style="color:red"><b>*</b></span>';
			}

			?>
			<tr>
				<td class="sq-backend-data">
					<table cellpadding="0" cellspacing="0" border="0">
						<tr>
							<?php
							for ($x = 0; $x < $levels; $x++) {
								$stalk_bg = '';
								if ($depth && $x >= $depth) $stalk_bg = $stalk;
								?>
								<td class="sq-backend-data" <?php echo $stalk_bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="20" height="1" alt="blank" /></td>
								<?php
							}
							?>
							<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
							<td class="sq-backend-data">
								&nbsp;<a href="<?php echo $dep_asset->getBackendHref('dependants'); ?>"><b><?php echo $dep_asset->name; ?></b></a>
							</td>
						</tr>
					</table>
				</td>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data">
					<?php echo $lock_message; ?>
				</td>
				<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" valign="top">
					<?php echo $dep_asset->getStatusDescription(); ?>
				</td>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data">
					<?php echo $status_message; ?>
				</td>
			</tr>
			<?php
			$child_deps = $dep_asset->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);

			$new_depth = $depth;
			if (empty($bg)) {
				$new_depth = 0;
			} else if ($last_bg) {
				$new_depth = $levels;
			} else if (!$depth) {
				$new_depth = 1;
			}

			if (!empty($child_deps)) {
				$this->_recursePaintDependants($owner, $dep_asset, $o, ($levels+1), $new_depth, empty($bg));
			}
		}

	}//end _recursePaintDependants


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
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$allow_edit	allow editing of link types if user has access
	*
	* @return boolean
	* @access public
	*/
	function paintLinking(&$owner, &$o, $allow_edit=true)
	{
		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$o->openSection('Assets linking to "'.$owner->name.'"');
		$o->openField('&nbsp;');
		echo '<i>NOTE: <b>TYPE_1</b> links are displayed in both the frontend and backend navigation but <b>TYPE_2</b> links are only displayed in the backend navigation.</i><br/><br/>';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = Array();
		$asset_links = $owner->getLinks(SQ_SC_LINK_ALL, '', true, 'minor');
		$editable_link_types = Array(SQ_LINK_TYPE_1 => link_type_name(SQ_LINK_TYPE_1),
									 SQ_LINK_TYPE_2 => link_type_name(SQ_LINK_TYPE_2)
									);

		foreach ($asset_links as $link) $assetids[$link['majorid']] = Array();

		if (empty($assetids)) {
			$result = Array();
		} else {
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');

			$where = 'cl.minorid IN ('.implode(', ', array_keys($assetids)).')
					  AND ct.treeid LIKE '.$concat.'
					  AND pt.treeid <= ct.treeid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.name, a.assetid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ON cl.linkid = ct.linkid,
						 '.SQ_TABLE_RUNNING_PREFIX.'asset_link pl INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt ON pl.linkid = pt.linkid
						 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON a.assetid = pl.minorid
					'.$where.' ORDER BY cl.linkid, ct.treeid, pt.treeid';

			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}
		}

		foreach ($asset_links as $link_data) $link_info[$link_data['majorid']] = $link_data;

		$output_lineages = Array();
		$editing_linkids = Array();
		foreach ($result as $treeid => $link_tree) {

			// work out some values we are going to need
			$parent_data = $link_tree[count($link_tree) -1];
			$majorid = $parent_data['assetid'];
			$link_type = $link_info[$majorid]['link_type'];
			$linkid = $link_info[$majorid]['linkid'];

			// dont even pretend that the root folder is going to
			// care that it is linking to this asset!!
			if ($majorid == 1) continue;

			// work out if the user can change the link types
			// the arg $allow_edit must be set to TRUE to try the write access test
			$can_edit = false;
			if ($allow_edit) {
				if ($owner->writeAccess(Array(), true, false) && in_array($link_type, array_keys($editable_link_types)) && !in_array($linkid, $editing_linkids)) $can_edit = true;
			}

			$asset_lineage = 'Root Folder > ';
			foreach ($link_tree as $tree_data) {
				if ($tree_data['name'] == '/') continue;
				if ($tree_data['assetid'] == $majorid) continue;
				$asset_lineage .= $tree_data['name'].' > ';
			}
			$asset_lineage .= '<b>'.$parent_data['name'].'</b>';
			if (isset($output_lineages[$asset_lineage]) && !$can_edit) continue;

			ob_start();
			?>
			<tr>
				<td class="sq-backend-data">[</td>
				<td align="center" class="sq-backend-data">
					<?php
					if (!$can_edit) {
						echo link_type_name($link_type);
					} else {
						combo_box($prefix.'_link_type['.$linkid.']', $editable_link_types, false, $link_type);
						$editing_linkids[] = $linkid;
					}
					?>
				</td>
				<td class="sq-backend-data">]</td>
				<td class="sq-backend-data"><?php echo $asset_lineage; ?></td>
			</tr>
			<?php
			$output_lineages[$asset_lineage] = ob_get_contents();
			ob_end_clean();

		}//end foreach result

		if (!count($output_lineages)) {
			echo 'None';
		} else {
			?><table border="0"><?php
			foreach ($output_lineages as $null => $lineage_data) echo $lineage_data;
			?></table><?php
		}

		$o->closeSection();

		return $owner->writeAccess(Array(), true, false);

	}//end paintLinking()


	/**
	* Processes the interface for viewing links to/from this asset
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
		if (!$owner->writeAccess(Array(), true, false)) return false;

		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		$done_something = false;
		$asset_links = $owner->getLinks(SQ_SC_LINK_ALL, '', true, 'minor');

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
	* Paints the interface for viewing the history of this asset
	* This includes going into rollback view mode
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintHistory(&$owner, &$o)
	{
		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// note that we dont prep this query for rollback select
		// because we are selecting directly from the rollback table
		$sql = 'SELECT '.SQ_TABLE_PREFIX.'effective_from AS efrom, '.SQ_TABLE_PREFIX.'effective_to AS eto, version
				FROM '.SQ_TABLE_ROLLBACK_PREFIX.'asset
				WHERE assetid = '.$db->quote($owner->id).'
				ORDER BY '.SQ_TABLE_PREFIX.'effective_from DESC';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$o->openSection('Version History');
		$o->openField('');

		if (SQ_ROLLBACK_VIEW) {
			?><p class="sq-backend-data"><b><i>The highlighted row in the version history is the current version of "<?php echo $owner->name; ?>" being viewed in rollback mode</i></b></p>
			<?php
		}

		hidden_field($prefix.'_rollback_version', '0');

		?>
		<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
			<tr>
				<td bgcolor="#DDDDDD" class="sq-backend-data"><b>Version Duration</b></td>
				<td bgcolor="#DDDDDD" class="sq-backend-data"><b>Time Since Version</b></td>
				<td bgcolor="#DDDDDD" class="sq-backend-data" align="center"><b>Version</b></td>
				<td bgcolor="#DDDDDD" class="sq-backend-data" align="center"><b>View</b></td>
			</tr>
		<?php

		foreach ($result as $data) {
			$from = $data['efrom'];
			$to   = $data['eto'];

			$bgcolour = '#F0F0F0';
			$show_view_btn = true;
			if (SQ_ROLLBACK_VIEW) {
				$rollback_from = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
				if ($rollback_from >= strtotime($from) && (empty($to) || strtotime($to) > $rollback_from)) {
					$bgcolour = '#FFE5E8';
					$show_view_btn = false;
				}
			}

			if (empty($to)) {
				$to = 'present';
				$ago = 'Most Recent';
			} else {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$ago = easy_time_total(time() - strtotime($from), true);
				list($fd, $ft) = explode(' ', $from);
				list($td, $tt) = explode(' ', $to);
				if ($fd == $td) $to = $tt;
			}

			?>
			<tr>
				<td bgcolor="<?php echo $bgcolour; ?>" class="sq-backend-data"><?php echo $from; ?> - <?php echo $to; ?></td>
				<td bgcolor="<?php echo $bgcolour; ?>" class="sq-backend-data"><?php echo $ago; ?></td>
				<td bgcolor="<?php echo $bgcolour; ?>" class="sq-backend-data" align="center"><?php echo $data['version']; ?></td>
				<td bgcolor="<?php echo $bgcolour; ?>" class="sq-backend-data" align="center">
				<?php
					if ($show_view_btn) {
						echo submit_button($prefix.'_rollback_mode', ' View ', 'this.form.'.$prefix.'_rollback_version.value = \''.$data['version'].'\'');
					} else {
						echo '&nbsp;';
					}
				?>
				</td>
			</tr>
			<?php
		}

		?>
		</table>
		<?php

		$o->closeField();
		$o->closeSection();

	}//end paintHistory()


	/**
	* Processes the interface for viewing the history of the asset
	* This includes going into rollback view mode
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processHistory(&$owner, &$o)
	{
		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		if (isset($_POST[$prefix.'_rollback_mode']) && isset($_POST[$prefix.'_rollback_version'])) {
			$version = $_POST[$prefix.'_rollback_version'];
			$url = $_SERVER['PHP_SELF'].'?SQ_ACTION=rollback_view_start&assetid='.$owner->id.'&version='.$version;
			$o->setRedirect($url, 'top');
		}

		return true;

	}//end processHistory()


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
