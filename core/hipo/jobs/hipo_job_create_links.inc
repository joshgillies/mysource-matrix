<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_create_links.inc,v 1.45 2007/01/08 06:36:21 tbarrett Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit asset permissions
*
* <pre>
* Initial Running Vars :
*    int    assetid             -  the assetid with whose children we are going to attempt to lock
*    Array  permission_changes  -  the values that we are trying to set for our children, in the form :
*                                  $changes = Array([perm] => Array (
*                                                                       Array(
*                                                                           'granted'         => $granted,
*                                                                           'userid'          => $userid,
*                                                                           'previous_access' => NULL,
*                                                                      ),
*                                                   ),
* </pre>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.45 $
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
		return translate('hipo_name_create_links');

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
					'name'			=> translate('hipo_confirmation'),
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
					'name'			=> translate('hipo_creating_links'),
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
					'name'			=> translate('hipo_updating_lookups'),
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
					'name'			=> translate('hipo_cascading_options'),
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
					'name'			=> translate('hipo_cascading_permissions'),
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
					'name'			=> translate('hipo_cascading_metadata'),
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
					'name'			=> translate('hipo_cascading_workflow'),
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
					'name'			=> translate('hipo_cascading_content_tags'),
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
					'name'			=> translate('hipo_cascading_roles'),
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
					'name'			=> translate('hipo_refreshing_asset_map'),
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
		// an array of assetids that are not supposed to be trashed when safe trash = Yes
		$this->_running_vars['do_not_trash'] = Array();
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
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0026', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_assetid'])) {
			trigger_localised_error('HIPO0025', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_pos'])) {
			trigger_localised_error('HIPO0024', E_USER_WARNING);
			return '';
		}

		// get a list of links that will be affected by this change
		if ($this->_running_vars['link_action'] == 'move') {
			$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
			$assets = $this->_running_vars['assets'];
			$trash_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			// Check if the assets have been moved to the trash. If there are any, warn about which links will be affected
			if ($new_parent->id == $trash_folder->id) {
				// if we are moving to the trash folder, we need to check if the safe trash pref has been enabled
				$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');

				// when checking for the number of links left in the system, make sure we disregard
				// any link to the trash folder because even if this asset is already in the trash,
				// we should warn about deleting it
				for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
					$current = current($assets);
					$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					$trash_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_folder->id, $asset->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
					$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_LINK_TYPE_1 + SQ_LINK_TYPE_2, '', TRUE, $current['linkid']);

					if (!empty($trash_link)) $num_other_links--;
					if (!$num_other_links) {
						// now lets get all the other links this asset has in the system and
						// display them to the user
						// note that we are also going to ask the major asset in the link
						// to describe it so it makes sense to the user looking at it
						$affected_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3 + SQ_LINK_NOTICE, '', TRUE, 'minor');
						$safe_trash_cron_job = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, 'minor');

						// We can't use array_diff() because it does a string comparison
						// ("Array" == "Array".... ooh nasty). So we will trawl through the
						// array to find the cron job and pluck it out
						foreach (array_keys($affected_links) as $affected_link_key) {
							if ($affected_links[$affected_link_key]['linkid'] == $safe_trash_cron_job[0]['linkid']) {
								unset($affected_links[$affected_link_key]);
								break;
							}
						}

						if (!empty($affected_links)) {
							$this->_running_vars['affected_links'][$assetid] = $affected_links;
						}

					}//end if !$num_other_links

					// if we can't safe trash for any reason whatsoever, then
					// let them know
					if ($safe_trash) {
						$trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($assetid, $current['linkid'], FALSE);
						if ($trash_errors) {
							$this->_running_vars['do_not_trash'][$assetid] = $trash_errors;
							$this->_running_vars['do_not_trash'][$assetid]['name'] = $asset->name;
						}
					}
				}//end for

			}//end if the assets have been moved to the trash
		}

		$this->_running_vars['can_skip_cascade'] = TRUE;
		$this->_running_vars['assets_children'] = Array();

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
		$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
		$assets = $this->_running_vars['assets'];
		if (isset($this->_running_vars['reordered_assets'])) {
			$reordered_assets = $this->_running_vars['reordered_assets'];
		}

		if ($this->_running_vars['link_action'] == 'create') {
			// we are creating a new link
			$o->openSection(translate('new_link_confirmation'));
				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$o->openField('&nbsp;');
						echo '<p>'.translate('hipo_confirm_new_links', $new_parent->name).'</p>';
						echo '<ul>';
						for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
							$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($assets);
							$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current['parentid']);
							echo '<li>"'.translate('hipo_from_under', $asset->name, $parent->name).'</li>';
						}
						echo '</ul>';
						echo '<p>'.translate('hipo_confirm_abort').'</p>';
				$o->closeField();
			$o->closeSection();

		} else if ($this->_running_vars['link_action'] == 'move') {

			// we are moving an asset
			$o->openSection(translate('hipo_move_confirmation'));

				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">'.translate('error').'</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$trash_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

				// Check if any assets were re-ordered outside the HIPO job (ie, parentid == to_parent_id)
				// and print them here to avoid confusion
				if (!empty($reordered_assets)) {
					$o->openField('&nbsp;');
							echo '<p>'.translate('hipo_assets_not_require_move', $new_parent->name).'</p>';
							echo '<ul>';
							for (reset($reordered_assets); NULL !== ($assetid = key($reordered_assets)); next($reordered_assets)) {
								$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								echo '<li>"'.$asset->name.'".</li>';
							}
							echo '</ul>';
					$o->closeField();
				}

				// if we are moving to the trash folder, we need to check if the safe trash pref has been enabled
				$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');

				// work out how many assets do we really have to trash
				// If we have nothing to trash, we no longer take out the "next"
				// button because of the possibility for marking for safe trash
				$nothing_to_trash = FALSE;
				$not_to_trash = 0;
				if ($safe_trash) {
					$to_trash = count($assets);
					$not_to_trash = count($this->_running_vars['do_not_trash']);
					$nothing_to_trash = ($not_to_trash == $to_trash);
				}

				$o->openField('&nbsp;');
					if ($new_parent->id == $trash_folder->id) {

						// confirm that they are trashing x assets
						if (count($assets) - $not_to_trash > 0) {
							?><p><?php echo translate('hipo_confirm_move_trash', count($assets) - $not_to_trash);?></p><?php
						}

						if ($safe_trash && $not_to_trash > 0) {
							if (count($assets)== $not_to_trash) {
								?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_all');?></strong></p><?php
							} else {
								?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_few', count($assets), $not_to_trash);?></strong></p><?php
							}
						}

						$shadow_assets = Array();
						foreach ($assets as $one_id => $one_info) {
							if (strpos($one_id,':')) $shadow_assets[] = $one_id;
						}
						if (!empty($shadow_assets)) {
							$shadow_info_array = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($shadow_assets);
							echo '<p><strong class="sq-backend-warning">'.translate('hipo_immediate_delete_shadow').':</strong>';
							echo '<ul>';
							foreach ($shadow_info_array as $shadow_info) {
								echo '<li>'.$shadow_info['short_name'].'</li>';
							}
							echo '</ul></p>';
						}

					} else {
						echo '<p>'.translate('hipo_confirm_move_under', $new_parent->name).'</p>';
						echo '<ul>';
						for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
							$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($assets);
							$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current['parentid']);
							echo '<li>"'.translate('hipo_from_under', $asset->name, $parent->name).'</li>';
						}
					}
					echo '</ul>';

					if (isset($this->_hipo_vars['no_next']) && $this->_hipo_vars['no_next']) {
						// if no next button printed, modify the confirm message a little
							echo '<p>'.translate('hipo_abort').'</p>';
					} else {
						echo '<p>'.translate('hipo_confirm_abort').'</p>';
					}

					if ($new_parent->id == $trash_folder->id) {
						if (!$safe_trash || ($safe_trash && !$nothing_to_trash)) {
							// do not paint this section if we have nothing to trash
							?><p><?php echo translate('hipo_trash_warn_break_urls'); ?></p><?php
							asset_finder($prefix.'_remap_new_url', NULL, Array(), 'sq_sidenav', TRUE);
						}
					}
				$o->closeField();


				// Check if the assets have been moved to the trash. If there are any, warn about which links will be affected
				if ($new_parent->id == $trash_folder->id) {
					// when checking for the number of links left in the system, make sure we disregard
					// any link to the trash folder because even if this asset is already in the trash,
					// we should warn about deleting it

					$trash_section_painted = FALSE;

					$affected_links = array_get_index($this->_running_vars,'affected_links',Array());
					foreach ($affected_links as $assetid => $links) {
						$trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($assetid);
						bam($trash_errors);

						$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						// now lets get all the other links this asset has in the system and
						// display them to the user
						// note that we are also going to ask the major asset in the link
						// to describe it so it makes sense to the user looking at it

						if (!$trash_section_painted) {
							$o->openSection(translate('warning_-_moving_to_trash'));
								$o->openField('&nbsp;');
									if ($safe_trash) {
										?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_notice_link') ?></strong></p><?php
									} else {
										?><p><?php echo translate('hipo_warning_trashing_affects_other_assets'); ?></p><?php
									}
									$trash_section_painted = TRUE;
						}

						echo $asset->name." (Id: #$asset->id)";

						$link_lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($asset->id);

						?><table class="sq-backend-table"><?php
						foreach ($links as $link_data) {
							$major =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['majorid'], $link_data['major_type_code']);
							$link_description = $major->describeLink($link_data['linkid']);
							$link_lineage_data = array_pop($link_lineages);
							$link_lineage = $link_lineage_data['lineage'];

							// construct a human readable lineage
							$num_in_lineage = count($link_lineage);
							$lineage = '';

							$i = 0;
							foreach ($link_lineage as $parentid => $name) {
								if ($i == ($num_in_lineage - 1)) {
									continue;
								} else if ($i <= 1 || $num_in_lineage <= 5) {
									$lineage .= $name.' &gt; ';
								} else if ($num_in_lineage - $i <= 3 && $num_in_lineage > 5) {
									$lineage .= $name.' &gt; ';
								} else if ($i == 2 && $num_in_lineage > 5) {
									$lineage .= '... &gt; ';
								}
								$i++;
							}

							$lineage .= '<b>'.array_pop($link_lineage).'</b>';

							?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php echo $lineage; ?>
									<?php echo (!empty($link_description)) ? '<br/>'.$link_description : ''; ?>
								</td>
							</tr>
							<?php
						}//end foreach
						?></table><?php

					}//end foreach

					// paint the not safe status
					if ($safe_trash && !empty($this->_running_vars['do_not_trash'])) {
						$warning_painted = FALSE;
						foreach($this->_running_vars['do_not_trash'] as $id => $data) {
							if (isset($data['status'])) {
								if (!$trash_section_painted) {
									$o->openSection(translate('warning_-_moving_to_trash'));
										$o->openField('&nbsp;');
										$trash_section_painted = TRUE;
								}
								if (!$warning_painted) {
									?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_status') ?></strong></p><?php
									$warning_painted = TRUE;
								}
								echo "$data[name] (Id: #$id), Current Status: ".get_asset_status_icon($data['status']).get_status_description($data['status']).'<br>';
							}
						}
						echo '</p>';
					}

					// paint the not safe children section
					if ($safe_trash && !empty($this->_running_vars['do_not_trash'])) {
						$warning_painted = FALSE;
						foreach($this->_running_vars['do_not_trash'] as $id => $data) {
							if (isset($data['children'])) {
								if (!$trash_section_painted) {
									$o->openSection(translate('warning_-_moving_to_trash'));
										$o->openField('&nbsp;');
										$trash_section_painted = TRUE;
								}
								if (!$warning_painted) {
									?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_children') ?></strong></p><?php
									echo '<ul>';
									$warning_painted = TRUE;
								}
								echo '<li>'.$data['name']." (Id: #$id)";
								echo '<ul>';
								foreach ($data['children'] as $child_info) {
									echo '<li>'.$child_info['name'].' (Id: #'.$child_info['minorid'].')';
									if ($child_info['link_type'] == SQ_LINK_TYPE_3 || $child_info['link_type'] == SQ_LINK_NOTICE) {
										echo '<br> &nbsp; '.translate('hipo_warning_safe_trash_link');
									} else if ($child_info['status'] >= SQ_STATUS_LIVE) {
										echo '<br> &nbsp; Current Status: '.get_asset_status_icon($child_info['status']).get_status_description($child_info['status']);

									}
									echo '</li>';
								}
								echo '</ul>';
								echo '</li>';
							}
						}
						if ($warning_painted) echo '</ul>';
						echo '</p>';
					}

					if ($trash_section_painted) {
						$o->closeField();
						$o->closeSection();
					}

					// paint the mark for safe trash
					if ($safe_trash && !empty($this->_running_vars['do_not_trash'])) {
						$o->openSection('Mark Assets for Deletion');
						$o->openField('');

						$assetids = array_keys($this->_running_vars['do_not_trash']);
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);

						?><p>Here is a summary of assets that cannot be trashed due to Safe Trash rules. You can mark these assets for deletion - this creates a Cron Job that will attempt to trash the asset on each run, if and when the above Safe Trash conditions are satisfied.</p>

						<div id="<?php echo $prefix ?>_mark">
						<?php if (count($assetids) > 1) {
							?><input type="checkbox" id="<?php echo $prefix ?>_mark_all" onclick="inputs = this.parentNode.getElementsByTagName('INPUT'); for (i = 0; i &lt; inputs.length; i++) { if ((inputs[i].type == 'checkbox') && (inputs[i]!= this)) inputs[i].checked = this.checked; }"> <em><?php echo translate('select_all'); ?></em>
							<hr />
							<?php } ?>
						<?php

						foreach ($assetids as $assetid) {
						?>
						<div><input type="checkbox" name="<?php echo $prefix ?>_mark[<?php echo $assetid ?>]" id="<?php echo $prefix ?>_mark_<?php echo $assetid ?>" value="1" <?php
						if (count($assetids) > 1) {
							?> onclick="selall = document.getElementById('<?php echo $prefix ?>_mark_all'); inputs = this.parentNode.parentNode.getElementsByTagName('INPUT'); sel_value = true; for (i = 0; i &lt; inputs.length; i++) { if ((inputs[i].type == 'checkbox') && (inputs[i] != selall)) { sel_value = sel_value && inputs[i].checked; } if (!sel_value) break; } selall.checked = sel_value;">
							<?php
						}
						?>
							<?php echo get_asset_type_icon($asset_info[$assetid]['type_code']); ?>
							<?php echo $asset_info[$assetid]['name']; ?> (Id: #<?php echo $assetid; ?>)</div>
						<?php
						}
						?>
						</div>
						<?php
						$o->closeField();
						$o->closeSection();
					}

				}//end if the assets have been moved to the trash
				$o->closeSection();
			}//end if action is 'move'

		$o->addHiddenField('form_submitted', 1);
		return TRUE;

	}//end paintConfirmation()


	/**
	* Process confirmation screen, setting up remaps and "mark for safe trash"
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
		// if we are moving to the trash and they want to remap the URL's of the asset that
		// is being trashed to another asset, go through and do all this for them

		if (isset($_POST['form_submitted']) && isset($_POST[$prefix.'_remap_new_url']['assetid'])) {
			$assets_to_remap = $this->_running_vars['assets'];
			foreach ($assets_to_remap as $source_assetid => $remap_info) {
				$dest_assetid = $_POST[$prefix.'_remap_new_url']['assetid'];
				if ($dest_assetid) {
					$dest_url = $_POST[$prefix.'_remap_new_url']['url'];

					if ($dest_assetid == $source_assetid) {
						$source_asset =&$GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.translate('hipo_cannot_remap_links_to_self', $source_asset->name).'</span>';
						return TRUE;
					}

					if (empty($dest_url)) {
						$source_asset =&$GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);
						$remap_asset =&$GLOBALS['SQ_SYSTEM']->am->getAsset($dest_assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.translate('hipo_cannot_remap_links_without_destination_url', $source_asset->name, $remap_asset->name).'</span>';
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
						return TRUE;
					}

					$rm =&$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
					if (!$rm->remapAllURLs($source_assetid, $dest_url)) {
						$source_asset =&$GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.translate('hipo_cannot_remap_urls', $source_asset->name, $dest_url).'</span>';
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
						return TRUE;
					}
				}
			}
		}//end if HIPO form submitted

		if (isset($_POST['form_submitted']) && isset($_POST[$prefix.'_mark'])) {

			$this->_running_vars['mark_for_deletion'] = $_POST[$prefix.'_mark'];


		}

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

		$assets = $this->_running_vars['process_create_links'];
		$assetid = key($assets);
		$value = reset($assets);

		unset($this->_running_vars['process_create_links'][$assetid]);

		if ($this->_running_vars['link_action'] == 'create') {

			$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

			if (is_null($new_parent)) {
				trigger_localised_error('HIPO0023', E_USER_WARNING, $this->_running_vars['to_parent_assetid']);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (is_null($asset)) {
				trigger_localised_error('HIPO0022', E_USER_WARNING, $assetid);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$step_data['message'] = 'Creating a link between "'.$asset->name.'" and "'.$new_parent->name.'".';

			$linkid = $new_parent->createLink($asset, $value['link_type'], '', $this->_running_vars['to_parent_pos']);
			if (!$linkid) {
				trigger_localised_error('HIPO0021', E_USER_WARNING, $asset->name, $new_parent->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$this->_running_vars['to_parent_pos']++;
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

			// if we are moving to the trash, we want to skip all the steps that ask us if we want
			// to cascade permissions, metadata, and workflow. We also need to get all the children
			// so the lookups can be updated later
			$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
			$moving_to_trash = FALSE;
			$bridge_id = NULL;

			$trash_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
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

				$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);

				// If we aren't allowed to trash this, then it's not going
				// anywhere, so there's no point running update lookups over
				// the children
				if (!isset($this->_running_vars['do_not_trash'][$assetid])) {
						$this->_running_vars['assets_children'] = array_merge($this->_running_vars['assets_children'], array_keys($children));
				}

				// delete links in which the minor is being deleted
				$affected_links = array_get_index(array_get_index($this->_running_vars,'affected_links',Array()),$assetid,Array());
				foreach ($affected_links as $link) {
					// we do not delete the link included in the $value array
					// because it will be deleted later on
					if ($link['linkid'] !== $value['linkid'] && !$safe_trash) {
						$success = $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);
					}
				}
			}

			// if we are moving a shadow asset to the trash, the bridge is responsible for deletion of links
			// and determining whether the shadow asset itself should be deleted
			// NOTE: a shadow asset CANNOT be linked to the Trash, therefore it (or the link) must be deleted
			if ($moving_to_trash && !is_null($bridge_id)) {

				$link  = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid'], $assetid, 'minor');
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid']);

				// if the major of this link contains no colon, and is not a bridge, then it's a real asset
				// with a shadow asset linked underneath, so get the asset manager to handle this delete
				if (!empty($link) && (FALSE === strpos($link['majorid'], ':')) && !implements_interface($asset, 'bridge')) {
					// if safe trash is enabled, make sure we can delete this link
					if (!$safe_trash || ($safe_trash && !in_array($assetid, array_keys($this->_running_vars['do_not_trash'])))) {

						if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($value['linkid'], FALSE)) {
							trigger_localised_error('HIPO0019', E_USER_WARNING, $assetid);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}

				} else {
					// its a shadow asset linked to a bridge or to another shadow,
					// so get the bridge to handle the delete
					$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridge_id);
					if (!$safe_trash || ($safe_trash && !in_array($assetid, array_keys($this->_running_vars['do_not_trash'])))) {

						if (!$bridge->deleteAssetLink($value['linkid'])) {
							trigger_localised_error('HIPO0019', E_USER_WARNING, $assetid);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge);
				}
			} else {

				$not_to_trash = !$moving_to_trash;
				$safe_to_trash = $moving_to_trash && (!$safe_trash || ($safe_trash && !in_array($assetid, array_keys($this->_running_vars['do_not_trash']))));

				// if we are not moving to trash, or this asset is safe to trash, proceed
				if ($not_to_trash || $safe_to_trash) {
					// if parent and current asset settings are the same then skip the cascades screen
					if ($this->_running_vars['can_skip_cascade']) {
						if (!$this->_canSkipCascadeSteps($assetid, $this->_running_vars['to_parent_assetid'])) {
							$this->_running_vars['can_skip_cascade'] = FALSE;
						}
					}

					$link   = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid'], $value['parentid']);
					$linkid = $GLOBALS['SQ_SYSTEM']->am->moveLink($value['linkid'], $this->_running_vars['to_parent_assetid'], $link['link_type'], $this->_running_vars['to_parent_pos']);
					if (!$linkid) {
						trigger_localised_error('HIPO0020', E_USER_WARNING, $assetid, $this->_running_vars['to_parent_assetid']);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
					$asset    =& $GLOBALS['SQ_SYSTEM']->am->getAsset($new_link['minorid'], $new_link['minor_type_code']);

					if (is_null($asset)) {
						trigger_localised_error('HIPO0019', E_USER_WARNING, $assetid);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
					$new_parent_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->_running_vars['to_parent_assetid']));
					$step_data['message'] = translate('hipo_move', $asset->name, $new_parent_info[$this->_running_vars['to_parent_assetid']]['name']);

					$this->_running_vars['to_parent_pos']++;
				} else {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					// If we are marking for deletion, then do so
					if (!$safe_to_trash) {
						$cron_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_attempt_safe_trash');

						if (isset($this->_running_vars['mark_for_deletion'][$asset->id])) {
							$link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, NULL, 'minor');
							if (!$link) {
								// Create the cron job
								$cron_job =& new Cron_Job_Attempt_Safe_Trash();
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
						$step_data['message'] = translate('hipo_creating_safe_trash_job', $asset->id);
					} else {
						$step_data['message'] = translate('hipo_skipping_asset', $asset->id);
					}
				}

			}//end else moving to trash

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if create

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['process_create_links'])) {
			unset($this->_running_vars['done_assetids']);
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
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
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
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		if (is_null($hh)) return FALSE;

		$assets = $this->_running_vars['assets'];

		$c = 1;
		for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
			$o->addHiddenField('link_assetid['.$c.']',  $assetid);
			$c++;
		}

		$o->addHiddenField('link_parentid', $this->_running_vars['to_parent_assetid']);

		$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
		if (is_null($parent)) {
			trigger_localised_error('HIPO0018', E_USER_WARNING, $this->_running_vars['to_parent_assetid']);
			return FALSE;
		}

		$o->openSection(translate('note'));
			$o->openField('&nbsp;');
				$o->addHiddenField('link_completed', '0');

				if (count($this->_running_vars['assets']) > 0) {
					if ($this->_running_vars['link_action'] == 'move') {
						$success_msg = translate('hipo_asset_move_successful', $parent->name);
					} else {
						$success_msg = translate('hipo_asset_link_successful', $parent->name);
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

				<p><?php echo translate('hipo_permission_workflow_metadata_display', $parent->name); ?></p>
				<p><?php echo translate('hipo_permission_workflow_metadata_cascade_select', $parent->name); ?></p>
				<p><?php echo translate('hipo_permission_workflow_metadata_cascade_none'); ?></p>
				<?php
				$ei =& $parent->getEI();

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

		$o->openSection(translate('cascade'));
			$o->openField('', 'cascade');
				echo ($pe) ? translate('permissions').' '.check_box('cascade[]', 'cascade_pe').'<br />' : '';
				echo ($ms) ? translate('metadata_schemas').' '.check_box('cascade[]', 'cascade_ms').'<br />' : '';
				echo ($wf) ? translate('workflow').' '.check_box('cascade[]', 'cascade_wf').'<br />' : '';
				echo ($ta) ? translate('content_tags').' '.check_box('cascade[]', 'cascade_ta').'<br />' : '';
				echo ($ro) ? translate('roles').' '.check_box('cascade[]', 'cascade_ro').'<br />' : '';
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
			$permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, $perm, NULL, FALSE, FALSE, TRUE, TRUE);
			$ppermissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($parentid, $perm, NULL, FALSE, FALSE, TRUE, TRUE);
			$diff = array_diff_assoc($ppermissions, $permissions);
			if (!empty($diff)) return FALSE;
		}

		// Workflow Schemas
		$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($assetid);
		$pschemas = $wfm->getSchemas($parentid);
		$diff = array_diff_assoc($pschemas, $schemas);
		if (!empty($diff)) return FALSE;

		// Metadata Schemas
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($assetid);
		$pschemas = $mm->getSchemas($parentid);
		$diff = array_diff_assoc($pschemas, $schemas);
		if (!empty($diff)) return FALSE;

		// Tag links
		$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
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
		$diff = array_diff_assoc($proles, $roles);
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
			$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->_running_vars['to_parent_assetid'], $perm, NULL, FALSE, FALSE, TRUE, TRUE);

			foreach ($set_perms as $userid => $granted) {
				$permission_vars[$perm][] = Array(
												'granted'			=> $granted,
												'userid'			=> $userid,
												'previous_access'	=> NULL,
											);
			}
		}

		$running_vars['permission_changes'] = $permission_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

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
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($this->_running_vars['to_parent_assetid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array(
								'granted'			=> $granted,
								'schemaid'			=> $schemaid,
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
		$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($this->_running_vars['to_parent_assetid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array(
								'granted'			=> $granted,
								'schemaid'			=> $schemaid,
								'previous_access'	=> NULL,
							 );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

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
		$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
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
		$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$this->_running_vars['to_parent_assetid'].'");');

		$refreshids = Array();

		foreach ($this->_running_vars['assets'] as $asset_details) {
			$refreshids[] = $asset_details['parentid'];
		}
		$refreshids = array_unique($refreshids);

		foreach ($refreshids as $refreshid) {
			$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$refreshid.'");');
		}

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
		if (!$this->processCreateLinks($step_data, '')) {
			$this->_addError('creating links failed');
			return FALSE;
		}

		// update lookups
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array(key($this->_running_vars['assets'])));
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
