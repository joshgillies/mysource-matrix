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
* $Id: hipo_job_create_link.inc,v 1.11 2004/01/20 02:38:22 mmcintyre Exp $
* $Name: not supported by cvs2svn $
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
*                                                                           'access'          => $access,
*                                                                           'userid'          => $userid,
*                                                                           'previous_access' => null,
*                                                                      ),
*                                                   ),
* </pre>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Create_Link extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Create Link';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array( //0
						'name'			=> 'Confirmation',
						'function_call'		=> Array(
												'paint_function'   => 'paintConfirmation',
												'process_function' => 'processConfirmation',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> false,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //1
						'name'			=> 'Updating Lookups',
						'hipo_job'		=> Array(
												'init_details_function' => 'updateLookups',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //2
						'name'			=> 'Cascading Options',
						'function_call'		=> Array(
												'paint_function'   => 'paintInterface',
												'process_function' => 'processInterface',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> false,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //3
						'name'			=> 'Cascading Permissions',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadePermissions',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //4
						'name'			=> 'Cascading Metadata',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadeMetadata',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //5
						'name'			=> 'Cascading Workflow',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadeWorkflow',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //6
						'name'			=> 'Refreshing Asset Map',
						'function_call'	=> Array(
												'paint_function'   => 'paintRefresh',
												'process_function' => 'processRefresh',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Create_Link($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		if (isset($this->_running_vars['linkid'])) return parent::_getCodeName().$this->_running_vars['linkid'];
		else return parent::_getCodeName().$this->_running_vars['assetid'];

	}//end _getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (!isset($this->_running_vars['linkid']) && !isset($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Create Link HIPO, no existing link ID or asset ID specified', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['parentid'])) {
			trigger_error('Cannot initialise Create Link HIPO, no new parent ID specified', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['link_type'])) {
			trigger_error('Cannot initialise Create Link HIPO, no link type specified', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_pos'])) {
			trigger_error('Cannot initialise Create Link HIPO, no new parent position specified', E_USER_WARNING);
			return '';
		}

		return parent::initialise($source_code_name);

	}//end initialise()


	function getHipoThreshold()
	{
		if (!isset($this->_running_vars)) return false;

	}


	/**
	* Paints a confirmation screen for creating the link
	*
	* This allows the user to back out if they want to, as well as be presented
	* with any information/warnings regarding the new link (eg. move to trash)
	*
	* @param array								&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @access Public
	* @return boolean
	*/
	function paintConfirmation(&$step_data, &$o, $prefix)
	{
		$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parentid']);

		// are we moving or are we creating a new link?
		if (isset($this->_running_vars['assetid'])) {

			// we are creating a new link
			$o->openSection('New Link Confirmation');
				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$o->openField('&nbsp;');
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
						echo '<p>Please confirm that you are creating a new link between "'.$new_parent->name.'" and "'.$asset->name.'".</p><p>Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.</p>';
				$o->closeField();
			$o->closeSection();

		} else {

			// we are moving an asset
			$o->openSection('Move Confirmation');

				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}


				$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->_running_vars['linkid']);
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
				$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');


				$o->openField('&nbsp;');
					if ($new_parent->id == $trash_folder->id) {
						echo '<p>Please confirm that you are moving "'.$asset->name.'" to the trash.</p>';
					} else {
						echo '<p>Please confirm that you are moving "'.$asset->name.'" from under "'.$parent->name.'" to under "'.$new_parent->name.'".</p>';
					}
					echo '<p>Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.</p>';
				$o->closeField();
				
				if ($new_parent->id == $trash_folder->id) {
					$o->openField('&nbsp;');
						echo '<p>When "'.$asset->name.'" is moved to the trash, any URLs that it currently has will be broken. Use the asset finder below to select another asset to remap the URLs of "'.$asset->name.'" to. If you do not want to remap the URLs, just leave the field empty.</p>';
						asset_finder($prefix.'_remap_new_url', $asset->id, Array(), 'window.opener.top');
					$o->closeField();
				}


				// check to see if this asset is getting moved to the trash for good !!!
				if ($new_parent->id == $trash_folder->id) {
					// when checking for the number of links left in the system, make sure we disregard
					// any link to the trash folder because even if this asset is already in the trash,
					// we should warn about deleting it
					$trash_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_folder->id, $asset->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
					$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_LINK_TYPE_1 + SQ_LINK_TYPE_2, '', true, $this->_running_vars['linkid']);
					if (!empty($trash_link)) $num_other_links--;

					if (!$num_other_links) {

						// now lets get all the other links this asset has in the system and
						// display them to the user
						// note that we are also going to ask the major asset in the link
						// to describe it so it makes sense to the user looking at it
						$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3 + SQ_LINK_NOTICE, '', true, 'minor');

						if (!empty($links)) {

							$o->openSection('Warning - Moving To Trash');
								$o->openField('&nbsp;');
									?><p>Moving "<?php echo $asset->name; ?>" to the trash is going to affect other assets in the system. A list of the affected assets is shown below along with a description of their purpose.</p><?php

									$link_lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($asset, SQ_LINK_TYPE_3 + SQ_LINK_NOTICE);

									?><table class="sq-backend-table"><?php
									foreach ($links as $link_data) {
										$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['majorid'], $link_data['major_type_code']);
										$link_description = $major->describeLink($link_data['linkid']);
										$link_lineage = $link_lineages[$link_data['linkid']]['lineage'];

										// construct a human readable lineage
										$num_in_lineage = count($link_lineage);
										$lineage = '';
										for ($i = 0; $i < $num_in_lineage; $i++) {
											$entry = $link_lineage[$i];
											if ($i == ($num_in_lineage -1)) {
												continue;
											} else if ($i <= 1 || $num_in_lineage <= 5) {
												$lineage .= $entry.' &gt; ';
											} else if ($num_in_lineage - $i <= 3 && $num_in_lineage > 5) {
												$lineage .= $entry.' &gt; ';
											} else if ($i == 2 && $num_in_lineage > 5) {
												$lineage .= '... &gt; ';
											}
										}
										$lineage .= '<b>'.$link_lineage[($num_in_lineage-1)].'</b>';
										
										?>
										<tr>
											<td class="sq-backend-table-cell">
												<b><?php echo $lineage; ?></b>
												<?php echo (!empty($link_description)) ? '<br/>'.$link_description : ''; ?>
											</td>
										</tr>
										<?php
									}
									?></table><?php
								$o->closeField();
							$o->closeSection();

						}//end if
					}
				}

			$o->closeSection();

		}//end if

		$o->addHiddenField('form_submitted', 1);
		
		return true;

	}//end paintConfirmation()


	/**
	* Process the user's confirmation that they want to create the new link
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processConfirmation(&$step_data, $prefix)
	{
		// has the next button been clicked ?
		if (isset($_REQUEST['form_submitted'])) {

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


			// if we are moving to the trash and they want to remap the URL's of the asset that
			// is being trashed to another asset, go through and do all this for them
			if (isset($_POST[$prefix.'_remap_new_url'])) {
				$assetid = $_POST[$prefix.'_remap_new_url']['assetid'];

				if ($assetid) {
					$url = $_POST[$prefix.'_remap_new_url']['url'];

					$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->_running_vars['linkid']);
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

					if ($assetid == $asset->id) {
						$this->_tmp['message'] = '<span style="color: #FF0000;">There is no point remapping the URLs of "'.$asset->name.'" to itself - they are still going to be broken. Please choose another asset.</span>';
						return true;
					}

					if (empty($url)) {
						$remap_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$this->_tmp['message'] = '<span style="color: #FF0000;">You cannot remap URLs of "'.$asset->name.'" to "'.$remap_asset->name.'" because it does not have a URL itself. Please choose another asset.</span>';
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
						return true;
					}

					$rm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
					if (!$rm->remapAllURLs($asset->id, $url)) {
						$this->_tmp['message'] = '<span style="color: #FF0000;">The system was unable to remap the URLs for "'.$asset->name.'" to "'.$url.'"</span>';
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($remap_asset);
						return true;
					}
				}
			}


			if (isset($this->_running_vars['assetid'])) {
				// create a new link
				$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parentid']);
				$asset      = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
				$linkid = $new_parent->createLink($asset, $this->_running_vars['link_type'], '', $this->_running_vars['to_parent_pos']);
			} else {
				// move the link
				$linkid = $GLOBALS['SQ_SYSTEM']->am->moveLink($this->_running_vars['linkid'], $this->_running_vars['parentid'], $this->_running_vars['link_type'], $this->_running_vars['to_parent_pos']);
			}

			if (!$linkid) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

			// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
			// we will be affecting out list of changes on
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_link['minorid'], $new_link['minor_type_code']);
			if (is_null($asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			// if we are moving to the trash, we want to skip all the steps that ask us if we want
			// to cascade permissions, metadata, and workflow
			$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			if ($this->_running_vars['parentid'] == $trash_folder->id) {
				$this->skipStep(2);
				$this->skipStep(3);
				$this->skipStep(4);
				$this->skipStep(5);
			}

			$this->_running_vars['assetid'] = $asset->id;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;
	
	}//end processConfirmation()


	/**
	* Paint the interface for accepting user input on what to cascade (permissions/metadata/workflow)
	*
	* @param array								&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @access public
	* @return boolean
	*/
	function paintInterface(&$step_data, &$o, $prefix)
	{
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		if (is_null($hh)) return false;

		$o->addHiddenField('link_assetid',  $this->_running_vars['assetid']);
		$o->addHiddenField('link_parentid', $this->_running_vars['parentid']);

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Asset #'.$this->_running_vars['assetid'].' not found', E_USER_WARNING);
			return false;
		}

		$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parentid']);
		if (is_null($parent)) {
			trigger_error('Asset #'.$this->_running_vars['parentid'].' not found', E_USER_WARNING);
			return false;
		}

		$o->openSection('Note');
			$o->openField('&nbsp;');
				$o->addHiddenField('link_completed', '0');
				?>
				Press the <b><i>Cascade</i></b> button below if you want to cascade the permissions/workflow/metadata of "<?php echo $parent->name; ?>" down to "<?php echo $asset->name; ?>" and its children. To skip the cascading, press the <b><i>Skip</i></b> button below.<br/><br/>
				The permission of the new parent "<?php echo $parent->name; ?>" are shown below. These changes will attempt to be set on "<?php echo $asset->name; ?>" and its children if you choose to cascade.<br/><br/>
				<b>Note:</b> Any assets that become children of "<?php echo $asset->name; ?>" during the cascade process will not have changes applied to them - only the <i>current</i> children.
				<?php
				$ei = &$parent->getEI();
				
				$ei->includeStaticScreen('permissions');
				$pe = paintInlinePermissions($parent, $o, $ei);
				
				$ei->includeStaticScreen('workflow');
				$wf = paintInlineWorkflow($parent, $o, $ei);
				
				$ei->includeStaticScreen('metadataSchemas');
				$ms = paintInlineMetadataSchemas($parent, $o, $ei);
				
				$o->addHiddenField('form_submitted', 1);
			$o->closeField();
		$o->closeSection();
		
		$o->openSection('Cascade');
			$o->openField('', 'cascade');
				echo ($pe) ? 'Permissions'.check_box('cascade[]', 'cascade_pe').'<br />' : '';
				echo ($ms) ? 'Metadata Schemas'.check_box('cascade[]', 'cascade_ms').'<br />' : '';
				echo ($wf) ? 'Workflow'.check_box('cascade[]', 'cascade_wf').'<br />' : '';
			$o->closeField();
		$o->closeSection();
		
		return true;

	}//end paintInterface()


	/**
	* Process the user's selections to cascade whatever, and skip those steps that they don't want to run
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$step_data, $prefix)
	{
		// has the next button been clicked ?
		if (isset($_REQUEST['form_submitted'])) {
			if (isset($_REQUEST['cascade'])) {
				// permissions
				if (!in_array('cascade_pe', $_REQUEST['cascade'])) $this->skipStep(3);
				// metadata
				if (!in_array('cascade_ms', $_REQUEST['cascade'])) $this->skipStep(4);
				//workflow
				if (!in_array('cascade_wf', $_REQUEST['cascade'])) $this->skipStep(5);
			} else {
				$this->skipStep(3); // permissions
				$this->skipStep(4); // metadata
				$this->skipStep(5); // workflow
			}
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;
	
	}//end processInterface()
	

	/**
	* To be run as Step 1. This will run the Update Lookups before retrieving user input about cascading permissions etc.
	*
	* Run before cascading because this is critical. Dont want them to bail out before doing this.
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function updateLookups(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_update_lookups';
		$options['auto_complete'] = true;
		$running_vars['assetids'] = Array($this->_running_vars['assetid']);

	}//end updateLookups()


	/**
	* Cascade permissions (if not skipped)
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function cascadePermissions(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_permissions';
		$permission_vars = Array();

		foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
			$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->_running_vars['parentid'], $perm, null, false, false, true);
			foreach ($set_perms as $userid => $access) {
				$permission_vars[$perm][] = Array('access'          => $access,
												  'userid'          => $userid,
												  'previous_access' => null,
												  );
			}
		}

		$running_vars['permission_changes'] = $permission_vars;
		$running_vars['assetid'] = $this->_running_vars['assetid'];

	}//end cascadePermissions()


	/**
	* Cascade metadata (if not skipped)
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function cascadeMetadata(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_metadata_schemas';
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($this->_running_vars['parentid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $access) {
			$schema_vars[] = Array('access'          => $access,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
								   );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assetid'] = $this->_running_vars['assetid'];

	}//end cascadeMetadata()


	/**
	* Cascade workflow (if not skipped)
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function cascadeWorkflow(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_workflow_schemas';
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($this->_running_vars['parentid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $access) {
			$schema_vars[] = Array('access'          => $access,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
						      );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assetid'] = $this->_running_vars['assetid'];

	}//end cascadeWorkflow()


	/**
	* Refreshes the asset map to reflect the new link changes we made
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
		$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$this->_running_vars['parentid'].'");');
		if (isset($this->_running_vars['old_parentid'])) {
			$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$this->_running_vars['old_parentid'].'");');
		}

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