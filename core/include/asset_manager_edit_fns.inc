<?php
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Manager_Edit_Fns extends MySource_Object
{

	/**
	* Holds a reference to the asset manager
	* @var object Asset_Manager
	*/
	var $am;

	/**
	* Holds a reference to the backend object
	* @var object Backend
	*/
	var $backend;

	/**
	* Constructor
	*
	* @param object Asset_Manager	&$am
	* @param object Backend			&$backend
	*/
	function Asset_Manager_Edit_Fns(&$am, &$backend)
	{
		$this->MySource_Object();
		$this->am = &$am;
		$this->backend = &$backend;
	}

	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @access public
	*/
	function paintBackend()
	{
		$o = &$this->backend->out;

		if (!isset($_GET['am_section'])) {
			if (isset($_GET['sq_from_frontend'])) $_GET['am_section'] = 'edit_asset';
			else $_GET['am_section'] = '';
		}

		$o->addFormActionGetVar('am_section', $_GET['am_section']);
		$o->addHiddenField('am_form_submitted', '1');

		switch($_GET['am_section']) {
			case 'add_asset' :
				$parent = &$this->am->getAsset($_GET['parent_assetid']);
				if (is_null($parent)) {
					trigger_error('Parent Asset #'.$_GET['parent_assetid'].' not found', E_USER_WARNING);
					break;
				}

				// make sure that we are allowed to link this type to the parent
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $_GET['type_code'], $_GET['link_type'])) !== true) {
					trigger_error($err_msg, E_USER_WARNING);
					break;
				}

				$o->addFormActionGetVar('parent_assetid', $_GET['parent_assetid']);
				$o->addFormActionGetVar('pos',            $_GET['pos']);
				$o->addFormActionGetVar('type_code',      $_GET['type_code']);
				$o->addFormActionGetVar('link_type',      $_GET['link_type']);

				$this->am->includeAsset($_GET['type_code']);
				$asset = new $_GET['type_code']();

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				if (!empty($_POST['am_form_submitted'])) {

					// start the transaction to create the asset and initial link
					$link = Array('asset'		=> &$parent,
								  'link_type'	=> $_GET['link_type'],
								  'value'		=> '',
								  'sort_order'	=> $_GET['pos']);

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$success = $asset->processBackend($this->backend->out, $link);

					if ($success) {
						$o->addMessage(SQ_BO_MSG_NOTICE, 'New '.$this->am->getTypeInfo($_GET['type_code'], 'name').' "'.$asset->name.'" created');
						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" was successfully linked to "'.$parent->name.'"');
						$this->am->rememberAsset($asset);
						$o->addFormActionGetVar('am_section', 'edit_asset');
						$o->addFormActionGetVar('assetid', $asset->id);
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

					} else {
						// the asset was not created fully or linking failed
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
				}

				$asset->paintBackend($this->backend->out);

				break;

			case 'edit_asset' :

				$o->addFormActionGetVar('assetid', $_GET['assetid']);

				$asset = &$this->am->getAsset($_GET['assetid']);
				if (is_null($asset)) {
					trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
					break;
				}

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				$link = Array();
				if (!empty($_POST['am_form_submitted'])) {

					$do_process_backend = true;

					// try to acquire a lock on this asset?
					if (isset($_POST['sq_acquire_lock'])) {

						//// START HIPO PROCESSING ////
			
						$vars = Array('assetid' => $asset->id, 'forceably_acquire' => (bool) $_POST['sq_acquire_by_force']);
						$complete_url = $_SERVER['REQUEST_URI'];

						require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_acquire_lock.inc';
						$init_hipo = new HIPO_Job_Acquire_Lock();

						$init_hipo->setRunningVars($vars);
						$init_hipo->setOption('on_complete_url', $complete_url);

						$code_name = $init_hipo->initialise();
						if ($code_name) {
							$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
							if ($url = $hh->getProcessURL($code_name)) $o->setRedirect($url);
						}
						
						////  END HIPO PROCESSING  ////

						$do_process_backend = false;
					}

					// try to release a lock on this asset?
					if (isset($_POST['sq_manual_release_lock'])) {
						$this->am->releaseLock($asset->id);
						$do_process_backend = false;
					}

					if ($do_process_backend && $asset->processBackend($this->backend->out, $link)) {

						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" Updated');

						$lock = $this->am->getLockInfo($asset->id);
						if (!empty($lock)) {
							// this asset is currently locked
							$user = &$this->am->getAsset($lock['userid']);
							if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
								if (false && isset($_POST['sq_release_lock']) && (int) $_POST['sq_release_lock'] == 1) {
									// committing changes and releasing the lock
									if (!$this->am->releaseLock($asset->id)) {
										trigger_error('Failed to release lock on "'.$asset->name.'"', E_USER_WARNING);
									}
								} else {
									// not releasing the lock, so re-acquire
									if (!$this->am->acquireLock($asset->id)) {
										trigger_error('Failed to re-acquire lock on "'.$asset->name.'"', E_USER_WARNING);
									}
								}
							}
						}
					}// endif
				}

				$asset->paintBackend($this->backend->out);
				break;

			case 'duplicate' :
				$this->_dupeAsset();
				break;

			case 'delete_asset' :
				$this->_deleteAsset();
				break;

			case 'link_asset' :
				$this->_linkAsset();
				break;

			case 'move_asset' :
				$this->_moveAsset();
				break;

			case 'forceably_acquire_lock' :
				$this->_forceablyAcquireLock();
				break;

			default   :
				$this->_printAssetTypeList();

		}//end switch

	}//end paintBackend();


	/**
	* Looks after the duplication of assets and their trees, includes the interface
	*
	* @access private
	*/
	function _dupeAsset()
	{
		$o = &$this->backend->out;

		$o->addFormActionGetVar('assetid', $_GET['assetid']);
		$asset = &$this->am->getAsset($_GET['assetid']);
		if (is_null($asset)) {
			trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
			break;
		}

		require_once SQ_LIB_PATH.'/progress_bar/progress_bar.inc';
		$pb = new Progress_Bar($o->getCurrentLocation(), $this);
		#$pb->testing |= 1; // can resize pop-up
		#$pb->testing |= 2; // confirm reload
		#$pb->testing |= 4; // manual reload
		if (!$pb->loaded) {
			$pb->setFns('_dupeInit', '_dupeInitPaint');
			$pb->setOption('auto_close', false);

			$pb->addStep('Acquire Locks', 
							'_dupeStepAcquireLocks', 
							'_dupeStepAcquireLocksInit', 
							'_dupeStepAcquireLocksDone');
			$pb->addStep('Duplicate', 
							'_dupeStepDuplicate', 
							'_dupeStepDuplicateInit', 
							'_dupeStepDuplicateDone');
			$pb->addStep('Remap Ids', 
							'_dupeStepRemap', 
							'_dupeStepRemapInit', 
							'_dupeStepRemapDone');
			$pb->addStep('Release Locks', 
							'_dupeStepReleaseLocks', 
							'_dupeStepReleaseLocksInit', 
							'_dupeStepReleaseLocksDone');

		}

		$pb->run();

		if ($pb->inPopUp()) {
			$o->openRaw();
		} else {
			$o->setHeading('Duplicate : '.$asset->name);
			$o->openSection('Options');
			$o->openField('', 'new_line');
		}

		$pb->paint();

		if ($pb->inPopUp()) {
			$o->closeRaw();
		} else {
			$o->closeSection();
			$o->commitButton();
		}

	}// end _dupeAsset()


	/**
	* Called by the Progress Bar to initialise the duplication process
	* returns true if ready to start
	*
	* @access private
	* @return boolean
	*/
	function _dupeInit(&$pb)
	{
		$vars = $pb->getRunningVars();
		$initialised = false;

		// first time here ?
		if (empty($vars)) {
			$asset = &$this->am->getAsset($_GET['assetid']);
			if (!is_null($asset)) {
				$vars = Array(	'assetid'           => $_GET['assetid'],
								'type_code'         => $asset->type(),
								'to_parent_assetid' => $_GET['to_parent_assetid'],
								'to_parent_pos'     => $_GET['to_parent_pos']
							);
			}// end if


		// they have posted what they want to duplicate ?
		} else if (!empty($_POST['duplicate_assetid'])) {
			$asset = &$this->am->getAsset($vars['assetid'], $vars['type_code']);
			$tree = $this->getAllChildLinks($vars['assetid']);

			$effective_duplicate_treeids = Array('' => Array());
			$indexed_effective_duplicate_treeids = Array();
			if (!empty($_POST['duplicate_treeids'])) {

				// make sure they are all in the proper order
				ksort($_POST['duplicate_treeids'], SORT_STRING);
				$duplicate_treeids = Array();
				for(reset($_POST['duplicate_treeids']); null !== ($treeid = key($_POST['duplicate_treeids'])); next($_POST['duplicate_treeids'])) {
					$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
					// if this is a top level link or if this links parent link is selected
					// then we can add it to the proper array
					// this strips out any selections whose parents where selected
					// and any dependent links (in hidden fields) whose parent wasn't selected
					if ($parent_treeid == '' || isset($duplicate_treeids[$parent_treeid])) {
						$duplicate_treeids[$treeid] = $_POST['duplicate_treeids'][$treeid];

						$dependant_treeid = $tree[$treeid]['dependant_treeid'];
						if(!isset($effective_duplicate_treeids[$dependant_treeid])) $effective_duplicate_treeids[$dependant_treeid] = Array();
						$effective_duplicate_treeids[$dependant_treeid][] = $treeid;
					}// end if

				}// end for

				ksort($duplicate_treeids, SORT_STRING);
				ksort($effective_duplicate_treeids, SORT_STRING);

				$indexed_effective_duplicate_treeids = Array();

				for(reset($effective_duplicate_treeids); null !== ($treeid = key($effective_duplicate_treeids)); next($effective_duplicate_treeids)) {
					$indexed_effective_duplicate_treeids[] = Array('treeid' => $treeid,
																	'dependants' => $effective_duplicate_treeids[$treeid]
																	);
				}// end for

			}// end if

			// create a unique list of assets that we will be aquiring locks/duplicating
			$map = Array();
			foreach($indexed_effective_duplicate_treeids as $data) {

				// if the treeid is blank, we are using the initating asset
				if ($data['treeid'] == '') {
					$assetid	= $vars['assetid'];
					$type_code	= $vars['type_code'];

				// else we are locking a sub asset
				} else {
					$assetid	= $tree[$data['treeid']]['assetid'];
					$type_code	= $tree[$data['treeid']]['type_code'];

				}// end if

				if (!isset($map[$assetid])) $map[$assetid] = $type_code;

			}// end foreach

			// create a unique list of assets to acquire locks for
			$vars['assets'] = Array();
			foreach($map as $assetid => $type_code) {
				$vars['assets'][] = Array('assetid' => $assetid, 'type_code' => $type_code);
			}// end foreach

/*
//			? ><table border="1"><tr><td valign="top">< ?php
//			pre_echo($_POST['duplicate_treeids']);
//			pre_echo($duplicate_treeids);
//			? ></td><td valign="top">< ?php
//			pre_echo($effective_duplicate_treeids);
//			? ></td><td valign="top">< ?php
//			pre_echo($tree);
//			? ></td><td valign="top">< ?php
//			pre_echo($indexed_effective_duplicate_treeids);
//			? ></td></tr></table>< ?php
*/
			// OK, what we have with the $effective_duplicate_treeids array is access to all the assets that
			// we are going to duplicate to have to duplicate, with all the assets that they will create
			// when duplicate is called (because it also duplicates any dependantly linked assets)

			pre_echo("Call duplicate() on ".count($effective_duplicate_treeids)." assets, which will duplicate ".(count($duplicate_treeids) + 1)." assets");

			$vars['tree'] = $tree;
			$vars['effective_duplicate_treeids'] = $indexed_effective_duplicate_treeids;

			$initialised = true;

		}// end if

		$pb->setRunningVars($vars);

		return $initialised;

	}// end _dupeInit()


	/**
	* Called by the Progress Bar to if _dupeInit() returns false
	* Used to select the assets to duplicate
	*
	* @access private
	*/
	function _dupeInitPaint(&$pb)
	{
		$o = &$this->backend->out;
		$vars = $pb->getRunningVars();
		if (empty($vars)) return;

		$asset = &$this->am->getAsset($vars['assetid'], $vars['type_code']);
		$tree = $this->getAllChildLinks($asset->id);

		if (!empty($tree)) {
			?><div style="text-align: right"><?php
			normal_button('select_all', 'Select All', 'duplicate_treeids_check(\'\', true);');
			normal_button('unselect_all', 'Unselect All', 'duplicate_treeids_check(\'\', false);');
			?></div><?php
		}// end if
		?>
		<table border="0" cellspacing="0" cellpadding="0">
			<tr>
				<td class="sq-backend-data">
					<?php check_box('duplicate_assetid', $asset->id, true, 'alert("This is a asset that you are duplicating, you cannot unset it"); this.checked = true;');?>
				</td>
				<td class="sq-backend-data" style="font-weight: bold;">
					<?php echo $asset->short_name, ' - Id #', $asset->id; ?>
				</td>
			</tr>
		</table>
		<?php

		if (!empty($tree)) {

			?>
			<script language="javascript" type="text/javascript">
				<!-- //

					var DUPLICATE_TREEIDS = ["<?php echo implode('" ,"', array_keys($tree));?>"];

					var DUPLICATE_TREEIDS_CHECK_RUNNING = false;
					function duplicate_treeids_check(treeid, checked) {
						if (DUPLICATE_TREEIDS_CHECK_RUNNING) return;
						DUPLICATE_TREEIDS_CHECK_RUNNING = true;

						var tmp_treeid = new String(treeid);

						// if we were checked make sure that all our parents are checked
						if (checked) {
							while (tmp_treeid.length > 0) {
								var elem = get_form_element("duplicate_treeids[" + tmp_treeid + "]");
								// this may be a dependant link , which is a hidden field
								if (elem.type != 'checkbox') continue;
								elem.checked = true;
								tmp_treeid = tmp_treeid.substr(0, tmp_treeid.length - <?php echo SQ_CONF_ASSET_TREE_SIZE; ?>);
							}
						}
						// Change all our children to be what we are
						for(var i = 0; i < DUPLICATE_TREEIDS.length; i++) {
							if (treeid == DUPLICATE_TREEIDS[i].substr(0, treeid.length)) {
								var elem = get_form_element("duplicate_treeids[" + DUPLICATE_TREEIDS[i] + "]");
								// this may be a dependant link , which is a hidden field
								if (elem.type != 'checkbox') continue;
								elem.checked = checked;
							}
						}

						DUPLICATE_TREEIDS_CHECK_RUNNING = false;
					}// end duplicate_treeids_check()

				// -->
			</script>
			<?php

			$level = 1;
			$indents = Array();
			$dependants = Array();
			$branch_img = $o->filesPath('/images/tree/branch.gif');
			$indent_start = '<td class="sq-backend-data" %';
			$indent_end   = '%><img src="'.$o->filesPath('/images/blank.gif').'" width="20" height="20" border="0" alt="blank" /></td>';

			$last_at_level = Array();
			ob_start();
			for(reset($tree); NULL !== ($treeid = key($tree)); next($tree)) {
				$prev_level = $level;
				$level      = (int) $tree[$treeid]['level'];

				$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
				$last_at_level[$parent_treeid] = $treeid;

				if ($level > $prev_level) {
					array_push($indents, $parent_treeid);
				} elseif ($level < $prev_level) {
					for($x = $prev_level; $x > $level; $x--) {
						array_pop($indents);
					}
				}

			?>
			<table border="0" cellspacing="0" cellpadding="0">
				<tr>
			<?php

				if (!empty($indents)) {
					echo $indent_start, implode($indent_end.$indent_start, $indents), $indent_end;
				}

				?>
					<td class="sq-backend-data" <?php echo '%', $treeid, '%'; ?>><img src="<?php echo $branch_img; ?>" width="20" height="20" border="0" alt="branch" /></td>
					<td class="sq-backend-data">
					<?php
						if ($tree[$treeid]['dependant']) {
							echo '&nbsp;';
							hidden_field('duplicate_treeids['.$treeid.']', $tree[$treeid]['linkid']);
						} else {
							// have we submitted this before ? if so and they checked, select it again for them
							check_box('duplicate_treeids['.$treeid.']', $tree[$treeid]['linkid'], !empty($_POST['duplicate_treeids'][$treeid]), 'duplicate_treeids_check("'.$treeid.'", this.checked);');
						}
					?>
					</td>
					<td class="sq-backend-data">
					<?php
						if (!$tree[$treeid]['dependant']) echo '<b>';
						echo $tree[$treeid]['short_name'], ' - Id #', $tree[$treeid]['assetid'];
						if (!$tree[$treeid]['dependant']) echo '</b>';
					?>
					</td>
				</tr>
				</table>
			<?php
			}// end for
			$tree_html = ob_get_contents();
			ob_end_clean();

			$bg_stalks_search  = Array();
			$bg_stalks_replace = Array();
			$bg_code = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
			for(reset($tree); NULL !== ($treeid = key($tree)); next($tree)) {
				$bg_stalks_search[] = '%'.$treeid.'%';
				// if this treeid was the last on it's level it doesn't have a stalk bg
				$bg_stalks_replace[] = (in_array($treeid, $last_at_level, true)) ? '' : $bg_code;
			}// end for

			echo str_replace($bg_stalks_search, $bg_stalks_replace, $tree_html);

		} // end if noto empty tree

	}// end _dupeInitPaint()


	/**
	* Called before _dupeStepAcquireLocks() to set-up some initial things
	*
	* @access private
	*/
	function _dupeStepAcquireLocksInit(&$pb)
	{
		$pb->setStatusMsg('Starting to Acquire Locks');

		$vars = $pb->getRunningVars();
		$vars['lock']['flag'] = 'assets';
		$vars['lock']['pos']  = 0;
		$vars['lock']['acquired_assets'] = Array();
		$vars['lock']['missed_assets']  = Array();

		$pb->setRunningVars($vars);

	}// end _dupeStepAcquireLocksInit()


	/**
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the aquiring of assets to be duped
	*
	* @access private
	*/
	function _dupeStepAcquireLocks(&$pb)
	{
		$vars = $pb->getRunningVars();
#speed_check();

		$status_msg = '';

		$attempted_assets = Array();
		// Get the next four locks
		$i = 1;
		while ($i <= 4 && count($vars['lock']['acquired_assets']) < count($vars['assets'])) {
			$i++;

			switch ($vars['lock']['flag']) {
				// are we still reading from the lock map ?
				case 'assets' :
					$assetid   = $vars['assets'][$vars['lock']['pos']]['assetid'];
					$type_code = $vars['assets'][$vars['lock']['pos']]['type_code'];
					break;

				// are we now fetching missed assets ? 
				case 'missed' :
					$assetid   = $vars['lock']['missed_assets'][$vars['lock']['pos']]['assetid'];
					$type_code = $vars['lock']['missed_assets'][$vars['lock']['pos']]['type_code'];
					break;

				// are we still reading from the lock map ?
				default :
					$pb->abort();
					trigger_error('Flag "'.$vars['lock']['flag'].'" not known', E_USER_ERROR);
					break;

			}// end switch

			if (in_array($assetid, $attempted_assets)) continue;

	#speed_check("three");
	#		pre_echo("Lock ASSET : $assetid");
			$asset = &$this->am->getAsset($assetid, $type_code);
			if (is_null($asset)) {
				trigger_error('Unable to find asset to lock, Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if
	#		pre_echo("Lock ASSET NAME : ".$asset->name);
	#speed_check("four");

			$attempted_assets[] = $assetid;

			// attempt to acquire the lock for this asset
			if (@$this->am->acquireLock($asset->id, $vars['assetid'])) {
				$vars['lock']['acquired_assets'][] = $asset->id;
				$status_msg .= 'Lock Acquired for '.$asset->name."\n";
	#speed_check("five");
			} else {
				$vars['lock']['missed_assets'][] = Array('assetid' => $assetid, 'type_code' => $type_code);
				$status_msg .= 'Unable to Acquire Lock for '.$asset->name.', will try again at end'."\n";
			}

			switch ($vars['lock']['flag']) {
				// are we still reading from the lock map ?
				case 'assets' :
					$vars['lock']['pos']++;
					if ($vars['lock']['pos'] >= count($vars['assets'])) {
						$vars['lock']['pos']  = 0;
						$vars['lock']['flag'] = 'missed';
					}
					break;

				// are we now fetching missed assets ? 
				case 'missed' :
					// if we acquired the last missed asset
					if ($acquired) {
						// remove it from the missed list
						// no need to adjust the pos because the elements will shift down
						array_splice($vars['lock']['missed_assets'], $vars['lock']['pos'], 1);

					} else {
						$vars['lock']['pos']++;
						if ($vars['lock']['pos'] >= count($vars['lock']['missed_assets'])) {
							$vars['lock']['pos']  = 0;
						}
					}
					break;

				// are we still reading from the lock map ?
				default :
					$pb->abort();
					trigger_error('Flag "'.$vars['lock']['flag'].'" not known', E_USER_ERROR);
					break;

			}// end switch

		}// end while

		$pb->setStatusMsg($status_msg);
		$pb->setPercent(count($vars['lock']['acquired_assets']) / count($vars['assets']));

speed_check('FINISHED');

		$this->am->updateLock($vars['assetid']);

		$pb->setRunningVars($vars);


	}// end _dupeStepAcquireLocks()


	/**
	* Called after _dupeStepAcquireLocks() above has set the percent to 100
	* Just clears the status msg
	*
	* @access private
	*/
	function _dupeStepAcquireLocksDone(&$pb)
	{
		$pb->setStatusMsg('');

		$vars = $pb->getRunningVars();
		unset($vars['lock']);
		$pb->setRunningVars($vars);

	}// end _dupeStepAcquireLocksDone()


	/**
	* Called before _dupeStepDuplicate() to set-up some initial things
	*
	* @access private
	*/
	function _dupeStepDuplicateInit(&$pb)
	{
		$pb->setStatusMsg('Starting Duplication');

		$vars = $pb->getRunningVars();
		$vars['dupe'] = Array();
		$vars['dupe']['pos'] = 0;
		$vars['dupe']['asset_map'] = Array();
		$vars['dupe']['link_map']  = Array();

		$pb->setRunningVars($vars);

	}// end _dupeStepDuplicateInit()


	/**
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the actual duplication of the assets
	*
	* @access private
	*/
	function _dupeStepDuplicate(&$pb)
	{
		$vars = $pb->getRunningVars();
#pre_echo($vars['dupe']);

#speed_check();
#		pre_echo("DUPE POS : ".$vars['dupe']['pos']);
		$treeid = $vars['effective_duplicate_treeids'][$vars['dupe']['pos']]['treeid'];
		$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);

#		pre_echo("TREE ID : ".$treeid);
		// if there is a blank treeid then we must be duplicating the initialising asset
		$link    = Array('asset' => null, 'link_type' => 0, 'sort_order' => -1, 'value' => '');
		$assetid   = 0;
		$type_code = '';
		$old_linkid = 0;
		// if the treeid is blank we are duping the initating asset
		if ($treeid == '') {
			$assetid			= $vars['assetid'];
			$type_code			= $vars['type_code'];
			$link['link_type']	= SQ_LINK_TYPE_1;
			$link['sort_order']	= $vars['to_parent_pos'];
			$link['asset']		= &$this->am->getAsset($vars['to_parent_assetid']);

		// else we are duping a sub asset
		} else {
			$assetid	= $vars['tree'][$treeid]['assetid'];
			$type_code	= $vars['tree'][$treeid]['type_code'];
			if ($parent_treeid == '') {
				$parent_assetid		= $vars['assetid'];
				$parent_type_code	= $vars['type_code'];
			} else {
				$parent_assetid		= $vars['tree'][$parent_treeid]['assetid'];
				$parent_type_code	= $vars['tree'][$parent_treeid]['type_code'];
			}
			// now get the assetid for the duped parent asset, not the original
			if (isset($vars['dupe']['asset_map'][$parent_assetid])) {
				$link['asset']	= &$this->am->getAsset($vars['dupe']['asset_map'][$parent_assetid], $parent_type_code);
			}

			$link['link_type']	= $vars['tree'][$treeid]['link_type'];
			$link['sort_order']	= $vars['tree'][$treeid]['sort_order'];
			$link['value']		= $vars['tree'][$treeid]['value'];

			$old_linkid = $vars['tree'][$treeid]['linkid'];

		}// end if

#speed_check("two");
		if (is_null($link['asset'])) {
			trigger_error('Unable to find parent asset for Asset Id #'.$assetid, E_USER_WARNING);
			$pb->abort();
			return;
		}

		// If this asset has already been duplicated,
		// then just link it to it's new parent
		if (isset($vars['dupe']['asset_map'][$assetid])) {

			$dupe = &$this->am->getAsset($vars['dupe']['asset_map'][$assetid], $type_code);
			if (is_null($dupe)) {
				trigger_error('Unable to find duplicated asset for Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if

			$pb->setStatusMsg('Linking '.$dupe->name);

			// if we have already created this link we don't need to do it again
			if (empty($old_linkid) || !in_array($old_linkid, $vars['dupe']['link_map'])) {
#			pre_echo('Create Link');
				$linkid = $link['asset']->createLink($dupe, $link['link_type'], $link['value'], $link['sort_order']);
				if (!$linkid) {
					$pb->abort();
					return;
				}// end if
			}// end if

		// otherwise duplicate and link
		} else {

#speed_check("three");
#			pre_echo("DUPLICATE ASSET : $assetid");
			$orig = &$this->am->getAsset($assetid, $type_code);
			if (is_null($orig)) {
				trigger_error('Unable to find original asset, Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if
			$pb->setStatusMsg('Duplicating '.$orig->name);
#speed_check("four");
			$dupe = &$orig->duplicate($link, $vars['dupe']['asset_map'], true, true);
#speed_check("five");
			if (is_null($dupe)) {
				trigger_error('Duplicate failed for Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if

#			pre_echo("DUPLICATED ASSETID : ".$dupe->id);

		}// end if

		if ($old_linkid) $vars['dupe']['link_map'][] = $old_linkid;

		$pb->setPercent(($vars['dupe']['pos'] + 1) / count($vars['effective_duplicate_treeids']));

		$vars['dupe']['pos']++;
#speed_check('FINISHED');

		$this->am->updateLock($vars['assetid']);

		$pb->setRunningVars($vars);

	}// end _dupeStepDuplicate()


	/**
	* Called after _dupeStepDuplicate() above has set the percent to 100
	* Just clears the status msg
	*
	* @access private
	*/
	function _dupeStepDuplicateDone(&$pb)
	{
		$pb->setStatusMsg('');
	}// end _dupeStepDuplicateDone()


	/**
	* Called before _dupeStepRemap() to set-up some initial things
	*
	* @access private
	*/
	function _dupeStepRemapInit(&$pb)
	{
		$pb->setStatusMsg('Starting Remap');

		$vars = $pb->getRunningVars();
		$vars['remap'] = Array();
		$vars['remap']['pos'] = 0;
		$pb->setRunningVars($vars);

	}// end _dupeStepRemapInit()


	/**
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the remapping of assetids from their original values to their new values
	*
	* @access private
	*/
	function _dupeStepRemap(&$pb)
	{
		$vars = $pb->getRunningVars();

		$assetid   = $vars['assets'][$vars['remap']['pos']]['assetid'];
		$type_code = $vars['assets'][$vars['remap']['pos']]['type_code'];

		$duped_assetid = $vars['dupe']['asset_map'][$assetid];

		$asset = &$this->am->getAsset($duped_assetid, $type_code);
		if (is_null($asset)) {
			trigger_error('Unable to find duplicated asset, Asset Id #'.$duped_assetid, E_USER_WARNING);
			$pb->abort();
			return;
		}// end if

		$pb->setStatusMsg('Remaping '.$asset->name);

		if (!$asset->remapAssetids($vars['dupe']['asset_map'])) {
			trigger_error('Remap failed for asset, Asset Id #'.$asset->id, E_USER_WARNING);
			$pb->abort();
			return;
		}// endif

		$vars['remap']['pos']++;

		$pb->setPercent($vars['remap']['pos'] / count($vars['assets']));
		$pb->setRunningVars($vars);

		$this->am->updateLock($vars['assetid']);

	}// end _dupeStepRemap()


	/**
	* Called after _dupeStepRemap() above has set the percent to 100
	* Just clears the status msg
	*
	* @access private
	*/
	function _dupeStepRemapDone(&$pb)
	{
		$pb->setStatusMsg('');
	}// end _dupeStepRemapDone()


	/**
	* Called before _dupeStepReleaseLocks() below
	*
	* @access private
	*/
	function _dupeStepReleaseLocksInit(&$pb)
	{
		$pb->setStatusMsg('Releasing All Locks');
	}// end _dupeStepReleaseLocksDone()


	/**
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the aquiring of assets to be duped
	*
	* @access private
	*/
	function _dupeStepReleaseLocks(&$pb)
	{
		$vars = $pb->getRunningVars();
		$this->am->releaseLock($vars['assetid']);
		$pb->setStatusMsg('All Locks Released');
		$pb->setPercent(1);

	}// end _dupeStepReleaseLocks()


	/**
	* Called after _dupeStepReleaseLocks() above has set the percent to 100
	* Just clears the status msg
	*
	* @access private
	*/
	function _dupeStepReleaseLocksDone(&$pb)
	{
		$pb->setStatusMsg('');
	}// end _dupeStepReleaseLocksDone()


	/**
	* Looks after the deletion of assets and their trees, includes the interface
	*
	* @access private
	*/
	function _deleteAsset()
	{
		$o = &$this->backend->out;

		if (isset($_GET['delete_linkid'])) {
			// they have confirmed their intention to delete this asset
			$asset = &$this->am->getAsset($_GET['delete_assetid']);
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($_GET['delete_linkid'], $asset->id, 'minor');
			$parent = &$this->am->getAsset($link['majorid'], $link['major_type_code']);
			$this->am->acquireLock($parent->id);
			$parent->deleteLink($link['linkid']);
			$this->am->releaseLock($parent->id);
			$trash = &$this->am->getSystemAsset('trash_folder');

			// action is done, close the window and refresh the asset map
			require_once 'XML/Tree.php';
			$xml = new XML_Tree();
			$xml_root = &$xml->addRoot('assets');
			$xml_root->addChild('asset', '', Array('assetid' => $parent->id));
			$xml_root->addChild('asset', '', Array('assetid' => $trash->id));
			?>
			<script language="Javascript" type="text/javascript">
				if (window.opener.top.sidenav && window.opener.top.sidenav.reload_asset) {
					window.opener.top.sidenav.reload_assets("<?php echo str_replace("\n", "\\n", addslashes($xml->get())); ?>");
				}
				window.close();
			</script>
			<?php
		} else {
			$o->addFormActionGetVar('delete_assetid', $_GET['assetid']);
			$o->addFormActionGetVar('delete_linkid', $_GET['linkid']);
			$asset = &$this->am->getAsset($_GET['assetid']);
			if (is_null($asset)) {
				trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
				break;
			}

			$o->openSection('Note');
			$o->openField('&nbsp;');
			echo 'You are deleting this asset to the trash. Below are a list of assets that will be affected by deleting this asset';
			$ei = &$asset->getEI();
			$ei->paintLinking($asset, $o, false);

			$o->closeSection();

			$o->openField('', 'commit');
			normal_button('cancel', 'Cancel', 'window.close()');
			echo '&nbsp;';
			submit_button('commit', 'Commit');
		}

	}// end _deleteAsset()


	/**
	* Ask a user if they want to cascade permission when moving an asset
	*
	* @return void
	* @access private
	*/
	function _moveAsset()
	{
		if (isset($_REQUEST['link_completed']) && $_REQUEST['link_completed']) {
			// close the window and refresh the asset map
			require_once 'XML/Tree.php';
			$xml = new XML_Tree();
			$xml_root = &$xml->addRoot('assets');
			$xml_root->addChild('asset', '', Array('assetid' => $_GET['link_parentid']));
			$xml_root->addChild('asset', '', Array('assetid' => $_GET['link_assetid']));
			$xml_root->addChild('asset', '', Array('assetid' => $_GET['link_old_parentid']));
			?>
			<script language="Javascript" type="text/javascript">
				if (window.opener.top.sidenav && window.opener.top.sidenav.reload_asset) {
					window.opener.top.sidenav.reload_assets("<?php echo str_replace("\n", "\\n", addslashes($xml->get())); ?>");
				}
				window.close();
			</script>
			<?php

		} else if (!isset($_GET['link_assetid'])) {
			$o = &$this->backend->out;
			$o->addFormActionGetVar('link_old_parentid', $_GET['old_parentid']);
		}

		return $this->_linkAsset();

	}// end _moveAsset()


	/**
	* Ask a user if they want to cascade permission when creating a new link
	*
	* @return void
	* @access private
	*/
	function _linkAsset()
	{
		$o = &$this->backend->out;

		if (isset($_REQUEST['link_completed']) && $_REQUEST['link_completed']) {

			// close the window and refresh the asset map
			require_once 'XML/Tree.php';
			$xml = new XML_Tree();
			$xml_root = &$xml->addRoot('assets');
			$xml_root->addChild('asset', '', Array('assetid' => $_GET['link_parentid']));
			$xml_root->addChild('asset', '', Array('assetid' => $_GET['link_assetid']));
			?>
			<script language="Javascript" type="text/javascript">
				if (window.opener.top.sidenav && window.opener.top.sidenav.reload_asset) {
					window.opener.top.sidenav.reload_assets("<?php echo str_replace("\n", "\\n", addslashes($xml->get())); ?>");
				}
				window.close();
			</script>
			<?php

		} else if (isset($_GET['link_assetid'])) {

			// they have confirmed their intention to move this asset
			$permission_vars = Array();
			foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
				$set_perms = $this->am->getPermission($_GET['link_parentid'], $perm, null, false, false, true);
				foreach ($set_perms as $userid => $access) {
					$permission_vars[$perm][] = Array('access'          => $access,
													  'userid'          => $userid,
													  'previous_access' => null,
													  );
				}
			}

			//// START HIPO PROCESSING ////
			
			$vars = Array('assetid' => $_GET['link_assetid'], 'permission_changes' => $permission_vars);
			$complete_url = $_SERVER['REQUEST_URI'].'&link_completed=1';

			require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_edit_permissions.inc';
			$init_hipo = new HIPO_Job_Edit_Permissions();

			$init_hipo->setRunningVars($vars);
			$init_hipo->setOption('on_complete_url', $complete_url);

			$code_name = $init_hipo->initialise();
			if ($code_name) {
				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				if ($url = $hh->getProcessURL($code_name)) $o->setRedirect($url);
			}
			
			////  END HIPO PROCESSING  ////

			
		} else {
			$o->addFormActionGetVar('link_assetid',  $_GET['assetid']);
			$o->addFormActionGetVar('link_parentid', $_GET['parentid']);

			$asset = &$this->am->getAsset($_GET['assetid']);
			if (is_null($asset)) {
				trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
				return;
			}

			$parent = &$this->am->getAsset($_GET['parentid']);
			if (is_null($asset)) {
				trigger_error('Asset #'.$_GET['parentid'].' not found', E_USER_WARNING);
				return;
			}

			// ask if they want to cascade permission
			$o->openSection('Note');
				$o->openField('&nbsp;');
					hidden_field('link_completed', '0');
					?>
					Press the <b><i>Cascade</i></b> button below if you want to cascade the permissions of "<?php echo $parent->name; ?>" down to "<?php echo $asset->name; ?>" and its children. To skip the cascading of permissions, press the <b><i>Skip</i></b> button below.<br/><br/>
					The permission of the new parent "<?php echo $parent->name; ?>" are shown below. These permissions will attempt to be set on "<?php echo $asset->name; ?>" and its children if you choose to cascade the permissions.<br/><br/>
					<b>Note:</b> Any assets that become children of "<?php echo $asset->name; ?>" during the cascade process will not have the permission changes applied to them - only the <i>current</i> children.
					<?php
					$ei = &$parent->getEI();
					$ei->paintPermissions($parent, $o, false);
				$o->closeField();
			$o->closeSection();

			$o->openField('', 'commit');
			normal_button('cancel', 'Skip', 'this.form.link_completed.value = \'1\'; this.form.submit()');
			echo '&nbsp;';
			submit_button('commit', 'Cascade');

		}

	}// end _linkAsset()


	/**
	* Looks after the warning interface for forceably acquire locks
	*
	* @access private
	*/
	function _forceablyAcquireLock()
	{
		$o = &$this->backend->out;

		if (isset($_GET['forceably_acquire_confirm'])) {
			// they have confirmed their intention to forceably acquire the lock on this asset
			$asset = &$this->am->getAsset($_GET['assetid']);

			// check that they entered the correct security key
			if (!validate_security_key()) {
				// the security key entered is wrong
				$o->openSection('Security Key Incorrect');
				$o->openField('&nbsp;');
				echo '<b><i>The security key entered was incorrect. The lock on "'.$asset->name.'" has not been forceably acquired. You may try again using the new security key displayed below.</i></b>';
				$o->closeSection();
			} else {
				$o->addOnLoad('window.opener.top.main.main_form.sq_acquire_by_force.value = \'1\'; window.opener.top.main.main_form.sq_acquire_lock.value = \'1\'; window.opener.top.main.sqSubmitEditForm(); window.close();');
				return;
			}
		}
		
		$o->addFormActionGetVar('assetid', $_GET['assetid']);
		$o->addFormActionGetVar('forceably_acquire_confirm', 1);
		$asset = &$this->am->getAsset($_GET['assetid']);
		if (is_null($asset)) {
			trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
			break;
		}

		$lock = $this->am->getLockInfo($asset->id, true, false);
		if (empty($lock)) {
			$o->addOnLoad("alert('No lock is held on \"$asset->name\"'); window.close();");
			return;
		}
		$user = &$this->am->getAsset($lock['userid']);
		$editing = &$this->am->getAsset($lock['source_asset']);

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$expires_in = easy_time_total(($lock['expires'] - time()), true);
		if (!$expires_in) $expires_in = '1 second';

		$o->openSection('Note');
		$o->openField('&nbsp;');
		echo 'You are about to forceably acquire the lock on "'.$asset->name.'" currently held by "'.$user->name.'" at "'.$editing->name.'" and due to expire in '.$expires_in.'. <b>Any changes being made to "'.$asset->name.'" or any other asset in the locking chain (<i>see full list below</i>) will be lost.</b>';
		
		$o->openField('&nbsp;');
		?>
		<table border="0">
			<tr>
				<td class="sq-backend-data">To acquire the lock, enter the 8 character string you see below into the box supplied, then press the Commit button</td>
			</tr>
			<tr>
				<td valign="top"><?php security_key(8, 20, 2.5); ?></td>
			</tr>
		</table>
		<?php
		$o->closeSection();

		$o->openSection('Other Assets In This Lock Chain');
		$o->openField('&nbsp;');
		if (empty($lock['chained_assets'])) {
			echo '<i>No other assets are in this lock chain.</i>';
		} else {
			echo '<ul>';
			foreach ($lock['chained_assets'] as $chained_assetid => $chained_info) {
				$chained_asset = &$this->am->getAsset($chained_assetid);
				if (is_null($chained_asset)) {
					trigger_error('Asset #'.$chained_assetid.' not found', E_USER_WARNING);
					break;
				}
				echo '<li class="sq-backend-data">'.$chained_asset->name.'</li>';
			}
			echo '</ul>';
		}
		$o->closeSection();

		$o->openField('', 'commit');
		normal_button('cancel', 'Cancel', 'window.close()');
		echo '&nbsp;';
		submit_button('commit', 'Commit');

	}// end _forceablyAcquireLock()


	/**
	* Returns all child links that the passed asset has, used by the duplication process
	*
	* @access public
	* @return Array()
	*/
	function getAllChildLinks($assetid)
	{
		static $_tmp = Array();

		if (!isset($_tmp['child_links'][$assetid])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$where = 'l.minorid = '.$db->quote($assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
					'.$where.' LIMIT 1';

			$treeid = $db->getOne($sql);
			if (DB::isError($treeid)) {
				trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			}

			$current_level = strlen($treeid) / SQ_CONF_ASSET_TREE_SIZE;

			$where = 't.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT SUBSTRING(t.treeid FROM '.$db->quote(strlen($treeid) + 1).') as treeid,
							(CHARACTER_LENGTH(t.treeid) / '.SQ_CONF_ASSET_TREE_SIZE.') '.(($current_level) ? ' - '.$current_level : '').' as level,
							l.linkid, a.assetid, a.short_name, a.type_code, l.link_type, l.sort_order, l.value, l.dependant
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
						  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.minorid = a.assetid
					'.$where.'
					ORDER BY t.treeid';

			$child_links = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
			if (DB::isError($child_links)) {
				trigger_error($child_links->getMessage().'<br/>'.$child_links->getUserInfo(), E_USER_ERROR);
			}

			// OK, what we are going to do is set up the effective dependant treeid for each tree link
			for(reset($child_links); null !== ($treeid = key($child_links)); next($child_links)) {
				if ($child_links[$treeid]['dependant']) {
					$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
					if ($parent_treeid == '') {
						$child_links[$treeid]['dependant_treeid'] = '';
					} else {
						$child_links[$treeid]['dependant_treeid'] = $child_links[$parent_treeid]['dependant_treeid'];
					}
				} else {
					$child_links[$treeid]['dependant_treeid'] = $treeid;
				}
			}// end for

			$_tmp['child_links'][$assetid] = $child_links;

		}// end if

		return $_tmp['child_links'][$assetid];

	}// getAllChildLinks()


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @access public
	*/
	function _printAssetTypeList()
	{
		$o = &$this->backend->out;

		//$o->addCrummingLink('Asset Types', $o->getCurrentLocation());
		$o->setHeading('Assets Types', 'create');
		$o->addHiddenField('am_action');
		$o->addHiddenField('am_new_type_code');

		$o->openSection('Assets Types');
		$o->openField('', 'new_line');


		$offspring = Array();
		for (reset($this->am->_asset_types); null !== ($type_code = key($this->am->_asset_types)); next($this->am->_asset_types)) {
			$parent = $this->am->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}//end for

		for (reset($offspring); null !== ($type_code = key($offspring)); next($offspring)) {
			sort($offspring[$type_code]);
		}//end for

	?>
		<script language="JavaScript" type="text/javascript">
		<!--

			var current_asset_name = '';

			function asset_checked(asset_name, customisation) {
				current_asset_name = asset_name;

				var button = get_form_element('edit_button');
				if (customisation) {
					button.value = 'Edit ' + asset_name;
				} else {
					button.value = '- - - - - - - - - - - - -';
				}
			}

			function create_custom() {

				if (get_form_element_value('am_type_code')) {

					var type_codes = new Array("<?php echo implode('", "', array_keys($this->am->_asset_types)); ?>");
					var new_type_code = '';
					var prompt_str = '';
					do {

						if (new_type_code == '') {
							prompt_str = 'Please Enter a unique Asset Type Code for your new customisation';
						} else {
							prompt_str = '"' + new_type_code + '" is already in use\nPlease enter a unique Asset Type Code';
						}

						new_type_code = prompt(prompt_str, new_type_code);
						// they hit cancel
						if (new_type_code == null) return false;

						// make sure it's in a proper format
						new_type_code = new_type_code.toLowerCase();
						new_type_code = new_type_code.replace(/ /g, '_');
						new_type_code = new_type_code.replace(/[^a-z_]/g, '');
						new_type_code = new_type_code.replace(/_+/g, '_');

					} while (array_search(type_codes, new_type_code) != null)

					set_hidden_field('am_new_type_code', new_type_code);
					set_hidden_field('am_action', 'create_custom');
					return true;
				}
				return false;
			}

		//-->
		</script>

		<table cellpadding="0" cellspacing="0" border="0">
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					&nbsp;Asset
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				$this->_recursePrintAssetList($offspring, 'asset');
			?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				//submit_button('create_button', 'Create Custom Asset', 'return create_custom();');
				//submit_button('edit_button', '- - - - - - - - - - - - -', 'if (this.value.substr(0, 4) == \'Edit\') { set_hidden_field(\'am_action\', \'edit\'); return true; } else { return false };');
			?>
				</td>
			</tr>
		</table>
	<?php
		$o->closeSection();

	}// end printAssetList()

	function _recursePrintAssetList(&$offspring, $parent)
	{
		$o = &$this->backend->out;

	?>
		<table cellpadding="0" cellspacing="0" border="0">
	<?php
		$num_kids = count($offspring[$parent]);
		for ($i = 0; $i < $num_kids; $i++) {

			$type_code = $offspring[$parent][$i];
			$end = ($i == $num_kids - 1);
			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?>
			<tr>
				<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					<script language="Javascript">sqPrintIcon("<?php echo $this->am->getAssetIconURL($type_code); ?>", "16", "16", "");</script>&nbsp;&nbsp;<?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?>
				</td>
			</tr>
		<?php
			if (!empty($offspring[$type_code])) {
			?>
				<tr>
					<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
					<td class="sq-backend-data">
				<?php
					$this->_recursePrintAssetList($offspring, $type_code);
				?>
					</td>
				</tr>
			<?php
			}//end if

		}//end for
	?>
		</table>
	<?php

	}//end _printAssetTypeList()

}//end class
?>
