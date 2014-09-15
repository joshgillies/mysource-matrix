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
* $Id: hipo_job_clone_assets.inc,v 1.42 2013/10/22 00:38:23 lwright Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clone assets
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.42 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Clone_Assets extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Clone_Assets($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('clone_asset_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
				text_box($class.'[SQ_DUPE_ASSET_THRESHOLD]',SQ_DUPE_ASSET_THRESHOLD, 1);
		} else {
				echo SQ_DUPE_ASSET_THRESHOLD;
		}
		echo ' '.translate('assets');
		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_DUPE_ASSET_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Update Lookups HIPO appends the ID of the asset that the lookups are being updated at
	* (the top most parent) so that any other update lookup jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
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
		return translate('hipo_name_clone_assets');

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
					'name'			=> translate('hipo_clone_options'),
					'function_call'	=> Array(
										'paint_function'	=> 'paintCloneOptions',
										'process_function'	=> 'processCloneOptions',
									   ),
					'running_mode'	=> 'web',
					'skip_step'		=> FALSE,
					'auto_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_acquiring_locks'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getLockAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_cloning_assets'),
					'function_call'	=> Array(
										'process_function'	=> 'processCloneAssets',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_remapping_assets'),
					'function_call'	=> Array(
										'process_function'	=> 'processRemap',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_releasing_locks'),
					'function_call'	=> Array(
										'process_function'	=> 'releaseLocks',
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
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run
	* in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_DUPE_ASSET_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / SQ_DUPE_ASSET_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$step_data = Array();

		// skip the confirmation screen, go straight into locking
		$job_type = '';
		$running_vars = Array();
		$options = '';
		$this->getLockAssetDetails($job_type, $running_vars, $options);
		$errors = $hh->freestyleHipo($job_type, $running_vars);
		if (!empty($errors)) {
			$this->_addError('acquiring locks failed');
			return FALSE;
		}

		$success = TRUE;

		// clone assets
		if ($success && !$this->processCloneAssets($step_data, '')) {
			$this->_addError('clone assets failed');
			$success = FALSE;
		}

		// handle remaps
		if ($success && !$this->processRemap($step_data, '')) {
			$this->_addError('remap failed');
			$success = FALSE;
		}


		// unlock assets - release locks regardless of previous non-lock-related failures
		if (!$this->releaseLocks($step_data, '')) {
			$this->_addError('releasing locks failed');
			return FALSE;
		}

		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// there must be assets in the _running_vars to clone
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0031', E_USER_WARNING);
			return FALSE;
		}

		foreach ($this->_running_vars['assets'] as $assetid => $info) {
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				trigger_localised_error('HIPO0084', E_USER_WARNING, $assetid);
				return FALSE;
			}
		}

		$this->_running_vars['remap'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string	&$job_type		the hipo job type
	* @param array	&$running_vars	array contains the information for the hipo to run
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('all'),
							'forceably_acquire'	=> FALSE,
							'dependants_only'	=> TRUE,
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks that were acquired while cloning assets
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['re_lock_assets'])) {
			$this->_running_vars['re_lock_assets'] = $this->_running_vars['assets'];
		}

		$assetid = key($this->_running_vars['re_lock_assets']);
		unset($this->_running_vars['re_lock_assets'][$assetid]);

		$children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_data) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'all');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = sprintf(translate('hipo_release_locks'), $asset_info[$assetid]['name']).translate('hipo_release_locks_dependents');

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end releaseLocks()


	/**
	* Paints some option for the user to specify what assets are to be cloned
	*
	* All dependant asset will be automatically cloned, so no interface will be
	* supplied for duping dependant assets which should save some space.
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param object	&$o			the hipo backend outputter object
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function paintCloneOptions(&$step_data, &$o, $prefix)
	{
		$to_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

		$o->openSection(translate('cloning_confirmation'));
			$o->openField(translate('confirm'));
				$o->addHiddenField('form_submitted', 1);
				?>
				<p><?php echo sprintf(translate('hipo_cloning_confirm'), htmlentities($to_parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>
				<ul>
					<?php
					$assets = $this->_running_vars['assets'];
					for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
						?><li><strong>"<?php echo htmlentities($asset_info[$assetid]['short_name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>"</strong></li><?php
					}
					?>
				</ul>
				</p>
				<p><!-- Show the change name dialog -->
				<script type="text/javascript">
				<!--
					function toggleChangeName() {
						toggleBox = document.getElementById('<?php echo $prefix; ?>_change_name');
						renamedialog = document.getElementById('<?php echo $prefix; ?>_rename_assets');
						if (toggleBox.checked == true) {
							renamedialog.style.visibility = "visible";
							renamedialog.style.display = "block";
						} else {
							renamedialog.style.visibility = "hidden";
							renamedialog.style.display = "none";
						}
					}
				-->
				</script>
				<input type="checkbox" id="<?php echo $prefix; ?>_change_name" name="<?php echo $prefix; ?>_change_name" onclick="toggleChangeName();">
				<label for="<?php echo $prefix; ?>_change_name">(Optionally) Change the name of the cloned assets</label>
				<div id="<?php echo $prefix; ?>_rename_assets" style="display:none;visibility:hidden;">
				<ul><table>
					<?php
					require_once(SQ_LIB_PATH.'/html_form/html_form.inc');
					$assets = $this->_running_vars['assets'];
					for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
					?>
					<tr>
						<td><li style="margin: 0;">Change from</li></td>
						<td><strong>"<?php echo htmlentities($asset_info[$assetid]['short_name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>"</strong></td>
						<td> to </td>
						<td><?php text_box($prefix.'_new_names['.$assetid.']', $asset_info[$assetid]['short_name'], '25'); ?></td>
					</tr>
					<?php
					}//end for
					?>
				</table>
				</ul>
				</div>
				</p>
				<p><?php echo translate('hipo_confirm_abort');?></p>
				<?php
			$o->closeField();
		$o->closeSection();

	}//end paintCloneOptions()


	/**
	* Processes the name changes if requested
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processCloneOptions(&$step_data, $prefix)
	{
		if (isset($_POST['form_submitted'])) {
			// Shortcut, if name change ticked change the names
			if (isset($_POST[$prefix.'_change_name']) && $_POST[$prefix.'_change_name']) {
				foreach ($_POST[$prefix.'_new_names'] as $assetid => $name) {
					if (!empty($name)) {
						$this->_running_vars['assets'][$assetid]['new_name'] = $name;
					}
				}
			}//end if

			// The step is complete after the user does the confirmation
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;

		}//end if

		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		}

		return TRUE;

	}//end processCloneOptions()


	/**
	* Processes the assets that need to be cloned, and clones them
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/
	function processCloneAssets(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['clone_assets'])) {
			$this->_running_vars['clone_assets'] = $this->_running_vars['assets'];
			$this->_running_vars['done_asset_count'] = 0;
			$this->_running_vars['todo_asset_count'] = 0;
			foreach ($this->_running_vars['clone_assets'] as $info) {
				$this->_running_vars['todo_asset_count'] += array_get_index($info, 'number_of_clones', 1);
			}
			reset($this->_running_vars['clone_assets']);
		}

		$assetid = key($this->_running_vars['clone_assets']);

		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

		if (is_null($parent_asset)) {
			trigger_localised_error('HIPO0030', E_USER_WARNING, $this->_running_vars['to_parent_assetid']);
			return FALSE;
		}

		$orig = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		if (is_null($orig)) {
			trigger_localised_error('HIPO0029', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$link = Array(
					'asset'			=> $parent_asset,
					'sort_order'	=> $this->_running_vars['to_parent_pos'],
					'value'			=> '',
				);
		if (!isset($this->_running_vars['link_type'])) {
			$orig_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->_running_vars['clone_assets'][$assetid]['parentid'], $orig->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);
			$link['link_type'] = $orig_link['link_type'];
		} else {
			$link['link_type'] = $this->_running_vars['link_type'];
		}

		if (!isset($this->_running_vars['value'])) {
			if (empty($orig_link)) {
				$orig_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->_running_vars['clone_assets'][$assetid]['parentid'], $orig->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);
			}//end if
			$link['value'] = $orig_link['value'];
		} else {
			$link['value'] = $this->_running_vars['value'];
		}

		if ($this->_running_vars['to_parent_pos'] != -1) {
			$this->_running_vars['to_parent_pos']++;
		}

		// NOTE: Content tags are not cloned as a component because the link
		// cloning will take care of that
		$components = Array(
						'attributes',
						'metadata_schemas',
						'metadata',
						'workflow',
						'permissions',
						'roles',
					  );

		$step_data['message'] = 'Cloning "'.htmlentities($orig->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'" to "'.htmlentities($parent_asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'".';

		$clone = $GLOBALS['SQ_SYSTEM']->am->cloneAsset($orig, $link, $remap, $components, TRUE);
		if (is_null($clone)) {
			trigger_localised_error('HIPO0028', E_USER_WARNING);
			return FALSE;
		}

		// Rename the cloned asset if specified
		if (isset($this->_running_vars['clone_assets'][$assetid]['new_name']) && !empty($this->_running_vars['clone_assets'][$assetid]['new_name'])) {
			if (isset($clone->vars['name'])) {
				$clone->setAttrValue('name', $this->_running_vars['clone_assets'][$assetid]['new_name']);
			}
			if (isset($clone->vars['short_name'])) {
				$clone->setAttrValue('short_name', $this->_running_vars['clone_assets'][$assetid]['new_name']);
			}
			if (isset($clone->vars['id_name'])) {
				$clone->setAttrValue('id_name', $this->_running_vars['clone_assets'][$assetid]['new_name']);
			}
			if (!$clone->saveAttributes()) {
				trigger_localised_error('HIPO0028', E_USER_WARNING);
				return FALSE;
			}
		}

		// the web path of this asset may confict with the children
		// of the parent asset that we are linking to, so check the integrity
		// of the path before using it
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if (isset($this->_running_vars['clone_assets'][$assetid]['new_name']) && !empty($this->_running_vars['clone_assets'][$assetid]['new_name'])) {
			if(isset($clone->vars['id_name'])){
				$valid_paths = make_valid_web_paths(Array(strtolower($clone->attr('id_name'))));
			}
			else {
				$valid_paths = make_valid_web_paths(Array(strtolower($clone->attr('name'))));
			}
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $clone->id, TRUE);
		} else {
			$valid_paths = make_valid_web_paths($orig->getWebPaths());
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $clone->id, TRUE);
		}

		$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($clone->id, $this->_running_vars['to_parent_assetid'], $link['link_type'], NULL, 'minor');

		$this->_running_vars['cloned_asset'][$clone->id]['linkid'] = $link_info['linkid'];
		$this->_running_vars['cloned_asset'][$clone->id]['parentid'] = $this->_running_vars['to_parent_assetid'];

		if (!$clone->saveWebPaths($good_paths)) return FALSE;

		// Check for Additional processing
		if (method_exists($orig, 'cloneDependantAdditional')) {
			$orig->cloneDependantAdditional($clone);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_asset);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($orig);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($clone);

		$this->_running_vars['remap'][$assetid] = $remap;

		if (!empty($this->_running_vars['finalise_function'])) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($this->_running_vars['finalise_function'][0]);
			call_user_func_array($this->_running_vars['finalise_function'], Array(&$clone, &$this->_running_vars));
		}

		if (empty($this->_running_vars['clone_assets'][$assetid]['number_of_clones']) || ($this->_running_vars['clone_assets'][$assetid]['number_of_clones'] == 1)) {
			unset($this->_running_vars['clone_assets'][$assetid]);
		} else {
			$this->_running_vars['clone_assets'][$assetid]['number_of_clones']--;
		}
		$this->_running_vars['done_asset_count']++;

		if (empty($this->_running_vars['clone_assets'])) {
			unset($this->_running_vars['done_asset_count']);
			unset($this->_running_vars['todo_asset_count']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (int)(($this->_running_vars['done_asset_count'] / $this->_running_vars['todo_asset_count']) * 100);
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processCloneAssets()


	/**
	* Remaps the new assets from their old assetid to their new assetid
	*
	* Uses a map returned from the clone() function in asset.inc
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/
	function processRemap(&$step_data, $prefix)
	{
		$assetid = key($this->_running_vars['remap']);

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$map = $this->_running_vars['remap'][$assetid];
		unset($this->_running_vars['remap'][$assetid]);

		$step_data['message'] = 'Remapping "'.htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'" and its dependant assets.';

		if (!$asset->remapAssetids($map)) {
			trigger_localised_error('HIPO0027', E_USER_WARNING, $asset->id);
			return FALSE;
		}

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['remap'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$remap_total = count($this->_running_vars['done_assetids'])+count($this->_running_vars['remap']);
			$remap_percentage = (int)(count($this->_running_vars['done_assetids']) / $remap_total * 100);
			$step_data['percent_done'] = $remap_percentage;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processRemap()


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
					echo '<p><b>'.sprintf(translate('hipo_asset_cloning_successful'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</b></p>';
					echo '<ul>';
					for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid,));
						echo '<li>"'.$asset_info[$assetid]['name'].'"</li>';
					}
					echo '</ul>';
				}

				?>

				<p><?php echo sprintf(translate('hipo_permission_workflow_metadata_display'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>
				<p><?php echo sprintf(translate('hipo_permission_workflow_metadata_cascade_select'), htmlentities($parent->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></p>
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
	* Refreshes the asset map to reflect cloned assets that were added
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
		// Only paint in the first step (otherwise it performs a double 'get_assets' request to the asset map)
		// unless using HIPO server, in which case this step will be triggered only once
		if (SQ_HIPO_USE_SERVER || $step_data['percent_done'] === 0) {
			$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_assets) window.opener.parent.frames["sq_sidenav"].reload_assets("'.$this->_running_vars['to_parent_assetid'].'");');
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


}//end class


?>
