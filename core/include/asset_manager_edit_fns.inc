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
* @package Resolve
*/
class Asset_Manager_Edit_Fns extends Resolve_Object
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
		$this->Resolve_Object();
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
			// we might be comming here from the frontend
			$url_asset = &$this->am->getAssetFromURL(current_protocol(), null, true, true);
			if (is_null($url_asset)) {
				$_GET['am_section'] = '';
			} else {
				$_GET['assetid']    = $url_asset->id;
				$_GET['am_section'] = 'edit_asset';
				$GLOBALS['FROM_FRONTEND'] = true;
			}
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
				if (($err_msg = $parent->canLinkToType($_GET['type_code'], $_GET['link_type'])) !== true) {
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
						$o->addMessage(SQ_BO_MSG_NOTICE, 'New '.$this->am->_asset_types[$_GET['type_code']]['name'].' "'.$asset->name.'" created');
						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" was successfully linked to "'.$parent->name.'"');
						$this->am->registerAsset($asset);
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
					
					// try to aquire a lock on this asset?
					if (isset($_POST['sq_aquire_lock'])) {
						if (!$asset->aquireLock()) return false;
					}

					// try to aquire a lock on this asset?
					if (isset($_POST['sq_manual_release_lock'])) {
						if (!$asset->releaseLock()) return false;
					}

					if ($asset->processBackend($this->backend->out, $link)) {

						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" Updated');
						
						$lock = $asset->getLockInfo();
						if (!empty($lock)) {
							// this asset is currently locked
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
							if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
								if (false && isset($_POST['sq_release_lock']) && (int) $_POST['sq_release_lock'] == 1) {
									// committing changes and releasing the lock
									if (!$asset->releaseLock()) {
										trigger_error('Failed to release lock on "'.$asset->name.'"', E_USER_WARNING);
									}
								} else {
									// not releasing the lock, so re-aquire
									if (!$asset->aquireLock()) {
										trigger_error('Failed to re-aquire lock on "'.$asset->name.'"', E_USER_WARNING);
									}
								}
							}
						}
					}
				}

				$asset->paintBackend($this->backend->out);
				break;

			case 'duplicate' :
				$this->_dupeAsset();
				break;

			case 'delete' :
				$this->_deleteAsset();
				break;

			default   :
				$this->_printAssetList();

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

//			$pb->addStep('Obtain Locks', '_dupeStepObtainLocks');
			$pb->addStep('Duplicate', '_dupeStepDuplicate', null, '_dupeStepDuplicateDone');
			$pb->addStep('Remap Ids', '_dupeStepRemap', null, '_dupeStepRemapDone');
//			$pb->addStep('Release Locks', '_dupeStepReleaseLocks', '_dupeStepReleaseLocksInit');
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

/*
//			?><table border="1"><tr><td valign="top"><?php
//			pre_echo($_POST['duplicate_treeids']);
//			pre_echo($duplicate_treeids);
//			?></td><td valign="top"><?php
//			pre_echo($effective_duplicate_treeids);
//			?></td><td valign="top"><?php
//			pre_echo($tree);
//			?></td><td valign="top"><?php
//			pre_echo($indexed_effective_duplicate_treeids);
//			?></td></tr></table><?php
*/
			// OK, what we have with the $effective_duplicate_treeids array is access to all the assets that 
			// we are going to duplicate to have to duplicate, with all the assets that they will create
			// when duplicate is called (because it also duplicates any dependantly linked assets)

			pre_echo("Call duplicate() on ".count($effective_duplicate_treeids)." assets, which will duplicate ".(count($duplicate_treeids) + 1)." assets");

			$vars['tree'] = $tree;
			$vars['effective_duplicate_treeids'] = $indexed_effective_duplicate_treeids;
			$vars['dupe_pos'] = 0;
			$vars['dupe_map'] = Array();
			$vars['remap_pos'] = 0;
			$vars['remaped_assetids'] = Array();

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

					function duplicate_treeids_check(treeid) {

						var tmp_treeid = new String(treeid);

						var clicked_elem = get_form_element("duplicate_treeids[" + tmp_treeid + "]");
						// if we were checked make sure that all our parents are checked
						if (clicked_elem.checked) {
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
								elem.checked = clicked_elem.checked;
							}
						}

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
							check_box('duplicate_treeids['.$treeid.']', $tree[$treeid]['linkid'], !empty($_POST['duplicate_treeids'][$treeid]), 'duplicate_treeids_check("'.$treeid.'");'); 
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
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the actual duplication of the assets
	*
	* @access private
	*/
	function _dupeStepDuplicate(&$pb)
	{
		$vars = $pb->getRunningVars();
#speed_check();
#		pre_echo("DUPE POS : ".$vars['dupe_pos']);
		$treeid = $vars['effective_duplicate_treeids'][$vars['dupe_pos']]['treeid'];
		$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);

#		pre_echo("TREE ID : ".$treeid);
		// if there is a blank treeid then we must be duplicating the initialising asset
		$link    = Array('asset' => null, 'link_type' => 0, 'sort_order' => -1, 'value' => '');
		$assetid   = 0;
		$type_code = '';
		// if the treeid is blank we are duping the initating asset
		if ($treeid == '') { 
			$assetid			= $vars['assetid'];
			$type_code			= $vars['type_code'];
			$link['link_type']	= SQ_LINK_TYPE_1;
			$link['sort_order']	= $vars['to_parent_pos'];
			$link['asset']		= &$GLOBALS['SQ_SYSTEM']->am->getAsset($vars['to_parent_assetid']);

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
			if (isset($vars['dupe_map'][$parent_assetid])) { 
				$link['asset']	= &$GLOBALS['SQ_SYSTEM']->am->getAsset($vars['dupe_map'][$parent_assetid], $parent_type_code);
			}

			$link['link_type']	= $vars['tree'][$treeid]['link_type'];
			$link['sort_order']	= $vars['tree'][$treeid]['sort_order'];
			$link['value']		= $vars['tree'][$treeid]['value'];

		}// end if

#speed_check("two");
		if (is_null($link['asset'])) { 
			trigger_error('Unable to find parent asset for Asset Id #'.$assetid, E_USER_WARNING);
			$pb->abort();
			return;
		}

#		pre_echo("ASSET ID : ".$assetid);

		// If this asset has already been duplicated, 
		// then just link it to it's new parent
		if (isset($vars['dupe_map'][$assetid])) { 
#		pre_echo("DUPE ASSET : $assetid");

			$dupe = &$GLOBALS['SQ_SYSTEM']->am->getAsset($vars['dupe_map'][$assetid], $type_code);
			if (is_null($dupe)) {
				trigger_error('Unable to find duplicated asset for Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if

			$pb->setStatusMsg('Linking '.$dupe->name);

			$linkid = $link['asset']->createLink($dupe, $link['link_type'], $link['value'], $link['sort_order']);
			if (!$linkid) {
				$pb->abort();
				return;
			}// end if

		// otherwise duplicate and link
		} else {

#speed_check("three");
#			pre_echo("DUPLICATE ASSET : $assetid");
			$orig = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			if (is_null($orig)) {
				trigger_error('Unable to find original asset, Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if
			$pb->setStatusMsg('Duplicating '.$orig->name);
#speed_check("four");
			$dupe = &$orig->duplicate($link, $vars['dupe_map'], true, true);
#speed_check("five");
			if (is_null($dupe)) {
				trigger_error('Duplicate failed for Asset Id #'.$assetid, E_USER_WARNING);
				$pb->abort();
				return;
			}// end if

#			pre_echo("DUPLICATED ASSETID : ".$dupe->id);

		}// end if

		$pb->setPercent(($vars['dupe_pos'] + 1) / count($vars['effective_duplicate_treeids']));

		$vars['dupe_pos']++;
#speed_check('FINISHED');

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
	* Called repeatedly by the Progress Bar over multiple page reloads until it set's the percent to 100
	* Performs the remapping of assetids from their original values to their new values
	*
	* @access private
	*/
	function _dupeStepRemap(&$pb)
	{
		$vars = $pb->getRunningVars();

		$treeids = $vars['effective_duplicate_treeids'][$vars['remap_pos']]['dependants'];

		for($i = 0, $total = count($treeids); $i < $total; $i++) {
			$treeid = $treeids[$i];

			// if the treeid is blank we are remaping the initating asset
			if ($treeid == '') { 
				$orig_assetid	= $vars['assetid'];
				$type_code		= $vars['type_code'];
			// else we are remaping a sub asset
			} else {
				$orig_assetid	= $vars['tree'][$treeid]['assetid'];
				$type_code		= $vars['tree'][$treeid]['type_code'];
			}// end if

			// if we have already done this assetid from somewhere else, no need to go any further
			if (in_array($vars['dupe_map'][$orig_assetid], $vars['remaped_assetids'])) continue;

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($vars['dupe_map'][$orig_assetid], $type_code);
			if (is_null($asset)) {
				trigger_error('Unable to find duplicated asset, Asset Id #'.$vars['dupe_map'][$orig_assetid], E_USER_WARNING);
				$pb->abort();
				return;
			}// end if
			
			// only set for the first one, the owner of the dependants
			if ($i == 0) $pb->setStatusMsg('Remaping '.$asset->name);

			if (!$asset->remapAssetids($vars['dupe_map'])) {
				trigger_error('Remap failed for asset, Asset Id #'.$asset->id, E_USER_WARNING);
				$pb->abort();
				return;
			}// endif

			$vars['remaped_assetids'][] = $asset->id;

		}// end for

		$pb->setPercent(($vars['remap_pos'] + 1) / count($vars['effective_duplicate_treeids']));
		$vars['remap_pos']++;
		$pb->setRunningVars($vars);

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
			$link = $asset->getLinkById($_GET['delete_linkid'], 'minor');
			$parent = &$this->am->getAsset($link['majorid'], $link['type_code']);
			$parent->aquireLock();
			$parent->deleteLink($link['linkid']);
			$parent->releaseLock();
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
			$ei->paintLinking($asset, $o);

			$o->closeSection();

			$o->commitButton();
		}

	}// end _deleteAsset()


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
			$sql = 'SELECT t.treeid
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($assetid).'
					LIMIT 1;';

			$treeid = $db->getOne($sql);
			if (DB::isError($treeid)) {
				trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			}

			$current_level = strlen($treeid) / SQ_CONF_ASSET_TREE_SIZE;

			$sql = 'SELECT SUBSTRING(t.treeid FROM '.$db->quote(strlen($treeid) + 1).') as treeid,
							(CHARACTER_LENGTH(t.treeid) / '.SQ_CONF_ASSET_TREE_SIZE.') '.(($current_level) ? ' - '.$current_level : '').' as level,
							l.linkid, a.assetid, a.short_name, a.type_code, l.link_type, l.sort_order, l.value, l.dependant
					FROM sq_asset_link_tree t
						  INNER JOIN sq_asset_link l ON t.linkid = l.linkid
						  INNER JOIN sq_asset a ON l.minorid = a.assetid
					WHERE t.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid).'
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
	function _printAssetList()
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
//			$bg_img = 'images/page_'.(($page[effective_visible]) ? '' : 'in').'visible_'.page::get_status_colour($page[effective_status]).'.gif';

			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?>
			<tr>
				<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
				<?php /*
					<  ?php radio_button('am_type_code', $type_code, false, 'asset_checked(\''.addslashes($this->am->_asset_types[$type_code]['name']).'\', '.(($this->am->_asset_types[$type_code]['customisation']) ? 'true' : 'false').');'); ?  >
					<span class="sq-backend-<  ?php echo ($this->am->_asset_types[$type_code]['customisation']) ? 'custom-' : ''?  >asset"><  ?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?  ></span>
					*/
				?>
					&nbsp;&nbsp;<?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?>
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

	}//end _printAssetList()

}//end class
?>
