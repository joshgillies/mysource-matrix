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
* $Id: workflow_schema_edit_fns.inc,v 1.38 2013/02/26 01:39:05 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Workflow_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Schema_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);


	}//end constructor


	public function paintWorkflowStreams(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = (boolean)$asset->writeAccess('links');
		$stream_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_stream', TRUE);
		$streamids = Array();

		hidden_field($prefix.'_new_selected_stream', '');
		if ($write_access === FALSE) {
			$selected_stream = NULL;
		} else {
			$selected_stream = $this->getCurrentStream($prefix);
			if ($selected_stream !== NULL) {
				$selected_stream = (string)$selected_stream->id;
				hidden_field($prefix.'_selected_stream', $selected_stream);
			}
		}

		foreach ($stream_links as $stream_link) {
			if ($stream_link['value'] == 'default_stream') {
				// Push the default stream into the front
				array_unshift($streamids, $stream_link['minorid']);
			} else {
				// Push any other streams into the back, so the default will
				// always be at the "front"
				array_push($streamids, $stream_link['minorid']);
			}
		}


		$streams_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($streamids);

		$selection_rules_strings = Array();
		foreach($streams_info as $streamid => $details) {
			$string = '';
			$stream = $GLOBALS['SQ_SYSTEM']->am->getAsset($streamid);
			$rules = is_array($stream->attr('stream_selection_rules')) ? $stream->attr('stream_selection_rules') : unserialize($stream->attr('stream_selection_rules'));
			if(empty($rules)) continue;
			$logic = $stream->attr('rules_logic');
			$string .= $logic === 'all' ? translate('Match on ALL rules:') : translate('Match on ANY rule:');


			$string .= '<ul>';
			foreach ($rules as $rule) {
				$string .= '<li>';
				$string .= sprintf(translate('%1$s matches "%2$s"'), $rule['keyword'], $rule['pattern']);

				$string .= '</li>';
			}
			$string .= '</ul>';
			$selection_rules_strings[$streamid] = $string;
		}


		?>
			<table class="sq-backend-table">
			<thead>
				<colgroup>
					<col width="70" />
					<col />
					<col />
					<col />
					<?php
					if ($write_access === TRUE) {
						?><col width="70" /><?php
					}
					?>
				</colgroup>
				<tr>
					<th><p><?php echo translate('Edit?'); ?></p></th>

					<th><?php echo translate('Stream Name'); ?></th>

					<th><?php echo translate('Workflow Summary'); ?></th>

					<th><?php echo translate('Selection Rules'); ?></th>

					<?php
					if ($write_access === TRUE) {
						?><th style="text-align:center"><b><?php echo translate('Delete?'); ?></b></th><?php

					}
					?>
				</tr>
			</thead>
			<tbody>
			<?php
			// The default stream is the first one on the list
			$alt_stream_num = 0;
			foreach ($streamids as $streamid) {
				$stream_info = $streams_info[$streamid];
				$streamid = (string)$streamid;

				// Get an abbreviated summary of steps - less full than that
				// of an edit order
				ob_start();
				$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($streamid, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);

				if (count($current_links) > 0) {
					echo '<table width="100%" style="border-collapse: collapse">';
					echo '<thead><colgroup>';
					echo '<col width="200" /><col width="*" /><col width="250"/>';
					echo '</colgroup></thead><tbody>';
					$current_step_num = 0;
					foreach ($current_links as $link) {
						$step_assetid = $link['minorid'];

						// Okay, now at current depth, so increment step number;
						$current_step_num++;

						$step_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($step_assetid);

						echo '<tr>';

						$border_style = '';
						if ($current_step_num === count($current_links)) {
							$border_style = ' style="border-bottom: 0;"';
						}

						echo '<td'.$border_style.'>';
						echo '<strong>'.$current_step_num.'.</strong> '.$step_asset->name.'</strong></td>';

						echo '<td'.$border_style.'>';
						$conditions = $GLOBALS['SQ_SYSTEM']->am->getLinks($step_assetid, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE);

						if (count($conditions) === 0) {
							echo '<span class="sq-backend-warning">'.translate('No conditions selected').'</span>';
						} else {
							if (($step_asset->attr('logic') === 'all') || ($step_asset->attr('logic_num') == count($conditions))) {
								// Nothing, just say what conditions there are.
							} else {
								echo '<em>'.sprintf(
									translate('At least %s condition(s) of:'),
									$step_asset->attr('logic_num')
								).'</em> ';
							}
						}

						$condition_names = Array();
						foreach ($conditions as $condition) {
							if (count($condition_names) > 3) {
								$condition_names[] = '...';
								break;
							}
							$userids = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('userid', 'workflow_step_condition', Array($condition['minorid']));
							$userid = $userids[$condition['minorid']];
							$user_name = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid);
							if (empty($user_name)) {
								$dynamic_userids = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('dynamic_userid', 'workflow_step_condition', Array($condition['minorid']));
								$dynamic_userid = $dynamic_userids[$condition['minorid']];
								if(empty($dynamic_userid)) {
									trigger_localised_error('SYS0348', sprintf(translate('User asset #%1$s set for the workflow step condition #%2$s does not exist'), $userid, $condition['minorid']), E_USER_NOTICE);
									continue;
								}
								$condition_name = $dynamic_userid;
							}
							else {
								$condition_name = $user_name[$userid]['name'].' (Id: #'.$userid.')';
							}

							if (!empty($user_name) && $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($user_name[$userid]['type_code'], 'user_group') === TRUE) {
								$logic = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('logic', 'workflow_step_condition', Array($condition['minorid']));
								$logic_num = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('logic_num', 'workflow_step_condition', Array($condition['minorid']));
								if ($logic[$condition['minorid']] === 'all') {
									$condition_name = sprintf(translate('All of %s'), $condition_name);
								} else {
									$condition_name = sprintf(translate('At least %s of %s'), $logic_num[$condition['minorid']], $condition_name);
								}
							}

							$condition_names[] = $condition_name;
						}
						echo implode('; ', $condition_names);

						echo '</td>';

						echo '<td'.$border_style.'><div>';
						$expiry = (int)$step_asset->attr('expiry_time');
						$expiry_text = easy_time_total($expiry);

						if ($expiry > 0) {
							if ($step_asset->attr('escalate')) {
								$esc_links = array_reverse($GLOBALS['SQ_SYSTEM']->am->getLinks($step_assetid, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE));
								echo sprintf(translate('<span class="sq-backend-warning">Escalates</span> after %1$s, %2$s escalation step(s)'), $expiry_text, count($esc_links));

							} else {
								echo sprintf(translate('Reminder sent after %1$s'), $expiry_text);

							}
						}

						echo '</div></td>';
						echo '</tr>';
					}

					// Add a drop point after the last main step
					echo '</tbody></table>';
				} else {
					echo '<span class="sq-backend-warning">'.translate('No workflow defined').'</span>';
				}

				$workflow_summary = ob_get_clean();

				$class_style = 'sq-table-row';
				if ($selected_stream === $streamid) {
					$class_style .= ' alt';
				}
				if ($alt_stream_num === 0) {
					?><tr class="sq-table-row"><td class="sq-backend-table-header-header" colspan="<?php echo 4 + ($write_access ? 1 : 0) ?>"><?php echo translate('Default Stream'); ?></td></tr><?php

				} else if ($alt_stream_num === 1) {
					?><tr class="sq-table-row"><td class="sq-backend-table-header-header" colspan="<?php echo 4 + ($write_access ? 1 : 0) ?>"><?php echo translate('Alternate Streams'); ?></td></tr><?php

				}
			?>
				<tr class="<?php echo $class_style ?>">
				<td><?php if (($write_access === TRUE) && ($selected_stream !== $streamid)) {
						?><p><a href="#" onclick="document.getElementById('<?php echo $prefix; ?>_new_selected_stream').value = <?php echo $streamid ?>; document.main_form.submit(); return false;"><?php echo trim(sq_get_icon(sq_web_path('lib').'/web/images/icons/edit_mode.png', 15, 15, $stream_info['name'], NULL, ' border="0"')); ?></a></p><?php
					} ?></td>
					<td><p><strong><?php echo $stream_info['name'] ?></strong></p></td>
					<td><?php echo $workflow_summary; ?></td>
					<td><?php if(isset($selection_rules_strings[$streamid])) echo ($selection_rules_strings[$streamid]);?></td>
					<?php
					if ($write_access === TRUE) {
						if ($alt_stream_num === 0) {
							?><td style="text-align:center" class="sq-backend-table-header"></td><?php
						} else {
							?><td style="text-align:center" class="sq-backend-table-header"><?php echo check_box($prefix.'_delete_stream[]', $streamid, FALSE); ?></td><?php
						}
					}
					?>
				</tr>
			<?php
				// After we have gone through once, the rest are non-default.
				$alt_stream_num++;
			}

			if (count($streams_info) <= 1) {
				?><tr class="sq-table-row"><td class="sq-backend-table-header-header" colspan="<?php echo 4 + ($write_access ? 1 : 0) ?>"><?php echo translate('Alternate Streams'); ?></td></tr><?php

				?><tr class="sq-table-row"><td></td><td colspan="<?php echo 3 + ($write_access ? 1 : 0) ?>"><?php
					echo '<strong>'.translate('There are no alternate streams defined.').'</strong> ';
					if ($write_access) {
				 		echo translate('You can create a new alternate stream using the interface below.');
				 	} else{
				 		echo translate('Lock the Workflow Schema to create a new alternate stream.');
					}
				 ?></td></tr><?php
			}

			if ($write_access === TRUE) {
				?><tr class="sq-table-row"><td class="sq-backend-table-header-header" colspan="5">'.<?php echo translate('Add New Workflow Stream'); ?></td></tr><?php
			?>
				<tr>
					<td></td>
					<td colspan="4"><?php
						check_box($prefix.'_new_stream', '1', FALSE, 'document.getElementById("'.$prefix.'_new_stream_name").disabled = !this.checked; if (this.checked == true) { document.getElementById("'.$prefix.'_new_stream_name").focus() };');
						label(translate('Create a new stream called:'), $prefix.'_new_stream');

						text_box($prefix.'_new_stream_name', '', 30, 0, FALSE, ' disabled="disabled"');
					?></td>
				</tr>
			<?php
			}
			?>
			</tbody>
			</table>
		<?php
		return FALSE;

	}//end paintWorkflowStreams()


	public function processWorkflowStreams(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		// Creating a stream
		if ((int)array_get_index($_POST, $prefix.'_new_stream', 0) === 1) {
			$new_stream_name = trim(array_get_index($_POST, $prefix.'_new_stream_name', ''));
			if ($new_stream_name === '') return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_stream');
			$new_stream = new Workflow_Stream();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_stream->setAttrValue('name', $new_stream_name);
			if (!$new_stream->create($schema_link)) return FALSE;
			$_POST[$prefix.'_new_selected_stream'] = $new_stream->id;
		}

		// Removing a stream - note that we cannot remove the default stream,
		// and that removing a stream only removes it from the current schema,
		// not delete it entirely. Therefore, it is possible for streams to be
		// sitting in the trash if the last link is removed.
		$remove_streams = array_get_index($_POST, $prefix.'_delete_stream', Array());
		foreach ($remove_streams as $remove_stream) {
			$stream_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $remove_stream, SQ_LINK_TYPE_2);
			if (empty($stream_link) === FALSE) {
				if ($stream_link['value'] !== 'default_stream') {
					if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($stream_link['linkid'])) {
						return FALSE;
					}
					if (array_get_index($_POST, $prefix.'_selected_stream', NULL) === $remove_stream) {
						unset($_POST[$prefix.'_selected_stream']);
					}
					if (array_get_index($_POST, $prefix.'_new_selected_stream', NULL) === $remove_stream) {
						unset($_POST[$prefix.'_new_selected_stream']);
					}
				}
			}
		}

		return TRUE;

	}//end processWorkflowStreams()


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNewStep(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getCurrentStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->paintNewStep($selected_stream, $o, $prefix);

	}//end paintNewStep()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNewStep(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getPriorStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->processNewStep($selected_stream, $o, $prefix);

	}//end processNewStep()


	/**
	* Paint the interface for printing the workflow summary,
	* which will also allow re-ordering of the page.
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintWorkflowSummary(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getCurrentStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->paintWorkflowSummary($selected_stream, $o, $prefix);

	}//end paintWorkflowSummary()


	/**
	* Paint the interface for printing the workflow summary,
	* which will also allow re-ordering of the page.
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processWorkflowSummary(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getPriorStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->processWorkflowSummary($selected_stream, $o, $prefix);

	}//end processWorkflowSummary()


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintSteps(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getCurrentStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->paintSteps($selected_stream, $o, $prefix);

	}//end paintSteps()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processSteps(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getPriorStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->processSteps($selected_stream, $o, $prefix);

	}//end processSteps()


	/**
	* Paint the interface for adding restrictions
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintWorkflowRestrictions(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('Current Restrictions'));

			$restrictions = is_array($asset->attr('schema_restrictions')) ? $asset->attr('schema_restrictions') : unserialize($asset->attr('schema_restrictions'));
			if (!empty($restrictions)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><?php echo translate('Asset Type'); ?></td>

						<td style="text-align: center;" class="sq-backend-table-header" width="70"><?php echo translate('Inherit').'?'; ?></td>

						<?php
						if ($write_access) {
						?>
						<td style="text-align: center;" class="sq-backend-table-header" width="70"><?php echo translate('Remove').'?'; ?></td>

						<?php
						}
						?>
					</tr>
					<?php
					foreach ($restrictions as $asset_type => $inheritance) {
						?>
						<tr>
							<td class="sq-backend-table-cell">
								<?php
									get_asset_type_icon($asset_type);
									echo ucfirst(str_replace('_', ' ', $asset_type));
								?>
							</td>
							<td style="text-align:center" class="sq-backend-table-cell">
								<?php
								if ($write_access) {
									check_box($prefix.'_asset_type_inherit['.$asset_type.']', '1', $inheritance);
								} else {
									echo '<img src="'.sq_web_path('lib').'/web/images/'.($inheritance ? 'tick' : 'cross').'.png" width=" 15" height="15" />';
								}
								?>
							</td>
							<?php
							if ($write_access) {
							?>
							<td style="text-align:center" class="sq-backend-table-cell">
								<?php check_box($prefix.'_remove_asset_type['.$asset_type.']'); ?>
							</td>
							<?php
							}
							?>
						</tr>
						<?php
					}//end foreach()
				?>
				</table>

				<?php
			} else {
				echo '<em>'.translate('No Restrictions').'</em>';

			}
		$o->closeField();

		if ($write_access) {
			// New Restrictions
			$o->openField(translate('New Restriction'));

				asset_type_chooser($prefix.'_add_restriction', TRUE, Array(), NULL, FALSE, TRUE);
			$o->closeField();
		}

	}//end paintSteps()


	/**
	* Processes the interface for adding wrestrictions
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processWorkflowRestrictions(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$prefix = $asset->getPrefix();
		$updated = FALSE;

		// handle restrictions
		// delete
		$current_restrictions = is_array($asset->attr('schema_restrictions')) ? $asset->attr('schema_restrictions') : unserialize($asset->attr('schema_restrictions'));
		$changes_made = FALSE;
		if (isset($_POST[$prefix.'_remove_asset_type'])) {
			foreach($_POST[$prefix.'_remove_asset_type'] as $remove_type => $ahfortheheckofit) {
				if (array_key_exists($remove_type, $current_restrictions)) {
					unset($current_restrictions[$remove_type]);
					$changes_made = TRUE;
				}
			}

			// If something to save, save it, dammit!
			if ($changes_made) {
				$asset->setAttrValue('schema_restrictions', serialize($current_restrictions));
				$asset->saveAttributes();
			}
		}

		// inherit type setting
		$current_restrictions = is_array($asset->attr('schema_restrictions')) ? $asset->attr('schema_restrictions') : unserialize($asset->attr('schema_restrictions'));
		$changes_made = FALSE;
		$inherit_enabled = isset($_POST[$prefix.'_asset_type_inherit']) ? $_POST[$prefix.'_asset_type_inherit'] : Array();
		foreach($current_restrictions as $asset_type => $inheritance) {
			if (isset($inherit_enabled[$asset_type])) {
				$current_restrictions[$asset_type] = 1;
			} else {
				$current_restrictions[$asset_type] = 0;
			}
			if ($inheritance != array_get_index($inherit_enabled, $asset_type, 0)) {
				$changes_made = TRUE;
			}

		}//end foreach

		if ($changes_made) {
			$asset->setAttrValue('schema_restrictions', serialize($current_restrictions));
			$asset->saveAttributes();
		}

		// add
		$current_restrictions = is_array($asset->attr('schema_restrictions')) ? $asset->attr('schema_restrictions') : unserialize($asset->attr('schema_restrictions'));
		$changes_made = FALSE;
		if (isset($_POST[$prefix.'_add_restriction']) && (isset($_POST[$prefix.'_add_restriction']['type_code']))) {
			foreach($_POST[$prefix.'_add_restriction']['type_code'] as $field => $type_code) {
				if (!empty($type_code)) {
					$inheritance = $_POST[$prefix.'_add_restriction']['inherit'][$field];
					if (!isset($current_restrictions[$type_code])) {
						$current_restrictions[$type_code] = $inheritance;
						$changes_made = TRUE;
					} else if ($current_restrictions[$type_code] !== $inheritance) {
						$current_restrictions[$type_code] = $inheritance;
						$changes_made = TRUE;
					}
				}
			}

			// If something to save, save it, dammit!
			if ($changes_made) {
				$asset->setAttrValue('schema_restrictions', serialize($current_restrictions));
				$asset->saveAttributes();
			}
		}

		return TRUE;

	}//end processSteps()


	/**
	* Generate an array representing the workflow structure for the schema at this point in time
	*
	* The generated array is a snap shot of the schema at a point in time that can get written
	* to the database and used for publishing etc.
	*
	* @param Asset	$asset	the asset we are generating the array for
	*
	* @return array
	* @access public
	*/
	public function generateWorkflowArray(Asset $asset, $top_parent='')
	{
		$workflow_array['steps']        = Array();
		$workflow_array['current_step'] = Array('1');
		$workflow_array['complete']     = FALSE;
		$workflow_array['schema_name']  = $asset->name;
		$workflow_array['started']      = time();
		$workflow_array['started_by']   = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$i = 1;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$step = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;
			$edit_fns = $step->getEditFns();
			$workflow_array['steps'][$i] = Array();
			if (!$edit_fns->generateWorkflowArray($step, $workflow_array['steps'][$i], $top_parent)) {
				return FALSE;
			}
			if ($i == 1) {
				$workflow_array['steps'][$i]['started'] = time();
			}
			$i++;
		}
		return $workflow_array;

	}//end generateWorkflowArray()


	public function getPriorStream($prefix)
	{
		$selected_stream = array_get_index($_POST, $prefix.'_selected_stream', NULL);

		if ($selected_stream === NULL) {
			return NULL;
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($selected_stream);
		}

	}//end getCurrentStream();


	public function getCurrentStream($prefix)
	{
		$selected_stream = array_get_index($_POST, $prefix.'_new_selected_stream', NULL);

		if (empty($selected_stream) === TRUE) {
			$selected_stream = array_get_index($_POST, $prefix.'_selected_stream', NULL);
			if (empty($selected_stream) === TRUE) return NULL;
		} else {
		}

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($selected_stream);

	}//end getCurrentStream();


	public function hasSelectedCurrentStream(Asset $asset, $prefix)
	{
		$selected_stream = $this->getCurrentStream($prefix);
		return ($selected_stream !== NULL);

	}//end hasSelectedCurrentStream()


	public function hasSelectedPriorStream(Asset $asset, $prefix)
	{
		$selected_stream = $this->getPriorStream($prefix);
		return ($selected_stream !== NULL);

	}//end hasSelectedPriorStream()


	//--        WORKFLOW SCHEMA USAGE        --//


	public function paintUsage(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		$page_size = 10;

		$o->openRaw();

		$schema_treeids_sql = 'SELECT
								treeid, assetid, granted, cascades
								FROM (sq_ast_lnk_tree ta
									JOIN sq_ast_lnk la ON ta.linkid = la.linkid
										JOIN sq_ast_wflow wa ON la.minorid = wa.assetid)
								WHERE wa.schemaid = :schemaid
								AND NOT EXISTS (
									SELECT * FROM (sq_ast_lnk_tree tb
													JOIN sq_ast_lnk lb ON tb.linkid = lb.linkid
														JOIN sq_ast_wflow wb ON lb.minorid = wb.assetid)
									WHERE wb.schemaid = wa.schemaid
									AND length(ta.treeid) - LENGTH(tb.treeid) = :tree_size AND SUBSTR(ta.treeid, 1, LENGTH(ta.treeid) - :tree_size_1) = tb.treeid
									AND wa.granted = wb.granted
									AND wa.cascades = wb.cascades
								) ORDER BY length(treeid), treeid';

		$query = MatrixDAL::preparePdoQuery($schema_treeids_sql);
		MatrixDAL::bindValueToPdo($query, 'schemaid', $asset->id);
		MatrixDAL::bindValueToPdo($query, 'tree_size', SQ_CONF_ASSET_TREE_SIZE);
		MatrixDAL::bindValueToPdo($query, 'tree_size_1', SQ_CONF_ASSET_TREE_SIZE);
		$treeids = MatrixDAL::executePdoAssoc($query);

		if (count($treeids) > 0) {
			$tree_tops = $this->_getTreeTops($asset, $treeids);

			$num_pages = ceil(count($tree_tops) / $page_size);
			$page_num = array_get_index($_POST, $prefix.'_usage_page', 1);
			$page_num = max($page_num, 1);
			$page_num = min($page_num, $num_pages);

			if ($num_pages == 1) {
				$page_tag = sprintf(translate('Page %1$s of %2$s'), $page_num, $num_pages);

			} else {
				$page_tag = sprintf(translate('Page <a style="color: #fff" href="#" onclick="return sq_pager_jump(\'%3$s\', sq_pager_prompt(1, %2$s));">%1$s</a> of %2$s'), $page_num, $num_pages, $prefix.'_usage_page');

			}
			$asset_count_tag = sprintf(translate('Displaying <strong>%1$s</strong> - <strong>%2$s</strong> of <strong>%3$s</strong> total trees'), ($page_num - 1) * $page_size + 1, min(count($tree_tops), $page_num * $page_size), count($tree_tops));



			$tree_tops = array_slice($tree_tops, ($page_num - 1) * $page_size, $page_size);

			$links = Array(
						'first'		=> '&lt;&lt;',
						'previous'	=> '&lt;',
						'page'		=> $page_tag,
						'next'		=> '&gt;',
						'last'		=> '&gt;&gt;',
					);

			if ($page_num > 1) {
				$links['first'] = '<a title="'.translate('Go to the first page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', 1)">'.$links['first'].'</a>';

				$links['previous'] = '<a title="'.translate('Go to the previous page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';

			} else {
				$links['first'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['first'].'</span>';

				$links['previous'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['previous'].'</span>';

			}

			if ($page_num < $num_pages) {
				$links['last'] = '<a title="'.translate('Go to the last page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.$num_pages.')">'.$links['last'].'</a>';

				$links['next'] = '<a title="'.translate('Go to the next page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num + 1).')">'.$links['next'].'</a>';

			} else {
				$links['last'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['last'].'</span>';

				$links['next'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['next'].'</span>';

			}

			hidden_field($prefix.'_usage_page', $page_num); ?>
			<table class="sq-backend-table">
			<thead>
			<tr>
				<td class="sq-backend-table-header-header"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></td>
				<td class="sq-backend-table-header-header" colspan="2" style="text-align: right; font-weight: normal"><?php echo $asset_count_tag; ?></span></td>
			</tr>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-cell"><?php echo translate('Asset'); ?></th>

				<th style="width: 15%" class="sq-backend-table-cell"><?php echo translate('Granted?') ?></th>

				<th style="width: 15ex" class="sq-backend-table-cell"><?php echo translate('Cascade?') ?></th>

			</tr>
			</thead>
			<tbody>
			<?php

			foreach ($tree_tops as $tt_assetid => &$tree_top_info) {
				$root_tree = $tree_top_info;
				unset($root_tree['sub_trees']);
				$root_trees = array_merge(Array($root_tree), $tree_top_info['sub_trees']);

				foreach ($root_trees as $root_assetid => &$tree_info) {

					?><tr class="sq-backend-table-row">
						<td class="sq-backend-table-cell">
						<?php

						$tag_line = get_asset_tag_line($tree_info['assetid'], 'details');

						echo ($tree_info['depth'] === 0 ? '' : '<span style="padding-left: '.($tree_info['depth']*5).'ex">');
						echo sprintf(translate('%1$s'), $tag_line);

						echo ($tree_info['depth'] === 0 ? '' : '</span>');

						?></td>
						<td class="sq-backend-table-cell">
							<strong><span style="color: <?php echo (($tree_info['granted'] == '0') ? 'red' : 'green'); ?>"><?php echo (($tree_info['granted'] == '0') ? translate('Denied') : translate('Applied')) ?></span></strong>


						</td>
						<td class="sq-backend-table-cell">
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($tree_info['cascades'] === '1' ? 'tick' : 'cross'); ?>.png" width="15" height="15" style="vertical-align: middle;" alt="<?php echo ($tree_info['cascades'] === '1' ? translate('Yes') : translate('No')); ?>" />
						</td>
					</tr>
					<?php
				}//end foreach entry

				unset($tree_info);

			}//end foreach top tree
			?>

			</tbody>
			</table>
			<?php

			$o->note(translate('<p>This screen shows which assets have this schema either applied or denied.</p>

	<p>Where the applying or denying of the schema is cascaded unaffected through a tree of assets, only the top asset of that tree will be displayed. However, if any children underneath the asset do anything different to its parent - such as denying the schema where its parent had applied (or vice-versa), or it no longer cascades to new assets - a child entry in the table is created.</p>'));


		} else {
			$o->note(translate('<p>This schema is not applied to any assets.</p>'));

		}

		$o->closeRaw();

		return FALSE;

	}//end paintUsage()


	/**
	* Get the "tree tops" in the supplied list of assets
	*
	* An asset is a "tree top" in a set of assets if none of its ancestors
	* in the asset tree are in the set with it.  Conversely if an asset's parent
	* or grandparent etc is in the set, then that asset is not a tree top.
	*
	* Returns an array indexed by treeid, where each value is another array
	* containing assetid and number of children of that assetid found in
	* the set.
	*
	* @param array	$assetids	The assetids we are analysing
	*
	* @return array
	* @access public
	*/
	public function _getTreeTops(Workflow_Schema $asset, Array $treeids)
	{
		$used_assetids = Array();

		$all_treeids = Array();
		foreach ($treeids as $treeid) {
			$all_treeids[$treeid['treeid']] = $treeid;
		}
		unset($treeids);

		$res = Array();
		foreach ($all_treeids as $tree_info) {
			$treeid = $tree_info['treeid'];
			$treeid_copy = $treeid;
			$top_parent  = $treeid;
			$depth       = 0;
			$used_sub_tree = FALSE;

			while (empty($treeid_copy) === FALSE) {
				if (isset($res[$treeid_copy]) === TRUE) {
					if ($used_sub_tree === FALSE) {
						$used_sub_tree = TRUE;
					}
				}

				if (($used_sub_tree === TRUE) && (array_key_exists($treeid_copy, $all_treeids) === TRUE)) {
					$top_parent = $treeid_copy;
					$depth++;
				}

				$treeid_copy = substr($treeid_copy, 0, -SQ_CONF_ASSET_TREE_SIZE);
			}

			$res[$treeid] = $tree_info;
			$res[$treeid]['top_parent'] = $top_parent;
			$res[$treeid]['depth'] = $depth;
			$res[$treeid]['sub_trees'] = Array();

			// Have we seen this asset before? If so, let it go, but mark it
			// to be dumped afterwards so we only have one of each asset
			if (array_search($tree_info['assetid'], $used_assetids) === FALSE) {
				$res[$treeid]['dupe'] = FALSE;
				$used_assetids[] = $tree_info['assetid'];
			} else {
				$res[$treeid]['dupe'] = TRUE;
			}
		}

		foreach ($res as $key => &$res_item) {
			if ($res_item['dupe'] === TRUE) {
				unset($res[$key]);
			} else {
				if ($res_item['depth'] > 0) {
					$res[$res_item['top_parent']]['sub_trees'][$res_item['treeid']] =& $res_item;
					unset($res[$key]);
				}
			}
		}

		// Sort each sub-tree top-to-bottom, to ensure correct tree order
		foreach ($res as $key => &$res_item) {
			uksort($res_item['sub_trees'], create_function('$key1,$key2', 'return strcmp($key1, $key2);'));
		}

		unset($res_item);

		return $res;

	}//end _getTreeTops()



	/**
	* Paint the interface for stream rules setup
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintStreamSelectionRules(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getCurrentStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->paintStreamSelectionRules($selected_stream, $o, $prefix);

	}//end paintStreamSelectionRules()


	/**
	* Paint the interface for stream rules setup
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processStreamSelectionRules(Workflow_Schema $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$selected_stream = $this->getPriorStream($prefix);
		if ($selected_stream === NULL) return FALSE;

		$edit_fns = $selected_stream->getEditFns();
		return $edit_fns->processStreamSelectionRules($selected_stream, $o, $prefix);

	}//end processStreamSelectionRules()




}//end class

?>
