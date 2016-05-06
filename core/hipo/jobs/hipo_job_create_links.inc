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
* $Id: hipo_job_create_links.inc,v 1.72 2013/10/22 00:38:23 lwright Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to create links
*
* <pre>
* Initial Running Vars :
* 	Array	assets		- the assets to move / create links to
* 							$assets = Array(
* 										[assetid]	=> Array (
* 														0	=> Array(
* 																'linkid'	=> string, (optional)
* 																'link_type'	=> int,
* 																'parentid'	=> string,
* 															   ),
* 													   ),
* 									  )
*	string	link_action	- what to do with the passed assets; can be either 'move' or 'create'
* </pre>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.72 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Create_Links extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Create_Links($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(implode('-',array_keys($this->_running_vars['assets'])));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Create Links');


	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('Confirmation'),

					'function_call'	=> Array(
										'paint_function'	=> 'paintConfirmation',
										'process_function'	=> 'processConfirmation',
									   ),
					'running_mode'	=> 'web',
					'auto_step'		=> FALSE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Creating Links'),

					'function_call'	=> Array(
										'process_function'	=> 'processCreateLinks',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Updating Lookups'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'updateLookups',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Cascading Options'),

					'function_call'	=> Array(
										'paint_function'	=> 'paintInterface',
										'process_function'	=> 'processInterface',
									   ),
					'running_mode'	=> 'web',
					'auto_step'		=> FALSE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Cascading Permissions'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'cascadePermissions',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Cascading Metadata'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'cascadeMetadata',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Cascading Workflow'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'cascadeWorkflow',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Cascading Content Tags'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'cascadeTags',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Cascading Roles'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'cascadeRoles',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Refreshing Asset Map'),

					'function_call'	=> Array(
										'paint_function'	=> 'paintRefresh',
										'process_function'	=> 'processRefresh',
									   ),
					'running_mode'	=> 'web',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Prepares and sets up the running vars of this hipo.
	* This is run before initialise(), because initialise() may not be run at all if this Job can be freestyle()'d
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0026', translate('Unable to initialise Create Link HIPO, no assets specified'), E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_assetid'])) {
			trigger_localised_error('HIPO0025', translate('Unable to initialise Create Link HIPO, no new parent ID specified'), E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_pos'])) {
			trigger_localised_error('HIPO0024', translate('Unable to initialise Create Link HIPO, no new parent position specified'), E_USER_WARNING);
			return '';
		}

		$this->_running_vars['assets_children'] = Array();

		// an array of assetids that are not supposed to be trashed when Safe-Trash = Yes
		$this->_running_vars['do_not_trash'] = Array();

		// Array of content container assetids which can't be trashed.
		$this->_running_vars['safe_edit_bc_restricted'] = Array();

		// for the sake of backwards compatibility, allow for a single link to be specified for an asset instead of an array of links
		// transform any asset link info given in the old format
		for (reset($this->_running_vars['assets']); NULL !== ($assetid = key($this->_running_vars['assets'])); next($this->_running_vars['assets'])) {
			$links = current($this->_running_vars['assets']);
			if (isset($links['linkid']) || isset($links['link_type']) || isset($links['parentid'])) {
				$this->_running_vars['assets'][$assetid] = Array($links);
			}
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* Returns the code_name for the new job
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string
	* @access public
	*/
	function initialise($source_code_name=NULL)
	{
		// get a list of links that will be affected by this change
		if ($this->_running_vars['link_action'] == 'move') {
			$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
			$assets = $this->_running_vars['assets'];
			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			// if we are moving to the trash folder, we need to check if the Safe-Trash pref has been enabled
			$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');

			// when checking for the number of links left in the system, make sure we disregard
			// any link to the trash folder because even if this asset is already in the trash,
			// we should warn about deleting it
			for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
				$links = current($assets);
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				for (reset($links); NULL !== ($link_idx = key($links)); next($links)) {
					// Check if the assets have been moved to the trash. If there are any, warn about which links will be affected
					$current  = current($links);
					$linkid   = $current['linkid'];
					$parentid = $current['parentid'];
					
					if ($new_parent->id == $trash_folder->id) {
						$ignore_link_id = isset($current['linkid']) ? $current['linkid'] : 0;
						$trash_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_folder->id, $asset->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
						$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2, '', TRUE, $ignore_link_id);

						if (!empty($trash_link)) $num_other_links--;
						if (!$num_other_links) {
							// now lets get all the other links this asset has in the system and
							// display them to the user
							// note that we are also going to ask the major asset in the link
							// to describe it so it makes sense to the user looking at it
							$affected_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, '', TRUE, 'minor');
							$safe_trash_cron_job = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, 'minor');

							// We can't trust $affected_links to contain all TYPE_3|NOTICE
							// links, as getLinks() also returns ALL shadow links. So weed
							// out any links that aren't TYPE_3|NOTICE
							foreach ($affected_links as $linkid => $affected_link) {
								if (!($affected_link['link_type'] & (SQ_LINK_TYPE_3 | SQ_LINK_NOTICE))) {
									unset($affected_links[$linkid]);
								}
							}

							// We can't use array_diff() because it does a string comparison
							// ("Array" == "Array".... ooh nasty). So we will trawl through the
							// array to find the cron job and pluck it out
							if (!empty($safe_trash_cron_job)) {
								foreach (array_keys($affected_links) as $affected_link_key) {
									if ($affected_links[$affected_link_key]['linkid'] == $safe_trash_cron_job[0]['linkid']) {
										unset($affected_links[$affected_link_key]);
										break;
									}
								}
							}

							if (!empty($affected_links)) {
								$this->_running_vars['affected_links'][$assetid] = $affected_links;
							}

						}//end if !$num_other_links

						// if we can't Safe-Trash for any reason whatsoever, then
						// let them know
						if ($safe_trash) {
							$trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($assetid, $ignore_link_id, FALSE);
							if ($trash_errors) {
								$this->_running_vars['do_not_trash'][$assetid] = $trash_errors;
								$this->_running_vars['do_not_trash'][$assetid]['name'] = $asset->name;
							}
						}

					}//end if trashing

					if (((boolean) $safe_trash === FALSE) || ($new_parent->id != $trash_folder->id)) {
						// (SM#6761) Work out whether our parent page is in a Safe Editing state, and if we are,
						// block the moving of this link. This occurs even if not moving to trash.
						if ($asset instanceof Bodycopy_Container) {
							$parent_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($parentid);
							if (($parent_info[$parentid]['status'] & SQ_SC_STATUS_SAFE_EDITING) > 0) {
								$this->_running_vars['safe_edit_bc_restricted'][$assetid][] = Array(
								    'linkid'   => $linkid,
									'parentid' => $parentid,
								);

								// Takes priority over safe trash.
								unset($this->_running_vars['do_not_trash'][$assetid]);
							}
						}
					}//end if not trashing, OR trashing without safe trash
				}//end for ($links)
			}//end for ($assets)
		}//end if action = move

		$this->_running_vars['can_skip_cascade'] = TRUE;

		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Paints a confirmation screen for creating the link
	*
	* This allows the user to back out if they want to, as well as be presented
	* with any information/warnings regarding the new link (eg. move to trash)
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param object	&$o			the backend outputter class
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function paintConfirmation(&$step_data, &$o, $prefix)
	{
		$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
		$assets = $this->_running_vars['assets'];
		if (isset($this->_running_vars['reordered_assets'])) {
			$reordered_assets = $this->_running_vars['reordered_assets'];
		}

		if ($this->_running_vars['link_action'] == 'create') {

			// We are creating a new link
			$o->openSection(translate('New Link Confirmation'));

				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$o->openField('');
						echo '<p>'.sprintf(translate('Please confirm that you are creating new link(s) for the following asset(s) under "%s":'), htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</p>';

						echo '<ul>';
						for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($assets);
							foreach ($current as $link) {
								$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);
								echo '<li>'.sprintf(translate('"%1$s" from under "%2$s"'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</li>';

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
							}
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						}
						echo '</ul>';
						echo '<p>'.translate('Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.').'</p>';

				$o->closeField();
			$o->closeSection();

		} else if ($this->_running_vars['link_action'] == 'move') {

			// we are moving an asset
			$o->openSection(translate('Move Confirmation'));


				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">'.strtoupper(translate('Error')).'</span>');

						echo $this->_tmp['message'];
					$o->closeField();
				}

				// Check if any assets were re-ordered outside the HIPO job (ie, parentid == to_parent_id)
				// and print them here to avoid confusion
				if (!empty($reordered_assets)) {
					$o->openField('&nbsp;');
							echo '<p>'.sprintf(translate('The following asset(s) have already been re-ordered under "%s". They do not need to be moved again:'), htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</p>';

							echo '<ul>';
							for (reset($reordered_assets); NULL !== ($assetid = key($reordered_assets)); next($reordered_assets)) {
								$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								echo '<li>"'.htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'".</li>';
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
							}
							echo '</ul>';
					$o->closeField();
				}

				if ($new_parent->id == $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('trash_folder')) {

					// We are moving to trash - lots of special stuff to do
					$this->_paintMoveToTrashConfirmation($o, $prefix);

				} else {
					$to_move   = count($assets);
					$cant_move = count($this->_running_vars['safe_edit_bc_restricted']);

					// We are moving somewhere else
					$o->openField('');
					if ($cant_move === $to_move) {
						?><p><strong class="sq-backend-warning"><?php echo translate('None of the selected assets can be moved from their original parents. See below for details.');?></strong></p><?php
						$this->_hipo_vars['no_next'] = TRUE;
					} else {
						if ($cant_move > 0) {
							// There shouldn't be a situation where the singular form of this is used - because
							// if 0/1 assets can be trashed then you'd get the "none trashable" message above.
							// But with multiple plural forms possible, translate_plural() is still required.
						?><p><strong class="sq-backend-warning"><?php echo sprintf(translate_plural(
								'You have selected to trash %1$s asset but it cannot be moved to the trash. See below for details.',
								'You have selected to trash %1$s assets but %2$s of these assets cannot be moved to the trash. See below for details.',
								$to_move
							), $to_move, $cant_move);?></strong></p><?php
						}

						echo '<p>'.sprintf(translate('Please confirm that you are moving the following asset(s) to under "%s":'), htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</p>';
						echo '<ul>';
					}

					for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$current = current($assets);
						foreach ($current as $link) {
							$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);

							$restricted = FALSE;
							if (array_key_exists($asset->id, $this->_running_vars['safe_edit_bc_restricted']) === TRUE) {
								foreach ($this->_running_vars['safe_edit_bc_restricted'][$asset->id] as $restricted_link) {
									if ($link['parentid'] === $restricted_link['parentid']) {
										$restricted = TRUE;
										break;
									}
								}
							}

							if ($restricted === FALSE) {
								$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);
								echo '<li>'.sprintf(translate('"%1$s" from under "%2$s"'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</li>';
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
							}
						}
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					}

					if ($cant_move < $to_move) {
						echo '</ul>';
					}

					if (isset($this->_hipo_vars['no_next']) && $this->_hipo_vars['no_next']) {
						// if no next button printed, modify the confirm message a little
						echo '<p>'.translate('Press the <b>Cancel</b> button to abort.').'</p>';

					} else {
						echo '<p>'.translate('Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.').'</p>';

					}

					$o->closeField();

					if ($cant_move > 0) {
						$o->openField(translate('Restricted Due To Parent Bodycopy in Safe Edit'), 'new_line');
						echo '<p>'.translate('The following assets cannot be moved away from their current parent bodycopy while the parent is in a Safe Editing status:').'</p>';
					
						echo '<ul>';
						$cant_move_assets = $this->_running_vars['safe_edit_bc_restricted'];
						for (reset($cant_move_assets); NULL !== ($assetid = key($cant_move_assets)); next($cant_move_assets)) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($cant_move_assets);
							foreach ($current as $link) {
								$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);
								echo '<li>'.sprintf(translate('"%1$s" [#%3$s] from under "%2$s" [#%4$s]'),
									htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), 
									htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET),
									$asset->id,
									$parent->id
								).'</li>';

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
							}
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						}

						echo '</ul>';
						$o->closeField();
					}

				}//end else

			$o->closeSection();

		}//end if action is 'move'

		$o->addHiddenField('form_submitted', 1);
		return TRUE;

	}//end paintConfirmation()


	/**
	* Paint the interface for confirming the trashing of asset(s)
	*
	* @param object	&$o		Backend outputter
	* @param string	$prefix	Prefix for form elements
	*
	* @return void
	* @access public
	*/
	function _paintMoveToTrashConfirmation(&$o, $prefix)
	{
		if (!empty($this->_running_vars['remap_choices'])) {

			// They've chosen an asset to remap to but need to choose which of its URLs to use
			$this->_paintRemapURLChooser($o, $prefix);

		} else {

			// check if the Safe-Trash pref has been enabled
			$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
			$assets = $this->_running_vars['assets'];

			// work out how many assets do we really have to trash
			// If we have nothing to trash, we no longer take out the "next"
			// button because of the possibility for marking for Safe-Trash
			$nothing_to_trash = FALSE;
			$not_to_trash = count($this->_running_vars['safe_edit_bc_restricted']);
			$to_trash     = count($assets);

			if ($safe_trash) {
				$not_to_trash += count($this->_running_vars['do_not_trash']);
			}
			
			$nothing_to_trash = ($not_to_trash == $to_trash);

			if (count($assets) === count($this->_running_vars['safe_edit_bc_restricted'])) {
				$this->_hipo_vars['no_next'] = TRUE;
			}

			$o->openField('');

			// Ask them to confirm that they want to trash all these assets
			if (count($assets) - $not_to_trash > 0) {
				echo sprintf(translate_plural(
					'Please confirm that you are moving %s asset to the trash.',
					'Please confirm that you are moving %s assets to the trash.',
					(count($assets) - $not_to_trash)
				), (count($assets) - $not_to_trash)).'</p>';
			}

			// Print Safe-Trash notices
			if ($not_to_trash > 0) {
				if (count($assets) === $not_to_trash) {
					?><p><strong class="sq-backend-warning"><?php echo translate('None of the selected assets can be moved to the trash. See below for details.');?></strong></p><?php

				} else {
					// There shouldn't be a situation where the singular form of this is used - because
					// if 0/1 assets can be trashed then you'd get the "none trashable" message above.
					// But with multiple plural forms possible, translate_plural() is still required.
					?><p><strong class="sq-backend-warning"><?php echo sprintf(
						translate_plural(
							'You have selected to trash %1$s asset but it cannot be moved to the trash. See below for details.',
							'You have selected to trash %1$s assets but %2$s of these assets cannot be moved to the trash. See below for details.',
							count($assets)
						), count($assets), $not_to_trash);?></strong></p><?php

				}
			}

			if (isset($this->_hipo_vars['no_next']) && $this->_hipo_vars['no_next']) {
				// if no next button printed, modify the confirm message a little
				echo '<p>'.translate('Press the <b>Cancel</b> button to abort.').'</p>';

			} else {
				echo '<p>'.translate('Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.').'</p>';

			}

			// Warn that shadow assets will not be trashed
			$shadow_assets = Array();
			foreach ($assets as $one_id => $one_info) {
				if (strpos($one_id,':') !== FALSE) $shadow_assets[] = $one_id;
			}
			if (!empty($shadow_assets)) {
				$shadow_info_array = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($shadow_assets);
				echo '<p><strong class="sq-backend-warning">'.translate('The following assets cannot be moved to trash and will be deleted immediately').':</strong>';

				echo '<ul>';
				foreach ($shadow_info_array as $shadow_info) {
					echo '<li>'.$shadow_info['short_name'].'</li>';
				}
				echo '</ul></p>';
			}

			$o->closeField();

			// Paint the list of affected assets, with appropriate warning message depending on
			// whether we are Safe-Trashing ('those assets will be affected' / 'you can't delete these assets'
			if ($safe_trash) {
				$affected_title = translate('Safe-Trash Linking Restrictions');

				$warning_msg = translate('The following assets are Notice- or Type-3-linked to other assets, and cannot be trashed until the links are removed:');

			} else {
				$affected_title = translate('Affected Assets');

				$warning_msg = translate('Trashing these assets will affect other assets that link to them.  The linked assets for each asset being trashed, with the purpose of each link, are shown below:');

			}
			$this->_paintAffectedAssetsWarning($o, $affected_title, $warning_msg);

			if ($safe_trash && !empty($this->_running_vars['do_not_trash'])) {
				// Paint other safe-trash sections
				$this->_paintSafeTrashStatusWarning($o);
				$this->_paintSafeTrashChildrenWarning($o);
			}

			if (empty($this->_running_vars['safe_edit_bc_restricted']) === FALSE) {
				$o->openField(translate('Restricted Due To Parent Bodycopy in Safe Edit'), 'new_line');
				echo '<p>'.translate('The following assets cannot be moved away from their current parent bodycopy while the parent is in a Safe Editing status:').'</p>';
			
				echo '<ul>';
				$cant_move_assets = $this->_running_vars['safe_edit_bc_restricted'];
				for (reset($cant_move_assets); NULL !== ($assetid = key($cant_move_assets)); next($cant_move_assets)) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					$current = current($cant_move_assets);
					foreach ($current as $link) {
						$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);
						echo '<li>'.sprintf(translate('"%1$s" [#%3$s] from under "%2$s" [#%4$s]'),
							htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), 
							htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET),
							$asset->id,
							$parent->id
						).'</li>';

						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				}

				echo '</ul>';
				$o->closeField();
			}

			if ($safe_trash && !empty($this->_running_vars['do_not_trash'])) {
				$this->_paintMarkForDeletionList($o, $prefix);
			}

			if (!$nothing_to_trash) {
				$this->_paintRemapAssetChooser($o, $prefix);
			}
		}//end else remap assetid not submitted

	}//end _paintMoveToTrashConfirmation()


	/**
	* Warn the user of assets whose trashing will affect other assets
	*
	* If Safe-Trash is enabled this indicates that assets cannot be trashed
	* otherwise it's just a notification
	*
	* @param object	&$o				Backend outputter
	* @param string	$title			Title for the list
	* @param string	$warning_msg	Message to be printed at the top of the list
	*
	* @return void
	* @access public
	*/
	function _paintAffectedAssetsWarning(&$o, $title, $warning_msg)
	{
		// when checking for the number of links left in the system, make sure we disregard
		// any link to the trash folder because even if this asset is already in the trash,
		// we should warn about deleting it
		$affected_links = array_get_index($this->_running_vars, 'affected_links', Array());
		if (!empty($affected_links)) {
			$o->addJsInclude(sq_web_path('lib').'/js/tooltip.js');
			$o->openField($title, 'new_line');
			echo '<p>'.$warning_msg.'</p>';
			foreach ($affected_links as $assetid => $links) {
				echo '<br />'.sprintf(translate('%1$s is linked to %2$s'), get_asset_tag_line($assetid), '<u class="clickable" onclick="toggleNextElt(this, \'TABLE\')" title="'.translate('Click to show/hide details').'">'.sprintf(translate('%d assets'), count($links)).'</u>');



				?>
				<table class="sq-backend-table" style="width: 80%; margin-left: 20px; display: none;">
				<?php
				foreach ($links as $link_data) {
					$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['majorid'], $link_data['major_type_code']);
					$link_description = $major->describeLink($link_data['linkid']);
					?>
					<tr>
						<td><?php echo get_asset_tag_line($major->id); ?></td>
						<td><?php echo (!empty($link_description)) ? $link_description : ''; ?></td>
					</tr>
					<?php
				}//end foreach
				?>
				</table>
				<?php
			}//end foreach
			echo '<br /><br />';
			$o->closeField();
		}//end if any affected links

	}//end _paintAffectedAssetsWarning()


	/**
	* Warn user of assets that can't be trashed because their status violates Safe-Trash rules
	*
	* @param object	&$o	Backend outputter
	*
	* @return void
	* @access public
	*/
	function _paintSafeTrashStatusWarning(&$o)
	{
		// paint the not safe status
		$warning_painted = FALSE;
		foreach ($this->_running_vars['do_not_trash'] as $id => $data) {
			if (isset($data['status'])) {
				if (!$warning_painted) {
					$o->openField(translate('Safe-Trash Status Restrictions'), 'new_line');

					?><p><?php echo translate('The following assets cannot be trashed until their statuses are set to Under Construction or Archived:') ?></p><?php

					echo '<ul>';
					$warning_painted = TRUE;
				}
				echo '<li>';
				echo sprintf(translate('"%1$s" [#%2$s]'), $data['name'], $id).' - ';

				echo translate('Current Status').': ';

				echo get_asset_status_icon($data['status']).get_status_description($data['status']).'<br />';
				echo '</li>';
			}
		}
		if ($warning_painted) {
			echo '</ul>';
			$o->closeField();
		}

	}//end _paintSafeTrashStatusWarning()


	/**
	* Warn the user of assets that can't be trashed because their children violate Safe-Trash rules
	*
	* @param object	&$o	Backend outputter
	*
	* @return void
	* @access public
	*/
	function _paintSafeTrashChildrenWarning(&$o)
	{
		$warning_painted = FALSE;
		foreach ($this->_running_vars['do_not_trash'] as $id => $data) {
			if (isset($data['children'])) {
				if (!$warning_painted) {
					$o->openField(translate('Safe-Trash Restrictions on Child Assets'), 'new_line');

					echo '<p>'.translate('The following assets cannot be trashed because one or more of their children do not satisfy the Safe-Trash conditions:').'</p>';

					echo '<ul>';
					$warning_painted = TRUE;
				}

				$details = '<ul style="display: none">';
				$problem_child_count = 0;
				foreach ($data['children'] as $child_info) {
					$details .= '<li>'.sprintf(translate('"%1$s" [#%2$s]'), $child_info['name'], $child_info['minorid']);

					if ($child_info['link_type'] == SQ_LINK_TYPE_3 || $child_info['link_type'] == SQ_LINK_NOTICE) {
						$details .= ' - '.translate('Asset has Notice or Type-3 links to other assets');

						$problem_child_count++;
					} else if ($child_info['status'] >= SQ_STATUS_LIVE) {
						$details .= ' - '.translate('Current Status').': '.get_asset_status_icon($child_info['status']).get_status_description($child_info['status']);

						$problem_child_count++;
					}
					$details .= '</li>';
				}
				$details .= '</ul>';

				// TODO: translate plural
				if ($problem_child_count) {
				    $main_text = sprintf(
				        translate('%1$s [#%2$s] has <u>%3$s child</u> that does not satisfy the Safe-Trash restrictions'),
				        $data['name'],
				        $id,
				        $problem_child_count
				    );
				} else {
				    $main_text = sprintf(
				        translate('%1$s [#%2$s] has <u>%3$s children</u> that do not satisfy the Safe-Trash restrictions'),
				        $data['name'],
				        $id,
				        $problem_child_count
				    );
				}
				$main_text = str_replace('<u>', '<u class="clickable" onclick="toggleNextElt(this, \'UL\')" title="'.translate('Click to show/hide details').'">', $main_text);
				echo '<li>';
				echo $main_text;
				echo $details;
				echo '</li>';
			}
		}
		if ($warning_painted) {
			echo '</ul>';
			$o->closeField();
		}

	}//end _paintSafeTrashChildrenWarning()


	/**
	* Paint the interface for marking assets for future trashing
	*
	* @param object	&$o		Backend outputter
	* @param string	$prefix	Prefix for form elements
	*
	* @return void
	* @access public
	*/
	function _paintMarkForDeletionList(&$o, $prefix)
	{
		$o->openField(translate('Mark Assets for Future Trashing'), 'new_line');


		$assetids = array_keys($this->_running_vars['do_not_trash']);
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);

		?><p>The assets that cannot be trashed due to Safe-Trash rules are listed below.  Selecting assets from this list will mark them for future trashing.  This creates a cron job that will automatically trash the asset as soon as the Safe-Trash rules are satisfied.</p>

		<div id="<?php echo $prefix ?>_mark">
			<?php
			if (count($assetids) > 1) {
				// enable select-all
				?>
				<input type="checkbox" id="<?php echo $prefix ?>_mark_all" onclick="inputs = this.parentNode.getElementsByTagName('INPUT'); for (i = 0; i &lt; inputs.length; i++) { if ((inputs[i].type == 'checkbox') && (inputs[i]!= this)) inputs[i].checked = this.checked; }">
				<label for="<?php echo $prefix ?>_mark_all"><em><?php echo translate('Select All'); ?></em></label>

				<hr />
				<?php
				$onclick = 'onclick="selall = document.getElementById(\''.$prefix.'_mark_all\'); inputs = this.parentNode.parentNode.getElementsByTagName(\'INPUT\'); sel_value = true; for (i = 0; i &lt; inputs.length; i++) { if ((inputs[i].type == \'checkbox\') && (inputs[i] != selall)) { sel_value = sel_value && inputs[i].checked; } if (!sel_value) break; } selall.checked = sel_value;"';
			} else {
				$onclick = '';
			}
			foreach ($assetids as $assetid) {
				?>
				<div>
					<input type="checkbox" name="<?php echo $prefix ?>_mark[<?php echo $assetid ?>]" id="<?php echo $prefix ?>_mark_<?php echo $assetid ?>" value="1" <?php echo $onclick; ?> >
					<label for="<?php echo $prefix ?>_mark_<?php echo $assetid ?>">
					<?php
					echo get_asset_type_icon($asset_info[$assetid]['type_code']);
					echo sprintf(translate('"%1$s" [#%2$s]'), $asset_info[$assetid]['name'], $assetid);

					?>
					</label>
				</div>
			<?php
			}
			?>
		</div>
		<?php
		echo '<br />';
		$o->closeField();

	}//end _paintMarkForDeletionList()


	/**
	* Paint the interface for choosing an asset to remap URLs of a soon-to-be-trashed asset to
	*
	* @param object	&$o		Backend outputter
	* @param string	$prefix	Prefix for form elements
	*
	* @return void
	* @access public
	*/
	function _paintRemapAssetChooser(&$o, $prefix)
	{
		// Paint the remap-URLs section if appropriate
		$existing_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_keys($this->_running_vars['assets']));
		foreach ($existing_urls as $url) {
			if (!empty($url)) {
				// At least one real URL exists, so ask if they want to remap URLs
				$o->openField(translate('URL Remapping'), 'new_line');

				echo '<p>'.translate('When these assets are moved to the trash, any URLs that they currently have will be broken. To remap their URLs to another asset (Eg. a "Page not found" asset), select that asset in the field below.').'</p>';

				asset_finder($prefix.'_remap_new_url', NULL, Array(), 'sq_sidenav', isset($_GET['SQ_BACKEND_PAGE'])? FALSE : TRUE);
				$o->closeField();
				break;
			}
		}

	}//end _paintRemapAssetChooser()


	/**
	* Paint the interface for choosing which of the selected asset's URLs should be used for remapping
	*
	* @param object	&$o		Backend outputter
	* @param string	$prefix	Prefix for form elements
	*
	* @return void
	* @access public
	*/
	function _paintRemapURLChooser(&$o, $prefix)
	{
		$o->openField('');
		echo '<p>'.translate('The asset you have chosen to remap to has several URLs.  Choose which of its URLs you would like to use').':</p>';

		foreach ($this->_running_vars['remap_choices'] as $i => $url) {
			radio_button($prefix.'_chosen_remap_url', $i, ($i==0));
			label($url['url'], $prefix.'_chosen_remap_url_'.$i);
			echo '<br />';
		}
		$o->closeField();

	}//end _paintRemapURLChooser()


	/**
	* Process confirmation screen, setting up remaps and "mark for Safe-Trash"
	* assets
	*
	* @param array	&$step_data	the step data for this job
	* @param string	$prefix		prefix to be used with this job
	*
	* @return boolean
	* @access public
	*/
	function processConfirmation(&$step_data, $prefix)
	{
		// if we are moving to the trash and they want to remap the URLs of the asset that
		// is being trashed to another asset, go through and do all this for them
		if (isset($_POST['form_submitted'])) {
			if (isset($_POST[$prefix.'_chosen_remap_url'])) {
				$dest_url_data = $this->_running_vars['remap_choices'][(int)$_POST[$prefix.'_chosen_remap_url']];
				$remap_url = ($dest_url_data['http'] ? 'http' : 'https').'://'.$dest_url_data['url'];
				$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
				foreach ($this->_running_vars['assets'] as $source_assetid => $links) {
					if (isset($this->_running_vars['do_not_trash'][$source_assetid])) {
						continue;
					}
					if (!$rm->remapAllURLs($source_assetid, $remap_url)) {
						$source_asset =$GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.sprintf(translate('The system was unable to remap the URLs for "%1$s" to "%2$s"'), htmlentities($source_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $remap_url).'</span>';

						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
						return TRUE;
					}
				}
			}

			if (!empty($_POST[$prefix.'_remap_new_url']['assetid'])) {
				$dest_assetid = $_POST[$prefix.'_remap_new_url']['assetid'];
				$dest_urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($dest_assetid);
				switch (count($dest_urls)) {
					case 1:
						// Target asset has exactly one URL - do the remap straight away
						$dest_url_data = reset($dest_urls);
						$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
						foreach ($this->_running_vars['assets'] as $source_assetid => $links) {
							if (isset($this->_running_vars['do_not_trash'][$source_assetid])) {
								continue;
							}
							if (!$rm->remapAllURLs($source_assetid, ($dest_url_data['http'] ? 'http' : 'https').'://'.$dest_url_data['url'])) {
								$source_asset =$GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);
								$this->_tmp['message'] = '<span style="color: #FF0000;">'.sprintf(translate('The system was unable to remap the URLs for "%1$s" to "%2$s"'), htmlentities($source_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $dest_url_data['url']).'</span>';

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
								return TRUE;
							}
						}
					break;
					case 0:
						// Cannot remap, the target asset has no URLs
						$remap_asset =$GLOBALS['SQ_SYSTEM']->am->getAsset($dest_assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.sprintf(translate('You cannot remap URLs to "%s" because it does not have a URL itself. Please choose another asset.'), htmlentities($remap_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</span>';

						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
						return TRUE;
					break;
					default:
						// Target asset has several URLs; ask the user which one to use
						$this->_running_vars['remap_choices'] = $dest_urls;
						$step_data['percent_done'] = 75;
						$ste_data['complete'] = FALSE;
						return TRUE;
					break;
				}//end switch
			}//end if remap assetid supplied

			if (isset($_POST[$prefix.'_mark'])) {
				$this->_running_vars['mark_for_deletion'] = $_POST[$prefix.'_mark'];
			}

		}//end if HIPO form submitted


		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}

		return TRUE;

	}//end processConfirmation()


	/**
	* Create Links
	*
	* @param array	&$step_data	Step Data
	* @param string	$prefix		prefix
	*
	* @return boolean
	* @access public
	*/
	function processCreateLinks(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!isset($this->_running_vars['process_create_links'])) {
			$this->_running_vars['process_create_links'] = $this->_running_vars['assets'];
		}
		if (!isset($this->_running_vars['done_links'])) {
			$this->_running_vars['done_links'] = Array();
		}

		if (!isset($this->_running_vars['link_count'])) {
			$link_count = 0;
			foreach ($this->_running_vars['assets'] as $assetid => $links) {
				$link_count += count($links);
			}
			$this->_running_vars['link_count'] = $link_count;
		}

		$assets  = $this->_running_vars['process_create_links'];
		$assetid = key($assets);
		$asset   = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$links   = reset($assets);

		$value    = reset($links);
		$link_idx = key($links);
		unset($this->_running_vars['process_create_links'][$assetid][$link_idx]);

		if ($this->_running_vars['link_action'] == 'create') {

			$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

			if (is_null($new_parent)) {
				trigger_localised_error('HIPO0023', sprintf(translate('Unable to create new link, invalid destination asset ID specified (#%d)'), $this->_running_vars['to_parent_assetid']), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (is_null($asset)) {
				trigger_localised_error('HIPO0022', sprintf(translate('Unable to create new link, invalid asset ID specified (%d)'), $assetid), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			// disallow multiple significant links between the same parent and child
			// NOTE: failing this will mean that further links (valid or not) will not be processed, in the same
			// manner as when the new link from $new_parent => $asset isn't created properly (see below)
			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			if ($new_parent->id != $trash_folder->id) {
				if (($value['link_type'] & SQ_SC_LINK_SIGNIFICANT) && $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($new_parent->id, $asset->id, SQ_SC_LINK_SIGNIFICANT)) {
					$to_parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
					trigger_localised_error('SYS0329', sprintf(translate('Asset "%1$s" (#%2$s) cannot be linked to "%3$s" (#%4$s), a significant link already exists between these assets'), $asset->name, $asset->id, $to_parent_asset->name, $to_parent_asset->id), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($to_parent_asset);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			$step_data['message'] = 'Creating a link between "'.htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'" and "'.htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'".';
			// Define the value
			if (isset($this->_running_vars['link_value']) && !empty($this->_running_vars['link_value'])) {
				$value['value'] = $this->_running_vars['link_value'];
			} else {
				$value['value'] = '';
			}

			$linkid = $new_parent->createLink($asset, $value['link_type'], $value['value'], $this->_running_vars['to_parent_pos']);

			if (!$linkid) {
				trigger_localised_error('HIPO0021', sprintf(translate('Unable to create link between "%1$s" and "%2$s".'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if ($this->_running_vars['to_parent_pos'] != -1) {
				$this->_running_vars['to_parent_pos']++;
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_parent);
			unset($new_parent);

			// if parent and current asset settings are the same then skip the cascades screen
			if ($this->_running_vars['can_skip_cascade']) {
				if (!$this->_canSkipCascadeSteps($assetid, $this->_running_vars['to_parent_assetid'])) {
					$this->_running_vars['can_skip_cascade'] = FALSE;
				}
			}

		} else if ($this->_running_vars['link_action'] == 'move') {

			$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid']);
			if ($link_info['locked'] == '1') {
				trigger_localised_error('HIPO0087', sprintf(translate('Unable to move asset #%s, the link is locked. Please check the settings on the Linking screen'), $assetid), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}//end if

			// Work out whether we should be restricting this link.
			$restricted_links = $this->_running_vars['safe_edit_bc_restricted'];
			$do_not_move      = FALSE;
			if (array_key_exists($assetid, $restricted_links) === TRUE) {
				foreach ($restricted_links[$assetid] as $restricted_link) {
					if ((string) $restricted_link['linkid'] === (string) $value['linkid']) {
						$do_not_move = TRUE;
						break;
					}
				}//end foreach
			}//end if

			// if we are moving to the trash, we want to skip all the steps that ask us if we want
			// to cascade permissions, metadata, and workflow. We also need to get all the children
			// so the lookups can be updated later
			$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
			$moving_to_trash = FALSE;
			$bridge_id = NULL;

			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			if ($this->_running_vars['to_parent_assetid'] == $trash_folder->id) {
				$this->skipStep(3);
				$this->skipStep(4);
				$this->skipStep(5);
				$this->skipStep(6);
				$this->skipStep(7);
				$this->skipStep(8);

				$moving_to_trash = TRUE;

				$id_parts = explode(':', $assetid);
				if (isset($id_parts[1])) $bridge_id = $id_parts[0];

				// If we aren't allowed to trash this, then it's not going
				// anywhere, so there's no point running update lookups over
				// the children
				if (($do_not_move === TRUE) || (isset($this->_running_vars['do_not_trash'][$assetid]) === FALSE)) {
					$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
					$this->_running_vars['assets_children'] = array_merge($this->_running_vars['assets_children'], array_keys($children));
				}

				// delete links in which the minor is being deleted
				$affected_links = array_get_index(array_get_index($this->_running_vars,'affected_links',Array()),$assetid,Array());
				foreach ($affected_links as $link) {
					// we do not delete the link included in the $value array
					// because it will be deleted later on
					if ($link['linkid'] !== $value['linkid'] && !$safe_trash) {
						$success = $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'], FALSE);
					}
				}
			}//end if moving to trash


			// if we are moving a shadow asset to the trash, the bridge is responsible for deletion of links
			// and determining whether the shadow asset itself should be deleted
			// NOTE: a shadow asset CANNOT be linked to the Trash, therefore it (or the link) must be deleted
			if ($moving_to_trash && !is_null($bridge_id)) {

				$link  = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid'], $assetid, 'minor');

				// if the major of this link contains no colon, and is not a bridge, then it's a real asset
				// with a shadow asset linked underneath, so get the asset manager to handle this delete
				if (!empty($link) && (FALSE === strpos($link['majorid'], ':')) && !implements_interface($asset, 'bridge')) {
					// if Safe-Trash is enabled, make sure we can delete this link
					if (!$safe_trash || ($safe_trash && !in_array($assetid, array_keys($this->_running_vars['do_not_trash'])))) {

						if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($value['linkid'], FALSE)) {
							trigger_localised_error('HIPO0019', sprintf(translate('Failed to move asset #%s'), $assetid), E_USER_WARNING);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}

				} else {
					// its a shadow asset linked to a bridge or to another shadow,
					// so get the bridge to handle the delete
					$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($bridge_id);
					if (!$safe_trash || ($safe_trash && !in_array($assetid, array_keys($this->_running_vars['do_not_trash'])))) {

						if (!$bridge->deleteAssetLink($value['linkid'])) {
							trigger_localised_error('HIPO0019', sprintf(translate('Failed to move asset #%s'), $assetid), E_USER_WARNING);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge);
				}
			} else {

				$not_to_trash = !$moving_to_trash;
				$safe_to_move = !$do_not_move;

				if ($moving_to_trash) {
					$safe_to_move = $safe_to_move && (!$safe_trash || ($safe_trash && !in_array($assetid, array_keys($this->_running_vars['do_not_trash']))));
				}

				// If this asset is safe to move, proceed
				if ($safe_to_move === TRUE) {

					$link            = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid'], $value['parentid']);
					$new_parent_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->_running_vars['to_parent_assetid']));

					// disallow multiple significant links between the same parent and child (outside the trash)
					// NOTE: failing this will mean that further links (valid or not) will not be processed, in the same
					// manner as when moving the link fails (see below)
					if ($not_to_trash) {
						if (($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) && $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->_running_vars['to_parent_assetid'], $asset->id, SQ_SC_LINK_SIGNIFICANT)) {
							trigger_localised_error('SYS0329', sprintf(translate('Asset "%1$s" (#%2$s) cannot be linked to "%3$s" (#%4$s), a significant link already exists between these assets'), $asset->name, $asset->id, $new_parent_info[$this->_running_vars['to_parent_assetid']]['name'], $this->_running_vars['to_parent_assetid']), E_USER_WARNING);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}

					// if parent and current asset settings are the same then skip the cascades screen
					if ($this->_running_vars['can_skip_cascade']) {
						if (!$this->_canSkipCascadeSteps($assetid, $this->_running_vars['to_parent_assetid'])) {
							$this->_running_vars['can_skip_cascade'] = FALSE;
						}
					}

					$linkid = $GLOBALS['SQ_SYSTEM']->am->moveLink($value['linkid'], $this->_running_vars['to_parent_assetid'], $link['link_type'], $this->_running_vars['to_parent_pos']);
					if (!$linkid) {
						trigger_localised_error('HIPO0020', sprintf(translate('Unable to move asset with ID #%1$s to parent with asset ID #%2$s'), $assetid, $this->_running_vars['to_parent_assetid']), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
					$asset    = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_link['minorid'], $new_link['minor_type_code']);

					if (is_null($asset)) {
						trigger_localised_error('HIPO0019', sprintf(translate('Failed to move asset #%s'), $assetid), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					$step_data['message'] = sprintf(translate('Moving "%1$s" to "%2$s"'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($new_parent_info[$this->_running_vars['to_parent_assetid']]['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));


					if ($this->_running_vars['to_parent_pos'] != -1) {
						$this->_running_vars['to_parent_pos']++;
					}

				} else {

					// If we are marking for deletion, then do so
					if (($moving_to_trash === TRUE) && ($safe_to_move === FALSE)) {
						$cron_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_attempt_safe_trash');

						if (isset($this->_running_vars['mark_for_deletion'][$asset->id])) {
							$link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, NULL, 'minor');
							if (!$link) {
								// Create the cron job
								$cron_job = new Cron_Job_Attempt_Safe_Trash();
								$cron_job->setAssetToUpdate($asset);

								$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
								$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
								if ($cron_manager->addJob($cron_job, $GLOBALS['SQ_SYSTEM']->user)) {
									$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
								} else {
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								}
								$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_job);
								unset($cron_job);
							}
						}
						$step_data['message'] = sprintf(translate('Creating Safe Trash Scheduled Job'), $asset->id);

					} else {
						$step_data['message'] = sprintf(translate('Skipping asset #%s'), $asset->id);

					}
				}//end else not actually trashing

			}//end else moving to trash

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if create

		if (empty($this->_running_vars['process_create_links'][$assetid])) {
			unset($this->_running_vars['process_create_links'][$assetid]);
			$this->_running_vars['done_links'][] = $assetid;
		}

		if (empty($this->_running_vars['process_create_links'])) {
			unset($this->_running_vars['done_links']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;

			if ($this->_running_vars['can_skip_cascade']) {
				$this->skipStep(3);
				$this->skipStep(4);
				$this->skipStep(5);
				$this->skipStep(6);
				$this->skipStep(7);
				$this->skipStep(8);
			}

		} else {
			$step_data['percent_done'] = ((count($this->_running_vars['done_links']) * 100) / $this->_running_vars['link_count']);
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		unset($asset);

		if (count(array_diff(array_keys($this->_running_vars['assets']), array_keys($this->_running_vars['do_not_trash']))) == 0) {
			if (empty($this->_running_vars['assets_children'])) {
				$this->skipStep(2);
			}
		}

		return TRUE;

	}//end processCreateLinks()


	/**
	* Paint the interface for accepting user input on what to cascade (permissions/metadata/workflow)
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param object	&$o			the backend outputter class
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function paintInterface(&$step_data, &$o, $prefix)
	{
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		if (is_null($hh)) return FALSE;

		$assets = $this->_running_vars['assets'];

		$c = 1;
		for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
			$o->addHiddenField('link_assetid['.$c.']',  $assetid);
			$c++;
		}

		$o->addHiddenField('link_parentid', $this->_running_vars['to_parent_assetid']);

		$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
		if (is_null($parent)) {
			trigger_localised_error('HIPO0018', sprintf(translate('Asset #%d not found'), $this->_running_vars['to_parent_assetid']), E_USER_WARNING);
			return FALSE;
		}

		$o->openSection(translate('Note'));

			$o->openField('&nbsp;');
				$o->addHiddenField('link_completed', '0');

				if (count($this->_running_vars['assets']) > 0) {
					if ($this->_running_vars['link_action'] == 'move') {
						$success_msg = sprintf(translate('The following asset(s) have been successfully moved under "%s":'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

					} else {
						$success_msg = sprintf(translate('The following asset(s) have been successfully linked under "%s":'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

					}
					echo '<p><b>'.$success_msg.'</b></p>';
					echo '<ul>';
					for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid,));
						echo '<li>"'.$asset_info[$assetid]['name'].'"</li>';
					}
					echo '</ul>';
				}

				?>

				<p><?php echo sprintf(translate('The permissions, workflow schemas, and metadata schemas of "%s" are shown below.'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>

				<p><?php echo sprintf(translate('To cascade the permissions, workflow schemas, and/or metadata schemas of "%s" to the newly linked asset(s) and their children, check the appropriate boxes below and press the <b>Next</b> button.'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>

				<p><?php echo translate('If you do not wish to cascade any permissions or schemas, press the <b>Next</b> button without checking any boxes.'); ?></p>

				<?php
				$ei = $parent->getEI();

				$ei->includeStaticScreen('permissions');
				$pe = paintInlinePermissions($parent, $o, $ei);

				$ei->includeStaticScreen('workflow');
				$wf = paintInlineWorkflow($parent, $o, $ei);

				$ei->includeStaticScreen('metadataSchemas');
				$ms = paintInlineMetadataSchemas($parent, $o, $ei);

				$ei->includeStaticScreen('tagging');
				$ta = paintInlineTagging($parent, $o, $ei);

				$ei->includeStaticScreen('roles');
				$ro = paintInlineRoles($parent, $o, $ei);

				$o->addHiddenField('form_submitted', 1);
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('Cascade'));

			$o->openField('', 'cascade');
				echo ($pe) ? translate('Permissions').' '.check_box('cascade[]', 'cascade_pe').'<br />' : '';

				echo ($ms) ? translate('Metadata Schemas').' '.check_box('cascade[]', 'cascade_ms').'<br />' : '';

				echo ($wf) ? translate('Workflow').' '.check_box('cascade[]', 'cascade_wf').'<br />' : '';

				echo ($ta) ? translate('Content Tags').' '.check_box('cascade[]', 'cascade_ta').'<br />' : '';

				echo ($ro) ? translate('Roles').' '.check_box('cascade[]', 'cascade_ro').'<br />' : '';

			$o->closeField();
		$o->closeSection();

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
		unset($parent);

		return TRUE;

	}//end paintInterface()


	/**
	* Process the user's selections to cascade whatever, and skip those steps that they don't want to run
	*
	* @param array	&$step_data	the step data for this job
	* @param string	$prefix		prefix to be used with this job
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$step_data, $prefix)
	{
		// has the next button been clicked ?
		if (isset($_REQUEST['form_submitted'])) {
			if (isset($_REQUEST['cascade'])) {
				// permissions
				if (!in_array('cascade_pe', $_REQUEST['cascade'])) {
					$this->skipStep(4);
				}
				// metadata
				if (!in_array('cascade_ms', $_REQUEST['cascade'])) {
					$this->skipStep(5);
				}
				// workflow
				if (!in_array('cascade_wf', $_REQUEST['cascade'])) {
					$this->skipStep(6);
				}
				// tags
				if (!in_array('cascade_ta', $_REQUEST['cascade'])) {
				$this->skipStep(7);
				}
				// roles
				if (!in_array('cascade_ro', $_REQUEST['cascade'])) {
				$this->skipStep(8);
				}
			} else {
				$this->skipStep(4); // permissions
				$this->skipStep(5); // metadata
				$this->skipStep(6); // workflow
				$this->skipStep(7); // tags
				$this->skipStep(8); // roles
			}
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}//end if

		return TRUE;

	}//end processInterface()


	/**
	* Returns true if current and parent asset's settings are equal
	* checked settings are: permissions, workflow sch, metadata sch, tag links
	*
	* @param string	$assetid	Id of the asset that is being moved
	* @param string	$parentid	Id of the new parent asset
	*
	* @return boolean
	* @access private
	*/
	function _canSkipCascadeSteps($assetid, $parentid)
	{
		// Permissions
		$perms = Array('Read' => SQ_PERMISSION_READ, 'Write' => SQ_PERMISSION_WRITE, 'Administrator' => SQ_PERMISSION_ADMIN);
		foreach ($perms as $title => $perm) {
			$permissions = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($assetid, $perm, NULL, NULL);
			$ppermissions = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($parentid, $perm, NULL, TRUE);
			$diff = array_diff_assoc_recursive($ppermissions, $permissions);
			if (!empty($diff)) return FALSE;
		}

		// Workflow Schemas
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($assetid, NULL, NULL);
		$pschemas = $wfm->getSchemas($parentid, NULL, FALSE, TRUE);
		$diff = array_diff_assoc_recursive($pschemas, $schemas);
		if (!empty($diff)) return FALSE;

		// Metadata Schemas
		// Compare all schemas applied with the cascadable schemas of the parent
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($assetid, NULL, NULL);
		$pschemas = $mm->getSchemas($parentid, NULL, TRUE);
		$diff = array_diff_assoc_recursive($pschemas, $schemas);
		if (!empty($diff)) return FALSE;

		// Tag links
		$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
		$tls = $tag_manager->getTagLinks($assetid);
		$ptls = $tag_manager->getTagLinks($parentid);

		$num_ptls = count($ptls);
		if ($num_ptls != 0) {
			if ($num_ptls == count($tls)) {
				foreach ($ptls as $pkey => $plink_details) {
					$tag_found = FALSE;
					foreach ($tls as $tkey => $tlink_details) {
						if ($plink_details['minorid'] == $tlink_details['minorid']) {
							$tag_found = TRUE;
						}
					}
					if (!$tag_found) return FALSE;
				}
			} else {
				return FALSE;
			}
		}

		// Roles
		$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($assetid);
		$proles = $GLOBALS['SQ_SYSTEM']->am->getRole($parentid);
		$diff = array_diff_assoc_recursive($proles, $roles);
		if (!empty($diff)) return FALSE;

		return TRUE;

	}//end _canSkipCascadeSteps()


	/**
	* This will run the Update Lookups before retrieving user input about cascading permissions etc.
	*
	* Run as Step 1 (before cascading) because this step is critical.
	* We dont want them to bail out before doing this.
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function updateLookups(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_update_lookups';
		$options['auto_complete'] = TRUE;

		$assetids = Array();
		$assets = $this->_running_vars['assets'];
		for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
			if (!isset($this->_running_vars['do_not_trash'][$assetid])) {
				$assetids[] = $assetid;
			}
		}

		if (!empty($this->_running_vars['assets_children'])) {
			$assetids = array_merge($assetids, $this->_running_vars['assets_children']);
		}

		$running_vars['assetids'] = $assetids;

	}//end updateLookups()


	/**
	* Cascade permissions (if not skipped)
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function cascadePermissions(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_permissions';
		$permission_vars = Array();

		foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
			$set_perms = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($this->_running_vars['to_parent_assetid'], $perm, NULL, TRUE);

			foreach ($set_perms as $userid => $perm_data) {
				$permission_vars[] = Array(
										'permission'		=> $perm,
										'granted'			=> $perm_data['granted'],
										'userid'			=> $perm_data['userid'],
										'previous_access'	=> NULL,
										'assetids'			=> array_keys($this->_running_vars['assets']),
										'cascades'			=> TRUE,
									 );
			}
		}

		$running_vars['permission_changes'] = $permission_vars;

	}//end cascadePermissions()


	/**
	* Cascade metadata (if not skipped)
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function cascadeMetadata(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_metadata_schemas';
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($this->_running_vars['to_parent_assetid'], NULL, TRUE);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array(
								'assetids'			=> array_keys($this->_running_vars['assets']),
								'granted'			=> $granted,
								'schemaid'			=> $schemaid,
								'cascades'			=> TRUE,
								'previous_access'	=> NULL,
							 );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

	}//end cascadeMetadata()


	/**
	* Cascade workflow (if not skipped)
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function cascadeWorkflow(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_workflow_schemas';
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($this->_running_vars['to_parent_assetid'], NULL, FALSE, TRUE);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array(
								'assetids'			=> array_keys($this->_running_vars['assets']),
								'granted'			=> $granted,
								'schemaid'			=> $schemaid,
								'cascades'			=> TRUE,
								'previous_access'	=> NULL,
							 );
		}
		$running_vars['schema_changes'] = $schema_vars;

	}//end cascadeWorkflow()


	/**
	* Cascade tags (if not skipped)
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function cascadeTags(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_tags';
		$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
		$current_tag_links = $tag_manager->getTagLinks($this->_running_vars['to_parent_assetid']);
		$tag_vars = Array();

		foreach ($current_tag_links as $link) {
			$tag_vars[] = Array(
							'tag_id'	=> $link['minorid'],
							'action'	=> 'add',
							'weight'	=> $link['value'],
						  );
		}

		$running_vars['tag_changes'] = $tag_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

	}//end cascadeTags()


	/**
	* Cascade tags (if not skipped)
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function cascadeRoles(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_roles';
		$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($this->_running_vars['to_parent_assetid']);
		$roles_vars = Array();

		foreach ($roles as $roleid => $users) {
			$role_vars['add'][$roleid] = $users;
		}
		$running_vars['role_changes'] = $role_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];


	}//end cascadeRoles()


	/**
	* Refreshes the asset map to reflect the new link changes we made
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param object	&$o			the backend outputter class
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function paintRefresh(&$step_data, &$o, $prefix)
	{
		// Only process in the later steps (otherwise it performs a double 'get_assets' request to the asset map)
		if ($step_data['percent_done'] > 0) {
			$refreshids = Array(
						   addslashes($this->_running_vars['to_parent_assetid']),
						  );

			foreach ($this->_running_vars['assets'] as $links) {
				foreach ($links as $link) {
					$refreshids[] = addslashes($link['parentid']);
				}
			}
			$refreshids = array_unique($refreshids);

			if (count($refreshids) > 0) {
				$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_assets) window.opener.parent.frames["sq_sidenav"].reload_assets("'.implode('|', $refreshids).'");');
			}
		}//end if

		return TRUE;

	}//end paintRefresh()


	/**
	* Does nothing
	*
	* @param array	&$step_data	the step data for this job
	* @param string	$prefix		prefix to be used with this job
	*
	* @return boolean
	* @access public
	*/
	function processRefresh(&$step_data, $prefix)
	{
		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}

		return TRUE;

	}//end processRefresh()


	/**
	* Performs the duties of this hipo, without showing any output
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$step_data = Array();
		$this->_running_vars['can_skip_cascade'] = FALSE;

		// process confirmation
		$result = TRUE;
		while ($result && !array_get_index($step_data, 'complete')) {
			$result = $this->processCreateLinks($step_data, '');
		}
		if (!$result) {
			$this->_addError('creating links failed');
			return FALSE;
		}

		// update lookups
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => array_keys($this->_running_vars['assets']));

		$lookup_errors = $hh->freestyleHipo('hipo_job_update_lookups', $vars);
		if (!empty($lookup_errors)) {
			$this->_addError('hipo_job_update_lookups failed');
			return FALSE;
		}

		// process cascading options
		$cascading_list = Array(
							'cascadePermissions'	=> 'cascade_pe',
							'cascadeMetadata'		=> 'cascade_ms',
							'cascadeWorkflow'		=> 'cascade_wf',
							'cascadeTags'			=> 'cascade_ta',
						  );

		foreach ($cascading_list as $fn_name => $value) {
			$job_type = '';
			$running_vars = Array();
			$options = '';

			if (isset($this->_running_vars[$value]) && $this->_running_vars[$value]) {
				$this->$fn_name($job_type, $running_vars, $options);
				$errors = $hh->freestyleHipo($job_type, $running_vars);
			}

			if (!empty($errors)) {
				$this->_addError('error occured during '.$fn_name);
				return FALSE;
			}

		}//end foreach

		return TRUE;

	}//end freestyle()


}//end class

?>
