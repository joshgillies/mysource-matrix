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
* $Id: hipo_job_create_links.inc,v 1.70 2013/07/25 23:23:49 lwright Exp $
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
* @version $Revision: 1.70 $
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

		$this->_running_vars['assets_children'] = Array();

		// an array of assetids that are not supposed to be trashed when Safe-Trash = Yes
		$this->_running_vars['do_not_trash'] = Array();

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
			// Check if the assets have been moved to the trash. If there are any, warn about which links will be affected
			if ($new_parent->id == $trash_folder->id) {
				// if we are moving to the trash folder, we need to check if the Safe-Trash pref has been enabled
				$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');

				// when checking for the number of links left in the system, make sure we disregard
				// any link to the trash folder because even if this asset is already in the trash,
				// we should warn about deleting it
				for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
					$links = current($assets);

					for (reset($links); NULL !== ($link_idx = key($links)); next($links)) {
						$current = current($links);

						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$trash_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_folder->id, $asset->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
						$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2, '', TRUE, $current['linkid']);

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
							$trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($assetid, $current['linkid'], FALSE);
							if ($trash_errors) {
								$this->_running_vars['do_not_trash'][$assetid] = $trash_errors;
								$this->_running_vars['do_not_trash'][$assetid]['name'] = $asset->name;
							}
						}
					}//end for ($links)
				}//end for ($assets)

			}//end if the assets have been moved to the trash
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
			$o->openSection(translate('new_link_confirmation'));
				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$o->openField('');
						echo '<p>'.translate('hipo_confirm_new_links', htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</p>';
						echo '<ul>';
						for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($assets);
							foreach ($current as $link) {
								$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);
								echo '<li>"'.translate('hipo_from_under', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</li>';
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
							}
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						}
						echo '</ul>';
						echo '<p>'.translate('hipo_confirm_abort').'</p>';
				$o->closeField();
			$o->closeSection();

		} else if ($this->_running_vars['link_action'] == 'move') {

			// we are moving an asset
			$o->openSection(translate('hipo_move_confirmation'));

				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">'.strtoupper(translate('error')).'</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				// Check if any assets were re-ordered outside the HIPO job (ie, parentid == to_parent_id)
				// and print them here to avoid confusion
				if (!empty($reordered_assets)) {
					$o->openField('&nbsp;');
							echo '<p>'.translate('hipo_assets_not_require_move', htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</p>';
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

					// We are moving somewhere else
					$o->openField('');
					echo '<p>'.translate('hipo_confirm_move_under', htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</p>';
					echo '<ul>';
					for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$current = current($assets);
						foreach ($current as $link) {
							$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['parentid']);
							echo '<li>'.translate('hipo_from_under', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</li>';
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
						}
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					}
					echo '</ul>';
					$o->closeField();

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
			$not_to_trash = 0;
			if ($safe_trash) {
				$to_trash = count($assets);
				$not_to_trash = count($this->_running_vars['do_not_trash']);
				$nothing_to_trash = ($not_to_trash == $to_trash);
			}

			$o->openField('');

			// Ask them to confirm that they want to trash all these assets
			if (count($assets) - $not_to_trash > 0) {
				echo translate('hipo_confirm_move_trash', count($assets) - $not_to_trash).'</p>';
				if (isset($this->_hipo_vars['no_next']) && $this->_hipo_vars['no_next']) {
					// if no next button printed, modify the confirm message a little
					echo '<p>'.translate('hipo_abort').'</p>';
				} else {
					echo '<p>'.translate('hipo_confirm_abort').'</p>';
				}
			}

			// Print Safe-Trash notices
			if ($safe_trash && $not_to_trash > 0) {
				if (count($assets)== $not_to_trash) {
					?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_all');?></strong></p><?php
				} else {
					?><p><strong class="sq-backend-warning"><?php echo translate('hipo_warning_safe_trash_few', count($assets), $not_to_trash);?></strong></p><?php
				}
			}

			// Warn that shadow assets will not be trashed
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

			$o->closeField();

			// Paint the list of affected assets, with appropriate warning message depending on
			// whether we are Safe-Trashing ('those assets will be affected' / 'you can't delete these assets'
			if ($safe_trash) {
				$affected_title = translate('hipo_safe_trash_linking_title');
				$warning_msg = translate('hipo_safe_trash_linking_msg');
			} else {
				$affected_title = translate('hipo_trash_warning_affected_title');
				$warning_msg = translate('hipo_trash_warning_affected_msg');
			}
			$this->_paintAffectedAssetsWarning($o, $affected_title, $warning_msg);

			if ($safe_trash && !empty($this->_running_vars['do_not_trash'])) {
				// Paint other safe-trash sections
				$this->_paintSafeTrashStatusWarning($o);
				$this->_paintSafeTrashChildrenWarning($o);
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
				echo '<br />'.translate('hipo_trash_warning_asset_linked_to', get_asset_tag_line($assetid), '<u class="clickable" onclick="toggleNextElt(this, \'TABLE\')" title="'.translate('click_to_toggle_details').'">'.translate('n_assets', count($links)).'</u>');
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
					$o->openField(translate('hipo_safe_trash_status_title'), 'new_line');
					?><p><?php echo translate('hipo_safe_trash_status_msg') ?></p><?php
					echo '<ul>';
					$warning_painted = TRUE;
				}
				echo '<li>';
				echo translate('asset_format', $data['name'], $id).' - ';
				echo translate('current_status').': ';
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
					$o->openField(translate('hipo_safe_trash_children_title'), 'new_line');
					echo '<p>'.translate('hipo_safe_trash_children_msg').'</p>';
					echo '<ul>';
					$warning_painted = TRUE;
				}

				$details = '<ul style="display: none">';
				$problem_child_count = 0;
				foreach ($data['children'] as $child_info) {
					$details .= '<li>'.translate('asset_format', $child_info['name'], $child_info['minorid']);
					if ($child_info['link_type'] == SQ_LINK_TYPE_3 || $child_info['link_type'] == SQ_LINK_NOTICE) {
						$details .= ' - '.translate('hipo_warning_safe_trash_link');
						$problem_child_count++;
					} else if ($child_info['status'] >= SQ_STATUS_LIVE) {
						$details .= ' - '.translate('current_status').': '.get_asset_status_icon($child_info['status']).get_status_description($child_info['status']);
						$problem_child_count++;
					}
					$details .= '</li>';
				}
				$details .= '</ul>';

				$code = ($problem_child_count == 1) ? 'hipo_safe_trash_child_label_one' : 'hipo_safe_trash_child_label_many';
				$sub_code = ($problem_child_count == 1) ? '1_child' : 'n_children';
				echo '<li>'.translate($code, translate('asset_format', $data['name'], $id), '<u class="clickable" onclick="toggleNextElt(this, \'UL\')" title="'.translate('click_to_toggle_details').'">'.translate($sub_code, $problem_child_count).'</u>');
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
		$o->openField(translate('hipo_safe_trash_mark_future_title'), 'new_line');

		$assetids = array_keys($this->_running_vars['do_not_trash']);
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);

		?><p>The assets that cannot be trashed due to Safe-Trash rules are listed below.  Selecting assets from this list will mark them for future trashing.  This creates a cron job that will automatically trash the asset as soon as the Safe-Trash rules are satisfied.</p>

		<div id="<?php echo $prefix ?>_mark">
			<?php
			if (count($assetids) > 1) {
				// enable select-all
				?>
				<input type="checkbox" id="<?php echo $prefix ?>_mark_all" onclick="inputs = this.parentNode.getElementsByTagName('INPUT'); for (i = 0; i &lt; inputs.length; i++) { if ((inputs[i].type == 'checkbox') && (inputs[i]!= this)) inputs[i].checked = this.checked; }">
				<label for="<?php echo $prefix ?>_mark_all"><em><?php echo translate('select_all'); ?></em></label>
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
					echo translate('asset_format', $asset_info[$assetid]['name'], $assetid);
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
				$o->openField(translate('hipo_trash_url_remapping_title'), 'new_line');
				echo '<p>'.translate('hipo_trash_url_remapping_msg').'</p>';
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
		echo '<p>'.translate('hipo_trash_url_remapping_choose_url').':</p>';
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
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.translate('hipo_cannot_remap_urls', htmlentities($source_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $remap_url).'</span>';
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
								$this->_tmp['message'] = '<span style="color: #FF0000;">'.translate('hipo_cannot_remap_urls', htmlentities($source_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $dest_url_data['url']).'</span>';
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
								return TRUE;
							}
						}
					break;
					case 0:
						// Cannot remap, the target asset has no URLs
						$remap_asset =$GLOBALS['SQ_SYSTEM']->am->getAsset($dest_assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">'.translate('hipo_cannot_remap_links_without_destination_url', htmlentities($remap_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</span>';
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
				trigger_localised_error('HIPO0023', E_USER_WARNING, $this->_running_vars['to_parent_assetid']);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (is_null($asset)) {
				trigger_localised_error('HIPO0022', E_USER_WARNING, $assetid);
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
					trigger_localised_error('SYS0329', E_USER_WARNING, $asset->name, $asset->id, $to_parent_asset->name, $to_parent_asset->id);
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
				trigger_localised_error('HIPO0021', E_USER_WARNING, htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($new_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
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

			$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid']);
			if ($link_info['locked'] == '1') {
				trigger_localised_error('HIPO0087', E_USER_WARNING, $assetid);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
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
							trigger_localised_error('HIPO0019', E_USER_WARNING, $assetid);
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

					$link            = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid'], $value['parentid']);
					$new_parent_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->_running_vars['to_parent_assetid']));

					// disallow multiple significant links between the same parent and child (outside the trash)
					// NOTE: failing this will mean that further links (valid or not) will not be processed, in the same
					// manner as when moving the link fails (see below)
					if ($not_to_trash) {
						if (($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) && $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->_running_vars['to_parent_assetid'], $asset->id, SQ_SC_LINK_SIGNIFICANT)) {
							trigger_localised_error('SYS0329', E_USER_WARNING, $asset->name, $asset->id, $new_parent_info[$this->_running_vars['to_parent_assetid']]['name'], $this->_running_vars['to_parent_assetid']);
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
						trigger_localised_error('HIPO0020', E_USER_WARNING, $assetid, $this->_running_vars['to_parent_assetid']);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
					$asset    = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_link['minorid'], $new_link['minor_type_code']);

					if (is_null($asset)) {
						trigger_localised_error('HIPO0019', E_USER_WARNING, $assetid);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					$step_data['message'] = translate('hipo_move', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), htmlentities($new_parent_info[$this->_running_vars['to_parent_assetid']]['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

					$this->_running_vars['to_parent_pos']++;

				} else {

					// If we are marking for deletion, then do so
					if (!$safe_to_trash) {
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
						$step_data['message'] = translate('hipo_creating_safe_trash_job', $asset->id);
					} else {
						$step_data['message'] = translate('hipo_skipping_asset', $asset->id);
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
			trigger_localised_error('HIPO0018', E_USER_WARNING, $this->_running_vars['to_parent_assetid']);
			return FALSE;
		}

		$o->openSection(translate('note'));
			$o->openField('&nbsp;');
				$o->addHiddenField('link_completed', '0');

				if (count($this->_running_vars['assets']) > 0) {
					if ($this->_running_vars['link_action'] == 'move') {
						$success_msg = translate('hipo_asset_move_successful', htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
					} else {
						$success_msg = translate('hipo_asset_link_successful', htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
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

				<p><?php echo translate('hipo_permission_workflow_metadata_display', htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>
				<p><?php echo translate('hipo_permission_workflow_metadata_cascade_select', htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>
				<p><?php echo translate('hipo_permission_workflow_metadata_cascade_none'); ?></p>
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
			$permissions = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($assetid, $perm, NULL, NULL);
			$ppermissions = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($parentid, $perm, NULL, TRUE);
			$diff = array_diff_assoc($ppermissions, $permissions);
			if (!empty($diff)) return FALSE;
		}

		// Workflow Schemas
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($assetid, NULL, NULL);
		$pschemas = $wfm->getSchemas($parentid, NULL, TRUE);
		$diff = array_diff_assoc($pschemas, $schemas);
		if (!empty($diff)) return FALSE;

		// Metadata Schemas
		// Compare all schemas applied with the cascadable schemas of the parent
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($assetid, NULL, NULL);
		$pschemas = $mm->getSchemas($parentid, NULL, TRUE);
		$diff = array_diff_assoc($pschemas, $schemas);
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
						   addslashes($this->running_vars['to_parent_assetid']),
						  );

			foreach ($this->_running_vars['assets'] as $links) {
				foreach ($links as $link) {
					$refreshids[] = addslashes($link['parentid']);
				}
			}
			$refreshids = array_unique($refreshids);

			$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_assets) window.opener.parent.frames["sq_sidenav"].reload_assets(["'.implode('","', $refreshids).'"]);');
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
