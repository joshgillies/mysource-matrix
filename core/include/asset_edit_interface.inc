<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: asset_edit_interface.inc,v 1.182 2003/11/18 15:58:52 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once 'XML/Tree.php';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/*
* Used by both paint() and process() to determine the access that the current user
* has - set in _getAccess()
*/
define('SQ_EI_READ',  1);
define('SQ_EI_WRITE', 2);

/**
* Asset_Edit_Interface
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Edit_Interface extends MySource_Object
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
	* @access public
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
		$parent_type = $this->_type_code;
		while(($parent_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($parent_type, 'parent_type')) != 'asset') {
			$this->_dirs[] = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($parent_type, 'dir');
		}

		// add the default for Asset
		$this->_dirs[] = SQ_INCLUDE_PATH.'/asset_edit';

		if(!($file = $this->_findFile('screens'))) return;

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error('Unable to Read File : '.$file."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		$this->_screens = Array();
		for($i = 0; $i < count($root->children); $i++) {
			$code_name = strtolower(preg_replace('/[^a-z_]/', '', $root->children[$i]->attributes['code_name']));
			if (!$code_name) continue;

			$force_unlock = ((isset($root->children[$i]->attributes['force_unlock'])) ? $root->children[$i]->attributes['force_unlock'] : '1');

			$this->_screens[$code_name] = Array(
												'name'			=> $root->children[$i]->attributes['display_name'],
												'icon'			=> $root->children[$i]->attributes['icon'],
												'force_unlock'	=> ($force_unlock != '0'),
												'lock_type'		=> (isset($root->children[$i]->attributes['lock_type'])) ? $root->children[$i]->attributes['lock_type'] : '',
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

	}//end getScreens()


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
			exit();
		}

		// if there are no screens or static screens, piss off
		if (!count($this->_screens) && empty($this->_edit_fns->static_screens)) return;


		  /////////////////////
		 //  ACTIVE SCREEN  //
		/////////////////////
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

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('asset_ei_screen', $active_screen);


		  ///////////////////////////////
		 //  LOCKING AND PERMISSIONS  //
		///////////////////////////////
		// do we have write access?
		$full_write_access = $owner->writeAccess('');

		// do we print the commit button (ie. have we printed an editable interface?)
		$print_commit_button = false;

		// do we unlock this asset when committing?
		$force_unlock_on_commit = true;

		if (!$creating) {
			// default to 'all', so if lock type is left out a lock can still be gotten and the asset edited
			$screen_lock_type = 'all';
			if (isset($this->_edit_fns->static_screens[$active_screen])) {
				if (!empty($this->_edit_fns->static_screens[$active_screen]['lock_type'])) {
					$screen_lock_type = $this->_edit_fns->static_screens[$active_screen]['lock_type'];
				}
			} else {
				// must be a normal screen
				if (!empty($this->_screens[$active_screen]['lock_type'])) {
					$screen_lock_type = $this->_screens[$active_screen]['lock_type'];
				}
			}// end if

			// do they have access to acquire the lock?
			$acquire_lock_access = $owner->canLiveEdit($screen_lock_type);

			$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, $screen_lock_type, true);
			
			if (strtolower($screen_lock_type) != 'none') {
				// lets tell the user if the asset is locked for editing
				// or if they can lock it to edit it
				$o->openSection('Locking / Editing');
				$o->openField('&nbsp;');
			?>
				<table class="sq-backend-lock-table">
					<tr>
						<td class="sq-backend-lock-table-header" style="width: 20px;">
							<img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" />
						</td>
						<td class="sq-backend-lock-table-header" align="left" nowrap>
							&nbsp;Lock Type
						</td>
						<td class="sq-backend-lock-table-header" width="100%">
							&nbsp;
						</td>
					</tr>
					<tr>
						<td width="100%" colspan="3">
							<table width="100%" cellspacing="1" cellpadding="0">
				<?php
			} else {
				$o->openRaw('');
			}

			$have_any_locks  = false;
			$have_all_locks  = true;
			$need_force_locks = false;
			$can_force_locks  = true;
			$class = 'sq-backend-lock-table-cell-alt';
			foreach($locks as $lock_type => $lock) {
				if ($class == 'sq-backend-lock-table-cell') $class = 'sq-backend-lock-table-cell-alt';
				else $class = 'sq-backend-lock-table-cell';

			?>
				<tr>
					<td class="<?php echo $class; ?>" style="width: 20px;" align="center">
						<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib'); ?>/web/images/icons/<?php echo ((empty($lock)) ? 'un' : ''); ?>locked.png", "16", "16", "");</script>
					</td>
					<td class="<?php echo $class; ?>">
						<?php echo ucwords(str_replace('_', ' ', $lock_type)); ?>
					</td>
					<td class="<?php echo $class; ?>">
						<?php
						if (empty($lock)) {
							$have_all_locks = false;
							if ($full_write_access && $acquire_lock_access) {
								echo '<p class="sq-backend-unlocked">Currently not held.</p>';
							} else {
								echo 'You do not have access to edit "'.$owner->name.'".';
							}

						} else {
							// this asset is currently locked
							// so display message to the user
							$user    = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
							$editing = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

							if (SQ_ROLLBACK_VIEW) $now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
							else $now = time();

							if (is_null($lock['expires'])) {
								$expires_in = 'The lock is being held indefinitely and will not expire. The lock must be released manually or be forceably acquired by a System Administrator.';
							} else {
								require_once SQ_FUDGE_PATH.'/general/datetime.inc';
								$expires_in = easy_time_total(($lock['expires'] - $now), true);
								if (!$expires_in) $expires_in = '1 second';
								$expires_in = 'The lock is due to expire in '.$expires_in;
							}

							?>
							<p class="sq-backend-locked-by-<?php echo ($GLOBALS['SQ_SYSTEM']->currentUser($user)) ? 'user' : 'someone-else'; ?>">
								Held by user "<?php echo $user->name; ?>" at "<?php echo $editing->name; ?>".
								<?php echo $expires_in; ?>
							</p>
							<?php

							// if this lock is held the current user
							if ($GLOBALS['SQ_SYSTEM']->currentUser($user)) {
								$have_any_locks = true;

							// else it's held by someone else, check if we can forceable acquire it
							} else {
								$need_force_locks = true;
								$have_all_locks   = false;
								// does the current user has a high
								// enough level of access to forceably acquire the lock
								if (!$owner->canForceablyAcquireLock($lock_type)) {
									$can_force_locks = false;
								}
							}

						}// end if
						?>
					</td>
				</tr>
			<?php
			}// end foreach lock types
			?>
						</table>
					</td>
				</tr>
			</table>

			<?php
			hidden_field('sq_lock_type', $screen_lock_type);

			if (!$have_all_locks || $have_any_locks) {
				?><span style="float: right; margin-top: 2px; text-align: right;"><?php
			}

			if (!$have_all_locks) {
				if ($need_force_locks) {
					if ($can_force_locks) {
						echo '<div class="sq-backend-locked-by-someone-else">You can forceably acquire this lock, which will remove the lock from "'.$user->name.'" and give the lock to yourself.<br>Any changes currently being made by "'.$user->name.'" will be lost if this lock is forceably acquired.</div>';

						// they aren't going to do this without a fight though
						// they'll have to go through our popup window first
						$popup_url = '?SQ_BACKEND_PAGE=main&backend_section=am&ignore_frames=1&am_section=forceably_acquire_lock&assetid='.$owner->id.'&sq_lock_type='.$screen_lock_type;

						hidden_field('sq_lock_acquire_by_force', '0');
						hidden_field('sq_lock_acquire', '0');
						normal_button('sq_forceably_acquire_lock', 'Acquire Lock(s)', "window.open('$popup_url', 'sq_forceably_acquire_lock_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300');");
					}
				} else if ($full_write_access && $acquire_lock_access) {
					hidden_field('sq_lock_acquire_by_force', '0');
					submit_button('sq_lock_acquire', 'Lock');
				}// end if
			}

			if ($have_any_locks) submit_button('sq_lock_release_manual', 'Release Lock(s)');

			if (!$have_all_locks || $have_any_locks) {
				?></span><?php
			}

			if (strtolower($screen_lock_type) != 'none') $o->closeSection();
			else $o->closeRaw();

		}// end if

		// check the active screen for special system defined static screens
		$file = null;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {

			$active_screen_name = $this->_edit_fns->static_screens[$active_screen]['name'];

			// if we are not creating, paint the system defined bit of this tab
			if (!$creating) {
				$paint_function = 'paint'.$active_screen;
				if ($this->$paint_function($owner, $o)) $print_commit_button = true;
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

			$active_screen_name = $this->_screens[$active_screen]['name'];

		}// end if


		  ///////////////////
		 //  WHERE AM I   //
		///////////////////
		if (isset($_GET['sq_link_path'])) {
			$link_path = $_GET['sq_link_path'];
		} else {
			$link_path = '';
		}
		$o->addFormActionGetVar('sq_link_path', $link_path, true);
		$o->addHiddenField('sq_link_path', $link_path);


		  ///////////////////
		 //  PREVIEW URL  //
		///////////////////
		// get the asset tree lineage
		if (isset($_GET['sq_asset_path'])) {
			// flash sends comma-separated assetids indicating the path
			$asset_path = split(',', ($_GET['sq_asset_path']));
		} else {
			// flash didn't send one (e.g initial matrix loading) - pretend it did by making a singleton set of the owner
			$asset_path = Array($owner->id);
		}

		$o->addFormActionGetVar('sq_asset_path', implode(',', $asset_path), true);

		$preview_url = '';
		$closest_parentid = $owner->id;
		foreach ($asset_path as $assetid) {
			if ($assetid != $owner->id) $ancestor =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			else $ancestor =& $owner;
			
			if (is_null($ancestor)) {
				$preview_url = '';
				break;
			}

			$paths = $ancestor->getWebPaths();
			if (!empty($paths)) $closest_parentid = $assetid;

			if ($preview_url == '') {
				// we need a base url
				$url = $ancestor->getUrl();
				if (!empty($url)) {
					$preview_url = $ancestor->getUrl();
					$closest_parentid = $assetid;
				}
			} else {
				// we have a base url, but we need the appropriate paths
				if (empty($paths)) {
					// no path defined from here onwards - just use what we've got so far
					break;
				} else {
					$preview_url .= "/" . array_shift($paths);
				}
			}
		}
		$o->addHiddenField('sq_preview_url', $preview_url);

		// store this preview url into the global vars so that any other asset
		// can use it as a way to know the content we are in
		$GLOBALS['sq_preview_url'] = $preview_url;


		  //////////////////////////
		 //  Navigation History  //
		//////////////////////////
		if (!isset($_SESSION['sq_nav_history']) || !is_array($_SESSION['sq_nav_history'])) $_SESSION['sq_nav_history'] = Array();

		// first work out if we have gone back to a previous entry
		if (!empty($_SESSION['sq_nav_history'])) {
			if (isset($_REQUEST['sq_nav_goback'])) {
				for ($i = count($_SESSION['sq_nav_history']); $i > $_REQUEST['sq_nav_goback']; $i--) {
					array_pop($_SESSION['sq_nav_history']);
					if (empty($_SESSION['sq_nav_history'])) break;
				}
			}
		}

		if (!$creating) {
			if (!empty($_SESSION['sq_nav_history'])) {
				$nav_count = count($_SESSION['sq_nav_history']);

				if ($nav_count >= 20) {
					array_shift($_SESSION['sq_nav_history']);
					$nav_count--;
				}

				$current_nav_node = $_SESSION['sq_nav_history'][($nav_count-1)];
				if ($current_nav_node['assetid'] != $owner->id) {
					$_SESSION['sq_nav_history'][$nav_count] = Array('assetid' => $owner->id, 'screen' => $active_screen);
				} else if ($current_nav_node['screen'] != $active_screen) {
					$_SESSION['sq_nav_history'][($nav_count -1)] = Array('assetid' => $owner->id, 'screen' => $active_screen);
				}
			} else {
				$_SESSION['sq_nav_history'][0] = Array('assetid' => $owner->id, 'screen' => $active_screen);
			}
		}


		  /////////////////////
		 //  PAGE HEADINGS  //
		/////////////////////
		// set correct page headings based on what we are doing
		if (!$creating) {
			$page_heading = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name').' - '.$owner->name.' (v. '.$owner->version.')';
		} else {
			$page_heading = 'Create new '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name');
		}
		$page_heading .= '&nbsp;&nbsp;|&nbsp;&nbsp;'.$active_screen_name;

		$o->setHeading($page_heading, '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($this->_type_code).'", "16", "16", "");</script>');

		if ($closest_parentid != $owner->id) {
			$closest_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($closest_parentid);
			if (!is_null($closest_parent)) {
				$o->setSubHeading('Currently editing '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($closest_parent->type(), 'name').' "'.$closest_parent->name.'"');
			}
		}


		  ///////////////////
		 //  SCREEN MENU  //
		///////////////////
		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';

		// add all the screens to the combo, but only if we aren't creating the object
		if (!$creating) {
			for(reset($this->_screens); null !== ($code = key($this->_screens)); next($this->_screens)) {
				$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
			}

			// add static screens for the asset
			foreach ($this->_edit_fns->static_screens as $code_name => $data) {
				$url = $current_location.'asset_ei_screen='.rawurlencode($code_name);
				$o->addStaticScreen($url, $data['name']);
			}
		} else {
			// if we are creating open the create file to see which fields we are supposed to print out
			list($create_sections, $active_screen) = $this->_getCreateSections(true);
			if (!count($create_sections)) return;
		}

		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));


		if ($file) {
			$input = new XML_Tree($file);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				trigger_error('Unable to Read File : '.$file."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
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
						if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
							$print_commit_button = true;
						}
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
							if ($field_access & SQ_EI_WRITE) $print_commit_button = true;

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
								if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
									$print_commit_button = true;
								}
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

		// we have printed an editable interface on a static screen - show the commit button
		if ($print_commit_button) {
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
					if (confirm("The status of '<?php echo $owner->name; ?>' has changed from <?php echo get_status_description($old_status); ?> to <?php echo getStatusDescription($owner->status); ?>. Do you want to view a dependant links report to view the status of the dependant assets below '<?php echo $owner->name; ?>'?")) {
						document.location = '<?php echo $owner->getBackendHref('dependants'); ?>';
					}
				</script>
				<?php
			}

			// try and find the file to print user defined
			$file = @$this->_findFile('screen_'.$active_screen);

		} else {

			// must be a normal screen
			if (empty($_GET['asset_ei_screen']) || empty($this->_screens[$_GET['asset_ei_screen']])) {
				trigger_error('Active Screen unknown unable to process', E_USER_WARNING);
				return false;
			}

			// try and find the file to print user defined
			$file = $this->_findFile('screen_'.$active_screen);
			if(!$file) return false;

		}// end if

		if ($file && $owner->writeAccess('')) {
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

		if ($ret_val) {
			// some attributes have been updated, so save the asset
			return $owner->saveAttributes();
		} else {
			return false;
		}

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

		// check show if condition for this node
		if (isset($node->attributes['show_if'])) {
			$show_if_fn = $node->attributes['show_if'];
			$edit_fns =& $owner->getEditFns();

			if (method_exists($edit_fns, $show_if_fn)) {
				if (!$edit_fns->$show_if_fn($owner)) return 0;
			} else {
				// if show if function doesn't exist, spew, then continue as normal
				trigger_error('Error : Show If function for section '. $node->name. ' not found.', E_USER_WARNING);
				return 0;
			}
		}

		// if there is nothing specified, and if an inherited access was specified we must use that
		if (empty($node->attributes['write_access']) && !is_null($inherit_access)) {
			if ($inherit_access & SQ_EI_WRITE)  $access |= SQ_EI_WRITE;

		// otherwise attempt to find write access
		} else {
			$wa = (empty($node->attributes['write_access'])) ? 'write:all' : $node->attributes['write_access'];
			// all these write access checks must have a lock type to use
			if (preg_match('/^(.+):(.*)$/', $wa, $matches)) {
				// got to have writeAccess at least (this checks for locks as well)
				if ($owner->writeAccess($matches[2])) {
					switch($matches[1]) {
						case 'root' :
							if ($GLOBALS['SQ_SYSTEM']->userRoot()) $access |= SQ_EI_WRITE;
							break;
						case 'sys_admin' :
							if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) $access |= SQ_EI_WRITE;
							break;
						case 'admin' :
							if ($owner->adminAccess($matches[2])) $access |= SQ_EI_WRITE;
							break;
						case 'write' :
							$access |= SQ_EI_WRITE; // this was checked above
							break;

						case 'none' :
							// meaning that this attribute is not to be written
							break;

					}// end switch
				}// end if write access
			}// end if preg

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
					case 'read' :
						if ($owner->readAccess()) $access |= SQ_EI_READ;
						break;
					default :
						// if it's admin or write access, there needs to be a lock type to use
						if (preg_match('/^(admin|write):(.*)$/', $ra, $matches)) {
							if ($matches[1] == 'admin') {
								if ($owner->adminAccess($matches[2])) $access |= SQ_EI_READ;
							} else {
								if ($owner->writeAccess($matches[2])) $access |= SQ_EI_READ;
							}
						}

				}// end switch
			}// end if

		}// end if

#		pre_echo($node->name.' -> '.$access);

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
			trigger_error('Unable to Read File : '.$file."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
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
						if ($field->children[0]->name != 'display_name') $display_name = 'Change ?';
						else $display_name = $field->children[0]->content;
						$o->openField($display_name);
							// if this asset is in workflow we do not let the change the status here
							// because they need to go off to the workflow screen
							$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
							$running_schemas = $wfm->getSchemas($owner->id, true, true);
							if (!empty($running_schemas)) {
								?><i>This asset is currently in workflow. Approval or rejection of the changes must be done on the <a href="<?php echo $owner->getBackendHref('workflow'); ?>">asset workflow screen</a></i><?php
							} else {
								$available_statii = $this->_paintStatusChange($owner, $o, $prefix);
								if (!empty($available_statii)) $print_commit_button = true;
							}
						$o->closeField();
					break;

					case 'future_status' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
						$fs_jobs = &Cron_Job_Future_Status::getActiveJobs($owner);
						if (!empty($fs_jobs)) {
							$o->openField('Future Statuses', 'new_line');
							?>
							<table class="sq-backend-table">
								<tr>
									<td class="sq-backend-table-header">Status</td>
									<td class="sq-backend-table-header">When</td>
									<td align="center" class="sq-backend-table-header">Delete ?</td>
								</tr>
							<?php

							for($i = 0; $i < count($fs_jobs); $i++) {
							?>
								<tr>
									<td class="sq-backend-table-cell">
										<?php echo $fs_jobs[$i]->statusName(); ?>
									</td>
									<td class="sq-backend-table-cell">
										<?php echo $fs_jobs[$i]->readableWhen(); ?>
									</td>
									<td align="center" class="sq-backend-table-cell">
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

						if ($owner->writeAccess('')) {
							$o->openField('New Future Statuses', 'new_line');
							$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
							$fs = new Cron_Job_Future_Status();
							$edit_fns = $fs->getEditFns();
							$edit_fns->paintInLineBackend($fs, $o, $prefix.'_new_fs');

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
	* Paints the interface for editing the status of an asset
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form vars
	*
	* @return array of statii shown in the drop down
	* @access public
	*/
	function _paintStatusChange(&$owner, &$o, $prefix)
	{
		$statii = $owner->getAvailableStatii();

		if (!empty($statii)) {
			$statii = array_reverse($statii, true);
			$statii[0] = '';
			$statii = array_reverse($statii, true);
			combo_box($prefix.'_change_status', $statii, false, 0);
		}

		return $statii;

	}//end _paintStatusChange()


	/**
	* Processes the interface for editing the status of an asset
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function _processStatusChange(&$owner, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_change_status']) && (int) $_POST[$prefix.'_change_status'] > 0) {

			//// START HIPO PROCESSING ////
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'new_status' => $_POST[$prefix.'_change_status']);
			$hh->queueHipo('hipo_job_edit_status', $vars);
			////  END HIPO PROCESSING  ////
		}

	}//end _processStatusChange()


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

		// process a status change on this screen if workflow is NOT running
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($owner->id, true, true);
		if (empty($running_schemas)) $this->_processStatusChange($owner, $o, $prefix);


		///// FUTURE STATUS CHANGES ////

		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (!is_null($cron_mgr)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

			if (!empty($_POST[$prefix.'_remove_fs'])) {
				$fs_jobs = &Cron_Job_Future_Status::getActiveJobs($owner);
				if (!empty($fs_jobs)) {

					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
						for($i = 0; $i < count($fs_jobs); $i++) {
							if (in_array($fs_jobs[$i]->id, $_POST[$prefix.'_remove_fs'])) {
								if ($cron_mgr->removeJob($fs_jobs[$i])) {
									$is_updated = true;
								}
							}// end if
						}// end for

						$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

					} else {
						trigger_error('Unable to acquire the lock on the Cron Manager, unable to remove jobs', E_USER_NOTICE);

					}// end if

				}// end if
			}// end if


			if ($owner->writeAccess('')) {

				$fs = new Cron_Job_Future_Status();
				$edit_fns = $fs->getEditFns();
				if ($edit_fns->processInLineBackend($fs, $o, $prefix.'_new_fs')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					if ($cron_mgr->addJob($fs, $GLOBALS['SQ_SYSTEM']->user)) {
						if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($fs->id, 'links')) {
							if ($fs->setAssetToUpdate($owner)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
								$is_updated = true;
								$GLOBALS['SQ_SYSTEM']->am->releaseLock($fs->id, 'links');
							} else {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							}
						} else {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						}
					} else {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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

		$created = false;

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
						if ($owner->adminAccess('all')) {
							$created = true;
							
							if ($field->children[0]->name != 'display_name') $display_name = 'Update Major Version ?';
							else $display_name = $field->children[0]->content;
							$o->openField($display_name);
							check_box($prefix.'_version_update_version');
						} else {
							$print_note = false;
						}
					break;

					case 'force_secure' :
						if ($field->children[0]->name != 'display_name') $display_name = 'Force Secure';
						else $display_name = $field->children[0]->content;
						$o->openField($display_name);
						$current_langs = $owner->getLanguages();
						// have to have admin access to set the force secure
						if ($owner->adminAccess('settings')) {
							$created = true;
							check_box($prefix.'_force_secure', '1', ($owner->force_secure == '1'));
						} else {
						?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($owner->force_secure) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
						<?php
						}
					break;


					case 'intl_languages' :
						if ($field->children[0]->name != 'display_name') $display_name = 'Languages';
						else $display_name = $field->children[0]->content;
						$o->openSection($display_name);

						require SQ_FUDGE_PATH.'/standards_lists/languages.inc';

						if ($owner->writeAccess('settings')) {
							$created = true;
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
							<table class="sq-backend-table" style="width: 0%;">
								<?php if ($owner->writeAccess('settings')) {
								?>
								<tr>
									<td class="sq-backend-table-header">&nbsp;</td>
									<td class="sq-backend-table-header"><b>Delete ?</b></td>
								</tr>
								<?php } ?>
							<?php
							foreach ($current_langs as $lang_code) {
								?>
								<tr>
									<td class="sq-backend-table-cell"><?php echo $standards_lists_languages[$lang_code]; ?></td>
									<?php if ($owner->writeAccess('settings')) {
										$created = true;
									?>
									<td class="sq-backend-table-cell" align="center"><?php check_box($prefix.'_rem_intl_language['.$lang_code.']'); ?></td>
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

						if ($owner->writeAccess('settings')) {
							$created = true;
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

						if ($owner->adminAccess('all')) {
							$created = true;
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

					case 'designs' :
						$system_designs = Array(
												'system_design::frontend' => 'The Design that is to be used when displaying this asset on the frontend',
												'system_design::login'    => 'The Design that is to be used whenever someone attempts to access this asset without permission',
											);
						$our_designs = Array(
												'system_design::frontend' => 0,
												'system_design::login'    => 0,
											);

						$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_LINK_TYPE_3, 'design', false);
						foreach($our_design_links as $link) {
							if (preg_match('/^(system_design|user_design)::.*$/', $link['value'])) {
								$our_designs[$link['value']] = $link['minorid'];
							}// end if
						}// end foreach

						// now add the blank one that they can customise
						$our_designs['user_design::'] = 0;

						$design_lookups = $owner->getDesignLookups();

						foreach($our_designs as $design_name => $designid) {

							preg_match('/^(system|user)_design::(.*)$/', $design_name, $matches);
							$system_design   = ($matches[1] == 'system');
							$new_user_design = (!$system_design && $design_name == 'user_design::' && !$designid && $owner->writeAccess('links'));
							if (!$system_design && $design_name == 'user_design::' && !$new_user_design) continue;

							if (!$new_user_design) {
								$display_name = ucwords($matches[1]).' Defined - '.ucwords($matches[2]).' Design';
							} else {
								$display_name = 'Create New User Defined Design';
							}
							$o->openSection($display_name);

							if ($new_user_design) {
								$o->openField('Name');
								text_box($prefix.'_designs['.$design_name.'][name]', '', 20, 30);
							} else {
								hidden_field($prefix.'_designs['.$design_name.'][name]', $design_name);
							}

							$o->openField('Current');

							if (!$owner->writeAccess('links')) {
								$created = true;
								echo '<i>';
								if (!$designid) {
									echo 'None';
								} else {
									$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($designid, '', true);
									echo $design->name.' (Id : #'.$design->id.')';
								}
								echo '</i>';

							} else {
								asset_finder($prefix.'_designs['.$design_name.'][designid]', $designid, Array('design' => 'D'));
							}


							if ($system_design && isset($system_designs[$design_name])) {
								$o->note($system_designs[$design_name]);
							}

							$o->closeField();
							if (!$new_user_design) {

								$effective_designs = Array();
								foreach($design_lookups as $url => $designs) {
									if (empty($designs[$design_name])) continue;
									$effective_designs[$url] = $designs[$design_name];
								}


								$o->openField('Effective');

								if (!empty($effective_designs)) {
									?>
									<table class="sq-backend-table">
										<tr>
											<td class="sq-backend-table-header">URL</td>
											<td class="sq-backend-table-header">Effective Design</td>
										</tr>
									<?php
									foreach ($effective_designs as $url => $designid) {
										$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($designid, '', true);
										if (is_null($design)) continue;
										?>
										<tr>
											<td class="sq-backend-table-cell"><?php echo $url; ?></td>
											<td class="sq-backend-table-cell"><?php echo $design->name.' (Id : # '.$design->id.')'; ?></td>
										</tr>
										<?php
									}
									?>
									</table>
									<?php
								} else {
									echo 'There are no effective designs';
								}

								$o->closeField();
							}// endif
							$o->closeSection();

						}// end foreach

					break;

					default :
						continue;
					break;
				}

				if ($print_note && isset($field->children[1]) && $field->children[1]->name == 'note' && $field->children[1]->content) {
					$o->note($field->children[1]->content);
				}
			}// end for

			$o->closeSection();

		}// end for

		return $created;

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

		// this is an array of assetids that are going to need their lookups updated
		$assets_affected_lookups = Array();

		//// VERSIONING ////
		if ($owner->adminAccess('all')) {
			if (isset($_POST[$prefix.'_version_update_version'])) {
				if (!$owner->incrementVersion('major')) {
					trigger_error('Failed updating major version of "'.$owner->name.'"', E_USER_WARNING);
				}
			}
		}


		//// FORCE SECURE ////
		if ($owner->adminAccess('settings')) {
			if ($owner->setForceSecure(!empty($_POST[$prefix.'_force_secure']))) {
				$assets_affected_lookups[] = $owner->id;
			}// end if
		}// end if




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

		if ($owner->adminAccess('all') && $owner->status == SQ_STATUS_UNDER_CONSTRUCTION) {
			if (isset($_POST[$prefix.'_morph_new_type']) && !empty($_POST[$prefix.'_morph_new_type'])) {
				// check that the security key was entered correctly
				if (validate_security_key()) {
					// errors for morphing will get reported in the morph function
					$owner->morph($_POST[$prefix.'_morph_new_type']);
				} else {
					trigger_error('The security key entered was incorrect. Asset type remains unchanged.', E_USER_WARNING);
				}
			}
		}


		///// FRONTEND AND LOGIN DESIGN CHANGES ////

		if (!empty($_POST[$prefix.'_designs']) && $owner->writeAccess('links')) {

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$designs_ok = true;
			$designs_changed = false;

			foreach ($_POST[$prefix.'_designs'] as $design_name => $design_data) {

				$new_designid = $design_data['designid'];

				if ($design_name == 'user_design::') {
					if (empty($design_data['name'])) continue;
					$old_designid = 0;
					$design_name .= strtolower($design_data['name']);

					if ($GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_TYPE_3, 'design', false, $design_name)) {
						trigger_error('Design Name "'.$design_data['name'].'" already in use', E_USER_WARNING);
						continue;
					}// end if

				} else {
					$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_TYPE_3, 'design', false, $design_name);
					$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

				}// end if

				// if they are the same then there is nothing to do.
				if ($old_designid == $new_designid) continue;

				// if there is an existing entry delete it
				if ($old_designid) {
					if (!$owner->deleteLink($old_design_link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = false;
						break;
					}
					$assets_affected_lookups[] = $old_designid;
				}

				if ($new_designid) {
					$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
					if (is_null($design)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = false;
						break;
					}
					if (!$owner->createLink($design, SQ_LINK_TYPE_3, $design_name)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = false;
						break;
					}
				}

				$designs_changed = true;

			}// end foreach

			if ($designs_ok) {
				if ($designs_changed) $assets_affected_lookups[] = $owner->id;
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}


		}// end if

		if (!empty($assets_affected_lookups)) {
			//// START HIPO PROCESSING ////
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetids' => array_unique($assets_affected_lookups));
			$hh->queueHipo('hipo_job_update_lookups', $vars);
			////  END HIPO PROCESSING  ////
		}// endif

		return true;

	}//end processSettings()

	
	/**
	* Paint the the access for permissions on a particular asset (used in popups etx)
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function paintInlinePermissions(&$owner, &$o)
	{
		$prefix = $owner->getPrefix();
		$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);

		if (empty($perms)) return false;

		$o->openSection('Permissions');
			$o->openField('&nbsp;');
			?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
						  <?php echo 'User'; ?>
						</td>
						<td class="sq-backend-table-header">
						  <?php echo 'Permission'; ?>
						</td>
						<td class="sq-backend-table-header" align="right">
						  <?php echo 'Access'; ?>
						</td>
					</tr>
			<?php

		foreach ($perms as $title => $perm) {
			
			$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);

			$direct = Array();
			foreach($permissions as $userid => $access) {
				if (!isset($direct[$access])) $direct[$access] = Array();
				$direct[$access][] = $userid;
			}
			ksort($direct);
			if (empty($direct)) continue;

			if (!empty($direct)) {
				foreach ($direct as $access => $userids) {
					foreach($userids as $userid) {
						if ($userid) {
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
							if (!$user->id || (!is_a($user, 'user') && !is_a($user, 'user_group'))) continue;
						}
						?>
						<tr>
							<td class="sq-backend-table-cell">
						<?php
							echo (($userid) ? $user->name.' ['.$user->type().' : '.$userid.']' : 'Public Access');
						?>
							</td>
							<td class="sq-backend-table-cell">
								<?php echo $title.' Access'?>
							</td>
							<td class="sq-backend-table-cell" align="right">
								<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>; font-weight:bold;"><?php echo (($access == '0') ? 'Denied' : 'Granted') ?></span>
							</td>
						</tr>
					<?php
					}// end foreach data
				}//end foreach inherited
			}// end if
		}// end foreach $perms
		?>
		</table>
		<?php
			$o->closeField();
		$o->closeSection();

		return true;
	
	}// end paintInlinePermissions()


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

		if ($allow_edit) $admin_access = $owner->adminAccess('permissions');
		else             $admin_access = false;

		foreach ($perms as $title => $perm) {
			$o->openSection($title.' Access');

			$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);

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
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header">
								<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Granted') ?></span>
							</td>
							<?php
							if ($admin_access) {
								?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td><?php
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
							<td class="sq-backend-table-cell">
						<?php
							echo (($userid) ? $user->name.' ['.$user->type().' : '.$userid.']' : 'Public Access');
						?>
							</td>
						<?php
							if ($admin_access) {
								?><td align="center" width="100" class="sq-backend-table-cell"><?php
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
					echo ' Specific Access ';
					asset_finder($prefix.'_permissions['.$perm.'][new][userid]', 0, Array('user' => 'D', 'user_group' => 'D'));
				$o->closeField();
			}// end if

			$o->closeSection();
		}// end foreach $perms

		if ($admin_access) {
			$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
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
		if ($owner->id && !$owner->adminAccess('')) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the permissions for <i>'.$owner->name.'</i>');
			exit();
		}

		// if we dont have admin access, dont process anything
		if (!$owner->adminAccess('permissions')) return false;

		$permission_vars = Array();
		$prefix = $owner->getPrefix();
		if (!isset($_POST[$prefix.'_permissions'])) return false;

		// are we cascading these permission changes to our children?
		$cascade_changes = !empty($_POST[$prefix.'_permissions']['cascade_changes']);

		$perms = Array('Administrator' => SQ_PERMISSION_ADMIN, 'Write' => SQ_PERMISSION_WRITE, 'Read' => SQ_PERMISSION_READ);
		foreach ($perms as $title => $perm) {
			// we need to check for write access each time because
			// we may actually be revoking write access and then
			// trying to put it back
			if (!$owner->adminAccess('permissions')) continue;

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
					if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($owner->id, 0, $perm, $post_data['public_access']['access'])) return false;
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
						if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($owner->id, $post_data['new']['userid'], $perm, $post_data['new']['access'])) return false;

					}
				}
			}

			// revoke [delete] access (grant or deny can be revoked)
			if (!empty($post_data['delete'])) {
				foreach ($post_data['delete'] as $userid => $on) {
					$userid = (int)$userid;
					if ($cascade_changes) {
						$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);
						$current_access = $current[$userid];
						$permission_vars[$perm][] = Array('access'          => -1,
														  'userid'          => $userid,
														  'previous_access' => $current_access,
														  );
					} else {
						if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($owner->id, $userid, $perm)) return false;
					}
				}
			}

			if (!empty($_POST[$prefix.'_permissions']['manual_cascade'])) {
				$current = $GLOBALS['SQ_SYSTEM']->am->getPermission($owner->id, $perm, null, false, false, true);
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
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'permission_changes' => $permission_vars);
			$hh->queueHipo('hipo_job_edit_permissions', $vars);
			////  END HIPO PROCESSING  ////

		}//end if we have permission to set

		return true;

	}//end processPermissions()


	/**
	* Paints the interface for viewing current workflows (used for popups etc)
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function paintInlineWorkflow(&$owner, &$o)
	{
		$prefix = $owner->getPrefix();

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($owner->id);

		if (empty($schemas)) return false;

		$o->openSection('Workflow Schemas');

		$direct = Array();
		foreach($schemas as $schemaid => $access) {
			if (!isset($direct[$access])) $direct[$access] = Array();
			$direct[$access][] = $schemaid;
		}
		ksort($direct);
		$o->openField('&nbsp;');
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
						Work Flow Schema
					</td>
					<td class="sq-backend-table-header" align="right">
						Access
					</td>
				</tr>
		<?php

		if (!empty($direct)) {
			foreach ($direct as $access => $schemaids) {
				foreach($schemaids as $schemaid) {
					if ($schemaid) {
						$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (!$schema->id || !is_a($schema, 'workflow_schema')) continue;
					}
				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
						?>
						</td>
						<td class="sq-backend-table-cell" align="right">
							<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>; font-weight: bold;"><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
						</td>
					</tr>
				<?php
				}// end foreach data
			}//end foreach inherited
		
		}// end if
			?></table><?php
			$o->closeField();
		$o->closeSection();
		
		return true;

	}//end paintInlineWorkflow()


	/**
	* Paints the interface for managing workflow
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$allow_edit	allow editing of permissions if user has access
	*
	* @access public
	*/
	function paintWorkflow(&$owner, &$o, $allow_edit=true)
	{
		$prefix = $owner->getPrefix();

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($owner->id);
		$running_schemas = $wfm->getSchemas($owner->id, true, true);

		// print any currently running workflows
		if (!empty($running_schemas)) {
			// cant edit workflow when workflow is currently running
			$admin_access = false;

			$o->openSection('Workflow for this asset is currently in progress');
				$o->openField('Change Status?');
					$available_statii = $this->_paintStatusChange($owner, $o, $prefix);
					?><span class="sq-backend-fineprint"><?php
					if (!empty($available_statii)) {
						echo '<br/>Change the status to approve or reject the changes made to this asset';
					} else {
						echo '<i>You cannot approve or reject changes at this time</i>';
					}
					?></span><?php
				$o->closeField();
				if (!empty($available_statii)) {
					$o->openField('Log Message');
						text_area($prefix.'_workflow_new_log_message', '', 60, 6);
						echo '&nbsp;';
						normal_button($prefix.'_workflow_status_update', 'Commit', 'submit_form()');
						?>
						<br/>You can enter a reason for your decision in the box above so that other approvers know why you have accepted or rejected the changes.
						<br/><a href="<?php echo $owner->getBackendHref('logs'); ?>&sq_backend_log_type=asset.workflow&sq_workflow_log_version=<?php echo substr($owner->version, 0, strrpos($owner->version, '.'));?>">Click here to view the workflow message log</a> - a list of system generated workflow messages as well as any comments made by other approvers.
						<?php
					$o->closeField();
				}

			$o->closeSection();

			$schema_workflows = $wfm->getSchemaWorkflows($owner->id);
			foreach ($schema_workflows as $schemaid => $workflow) {
				if (!in_array($schemaid, $running_schemas)) continue;

				$schema_title = "Running Workflow - ".$workflow['schema_name'];
				if (isset($workflow['complete']) && $workflow['complete']) {
					$schema_title .= ' <span style="color:#008800">[Completed]</span>';
				} else {
					$schema_title .= ' <span style="color:#880000">[Not Completed]</span>';
				}
				$o->openSection($schema_title);

				$o->openField('Workflow Started');
					echo $GLOBALS['SQ_SYSTEM']->datetime($workflow['started']);
				$o->closeField();
				$o->openField('Time Running');
					require_once SQ_FUDGE_PATH.'/general/datetime.inc';
					echo easy_time_total(time() - $workflow['started'], true);
				$o->closeField();

				foreach ($workflow['steps'] as $stepid => $step_data) {
					$section_title = "Step $stepid - ".$step_data['step_name'];
					if (!$workflow['current_step'] || $workflow['current_step'] > $stepid) {
						$section_title .= ' <span style="color:#008800">[Completed]</span>';
					} else {
						$section_title .= ' <span style="color:#880000">[Not Completed]</span>';
					}

					$o->openSection($section_title);

					if ($step_data['started']) {
						$o->openField('Step Duration');
							echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['started']);
							echo ' - ';
						if ($step_data['completed']) echo $GLOBALS['SQ_SYSTEM']->datetime($step_data['completed']);
						else echo '??';
						echo ' (<i>';
						require_once SQ_FUDGE_PATH.'/general/datetime.inc';
						if ($step_data['completed']) {
							echo easy_time_total($step_data['completed'] - $step_data['started'], true);
						} else {
							echo easy_time_total(time() - $step_data['started'], true);
						}
						echo '</i>)';
					}

					$o->openField('Step Logic');
					if (isset($wfm->_valid_cond_logic[$step_data['logic']])) {
						// a special type of logic statement
						$step_logic = $step_data['logic'];
						$step_logic_num = '';
					} else {
						if ($step_data['logic'] != 'all') {
							list($step_logic, $step_logic_num) = explode(' ', $step_data['logic']);
						} else {
							$step_logic = 'all';
							$step_logic_num = '';
						}
					}

					echo '<i>'.$wfm->_valid_step_logic[$step_logic];
					if ($step_logic_num) echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '')." must be met</i>";

					$o->openField('Conditions');

					$has_conds = false;
					ob_start();
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header">&nbsp;</td>
							<td class="sq-backend-table-header" width="270"><b>Group Logic</b></td>
							<td align="center" width="100" class="sq-backend-table-header"><b>Complete ?</b></td>
						</tr>
					<?php
					foreach ($step_data['conditions'] as $pub => $cond_data) {
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
						if (!$asset->id) continue;
						?>
						<tr>
							<td class="sq-backend-table-cell"><?php echo '&nbsp;'.$asset->name.' ['.$asset->type()." : $pub]";?></td>
							<td class="sq-backend-table-cell">
						<?php
						if (isset($wfm->_valid_cond_logic[$cond_data['logic']])) {
							// a special type of logic statement
							$cond_logic = $cond_data['logic'];
							$cond_logic_num = '';
						} else {
							if ($cond_data['logic'] != 'all') {
								list($cond_logic, $cond_logic_num) = explode(' ', $cond_data['logic']);
							} else {
								$cond_logic = 'all';
								$cond_logic_num = '';
							}
						}

						if (is_a($asset, 'user')) {
							echo '&nbsp;';
						} else {
							echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
							if ($cond_logic_num) echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '')." must approve</i>";
						}

						?></td>

							<td align="center" class="sq-backend-table-cell">
							<?php
								if (isset($cond_data['complete']) && $cond_data['complete']) {
									echo '<span style="color:#008800"><b>Yes</b></span>';
								} else {
									// do some extra checking
									if (is_a($asset, 'user')) {
										echo ((isset($cond_data['published_by'][$asset->id])) ? '<span style="color:#008800"><b>Yes</b></span>' : '&nbsp;');
									} else {
										if ($cond_logic_num) $user_count = $cond_logic_num;
										else $user_count = count($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false));

										echo '<span style="color:#';
										if (count($cond_data['published_by']) >= $user_count) echo '008800';
										else echo '880000';
										echo '"><b>';
										echo count($cond_data['published_by']).' of '.$user_count;
										echo '</b></span>';
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
			// there is no running workflow so admin access can be set
			if ($allow_edit) $admin_access = $owner->adminAccess('workflow');
			else              $admin_access = false;
		}

		// lets show some workflow warnings about metadata if we need to
		if ($owner->status & SQ_SC_STATUS_PENDING) {
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$m_complete = $mm->requiredFieldsComplete($owner->id);
			if (!$m_complete) {
				$o->openSection('Metadata Warning');
					$o->openField('&nbsp;');
						?>
						<p class="sq-backend-data"><span style="color:#880000"><b>WARNING:</b></span> "<?php echo $owner->name; ?>" is currently in an approval process but some required metadata fields are not yet compelete. These fields must be completed before "<?php echo $owner->name; ?>" can become 'Approved' to go live.</p>
						<p class="sq-backend-data">These metadata fields can be completed by any editor or administrator of "<?php echo $owner->name; ?>" by going to the <a href="<?php echo $owner->getBackendHref('metadata'); ?>">Metadata screen</a>.</p>
						<?php
					$o->closeField();
				$o->closeSection();
			}
		}

		$o->openSection('Workflow Schemas');

		$direct = Array();
		foreach($schemas as $schemaid => $access) {
			if (!isset($direct[$access])) $direct[$access] = Array();
			$direct[$access][] = $schemaid;
		}
		ksort($direct);

		if (!empty($direct)) {
			$o->openField('&nbsp;');
			foreach ($direct as $access => $schemaids) {
			?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td><?php
						}
						?>
					</tr>
				<?php
				foreach($schemaids as $schemaid) {
					if ($schemaid) {
						$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (!$schema->id || !is_a($schema, 'workflow_schema')) continue;
					}
				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
						?>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-cell"><?php
							check_box($prefix.'_workflow[delete]['.$schemaid.']');
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
				combo_box($prefix.'_workflow[new][access]', Array('' => '', '1' => 'Apply', '0' => 'Deny'), false, '');
				echo ' Workflow schema ';
				asset_finder($prefix.'_workflow[new][schemaid]', 0, Array('workflow_schema' => 'D'));
			$o->closeField();
		}

		$o->closeSection();

		if ($admin_access) {
			$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
			if (count($kiddies)) {
				$o->openSection('Cascade Options');
					$o->openField('Note');
						echo 'These actions will affect '.count($kiddies).' other assets below "'.$owner->name.'"';
					$o->closeField();
					$o->openField('&nbsp;');
						check_box($prefix.'_workflow[cascade_changes]', '1', true);
						echo ' Cascade schema changes';
						echo '<br/>';
						check_box($prefix.'_workflow[manual_cascade]');
						echo ' Manually cascade schemas';
					$o->closeField();
				$o->closeSection();
			}
		}

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
		$updated = false;
		$prefix = $owner->getPrefix();

		// process a status change on this screen if workflow is running
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($owner->id, true, true);
		if (!empty($running_schemas)) {
			$this->_processStatusChange($owner, $o, $prefix);
			$updated = true;
		}

		// first we need to process any messages that may have been submitted
		if (isset($_POST[$prefix.'_workflow_new_log_message'])) {
			$message_body = trim($_POST[$prefix.'_workflow_new_log_message']);
			if (!empty($message_body)) {
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				$message = $ms->newMessage(Array(), 'Workflow User Log', $message_body, 'asset.workflow.userlog');
				$message->parameters['assetid'] = $owner->id;
				$message->parameters['version'] = substr($owner->version, 0, strrpos($owner->version, '.'));
				$message->send();

				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());
				$log_body = 'Workflow log message by "'.$user->name.'" added successfully to asset "'.$owner->name.'"';
				$log = $ms->newMessage(Array(), 'User Log Message Added', $log_body, 'backend');
				$log->parameters['assetid'] = $owner->id;
				$log->send();
				$updated = true;
			}
		}

		// from here on in, if we dont have admin access - piss off
		if ($owner->id && !$owner->adminAccess('')) {
			if (!$updated) {
				$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the workflow schemas for <i>'.$owner->name.'</i>');
				exit();
			}
			return $updated;
		}

		// if we dont have admin access, dont process anything
		if (!$owner->adminAccess('workflow')) return $updated;

		if (!isset($_POST[$prefix.'_workflow'])) return $updated;

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		// are we cascading these schema changes to our children?
		$cascade_changes = false;
		if (isset($_POST[$prefix.'_workflow']['cascade_changes']) && $_POST[$prefix.'_workflow']['cascade_changes']) {
			$cascade_changes = true;
		}

		$schema_vars = Array();
		$post_data = $_POST[$prefix.'_workflow'];

		// apply a schema
		if (!empty($post_data['new']['schemaid'])) {
			if (!isset($post_data['new']['access']) || $post_data['new']['access'] == '') {
				trigger_error('You need to select the type of access in order to apply schema #'.$post_data['new']['schemaid'], E_USER_NOTICE);
			} else {
				if ($cascade_changes) {
					// cascading access of some kind, so queue it up for the HIPO
					$schema_vars[] = Array('access'          => (int)$post_data['new']['access'],
										   'schemaid'        => (int)$post_data['new']['schemaid'],
										   'previous_access' => null,
										   );
				} else {
					// lets do this ourselves
					if (!$wfm->setSchema($owner->id, (int)$post_data['new']['schemaid'], (int)$post_data['new']['access'])) return $updated;
				}
			}
		}

		// revoke [deny] schema (apply or deny can be revoked)
		if (!empty($post_data['delete'])) {
			foreach ($post_data['delete'] as $schemaid => $on) {
				$schemaid = (int)$schemaid;
				if ($cascade_changes) {
					$schemas = $wfm->getSchemas($owner->id);
					$current_access = $schemas[$schemaid];
					$schema_vars[] = Array('access'          => -1,
										   'schemaid'        => $schemaid,
										   'previous_access' => $current_access,
										   );
				} else {
					if (!$wfm->deleteSchema($owner->id, $schemaid)) return $updated;
				}
			}
		}

		if (isset($_POST[$prefix.'_workflow']['manual_cascade']) && $_POST[$prefix.'_workflow']['manual_cascade']) {
			$schemas = $wfm->getSchemas($owner->id);
			foreach ($schemas as $schemaid => $access) {
				$schema_vars[] = Array('access'          => $access,
									   'schemaid'        => $schemaid,
									   'previous_access' => null,
									   );
			}
		}


		if (!empty($schema_vars)) {

			//// START HIPO PROCESSING ////
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'schema_changes' => $schema_vars);
			$hh->queueHipo('hipo_job_edit_workflow_schemas', $vars);
			////  END HIPO PROCESSING  ////

		}//end if we have schemas to set

		return true;

	}//end processWorkflow()


	/**
	* Paint the interface for viewing metadata schemas (used in popups etc)
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function paintInlineMetadataSchemas(&$owner, &$o)
	{
		$prefix = $owner->getPrefix();

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($owner->id);

		$direct = Array();
		foreach($schemas as $schemaid => $access) {
			if (!isset($direct[$access])) $direct[$access] = Array();
			$direct[$access][] = $schemaid;
		}
		ksort($direct);

		if (empty($direct)) return false;

		$o->openSection('Metadata Schemas');

		$o->openField('&nbsp;');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					Metadata Schema
				</td>
				<td class="sq-backend-table-header" align="right">
					Access
				</td>
			</tr>
		
		<?php
		foreach ($direct as $access => $schemaids) {
			foreach($schemaids as $schemaid) {
				if ($schemaid) {
					$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					if (!$schema->id || !is_a($schema, 'metadata_schema')) continue;
				}
			?>
				<tr>
					<td class="sq-backend-table-cell">
					<?php
					echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
					?>
					</td>
					<td class="sq-backend-table-cell" align="right">
						<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>; font-weight: bold; "><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
					</td>
				</tr>
			<?php
			}// end foreach data
		}//end foreach inherited
			?></table><?php
			$o->closeField();
		$o->closeSection();

		return true;

	}//end paintInlineMetadataSchemas()


	/**
	* Paint the interface for managing metadata schamas
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function paintMetadataSchemas(&$owner, &$o)
	{
		$prefix = $owner->getPrefix();

		$admin_access = $owner->adminAccess('metadata');

		$o->openSection('Metadata Schemas');

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($owner->id);

		$direct = Array();
		foreach($schemas as $schemaid => $access) {
			if (!isset($direct[$access])) $direct[$access] = Array();
			$direct[$access][] = $schemaid;
		}
		ksort($direct);

		if (!empty($direct)) {
			$o->openField('&nbsp;');
			foreach ($direct as $access => $schemaids) {
			?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header">Delete ?</td><?php
						}
						?>
					</tr>
				<?php
				foreach($schemaids as $schemaid) {
					if ($schemaid) {
						$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (!$schema->id || !is_a($schema, 'metadata_schema')) continue;
					}
				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
						?>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-cell"><?php
							check_box($prefix.'_metadata[delete]['.$schemaid.']');
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
				combo_box($prefix.'_metadata[new][access]', Array('' => '', '1' => 'Apply', '0' => 'Deny'), false, '');
				echo ' Metadata schema ';
				asset_finder($prefix.'_metadata[new][schemaid]', 0, Array('metadata_schema' => 'D'));
			$o->closeField();
		}

		$o->closeSection();

		if ($admin_access) {
			$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
			if (count($kiddies)) {
				$o->openSection('Cascade Options');
					$o->openField('Note');
						echo 'These actions will affect '.count($kiddies).' other assets below "'.$owner->name.'"';
					$o->closeField();
					$o->openField('&nbsp;');
						check_box($prefix.'_metadata[cascade_changes]', '1', true);
						echo ' Cascade schema changes';
						echo '<br/>';
						check_box($prefix.'_metadata[manual_cascade]');
						echo ' Manually cascade schemas';
					$o->closeField();
				$o->closeSection();
			}
		}

		return $admin_access;

	}//end paintMetadataSchemas()


	/**
	* Process the interface for managing metadata schemas
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @access public
	* @return boolean TRUE is there is a change, false on error or no change
	*/
	function processMetadataSchemas(&$owner, &$o)
	{
		// if we dont have admin access, piss off
		if ($owner->id && !$owner->adminAccess('')) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the metadata schemas for <i>'.$owner->name.'</i>');
			exit();
		}

		// if we dont have admin access, dont process anything
		if (!$owner->adminAccess('metadata')) return false;

		$prefix = $owner->getPrefix();
		if (!isset($_POST[$prefix.'_metadata'])) return false;

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// are we cascading these schema changes to our children?
		$cascade_changes = false;
		if (isset($_POST[$prefix.'_metadata']['cascade_changes']) && $_POST[$prefix.'_metadata']['cascade_changes']) {
			$cascade_changes = true;
		}

		$schema_vars = Array();
		$post_data = $_POST[$prefix.'_metadata'];

		// apply a schema
		if (!empty($post_data['new']['schemaid'])) {
			if (!isset($post_data['new']['access']) || $post_data['new']['access'] == '') {
				trigger_error('You need to select the type of access in order to apply schema #'.$post_data['new']['schemaid'], E_USER_NOTICE);
			} else {
				if ($cascade_changes) {
					// cascading access of some kind, so queue it up for the HIPO
					$schema_vars[] = Array('access'          => (int)$post_data['new']['access'],
										   'schemaid'        => (int)$post_data['new']['schemaid'],
										   'previous_access' => null,
										   );
				} else {
					// lets do this ourselves
					if (!$mm->setSchema($owner->id, (int)$post_data['new']['schemaid'], (int)$post_data['new']['access'])) return false;
				}
			}
		}

		// revoke [deny] schema (apply or deny can be revoked)
		if (!empty($post_data['delete'])) {
			foreach ($post_data['delete'] as $schemaid => $on) {
				$schemaid = (int)$schemaid;
				if ($cascade_changes) {
					$schemas = $mm->getSchemas($owner->id);
					$current_access = $schemas[$schemaid];
					$schema_vars[] = Array('access'          => -1,
										   'schemaid'        => $schemaid,
										   'previous_access' => $current_access,
										   );
				} else {
					if (!$mm->deleteSchema($owner->id, $schemaid)) return false;
				}
			}
		}

		if (isset($_POST[$prefix.'_metadata']['manual_cascade']) && $_POST[$prefix.'_metadata']['manual_cascade']) {
			$schemas = $mm->getSchemas($owner->id);
			foreach ($schemas as $schemaid => $access) {
				$schema_vars[] = Array('access'          => $access,
									   'schemaid'        => $schemaid,
									   'previous_access' => null,
									   );
			}
		}


		if (!empty($schema_vars)) {

			//// START HIPO PROCESSING ////
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'schema_changes' => $schema_vars);
			$hh->queueHipo('hipo_job_edit_metadata_schemas', $vars);
			////  END HIPO PROCESSING  ////

		}//end if we have schemas to set

		return true;

	}//end processMetadataSchemas()


	/**
	* Paint the interface for managing metadata values
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	*
	* @access public
	* @return boolean
	*/
	function paintMetadata(&$owner, &$o)
	{
		$prefix = $owner->getPrefix();

		$write_access = $owner->writeAccess('metadata');

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($owner->id, true);

		if (empty($schemas)) {
			$o->openSection('There are no metadata schemas applied for this asset.');
				$o->openField('&nbsp;');
				echo 'You must apply one or more schemas before entering metadata values.';
				$o->closeField();
			$o->closeField();
			return false;
		}

		if (file_exists($owner->data_path.'/metadata.php')) {
			$o->openSection('Current Metadata');
				$o->openField('&nbsp;');
					ob_start();
					require($owner->data_path.'/metadata.php');
					$metadata = ob_get_contents();
					ob_end_clean();
					echo nl2br(htmlspecialchars(trim($metadata)));
				$o->closeField();
			$o->closeSection();
		}

		$o->openSection('Enter Metadata Values');
			$o->openField('&nbsp;');
			?>
			You can use keyword replacements in place of values for metadata fields on this page.<br/>
			For a complete list of available keyword replacements <a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">click here</a>
			<br/><br/>
			<span style="color:#FF0000"> * </span>denotes a required field
			<?php
			$o->closeField();
		$o->closeSection();

		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) continue;
			$values = $mm->getSchemaMetadata($owner->id, $schema->id);
			$edit_fns = $schema->getEditFns();
			$edit_fns->paintInlineValueInterface($schema, $o, $values, $write_access);
		}

		return $write_access;

	}//end paintMetadata()


	/**
	* Process the interface for managing metadata values
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @access public
	* @return boolean TRUE is there is a change, false on error or no change
	*/
	function processMetadata(&$owner, &$o)
	{
		// if we dont have write access, dont process anything
		if (!$owner->writeAccess('metadata')) return false;

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$schemas = $mm->getSchemas($owner->id, true);

		if (!empty($schemas)) {
			$new_values = Array();
			foreach ($schemas as $schemaid) {
				$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
				$edit_fns = $schema->getEditFns();
				$edit_fns->processInlineValueInterface($schema, $new_values);
			}
			if (!$mm->setMetadata($owner->id, $new_values)) return false;
		}
		return $mm->generateContentFile($owner->id);

	}//end processMetadata()


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

		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, 'all');
		$locks_acquired = Array();
		foreach($locks as $lock_type => $lock) {
			if (!empty($lock)) $locks_acquired[] = ucwords(str_replace('_', ' ', $lock_type));
		}
		$lock_message = '';
		if (!empty($locks_acquired)) {
			$lock_message = '<span style="color:red"><b>[LOCKED]</b></span> (Types : '.implode(', ', $locks_acquired).')';
		} else {
			$lock_message = '<span style="color:green"><b>[UNLOCKED]</b></span>';
		}

		?>
		<table cellpadding="0" cellspacing="0" border="0" bgcolor="#E6E0EA" width="100%">
			<tr>
				<td class="sq-backend-data" style="background-color:#E6E0EA">
					<table cellpadding="0" cellspacing="0" border="0">
						<tr>
							<td class="sq-backend-data" style="background-color:#E6E0EA"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="20" border="0" alt="" /></td>
							<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
								<a href="<?php echo $owner->getBackendHref('dependants'); ?>"><b><?php echo $owner->name; ?></b></a>
							</td>
						</tr>
					</table>
				</td>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" valign="top" nowrap style="background-color:#E6E0EA">
					<?php echo $lock_message; ?>
				</td>
				<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" colspan="3" nowrap style="background-color:#E6E0EA">
					<?php echo $owner->getStatusDescription(); ?>
				</td>
				<td width="100%">&nbsp;</td>
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
	* @param object	Asset				$owner				the asset whose interface we are painting
	* @param object	Asset				$owner				the asset whose dependants we are painting
	* @param object	Backend_Outputter	$o					the outputter class
	* @param int						$levels				how many levels down are we
	* @param array(int)					$print_stalks_at	what levels to print stalks at
	*
	* @return void
	* @access public
	*/
	function _recursePaintDependants(&$owner, &$asset, &$o, $levels=1, $print_stalks_at=Array())
	{
		$dependant_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		$num_kids = count($dependant_links);
		for ($i = 0; $i < $num_kids; $i++) {
			$dep_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_links[$i]['minorid'], $dependant_links[$i]['minor_type_code']);
			$end = ($i == $num_kids - 1);
			$stalk = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
			$bg = ($end) ? '' : $stalk;

			$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($dep_asset->id, 'all');
			$locks_acquired = Array();
			foreach($locks as $lock_type => $lock) {
				if (!empty($lock)) $locks_acquired[] = ucwords(str_replace('_', ' ', $lock_type));
			}
			$lock_message = '';
			if (!empty($locks_acquired)) {
				$lock_message = '<span style="color:red"><b>[LOCKED]</b></span> (Types : '.implode(', ', $locks_acquired).')';
			} else {
				$lock_message = '<span style="color:green"><b>[UNLOCKED]</b></span>';
			}

			$status_message = '';
			if ($dep_asset->status < $owner->status) {
				$status_message = '<span style="color:red"><b>**</b></span>';
			} else if ($dep_asset->status > $owner->status) {
				$status_message = '<span style="color:red"><b>*</b></span>';
			}

			$new_print_stalks_at = Array();

			?>
			<tr>
				<td class="sq-backend-data" style="background-color:#E6E0EA">
					<table cellpadding="0" cellspacing="0" border="0">
						<tr>
							<?php
							for ($x = 0; $x < $levels; $x++) {
								$stalk_bg = '';
								if (in_array($x, $print_stalks_at)) {
									$stalk_bg = $stalk;
									$new_print_stalks_at[] = $x;
								}
								?>
								<td class="sq-backend-data" <?php echo $stalk_bg; ?> style="background-color:#E6E0EA"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="20" height="1" alt="blank" /></td>
								<?php
							}
							?>
							<td class="sq-backend-data" <?php echo $bg; ?> style="background-color:#E6E0EA"><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
							<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
								&nbsp;<a href="<?php echo $dep_asset->getBackendHref('dependants'); ?>"><b><?php echo $dep_asset->name; ?></b></a>
							</td>
						</tr>
					</table>
				</td>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
					<?php echo $lock_message; ?>
				</td>
				<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" valign="top" nowrap style="background-color:#E6E0EA">
					<?php echo $dep_asset->getStatusDescription(); ?>
				</td>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
				<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
					<?php echo $status_message; ?>
				</td>
			</tr>
			<?php
			$child_deps = $GLOBALS['SQ_SYSTEM']->am->getLinks($dep_asset->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);

			if (!empty($bg)) $new_print_stalks_at[] = $levels;

			if (!empty($child_deps)) {
				$this->_recursePaintDependants($owner, $dep_asset, $o, ($levels+1), $new_print_stalks_at);
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
		// If we are allowed to edit, we check the write effective access, minus live edit access
		// We are ignoring live edit access because changes to links are not reliant on
		// status of the asset (because the are not save edited)
		$created = false;

		$write_access = ($allow_edit) ? $owner->writeAccess('links') :  false;

		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$o->openSection('Current Linking');
		$o->openField('&nbsp;');
		echo '<i>NOTE: <b>TYPE_1</b> links are displayed in both the frontend and backend navigation but <b>TYPE_2</b> links are only displayed in the backend navigation.</i><br/><br/>';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$assetids = Array();
		$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', true, 'minor');
		$editable_link_types = Array(SQ_LINK_TYPE_1 => link_type_name(SQ_LINK_TYPE_1),
									 SQ_LINK_TYPE_2 => link_type_name(SQ_LINK_TYPE_2)
									);

		foreach ($asset_links as $link) $assetids[$link['majorid']] = Array();

		if (empty($assetids)) {
			$result = Array();
		} else {

			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
			$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.name, a.assetid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ON cl.linkid = ct.linkid,
						 '.SQ_TABLE_RUNNING_PREFIX.'asset_link pl
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt ON pl.linkid = pt.linkid
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON a.assetid = pl.minorid
					';
			$where = 'cl.minorid IN ('.implode(', ', array_keys($assetids)).')
					  AND ct.treeid LIKE '.$concat.'
					  AND pt.treeid <= ct.treeid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$sql .= $where.'
					ORDER BY cl.linkid, ct.treeid, pt.treeid';


			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}
		}

		foreach ($asset_links as $link_data) $link_info[$link_data['majorid']][] = $link_data;

		$output_lineages = Array();
		$editing_linkids = Array();
		foreach ($result as $treeid => $link_tree) {

			// work out some values we are going to need
			$parent_data = $link_tree[count($link_tree) -1];
			$majorid = $parent_data['assetid'];

			foreach ($link_info[$majorid] as $link_data) {

				$link_type = $link_data['link_type'];
				$linkid = $link_data['linkid'];

				// dont even pretend that the root folder is going to
				// care that it is linking to this asset!!
				//if ($majorid == 1) continue;

				// work out if the user can change the link types
				// the arg $allow_edit must be set to TRUE to try the write access test
				$can_edit = ($write_access && in_array($link_type, array_keys($editable_link_types)) && !in_array($linkid, $editing_linkids));

				$asset_lineage = '';
				foreach ($link_tree as $tree_data) {
					if ($tree_data['assetid'] == $majorid) continue;
					$asset_lineage .= $tree_data['name'].' &gt; ';
				}
				$asset_lineage .= '<b>'.$parent_data['name'].'</b>';
				if (isset($output_lineages[$asset_lineage]) && !$can_edit) continue;

				ob_start();
				?>
				<tr>
					<td class="sq-backend-table-cell" style="text-align: center;">
						<?php echo $linkid; ?>
					</td>
					<td class="sq-backend-table-cell" style="text-align: center;">
						<?php
						if (!$can_edit) {
							echo link_type_name($link_type);
						} else {
							combo_box($prefix.'_link_type['.$linkid.']', $editable_link_types, false, $link_type);
							$editing_linkids[] = $linkid;
						}
						?>
					</td>
					<td class="sq-backend-table-cell"><?php echo $asset_lineage; ?></td>
				</tr>
				<?php
				$output_lineages[] = ob_get_contents();
				ob_end_clean();
			}

		}//end foreach result

		if (!count($output_lineages)) {
			echo 'None';
		} else {
			?>
			<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="text-align: center; white-space: nowrap;">Link Id</td>
				<td class="sq-backend-table-header" style="text-align: center;">Link Type</td>
				<td class="sq-backend-table-header" width="100%">Lineage</td>
			</tr>
			<?php
			foreach ($output_lineages as $null => $lineage_data) echo $lineage_data;
			?>
			</table>
			<?php
		}// endif

		$o->closeSection();

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
		$fl_jobs = &Cron_Job_Future_Lineage::getActiveJobs($owner);

		if (!empty($fl_jobs) || $write_access) {
			$created = true;
			$o->openSection('Future Linking');
			if (!empty($fl_jobs)) {
				$o->openField('', 'new_line');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							Action
						</td>
						<td class="sq-backend-table-header">
							When
						</td>
				<?php if ($write_access) { 
					$created = true;
					?>
						<td class="sq-backend-table-header" style="text-align: center;">
							Delete ?
						</td>
				<?php } // end if ?>
					</tr>
				<?php

				for($i = 0; $i < count($fl_jobs); $i++) {
				?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo nl2br($fl_jobs[$i]->readableDescription()); ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $fl_jobs[$i]->readableWhen(); ?>
						</td>
				<?php if ($write_access) { 
					$created = true;
					?>
						<td class="sq-backend-table-cell" style="text-align: center;">
						<?php
							if ($fl_jobs[$i]->canDelete()) {
								check_box($prefix.'_remove_fl[]', $fl_jobs[$i]->id);
							} else {
								echo '&nbsp;';
							}// end if
						?>
						</td>
				<?php } // end if ?>
					</tr>
				<?php
				}// end foreach

				?>
				</table>
				<br/>
				<?php

			}// end if

			if ($write_access) {
				$created = true;
				$fl = new Cron_Job_Future_Lineage();
				$fl->setAssetInLink($owner, 'minor');
				$edit_fns = $fl->getEditFns();
				$edit_fns->paintInLineBackend($fl, $o, $prefix.'_new_fl');
			}// end if

			$o->closeSection();
		}// end if

		return $created;

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
		if ($owner->id && !$owner->writeAccess('')) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the links of <i>'.$owner->name.'</i>');
			exit();
		}

		// if we dont have effective write access, dont process anything
		if (!$owner->writeAccess('links')) return false;

		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		$done_something = false;
		$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', true, 'minor');

		foreach ($asset_links as $link) {
			if (!isset($_POST[$prefix.'_link_type'][$link['linkid']])) continue;
			$new_type = $_POST[$prefix.'_link_type'][$link['linkid']];
			if ($link['link_type'] == $new_type) continue;
			if (!$GLOBALS['SQ_SYSTEM']->am->_updateLink($link['linkid'], $new_type)) continue;
			$done_something = true;
		}

		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (!is_null($cron_mgr)) {

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');

			if (!empty($_POST[$prefix.'_remove_fl'])) {
				$fl_jobs = &Cron_Job_Future_Lineage::getActiveJobs($owner);
				if (!empty($fl_jobs)) {

					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
						for($i = 0; $i < count($fl_jobs); $i++) {
							if (in_array($fl_jobs[$i]->id, $_POST[$prefix.'_remove_fl'])) {
								if ($cron_mgr->removeJob($fl_jobs[$i])) {
									$is_updated = true;
								}
							}// end if
						}// end for

						$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

					} else {
						trigger_error('Unable to acquire the lock on the Cron Manager, unable to remove jobs', E_USER_NOTICE);

					}// end if

				}// end if
			}// end if

			$fl = new Cron_Job_Future_Lineage();
			$edit_fns = $fl->getEditFns();
			if($fl->setAssetInLink($owner, 'minor') && $edit_fns->processInLineBackend($fl, $o, $prefix.'_new_fl')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				if ($cron_mgr->addJob($fl, $GLOBALS['SQ_SYSTEM']->user)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$done_something = true;
				} else {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				}

			}// end if

		}// end if

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
		if (!$owner->writeAccess('')) return false;

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
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Version Duration</td>
				<td class="sq-backend-table-header">Time Since Version</td>
				<td class="sq-backend-table-header" align="center">Version</td>
				<td class="sq-backend-table-header" align="center">View</td>
			</tr>
		<?php

		foreach ($result as $data) {
			$from = $data['efrom'];
			$to   = $data['eto'];

			$class = 'sq-backend-table-cell';
			$show_view_btn = true;
			if (SQ_ROLLBACK_VIEW) {
				$rollback_from = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
				if ($rollback_from >= strtotime($from) && (empty($to) || strtotime($to) > $rollback_from)) {
					$class .= '-alt';
					$show_view_btn = false;
				}
			}

			if (empty($to)) {
				$to = 'present';
				$ago = 'Most Recent';
				$show_view_btn = false;
			} else {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$ago = easy_time_total(time() - strtotime($from), true);
				list($fd, $ft) = explode(' ', $from);
				list($td, $tt) = explode(' ', $to);
				if ($fd == $td) $to = $tt;
				else $to = date('d/m/Y H:i:s', strtotime($to));
			}

			?>
			<tr>
				<td class="<?php echo $class; ?>"><?php echo date('d/m/Y H:i:s', strtotime($from)); ?> - <?php echo $to; ?></td>
				<td class="<?php echo $class; ?>"><?php echo $ago; ?></td>
				<td class="<?php echo $class; ?>" align="center"><?php echo $data['version']; ?></td>
				<td class="<?php echo $class; ?>" align="center">
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
	* Paints the interface for viewing logged information about the asset
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintLogs(&$owner, &$o)
	{
		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql   = 'SELECT DISTINCT type FROM '.SQ_TABLE_RUNNING_PREFIX.'internal_message ';
		$where = 'userto = 0 AND type LIKE '.$db->quote('asset%');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$log_types = $db->getCol($sql.$where);
		if (DB::isError($log_types)) {
			trigger_error($log_types->getMessage().'<br/>'.$log_types->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$active_log_type = (isset($_REQUEST['sq_backend_log_type'])) ? $_REQUEST['sq_backend_log_type'] : $log_types[0];
		$o->addFormActionGetVar('sq_backend_log_type', $active_log_type);
		$o->openSection('Logged Messages');
			$o->openField('Log Type');
				$type_options = Array();
				foreach ($log_types as $type) $type_options[$type] = ucwords(str_replace('.', ' ', $type));
				combo_box('sq_backend_log_type', $type_options, false, $active_log_type);
				echo '&nbsp;';
				normal_button($prefix.'_go', 'View', 'submit_form()');
			$o->closeField();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		switch ($active_log_type) {

			case 'asset.linking' :
				$minor_logs = $ms->getLogs($active_log_type, Array('minorid' => $owner->id));
				$major_logs = $ms->getLogs($active_log_type, Array('majorid' => $owner->id));
				$logs = array_merge($minor_logs, $major_logs);
				usort($logs, Array('Messaging_Service', 'sortMessages'));
			break;

			case 'asset.workflow' :
				// for workflow we want to show the user log messages as well
				if (isset($_REQUEST['sq_workflow_log_version'])) {
					$o->addFormActionGetVar('sq_workflow_log_version', $_REQUEST['sq_workflow_log_version']);
					$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id,
																 'version' => $_REQUEST['sq_workflow_log_version']
																 )
								);
					$user_logs = $ms->getLogs($active_log_type.'.userlog', Array('assetid' => $owner->id,
																			   'version' => $_REQUEST['sq_workflow_log_version']
																				)
								);
				} else {
					$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id));
					$user_logs = $ms->getLogs($active_log_type.'.userlog', Array('assetid' => $owner->id));
				}
				$logs = array_merge($logs, $user_logs);
				usort($logs, Array('Messaging_Service', 'sortMessages'));
			break;

			default :
				$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id));
			break;

		}//end switch

		if (empty($logs)) {
			$o->openSection('There are no logged messages of this type for "'.$owner->name.'"');
		} else {
			$o->openField('');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header" nowrap width="120">Log Time</td>
						<td class="sq-backend-table-header" nowrap width="120">Logged By</td>
						<td class="sq-backend-table-header" width="100%">Message</td>
					</tr>
				<?php
				$num_printed  = 0;
				$num_per_page = 10;
				$num_logs = count($logs);
				$start_printing_at = 0;
				if (isset($_REQUEST[$prefix.'_start_at'])) $start_printing_at = (int) $_REQUEST[$prefix.'_start_at'];
				for ($i = 0; $i < $num_logs; $i++) {
					if ($i < $start_printing_at) {
						$i++;
						continue;
					}
					$log_data = $logs[$i];
					?>
					<tr>
						<td class="sq-backend-table-cell" nowrap><?php echo ts_iso8601($log_data['sent']); ?></td>
						<td class="sq-backend-table-cell" nowrap><?php echo $log_data['from_name']; ?></td>
						<td class="sq-backend-table-cell" width="100%"><b><?php echo $log_data['subject']; ?></b><br/><?php echo $log_data['body']; ?></td>
					</tr>
					<?php
					$num_printed++;
					if ($num_printed >= $num_per_page) break;
				}
				?>
					<tr>
						<td colspan="3" align="center" class="sq-backend-table-header">
						<?php
						hidden_field($prefix.'_start_at', $start_printing_at);
						if ($start_printing_at > 0) {
							?><a style="text-decoration: none;" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix; ?>_start_at', '<?php echo $start_printing_at - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
						} else {
							?><span style="color: #84848F"><?php
						}
						?>
						&lt;&lt; Previous Page
						<?php
						if ($start_printing_at > 0) {
							?></a><?php
						} else {
							?></span><?php
						}
						?>
						&nbsp;<b>Page <?php echo round(($start_printing_at + $num_per_page) / $num_per_page); ?> of <?php echo round(($num_logs / $num_per_page) + 0.5); ?></b>&nbsp;

						<?php
						if (($start_printing_at + $num_per_page) < $num_logs) {
							?><a style="text-decoration: none;" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix; ?>_start_at', '<?php echo $start_printing_at + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
						} else {
							?><span style="color: #84848F"><?php
						}
						?>
						Next Page &gt;&gt;
						<?php
						if (($start_printing_at + $num_per_page) < $num_logs) {
							?></a><?php
						} else {
							?></span><?php
						}
						?>
						</td>
					</tr>
				</table>
				<?php
			$o->closeField();
		}
		$o->closeSection();

		return false;

	}//end paintLogs()


	/**
	* Processes the interface for viewing logged information about the asset
	*
	* @param object	Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processLogs(&$owner, &$o)
	{
		return true;

	}//end processLogs()


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
