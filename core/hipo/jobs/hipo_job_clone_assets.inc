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
* $Id: hipo_job_clone_assets.inc,v 1.13 2005/05/11 06:05:07 lwright Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clone assets
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Clone_Assets extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Clone Assets';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'			=> 'Clone Options',
						'function_call'	=> Array(
											'paint_function'	=> 'paintCloneOptions',
											'process_function'	=> 'processCloneOptions',
										   ),
						'running_mode'	=> 'web',
						'skip_step'		=> false,
						'auto_step'		=> false,
						'allow_cancel'	=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Acquiring Locks',
						'hipo_job'		=> Array(
											'init_details_function'	=> 'getLockAssetDetails',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Cloning Assets',
						'function_call'	=> Array(
											'process_function'	=> 'processCloneAssets',
										   ),
						'running_mode'	=> 'server',
						'skip_step'		=> false,
						'auto_step'		=> true,
						'allow_cancel'	=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Remapping Assets',
						'function_call'	=> Array(
											'process_function'	=> 'processRemap',
										   ),
						'running_mode'	=> 'server',
						'skip_step'		=> false,
						'auto_step'		=> true,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Releasing Locks',
						'function_call'	=> Array(
											'process_function'	=> 'releaseLocks',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'			=> 'Refreshing Asset Map',
						'function_call'	=> Array(
											'paint_function'	=> 'paintRefresh',
											'process_function'	=> 'processRefresh',
										   ),
						'running_mode'	=> 'web',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
				  );


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
	* @param object Backend_Outputter	&$o		reference to the backend outputter
	* @param string						$class	calling class name
	*
	* @return void
	* @access private
	*/
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();

		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('clone_asset_threshold'));

		if ($write_access) {
				text_box($class.'[SQ_DUPE_ASSET_THRESHOLD]',SQ_DUPE_ASSET_THRESHOLD, 5);
		} else {
				echo SQ_DUPE_ASSET_THRESHOLD;
		}
		echo ' '. translate('assets');

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	function getConfigVars()
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
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processLookups($this->_steps[0], get_class($this))) {
				return false;
			}
		}
		return true;

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
			return false;
		}

		$this->_running_vars['remap'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = true;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('all'),
							'forceably_acquire'	=> false,
							'dependants_only'	=> true,
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks that were acquired while cloning assets
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
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

		$children = &$GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_type_code) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'all');
		}

		$asset_info = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('hipo_release_locks', $asset_info[$assetid]['name']).translate('hipo_release_locks_dependents');

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end releaseLocks()


	/**
	* Paints some option for the user to specify what assets are to be cloned
	*
	* All dependant asset will be automatically cloned, so no interface will be
	* supplied for duping dependant assets which should save some space.
	*
	* @param &array							$step_data	a reference to the step data for this step
	* @param &object Hipo_Backend_Outputter	$o			the hipo backend outputter object
	* @param string							$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function paintCloneOptions(&$step_data, &$o, $prefix)
	{
		$to_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

		$o->openSection(translate('cloning_confirmation'));
			$o->openField(translate('confirm'));
				?>
				<p><?php echo translate('hipo_cloning_confirm', $to_parent->name); ?></p>
				<ul>
					<?php
					$assets = $this->_running_vars['assets'];
					for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
						$asset_info = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
						?><li>"<?php echo $asset_info[$assetid]['short_name']; ?>"</li><?php
					}
					?>
				</ul>
				</p><p><?php echo translate('hipo_confirm_abort');?></p>
				<?php
			$o->closeField();
		$o->closeSection();

	}//end paintCloneOptions()


	/**
	* Does nothing
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processCloneOptions(&$step_data, $prefix)
	{
		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;

	}//end processCloneOptions()


	/**
	* Processes the assets that need to be cloned, and clones them
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/
	function processCloneAssets(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['clone_assets'])) {
			 $this->_running_vars['clone_assets'] = $this->_running_vars['assets'];
		}

		$assetid = key($this->_running_vars['clone_assets']);

		$parent_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

		if (is_null($parent_asset)) {
			trigger_localised_error('HIPO0030', E_USER_WARNING, $this->_running_vars['to_parent_assetid']);
			return false;
		}

		$orig = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		if (is_null($orig)) {
			trigger_localised_error('HIPO0029', E_USER_WARNING, $assetid);
			return false;
		}

		$orig_link = &$GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->_running_vars['clone_assets'][$assetid]['parentid'], $orig->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3);

		unset($this->_running_vars['clone_assets'][$assetid]);

		$link = Array(
					'asset'			=> $parent_asset,
					'link_type'		=> $orig_link['link_type'],
					'sort_order'	=> $this->_running_vars['to_parent_pos'],
					'value'			=> '',
				);

		$this->_running_vars['to_parent_pos']++;

		$components = Array(
						'attributes',
						'metadata_schemas',
						'metadata',
						'workflow',
						'permissions',
					  );

		$step_data['message'] = 'Cloning "'.$orig->name.'" to "'.$parent_asset->name.'".';

		$clone = &$GLOBALS['SQ_SYSTEM']->am->cloneAsset($orig, $link, $remap, $components, true);

		if (is_null($clone)) {
			trigger_localised_error('HIPO0028', E_USER_WARNING);
			return false;
		}

		// the web path of this asset may confict with the children
		// of the parent asset that we are linking to, so check the integrity
		// of the path before using it
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths($orig->getWebPaths());
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $clone->id, true);

		if (!$clone->saveWebPaths($good_paths)) return false;

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_asset);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($orig);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($clone);

		$this->_running_vars['remap'][$assetid] = $remap;

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['clone_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processCloneAssets()


	/**
	* Remaps the new assets from their old assetid to their new assetid
	*
	* Uses a map returned from the clone() function in asset.inc
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/
	function processRemap(&$step_data, $prefix)
	{
		$assetid = key($this->_running_vars['remap']);

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$map = $this->_running_vars['remap'][$assetid];
		unset($this->_running_vars['remap'][$assetid]);

		$step_data['message'] = 'Remapping "'.$asset->name.'" and its dependant assets.';

		if (!$asset->remapAssetids($map)) {
			trigger_localised_error('HIPO0027', E_USER_WARNING, $asset->id);
			return false;
		}

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['remap'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['remap'])) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processRemap()


	/**
	* Refreshes the asset map to reflect cloned assets that were added
	*
	* @param array							&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter	&$o			the backend outputter class
	* @param string							$prefix		prefix for form vars
	*
	* @access public
	* @return boolean
	*/
	function paintRefresh(&$step_data, &$o, $prefix)
	{
		$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$this->_running_vars['to_parent_assetid'].'");');

		return true;

	}//end paintRefresh()


	/**
	* Does nothing
	*
	* @param array	&$step_data	the step data for this job
	* @param string	$prefix		prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processRefresh(&$step_data, $prefix)
	{
		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;

	}//end processRefresh()


}//end class


?>
