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
* $Id: hipo_job_clone_asset.inc,v 1.1 2004/02/27 05:32:34 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clone assets
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Clone_Asset extends HIPO_Job
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
						'name'				=> 'Clone Options',
						'function_call'		=> Array(
												'paint_function'   => 'paintCloneOptions',
												'process_function' => 'processCloneOptions',
												),
						'running_mode'		=> 'web',
						'skip_step'			=> false,
						'auto_step'			=> false,
						'percent_done'		=> 0,
						'complete'			=> false,
						'message'			=> '',
					),
					Array(
						'name'				=> 'Acquiring Locks',
						'hipo_job'			=> Array(
												'init_details_function' => 'getLockAssetDetails',
												),
						'running_mode'		=> 'server',
						'skip_step'			=> false,
						'auto_step'			=> true,
						'percent_done'		=> 0,
						'complete'			=> false,
						'message'			=> '',
					),
					Array(
						'name'				=> 'Cloning Assets',
						'function_call'		=> Array(
												'process_function' => 'processCloneAssets',
												),
						'running_mode'		=> 'server',
						'skip_step'			=> false,
						'auto_step'			=> true,
						'percent_done'		=> 0,
						'complete'			=> false,
						'message'			=> '',
					),
					Array(
						'name'				=> 'Remapping Assets',
						'function_call'		=> Array(
												'process_function' => 'processRemap',
												),
						'running_mode'		=> 'server',
						'skip_step'			=> false,
						'auto_step'			=> true,
						'percent_done'		=> 0,
						'complete'			=> false,
						'message'			=> '',
					),
					Array(
						'name'				=> 'Releasing Locks',
						'function_call'		=> Array(
												'process_function' => 'releaseLocks',
												),
						'running_mode'		=> 'server',
						'auto_step'			=> true,
						'skip_step'			=> false,
						'percent_done'		=> 0,
						'complete'			=> false,
						'message'			=> '',
					),
					Array(
						'name'				=> 'Refreshing Asset Map',
						'function_call'		=> Array(
												'paint_function'   => 'paintRefresh',
												'process_function' => 'processRefresh',
												),
						'running_mode'		=> 'web',
						'auto_step'			=> true,
						'skip_step'			=> false,
						'percent_done'		=> 0,
						'complete'			=> false,
						'message'			=> '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Clone_Asset($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();
		
		// metadata regeneration threshhold HIPO config entry
		$o->openField('Clone Asset Threshold');

		if($write_access) {
			text_box($class.'[SQ_DUPE_ASSET_THRESHOLD]',SQ_DUPE_ASSET_THRESHOLD, 2);
		} else {
			echo SQ_DUPE_ASSET_THRESHOLD;
		}

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
			'SQ_DUPE_ASSET_THRESHOLD' => Array('editable' => 1, 'default' => 1),
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
		return parent::getCodeName().'-'.$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	* 
	* @return float
	* @access public
	*/
	function getThreshold()
	{
		if (SQ_DUPE_ASSET_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) return 0;
		return ((count($this->_running_vars['todo_assetids']) / SQ_DUPE_ASSET_THRESHOLD) * 100);
	
	}//end getThreshold()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	* 
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processLookups($this->_steps[0], get_class($this))) return false;
		}
		return true;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start updating lookups, we need to have an asset id to start updating lookups at
		if (empty($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Clone Asset HIPO, no start asset IDs specified', E_USER_WARNING);
			return false;
		}

		/////// create some storage facilities ////////
		
		// a list of clone assets and the current position of the cloned asset getting processed
		$this->_running_vars['clone']              = Array();
		$this->_running_vars['clone']['pos']       = 0;
		
		// a map of old assetids => new assetids. This data is returned from
		// a call to clone() in asset.inc and is used to remap assets
		$this->_running_vars['clone']['asset_map'] = Array();

		// a map of linkids so that links are not created twice
		$this->_running_vars['clone']['link_map']  = Array();

		// a list of remaped asset positions used in remapping assets
		$this->_running_vars['remap']              = Array();
		$this->_running_vars['remap']['pos']       = 0;

		$todo_assetids = $this->_running_vars['assetid'];

		return parent::prepare();
	
	}//end prepare()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Update Lookups HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (!$this->is_prepared) {
			$this->prepare();
		}
		return parent::initialise($source_code_name);

	}//end initialise()

	
	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_lock';

		$options['auto_complete'] = true;

		$running_vars['assetid']         = $this->_running_vars['assetid'];
		$running_vars['lock_type']       = 'all';
		$running_vars['dependants_only'] = true;

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the permissions
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (!is_null($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
			$step_data['message'] = 'Releasing Locks';
		}
		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end releaseLocks()


	/**
	* Returns the number of non dependant links from a given array
	*
	* @param Array $links the links array
	* 
	* @return integer the number of non-dependant links
	* @access public
	*/
	function getNonDependantLinksCount($links)
	{
		$non_dependants = 0;
		for(reset($links); null !== ($treeid = key($links)); next($links)) {
			if (!$links[$treeid]['dependant']) $non_dependants++;
		}

		return $non_dependants;
	
	}//end getNonDependantLinksCount()


	/**
	* Paints some option for the user to specify what assets are to cloned
	* All dependant asset will be automatically cloned, so no interface will be 
	* supplied for duping dependant assets which should save some space.
	*
	* @param &Array								$step_data	a reference to the step data for this step
	* @param &object Hipo_Backend_Outputter		$o			the hipo backend outputter object
	* @param String								$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function paintCloneOptions(&$step_data, &$o, $prefix)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid'], $this->_running_vars['type_code']);

		// we are only interested in type 1 and type 2 link types for the moment
		// the clone function for each asset will take care or notice links and type 3 links
		$tree = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($asset->id, SQ_SC_LINK_BACKEND_NAV);

		$o->addHiddenField('clone_assetid', $asset->id);
		$non_dependants = $this->getNonDependantLinksCount($tree);

		$o->openSection('Cloning '.$asset->short_name.' ('.(count($tree) + 1).' Assets)');
			$o->openField('Select Assets to Clone');

				// if there are no NON dependant assets to clone, there is no point
				// showing an interface for choosing which assets to clone
				if ($non_dependants == 0) {
					?>
						<p><?php echo $asset->short_name; ?> has no non-dependant children and therefore only the dependant children of <?php echo $asset->short_name; ?> will be cloned.</p>

						<?php
						// we still need to add some information about the dependant assets of the initiating asset
						for (reset($tree); null !== ($treeid = key($tree)); next($tree)) {
							if ($tree[$treeid]['dependant']) {
								$o->addHiddenField('clone_treeids['.$treeid.']', $tree[$treeid]['linkid']);
							}
						}
				// there is one or more NON-dependant asset
				} else {
					?>
						<p>NB. Dependant assets are not shown in the list of available assets to clone as they are required by their parent assets to function properly.</p>
					<?php

					?>
					<table border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td class="sq-backend-data" style="font-weight: bold;">
								<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($asset->type()); ?>", "16", "16", "");</script>&nbsp;
								<?php echo $asset->short_name ?>
							</td>
							<td class="sq-backend-data">
								&nbsp;<b>[ <?php check_box('clone_treeids[0]', $asset->id, true, 'ALL_CHECKED = (ALL_CHECKED) ? false : true; check_all(this.form, "clone_treeids.*", ALL_CHECKED);'); ?> check/uncheck all ]</b>
							</td>
						</tr>
					</table>
					<?php

					if (!empty($tree)) {

						?>
						<script language="javascript" type="text/javascript">
							<!-- //
								// initially, they are all checked
								var ALL_CHECKED = true;
								var CLONE_TREEIDS = ["<?php echo implode('" ,"', array_keys($tree));?>"];

								function clone_treeids_check(treeid, checked) {
									var tmp_treeid = new String(treeid);

									// if we were checked make sure that all our parents are checked
									if (checked) {
										while (tmp_treeid.length > 0) {
											var elem = get_form_element("clone_treeids[" + tmp_treeid + "]");
											// this may be a dependant link , which is a hidden field
											if (elem.type != 'checkbox') continue;
											elem.checked = true;
											tmp_treeid = tmp_treeid.substr(0, tmp_treeid.length - <?php echo SQ_CONF_ASSET_TREE_SIZE; ?>);
										}
									}
									// Change all our children to be what we are
									for(var i = 0; i < CLONE_TREEIDS.length; i++) {
										if (treeid == CLONE_TREEIDS[i].substr(0, treeid.length)) {
											var elem = get_form_element("clone_treeids[" + CLONE_TREEIDS[i] + "]");
											// this may be a dependant link , which is a hidden field
											if (elem.type != 'checkbox') continue;
											elem.checked = checked;
										}
									}
								}// end clone_treeids_check()

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
							for (reset($tree); null !== ($treeid = key($tree)); next($tree)) {
								$prev_level = $level;
								// we need to cast the float to an int (well, 'string' float)
								$level = (int) $tree[$treeid]['level'];

								$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
								$last_at_level[$parent_treeid] = $treeid;

								if ($level > $prev_level) {
									array_push($indents, $parent_treeid);
								} elseif ($level < $prev_level) {
									for ($x = $prev_level; $x > $level; $x--) {
										array_pop($indents);
									}
								}

								?>
								<table border="0" cellspacing="0" cellpadding="0">
									<tr>
										<?php

										if (!$tree[$treeid]['dependant']) {
											if (!empty($indents)) {
												echo $indent_start, implode($indent_end.$indent_start, $indents), $indent_end;
											}

											?>
												<td class="sq-backend-data" <?php echo '%', $treeid, '%'; ?>><img src="<?php echo $branch_img; ?>" width="20" height="20" border="0" alt="branch" /></td>
												<td class="sq-backend-data">
												<?php
													check_box('clone_treeids['.$treeid.']', $tree[$treeid]['linkid'], true, 'clone_treeids_check("'.$treeid.'", this.checked);');
												?>
												</td>
												<td class="sq-backend-data">
													<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($tree[$treeid]['type_code']); ?>", "16", "16", "");</script>&nbsp;
													<?php
														echo $tree[$treeid]['short_name'];
													?>
												</td>
											<?php
										} else {
											// set a hidden field for the dependant asset
											$o->addHiddenField('clone_treeids['.$treeid.']', $tree[$treeid]['linkid']);
										}
										?>

										</tr>
									</table>
								<?php

							}// end for

							$tree_html = ob_get_contents();
						ob_end_clean();

						$bg_stalks_search  = Array();
						$bg_stalks_replace = Array();
						$bg_code = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
						
						for (reset($tree); null !== ($treeid = key($tree)); next($tree)) {
							$bg_stalks_search[] = '%'.$treeid.'%';
							// if this treeid was the last on it's level it doesn't have a stalk bg
							$bg_stalks_replace[] = (in_array($treeid, $last_at_level, true)) ? '' : $bg_code;
						}// end for

						echo str_replace($bg_stalks_search, $bg_stalks_replace, $tree_html);

					}// end if not empty tree
				}//end if dependants

			$o->closeField();
		$o->closeSection();

	}//end paintCloneOptions()


	/**
	* Processes the assets to clone that are chosen by the user.
	*
	* @param &Array		$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/
	function processCloneOptions(&$step_data, $prefix)
	{
		if (!isset($_POST['clone_assetid'])) return true;

		$tree = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($this->_running_vars['assetid']);

		$effective_clone_treeids = Array('' => Array());
		$indexed_effective_clone_treeids = Array();

		if (!empty($_POST['clone_treeids'])) {

			// make sure they are all in the proper order
			ksort($_POST['clone_treeids'], SORT_STRING);
			$clone_treeids = Array();
			
			for (reset($_POST['clone_treeids']); null !== ($treeid = key($_POST['clone_treeids'])); next($_POST['clone_treeids'])) {
				$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
			
				// if this is a top level link or if this links parent link is selected
				// then we can add it to the proper array
				// this strips out any selections whose parents where selected
				// and any dependent links (in hidden fields) whose parent wasn't selected
			
				if ($parent_treeid == '' || isset($clone_treeids[$parent_treeid])) {
					
					if (!isset($tree[$treeid])) continue;

					$clone_treeids[$treeid] = $_POST['clone_treeids'][$treeid];
					$dependant_treeid = $tree[$treeid]['dependant_treeid'];
					
					if (!isset($effective_clone_treeids[$dependant_treeid])) {
						$effective_clone_treeids[$dependant_treeid] = Array();
					}
					$effective_clone_treeids[$dependant_treeid][] = $treeid;
				
				}// end if
			}// end for

			ksort($clone_treeids, SORT_STRING);
			ksort($effective_clone_treeids, SORT_STRING);
			$indexed_effective_clone_treeids = Array();

			for (reset($effective_clone_treeids); null !== ($treeid = key($effective_clone_treeids)); next($effective_clone_treeids)) {
				$indexed_effective_clone_treeids[] = Array(
					'treeid'     => $treeid,
					'dependants' => $effective_clone_treeids[$treeid],
				);
			}// end for
		
		}//end if

		// create a unique list of assets that we will be aquiring locks/duplicating
		$map = Array();
		foreach ($indexed_effective_clone_treeids as $data) {

			// if the treeid is blank, we are using the initating asset
			if ($data['treeid'] == '') {
				$assetid	= $this->_running_vars['assetid'];
				$type_code	= $this->_running_vars['type_code'];

			// else we are locking a sub asset
			} else {
				$assetid	= $tree[$data['treeid']]['assetid'];
				$type_code	= $tree[$data['treeid']]['type_code'];

			}// end if

			if (!isset($map[$assetid])) $map[$assetid] = $type_code;

		}// end foreach

		// create a unique list of assets to acquire locks for
		$this->_running_vars['assets'] = Array();
		
		foreach ($map as $assetid => $type_code) {
			$this->_running_vars['assets'][] = Array('assetid' => $assetid, 'type_code' => $type_code);
		}

		// OK, what we have with the $effective_clone_treeids array is access to all the assets that
		// we are going to clone to have to clone, with all the assets that they will create
		// when clone is called (because it also clones any dependantly linked assets)

		$this->_running_vars['tree'] = $tree;
		$this->_running_vars['effective_clone_treeids'] = $indexed_effective_clone_treeids;

		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;

		return true;

	}//end processCloneOptions()


	/**
	* Processes the assets that need to be cloned, and clones them
	*
	* @param &Array		$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/ 
	function processCloneAssets(&$step_data, $prefix)
	{
		$treeid = $this->_running_vars['effective_clone_treeids'][$this->_running_vars['clone']['pos']]['treeid'];
		$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);

		// if there is a blank treeid then we must be duplicating the initialising asset
		$link    = Array('asset' => null, 'link_type' => 0, 'sort_order' => -1, 'value' => '');
		$assetid   = 0;
		$type_code = '';
		$old_linkid = 0;
	
		// if the treeid is blank we are duping the initating asset
		if ($treeid == '') {
			$assetid			= $this->_running_vars['assetid'];
			$type_code			= $this->_running_vars['type_code'];
			$link['link_type']	= SQ_LINK_TYPE_1;
			$link['sort_order']	= $this->_running_vars['to_parent_pos'];
			$link['asset']		= &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

		// else we are cloning a sub asset
		} else {
			$assetid	= $this->_running_vars['tree'][$treeid]['assetid'];
			$type_code	= $this->_running_vars['tree'][$treeid]['type_code'];
			
			if ($parent_treeid == '') {
				$parent_assetid		= $this->_running_vars['assetid'];
				$parent_type_code	= $this->_running_vars['type_code'];
			} else {
				$parent_assetid		= $this->_running_vars['tree'][$parent_treeid]['assetid'];
				$parent_type_code	= $this->_running_vars['tree'][$parent_treeid]['type_code'];
			}
		
			// now get the assetid for the cloned parent asset, not the original
			if (isset($this->_running_vars['clone']['asset_map'][$parent_assetid])) {
				$link['asset']	= &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['clone']['asset_map'][$parent_assetid], $parent_type_code);
			}

			$link['link_type']	= $this->_running_vars['tree'][$treeid]['link_type'];
			$link['sort_order']	= $this->_running_vars['tree'][$treeid]['sort_order'];
			$link['value']		= $this->_running_vars['tree'][$treeid]['value'];

			$old_linkid = $this->_running_vars['tree'][$treeid]['linkid'];

		}// end if

		if (is_null($link['asset'])) {
			trigger_error('Unable to find parent asset for Asset Id #'.$assetid, E_USER_WARNING);
			return false;
		}

		// If this asset has already been cloned,
		// then just link it to it's new parent
		if (isset($this->_running_vars['clone']['asset_map'][$assetid])) {
			$clone = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['clone']['asset_map'][$assetid], $type_code);
		
			if (is_null($clone)) {
				trigger_error('Unable to find cloned asset for Asset Id #'.$assetid, E_USER_WARNING);
				return false;
			}// end if

			$step_data['message'] = 'Linking '.$clone->name;

			// if we have already created this link we don't need to do it again
			if (empty($old_linkid) || !in_array($old_linkid, $this->_running_vars['clone']['link_map'])) {
				$linkid = $link['asset']->createLink($clone, $link['link_type'], $link['value'], $link['sort_order']);
				if (!$linkid) return false;
			}// end if

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($clone);

		// otherwise clone and link
		} else {

			$orig = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			
			if (is_null($orig)) {
				trigger_error('Unable to find original asset, Asset Id #'.$assetid, E_USER_WARNING);
				return false;
			}// end if

			$step_data['message'] = 'Cloning '.$orig->name;
			$clone = &$GLOBALS['SQ_SYSTEM']->am->cloneAsset($orig, $link, $this->_running_vars['clone']['asset_map'], Array('all'), true);
		
			if (is_null($clone)) {
				trigger_error('Clone failed for Asset Id #'.$assetid, E_USER_WARNING);
				return false;
			}// end if

		
			// the web path of this asset may confict with the children
			// of the parent asset that we are linking to, so check the integrity
			// of the path before using it
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_paths = make_valid_web_paths($orig->getWebPaths());
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $clone->id, true);

			if (!$clone->saveWebPaths($good_paths)) {
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($clone);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($orig);

		}// end if

		if ($old_linkid) $this->_running_vars['clone']['link_map'][] = $old_linkid;
		$step_data['percent_done'] = ($this->_running_vars['clone']['pos'] + 1) / count($this->_running_vars['effective_clone_treeids']) * 100;
		
		if (count($this->_running_vars['effective_clone_treeids']) > $this->_running_vars['clone']['pos'] + 1) {
			$this->_running_vars['clone']['pos']++;
		} else {
			$step_data['complete'] = true;
			$step_data['percent_done'] = 100;
		}

		// we don't need this anymore
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($link['asset']);
		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->_running_vars['assetid'], 'all');

		return true;
	
	}//end processCloneAssets()


	/**
	* Remaps the new assets from their old assetid to their new assetid, 
	* using a map returned from the clone() function in asset.inc
	*
	* @param &Array		$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form vairables
	*
	* @return boolean
	* @access public
	*/
	function processRemap(&$step_data, $prefix)
	{
		$assetid   = $this->_running_vars['assets'][$this->_running_vars['remap']['pos']]['assetid'];
		$type_code = $this->_running_vars['assets'][$this->_running_vars['remap']['pos']]['type_code'];

		$cloned_assetid = $this->_running_vars['clone']['asset_map'][$assetid];

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($cloned_assetid, $type_code);
		if (is_null($asset)) {
			trigger_error('Unable to find cloned asset, Asset Id #'.$cloned_assetid, E_USER_WARNING);
			return false;
		}

		$step_data['message'] = 'Remaping '.$asset->name;

		if (!$asset->remapAssetids($this->_running_vars['clone']['asset_map'])) {
			trigger_error('Remap failed for asset, Asset Id #'.$asset->id, E_USER_WARNING);
			return false;
		}

		$step_data['percent_done'] = $this->_running_vars['remap']['pos'] / count($this->_running_vars['assets']) * 100;

		if (count($this->_running_vars['assets']) > $this->_running_vars['remap']['pos'] + 1) {
			$this->_running_vars['remap']['pos']++;
		} else {
			$step_data['complete'] = true;
			$step_data['percent_done'] = 100;
		}
		
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->_running_vars['assetid'], 'all');

		return true;

	}//end processRemap()


	/**
	* Refreshes the asset map to reflect cloned assets that were added
	*
	* @param array								&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @access Public
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
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
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