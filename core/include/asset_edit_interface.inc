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
* $Id: asset_edit_interface.inc,v 1.285.2.8 2006/11/22 02:50:36 emcdonald Exp $
*
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
* @version $Revision: 1.285.2.8 $
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
	var $_edit_fns = NULL;

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

		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code);
		$asset_dir = SQ_SYSTEM_ROOT.'/'.$type_info['dir'];
		$class_name = $this->_type_code.'_edit_fns';
		require_once $asset_dir.'/'.$class_name.'.inc';
		$this->_edit_fns =& new $class_name();

		// check that the edit functions have a details tab or else
		// we cant even create assets of this type let alone edit them
		assert_isset($this->_edit_fns->static_screens['details'], translate('no_details_screen_defined', $type_code));

		$this->_dirs = Array($asset_dir);
		$parent_type = $type_info['parent_type'];

		while ($parent_type != 'asset') {
			$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($parent_type);
			$this->_dirs[] = SQ_SYSTEM_ROOT.'/'.$type_info['dir'];
			$parent_type = $type_info['parent_type'];
		}

		// add the default for Asset
		$this->_dirs[] = SQ_INCLUDE_PATH.'/asset_edit';

		if (!($file = $this->findFile('screens'))) return;

		$input =& new XML_Tree($file);
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_localised_error('SYS0230', E_USER_WARNING, $file, $root->getMessage(), $root->getUserInfo());
			return;
		}

		$this->_screens = Array();
		for ($i = 0; $i < count($root->children); $i++) {
			$code_name = strtolower(preg_replace('/[^a-z_]/', '', $root->children[$i]->attributes['code_name']));
			if (!$code_name) continue;

			$force_unlock = ((isset($root->children[$i]->attributes['force_unlock'])) ? $root->children[$i]->attributes['force_unlock'] : '1');

			$this->_screens[$code_name] = Array(
											'name'			=> $root->children[$i]->attributes['display_name'],
											'force_unlock'	=> ($force_unlock != '0'),
											'lock_type'		=> (isset($root->children[$i]->attributes['lock_type'])) ? $root->children[$i]->attributes['lock_type'] : '',
											'invisible'		=> (!empty($root->children[$i]->attributes['invisible'])),
										  );


			if (!empty($root->children[$i]->attributes['default'])) {
			  $this->_default_screen = $code_name;
			}
		}//end for

	}//end constructor


	/**
	* Returns the screens array
	*
	* @return array
	* @access public
	*/
	function getScreens()
	{
		return $this->_screens;

	}//end getScreens()


	/**
	* Paints the interface for the currently selected frame
	*
	* @param object		&$owner		the asset whose interface we are painting
	* @param object		&$o			the outputter class
	* @param boolean	$creating	whether we are creating or editing
	*
	* @return NULL
	* @access public
	*/
	function paint(&$owner, &$o, $creating)
	{
		// if we dont have read access, go away
		if ($owner->id && !$owner->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $owner->name));
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
			if (empty($allowed_screens)) {
				$check_screen_restrictions = FALSE;
			}
		}


		  /////////////////////
		 //  ACTIVE SCREEN  //
		/////////////////////
		$create_sections = Array();
		if (!$creating) {
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

		} else {
			// if we are creating open the create file to see which fields we are supposed to print out
			list($create_sections, $active_screen) = $this->_getCreateSections(TRUE);
			if (!count($create_sections)) return;

		}//end if


		if ($check_screen_restrictions && !isset($allowed_screens[$active_screen])) {
			// this user is not allowed to view the current screen
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_inline_asset_screen', $active_screen, $owner->name));
			exit();
		}

		if (!isset($this->_edit_fns->static_screens[$active_screen]) && empty($this->_screens[$active_screen])) {
			// we cant find this screen, so we will use the details screen as a backup
			trigger_localised_error('SYS0076', E_USER_WARNING, $active_screen);
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
		$print_commit_button = FALSE;

		// do we unlock this asset when committing?
		$force_unlock_on_commit = TRUE;

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
			}

			// do they have access to acquire the lock?
			$acquire_lock_access = $owner->canLiveEdit($screen_lock_type);

			$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, $screen_lock_type, TRUE);

			$have_any_locks    = FALSE;
			$have_all_locks    = TRUE;
			$need_force_locks  = FALSE;
			$can_force_locks   = TRUE;
			$do_the_limbo_lock = TRUE;
			foreach ($locks as $lock_type => $lock) {

				if (empty($lock)) {
					$have_all_locks = FALSE;
				} else {
					// this asset is currently locked
					$user    =& $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
					$editing =& $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

					if ($GLOBALS['SQ_SYSTEM']->currentUser($user)) {
						$have_any_locks = TRUE;
					} else {
						$have_all_locks = FALSE;
						$do_the_limbo_lock = FALSE;

						// this lock is held by someone else so check if we can forceable acquire it
						$need_force_locks = TRUE;
						// does the current user has a high
						// enough level of access to forceably acquire the lock
						if (!$owner->canForceablyAcquireLock($lock_type)) {
							$can_force_locks = FALSE;
						}

					}
				}//end if

			}//end foreach lock types

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
					$o->openSection(translate('locking_editing'));
					$o->openRaw();
						?>
						<p style="margin-top: 5px; margin-bottom: 2px"><?php
							if (!$have_all_locks) {
								if ($need_force_locks) {
									if ($can_force_locks) {
										echo '<div class="sq-backend-locked-by-someone-else">'.translate('forcibly_acquire_lock', $user->name).'</div>';
									}
								} else if ($full_write_access && $acquire_lock_access) {
									echo translate('acquire_lock', translate('acquire_lock_button'), $owner->name);
								}
							}
							if ($have_any_locks) {
								echo translate('release_lock', translate('release_lock_button'), $owner->name);
							}
							?>
							<b><a href="#" onClick="Javascript: sq_toggle_lock_info();" id="sq_lock_info_toggle"><?php echo translate('show_lock_details'); ?></a></b><br />
						</p>

						<div id="sq_lock_info" style="display: none;">
						<table class="sq-backend-table">
							<tr>
								<td class="sq-backend-table-header" style="width: 20px;">
									<img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" />
								</td>
								<td class="sq-backend-table-header" align="left" nowrap>
									&nbsp;<?php echo translate('lock_type'); ?>
								</td>
								<td class="sq-backend-table-header" width="100%">
									&nbsp;
								</td>
							</tr>
							<tr>
								<td width="100%" colspan="3">
									<table width="100%" cellspacing="0" cellpadding="0">
									<?php
									$class = 'sq-backend-table-cell-alt';
									foreach ($locks as $lock_type => $lock) {
										?>
										<tr>
											<td class="sq-backend-table-cell" style="width: 20px;" align="center">
												<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib'); ?>/web/images/icons/<?php echo ((empty($lock)) ? 'un' : ''); ?>locked.png", "16", "16", "");</script>
												</td>
											<td class="sq-backend-table-cell">
												<?php echo ucwords(str_replace('_', ' ', $lock_type)); ?>
											</td>
											<td class="sq-backend-table-cell">
												<?php
												if (empty($lock)) {

													if ($full_write_access && $acquire_lock_access) {
														echo translate('currently_not_held').'.';
													} else {
														echo translate('no_access_to_edit', $owner->name);
													}

												} else {
													// this asset is currently locked so display message to the user
													$user    =& $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
													$editing =& $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

													if (SQ_ROLLBACK_VIEW) {
														$now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
													} else {
														$now = time();
													}

													if (is_null($lock['expires'])) {
														$expires_in = translate('lock_held_indefinitely');
													} else {
														require_once SQ_FUDGE_PATH.'/general/datetime.inc';
														$expires_in = easy_time_total(($lock['expires'] - $now), TRUE);
														if (!$expires_in) {
															$expires_in = '1 '.translate('second');
														}
														$expires_in = translate('due_to_expire', $expires_in);
													}

													echo translate('lock_held_by', $user->name, $editing->name).'. ';
													echo $expires_in;
												}//end if
												?>
											</td>
										</tr>
										<?php
									}//end foreach lock types
									?>
									</table>
								</td>
							</tr>
						</table>
						</div>

						<?php
						if (!$have_all_locks || $have_any_locks) {
							?><div style="margin-top: 4px; margin-bottom: 0px"><?php
						}

						if (!$have_all_locks) {
							if ($need_force_locks) {
								if ($can_force_locks) {
									// they aren't going to do this without a fight though
									// they'll have to go through our popup window first
									$popup_url = '?SQ_BACKEND_PAGE=main&backend_section=am&ignore_frames=1&am_section=forceably_acquire_lock&assetid='.$owner->id.'&sq_lock_type='.$screen_lock_type;

									hidden_field('sq_lock_acquire_by_force', '0');
									hidden_field('sq_lock_acquire', '0');
									normal_button('sq_forceably_acquire_lock', translate('acquire_lock_button'), "window.open('$popup_url', 'sq_forceably_acquire_lock_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300');");
									echo '&nbsp;';
								}
							} else if ($full_write_access && $acquire_lock_access) {
								hidden_field('sq_lock_acquire_by_force', '0');
								submit_button('sq_lock_acquire', translate('acquire_lock_button'), '', 'accesskey="a"');
								echo '&nbsp;';
							}//end if
						}

						if ($have_any_locks) {
							submit_button('sq_lock_release_manual', translate('release_lock_button'), '', 'accesskey="r"');
							echo '&nbsp;';
						}

						if (!$have_all_locks || $have_any_locks) {
							?></div><?php
						}

					$o->closeRaw();
					$o->closeSection();

				}//end if locktype not none

			}//end else (not in limbo or not do the limbo lock)

			$o->openRaw();
				hidden_field('sq_lock_type', $screen_lock_type);
			$o->closeRaw();

		}//end if

		// we need to check if we are going to paint the layout here, so no static screens get printed
		$paint_layout = FALSE;
		if (SQ_IN_LIMBO && !$creating) {
			$lm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			if ($lm->hasLayout($owner->type(), $active_screen)) {
				$paint_layout = TRUE;
			}
		}


		// check the active screen for special system defined static screens
		$file = NULL;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {
			$active_screen_name = $this->_edit_fns->static_screens[$active_screen]['name'];

			// only paint the static screens if no layouts are getting painted
			if (!$paint_layout) {
				// if we are not creating, paint the system defined bit of this tab
				if (!$creating) {
					$paint_function = 'paint'.$active_screen;
					$this->includeStaticScreen($active_screen);
					if ($paint_function($owner, $o, $this)) {
						$print_commit_button = TRUE;
					}
				}

				$force_unlock_on_commit = $this->_edit_fns->static_screens[$active_screen]['force_unlock'];
			}
			// try and find the file to print user defined section of the tab
			$file = @$this->findFile('screen_'.$active_screen);

		} else {

			// try and find the file with the screen details
			if (!($file = $this->findFile('screen_'.$active_screen))) {
				return;
			}

			$force_unlock_on_commit = $this->_screens[$active_screen]['force_unlock'];
			$active_screen_name = $this->_screens[$active_screen]['name'];

		}//end if


		  ///////////////////
		 //  WHERE AM I   //
		///////////////////
		if (isset($_REQUEST['sq_link_path'])) {
			$link_path = $_REQUEST['sq_link_path'];
		} else {
			$link_path = '';
		}
		$o->addFormActionGetVar('sq_link_path', $link_path, TRUE);
		$o->addHiddenField('sq_link_path', $link_path);


		  ///////////////////
		 //  PREVIEW URL  //
		///////////////////
		// get the asset tree lineage
		$asset_path = Array();
		if (isset($_REQUEST['sq_asset_path'])) {
			// asset map sends comma-separated assetids indicating the path
			$matches = preg_split('|,([0-9]+:?),?|', ($_REQUEST['sq_asset_path']), 0, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
			for (reset($matches); NULL !== ($k = key($matches)); next($matches)) {
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
				foreach ($url_lineage as $asset_info) {
					$asset_path[] = $asset_info['assetid'];
				}
			} else {
				// asset map didn't send one (e.g initial matrix loading)
				// pretend it did by making a singleton set of the owner
				if ($owner->id) $asset_path[] = $owner->id;
			}
		}

		$o->addFormActionGetVar('sq_asset_path', implode(',', $asset_path), TRUE);

		$preview_url = '';
		$closest_parentid = $owner->id;

		foreach ($asset_path as $assetid) {
			if ($assetid != $owner->id) {
				$ancestor =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			} else {
				$ancestor =& $owner;
			}

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
					$preview_url .= '/'.array_shift($paths);
				}
			}
		}//end foreach
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
			$o->addFormActionGetVar('ignore_frames', '1', TRUE);
		}

		// set the current screen for both backend and frontend editing
		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';
		if (SQ_IN_LIMBO && isset($_REQUEST['limbo_assetid'])) {
			$current_location .= 'limbo_assetid='.$_REQUEST['limbo_assetid'].'&';
		}
		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));

		// once we have a preview URL, we can paint the layout if there is one
		// paint the layout now, if needed
		if ($paint_layout) {
			if (NULL !== ($layout =& $lm->getLayout($owner->type(), $active_screen))) {
				return $this->paintLayout($owner, $o, $layout);
			}
		}


		  //////////////////////////
		 //  Navigation History  //
		//////////////////////////
		if (!isset($_SESSION['sq_nav_history']) || !is_array($_SESSION['sq_nav_history'])) {
			$_SESSION['sq_nav_history'] = Array();
		}

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
		if (!$creating) {
			$page_heading .= '&nbsp;&nbsp;<a class="sq_asset_info_toggle" href="#" onclick="Javascript: sq_toggle_asset_info();"><span id="sq_asset_info_toggle">[ '.translate('more_info').' ]</span></a>';
		}

		$o->setHeading($page_heading, '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($this->_type_code).'", "16", "16", "");</script>');

		$sub_heading = '';
		if ($closest_parentid != $owner->id) {
			$closest_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($closest_parentid);
			if (!is_null($closest_parent)) {
				$sub_heading = translate('currently_editing', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($closest_parent->type(), 'name'), $closest_parent->name).'<br/>';
			}
		}

		if (!$creating) {
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($owner->created_userid, $owner->updated_userid, $owner->published_userid), 'user', FALSE);
			$created_by_name = (isset($user_info[$owner->created_userid]['short_name'])) ? $user_info[$owner->created_userid]['short_name'] : '&lt;unknown&gt;';
			$updated_by_name = (isset($user_info[$owner->updated_userid]['short_name'])) ? $user_info[$owner->updated_userid]['short_name'] : '&lt;unknown&gt;';
			$published_by_name = (isset($user_info[$owner->published_userid])) ? $user_info[$owner->published_userid]['short_name'] : '&lt;unknown&gt;';

			$sub_heading .= '<div id="sq_asset_info" style="display: none;">
							<table>
								<tr>
									<td valign="top">
										<script language="JavaScript" type="text/javascript">sq_print_icon("'.sq_web_path('lib').'/web/images/icons/info.png", "16", "16", "");</script>
									</td>
									<td class="sq-backend-asset-info" nowrap="nowrap">
									'.translate('created_by', $created_by_name, readable_datetime($owner->created)).'<br/>
									'.translate('last_updated', $updated_by_name, readable_datetime($owner->updated)).'<br/>';

			if (!is_null($owner->published)) {
				$sub_heading .= translate('last_published', $published_by_name, readable_datetime($owner->published));
			} else {
				$sub_heading .= translate('never_published');
			}

			$sub_heading .=	'		</td>
								</tr>
							</table>
							<div>';
		}
		$o->setSubHeading($sub_heading);


		  ///////////////////
		 //  SCREEN MENU  //
		///////////////////
		// add all the screens to the combo, but only if we aren't creating the object
		if (!$creating) {
			// add static screens for the asset
			foreach ($this->_edit_fns->static_screens as $code_name => $data) {
				if ($check_screen_restrictions && !isset($allowed_screens[$code_name])) {
					continue;
				}
				$url = $current_location.'asset_ei_screen='.rawurlencode($code_name);
				$o->addStaticScreen($url, $data['name']);
			}
			for (reset($this->_screens); NULL !== ($code = key($this->_screens)); next($this->_screens)) {
				if ($check_screen_restrictions && !isset($allowed_screens[$code])) {
					continue;
				}
				if ($this->_screens[$code]['invisible'] && $code != $active_screen) {
					continue;
				}
				$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
			}
		}


		  //////////////////////
		 //  EDIT INTERFACE  //
		//////////////////////
		if ($file) {
			$print_commit_button |= $this->printEditInterface('screen_'.$active_screen, $owner, $o, $creating, $create_sections);
		}

		// we have printed an editable interface on a static screen - show the commit button
		if ($print_commit_button) {
			$o->commitButton('', $force_unlock_on_commit);
		}

	}//end paint()


	/**
	* Paints out the interface for the passed asset inline
	*
	* This means that it doesn't include the Commit Button, Locking, Page Headings, Screen Menu
	*
	* @param object		&$owner			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param boolean	$creating		whether we are creating or editing
	* @param string		$active_screen	if we are not creating, use this screen
	*
	* @return boolean
	* @access public
	*/
	function paintInline(&$owner, &$o, $creating, $active_screen)
	{
		$print_commit_button = FALSE;

		// if we dont have read access, go away
		if ($owner->id && !$owner->readAccess()) {
			$o->openField('');
			?>
				<span class="error"><?php echo translate('cannot_access_asset', $owner->name); ?></span>
			<?php
			$o->closeField();
			return FALSE;
		}

		// if there are no screens or static screens, piss off
		assert_false(!count($this->_screens) && empty($this->_edit_fns->static_screens));


		  /////////////////////////////
		 //  ALLOWED SCREEN ACCESS  //
		/////////////////////////////
		$check_screen_restrictions = !$creating && (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()));
		if ($check_screen_restrictions) {
			$allowed_screens = $this->getAllowedScreens($owner);
			if (empty($allowed_screens)) {
				$check_screen_restrictions = FALSE;
			}
		}


		  /////////////////////
		 //  ACTIVE SCREEN  //
		/////////////////////
		$create_sections = Array();
		if ($creating) {
			// if we are creating open the create file to see which fields we are supposed to print out
			list($create_sections, $active_screen) = $this->_getCreateSections(TRUE);
			if (!count($create_sections)) return;

		}//end if


		// this user is not allowed to view the current screen
		if ($check_screen_restrictions && !isset($allowed_screens[$active_screen])) {
			$o->openField('');
			?>
				<span class="error"><?php echo translate('cannot_access_inline_asset_screen', $active_screen, $owner->name); ?></span>
			<?php
			$o->closeField();
			return FALSE;
		}

		if (!isset($this->_edit_fns->static_screens[$active_screen]) && empty($this->_screens[$active_screen])) {
			// we cant find this screen, so we will use the details screen as a backup
			trigger_localised_error('SYS0076', E_USER_WARNING, $active_screen);
			$active_screen = 'details';
		}

		// we need to check if we are going to paint the layout here, so no static screens get printed
		$paint_layout = FALSE;
		if (SQ_IN_LIMBO && !$creating) {
			$lm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			if ($lm->hasLayout($owner->type(), $active_screen)) {
				$paint_layout = TRUE;
			}
		}

		// check the active screen for special system defined static screens
		$file = NULL;
		if (isset($this->_edit_fns->static_screens[$active_screen])) {
			$active_screen_name = $this->_edit_fns->static_screens[$active_screen]['name'];

			// only paint the static screens if no layouts are getting painted
			if (!$paint_layout) {
				// if we are not creating, paint the system defined bit of this tab
				if (!$creating) {
					$paint_function = 'paint'.$active_screen;
					$this->includeStaticScreen($active_screen);
					if ($paint_function($owner, $o, $this)) {
						$print_commit_button = TRUE;
					}
				}

				$force_unlock_on_commit = $this->_edit_fns->static_screens[$active_screen]['force_unlock'];
			}
			// try and find the file to print user defined section of the tab
			$file = @$this->findFile('screen_'.$active_screen);

		} else {

			// try and find the file with the screen details
			if (!($file = $this->findFile('screen_'.$active_screen))) {
				return;
			}

			$force_unlock_on_commit = $this->_screens[$active_screen]['force_unlock'];
			$active_screen_name = $this->_screens[$active_screen]['name'];

		}//end if

		// once we have a preview URL, we can paint the layout if there is one
		// paint the layout now, if needed
		if ($paint_layout) {
			if (NULL !== ($layout =& $lm->getLayout($owner->type(), $active_screen))) {
				return $this->paintLayout($owner, $o, $layout);
			}
		}

		  //////////////////////
		 //  EDIT INTERFACE  //
		//////////////////////
		if ($file) {
			$print_commit_button |= $this->printEditInterface('screen_'.$active_screen, $owner, $o, $creating, $create_sections);
		}

		return $print_commit_button;

	}//end paintInline()


	/**
	* Print the edit interface from the supplied XML file(s)
	*
	* @param string		$screen				The XML file to print
	* @param object		&$owner				The asset having its interface printed
	* @param object		&$o					The outputter object to use to paint
	* @param boolean	$creating			Whether we are creating the asset rather than editing
	* @param array		$create_sections	The sections to print when creating
	*
	* @return boolean
	* @access public
	*/
	function printEditInterface($screen, &$owner, &$o, $creating=FALSE, $create_sections=Array())
	{
		// grab the closest language file generated for this screen, then
		// unserialise it to get our XML_Tree root
		$root = $this->findScreen($screen);
		if (is_null($root)) return FALSE;

		$print_commit_button = FALSE;

		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		// Loop over the sections
		for ($i = 0; $i < count($root->children); $i++) {
			$section =& $root->children[$i];
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) {
				continue;
			}

			// if this ain't a section, or if we are printing the create interface and this section
			// is not included in that, ignore it
			if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) {
				continue;
			}

			$section_access = $this->_getAccess($owner, $section, $prefix);
			if (!($section_access & SQ_EI_READ)) {
				continue;
			}

			// if a translated display name has been specified, use it
			if (isset($section->attributes['display_name'])) {
				$o->openSection($section->attributes['display_name']);
			} else {
				$o->openSection($section->attributes['name']);
			}

			if (!($num_fields = count($section->children))) {
				continue;
			}

			if ($section->children[0]->name == 'function_call') {

				$paint_node =& $section->children[0]->children[0];

				if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
					$o->openField('');
					echo translate('section_not_found', $section->attributes['name']);
					$o->closeField();
				} else {

					// if we are printing the create interface and this section is not included in that, ignore it
					if ($creating && !in_array($paint_node->attributes['name'], $create_sections[$section->attributes['name']])) {
						continue;
					}

					if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
						$print_commit_button = TRUE;
					}
				}

			// they must just be normal fields
			} else {
				$section_note = NULL;

				// Loop over the fields
				for ($j = 0; $j < $num_fields; $j++) {

					$field =& $section->children[$j];

					switch ($field->name) {
						case 'field':
							$display_name = '';
							$field_note   = '';
							$attr_tag     = NULL;

							for ($sub_index = 0; $sub_index < count($field->children); $sub_index++) {
								$sub_field =& $field->children[$sub_index];
								switch ($sub_field->name) {
									case 'display_name':
										$display_name = $sub_field->content;
									break;

									case 'note':
										$field_note = $sub_field->content;
									break;

									default:
										if (!is_null($attr_tag)) {
											trigger_localised_error('SYS0194', E_USER_WARNING);
										} else {
											$attr_tag =& $sub_field;
										}
									break;
								}//end switch

							}

							// if we are in limbo and this field is not to be printed in limbo - dont print it
							if (SQ_IN_LIMBO && (isset($field->attributes['limbo_access']) && (int)$field->attributes['limbo_access'] == 0)) {
								continue;
							}

							$field_access = $this->_getAccess($owner, $field, $prefix, $section_access);
							if (!($field_access & SQ_EI_READ)) {
								continue;
							}

							if (!isset($field->attributes['format'])) {
								$field->attributes['format'] = '';
							}

							if (!empty($field->attributes['attribute'])) {
								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->name != 'field' || ($creating && (empty($field->attributes['attribute']) || !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']])))) {
									continue;
								}

								$o->openField($display_name, $field->attributes['format'], '', isset($field->attributes['hidden']));

								$attr =& $owner->getAttribute($field->attributes['attribute']);
								if (is_null($attr)) continue;
								if (!$attr->setEditParams($attr_tag)) continue;
								$attr->paint($prefix.'_'.$attr->id, !($field_access & SQ_EI_WRITE));
								if ($field_access & SQ_EI_WRITE) {
									$print_commit_button = TRUE;
								}

								if (!empty($field_note)) {
									$field_note = str_replace('%description%', $attr->description, $field_note);
								}

							} else if ($attr_tag->name == 'function_call') {

								$paint_node =& $attr_tag->children[0];

								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->name != 'field' || ($creating && !in_array($paint_node->attributes['name'], $create_sections[$section->attributes['name']]))) {
									continue;
								}

								$o->openField($display_name, $field->attributes['format'], '', isset($field->attributes['hidden']));

								if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
									$o->note(translate('specific_painting_function_not_found', $paint_node->attributes['name']));
								} else {
									if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
										$print_commit_button = TRUE;
									}
								}

							}//end if

							if (!empty($field_note)) $o->note($field_note);

						break;

						case 'note':
							// notice that only the last "note" will be used if multiple are supplied
							if (!empty($field->content)) {
								$section_note = $field->content;
							}
						break;

						default:
							trigger_localised_error('SYS0140', E_USER_WARNING, $field->name);
							continue;

					}//end switch

				}//end for fields

			}//end else section uses function call

			if (!empty($section_note)) {
				$o->sectionNote($section_note);
			}

			$o->closeSection();

		}//end for

		return $print_commit_button;

	}//end printEditInterface()


	/**
	* Acquires any locks needed by limbo
	*
	* @param object	&$owner				the owner of this interface
	* @param object	&$o					the backend outputter
	* @param string	$active_screen		the active screen we are on
	* @param string	$screen_lock_type	the lock type of this screen
	*
	* @return boolean
	* @access public
	*/
	function acquireLimboLocks(&$owner, &$o, $active_screen='details', $screen_lock_type=NULL)
	{
		$locks = Array();
		if (!is_null($screen_lock_type)) {
			$locks[] = $screen_lock_type;
		}

		$lm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');

		if ($lm->hasLayout($owner->type(), $active_screen)) {

			$layout =& $lm->getLayout($owner->type(), $active_screen);
			if (is_null($layout)) return FALSE;

			$keywords = $layout->getLayoutKeywords();
			if (empty($keywords)) return FALSE;

			if (isset($keywords['custom']) && !empty($keywords['custom'])) {
				// acquire any locks that the owner needs to paint the layout
				$locks = array_merge($locks, $owner->getEditingLocks($keywords['custom']));
			}

			// if there are other screens used by the layout, then we need to acquire locks on those screens as well
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
				}

				$locks[] = $screen_lock_type;

			}//end foreach

		}//end if hasLayout

		$locks = array_unique($locks);
		$have_all_locks = TRUE;
		foreach ($locks as $lock_name) {
			$lock_info = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, $lock_name, TRUE);
			foreach ($lock_info as $lock_type => $lock_data) {
				if (!isset($lock_data['userid']) || $lock_data['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
					$have_all_locks = FALSE;
				}
			}
		}

		if (!$have_all_locks) {
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetids'			=> Array($owner->id,),
						'lock_type'			=> $locks,
						'forceably_acquire'	=> FALSE,
					);
			$hh->queueHipo('hipo_job_acquire_locks', $vars);
			$url = $hh->runQueuedJobs();
			if (!empty($url)) {
				$o->setRedirect(strip_url(current_url()).'/?sq_redirect_url='.urlencode($url));
			}
		}

	}//end acquireLimboLocks()


	/**
	* Prints limbo in a specified layout
	*
	* @param object	&$owner		the owner of this interface
	* @param object	&$o			the backend outputter
	* @param object	&$layout	the layout to be painted
	*
	* @return boolean
	* @access public
	*/
	function paintLayout(&$owner, &$o, &$layout)
	{
		$keywords = $layout->getLayoutKeywords();
		$print_commit_button = FALSE;

		// find what screens we need to process
		if (!isset($keywords['screens'])) {
			$keywords['screens'] = Array();
		}

		if (!isset($keywords['entire_screens'])) {
			$keywords['entire_screens'] = Array();
		}

		$screens = array_merge(array_keys($keywords['screens']), array_keys($keywords['entire_screens']));
		$screens = array_unique($screens);

		$this->_tmp['layout_keyword_replacements'] = Array();

		// let the backend outputter know that we want buffering turned off
		$o->setBuffering(FALSE);

		// loop over the screens and get the xml file for each screen
		// generate some keywords for each screen
		foreach ($screens as $screen) {

			// check to see if we want the entire screen
			$entire_screen = FALSE;
			if (in_array($screen, array_keys($keywords['entire_screens']))) {
				$entire_screen = TRUE;
			}

			// if we have a static screen on our hands, then we want to
			// call the function to paint it
			if (isset($this->_edit_fns->static_screens[$screen])) {
				$static_keywords = (isset($keywords['screens'][$screen])) ? $keywords['screens'][$screen] : NULL;
				if (!is_null($static_keywords) || $entire_screen) {
					if (is_null($static_keywords)) {
						$static_keywords = Array();
					}
					if ($this->_getStaticScreenKeywordReplacements($owner, $o, $screen, $static_keywords, $entire_screen)) {
						$print_commit_button = TRUE;
					}
				}
			}//end if

			// Metadata & workflow don't have an xml file defined, so move onto the next screen.
			if ($screen == 'metadata') continue;
			if ($screen == 'workflow') continue;
			if ($screen == 'tagging') continue;

			// its not a static screen, so get the xml file and parse it for
			// the keywords that we want, if the xml file exists
			$file = $this->findLanguageFile('screen_'.$screen);

			if (empty($file)) {
				$file  = $this->findFile('screen_'.$screen);
				$input =& new XML_Tree($file);
				$root  =& $input->getTreeFromFile();
				if (PEAR::isError($root)) {
					trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING); // asst-val-db fix
					return FALSE;
				}
			} else {
				$root = unserialize(file_get_contents($file));
			}

			if ($root) {
				// get the layout replacements for this NON-static screen
				if ($this->_getLayoutKeywordReplacements($root, $o, $owner, $keywords, $screen, $entire_screen)) {
					$print_commit_button = TRUE;
				}
			}//end if

		}//end foreach

		// if there are some custom keywords, get some replacements from the owner
		if (isset($keywords['custom']) && !empty($keywords['custom'])) {
			$custom_replacements = $owner->getCustomKeywordReplacements($keywords['custom']);

			if (!empty($custom_replacements)) {
				assert_isset_array_index($custom_replacements, 'replacements');
				assert_isset_array_index($custom_replacements, 'print_commit_button');
				$replacements = Array();

				// rebuild the keyword so the bodycopy can properly replace it
				foreach ($custom_replacements['replacements'] as $keyword => $replacement) {
					$replacements['__custom-'.$keyword] = $replacement;
				}
				$this->_tmp['layout_keyword_replacements'] = array_merge($this->_tmp['layout_keyword_replacements'], $replacements);
				if ($custom_replacements['print_commit_button']) {
					$print_commit_button = TRUE;
				}

			}//end if

		}//end if

		if (!isset($keywords['layout'])) {
			$keywords['layout'] = Array();
		}

		// set keywords for the commit button, screen menu dropdown, or any of the
		// available keywords for asset being exposed
		foreach ($keywords['layout'] as $keyword) {
			if (($keyword == 'commit_button') && $print_commit_button) {
				ob_start();
					$o->commitButton('', FALSE);
					$this->_tmp['layout_keyword_replacements']['commit_button'] = ob_get_contents();
				ob_end_clean();
				continue;

			} else if ($keyword == 'screen_menu') {
				$lm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
				$current_location = $o->getCurrentLocation();
				$current_location .= (strstr($current_location, '?')) ? '&' : '?';

				// if we're using a different limbo assetid, make sure that the
				// assetid is being brought through
				if (SQ_IN_LIMBO && isset($_REQUEST['limbo_assetid'])) {
					$current_location .= 'limbo_assetid='.$_REQUEST['limbo_assetid'].'&';
				}

				for (reset($this->_screens); NULL !== ($code = key($this->_screens)); next($this->_screens)) {
					if (!$lm->hasLayout($owner->type(), $code)) continue;
					$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
				}
				foreach ($this->_edit_fns->static_screens as $code_name => $data) {
					if (!$lm->hasLayout($owner->type(), $code_name)) {
						continue;
					}
					$url = $current_location.'asset_ei_screen='.rawurlencode($code_name);
					$o->addStaticScreen($url, $data['name']);
				}
				ob_start();
					$o->printScreenMenu();
					$this->_tmp['layout_keyword_replacements']['screen_menu'] = ob_get_contents();
				ob_end_clean();
				continue;

			} else {
				$this->_tmp['layout_keyword_replacements'][$keyword] = $owner->getKeywordReplacement($keyword);
			}
		}//end foreach

		// turn buffering back on so the output is not malformed
		$o->setBuffering(TRUE);

		if (!$print_commit_button) {
			$this->_tmp['layout_keyword_replacements']['commit_button'] = '';
		}

		// tell the layout what the keyword replacements are
		$layout->setKeywordReplacements($this->_tmp['layout_keyword_replacements']);

		// we need to open a raw section so that the form gets printed before the contents of the layout
		$o->openRaw();
			$layout->printBody();
		$o->closeRaw();

		return TRUE;

	}//end paintLayout()


	/**
	* Gets a keyword replacement for static screens.
	*
	* @param object		&$owner				the owner of this asset
	* @param object		&$o					the backend outputter object
	* @param string		$screen				the static screen to get a replacement for
	* @param array|NULL	$static_keywords	the wantedkeyowrd of this static screen
	* @param boolean	$get_screen			if TRUE the entire screen will be captured into a replacement
	*
	* @return boolean
	* @access private
	*/
	function _getStaticScreenKeywordReplacements(&$owner, &$o, $screen, $static_keywords, $get_screen=FALSE)
	{
		$this->includeStaticScreen($screen);
		$print_commit_button = FALSE;

		ob_start();
			$paint_function = 'paint'.$screen;

			if ($paint_function($owner, $o, $this, $static_keywords)) {
				$print_commit_button = TRUE;
			}

			if ($get_screen) {
				$this->_tmp['layout_keyword_replacements'][$screen.'-'] = ob_get_contents();
			}
		ob_end_clean();

		return $print_commit_button;

	}//end _getStaticScreenKeywordReplacements()


	/**
	* Gets the keyword replacements to be used in the layout
	*
	* @param object		&$root				the root of the xml screen
	* @param object		&$o					the backend outputter object
	* @param object		&$owner				the owner of this asset
	* @param array		$layout_keywords	an array of keywords to get replacements for
	* @param string		$screen				the name of the current screen
	* @param boolean	$get_screen			if TRUE the entire screen will be captured into a replacement
	*
	* @return boolean
	* @access private
	*/
	function _getLayoutKeywordReplacements(&$root, &$o, &$owner, $layout_keywords, $screen='details', $get_screen=FALSE)
	{
		$prefix = $owner->getPrefix();
		$print_commit_button = FALSE;

		// start buffering anything and trash the content at the end
		ob_start();

		for ($i = 0; $i < count($root->children); $i++) {

			$section =& $root->children[$i];

			// find out if we have permission to print this section
			$limbo_access = (isset($section->attributes['limbo_access']) && (int) $section->attributes['limbo_access'] == 1) ? TRUE : FALSE;

			// find out what we need to get in this loop
			$get_section = FALSE;
			$section_name = strtolower(str_replace(' ', '_', $section->attributes['name']));
			if (isset($layout_keywords['screens'][$screen]['sections'][$section_name])) {
				$get_section = TRUE;
				$section_keyword = $layout_keywords['screens'][$screen]['sections'][$section_name];
			}

			if (!($num_fields = count($section->children))) {
				continue;
			}

			$section_access = $this->_getAccess($owner, $section, $prefix);
			if (!($section_access & SQ_EI_READ)) {
				continue;
			}

			// if we are getting this section, start buffering output
			if ($get_section) ob_start();
			if ($get_section || $get_screen) {
				$o->openSection($section->attributes['name']);
			}

			if ($section->children[0]->name == 'function_call') {
				if ($get_section || $get_screen) {
					$paint_node =& $section->children[0]->children[0];

					if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
						$o->openField('');
						echo translate('section_not_found', $section->attributes['name']);
						$o->closeField();
					} else {
						if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
							$print_commit_button = TRUE;
						}
					}
				}

			// they must just be normal fields
			} else {
				$section_note = NULL;

				// Loop over the fields
				for ($j = 0; $j < $num_fields; $j++) {
					$field =& $section->children[$j];

					switch ($field->name) {
						case 'field':
							$display_name = '';
							$field_note   = '';
							$attr_tag     = NULL;

							for ($sub_index = 0; $sub_index < count($field->children); $sub_index++) {
								$sub_field =& $field->children[$sub_index];
								switch ($sub_field->name) {
									case 'display_name':
										$display_name = $sub_field->content;
									break;

									case 'note':
										$field_note = $sub_field->content;
									break;

									default:
										if (!is_null($attr_tag)) {
											trigger_localised_error('SYS0194', E_USER_WARNING);
										} else {
											$attr_tag =& $sub_field;
										}
									break;
								}//end switch
							}

							$field_access = $this->_getAccess($owner, $field, $prefix, $section_access);
							if (!($field_access & SQ_EI_READ)) {
								continue;
							}

							if (!isset($field->attributes['format'])) {
								$field->attributes['format'] = '';
							}

							// reset get_field
							$get_field = FALSE;

							if (!empty($field->attributes['attribute'])) {
								// find out what we need to get in this loop
								if (isset($layout_keywords['screens'][$screen]['fields'][$field->attributes['attribute']])) {
									$get_field = TRUE;
									$field_keyword = $layout_keywords['screens'][$screen]['fields'][$field->attributes['attribute']];
								}

								// if we do not want anything in this loop, continue
								if (!$get_field && !$get_section && !$get_screen) {
									continue;
								}

								if ($field->name != 'field') continue;
								$o->openField($display_name, $field->attributes['format'], '', isset($field->attributes['hidden']));

								$attr =& $owner->getAttribute($field->attributes['attribute']);

								if (is_null($attr)) continue;
								if (!$attr->setEditParams($attr_tag)) continue;
								// if this is not a wanted keyword, then continue on our way
								if (isset($layout_keywords['screens'][$screen]['fields'][$field->attributes['attribute']]))

								// start buffering for this field
								if ($get_field) ob_start();

								$attr->paint($prefix.'_'.$attr->id, !(($field_access & SQ_EI_WRITE) && $limbo_access));
								if (($field_access & SQ_EI_WRITE) && $limbo_access) {
									$print_commit_button = TRUE;
								}

								if (!empty($field_note)) {

									$field_note = str_replace('%description%', $attr->description, $field_note);
									$o->note($field_note);
								}

								// end buffering
								if ($get_field) {
									$this->_tmp['layout_keyword_replacements'][$field_keyword] = ob_get_contents();
									ob_end_flush();
								}
								$o->closeField();

							} else if ($attr_tag->name == 'function_call') {

								if (isset($field->attributes['keyword'])) {
									// find out what we need to get in this loop
									if (isset($layout_keywords['screens'][$screen]['fields'][$field->attributes['keyword']])) {
										$get_field = TRUE;
										$field_keyword = $layout_keywords['screens'][$screen]['fields'][$field->attributes['keyword']];
									}
								}

								$paint_node =& $attr_tag->children[0];
								if ($field->name != 'field') continue;

								$o->openField($display_name, $field->attributes['format'], '', isset($field->attributes['hidden']));

								if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
									echo translate('painting_funtion_not_found');
								} else {
									if ($get_field) ob_start();

									if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
										$print_commit_button = TRUE;
									}
									// get a replacement for this field is there is a keyword for it
									if ($get_field) {
										$this->_tmp['layout_keyword_replacements'][$field_keyword] = ob_get_contents();
										ob_end_flush();
									}
								}//end if

								$o->closeField();

							}//end if
						break;

						case 'note':
							// notice that only the last "note" will be used if multiple are supplied
							if (!empty($field->content)) {
								$section_note = $field->content;
							}
						break;
					}//end switch
				}//end for
			}//end else section == function_call

			if ($get_section || $get_screen) {
				if (!empty($section_note)) {
					$o->sectionNote($section_note);
				}
				$o->closeSection();
			}

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

	}//end _getLayoutKeywordReplacements()


	/**
	* Processes the results of the form posting, returns TRUE anything was processed/changed
	*
	* @param object		&$owner		the asset whose interface we are painting
	* @param object		&$o			the outputter class
	* @param boolean	$creating	whether we are creating or editing
	*
	* @return boolean
	* @access public
	*/
	function process(&$owner, &$o, $creating)
	{
		$current_screen = (empty($_REQUEST['asset_ei_screen'])) ? 'details' : $_REQUEST['asset_ei_screen'];
		return $this->processInline($owner, $o, $creating, $current_screen);

	}//end process()


	/**
	* Processes the results of the form posting, ignores the screen that is set in the get vars, relies on being passed the correct screen
	*
	* @param object		&$owner			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param boolean	$creating		whether we are creating or editing
	* @param string		$current_screen	Current Screen
	*
	* @return boolean
	* @access public
	*/
	function processInline(&$owner, &$o, $creating, $current_screen)
	{
		// used to check if anything has bed saved
		$saved = FALSE;

		// if there are no screens or static screens, piss off
		if (!count($this->_screens) && empty($this->_edit_fns->static_screens)) {
			return FALSE;
		}

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {

			list($create_sections, $current_screen) = $this->_getCreateSections(FALSE);
			if (!count($create_sections)) return FALSE;

			$screens = Array($current_screen);


		} else {

			$screens = Array($current_screen);

			$lm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			$layout =& $lm->getLayout($owner->type(), $current_screen);

			// if we have a layout and there are some defined screen components, then these
			// screens also need to be processed
			if (!is_null($layout)) {
				$keywords = $layout->getLayoutKeywords();

				if (isset($keywords['screens']) && !empty($keywords['screens'])) {
					$screens = array_unique(array_merge(array_keys($keywords['screens']), $screens));
				}

				if (isset($keywords['entire_screens']) && !empty($keywords['entire_screens'])) {
					$screens = array_unique(array_merge(array_keys($keywords['entire_screens']), $screens));
				}

				// if there are some special keywords, we need to ask the owner to process them
				if (isset($keywords['custom']) && !empty($keywords['custom'])) {
					$owner->processCustomKeywords($keywords['custom']);
				}
			}
		}
		foreach ($screens as $active_screen) {
			$ret_val = FALSE;

			if (!$creating) {
				  /////////////////////////////
				 //  ALLOWED SCREEN ACCESS  //
				/////////////////////////////
				if (!$owner->adminAccess('')) {
					$allowed_screens = $this->getAllowedScreens($owner);

					if (!empty($allowed_screens) && !isset($allowed_screens[$active_screen])) {
						// this user is not allowed to view the current screen
						$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $owner->name));
						exit();
					}
				}

			}//end if

			// check the active screen for special system defined static screens
			$file = NULL;
			if (isset($this->_edit_fns->static_screens[$active_screen])) {

				// save the status for later so we can see if it changed
				$old_status = $owner->status;

				// process the system defined bit of this tab if we
				// are not creating the asset
				if (!$creating) {
					$process_function = 'process'.$active_screen;
					$this->includeStaticScreen($active_screen);
					if ($process_function($owner, $o, $this)) {
						$ret_val = TRUE;
					}
				}

				// try and find the file to print user defined
				$file = @$this->findLanguageFile('screen_'.$active_screen);

				if (empty($file)) {
					// no language files, try and find the base structure
					$file  = @$this->findFile('screen_'.$active_screen);

					// still can't find anything?!
					if (empty($file)) {
						$root = NULL;
					} else {
						$input =& new XML_Tree($file);
						$root  =& $input->getTreeFromFile();
						if (PEAR::isError($root)) {
							trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING); // ast-val-db fix
							$root = NULL;
						}
					}
				} else {
					$root = unserialize(file_get_contents($file));
				}
			} else {

				// must be a normal screen
				if (empty($this->_screens[$active_screen])) {
					trigger_localised_error('SYS0076', E_USER_WARNING, $active_screen);
					return FALSE;
				}

				// try and find the file to print user defined
				$root = $this->findScreen('screen_'.$active_screen);

			}//end if

			if (!is_null($root) && $owner->writeAccess('')) {

				// prefix for all the form vars
				$prefix = $owner->getPrefix();

				// Loop over the sections
				for ($i = 0; $i < count($root->children); $i++) {
					$section =& $root->children[$i];
					// if we are in limbo and this section is not to be printed in limbo - dont print it
					if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) {
						continue;
					}

					// if this ain't a section, or if we are printing the create interface and this section
					// is not included in that, ignore it
					if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) {
						continue;
					}

					if (!($num_fields = count($section->children))) {
						continue;
					}

					$section_access = $this->_getAccess($owner, $section, $prefix);
					if (!($section_access & SQ_EI_READ)) {
						continue;
					}

					// if this is section only has function call, then process accordingly
					if ($section->children[0]->name == 'function_call') {

						$process_node =& $section->children[0]->children[1];

						// if there is nothing declared there is nothing for us to do
						if (empty($process_node->attributes['name'])) {
							continue;
						// There is something declared but the bloody thing doesn't exist - inform the Idiot
						} else if (!method_exists($this->_edit_fns, $process_node->attributes['name'])) {
							trigger_localised_error('SYS0188', E_USER_WARNING, $section->attributes['name']);
						} else {
							// if we are processing the create interface and this section function is not included in that, ignore it
							if ($creating && !in_array($process_node->attributes['name'], $create_sections[$section->attributes['name']])) {
								continue;
							}

							if ($this->_edit_fns->{$process_node->attributes['name']}($owner, $o, $prefix)) {
								$ret_val = TRUE;
							}

						}

					// they must just be normal fields
					} else {
						// Loop over the fields
						for ($j = 0; $j < $num_fields; $j++) {
							$field =& $section->children[$j];

							if ($field->name == 'note') continue;

							$last_child = count($field->children)-1;

							$field_access = $this->_getAccess($owner, $field, $prefix, $section_access);
							if (!($field_access & SQ_EI_WRITE)) {
								continue;
							}

							if (!empty($field->attributes['attribute'])) {
								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->name != 'field' || ($creating && (empty($field->attributes['attribute']) || !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']])))) {
									continue;
								}

								$attr =& $owner->getAttribute($field->attributes['attribute']);
								if (is_null($attr)) continue;
								if (!$attr->setEditParams($field->children[$last_child])) {
									continue;
								}

								$revert_attr = $attr;
								$attr->process($prefix.'_'.$attr->id);
								if ($attr->processed && $owner->setAttrValue($attr->name, $attr->value)) {
									$ret_val = TRUE;
								} else {
									// processing failed so revert to old version
									$attr = $revert_attr;
								}
								unset($revert_attr);

							} else if ($field->children[$last_child]->name == 'function_call') {

								$process_node =& $field->children[$last_child]->children[1];

								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->name != 'field' || ($creating && !in_array($process_node->attributes['name'], $create_sections[$section->attributes['name']]))) {
									continue;
								}

								// if there is nothing declared there is nothing for use to do
								if (empty($process_node->attributes['name'])) {
									continue;
								// There is something declared but the bloody thing doesn't exist - inform the Idiot
								} else if (!method_exists($this->_edit_fns, $process_node->attributes['name'])) {
									trigger_localised_error('SYS0187', E_USER_WARNING, $section->attributes['name'], $field->children[$last_child]->content);
								} else {
									if ($this->_edit_fns->{$process_node->attributes['name']}($owner, $o, $prefix)) {
										$ret_val = TRUE;
									}
								}

							}//end if

						}//end for fields

					}//end else section funcion call

				}//end for

			}//end if $file

			if ($ret_val) {
				// some attributes have been updated, so save the asset
				if ($owner->saveAttributes()) $saved = TRUE;
			}

		}//end foreach screens

		return $saved;

	}//end processInline()


	/**
	* Reads the write_access and read_access attributes of the passed node and returns
	* at bitmask that can be used with the SQ_EI_READ and SQ_EI_WRITE constants
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	$node			the xml node to check for 'write_access' and 'read_access' attrs
	* @param string	$prefix			the prefix
	* @param int	$inherit_access	used if an access is not specified
	*
	* @return int
	* @access private
	*/
	function _getAccess(&$owner, $node, $prefix, $inherit_access=NULL)
	{
		$access = 0;

		// check show if condition for this node
		if (isset($node->attributes['show_if'])) {
			$show_if_fn = $node->attributes['show_if'];
			$edit_fns =& $owner->getEditFns();

			if (method_exists($edit_fns, $show_if_fn)) {
				if (!$edit_fns->$show_if_fn($owner, $prefix)) return 0;
			} else {
				// if show if function doesn't exist, spew, then continue as normal
				trigger_localised_error('SYS0136', E_USER_WARNING, $show_if_fn, $node->name);
				return 0;
			}
		}

		// if there is nothing specified, and if an inherited access was specified we must use that
		if (empty($node->attributes['write_access']) && !is_null($inherit_access)) {
			if ($inherit_access & SQ_EI_WRITE) {
				$access |= SQ_EI_WRITE;
			}

		// otherwise attempt to find write access
		} else {
			$wa = (empty($node->attributes['write_access'])) ? 'write:all' : $node->attributes['write_access'];
			// all these write access checks must have a lock type to use
			if (preg_match('/^(.+):(.*)$/', $wa, $matches)) {
				// got to have writeAccess at least (this checks for locks as well)
				if ($owner->writeAccess($matches[2])) {
					switch ($matches[1]) {
						case 'root' :
							if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
								$access |= SQ_EI_WRITE;
							}
						break;
						case 'sys_admin' :
							if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
								$access |= SQ_EI_WRITE;
							}
						break;
						case 'admin' :
							if ($owner->adminAccess($matches[2])) {
								$access |= SQ_EI_WRITE;
							}
						break;
						case 'write' :
							$access |= SQ_EI_WRITE; // this was checked above
						break;

						case 'none' :
							// meaning that this attribute is not to be written
						break;

					}//end switch
				}//end if write access
			}//end if preg

		}//end else

		// if they have write access they have read access by definition
		if ($access & SQ_EI_WRITE) {
			$access |= SQ_EI_READ;
		} else {

			// if there is nothing specified, and if an inherited access was specified we must use that
			if (empty($node->attributes['read_access']) && !is_null($inherit_access)) {
					if ($inherit_access & SQ_EI_READ) {
						$access |= SQ_EI_READ;
					}

			// otherwise attempt to find read access
			} else {
				$ra = (empty($node->attributes['read_access'])) ? 'read' : $node->attributes['read_access'];
				switch ($ra) {
					case 'root' :
						if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
							$access |= SQ_EI_READ;
						}
					break;
					case 'sys_admin' :
						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
							$access |= SQ_EI_READ;
						}
					break;
					case 'read' :
						if ($owner->readAccess()) $access |= SQ_EI_READ;
					break;
					default :
						// if it's admin or write access, there needs to be a lock type to use
						if (preg_match('/^(admin|write):(.*)$/', $ra, $matches)) {
							if ($matches[1] == 'admin') {
								if ($owner->adminAccess($matches[2])) {
									$access |= SQ_EI_READ;
								}
							} else {
								if ($owner->writeAccess($matches[2])) {
									$access |= SQ_EI_READ;
								}
							}
						}

				}//end switch
			}//end else

		}//end else

		return $access;

	}//end _getAccess()


	/**
	* Get an array of the screens we are allowed to access for the asset we are painting
	*
	* @param object	&$owner	the asset whose interface we are painting
	*
	* @return array
	* @access public
	*/
	function getAllowedScreens(&$owner)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// get all the user groups that we exist in
		$groups = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
		if (empty($groups)) return Array(); // all screens allowed

		for (reset($groups); NULL !== ($k = key($groups)); next($groups)) {
			$groups[$k] = $db->quote($groups[$k]);
		}

		$sql = 'SELECT screen, section
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_edit_access ';
		$where = 'userid IN ('.implode(',', $groups).')';

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$where .= ' AND (type_code = '.$db->quote('0').'
					OR type_code IN (
									SELECT inhd_type_code
									FROM sq_ast_typ_inhd
									WHERE type_code = '.$db->quote($owner->type()).'
									))';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$allowed_screens = $db->getAssoc($sql.$where, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);

		assert_valid_db_result($allowed_screens);
		return $allowed_screens;

	}//end getAllowedScreens()


	/**
	* Takes the requested filename and searches back up the directories to find the matching file
	* closest to the type of the asset that we are creating or editing
	*
	* @param string	$file_name	the file to find
	*
	* @return string
	* @access public
	*/
	function findFile($file_name)
	{
		$file_name = 'edit_interface_'.$file_name.'.xml';

		$file = '';
		for ($i = 0; $i < count($this->_dirs); $i++) {
			if (file_exists($this->_dirs[$i].'/'.$file_name)) {
				$file = $this->_dirs[$i].'/'.$file_name;
				break;
			}
		}
		if (!$file) {
			trigger_localised_error('SYS0065', E_USER_WARNING, $file_name, $this->_type_code);
			return FALSE;
		}

		return $file;

	}//end findFile()


	/**
	* Find a list of language files to extract from, searching up the
	* directories for the closest match in the current language, then running
	* through fallbacks (ie. the system's default backend locale)
	*
	* @param string		$file_name		the file to find
	* @param boolean	$include_asset	whether the base asset is included
	*									in search
	*
	* @return array
	* @access public
	*/
	function findLanguageFile($file_name, $include_asset=TRUE)
	{
		$type_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->_type_code, $include_asset);
		array_unshift($type_parents, $this->_type_code);

		$file = '';
		$test_locales = Array();
		foreach ($GLOBALS['SQ_SYSTEM']->lm->locale_stack as $locale) {
			list($lang, $country, $variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($locale);

			if (!is_null($variant)) {
				$test_locales[] = $lang.'_'.$country.'@'.$variant;
			}
			if (!is_null($country)) {
				$test_locales[] = $lang.'_'.$country;
			}
			$test_locales[] = $lang;
		}

		foreach ($test_locales as $locale) {

			foreach ($type_parents as $type_code) {
				$file_path = SQ_DATA_PATH.'/private/asset_types/'.$type_code.'/localised_screens/'.$file_name.'.'.$locale;
				if (file_exists($file_path)) {
					$file = $file_path;
					break;
				}
			}

			if (!empty($file)) break;

		}
		return $file;

	}//end findLanguageFile()


	/**
	* Find the closest applicable screen and return the root node
	*
	* Probably need a better way of doing this, but basically this is what it
	* does:
	* 1. Looks for the closest screen structure file, if it is EMPTY then
	*    return the empty file - we don't need any localised stuff
	* 2. If it is not empty, we then look for the closest language file -
	*    even if a partial version of a parent is there it *should* be
	*    localised in any case
	*
	* @param string	$screen_name	the whole screen name (including the
	*								'screen_' or 'static_screen_' part)
	*
	* @return object
	* @access public
	*/
	function &findScreen($screen_name)
	{
		$root = NULL;

		if (($file = $this->findFile($screen_name))) {
			$input =& new XML_Tree($file);
			$root  =& $input->getTreeFromFile();
			if (PEAR::isError($root)) {
				trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
				$root = NULL;
				return $root;
			}

			// if empty, then just skip the searching for language files,
			// otherwise find the closest language file
			if (!empty($root) && !empty($root->children)) {
				$lang_file = $this->findLanguageFile($screen_name);
				if (empty($lang_file)) {
					trigger_localised_error('SYS0197', E_USER_WARNING, $screen_name);
					$root = NULL;
					return $root;
				} else {
					$root = unserialize(file_get_contents($lang_file));
				}
			}
		}

		return $root;

	}//end findScreen()


	/**
	* Returns the Sections and fields that are to be used in a create interface
	* and the screen that these belong to
	*
	* @param boolean	$painting	if we are painting or processing
	*
	* @return array
	* @access private
	*/
	function _getCreateSections($painting)
	{
		$create_sections = Array();

		if (!($create_file = $this->findFile('create'))) {
			return Array($create_sections, '');
		}

		$input =& new XML_Tree($create_file);
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return Array($create_sections, '');
		}

		// Loop over the sections
		for ($i = 0; $i < count($root->children); $i++) {
			$section =& $root->children[$i];
			if ($section->name != 'section') continue;

			$create_sections[$section->attributes['name']] = Array();
			// Loop over the fields
			for ($j = 0; $j < count($section->children); $j++) {

				switch ($section->children[$j]->name) {
					case 'function_call' :
						$fn_call_node =& $section->children[$j];
						$fn_node =& $fn_call_node->children[(($painting) ? 0 : 1)];
						$create_sections[$section->attributes['name']][] = $fn_node->attributes['name'];
					break;

					case 'field' :
						$field =& $section->children[$j];

						if (!empty($field->attributes['attribute'])) {
							$create_sections[$section->attributes['name']][] = $field->attributes['attribute'];
						} else {
							$fn_call_node = reset($field->children);
							while ($fn_call_node->name != 'function_call') {
								$fn_call_node = next($field->children);
							}
							$fn_node =& $fn_call_node->children[(($painting) ? 0 : 1)];
							$create_sections[$section->attributes['name']][] = $fn_node->attributes['name'];
						}//end if
					break;

				}// end switch

			}//end for

		}//end for
		return Array($create_sections, $root->attributes['screen']);

	}//end _getCreateSections()


	/**
	* Require the file which contains the interface functions for the passed screen
	*
	* @param string	$screen	the code for the static screen we are including
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
