<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: asset_edit_interface.inc,v 1.212 2004/05/27 23:41:59 gsherwood Exp $
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

		// check that the edit functions have a details tab or else
		// we cant even create assets of this type let alone edit them
		assert_isset($this->_edit_fns->static_screens['details'], "Asset Edit Functions for '$type_code' do not define a 'details' tab");

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
		assert_false(!count($this->_screens) && empty($this->_edit_fns->static_screens));


		  /////////////////////////////
		 //  ALLOWED SCREEN ACCESS  //
		/////////////////////////////
		$check_screen_restrictions = !$creating && (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()));
		if ($check_screen_restrictions) {
			$allowed_screens = $this->getAllowedScreens($owner);
			if (isset($allowed_screens['0']['0'])) $check_screen_restrictions = false;
		}


		  /////////////////////
		 //  ACTIVE SCREEN  //
		/////////////////////
		$create_sections = Array();
		if (empty($_REQUEST['asset_ei_screen'])) {
			if ((SQ_IN_LIMBO || isset($_GET['sq_from_frontend'])) && !empty($this->_default_screen)) {
				$active_screen = $this->_default_screen;
			} else {
				$active_screen = 'details';
			}
			$_REQUEST['asset_ei_screen'] = $active_screen;
		} else {
			$active_screen = $_REQUEST['asset_ei_screen'];
		}

		if ($check_screen_restrictions && !isset($allowed_screens[$active_screen])) {
			// this user is not allowed to view the current screen
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the <i>'.$active_screen.'</i> screen of "'.$owner->name.'"');
			exit();
		}

		if (!isset($this->_edit_fns->static_screens[$active_screen]) && empty($this->_screens[$active_screen])) {
			// we cant find this screen, so we will use the details screen as a backup
			trigger_error('Active Screen ("'.$active_screen.'") unknown', E_USER_WARNING);
			$active_screen = 'details';
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

			$have_any_locks    = false;
			$have_all_locks    = true;
			$need_force_locks  = false;
			$can_force_locks   = true;
			$do_the_limbo_lock = true;
			foreach($locks as $lock_type => $lock) {

				if (empty($lock)) {
					$have_all_locks = false;
				} else {
					// this asset is currently locked
					$user    = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
					$editing = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

					if ($GLOBALS['SQ_SYSTEM']->currentUser($user)) {
						$have_any_locks = true;
					} else {
						$have_all_locks = false;
						$do_the_limbo_lock = false;

						// this lock is held by someone else so check if we can forceable acquire it
						$need_force_locks = true;
						// does the current user has a high
						// enough level of access to forceably acquire the lock
						if (!$owner->canForceablyAcquireLock($lock_type)) {
							$can_force_locks = false;
						}

					}
				}// end if

			}// end foreach lock types

			if (SQ_IN_LIMBO && $do_the_limbo_lock) {

				  /////////////////////
				 //  LIMBO LOCKING  //
				/////////////////////

				// even if we have all the locks for this screen, limbo might require more locks
				// so acquire any more locks that might be needed
				$this->acquireLimboLocks($owner, $o, $active_screen, $screen_lock_type);

			} else {

				if (strtolower($screen_lock_type) != 'none') {
					// lets tell the user if the asset is locked for editing
					// or if they can lock it to edit it
					$o->openSection('Locking / Editing');
					$o->openField('&nbsp;');

					?><div style="margin-bottom: 4px;"><?php
						if (!$have_all_locks) {
							if ($need_force_locks) {
								if ($can_force_locks) {
									echo '<div class="sq-backend-locked-by-someone-else">You can forceably acquire this lock, which will remove the lock from "'.$user->name.'" and give the lock to yourself.<br>Any changes currently being made by "'.$user->name.'" will be lost if this lock is forceably acquired.</div>';
								}
							} else if ($full_write_access && $acquire_lock_access) {
								echo 'Press the <i><b>Lock</b></i> button to lock "'.$owner->name.'" for editing. ';
							}
						}
						if ($have_any_locks) echo 'Press the <i><b>Release Lock(s)</b></i> button to release the locks you hold on "'.$owner->name.'". ';
						?>
						<b><a href="#" onClick="Javascript: sq_toggle_lock_info();" id="sq_lock_info_toggle">Show Lock Details</a></b>
					</div>

					<div id="sq_lock_info" style="display: none;">
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
								if ($full_write_access && $acquire_lock_access) {
									echo '<p class="sq-backend-unlocked">Currently not held.</p>';
								} else {
									echo 'You do not have access to edit "'.$owner->name.'".';
								}

							} else {
								// this asset is currently locked so display message to the user
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
				</div>

				<?php

				if (!$have_all_locks || $have_any_locks) {
					?><div style="margin-top: 4px;"><?php
				}

				if (!$have_all_locks) {
					if ($need_force_locks) {
						if ($can_force_locks) {

							// they aren't going to do this without a fight though
							// they'll have to go through our popup window first
							$popup_url = '?SQ_BACKEND_PAGE=main&backend_section=am&ignore_frames=1&am_section=forceably_acquire_lock&assetid='.$owner->id.'&sq_lock_type='.$screen_lock_type;

							hidden_field('sq_lock_acquire_by_force', '0');
							hidden_field('sq_lock_acquire', '0');
							normal_button('sq_forceably_acquire_lock', 'Acquire Lock(s)', "window.open('$popup_url', 'sq_forceably_acquire_lock_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300');");
							echo '&nbsp;';
						}
					} else if ($full_write_access && $acquire_lock_access) {
						hidden_field('sq_lock_acquire_by_force', '0');
						submit_button('sq_lock_acquire', 'Lock');
						echo '&nbsp;';
					}// end if
				}

				if ($have_any_locks) {
					submit_button('sq_lock_release_manual', 'Release Lock(s)');
					echo '&nbsp;';
				}

				if (!$have_all_locks || $have_any_locks) {
					?></div><?php
				}

				if (strtolower($screen_lock_type) != 'none') $o->closeSection();
				else $o->closeRaw();

			}//end if

			$o->openRaw();
				hidden_field('sq_lock_type', $screen_lock_type);
			$o->closeRaw();

		}// end if

		// We need to check if we are going to paint the layout here, so no static screens get printed
		$paint_layout = false;
		if (SQ_IN_LIMBO && !$creating) {
			$lm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			if ($lm->hasLayout($owner->type(), $active_screen)) $paint_layout = true;
		}

		// check the active screen for special system defined static screens
		$file = null;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {
			$active_screen_name = $this->_edit_fns->static_screens[$active_screen]['name'];

			// only paint the static screens if no layouts are getting painted
			if (!$paint_layout) {
				// if we are not creating, paint the system defined bit of this tab
				if (!$creating) {
					$paint_function = 'paint'.$active_screen;
					$this->includeStaticScreen($active_screen);
					if ($paint_function($owner, $o, $this)) $print_commit_button = true;
				}

				$force_unlock_on_commit = $this->_edit_fns->static_screens[$active_screen]['force_unlock'];
			}
			// try and find the file to print user defined section of the tab
			$file = @$this->_findFile('screen_'.$active_screen);
		} else {

			// try and find the file with the screen details
			if(!($file = $this->_findFile('screen_'.$active_screen))) return;

			$force_unlock_on_commit = $this->_screens[$active_screen]['force_unlock'];

			$active_screen_name = $this->_screens[$active_screen]['name'];

		}// end if


		  ///////////////////
		 //  WHERE AM I   //
		///////////////////
		if (isset($_REQUEST['sq_link_path'])) {
			$link_path = $_REQUEST['sq_link_path'];
		} else {
			$link_path = '';
		}
		$o->addFormActionGetVar('sq_link_path', $link_path, true);
		$o->addHiddenField('sq_link_path', $link_path);


		  ///////////////////
		 //  PREVIEW URL  //
		///////////////////
		// get the asset tree lineage
		$asset_path = Array();
		if (isset($_REQUEST['sq_asset_path'])) {
			// asset map sends comma-separated assetids indicating the path
			$matches = preg_split('|,([0-9]+:?),?|', ($_REQUEST['sq_asset_path']), 0, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
			for (reset($matches); null !== ($k = key($matches)); next($matches)) {
				// the first match is always fine
				if ($k == 0) {
					$asset_path[] = $matches[$k];
					continue;
				}

				// if a match ends with the shadow id seperator, the next match
				// is actually part of this match
				if (substr($matches[$k], -1) == ':') {
					$asset_path[] = $matches[$k].$matches[$k+1];
					next($matches);
					continue;
				}
		
				$asset_path[] = $matches[$k];
			}
		} else {
			if (SQ_IN_LIMBO) {
				$url_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL();
				foreach($url_lineage as $asset_info) $asset_path[] = $asset_info['assetid'];
			} else {
				// asset map didn't send one (e.g initial matrix loading)
				// pretend it did by making a singleton set of the owner
				if ($owner->id) $asset_path[] = $owner->id;
			}
		}

		$o->addFormActionGetVar('sq_asset_path', implode(',', $asset_path), true);

		$preview_url = '';
		$closest_parentid = $owner->id;

		foreach ($asset_path as $assetid) {
			if ($assetid != $owner->id) $ancestor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			else $ancestor = &$owner;

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


		  ////////////////
		 /// LAYOUTS  ///
		////////////////
		// if we are ignoring frames - normally because we are editing in limbo, we want
		// to make sure we keep this setting for the future before we use layouts to print the interface
		if (isset($_REQUEST['ignore_frames']) && $_REQUEST['ignore_frames']) {
			$o->addHiddenField('ignore_frames', '1');		
		}

		// set the current screen for both backend and frontend editing
		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';
		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));

		// once we have a preview URL, we can paint the layout if there is one
		// paint the layout now, if needed
		if ($paint_layout) {
			if (null !== ($layout = &$lm->getLayout($owner->type(), $active_screen))) {
				return $this->paintLayout($owner, $o, $layout);
			}
		}


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
			$page_heading = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name').' - '.$owner->name.' (#'.$owner->id.', v. '.$owner->version.')';
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
		// add all the screens to the combo, but only if we aren't creating the object
		if (!$creating) {
			for(reset($this->_screens); null !== ($code = key($this->_screens)); next($this->_screens)) {
				if ($check_screen_restrictions && !isset($allowed_screens[$code])) continue;
				$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
			}

			// add static screens for the asset
			foreach ($this->_edit_fns->static_screens as $code_name => $data) {
				if ($check_screen_restrictions && !isset($allowed_screens[$code_name])) continue;
				$url = $current_location.'asset_ei_screen='.rawurlencode($code_name);
				$o->addStaticScreen($url, $data['name']);
			}
		} else {
			// if we are creating open the create file to see which fields we are supposed to print out
			list($create_sections, $active_screen) = $this->_getCreateSections(true);
			if (!count($create_sections)) return;
		}


		  //////////////////////
		 //  EDIT INTERFACE  //
		//////////////////////
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
				// if we are in limbo and this section is not to be printed in limbo - dont print it
				if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) continue;

				// if this ain't a section, or if we are printing the create interface and this section
				// is not included in that, ignore it
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
								echo 'Function "'.$paint_node->attributes['name'].'" not found for painting';
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

	}//end paint()


	/**
	* Acquires any locks needed by limbo
	*
	* @param object Asset					&$owner				the owner of this interface
	* @param object Backend_Outputter		&$o					the backend outputter
	* @param string							$active_screen		the active screen we are on
	* @param string							$screen_lock_type	the lock type of this screen
	*
	* @return boolean
	* @access public
	*/
	function acquireLimboLocks(&$owner, &$o, $active_screen='details', $screen_lock_type='all')
	{
		$locks = Array($screen_lock_type);

		$lm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
		if ($lm->hasLayout($owner->type(), $active_screen)) {

			$layout = &$lm->getLayout($owner->type(), $active_screen);
			if (is_null($layout)) return false;
			$keywords = $layout->getLayoutKeywords();
			if (empty($keywords)) return false;
			if (isset($keywords['custom']) && !empty($keywords['custom'])) {
				// acquire any locks that the owner needs to paint the layout
				$owner->acquireEditingLocks($keywords['custom']);
			}

			// if there are other screens used by the layout, then we need to acquire locks on those
			// screens as well
			$screens = array_merge(array_keys($keywords['screens']), array_keys($keywords['entire_screens']));
			$screens = array_unique($screens);

			foreach ($screens as $screen) {
				// default to 'all', so if lock type is left out a lock can still be gotten and the asset edited
				$screen_lock_type = 'all';
				if (isset($this->_edit_fns->static_screens[$screen])) {
					if (!empty($this->_edit_fns->static_screens[$screen]['lock_type'])) {
						$screen_lock_type = $this->_edit_fns->static_screens[$screen]['lock_type'];
					}
				} else {
					// must be a normal screen
					if (!empty($this->_screens[$screen]['lock_type'])) {
						$screen_lock_type = $this->_screens[$screen]['lock_type'];
					}
				}// end if

				$locks[] = $screen_lock_type;

			}//end foreach
			
		}//end if hasLayout

		$locks = array_unique($locks);
		$have_all_locks = true;
		foreach ($locks as $lock_name) {
			$lock_info = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, $lock_name, true);
			foreach ($lock_info as $lock_type => $lock_data) {
				if ($lock_data['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserId()) $have_all_locks = false;
			}
		}

		if (!$have_all_locks) {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'lock_type' => $locks, 'forceably_acquire' => false);
			$hh->queueHipo('hipo_job_acquire_lock', $vars);
			$url = $hh->runQueuedJobs();
			if (!empty($url)) $o->setRedirect(strip_url(current_url()).'/?sq_redirect_url='.urlencode($url));
		}

	}//end acquireLimboLocks()


	/**
	* Prints limbo in a specified layout
	*
	* @param &object Asset				$owner		the owner of this interface
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param &object layout				$layout		the layout to be painted
	*
	* @return boolean
	* @access public
	*/
	function paintLayout(&$owner, &$o, &$layout)
	{
		$keywords = $layout->getLayoutKeywords();
		$print_commit_button = false;

		// find what screens we need to process
		if (!isset($keywords['screens'])) $keywords['screens'] = Array();
		if (!isset($keywords['entire_screens'])) $keywords['entire_screens'] = Array();

		$screens = array_merge(array_keys($keywords['screens']), array_keys($keywords['entire_screens']));
		$screens = array_unique($screens);

		$this->_tmp['layout_keyword_replacements'] = Array();

		// let the backend outputter know that we want buffering turned off
		$o->setBuffering(false);
		
		// loop over the screens and get the xml file for each screen
		// generate some keywords for each screen
		foreach ($screens as $screen) {

			// check to see if we want the entire screen
			$entire_screen = false;
			if (in_array($screen, array_keys($keywords['entire_screens']))) $entire_screen = true;

			// if we have a static screen on our hands, then we want to 
			// call the function to paint it
			if (isset($this->_edit_fns->static_screens[$screen])) {
				$static_keywords = (isset($keywords['screens'][$screen])) ? $keywords['screens'][$screen] : null;
				if (!is_null($static_keywords) || $entire_screen) {
					if (is_null($static_keywords)) $static_keywords = Array();
					if ($this->_getStaticScreenKeywordReplacements($owner, $o, $screen, $static_keywords, $entire_screen)) {
						$print_commit_button = true;
					}
				}
			}//end if

			// its not a static screen, so get the xml file and parse it for
			// the keywords that we want, if the xml file exists
			$file = @$this->_findFile('screen_'.$screen);

			if ($file) {
				$input = new XML_Tree($file);
				$root  = &$input->getTreeFromFile();
				
				if (PEAR::isError($root)) {
					trigger_error('Unable to Read File : '.$file."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
					continue;
				}//end if

				// get the layout replacements for this NON-static screen
				if ($this->_getLayoutKeywordReplacements($root, $o, $owner, $keywords, $screen, $entire_screen)) {
					$print_commit_button = true;
				}
			}//end if

		}//end foreach

		// if there are some custom keywords, get some replacements from the owner
		if (isset($keywords['custom']) && !empty($keywords['custom'])) {
			$custom_replacements = $owner->getCustomKeywordReplacements($keywords['custom']);
			
			if (!empty($custom_replacements)) {
				$replacements = Array();
				
				// rebuild the keyword so the bodycopy can properly replace it
				foreach ($custom_replacements as $keyword => $replacement) {
					$replacements['__custom-'.$keyword] = $replacement;
				}
				$this->_tmp['layout_keyword_replacements'] = array_merge($this->_tmp['layout_keyword_replacements'], $replacements);
				$print_commit_button = true;

			}//end if

		}//end if

		if (!isset($keywords['layout'])) $keywords['layout'] = Array();

		// we have printed an editable interface on a static screen - show the commit button
		if (in_array('commit_button', $keywords['layout']) && $print_commit_button) {
			ob_start();
				$o->commitButton('Commit', false);
				$this->_tmp['layout_keyword_replacements']['commit_button'] = ob_get_contents();
			ob_end_clean();
		}
		
		// show the screen selection menu
		if (in_array('screen_menu', $keywords['layout'])) {
			$lm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			$current_location = $o->getCurrentLocation();
			$current_location .= (strstr($current_location, '?')) ? '&' : '?';
			
			for(reset($this->_screens); null !== ($code = key($this->_screens)); next($this->_screens)) {
				if (!$lm->hasLayout($owner->type(), $code)) continue;
				$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
			}
			foreach ($this->_edit_fns->static_screens as $code_name => $data) {
				if (!$lm->hasLayout($owner->type(), $code_name)) continue;
				$url = $current_location.'asset_ei_screen='.rawurlencode($code_name);
				$o->addStaticScreen($url, $data['name']);
			}
			
			ob_start();
				$o->printScreenMenu();
				$this->_tmp['layout_keyword_replacements']['screen_menu'] = ob_get_contents();
			ob_end_clean();
		}

		// turn buffering back on so the output is not malformed
		$o->setBuffering(true);

		// tell the layout what the keyword replacements are
		$layout->setKeywordReplacements($this->_tmp['layout_keyword_replacements']);

		// we need to open a raw section so that the form gets printed before the contents of the layout
		$o->openRaw();
			$layout->printBody();
		$o->closeRaw();

		return true;

	}//end paintLayout()


	/**
	* Gets a keyword replacement for static screens.
	*
	* @param &object Asset				$owner				the owner of this asset
	* @param &object Backend_Outputter	$o					the backend outputter object
	* @param string						$screen				the static screen to get a replacement for
	* @param Array()|null				$static_keywords	the wantedkeyowrd of this static screen
	* @param boolean					$get_screen			if TRUE the entire screen will be captured into a replacement
	*
	* @return boolean
	* @access private
	*/
	function _getStaticScreenKeywordReplacements(&$owner, &$o, $screen, $static_keywords, $get_screen=false)
	{
		$this->includeStaticScreen($screen);
		$print_commit_button = false;
		
		ob_start();
			if (isset($_POST['am_form_submitted']) && $_POST['am_form_submitted']) {
				// the form was submitted, so we need to process the static screen as well
				$process_function = 'process'.$screen;
				$process_function($owner, $o, $this, $static_keywords);
			}
			$paint_function = 'paint'.$screen;
			if ($paint_function($owner, $o, $this, $static_keywords)) $print_commit_button = true;
			if ($get_screen) $this->_tmp['layout_keyword_replacements'][$screen.'-'] = ob_get_contents();
		ob_end_clean();

		return $print_commit_button;

	}//end getStaticScreenKeywordReplacements()


	/**
	* Gets the keyword replacements to be used in the layout
	*
	* @param &object XML_Tree			$root				the root of the xml screen
	* @param &object Backend_Outputter	$o					the backend outputter object
	* @param &object Asset				$owner				the owner of this asset
	* @param Array						$layout_keywords	an array of keywords to get replacements for
	* @param string						$screen				the name of the current screen
	* @param boolean					$get_screen			if TRUE the entire screen will be captured into a replacement
	*
	* @return boolean
	* @access private
	*/
	function _getLayoutKeywordReplacements(&$root, &$o, &$owner, $layout_keywords, $screen='details', $get_screen=false)
	{
		$prefix = $owner->getPrefix();
		$print_commit_button = false;

		// start buffering anything and trash the content at the end
		ob_start();

		for ($i = 0; $i < count($root->children); $i++) {

			$section = &$root->children[$i];

			// find out if we have permission to print this section
			$limbo_access = (isset($section->attributes['limbo_access']) && (int) $section->attributes['limbo_access'] == 1) ? true : false;

			// find out what we need to get in this loop
			$get_section = false;
			$section_name = strtolower(str_replace(' ', '_', $section->attributes['name']));
			if (isset($layout_keywords['screens'][$screen]['sections'][$section_name])) {
				$get_section = true;
				$section_keyword = $layout_keywords['screens'][$screen]['sections'][$section_name];
			}

			if (!($num_fields = count($section->children))) continue;
			$section_access = $this->_getAccess($owner, $section);
			if (!($section_access & SQ_EI_READ)) continue;

			// if we are getting this section, start buffering output
			if ($get_section) ob_start();
			if ($get_section || $get_screen) $o->openSection($section->attributes['name']);

			if ($section->children[0]->name == 'function_call' && ($get_section || $get_screen)) {

				$paint_node = &$section->children[0]->children[0];

				if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
					$o->openField('');
					echo 'Function not found for painting section "', $section->attributes['name'], '"';
					$o->closeField();
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

					// reset get_field
					$get_field = false;

					if (!empty($field->attributes['attribute'])) {

						// find out what we need to get in this loop
						if (isset($layout_keywords['screens'][$screen]['fields'][$field->attributes['attribute']])) {
							$get_field = true;
							$field_keyword = $layout_keywords['screens'][$screen]['fields'][$field->attributes['attribute']];
						}

						// if we do not want anything in this loop, continue
						if (!$get_field && !$get_section && !$get_screen) continue;

						if ($field->name != 'field') continue;
						$o->openField($field->children[0]->content, $field->attributes['format']);

						$attr = &$owner->getAttribute($field->attributes['attribute']);
						if (is_null($attr)) continue;
						if (!$attr->setEditParams($field->children[2])) continue;
						// if this is not a wanted keyword, then continue on our way
						if (isset($layout_keywords['screens'][$screen]['fields'][$field->attributes['attribute']]))

						// start buffering for this field
						if ($get_field) ob_start();

						$attr->paint($prefix, !(($field_access & SQ_EI_WRITE) && $limbo_access));
						if (($field_access & SQ_EI_WRITE) && $limbo_access) $print_commit_button = true;

						if ($field->children[1]->name == 'note' && $field->children[1]->content) {
							$field->children[1]->content = str_replace('%description%', $attr->description, $field->children[1]->content);
						}

						// end buffering
						if ($get_field) {
							$this->_tmp['layout_keyword_replacements'][$field_keyword] = ob_get_contents();
							ob_end_flush();
						}
						$o->closeField();

					} else if ($field->children[2]->name == 'function_call') {
						
						if (!isset($field->attributes['keyword'])) continue;

						// find out what we need to get in this loop
						if (isset($layout_keywords['screens'][$screen]['fields'][$field->attributes['keyword']])) {
							$get_field = true;
							$field_keyword = $layout_keywords['screens'][$screen]['fields'][$field->attributes['keyword']];
						}

						$paint_node = &$field->children[2]->children[0];
						// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
						if ($field->name != 'field') continue;

						$o->openField($field->children[0]->content, $field->attributes['format']);

						if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
							echo 'Function not found for painting';
						} else {
							if ($get_field) ob_start();

							if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
								$print_commit_button = true;
							}
							// get a replacement for this field is there is a keyword for it
							if ($get_field) {
								$this->_tmp['layout_keyword_replacements'][$field_keyword] = ob_get_contents();
								ob_end_flush();
							}
						}//end if

						$o->closeField();

					}// end if

					if ($field->children[1]->name == 'note' && $field->children[1]->content) {
						echo '<br/>&nbsp;';
						$o->note($field->children[1]->content);
					}//end if
				}//end for
			}//end if section == function_call

			if ($get_section || $get_screen) $o->closeSection();

			if ($get_section) {
				$this->_tmp['layout_keyword_replacements'][$section_keyword] = ob_get_contents();
				ob_end_flush();
			}
		}//end for

		if ($get_screen) {
			// we might have some part of a static screen in here, so append to the end of the keyword
			if (!isset($this->_tmp['layout_keyword_replacements'][$screen.'-'])) {
				$this->_tmp['layout_keyword_replacements'][$screen.'-'] = '';
			}
			$this->_tmp['layout_keyword_replacements'][$screen.'-'] .= ob_get_contents();
		}

		// trash
		ob_end_clean();

		return $print_commit_button;

	}//end getLayoutKeywordReplacements()


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
		// used to check if anything has bed saved
		$saved = false;

		// if there are no screens or static screens, piss off
		if (!count($this->_screens) && empty($this->_edit_fns->static_screens)) return false;

		$current_screen = (empty($_REQUEST['asset_ei_screen'])) ? 'details' : $_REQUEST['asset_ei_screen'];
		$screens = Array($current_screen);

		if (!$creating) {
			$lm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			$layout = &$lm->getLayout($owner->type(), $current_screen);

			// if we have a layout and there are some defined screen components, then these
			// screens also need to be processed
			if (!is_null($layout)) {
				$keywords = $layout->getLayoutKeywords();
				if (isset($keywords['screens']) && !empty($keywords['screens'])) {
					$screens = array_unique(array_merge(array_keys($keywords['screens']), $screens));
				}
				// if there are some special keywords, we need to ask the owner to process them
				if (isset($keywords['custom']) && !empty($keywords['custom'])) {
					$owner->processCustomKeywords($keywords['custom']);
				}
			}
		}


		foreach ($screens as $active_screen) {

			$ret_val = false;
			$create_sections = Array();

			// if we are creating open the create file to see which fields we are supposed to print out
			if ($creating) {

				list($create_sections, $active_screen) = $this->_getCreateSections(false);
				if (!count($create_sections)) return false;

			} else {

				  /////////////////////////////
				 //  ALLOWED SCREEN ACCESS  //
				/////////////////////////////
				if (!$owner->adminAccess('')) {
					$allowed_screens = $this->getAllowedScreens($owner);

					if (!isset($allowed_screens['0']['0']) && !isset($allowed_screens[$active_screen])) {
						// this user is not allowed to view the current screen
						$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the <i>'.$active_screen.'</i> screen of "'.$owner->name.'"');
						exit();
					}
				}

			}//end if

			// check the active screen for special system defined static screens
			$file = null;
			if (isset($this->_edit_fns->static_screens[$active_screen])) {

				// save the status for later so we can see if it changed
				$old_status = $owner->status;

				// process the system defined bit of this tab if we
				// are not creating the asset
				if (!$creating) {
					$process_function = 'process'.$active_screen;
					$this->includeStaticScreen($active_screen);
					if ($process_function($owner, $o, $this)) $ret_val = true;
				}

				// try and find the file to print user defined
				$file = @$this->_findFile('screen_'.$active_screen);

			} else {

				// must be a normal screen
				if (empty($this->_screens[$active_screen])) {
					trigger_error('Active Screen "'.$active_screen.'" unknown unable to process', E_USER_WARNING);
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
					// if we are in limbo and this section is not to be printed in limbo - dont print it

					if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) continue;

					// if this ain't a section, or if we are printing the create interface and this section
					// is not included in that, ignore it
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
				if ($owner->saveAttributes()) $saved = true;
			}
		}//end foreach screens

		return $saved;

	}//end process();


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

		return $access;

	}//end _getAccess()


	/**
	* Get an array of the screens we are allowed to access for the asset we are painting
	*
	* @param object		Asset	$owner	the asset whose interface we are painting
	*
	* @return Array()
	* @access public
	*/
	function getAllowedScreens(&$owner)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all the user groups that we exist in
		$groups = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
		if (empty($groups)) return Array();

		for (reset($groups); NULL !== ($k = key($groups)); next($groups)) $groups[$k] = $db->quote($groups[$k]);

		$sql = 'SELECT screen, section
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_editing_access ';
		$where = 'userid IN ('.implode(',', $groups).')';

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$where .= ' AND type_code = '.$db->quote('0').'
					OR type_code IN (~SQ0~)';
		$subs = Array( 'SELECT inherited_type_code
						FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
						WHERE type_code = '.$db->quote($owner->type()));
		$where = db_extras_subquery($db, $where, $subs);
		if (DB::isError($where)) trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$allowed_screens = $db->getAssoc($sql.$where, false, Array(), DB_FETCHMODE_ASSOC, true);
		if (DB::isError($allowed_screens)) {
			trigger_error($allowed_screens->getMessage().'<br/>'.$allowed_screens->getUserInfo(), E_USER_ERROR);
		}

		return $allowed_screens;

	}//end getAllowedScreens()


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

	}//end _findFile()


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

		if (!($create_file = $this->_findFile('create'))) return Array($create_sections, '');

		$input = new XML_Tree($create_file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return Array($create_sections, '');
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

	}//end _getCreateSections()


	/**
	* Require the file which contains the interface functions for the passed screen
	*
	* @param string		$screen		the code for the static screen we are including
	*
	* @return void
	* @access public
	*/
	function includeStaticScreen($screen)
	{
		require_once SQ_INCLUDE_PATH.'/asset_static_screens/'.$screen.'.inc';

	}//end includeStaticScreen()


}//end class

?>
