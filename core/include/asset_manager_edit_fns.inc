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
* $Id: asset_manager_edit_fns.inc,v 1.75 2004/06/09 04:50:06 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

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

	}//end constructor


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @access public
	*/
	function paintBackend()
	{
		$o = &$this->backend->out;

		if (!isset($_REQUEST['am_section'])) {
			if (isset($_GET['sq_from_frontend'])) $_REQUEST['am_section'] = 'edit_asset';
			else if (SQ_IN_LIMBO) $_REQUEST['am_section'] = 'limbo';
			else $_REQUEST['am_section'] = '';
		}

		$o->addFormActionGetVar('am_section', $_REQUEST['am_section']);
		$o->addHiddenField('am_form_submitted', '1');

		switch($_REQUEST['am_section']) {

			case 'add_asset' :

				$parent = &$this->am->getAsset($_REQUEST['parent_assetid']);
				if (is_null($parent)) {
					trigger_error('Parent Asset #'.$_REQUEST['parent_assetid'].' not found', E_USER_WARNING);
					break;
				}

				// make sure that we are allowed to link this type to the parent
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $_REQUEST['type_code'], $_REQUEST['link_type'])) !== true) {
					trigger_error($err_msg, E_USER_WARNING);
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
					$link = Array('asset'		=> &$parent,
								  'link_type'	=> $_REQUEST['link_type'],
								  'value'		=> '',
								  'sort_order'	=> $_REQUEST['pos']);

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
				}

				$asset->paintBackend($this->backend->out);

			break;

			case 'edit_asset' :
			case 'limbo'      :

				$asset = &$this->am->getAsset($_REQUEST['assetid']);
				if (is_null($asset)) {
					trigger_error('No Asset found for Id #'.$_REQUEST['assetid'], E_USER_WARNING);
					return;
				}
				if (SQ_IN_LIMBO) {
					$limbo_asset = &$this->am->getAsset($_REQUEST['limbo_assetid']);
					// if the limbo asset is invalid, just use the current asset
					if (is_null($limbo_asset)) $limbo_asset = &$asset;
					$o->addFormActionGetVar('limbo_assetid', $limbo_asset->id);
				}

				$o->addFormActionGetVar('assetid', $asset->id);
				$o->addHiddenField('backend_assetid', $asset->id);

				// if the form has been submitted then, process it
				$link = Array();
				if (!empty($_POST['process_form']) && !empty($_POST['am_form_submitted'])) {

					$do_process_backend = true;

					// try to acquire a lock on this asset?
					if (!empty($_POST['sq_lock_acquire']) && !empty($_POST['sq_lock_type'])) {
						//// START HIPO PROCESSING ////
						$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
						$vars = Array('assetid' => $asset->id, 'lock_type' => $_POST['sq_lock_type'], 'forceably_acquire' => (bool) $_POST['sq_lock_acquire_by_force']);
						$hh->queueHipo('hipo_job_acquire_lock', $vars);
						////  END HIPO PROCESSING  ////
						$do_process_backend = false;
					}

					// try to release a lock on this asset?
					if (!empty($_POST['sq_lock_release_manual']) && !empty($_POST['sq_lock_type'])) {
						$this->am->releaseLock($asset->id, $_POST['sq_lock_type']);
						$do_process_backend = false;
					}

					if ($do_process_backend && $asset->processBackend($this->backend->out, $link)) {

						if (!empty($_POST['sq_lock_release']) && !empty($_POST['sq_lock_type'])) {
							// committing changes and releasing the lock
							if (!$this->am->releaseLock($asset->id, $_POST['sq_lock_type'])) {
								trigger_error('Failed to release "'.$_POST['sq_lock_type'].'" lock on "'.$asset->name.'"', E_USER_WARNING);
							}
						} else {
							// not releasing the lock, so re-acquire
							if (!$this->am->updateLock($asset->id, $_POST['sq_lock_type'])) {
								trigger_error('Failed to re-acquire "'.$_POST['sq_lock_type'].'" lock on "'.$asset->name.'"', E_USER_WARNING);
							}
						}
					}// end if

					// the backend of the asset has been processed so lets see if we have any HIPO
					// jobs that need to be run
					$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$url = $hh->runQueuedJobs();
					if (!empty($url)) $o->setRedirect($url);
				}

				if (SQ_IN_LIMBO) $limbo_asset->printFrontend();
				else $asset->paintBackend($this->backend->out);

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
	* @access private
	*/
	function _forceablyAcquireLock()
	{
		$o = &$this->backend->out;

		if (isset($_GET['forceably_acquire_confirm'])) {
			// they have confirmed their intention to forceably acquire the lock on this asset
			$asset = &$this->am->getAsset($_GET['assetid']);
			if (is_null($asset)) {
				trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
				return;
			}

			// check that they entered the correct security key
			if (!validate_security_key()) {
				// the security key entered is wrong
				$o->openSection('Security Key Incorrect');
				$o->openField('&nbsp;');
				echo '<b><i>The security key entered was incorrect. The lock on "'.$asset->name.'" has not been forceably acquired. You may try again using the new security key displayed below.</i></b>';
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
		$asset = &$this->am->getAsset($_GET['assetid']);
		if (is_null($asset)) {
			trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
			return;
		}

		$locks = $this->am->getLockInfo($asset->id, $_GET['sq_lock_type'], true, true);
		$got_locks = false;
		foreach($locks as $lock_type => $lock) {
			if (!empty($lock)) {
				$got_locks = true;
				break;
			}
		}
		if (!$got_locks) {
			$o->addOnLoad('alert("No locks are held on \\"'.addslashes($asset->name).'\\""); window.close();');
			return;
		}

		$o->openSection('Forceably Acquire');
		$o->openField('&nbsp;');
		?>
			You are about to forceably acquire the following locks on "<?php echo $asset->name; ?>"<br>
			<b>Any changes being made to "<?php echo $asset->name; ?>" or any other asset in the locking chain (<i>see full list below</i>) will be lost.</b>
		<?php 
		
		$o->openField('&nbsp;');
		?>
		<table border="0">
			<tr>
				<td class="sq-backend-data">
					To acquire the lock, enter the 8 character string you see below into the box supplied, 
					then press the Commit button
				</td>
			</tr>
			<tr>
				<td valign="top"><?php security_key(8, 20, 2.5); ?></td>
			</tr>
		</table>
		<table class="sq-backend-table" style="width: auto;">
			<tr>
				<td class="sq-backend-table-header">
					Lock Type
				</td>
				<td class="sq-backend-table-header">
					&nbsp;
				</td>
				<td class="sq-backend-table-header">
					Other Assets in Chain
				</td>
			</tr>
		<?php

		foreach($locks as $lock_type => $lock) {
			if (empty($lock)) continue;
			// if this lock is held the current user, we aren't really forcing this one
			if ($lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserId()) continue;
		?>
			<tr>
				<td class="sq-backend-table-cell">
					<?php echo ucwords(str_replace('_', ' ', $lock_type)); ?>
				</td>
				<td class="sq-backend-table-cell">
					<?php

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
					<p class="sq-backend-locked-by-someone-else">
						Held by user "<?php echo $user->name; ?>" at "<?php echo $editing->name; ?>".<br>
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
							$chained_asset = &$this->am->getAsset($chained_info['assetid']);
							if (is_null($chained_asset)) {
								trigger_error('Asset #'.$chained_assetid.' not found', E_USER_WARNING);
								continue;
							}
							echo '<li class="sq-backend-data">'.$chained_asset->name.'</li>';
						}// end foreach
						echo '</ul>';
					}// end if
				?>
				</td>
			</tr>
		<?php
		}// end foreach lock types
		?>

		</table>
		<?php
		$o->closeSection();

		$o->openField('', 'commit');
		normal_button('cancel', 'Cancel', 'window.close()');
		echo '&nbsp;';
		submit_button('commit', 'Commit');

	}//end _forceablyAcquireLock()


	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @access public
	*/
	function _printAssetTypeList()
	{
		$o = &$this->backend->out;

		//$o->addCrummingLink('Asset Types', $o->getCurrentLocation());
		$o->setHeading('Assets Types', '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$o->filesPath('/images/icons/header/asset_tree.png').'", "20", "20", "HIPO Icon");</script>');

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

	}//end printAssetList()


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
					<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $this->am->getAssetIconURL($type_code); ?>", "16", "16", "");</script>&nbsp;&nbsp;<?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?>
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

	}//end _recursePrintAssetList()


}//end class

?>
