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
* $Id: hipo_job_create_links.inc,v 1.3 2005/02/21 04:57:21 ndvries Exp $
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
*                                                                           'previous_access' => null,
*                                                                      ),
*                                                   ),
* </pre>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Create_Links extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Create Links';

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
						'allow_cancel'	=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //1
						'name'			=> 'Creating Links',
						'function_call'	=> Array(
												'process_function' => 'processCreateLinks',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //2
						'name'			=> 'Updating Lookups',
						'hipo_job'		=> Array(
												'init_details_function' => 'updateLookups',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //3
						'name'			=> 'Cascading Options',
						'function_call'		=> Array(
												'paint_function'   => 'paintInterface',
												'process_function' => 'processInterface',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> false,
						'skip_step'		=> false,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //4
						'name'			=> 'Cascading Permissions',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadePermissions',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //5
						'name'			=> 'Cascading Metadata',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadeMetadata',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //6
						'name'			=> 'Cascading Workflow',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadeWorkflow',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'allow_cancel'	=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //7
						'name'			=> 'Refreshing Asset Map',
						'function_call'	=> Array(
												'paint_function'   => 'paintRefresh',
												'process_function' => 'processRefresh',
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
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Create_Link($code_name='')
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
	* Set up vars and database information that the HIPO will need to run
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (empty($this->_running_vars['assets'])) {
			trigger_error('Cannot initialise Create Link HIPO, no assets specified', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_assetid'])) {
			trigger_error('Cannot initialise Create Link HIPO, no new parent ID specified', E_USER_WARNING);
			return '';
		}
		if (!isset($this->_running_vars['to_parent_pos'])) {
			trigger_error('Cannot initialise Create Link HIPO, no new parent position specified', E_USER_WARNING);
			return '';
		}

	//	$GLOBALS['SQ_SYSTEM']->setLastAction('system:create_link', 'Creating Link', $this->_running_vars['assetid']);
		return parent::initialise($source_code_name);

	}//end initialise()


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
		$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
		$assets = $this->_running_vars['assets'];
		if (isset($this->_running_vars['reordered_assets'])) {
			$reordered_assets = $this->_running_vars['reordered_assets'];
		}

		if ($this->_running_vars['link_action'] == 'create') {
			// we are creating a new link
			$o->openSection('New Link Confirmation');
				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$o->openField('&nbsp;');
						echo '<p>Please confirm that you are creating new link(s) for the following asset(s) under "'.$new_parent->name.'":</p>';
						echo '<ul>';
						for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
							$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($assets);
							$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current['parentid']);
							echo '<li>"'.$asset->name.'" from under "'.$parent->name.'".</li>';
						}
						echo '</ul>';
						echo '<p>Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.</p>';
				$o->closeField();
			$o->closeSection();

		} elseif ($this->_running_vars['link_action'] == 'move') {

			// we are moving an asset
			$o->openSection('Move Confirmation');

				if (!empty($this->_tmp['message'])) {
					$o->openField('<span style="color: #FF0000;">Error</span>');
						echo $this->_tmp['message'];
					$o->closeField();
				}

				$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

				// Check if any assets were re-ordered outside the HIPO job (ie, parentid == to_parent_id)
				// and print them here to avoid confusion
				if (!empty($reordered_assets)) {
					$o->openField('&nbsp;');
							echo '<p>The following asset(s) have already been re-ordered under "'.$new_parent->name.'". They do not need to be moved again:</p>';
							echo '<ul>';
							for (reset($reordered_assets); null !== ($assetid = key($reordered_assets)); next($reordered_assets)) {
								$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								echo '<li>"'.$asset->name.'".</li>';
							}
							echo '</ul>';
					$o->closeField();
				}

				$o->openField('&nbsp;');
					if ($new_parent->id == $trash_folder->id) {
						echo '<p>Please confirm that you are moving '.count($assets).' asset(s) to the trash.</p>';
					} else {
						echo '<p>Please confirm that you are moving the following asset(s) to under "'.$new_parent->name.'":</p>';
						echo '<ul>';
						for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
							$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							$current = current($assets);
							$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current['parentid']);
							echo '<li>"'.$asset->name.'" from under "'.$parent->name.'".</li>';
						}
					}
					echo '</ul>';
					echo '<p>Press the <b>Next</b> button below to confirm or the <b>Cancel</b> button to abort.</p>';

					if ($new_parent->id == $trash_folder->id) {
						echo '<p>When these asset(s) are moved to the trash, any URLs that they currently have will be broken. Use the asset finder below to select another asset to remap the URL to (Eg. a "Page not found" asset). If you do not want to remap the URLs, just leave the field empty.</p>';
						asset_finder($prefix.'_remap_new_url', null, Array(), 'sq_sidenav', true);
					}
				$o->closeField();


				// Check if the assets have been moved to the trash. If there are any, warn about which links will be affected
				if ($new_parent->id == $trash_folder->id) {
					// when checking for the number of links left in the system, make sure we disregard
					// any link to the trash folder because even if this asset is already in the trash,
					// we should warn about deleting it

					$trash_section_painted = false;

					for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
					//foreach ($this->_running_vars['assets'] as $asset_node) {
						$current = current($assets);
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$trash_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_folder->id, $asset->id, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
						$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_LINK_TYPE_1 + SQ_LINK_TYPE_2, '', true, $current['linkid']);

						if (!empty($trash_link)) $num_other_links--;

						if (!$num_other_links) {

							// now lets get all the other links this asset has in the system and
							// display them to the user
							// note that we are also going to ask the major asset in the link
							// to describe it so it makes sense to the user looking at it
							$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3 + SQ_LINK_NOTICE, '', true, 'minor');

							if (!empty($links)) {
								if (!$trash_section_painted) {
									$o->openSection('Warning - Moving To Trash');
										$o->openField('&nbsp;');
											?><p>Moving these assets to the trash is going to affect other assets in the system. A list of the affected assets is shown below along with a description of their purpose.</p><?php
											$trash_section_painted = true;
								}

								echo $asset->name;

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
							}//end if
						}//end if
					} //end foreach

					if ($trash_section_painted) {
						$o->closeField();
						$o->closeSection();
					}
				}
				$o->closeSection();
			}//end if

		$o->addHiddenField('form_submitted', 1);
		return true;
	}//end paintConfirmation()


	/**
	* Does nothing.
	*
	* @param	&array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processConfirmation(&$step_data, $prefix)
	{
		if ($step_data['percent_done'] == 0) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;

	}//end processConfirmation()


	function processCreateLinks(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!isset($this->_running_vars['process_create_links']))
			 $this->_running_vars['process_create_links'] = $this->_running_vars['assets'];

		$assets = $this->_running_vars['process_create_links'];
		$assetid = key($assets);
		$value = reset($assets);
		unset($this->_running_vars['process_create_links'][$assetid]);

		if ($this->_running_vars['link_action'] == 'create') {
			// create a new link
			$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);

			if (is_null($new_parent)) {
				trigger_error('Cannot create new link, invalid destination asset ID specified (#'.$this->_running_vars['to_parent_assetid'].')', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (is_null($new_parent)) {
				trigger_error('Cannot create new link, invalid asset ID specified ('.$assetid.')', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$step_data['message'] = 'Creating a link between "'.$asset->name.'" and "'.$new_parent->name.'".';

			$linkid = $new_parent->createLink($asset, $value['link_type'], '', $this->_running_vars['to_parent_pos']);
			if (!$linkid) {
				trigger_error('Unable to create a link between "'.$asset->name.'" and "'.$new_parent->name.'".', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$this->_running_vars['to_parent_pos']++;

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_parent);
			unset($new_parent);

		} elseif ($this->_running_vars['link_action'] == 'move') {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($value['linkid'], $value['parentid']);

			$linkid = $GLOBALS['SQ_SYSTEM']->am->moveLink($value['linkid'], $this->_running_vars['to_parent_assetid'], $link['link_type'], $this->_running_vars['to_parent_pos']);

			if (!$linkid) {
				trigger_error('Unable to move asset with ID #'.$assetid.' to the parent with asset ID $#'.$this->_running_vars['to_parent_assetid'], E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_link['minorid'], $new_link['minor_type_code']);

			if (is_null($asset)) {
				trigger_error('Failed to move asset #'.$assetid.'.', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			$new_parent_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->_running_vars['to_parent_assetid']));
			$step_data['message'] = 'Moving "'.$asset->name.'" to "'.$new_parent_info[$this->_running_vars['to_parent_assetid']]['name'].'".';

			$this->_running_vars['to_parent_pos']++;

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			// if we are moving to the trash, we want to skip all the steps that ask us if we want
			// to cascade permissions, metadata, and workflow
			$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			if ($this->_running_vars['to_parent_assetid'] == $trash_folder->id) {
				$this->skipStep(3);
				$this->skipStep(4);
				$this->skipStep(5);
				$this->skipStep(6);
			}
		}

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['process_create_links'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = false;
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		unset($asset);

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

		$assets = $this->_running_vars['assets'];

		$c = 1;
		for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
			$o->addHiddenField('link_assetid['.$c.']',  $assetid);
			$c++;
		}

		$o->addHiddenField('link_parentid', $this->_running_vars['to_parent_assetid']);

		$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['to_parent_assetid']);
		if (is_null($parent)) {
			trigger_error('Asset #'.$this->_running_vars['to_parent_assetid'].' not found', E_USER_WARNING);
			return false;
		}

		$o->openSection('Note');
			$o->openField('&nbsp;');
				$o->addHiddenField('link_completed', '0');

				if (count($this->_running_vars['assets']) > 0) {
					echo '<p><b>The following asset(s) have been successfully linked under "'.$parent->name.'":</b></p>';
					echo '<ul';
					for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid,));
						echo '<li>"'.$asset_info[$assetid]['name'].'"</li>';
					}
					echo '</ul>';
				}

				?>

				<p>The permissions, workflow schemas, and metadata schemas of "<?php echo $parent->name; ?>" are shown below.</p>

				<p>To cascade the permissions, workflow schemas, and/or metadata schemas of "<?php echo $parent->name; ?>" to the newly linked asset(s) and their children, check the appropriate boxes below and press the <b><i>Next</i></b> button.</p>

				<p>If you do not wish to cascade any permissions or schemas, press the <b><i>Next</i></b> button without checking any boxes.</p>
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

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
		unset($parent);

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
				if (!in_array('cascade_pe', $_REQUEST['cascade'])) $this->skipStep(4);
				// metadata
				if (!in_array('cascade_ms', $_REQUEST['cascade'])) $this->skipStep(5);
				//workflow
				if (!in_array('cascade_wf', $_REQUEST['cascade'])) $this->skipStep(6);
			} else {
				$this->skipStep(4); // permissions
				$this->skipStep(5); // metadata
				$this->skipStep(6); // workflow
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

		$assetids = Array();
		$assets = $this->_running_vars['assets'];

		for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
			$assetids[] = key($assets);
		}
		$running_vars['assetids'] = $assetids;

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
			$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->_running_vars['to_parent_assetid'], $perm, null, false, false, true);
			foreach ($set_perms as $userid => $granted) {
				$permission_vars[$perm][] = Array('granted'         => $granted,
												  'userid'          => $userid,
												  'previous_access' => null,
												  );
			}
		}

		$running_vars['permission_changes'] = $permission_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

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
		$schemas = $mm->getSchemas($this->_running_vars['to_parent_assetid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array('granted'         => $granted,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
								   );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

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
		$schemas = $wfm->getSchemas($this->_running_vars['to_parent_assetid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array('granted'         => $granted,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
						      );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assets'] = $this->_running_vars['assets'];

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
		$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$this->_running_vars['to_parent_assetid'].'");');

		foreach ($this->_running_vars['assets'] as $asset_details) {
			$refreshids[] = $asset_details['parentid'];
		}
		$refreshids = array_unique($refreshids);

		foreach ($refreshids as $refreshid) {
			$o->addOnLoad('if (window.opener.parent.frames["sq_sidenav"] && window.opener.parent.frames["sq_sidenav"].reload_asset) window.opener.parent.frames["sq_sidenav"].reload_asset("'.$refreshid.'");');
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
