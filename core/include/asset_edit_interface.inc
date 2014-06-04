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
* $Id: asset_edit_interface.inc,v 1.380 2013/03/08 02:19:51 ewang Exp $
*
*/


// require_once 'XML/Tree.php';
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
* @version $Revision: 1.380 $
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
		$this->_type_code = basename($type_code);

		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code);
		$asset_dir = SQ_SYSTEM_ROOT.'/'.$type_info['dir'];
		$class_name = $this->_type_code.'_edit_fns';
		require_once $asset_dir.'/'.$class_name.'.inc';
		$this->_edit_fns = new $class_name();

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

		try {
			$root = simplexml_load_string(file_get_contents($file), 'SimpleXMLElement', LIBXML_NOCDATA);
		} catch (Exception $e) {
			throw new Exception('Unable to parse screen list file "'.$file.'": '.$e->getMessage());
		}

		$this->_screens = Array();
		foreach ($root->screen as $screen_node) {
			$code_name = strtolower(preg_replace('/[^a-z_]/', '', (string)$screen_node->attributes()->code_name));
			if (!$code_name) continue;

			$force_unlock = ((isset($screen_node->attributes()->force_unlock)) ? (string)$screen_node->attributes()->force_unlock : '1');

			$this->_screens[$code_name] = Array(
											'name'			=> (string)$screen_node->attributes()->display_name,
											'force_unlock'	=> ($force_unlock != '0'),
											'lock_type'		=> (isset($screen_node->attributes()->lock_type)) ? (string)$screen_node->attributes()->lock_type : '',
											'invisible'		=> (!empty($screen_node->attributes()->invisible)),
										  );


			if (!empty($screen_node->attributes()->default)) {
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
	* Sets the preview url and returns the closest parent id which is used in paint() only
	*
	* @param object	&$owner	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	*
	* @return int
	* @access private
	*/
	function _determinePreviewUrl(&$owner, &$o)
	{
		// get the asset tree lineage
		$asset_path = Array();
		if (isset($_REQUEST['sq_asset_path'])) {
			// asset map sends comma-separated assetids indicating the path
			$matches = preg_split('|,([0-9]+:?),?|', $_REQUEST['sq_asset_path'], 0, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
			for (reset($matches); NULL !== ($k = key($matches)); next($matches)) {
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
				$ancestor = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			} else {
				$ancestor = $owner;
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

		return $closest_parentid;

	}//end _determinePreviewUrl()


	/**
	* Paints the interface for the currently selected frame
	*
	* @param object		&$owner		the asset whose interface we are painting
	* @param object		&$o			the outputter class
	* @param boolean	$creating	whether we are painting the default create interface or something else
	*
	* @return NULL
	* @access public
	*/
	function paint(&$owner, &$o, $creating)
	{
		// if we dont have read access, go away
		if ($owner->id && !$owner->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
			exit();
		}

		if ($creating && SQ_IN_BACKEND) {
			$o->addOnLoad('focusFirstTextInput();');
		}
		// if there are no screens or static screens
		assert_false(!count($this->_screens) && empty($this->_edit_fns->static_screens));

		// set the type code again in case it has changed (like when morphing)
		$this->_type_code = $owner->type();


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

		if ($check_screen_restrictions) {
			$user_is_not_allowed = TRUE;

			foreach ($allowed_screens as $allowed_screen) {
				// if the allowed screen is in our array or if the access if granted too all the screens
				// be happy or present the user with login box
				if (isset($allowed_screen['screen']) && ($allowed_screen['screen'] === 0)) {
					$user_is_not_allowed = FALSE;
				} else if (isset($allowed_screen['screen']) && ($allowed_screen['screen'] == $active_screen)) {
					$user_is_not_allowed = FALSE;
				}//end if
			}//end foreach

			if ($user_is_not_allowed) {
				// this user is not allowed to view the current screen
				$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_inline_asset_screen', $active_screen, htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
				exit();
			}//end if
		}

		if (SQ_IN_LIMBO && $owner->id) {
				// broadcast the 'entering simple edit'
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_enter_simple_edit', $owner);
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

		// if we are in LIMBO and release locks button was clicked then return to normal page
		if (SQ_IN_LIMBO && !empty($_REQUEST['sq_lock_release_manual'])) {
			?>
			<script>top.location='<?php echo current_url(TRUE, TRUE);  ?>';</script>
			<?php
			exit();
		}

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
					$user    = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
					$editing = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

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

			if (SQ_IN_LIMBO && $do_the_limbo_lock && !SQ_DESIGN_NO_FRAME) {

				  /////////////////////
				 //  LIMBO LOCKING  //
				/////////////////////

				// even if we have all the locks for this screen, limbo might require more locks
				// so acquire any more locks that might be needed
				$this->acquireLimboLocks($owner, $o, $active_screen, $screen_lock_type);

			} else {

				if ((strtolower($screen_lock_type) != 'none') && empty($_REQUEST['print_view'])) {
					// lets tell the user if the asset is locked for editing
					// or if they can lock it to edit it
					$o->openRaw();
					?>
						<div class="sq-backend-section-div-wrapper section custom first">
							<div class="sq-backend-section-div">
								<div id="sq-aquire-locks-wrapper" class="clearfix">
									<?php
										if (!$have_all_locks || $have_any_locks) {
											?><div class="sq-locks-button-wrapper"><?php
										}

										if (!$have_all_locks) {
											hidden_field('sq_lock_acquire', '0');
											if ($need_force_locks) {
												if ($can_force_locks) {
													// they aren't going to do this without a fight though
													// they'll have to go through our popup window first
													$popup_url = '?SQ_BACKEND_PAGE=main&backend_section=am&ignore_frames=1&am_section=forceably_acquire_lock&assetid='.$owner->id.'&sq_lock_type='.$screen_lock_type;

													hidden_field('sq_lock_acquire_by_force', '0');
													normal_button('sq_forceably_acquire_lock', translate('acquire_lock_button'), "window.open('$popup_url', 'sq_forceably_acquire_lock_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300');", 'class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button acquire force"');
													echo '&nbsp;';
												}
											} else if ($full_write_access && $acquire_lock_access) {
												hidden_field('sq_lock_acquire_by_force', '0');
												normal_button('', translate('acquire_lock_button'), "document.getElementById('sq_lock_acquire').value = 1; submit_form(this.form)", 'accesskey="a" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button acquire"');
												echo '&nbsp;';
											}//end if
										}

										if ($have_any_locks) {
											hidden_field('sq_lock_release_manual', 0);
											normal_button('', translate('release_lock_button'), "document.getElementById('sq_lock_release_manual').value = 1; submit_form(this.form)", 'accesskey="r" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button release"');
											echo '&nbsp;';
										}

										if (!$have_all_locks || $have_any_locks) {
											?></div>
									<?php
									}
									?>

									<p class="sq-lock-message"><?php
										if (!$have_all_locks) {
											if ($need_force_locks) {
												if ($can_force_locks) {
													echo '<div class="sq-backend-locked-by-someone-else">'.translate('forcibly_acquire_lock', htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</div>';
												}
											} else if ($full_write_access && $acquire_lock_access) {
												echo translate('acquire_lock', translate('acquire_lock_button'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
											}
										}
										if ($have_any_locks) {
											echo translate('release_lock', translate('release_lock_button'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
										}
										?>
										<b class="clickable" onclick="sq_toggle_lock_info();" id="sq_lock_info_toggle"><?php echo translate('show_lock_details'); ?></b><br />
									</p><!--#sq-lock-message-->

									<div id="sq_lock_info" style="display: none;">
										<table class="sq-backend-table">
											<tr>
												<th class="sq-backend-table-header" style="width: 20px;">
													
												</th>
												<th class="sq-backend-table-header" align="left" nowrap>
													<?php echo translate('lock_type'); ?>
												</th>
												<th class="sq-backend-table-header">
													<?php echo translate('lock_details'); ?>
												</th>
											</tr>
										<?php
										$class = 'sq-backend-table-cell-alt';
										foreach ($locks as $lock_type => $lock) {
											?>
											<tr>
												<td class="sq-backend-table-cell" align="center">
													<img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/<?php echo ((empty($lock)) ? 'un' : ''); ?>locked.png" />
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
															echo translate('no_access_to_edit', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
														}

													} else {
														// this asset is currently locked so display message to the user
														$user    = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
														$editing = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

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

														echo translate('lock_held_by', htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlspecialchars($editing->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'. ';
														echo $expires_in;
													}//end if
													?>
												</td>
											</tr>
											<?php
										}//end foreach lock types
										?>
										</table>
									</div><!--#sq_lock_info-->

								</div><!--#sq-aquire-locks-wrapper-->
							</div><!--.sq-backend-section-div-->
						</div><!--.sq-backend-section-div-wrapper-->
					<?php
					$o->closeRaw();

					//$o->openSection(translate('locking_editing'));
					$o->openRaw();
						?>
						<!--

						
						-->
						
						<?php
					$o->closeRaw();
					//$o->closeSection();

				}//end if locktype not none

			}//end else (not in limbo or not do the limbo lock)

			$o->openRaw();
				hidden_field('sq_lock_type', $screen_lock_type);
			$o->closeRaw();

		}//end if

		// we need to check if we are going to paint the layout here, so no static screens get printed
		$paint_layout = FALSE;
		if (SQ_IN_LIMBO && !$creating) {
			// Check for a layout based on root node
			$layout_pattern = 'layout::'.$owner->type().'::'.$active_screen;
			$lookup_values = $owner->getLookupValues(TRUE, $layout_pattern);
			foreach ($lookup_values as $values) {
				foreach ($values as $look_key => $look_value) {
					if (isset($look_value['value']) && ($look_value['value'] > 0)) {
						$paint_layout = TRUE;
					}
				}
			}

			// If none found, try layout manager
			$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			if ($lm->hasLayout($owner, $active_screen)) {
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
		// determine and set the preview url, and return the closest parent id to be used below
		// in determining the page headings
		$closest_parentid = $this->_determinePreviewUrl($owner, $o);


		  ////////////////
		 /// LAYOUTS  ///
		////////////////
		// if we are ignoring frames - normally because we are editing in limbo, we want
		// to make sure we keep this setting for the future before we use layouts to print the interface
		if (isset($_REQUEST['ignore_frames']) && $_REQUEST['ignore_frames']) {
			$o->addFormActionGetVar('ignore_frames', '1', TRUE);
		}

		// once we have a preview URL, we can paint the layout if there is one
		// paint the layout now, if needed
		if ($paint_layout) {
			// Find a layout based on root node
			$layout_pattern = 'layout::'.$owner->type().'::'.$active_screen;
			$lookup_values = $owner->getLookupValues(TRUE, $layout_pattern);
			$layoutid = '';
			foreach ($lookup_values as $values) {
				foreach ($values as $look_key => $look_value) {
					if (isset($look_value['value'])) {
						$layoutid = $look_value['value'];
					}
				}
			}
			$layout = NULL;
			if ((!empty($layoutid)) && ($layoutid > 0)) {
				$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($layoutid);
			}
			if (!is_null($layout)) {
				$o->addFormActionGetVar('asset_ei_screen', $active_screen, TRUE);
				return $this->paintLayout($owner, $o, $layout);
			}
			// Otherwise refer to layout manager
			if (NULL !== ($layout = $lm->getLayout($owner, $active_screen))) {
				$o->addFormActionGetVar('asset_ei_screen', $active_screen, TRUE);
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
			$page_heading = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name').' ';
			if (empty($_REQUEST['print_view'])) {
				$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($owner->id);
				$page_heading .= '<ul class="info_asset_finder">';
				foreach ($infos as $assetid => $info) {
					$page_heading .= '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				$page_heading .= '</ul>';
			} else {
				$page_heading .= htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).' (Id: #'.$owner->id.') ';
			}
			if ($owner->version) {
				$page_heading .= '<span class="sq-info-asset-version">v. '.$owner->version.'</span>';
			} else {
				// Don't add version span if there is no version number available
			}
		} else {
			$page_heading = 'Create new '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->_type_code, 'name');
		}
		$page_heading .= ' <span class="sq-info-asset-screen-name">'.$active_screen_name.'</span>';

		$o->setHeading($page_heading, sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($this->_type_code), '', '', ''));

		$o->setPageTitle(htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
		$sub_heading = '';
		if ($closest_parentid != $owner->id) {
			$closest_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($closest_parentid);
			if (!is_null($closest_parent)) {
				$sub_heading = translate('currently_editing', '<strong>'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($closest_parent->type(), 'name').'</strong>', '<strong>'.htmlspecialchars($closest_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</strong>';
			}
		}
		$o->setSubHeading($sub_heading);


		  ///////////////////
		 //  SCREEN MENU  //
		///////////////////
		// add all the screens to the combo, but only if we aren't creating the object
		// set the current screen for both backend and frontend editing
		$current_location = $o->getCurrentLocation();
		list($current_loc_base, $current_loc_query) = explode('?', $current_location);
		if (!$creating) {
			// add static screens for the asset
			foreach ($this->_edit_fns->static_screens as $code_name => $data) {
				if ($check_screen_restrictions) {
					$user_is_not_allowed = TRUE;
					foreach ($allowed_screens as $allowed_screen) {
						if (isset($allowed_screen['screen']) && ($allowed_screen['screen'] == $code_name)) {
							$user_is_not_allowed = FALSE;
						}//end if
					}//end foreach

					if ($user_is_not_allowed) continue;
				}//end if
				$url = replace_query_string_vars(Array('asset_ei_screen' => rawurlencode($code_name)), $current_loc_base, $current_loc_query);
				$o->addStaticScreen($url, $data['name']);
			}
			for (reset($this->_screens); NULL !== ($code = key($this->_screens)); next($this->_screens)) {
				if ($check_screen_restrictions) {
					$user_is_not_allowed = TRUE;
					foreach ($allowed_screens as $allowed_screen) {
						if (isset($allowed_screen['screen']) && ($allowed_screen['screen'] == $code)) {
							$user_is_not_allowed = FALSE;
						}//end if
					}//end foreach

					if ($user_is_not_allowed) continue;
				}//end if
				if ($this->_screens[$code]['invisible'] && $code != $active_screen) {
					continue;
				}
				$url = replace_query_string_vars(Array('asset_ei_screen' => rawurlencode($code)), $current_loc_base, $current_loc_query);
				$o->addScreen($url, $this->_screens[$code]['name']);
			}//end for
		}//end if

		// Javascript code defined in edit function's paint function for Commit button's onclick event
		$onclick_js = '';

		  //////////////////////
		 //  EDIT INTERFACE  //
		//////////////////////
		if ($file) {
			$print_commit_button |= $this->printEditInterface('screen_'.$active_screen, $owner, $o, $creating, $create_sections, $onclick_js);
		}

		// we have printed an editable interface on a static screen - show the commit button
		if ($print_commit_button && empty($_REQUEST['print_view'])) {
			$o->commitButton('', $force_unlock_on_commit, TRUE, $onclick_js);
		}

	}//end paint()


	/**
	* Paints out the interface for the passed asset inline
	*
	* This means that it doesn't include the Commit Button, Locking, Page Headings, Screen Menu
	*
	* @param object		&$owner			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param boolean	$creating		whether we are processing the default create screen
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
				<span class="error"><?php echo translate('cannot_access_asset', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></span>
			<?php
			$o->closeField();
			return FALSE;
		}

		// if there are no screens or static screens
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
		if ($check_screen_restrictions) {
			$user_is_not_allowed = TRUE;
			foreach ($allowed_screens as $allowed_screen) {
				if (isset($allowed_screen['screen']) && ($allowed_screen['screen'] == $active_screen)) {
					$user_is_not_allowed = FALSE;
				}//end if
			}//end foreach

			if ($user_is_not_allowed) {
				$o->openField('');
				?>
					<span class="error"><?php echo translate('cannot_access_inline_asset_screen', $active_screen, htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></span>
				<?php
				$o->closeField();
				return FALSE;
			}//end if
		}//end if

		if (!isset($this->_edit_fns->static_screens[$active_screen]) && empty($this->_screens[$active_screen])) {
			// we cant find this screen, so we will use the details screen as a backup
			trigger_localised_error('SYS0076', E_USER_WARNING, $active_screen);
			$active_screen = 'details';
		}

		// we need to check if we are going to paint the layout here, so no static screens get printed
		$paint_layout = FALSE;
		if (SQ_IN_LIMBO && !$creating) {
			$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			if ($lm->hasLayout($owner, $active_screen)) {
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
			if (NULL !== ($layout = $lm->getLayout($owner, $active_screen))) {
				$o->addFormActionGetVar('asset_ei_screen', $active_screen, TRUE);
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
	* @param string		$onclick_js			Javascript code for the commit button's click event
	*
	* @return boolean
	* @access public
	*/
	function printEditInterface($screen, &$owner, &$o, $creating=FALSE, $create_sections=Array(), &$onclick_js='')
	{
		// grab the closest language file generated for this screen, then
		// unserialise it to get our XML_Tree root
		$root = $this->findScreen($screen);
		if (is_null($root)) return FALSE;

		$print_commit_button = FALSE;

		// prefix for all the form vars
		$prefix = $owner->getPrefix();

		// Loop over the sections
		foreach ($root->section as $section) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes()->limbo_access) || (int)$section->attributes()->limbo_access == 0)) {
				continue;
			}

			// if this ain't a section, or if we are printing the create interface and this section
			// is not included in that, ignore it
			if ($creating && empty($create_sections[(string)$section->attributes()->name])) {
				continue;
			}

			$section_access = $this->_getAccess($owner, $section, $prefix);
			if (!($section_access & SQ_EI_READ)) {
				continue;
			}

			// if a translated display name has been specified, use it
			if (!is_null($section->attributes()->display_name)) {
				$o->openSection((string)$section->attributes()->display_name);
			} else {
				$o->openSection((string)$section->attributes()->name);
			}

			if (!($num_fields = count($section->children()))) {
				continue;
			}

			$section_note = NULL;
			if ($section->function_call) {

				$paint_node = $section->function_call->paint_function;

				if (!isset($paint_node->attributes()->name) || !method_exists($this->_edit_fns, (string)$paint_node->attributes()->name)) {
					$o->openField('');
					echo translate('section_not_found', (string)$section->attributes()->name);
					$o->closeField();
				} else {
					// if we are printing the create interface and this section is not included in that, ignore it
					if ($creating && !in_array((string)$paint_node->attributes()->name, $create_sections[(string)$section->attributes()->name])) {
						continue;
					}

					if ($this->_edit_fns->{(string) $paint_node->attributes()->name}($owner, $o, $prefix)) {
						$print_commit_button = TRUE;
					}
				}

				// See if there's a section note
				if (isset($section->note)) {
					if (count($section->note) > 1) {
						throw new Exception('Unable to parse edit interface for asset type "'.$owner->type().'", screen "'.$screen.'": a section has multiple section notes; it should have just one');
					} else {
						$section_note = (string)$section->note;
					}
				}

			// they must just be normal fields
			} else {

				// Loop over the fields
				foreach ($section->children() as $field) {
					switch ($field->getName()) {
						case 'field':
							$display_name = '';
							$name = '';
							$field_note   = '';
							$attr_tag     = NULL; // TODO: TOF what is $attr_tag
							foreach ($field->children() as $sub_field) {
								switch ($sub_field->getName()) {
									case 'display_name':
										$display_name = (string)$sub_field;
									break;
									case 'note':
										$field_note = (string)$sub_field;
									break;

									default:
										if (!is_null($attr_tag)) {
											trigger_localised_error('SYS0194', E_USER_WARNING);
										} else {
											$attr_tag = $sub_field;
										}
									break;
								}//end switch

							}

							// if we are in limbo and this field is not to be printed in limbo - dont print it
							if (SQ_IN_LIMBO && (isset($field->attributes()->limbo_access) && (int)$field->attributes()->limbo_access == 0)) {
								continue;
							}

							$field_access = $this->_getAccess($owner, $field, $prefix, $section_access);
							if (!($field_access & SQ_EI_READ)) {
								continue;
							}

							if (!isset($field->attributes()->format)) {
								$field->attributes()->format = '';
							}

							if (!empty($field->attributes()->attribute)) {
								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if (($field->getName() != 'field') || ($creating && (empty($field->attributes()->attribute) || !in_array((string)$field->attributes()->attribute, $create_sections[(string)$section->attributes()->name])))) {
									continue;
								}

								// TODO TOF fix display_name
								$o->openField($display_name, $field->attributes()->format, '', isset($field->attributes()->hidden));
								$attr = $owner->getAttribute((string)$field->attributes()->attribute);

								if (is_null($attr)) continue;
								// TODO: TOF fix the line below
								if (!$attr->setEditParams($attr_tag)) continue;
								$attr->paint($prefix.'_'.$attr->id, !($field_access & SQ_EI_WRITE), $owner);
								if ($field_access & SQ_EI_WRITE) {
									$print_commit_button = TRUE;
								}

								if (!empty($field_note)) {
									$field_note = str_replace('%description%', $attr->description, $field_note);
								}

							} else if ($attr_tag->getName() == 'function_call') {

								$paint_node = $attr_tag->paint_function;

								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->getName() != 'field' || ($creating && !in_array((string) $paint_node->attributes()->name, $create_sections[(string) $section->attributes()->name]))) {
									continue;
								}

								$o->openField($display_name, $field->attributes()->format, '', isset($field->attributes()->hidden));

								if (empty($paint_node->attributes()->name) || !method_exists($this->_edit_fns, $paint_node->attributes()->name)) {
									$o->note(translate('specific_painting_function_not_found', $paint_node->attributes()->name));
								} else {
									
									$method_name = (string) $paint_node->attributes()->name;
									if ($this->_edit_fns->{$method_name}($owner, $o, $prefix, $onclick_js)) {
										$print_commit_button = TRUE;
									}
								}

							}//end if

							if (!empty($field_note)) $o->note($field_note);

						break;

						case 'note':
							// notice that only the last "note" will be used if multiple are supplied
							if (!empty($field)) {
								$section_note = (string)$field;
							}
						break;

						default:
							trigger_localised_error('SYS0140', E_USER_WARNING, $field->name);
							continue;

					}//end switch

				}//end foreach fields

			}//end else section uses function call

			if (!empty($section_note)) {
				$o->sectionNote($section_note);
			}

			$o->closeSection();

		}//end foreach

		// paint the choosing link section when creating a new asset in backend
		if($creating && SQ_IN_BACKEND) {
			$o->openSection(translate('link'));
			$this->_edit_fns->paintCreateLink($owner, $o, $prefix);
			$o->closeSection();
		}
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

		$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');

		if ($lm->hasLayout($owner, $active_screen)) {

			$layout = $lm->getLayout($owner, $active_screen);
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
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetids'			=> Array($owner->id,),
						'lock_type'			=> $locks,
						'forceably_acquire'	=> FALSE,
					);

			// if redirect locations is already set, pass it onto hipo job
			$on_complete_url = (empty($o->_redirect) ? '' : $o->_redirect);
			$hh->queueHipo('hipo_job_acquire_locks', $vars, $on_complete_url);
			$url = $hh->runQueuedJobs();
			if (!empty($url)) {
				// redirect to the correct url and fire hipo (not passing sq_redirect_url)
				// so that we do not redirect and refresh many times
				$o->setRedirect($url);
			}
		}

		$o->openRaw();
			hidden_field('sq_limbo_lock_type', urlencode(serialize($locks)));
		$o->closeRaw();

	}//end acquireLimboLocks()


	/**
	* Wrapper Function : Prints limbo in a specified layout
	*
	* @param object	&$owner		the owner of this interface
	* @param object	&$o			the backend outputter
	* @param object	&$layout	the layout to be painted
	* @param boolean	$type_code_prefix	whether to use type_code as a part of metadata field id's prefix
	*
	* @return boolean
	* @access public
	*/
	function paintLayout(&$owner, &$o, &$layout, $type_code_prefix=FALSE)
	{
		// Get the order in which keywords are placed in the raw content in container.
		// This order is required to deciede which keyword to evaluate first. Say a metadata keyword is placed
		// after custom keyword in a container, if it is evaluted before custom keyword, the the js code for wysiwyg (i.e.
		// if metadata contains wysiwyg field) is placed with metadata keyword. In this case, custom key will only have 
		// function call to wysiwyg (as the wysiwyg js code printed only once in a page), but since it is defined later in page, 
		// its will give 'undefined function call' js error.
		//
		// See bug #3698 + bug #4458 Conflicting WYSIWYG's
		$layout_keywords_order = $this->_getLayoutKeywordsOrder($layout);

		$this->getKeywordsReplacementsForPaint($owner, $o, $layout, $layout_keywords_order, TRUE, FALSE, $type_code_prefix);

		// we need to open a raw section so that the form gets printed before the contents of the layout
		$o->openRaw();
			$layout->printBody();
		$o->closeRaw();

		return TRUE;

	}//end paintLayout()


	/**
	* Gets keywords replacements for printing limbo in a specified layout
	*
	* @param object  &$owner						the owner of this interface
	* @param object	 &$o							the backend outputter
	* @param object	 &$layout					the layout to be painted
	* @param array	 &$layout_keywords_order		order of appearence of the keywords
	* @param boolean &$set_keywords_replacement	set keyword in body or just return replacements
	* @param boolean &$invoke_backend			the layout to be painted
	* @param boolean $type_code_prefix	whether to use type_code as a part of metadata field id's prefix
	*
	* @return mixed
	* @access public
	*/
	public function getKeywordsReplacementsForPaint(&$owner, &$o, &$layout, $layout_keywords_order, $set_keywords_replacement = TRUE, $invoke_backend = FALSE, $type_code_prefix=FALSE)
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

		foreach ($layout_keywords_order as $order_keyword) {
			// loop over the screens and get the xml file for each screen
			// generate some keywords for each screen
			if ($order_keyword == 'screen') {
				foreach ($screens as $screen) {
					// check to see if we want the entire screen
					$entire_screen = FALSE;
					if (in_array($screen, array_keys($keywords['entire_screens']))) {
						$entire_screen = TRUE;
					}

					// there is a problem with metadataSchemas and lookup settings
					// a quick fix was to add in the "if" statement a condition for
					// metadataschema and lookupSetting

					// if we have a static screen on our hands, then we want to
					// call the function to paint it
					if (isset($this->_edit_fns->static_screens[$screen]) || (($screen == 'metadataschemas') && (isset($this->_edit_fns->static_screens['metadataSchemas']))) || (($screen == 'lookupvalues') && (isset($this->_edit_fns->static_screens['lookupValues']))) ) {
						$static_keywords = (isset($keywords['screens'][$screen])) ? $keywords['screens'][$screen] : NULL;
						// for situations where there are keywords from a static screen like 'metadata' and the commit_button keyword is used
						if (!is_null($static_keywords)) {
							$print_commit_button = TRUE;
						}
						if (!is_null($static_keywords) || $entire_screen) {
							if (is_null($static_keywords)) {
								$static_keywords = Array();
							}
							if ($screen == 'metadataschemas') {
								if ($this->_getStaticScreenKeywordReplacements($owner, $o, 'metadataSchemas', $static_keywords, $entire_screen)) {
									$print_commit_button = TRUE;
								}
							} else if ($screen == 'lookupvalues') {
								if ($this->_getStaticScreenKeywordReplacements($owner, $o, 'lookupValues', $static_keywords, $entire_screen)) {
									$print_commit_button = TRUE;
								}
							} else if ($screen == 'metadata') {
								$layout_options = $layout->attr('options');
								$layout_options['type_code_prefix'] = $type_code_prefix;
								if ($this->_getStaticScreenKeywordReplacements($owner, $o, $screen, $static_keywords, $entire_screen, $layout_options)) {
									$print_commit_button = TRUE;
								}
							} else if ($this->_getStaticScreenKeywordReplacements($owner, $o, $screen, $static_keywords, $entire_screen)) {
								$print_commit_button = TRUE;
							}
						}
					}//end if
					// Metadata & workflow don't have an xml file defined, so move onto the next screen.
					if ($screen == 'metadata') continue;
					if ($screen == 'workflow') continue;
					if ($screen == 'tagging') continue;
					if ($screen == 'settings') continue;
					if ($screen == 'permissions')	continue;
					if ($screen == 'roles') continue;
					if ($screen == 'metadataschemas') continue;
					if ($screen == 'dependants') continue;
					if ($screen == 'linking') continue;
					if ($screen == 'history') continue;
					if ($screen == 'logs') continue;
					if ($screen == 'preview') continue;
					if ($screen == 'tagging') continue;
					if ($screen == 'lookupvalues') continue;
					if ($screen == 'layouts') continue;
		
					// its not a static screen, so get the xml file and parse it for
					// the keywords that we want, if the xml file exists
					$file = $this->findLanguageFile('screen_'.$screen);
					if (empty($file)) {
						$file  = $this->findFile('screen_'.$screen);
						if(empty($file)) {
							continue;
						}
						try {
							$root = simplexml_load_string(file_get_contents($file), 'SimpleXMLElement', LIBXML_NOCDATA);
						} catch (Exception $e) {
							throw new Exception('Unable to parse localised screen "'.$screen.'" for asset type "'.$owner->type().'": '.$e->getMessage());
						}
		
					} else {
						$root = simplexml_load_string(file_get_contents($file), 'SimpleXMLElement', LIBXML_NOCDATA);
					}
		
					if ($root) {
						// get the layout replacements for this NON-static screen
						if ($this->_getLayoutKeywordReplacements($root, $o, $owner, $keywords, $screen, $entire_screen)) {
							$print_commit_button = TRUE;
						}
					}//end if
	
				}//end foreach

			}// End if screen

			// if there are some custom keywords, get some replacements from the owner
			if (isset($keywords['custom']) && !empty($keywords['custom']) && $order_keyword == 'custom') {
				$custom_replacements = $owner->getCustomKeywordReplacements($keywords['custom'], $invoke_backend);

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
		
		}// End foreach layout_keywords_order

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

			} else if (($keyword == 'commit_button_clean') && $print_commit_button) {
				ob_start();
					$o->commitButton('', FALSE, FALSE);
					$this->_tmp['layout_keyword_replacements']['commit_button_clean'] = ob_get_contents();
				ob_end_clean();
				continue;

			} else if (($keyword == 'release_locks_button')) {
				ob_start();
					submit_button('sq_lock_release_manual', translate('release_lock_button'), '', 'accesskey="r"');
					$this->_tmp['layout_keyword_replacements']['release_locks_button'] = ob_get_contents();
				ob_end_clean();
				continue;

			} else if ($keyword == 'screen_menu') {
				$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
				// if we're using a different limbo assetid, make sure that the
				// assetid is being brought through
				if (SQ_IN_LIMBO && isset($_REQUEST['limbo_assetid'])) {
					$o->addFormActionGetVar('limbo_assetid', $_REQUEST['limbo_assetid'], TRUE);
				}
				$current_location = $o->getCurrentLocation();
				list($current_loc_base, $current_loc_query) = explode('?', $current_location);

				for (reset($this->_screens); NULL !== ($code = key($this->_screens)); next($this->_screens)) {
					if (!$lm->hasLayout($owner, $code)) continue;
					$url = replace_query_string_vars(Array('asset_ei_screen' => rawurlencode($code)), $current_loc_base, $current_loc_query);
					$o->addScreen($url, $this->_screens[$code]['name']);
				}
				foreach ($this->_edit_fns->static_screens as $code_name => $data) {
					if (!$lm->hasLayout($owner, $code_name)) {
						continue;
					}
					$url = replace_query_string_vars(Array('asset_ei_screen' => rawurlencode($code_name)), $current_loc_base, $current_loc_query);
					$o->addStaticScreen($url, $data['name']);
				}
				ob_start();
					$o->printScreenMenu();
					$this->_tmp['layout_keyword_replacements']['screen_menu'] = ob_get_contents();
				ob_end_clean();
			} else if ($keyword == 'context_switcher') {
				ob_start();
					$o->printSimpleContextSwitcher();
					$this->_tmp['layout_keyword_replacements']['context_switcher'] = ob_get_contents();
				ob_end_clean();

				continue;
			} else {
				$this->_tmp['layout_keyword_replacements'][$keyword] = $owner->getKeywordReplacement($keyword);

				// Replace Global Keywords when in a Simple Edit Layout
				if (strpos($this->_tmp['layout_keyword_replacements'][$keyword], '%globals_') === 0) {
					replace_global_keywords($this->_tmp['layout_keyword_replacements'][$keyword]);
				}
			}
		}//end foreach

		// turn buffering back on so the output is not malformed
		$o->setBuffering(TRUE);

		if (!$print_commit_button) {
			$this->_tmp['layout_keyword_replacements']['commit_button'] = '';
			$this->_tmp['layout_keyword_replacements']['commit_button_clean'] = '';
		}

		if (!$set_keywords_replacement) return $this->_tmp['layout_keyword_replacements'] ;

		// tell the layout what the keyword replacements are
		$layout->setKeywordReplacements($this->_tmp['layout_keyword_replacements']);

	}//end getKeywordsReplacementsForPaint()


	/**
	* Gets a keyword replacement for static screens.
	*
	* @param object		&$owner			the owner of this asset
	* @param object		&$o			the backend outputter object
	* @param string		$screen			the static screen to get a replacement for
	* @param array|NULL	$static_keywords	the wantedkeyowrd of this static screen
	* @param boolean	$get_screen		if TRUE the entire screen will be captured into a replacement
	* @param Array		$layout_options		the Simple Edit layout options
	*
	* @return boolean
	* @access private
	*/
	function _getStaticScreenKeywordReplacements(&$owner, &$o, $screen, $static_keywords, $get_screen=FALSE, Array $layout_options=Array())
	{
		$this->includeStaticScreen($screen);
		$print_commit_button = FALSE;

		ob_start();
			$paint_function = 'paint'.$screen;
			if ($paint_function($owner, $o, $this, $static_keywords, $layout_options)) {
				$print_commit_button = TRUE;
			}

			if ($get_screen) {
				if ($screen == 'metadataSchemas') {
					$this->_tmp['layout_keyword_replacements']['metadataschemas-'] = ob_get_contents();
				} else if ($screen == 'lookupValues') {
					$this->_tmp['layout_keyword_replacements']['lookupvalues-'] = ob_get_contents();
				} else {
					$this->_tmp['layout_keyword_replacements'][$screen.'-'] = ob_get_contents();
				}
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
		foreach ($root->section as $section) {

			// find out if we have permission to print this section
			$limbo_access = (isset($section->attributes()->limbo_access) && (int) $section->attributes()->limbo_access == 1) ? TRUE : FALSE;
			if ($limbo_access == FALSE) continue;

			// find out what we need to get in this loop
			$get_section = FALSE;
			$section_name = strtolower(str_replace(' ', '_', (string)$section->attributes()->name));
			if (isset($layout_keywords['screens'][$screen]['sections'][$section_name])) {
				$get_section = TRUE;
				$section_keyword = $layout_keywords['screens'][$screen]['sections'][$section_name];
			}

			if (!($num_fields = count($section->children()))) {
				continue;
			}

			$section_access = $this->_getAccess($owner, $section, $prefix);
			if (!($section_access & SQ_EI_READ)) {
				continue;
			}

			// if we are getting this section, start buffering output
			if ($get_section) ob_start();
			if ($get_section || $get_screen) {
				$o->openSection((string)$section->attributes()->name);
			}

			if (isset($section->function_call)) {// TODO TOF FIX
				if ($get_section || $get_screen) {
					$paint_node = $section->children[0]->children[0];

					if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
						$o->openField('');
						echo translate('section_not_found', $section->attributes['name']);
						$o->closeField();
					} else {
						if ($this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix)) {
							$print_commit_button = TRUE;
						}
					}

					// Loop over the fields to see if there's a section note
					for ($j = 0; $j < count($section->children); $j++) {
						switch ($section->children[$j]->name) {
							case 'note':
								// notice that only the last "note" will be used if multiple are supplied
								if (!empty($section->children[$j]->content)) {
									$section_note = $section->children[$j]->content;
								}
							break;
						}
					}
				}

			// they must just be normal fields
			} else {
				$section_note = NULL;

				// Loop over the fields
				foreach ($section->children() as $field) {
					switch ($field->getName()) {
						case 'field':
							$display_name = '';
							$field_note   = '';
							$attr_tag     = NULL;

							foreach ($field->children() as $sub_field) {
								switch ($sub_field->getName()) {
									case 'display_name':
										$display_name = (string)$sub_field;
									break;

									case 'note':
										$field_note = (string)$sub_field;
									break;

									default:
										if (!is_null($attr_tag)) {
											trigger_localised_error('SYS0194', E_USER_WARNING);
										} else {
											$attr_tag = $sub_field;
										}
									break;
								}//end switch
							}

							$field_access = $this->_getAccess($owner, $field, $prefix, $section_access);
							if (!($field_access & SQ_EI_READ)) {
								continue;
							}

							if (!isset($field->attributes()->format)) {
								$field->attributes()->format = '';
							}

							// reset get_field
							$get_field = FALSE;

							if (isset($field->attributes()->attribute)) {
								// find out what we need to get in this loop
								if (isset($layout_keywords['screens'][$screen]['fields'][(string)$field->attributes()->attribute])) {
									$get_field = TRUE;
									$field_keyword = $layout_keywords['screens'][$screen]['fields'][(string)$field->attributes()->attribute];
								}
								// if we do not want anything in this loop, continue
								if (!$get_field && !$get_section && !$get_screen) {
									continue;
								}

								if ($field->getName() != 'field') continue;
								$o->openField($display_name, (string)$field->attributes()->format, '', isset($field->attributes()->hidden));

								$attr = $owner->getAttribute((string)$field->attributes()->attribute);

								if (is_null($attr)) continue;
								if (!$attr->setEditParams($attr_tag)) continue;
								// if this is not a wanted keyword, then continue on our way
								if (isset($layout_keywords['screens'][$screen]['fields'][(string)$field->attributes()->attribute]))

								// start buffering for this field
								if ($get_field) ob_start();

								$attr->paint($prefix.'_'.$attr->id, !(($field_access & SQ_EI_WRITE) && $limbo_access), $owner);
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
							
								// Field id layout keyword replacement
								if (isset($layout_keywords['screens'][$screen]['field_ids'][(string)$field->attributes()->attribute])) {
									$field_id_keyword = $layout_keywords['screens'][$screen]['field_ids'][(string)$field->attributes()->attribute];
									$this->_tmp['layout_keyword_replacements'][$field_id_keyword] = $prefix.'_'.$attr->id;
								}

							} else if ($attr_tag->getName() == 'function_call') {

								if (isset($field->attributes()->keyword)) {
									// find out what we need to get in this loop
									if (isset($layout_keywords['screens'][$screen]['fields'][(string)$field->attributes()->keyword])) {
										$get_field = TRUE;
										$field_keyword = $layout_keywords['screens'][$screen]['fields'][(string)$field->attributes()->keyword];
									}
								}

								$paint_node = $attr_tag->paint_function;
								if ($field->getName() != 'field') continue;

								$o->openField($display_name, (string)$field->attributes()->format, '', isset($field->attributes()->hidden));

								if ((!isset($paint_node->attributes()->name)) || !method_exists($this->_edit_fns, (string)$paint_node->attributes()->name)) {
									echo translate('painting_funtion_not_found');
								} else {
									if ($get_field) ob_start();

									if ($this->_edit_fns->{(string)$paint_node->attributes()->name}($owner, $o, $prefix)) {
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
							if (!empty($field->note)) {
								$section_note = (string)$field->note;
							}
						break;
					}//end switch
				}//end foreach
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
		// determine and set the preview url
		// we don't care about the returned value, it's not used here
		$this->_determinePreviewUrl($owner, $o);

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

		// if there are no screens or static screens
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

			$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
			$layout = $lm->getLayout($owner, $current_screen);

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

					if (!empty($allowed_screens)) {
						$user_is_not_allowed = TRUE;
						foreach ($allowed_screens as $allowed_screen) {
							if (isset($allowed_screen['screen']) && ($allowed_screen['screen'] == $active_screen)) {
								$user_is_not_allowed = FALSE;
							}//end if
						}//end foreach

						if ($user_is_not_allowed) {
								// this user is not allowed to view the current screen
								$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
								exit();
						}//end if
					}//end if
				}//end if

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

				// If we have just reverted to the system version, so don't overwrite those 
				// with the attribute values submitted in safe edit mode (see bug #5786)
				if (isset($owner->_tmp['reverting_to_system_version']) && $owner->_tmp['reverting_to_system_version']) {
					return TRUE;
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
						try {
							$root = simplexml_load_string(file_get_contents($file), 'SimpleXMLElement', LIBXML_NOCDATA);
						} catch (Exception $e) {
							throw new Exception('Unsable to find static screen "'.$active_screen.'" for asset type "'.$owner->type().'": '.$e->getMessage());
						}
					}
				} else {
					$root_from_file = file_get_contents($file);
					$root = $output = simplexml_load_string($root_from_file);
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

				// Loop over the sections (skipping any non-sections that may
				// have crept in for some unknown reason)
				foreach ($root->section as $section) {
					// if we are in limbo and this section is not to be printed in limbo - dont print it

					if (SQ_IN_LIMBO && (!isset($section->attributes()->limbo_access) || (int)$section->attributes()->limbo_access == 0)) {
						continue;
					}

					// if we are printing the create interface and this section
					// is not included in that, ignore it
					if ($creating && empty($create_sections[(string)$section->attributes()->name])) {
						continue;
					}

					if (!($num_fields = count($section->children()))) {
						continue;
					}

					$section_access = $this->_getAccess($owner, $section, $prefix);
					if (!($section_access & SQ_EI_READ)) {
						continue;
					}

					// if this is section only has function call, then process accordingly
					if ($section->function_call) {

						$process_node = $section->function_call->process_function;

						// if there is nothing declared there is nothing for us to do
						if (empty($process_node) || empty($process_node->attributes()->name)) {
							continue;
						// There is something declared but it doesn't exist
						} else if (!method_exists($this->_edit_fns, (string)$process_node->attributes()->name)) {
							trigger_localised_error('SYS0188', E_USER_WARNING, $section->attributes()->name);
						} else {
							// if we are processing the create interface and this section function is not included in that, ignore it
							if ($creating && !in_array((string)$process_node->attributes()->name, $create_sections[(string)$section->attributes()->name])) {
								continue;
							}

							if ($this->_edit_fns->{(string)$process_node->attributes()->name}($owner, $o, $prefix)) {
								$ret_val = TRUE;
							}

						}

					// they must just be normal fields
					} else {
						// Loop over the fields
						foreach ($section->field as $field) {
							if ($field->getName() == 'note') continue;

							$child_fields = $field->children();
							// in the previous implementation of matrix the first child
							// was the "display_name" and the second last was the "note"
							// So the last child was the remaining element (i.e.: text, boolean, email)
							// we are now changing it so $last_child variable contains the element which
							// is not the "display_name" or not the "note' element
							// $last_child   = $child_fields->{count($child_fields)-1};
							$last_child = NULL;
							foreach ($child_fields as $child) {
								$child_name = $child->getName();
								if (($child_name != 'display_name') && ($child_name != 'note')) {
									$last_child = $child;
									break;
								}
							}

							$field_access = $this->_getAccess($owner, $field, $prefix, $section_access);
							if (!($field_access & SQ_EI_WRITE)) {
								continue;
							}

							if (!empty($field->attributes()->attribute)) {
								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->getName() != 'field' || ($creating && (empty($field->attributes()->attribute) || !in_array((string)$field->attributes()->attribute, $create_sections[(string)$section->attributes()->name])))) {
									continue;
								}

								$attr = $owner->getAttribute((string)$field->attributes()->attribute);
								if (is_null($attr)) continue;
								if (!$attr->setEditParams($last_child)) continue;
							
								$revert_attr = $attr;
								$attr->process($prefix.'_'.$attr->id, $owner->id);
								if ($attr->processed && $owner->setAttrValue($attr->name, $attr->value)) {
									$ret_val = TRUE;
								} else {
									// processing failed so revert to old version
									$attr = $revert_attr;
								}
								unset($revert_attr);

							} else if ($last_child->getName() == 'function_call') {

								// If there is no process function node at all, then skip
								if (!isset($last_child->process_function)) continue;

								$process_node = $last_child->process_function;

								// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
								if ($field->getName() != 'field' || ($creating && !in_array((string)$process_node->attributes()->name, $create_sections[(string)$section->attributes()->name]))) {
									continue;
								}

								// if there is nothing declared there is nothing for use to do
								if (!isset($process_node->attributes()->name)) {
									continue;
								// There is something declared but it doesn't exist
								} else if (!method_exists($this->_edit_fns, (string)$process_node->attributes()->name)) {
									trigger_localised_error('SYS0187', E_USER_WARNING, (string)$section->attributes()->name, (string)$last_child);
								} else {
									if ($this->_edit_fns->{(string)$process_node->attributes()->name}($owner, $o, $prefix)) {
										$ret_val = TRUE;
									}
								}

							}//end if

						}//end foreach fields

					}//end else section funcion call

				}//end foreach

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
		if (isset($node->attributes()->show_if)) {
			$show_if_fn = (string)$node->attributes()->show_if;
			$edit_fns = $owner->getEditFns();

			if (method_exists($edit_fns, $show_if_fn)) {
				if (!$edit_fns->$show_if_fn($owner, $prefix)) return 0;
			} else {
				// if show if function doesn't exist, error, then continue as normal
				trigger_localised_error('SYS0136', E_USER_WARNING, $show_if_fn, $node->name);
				return 0;
			}
		}

		// if there is nothing specified, and if an inherited access was specified we must use that
		if (!isset($node->attributes()->write_access) && !is_null($inherit_access)) {
			if ($inherit_access & SQ_EI_WRITE) {
				$access |= SQ_EI_WRITE;
			}

		// otherwise attempt to find write access
		} else {
			$wa = (!isset($node->attributes()->write_access)) ? 'write:all' : (string)$node->attributes()->write_access;
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
			if (!isset($node->attributes()->read_access) && !is_null($inherit_access)) {
					if ($inherit_access & SQ_EI_READ) {
						$access |= SQ_EI_READ;
					}

			// otherwise attempt to find read access
			} else {
				$ra = (!isset($node->attributes()->read_access)) ? 'read' : (string)$node->attributes()->read_access;
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
		$db = MatrixDAL::getDb();

		// get all the user groups that we exist in
		$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$groups = $user->getUserGroups();
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

		if (empty($groups)) return Array(); // all screens allowed

		$bind_vars = Array();
		foreach ($groups as $key => $group) {
			$bind_vars['group_'.$key] = (string) $group;	
		}

		$sql = 'SELECT screen, section
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_edit_access ';
		$where = 'userid IN (:'.implode(', :', array_keys($bind_vars)).')';

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$where .= ' AND (type_code = :null_type_code
					OR type_code IN (
									SELECT inhd_type_code
									FROM sq_ast_typ_inhd
									WHERE type_code = :type_code
									))';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			MatrixDAL::bindValueToPdo($query, 'null_type_code', '0');
			MatrixDAL::bindValueToPdo($query, 'type_code',      $owner->type());
			$allowed_screens = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get allowed screens for type code: '.$owner->type().' due to database error: '.$e->getMessage());
		}
		return $allowed_screens;

	}//end getAllowedScreens()


	/**
	* Takes the requested filename and searches back up the directories to find the matching file
	* closest to the type of the asset that we are creating or editing
	*
	* @param string	$file_name	the file to find
	* @param string $throw_error	this boolean controls if the findFile should throw an error or not
	*
	* @return string
	* @access public
	*/
	function findFile($file_name, $throw_error=TRUE)
	{	
		$file_name = 'edit_interface_'.$file_name.'.xml';

		$file = '';
		for ($i = 0; $i < count($this->_dirs); $i++) {
			if (file_exists($this->_dirs[$i].'/'.$file_name)) {
				$file = $this->_dirs[$i].'/'.$file_name;
				break;
			}
		}
		if ( (!$file) && $throw_error) {
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
			try {
				$root = simplexml_load_string(file_get_contents($file), 'SimpleXMLElement', LIBXML_NOCDATA);
			} catch (Exception $e) {
				throw new Exception('Unable to parse screen file "'.$file.'": '.$e->getMessage());
			}

			// if empty, then just skip the searching for language files,
			// otherwise find the closest language file
			if (!empty($root) && (count($root->children()) > 0)) {
				$lang_file = $this->findLanguageFile($screen_name);
				if (empty($lang_file)) {
					trigger_localised_error('SYS0197', E_USER_WARNING, $screen_name);
					$root = NULL;
					return $root;
				} else {
					$root_xml = file_get_contents($lang_file);
					$root = simplexml_load_string($root_xml);
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

		try {
			$root = simplexml_load_string(file_get_contents($create_file));
		} catch (Exception $e) {
			throw new Exception('Unable to parse Create screen file "'.$create_file.'": '.$e->getMessage());
		}

		// Loop over the sections
		foreach ($root->section as $section) {

			$create_sections[(string)$section->attributes()->name] = Array();
			// Loop over the fields

			foreach ($section->children() as $field) {
				switch ($field->getName()) {
					case 'function_call' :
						if ($painting) {
							$node_name = 'paint_function';
						} else {
							$node_name = 'process_function';
						}
						$fn_node = $field->{$node_name};
						$create_sections[(string)$section->attributes()->name][] = (string)$fn_node->attributes()->name;
					break;

					case 'field' :
						if (isset($field->attributes()->attribute)) {
							$create_sections[(string)$section->attributes()->name][] = (string)$field->attributes()->attribute;
						} else if (isset($field->function_call)) {
							// There is a function call
							if ($painting) {
								$node_name = 'paint_function';
							} else {
								$node_name = 'process_function';
							}
							$fn_node = $field->function_call->{$node_name};
							if (isset($fn_node['name'])) {
								$create_sections[(string)$section->attributes()->name][] = (string)$fn_node->attributes()->name;
							}
						} else {
							throw new Exception('Unable to parse Create screen file "'.$create_file.'": it contains a field tag without an attribute or function call specified:'.$field->asXml());
						} //end if
					break;

				}// end switch

			}//end foreach

		}//end foreach

		return Array($create_sections, (string)$root->attributes()->screen);
//		return Array($create_sections, $root->attributes['screen']);

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


	/**
	* Takes the requested filename and searches back up the directories to find the matching file
	* closest to the type of the asset that we are creating or editing
	*
	* @param string	$file_name	the file to find
	* @param string $throw_error	this boolean controls if the findFile should throw an error or not
	*
	* @return string
	* @access public
	*/
	function findSpecificFile($file_name, $throw_error=TRUE)
	{
		if (file_exists($file_name)) return $file_name;
		if ($throw_error) {
			trigger_localised_error('SYS0065', E_USER_WARNING, $file_name, $this->_type_code);
			return FALSE;
		}

		return FALSE;

	}//end findSpecificFile()


	/**
	* works the same way as findScreen but uses a specific file name instead
	* @param string	$specific_file	the name of the specific file
	*
	* @return object
	* @access public
	*/
	function &findSpecificScreen($specific_file)
	{
		$root = NULL;
		if (($file = $this->findSpecificFile($specific_file))) {
			try {
				$root = simplexml_load_string(file_get_contents($file), 'SimpleXMLElement', LIBXML_NOCDATA);
				return $root;
			} catch (Exception $e) {
				throw new Exception('Unable to parse specific screen file "'.$file.'": '.$e->getMessage());
				$root = NULL;
				return $root;
			}

			// if empty, then just skip the searching for language files,
			// otherwise find the closest language file
			if (!empty($root) && (count($root->children()) > 0)) {
				$lang_file = $this->findLanguageFile($screen_name, FALSE);
				if (empty($lang_file)) {
					$root = NULL;
					return $root;
				} else {
					$root = unserialize(file_get_contents($lang_file));
				}
			}
		}

		return $root;

	}//end findSpecificScreen()


	/**
	*
	* prerequisites: the asset_type should exist $asset_type and
	* the outputter should be already initialised.
	* Info: This function will print all the keywords of the asset type
	* passed as a parameter.
	* outcome: It will use the outputter $o to display all the simple
	* edit keywords available
	*
	* @param string	$asset_type	the asset type
	* @param object &$o			the outputter
	*
	* @return void
	* @access public
	*/
	function getSimpleEditKeywords($asset_type='page_standard', &$o)
	{

		$ei = new Asset_Edit_Interface($asset_type);

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);
		$as = new $asset_type();
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_type);
		$asset_dir = SQ_SYSTEM_ROOT.'/'.$asset_info['dir'];
		$ef = $as->getEditFns();

		$screens_static = Array();
		$screens_specific = Array();

		foreach ($ef->static_screens as $key => $screen) {
			$screens_static[$screen['name']] = $key;
		}

		$screens_all = $screens_static;
		foreach ($ei->getScreens() as $key => $screen) {
			$screens_specific[$screen['name']] = $key;
			$screens_all[$screen['name']] = $key;
		}

		$o->openSection('General Screens');

		// loop through all the screen and all the xml files
		foreach ($screens_all as $current_screen_name => $current_screen) {

			// the dependant and format screens have some problems with the links displayed
			// therefore we dont display dependants and format screens keywords
			// until they are fixed
			if ($current_screen == 'dependants') continue;
			$section_array = Array();

			if ($this->findFile('static_screen_'.$current_screen, FALSE)) {
				$root = $this->findScreen('static_screen_'.$current_screen);

				foreach ($root->section as $section) {

					// if the section should not be displayed, continue
					if ((isset($section->attributes()->limbo_access)) && (((int) $section->attributes()->limbo_access) === 0)) {
						continue;
					}

					$current_section_name = (string) $section->attributes()->name;

					if (!isset($section_array[$current_section_name])) {
						$section_array[$current_section_name] = Array();
					}

					foreach ($section->children() as $field) {
						$keyword = (string) $field->attributes()->code;
						if (!empty($keyword)) {
							$section_array[$current_section_name][] = $keyword;
						}
					}

				}
			}

			if ($this->findFile('screen_'.$current_screen, FALSE)) {
				$root = $this->findScreen('screen_'.$current_screen);

				foreach ($root->section as $section) {

					// if the section should not be displayed, continue
					if ((isset($section->attributes()->limbo_access)) && (((int) $section->attributes()->limbo_access) === 0)) {
						continue;
					}

					$current_section_name = (string) $section->attributes()->name;
					if (!isset($section_array[$current_section_name])) {
						$section_array[$current_section_name] = Array();
					}

					foreach ($section->children() as $field) {
						$keyword = (string) $field->attributes()->attribute;
						if (!empty($keyword)) {
							$section_array[$current_section_name][] = $keyword;

						}
					}
				}
			}

			if ($tmpFile=$this->findSpecificFile($asset_dir.'/edit_interface_screen_'.$current_screen.'.xml', FALSE)) {

				$root = $this->findSpecificScreen($tmpFile);

				foreach ($root->section as $section) {

					// if the section should not be displayed, continue
					if ((isset($section->attributes()->limbo_access)) && ((int) $section->attributes()->limbo_access === 0)) {
						continue;
					}

					$current_section_name = (string) $section->attributes()->name;

					if (!isset($section_array[$current_section_name])) {
						$section_array[$current_section_name] = Array();
					}

					foreach ($section->children() as $field) {
						$keyword = (string) $field->attributes()->attribute;
						if (empty($keyword)) {
							$keyword = $field->attributes()->keyword;
						}
						if (!empty($keyword)) {
							$section_array[$current_section_name][] = $keyword;
						}
					}

				}

				// remove duplicates fields
				foreach ($section_array as $key => $value) {
					$value = (array_unique($value));
					$section_array[$key]= $value;
				}
			}//end if

			$counter = 0;
			reset($screens_specific);

			if (!strcmp($current_screen, current($screens_specific))) {
				$o->closeSection();
				$o->openSection('Asset Specific Screens');
			} 

			// the dependant and format screens have some problems with the links displayed
			// therefore we dont display dependants and format screens keywords
			// until they are fixed
			if ($current_screen == 'formats') continue;

			if (strtolower($current_screen) != 'contents') {
			   $o->openSection($current_screen_name.' screen'.' <span class="screen_kw">(%'.str_replace(' ', '_', strtolower($current_screen)).'-%)</span>');
			} else {
				if (in_array($asset_type, Array('page_standard', 'calendar_event_multi_date'))) {
					$o->openSection($current_screen_name.' screen'.' <span class="screen_kw">(%__custom-'.str_replace(' ', '_', strtolower($current_screen)).'%)</span>');
					$o->closeSection();
					continue;
				} else {
					// list of asset types that should not get this limbo keyword
					$filter_list = Array('image');
					if (!in_array($asset_type, $filter_list)) {
						$o->openSection($current_screen_name.' screen'.' <span class="screen_kw">(%'.str_replace(' ', '_', strtolower($current_screen)).'-%)</span>');
					} else {
						continue;
					}
				}
			}

			$o->openField('', 'wide_col');
			if (count($section_array) > 0 ){
			?>
				<table class="sq-backend-table compact">
					<tr>
						<th style="width: 40%">Name</th>
						<th style="width: 60%">Keyword</th>
					</tr>
					<?php
					foreach ($section_array as $tmp_section => $tmp_fields) {
					?>
						<tr>
							<?php
							// if the section name is empty
							if (strlen($tmp_section) > 0) {
								echo '<th>'.$tmp_section.' Section</th>';
								echo '<th>%'.$current_screen.'-S_'.strtolower(str_replace(' ', '_', $tmp_section)).'%</th>';
							} else {
								echo '<th colspan="2">&nbsp;</th>';
							}
							?>
						</tr>
						<?php
						// display the fields of that section
						foreach ($tmp_fields as $tmp_field) {
						?>
							<tr>
								<td><?php echo ucwords(str_replace('_', ' ', $tmp_field)) ?></td>
								<td>%<?php echo $current_screen.'-F_'.$tmp_field ?>%</td>
							</tr>
						<?php
						}//end foreach
					}//end foreach			
				?>
				</table>
			<?php
			} else {
				echo '<em>No available section or field keywords.</em>';
			}
			$o->closeField();

			if (!strcmp(strtolower($current_screen_name), 'metadata')) {
				$o->openSection('Specific Metadata Keywords');
				$o->openField('', 'wide_col');
				?>
					<table class="sq-backend-table compact">
						<tr>
							<th style="width: 40%">Name</th>
							<th style="width: 60%">Keyword</th>
						</tr>
						<tr>
							<td>Print a specific metadata section name</td>
							<td>%metadata-F_section_{metadata section id}_name%</td>
						</tr>
						<tr>
							<td>Print a specific metadata section description</td>
							<td>%metadata-F_section_{metadata section id}_description%</td>
						</tr>
						<tr>
							<td>Print a specific metadata section interface</td>
							<td>%metadata-F_section_{metadata section id}_values%</td>
						</tr>
						<tr>
							<td>Print a specific metadata schema field</td>
							<td>%metadata-F_{metadata schema field id}%<br/>For example: <strong>%metadata-F_99999%</strong></td>
						</tr>
					</table>
				<?php
				$o->closeField();
				$o->closeSection();
			}

			$o->closeSection();
		}//end foreach

	}//end getSimpleEditKeywords()

	/**
	* Checkes the raw content within the layout to find the order in which keywords are placed
	* See bug#3698
	*
	* @param object	$layout		the layout of which keyword's order is evaluated
	*
	* @return array
	* @access private
	*/
	function _getLayoutKeywordsOrder($layout)
	{
		// Check which of screen or custom keywords are placed before in raw content
		// This should be FALSE instead of 0 because making it 0, we mean that it is actually found already
		$custom_pos = FALSE;
		$screen_pos = FALSE;

		// Defaut order 
		$layout_keywords_order = Array(
				0 => 'screen',
				1 => 'custom'
			);
		
		$containers = $layout->getContainers();

		foreach (array_keys($containers) as $key) {

			$container = $containers[$key];
			ob_start();
				$container->getEditFns()->paint($container, FALSE, FALSE);
				$raw_content = ob_get_contents();
			ob_end_clean();

			$custom_pos = strpos($raw_content, '%__custom');
			
			$screen_pos_metadata = strpos($raw_content, '%metadata');
			$screen_pos_lookups = strpos($raw_content, '%lookupvalues-');

			if ($screen_pos_metadata !== FALSE) $screen_pos = $screen_pos_metadata;

			if (($screen_pos_lookups !== FALSE) && ($screen_pos_metadata > $screen_pos_lookups)) $screen_pos = $screen_pos_lookups;

			// If any of the keywords are found then exit, as we are checking their first occurance only
			if ($custom_pos || $screen_pos) {
				break;
			}
		}

		// Check which of screen or custom keywords are placed before in raw content
		// if we just find a custom keyword and no screen keyword we should make sure we 
		// still place 'custom' before 'screen' in layout_keywords_order
		// Bug #4458 Conflicting WYSIWYG's
		if (($screen_pos > $custom_pos && $custom_pos !== FALSE) || ($custom_pos !== FALSE && $screen_pos === FALSE)) {
			$layout_keywords_order = Array(
					0 => 'custom',
				 	1 => 'screen'
				);
		}

		return $layout_keywords_order;

	}// end function

}//end class

?>
