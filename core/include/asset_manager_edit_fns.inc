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
* $Id: asset_manager_edit_fns.inc,v 1.109 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.109 $
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
		$this->am = $am;
		$this->backend = $backend;

	}//end constructor


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @return void
	* @access public
	*/
	function paintBackend()
	{
		$o = $this->backend->out;

		if (!isset($_REQUEST['am_section'])) {
			if (isset($_GET['sq_from_frontend'])) {
				$_REQUEST['am_section'] = 'edit_asset';
			} else if (SQ_IN_LIMBO) {
				$_REQUEST['am_section'] = 'limbo';
			} else {
				$_REQUEST['am_section'] = '';
			}
		}

		$o->addFormActionGetVar('am_section', $_REQUEST['am_section']);
		$o->addHiddenField('am_form_submitted', '1');

		switch ($_REQUEST['am_section']) {

			case 'add_asset' :

				$parent = $this->am->getAsset($_REQUEST['parent_assetid']);
				if (is_null($parent)) {
					trigger_localised_error('SYS0200', E_USER_WARNING, $_REQUEST['parent_assetid']);
					break;
				}

				// make sure that we are allowed to link this type to the parent
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $_REQUEST['type_code'], $_REQUEST['link_type'])) !== TRUE) {
					trigger_localised_error('SYS0303', E_USER_WARNING, $err_msg);
					break;
				}

				$o->addFormActionGetVar('parent_assetid', $_REQUEST['parent_assetid']);
				$o->addFormActionGetVar('pos',            $_REQUEST['pos']);
				$o->addFormActionGetVar('type_code',      $_REQUEST['type_code']);
				$o->addFormActionGetVar('link_type',      $_REQUEST['link_type']);

				$this->am->includeAsset($_REQUEST['type_code']);
				$asset = new $_REQUEST['type_code']();

				// if the form has been submitted then, process it
				if (!empty($_REQUEST['process_form']) && !empty($_REQUEST['am_form_submitted'])) {

					// start the transaction to create the asset and initial link
					$link = Array(
								'asset'			=> &$parent,
								'link_type'		=> $_REQUEST['link_type'],
								'value'			=> '',
								'sort_order'	=> $_REQUEST['pos'],
							);

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$success = $asset->processBackend($this->backend->out, $link);

					if ($success) {
						$o->addFormActionGetVar('am_section', 'edit_asset');
						$o->addFormActionGetVar('assetid', $asset->id);
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					} else {
						// the asset was not created fully or linking failed
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}

					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}

				$asset->paintBackend($this->backend->out);

			break;

			case 'edit_asset' :
			case 'limbo'      :

				$asset = $this->am->getAsset($_REQUEST['assetid']);
				if (is_null($asset)) {
					trigger_localised_error('SYS0195', E_USER_WARNING, $_REQUEST['assetid']);
					return;
				}
				if (SQ_IN_LIMBO) {
					$limbo_asset = $this->am->getAsset($_REQUEST['limbo_assetid']);
					// if the limbo asset is invalid, just use the current asset
					if (is_null($limbo_asset)) $limbo_asset = $asset;
					$o->addFormActionGetVar('limbo_assetid', $limbo_asset->id, TRUE);
				}

				$o->addFormActionGetVar('assetid', $asset->id);
				$o->addHiddenField('backend_assetid', $asset->id);

				// if the form has been submitted then, process it
				$link = Array();
				if (!empty($_POST['process_form']) && !empty($_POST['am_form_submitted'])) {

					$do_process_backend = TRUE;

					// try to acquire a lock on this asset?
					if (!empty($_POST['sq_lock_acquire']) && !empty($_POST['sq_lock_type'])) {
						$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
						$vars = Array(
									'assetids'			=> Array($asset->id,),
									'lock_type'			=> $_POST['sq_lock_type'],
									'forceably_acquire'	=> (bool) $_POST['sq_lock_acquire_by_force'],
								);
						$hh->queueHipo('hipo_job_acquire_locks', $vars);
						$do_process_backend = FALSE;
					}

					// try to release a lock on this asset?
					if (!empty($_POST['sq_lock_release_manual']) && !empty($_POST['sq_lock_type'])) {
						$this->am->releaseLock($asset->id, $_POST['sq_lock_type']);

						// limbo might require more locks so release them
						if (SQ_IN_LIMBO && !empty($_POST['sq_limbo_lock_type'])) {
							foreach (unserialize(urldecode($_POST['sq_limbo_lock_type'])) as $limbo_lock_type) {
								if ($limbo_lock_type != $_POST['sq_lock_type']) {
									$this->am->releaseLock($asset->id, $limbo_lock_type);
								}
							}
						}
						$do_process_backend = FALSE;
					}

					//Bug #5143: Commiting from "approved to go live" causes some attributes on some assets to be cleared.
					//To safeguard against this, there is no point of having locks if we are Archived/Safe Edit Approved as write access will be blocked.	
					if ($do_process_backend && !$asset->accessEffective()){
						if (!$this->am->releaseLock($asset->id, 'attributes')) {
							trigger_localised_error('SYS0182', E_USER_WARNING, 'attributes', $asset->name);
							$do_process_backend = FALSE;
						}
					}

					if ($do_process_backend && $asset->processBackend($this->backend->out, $link)) {

						if (!empty($_POST['sq_lock_release']) && !empty($_POST['sq_lock_type'])) {
							// committing changes and releasing the lock
							if (!$this->am->releaseLock($asset->id, $_POST['sq_lock_type'])) {
								trigger_localised_error('SYS0182', E_USER_WARNING, $_POST['sq_lock_type'], $asset->name);
							}
						} else {
							// not releasing the lock, so re-acquire
							if (!$this->am->updateLock($asset->id, $_POST['sq_lock_type'])) {
								trigger_localised_error('SYS0181', E_USER_WARNING, $_POST['sq_lock_type'], $asset->name);
							}
						}
					}//end if

					// the backend of the asset has been processed so lets see if we have any HIPO
					// jobs that need to be run
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$url = $hh->runQueuedJobs();
					if (!empty($url)) $o->setRedirect($url);
				}//end if process form and form submission

				if (SQ_IN_LIMBO) {
					$limbo_asset->initLimbo();
					$limbo_asset->printFrontend();
				} else {
					$asset->paintBackend($this->backend->out);
				}

			break;

			case 'forceably_acquire_lock' :
				$this->_forceablyAcquireLock();
			break;

			default :
				$this->_printAssetTypeList();

		}//end switch

	}//end paintBackend()


	/**
	* Looks after the warning interface for forceably acquire locks
	*
	* @return void
	* @access private
	*/
	function _forceablyAcquireLock()
	{
		$o = $this->backend->out;

		if (isset($_GET['forceably_acquire_confirm'])) {
			// they have confirmed their intention to forceably acquire the lock on this asset
			$asset = $this->am->getAsset($_GET['assetid']);
			if (is_null($asset)) {
				trigger_localised_error('SYS0086', E_USER_WARNING, $_GET['assetid']);
				return;
			}

			// check that they entered the correct security key
			if (!validate_security_key()) {
				// the security key entered is wrong
				$o->openSection(translate('security_key_incorrect'));
				$o->openField('&nbsp;');
				echo '<b><i>'.translate('security_key_incorrect_message', $asset->name).'</i></b>';
				$o->closeSection();
			} else {
				$o->addOnLoad('window.opener.parent.frames["sq_main"].set_hidden_field(\'sq_lock_acquire_by_force\', \'1\'); window.opener.parent.frames["sq_main"].set_hidden_field(\'sq_lock_acquire\', \'1\'); window.opener.parent.frames["sq_main"].set_hidden_field(\'sq_lock_type\', \''.addslashes($_GET['sq_lock_type']).'\'); window.opener.parent.frames["sq_main"].submit_form(); window.close();');
				return;
			}
		}

		$o->addFormActionGetVar('assetid', $_GET['assetid']);
		$o->addFormActionGetVar('sq_lock_type', $_GET['sq_lock_type']);
		$o->addFormActionGetVar('forceably_acquire_confirm', '1');
		$o->addFormActionGetVar('ignore_frames', '1');
		$asset = $this->am->getAsset($_GET['assetid']);
		if (is_null($asset)) {
			trigger_localised_error('SYS0086', E_USER_WARNING, $_GET['assetid']);
			return;
		}

		$locks = $this->am->getLockInfo($asset->id, $_GET['sq_lock_type'], TRUE, TRUE);
		$got_locks = FALSE;
		foreach ($locks as $lock_type => $lock) {
			if (!empty($lock)) {
				$got_locks = TRUE;
				break;
			}
		}
		if (!$got_locks) {
			$o->addOnLoad('alert(js_translate("no_lock_held", "'.addslashes($asset->name).'")); window.close();');
			return;
		}

		$o->openSection(translate('forcibly_acquire'));
			$o->openField('');
				$o->note(translate('forcibly_acquire_lock_can_cause_losses', htmlspecialchars($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlspecialchars($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
			$o->openField('');
		?>
		<table border="0">
			<tr>
				<td>
					<?php echo translate('enter_character_security_string'); ?>
				</td>
			</tr>
			<tr>
				<td valign="top"><?php security_key(4, 4, 2); ?></td>
			</tr>
		</table>
		<table class="sq-backend-table" style="width: auto;">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('lock_type'); ?>
				</td>
				<td class="sq-backend-table-header">
					&nbsp;
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('other_assets_in_chain'); ?>
				</td>
			</tr>
		<?php

		foreach ($locks as $lock_type => $lock) {
			if (empty($lock)) continue;
			// if this lock is held the current user, we aren't really forcing this one
			if ($lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserId()) {
				continue;
			}
		?>
			<tr>
				<td class="sq-backend-table-cell">
					<?php echo ucwords(str_replace('_', ' ', $lock_type)); ?>
				</td>
				<td class="sq-backend-table-cell">
					<?php

					// this asset is currently locked
					// so display message to the user
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

					?>
					<p class="sq-backend-locked-by-someone-else">
						Held by user "<?php echo htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>" at "<?php echo htmlspecialchars($editing->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>".<br>
						<?php echo $expires_in; ?>
					</p>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					if (empty($lock['chained_assets'])) {
						echo '<i>No other assets are in this lock chain.</i>';
					} else {
						echo '<ul>';
						foreach ($lock['chained_assets'] as $chained_info) {
							$chained_asset = $this->am->getAsset($chained_info['assetid']);
							if (is_null($chained_asset)) {
								trigger_localised_error('SYS0086', E_USER_WARNING, $chained_info['assetid']);
								continue;
							}
							echo '<li class="sq-backend-data">'.htmlspecialchars($chained_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</li>';
						}//end foreach
						echo '</ul>';
					}//end if
				?>
				</td>
			</tr>
		<?php
		}//end foreach lock types
		?>

		</table>
		<?php
		$o->closeSection();

		$o->openField('', 'commit');
		normal_button('cancel', translate('cancel'), 'window.close()');
		echo '&nbsp;';
		submit_button('commit', SQ_CONF_COMMIT_BUTTON_TEXT);

	}//end _forceablyAcquireLock()


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @return void
	* @access public
	*/
	function _printAssetTypeList()
	{
		$o = $this->backend->out;

		$o->setHeading('Asset Types', sq_get_icon($o->filesPath('/images/icons/header/asset_tree.png'), '', '', 'Asset Types'));
		$o->setPageTitle('Asset Types');

		$o->addHiddenField('am_action');
		$o->addHiddenField('am_new_type_code');

		$o->openSection(translate('asset_types'));
		$o->openField('', 'wide_col');


		$offspring = Array();
		for (reset($this->am->_asset_types); NULL !== ($type_code = key($this->am->_asset_types)); next($this->am->_asset_types)) {
			$parent = $this->am->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) {
				$offspring[$parent] = Array();
			}
			$offspring[$parent][] = $type_code;
		}//end for

		for (reset($offspring); NULL !== ($type_code = key($offspring)); next($offspring)) {
			sort($offspring[$type_code]);
		}

		?>
		<table class="sq-asset-tree">
			<tr>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="20" border="0" alt="branch" /></td>
				<td>
					&nbsp;Asset
				</td>
			</tr>
			<tr>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td>
					<?php
						$this->_recursePrintAssetList($offspring, 'asset');
					?>
				</td>
			</tr>
			<tr>
				<td><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
			</tr>
		</table>
	<?php
		$o->closeSection();

	}//end _printAssetTypeList()


	/**
	* Print a portion of the asset type tree, recursively
	*
	* @param array	&$offspring	Map of parents => children
	* @param object	$parent		The parent whose subtree should be printed
	*
	* @return void
	* @access private
	*/
	function _recursePrintAssetList(&$offspring, $parent)
	{
		$o = $this->backend->out;

		?>
		<table>
		<?php
		$num_kids = count($offspring[$parent]);
		for ($i = 0; $i < $num_kids; $i++) {

			$type_code = $offspring[$parent][$i];
			$end = ($i == $num_kids - 1);
			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?>
			<tr>
				<td <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td>
					<?php sq_print_icon($this->am->getAssetIconURL($type_code), "16", "16", ""); ?>&nbsp;&nbsp;<?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?>
				</td>
			</tr>
		<?php
			if (!empty($offspring[$type_code])) {
			?>
				<tr>
					<td <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
					<td>
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

	}//end _recursePrintAssetList()


}//end class

?>
