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
* $Id: workflow_stream_edit_fns.inc,v 1.2 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Workflow_Stream_Edit_Fns
*
* Purpose
*    Edit functions file for the Workflow Stream asset.
*
* @see Asset_Edit_Fns
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Stream_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param Workflow_Stream	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNewStep(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		echo translate('Create a new step called').' ';

		text_box($prefix.'_new_step', '', 20);
		return TRUE;

	}//end paintNewStep()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param Workflow_Stream	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNewStep(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		if (isset($_POST[$prefix.'_new_step']) && trim($_POST[$prefix.'_new_step'])) {
			$new_step_name = trim($_POST[$prefix.'_new_step']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step');
			$new_step = new Workflow_Step();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_step->setAttrValue('name', $new_step_name);
			if (!$new_step->create($schema_link)) return FALSE;
		}

		return TRUE;

	}//end processNewStep()


	/**
	* Paint the interface for printing the workflow summary,
	* which will also allow re-ordering of the page.
	*
	* @param Workflow_Stream	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintWorkflowSummary(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		$o->addCssInclude(sq_web_path('data').'/asset_types/'.$asset->type().'/css/workflow_stream_edit_fns.css');
		$o->addJsInclude(sq_web_path('data').'/asset_types/'.$asset->type().'/js/workflow_stream_edit_fns.js');
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$write_access = $asset->writeAccess('links');
		if ($write_access) {
			$o->addOnLoad('MatrixWorkflowSchema.onLoad();');
		}

			// show the workflow summary, and allow us to reorder it
			$steps_to_process = Array(
									Array(
										'assetid'	=> $asset->id,
										'depth' 	=> -1,
									)
								);

			$step_order = Array();

			while (count($steps_to_process) > 0) {
				$current_step = array_shift($steps_to_process);
				if ($current_step['depth'] >= 0) {
					$step_order[] = $current_step;
				}
				$current_links = array_reverse($am->getLinks($current_step['assetid'], SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE));
				foreach ($current_links as $current_link) {
					$new_step = Array(
									'assetid'	=> $current_link['minorid'],
									'depth'		=> $current_step['depth'] + 1,
								);
					array_unshift($steps_to_process, $new_step);
				}
			}

			if (count($step_order) > 0) {
				$step_nums = Array(0);
				$current_depth = 0;
				foreach ($step_order as $step) {
					if ($step['depth'] > $current_depth) {
						// This step is deeper, ie. starting a new escalation step
						array_push($step_nums, 0);
						echo '<div class="sq-workflow-step-escalation-indent">';
						$current_depth = $step['depth'];
					}

					while ($step['depth'] < $current_depth) {
						// exiting an escalation step... but we need to be able to exit more than one at once
						array_pop($step_nums);
						$current_depth--;
						echo '<div class="sq-workflow-step-divider"></div>';
						echo '</div>';
					}

					echo '<div class="sq-workflow-step-divider"></div>';

					// Okay, now at current depth, so increment step number;
					$current_step_num = array_pop($step_nums);
					$current_step_num++;
					array_push($step_nums, $current_step_num);

					$step_assetid = $step['assetid'];
					$step_asset =& $am->getAsset($step_assetid);

					echo '<div id="'.$prefix.'workflow-step-'.$step_assetid.'" class="sq-workflow-step">';
					if ($write_access) {
						hidden_field($prefix.'_step_order[]', $step_assetid);
					}

					echo '<div class="sq-workflow-step-drag">';
					if ($write_access) {
						sq_print_icon(sq_web_path('data').'/asset_types/'.$asset->type().'/files/wf_drag_handle.png', 16, 16, 'Re-Order', 'Re-Order Step', 'align="absmiddle" class="sq-workflow-step-drag-handle"');
					}
					echo 'Step '.implode('.', $step_nums).'</div>';
					echo '<div class="sq-workflow-step-tag-line">'.$step_asset->name.'</div>';
					echo '<div class="sq-workflow-step-condition"><strong>Conditions:</strong> ';
						$conditions = $am->getLinks($step_assetid, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE);
						if (count($conditions) === 1) {
							$condition = reset($conditions);
							$userids = $am->getAttributeValuesByName('userid', 'workflow_step_condition', Array($condition['minorid']));
							$userid = $userids[$condition['minorid']];
							if(empty($userid)) {
								$dynamic_userids = $am->getAttributeValuesByName('dynamic_userid', 'workflow_step_condition', Array($condition['minorid']));
								echo $dynamic_userids[$condition['minorid']];
							}
							else {
								echo get_asset_tag_line($userid);
							}
						} else if (count($conditions) === 0) {
							echo '<span class="sq-backend-warning">None selected</span>';
						} else {
							if (($step_asset->attr('logic') === 'all') || ($step_asset->attr('logic_num') == count($conditions))) {
								echo 'All '.count($conditions);
							} else {
								echo 'At least '.$step_asset->attr('logic_num').' of '.count($conditions);
							}
						}
					echo '</div>';

					echo '<div class="sq-workflow-step-expiry"><strong>Expiry:</strong> ';
						$expiry = (int)$step_asset->attr('expiry_time');
						if ($expiry === 0) {
							echo 'None set';
						} else {
							if ($step_asset->attr('escalate')) {
								echo '<span class="sq-backend-warning">Escalates</span>';
							} else {
								echo 'Reminder';
							}
							echo ' after ';

							$hours = ($expiry / 3600);

							echo easy_time_total($expiry);
						}

					echo '</div>';

					if ($step['depth'] === 0) {
						$step_url = '#step-'.$step_assetid;
					} else {
						$step_url = $step_asset->getBackendHref('details');
					}
					echo '<div class="sq-workflow-step-edit-link">[ <a href="'.$step_url.'">Edit this step</a> ]</div>';
					echo '<div style="clear: both"></div>';
					echo '</div>';
				}

				// Do we have any overhanging escalation steps for the last main step?
				while ($current_depth > 0) {
					echo '<div class="sq-workflow-step-divider"></div>';
					echo '</div>';
					$current_depth--;
				}

				// Add a drop point after the last main step
				echo '<div class="sq-workflow-step-divider"></div>';
			}


		return $write_access;

	}//end paintWorkflowSummary()


	/**
	* Paint the interface for printing the workflow summary,
	* which will also allow re-ordering of the page.
	*
	* @param Workflow_Stream	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processWorkflowSummary(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$new_step_order = array_get_index($_POST, $prefix.'_step_order', Array());
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$links_to_build = Array($asset->id);
		$link_assetids = Array();

		while (count($links_to_build) > 0) {
			$assetid = array_shift($links_to_build);
			$current_links = $am->getLinks($assetid, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);

			// If we have at least 2 links, we can at least move it
			// Even if only 1 though, we need to see if it has multiple
			// escalation steps
			foreach ($current_links as $link) {
				$links_to_build[] = $link['minorid'];
				if (count($current_links) >= 2) {
					$new_index = array_search($link['minorid'], $new_step_order);
					$link_assetids[$assetid][$new_index] = $link['minorid'];
				}
			}
		}

		foreach (array_keys($link_assetids) as $link_key) {
			// Sort keys, then use PHP's rule about numeric indices to get the
			// proper sort order for each parent
			ksort($link_assetids[$link_key]);
			$link_assetids[$link_key] = array_values($link_assetids[$link_key]);
			foreach ($link_assetids[$link_key] as $sort_order => $minor_assetid) {
				$link = $am->getLinkByAsset($link_key, $minor_assetid, SQ_LINK_TYPE_2);
				if ($sort_order != $link['sort_order']) {
					// If the sort order is different, then update it
					$am->moveLinkPos($link['linkid'], $sort_order);
				}
			}
		}

		return TRUE;

	}//end processWorkflowSummary()


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param Workflow_Stream	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintSteps(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('links');
		$ok = $write_access;

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		$i = 1;
		foreach ($current_links as $link_data) {
			$step = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;

			$o->openSection('Step '.$i.' - '.$step->name);
				if ($write_access) {
					$o->openField('Delete ?');
						check_box($prefix.'_delete_steps['.$link_data['linkid'].']');
						label(translate('Delete this step and all its conditions?'), $prefix.'_delete_steps['.$link_data['linkid'].']');

					$o->closeField();
				}
				$edit_fns = $step->getEditFns();
				if (!$edit_fns->paintStep($step, $o)) $ok = FALSE;
			$o->closeSection();

			$i++;
		}

		return $ok;

	}//end paintSteps()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param Workflow_Stream	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processSteps(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;
		$updated = FALSE;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$step = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;
			$edit_fns = $step->getEditFns();
			if ($edit_fns->processStep($step, $o)) $updated = TRUE;
		}

		// delete any steps that need deleting
		if (isset($_POST[$prefix.'_delete_steps'])) {
			$delete_steps = $_POST[$prefix.'_delete_steps'];
			foreach ($delete_steps as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = TRUE;
			}
		}

		return $updated;

	}//end processSteps()


	/**
	* Generate an array representing the workflow structure for the schema at this point in time
	*
	* The generated array is a snap shot of the schema at a point in time that can get written
	* to the database and used for publishing etc.
	*
	* @param Workflow_Stream	$asset	the asset we are generating the array for
	* @param int				$top_parent	the dependant top parent of current asset
	*
	* @return array
	* @access public
	*/
	public function generateWorkflowArray(Workflow_Stream $asset, $top_parent='')
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



	/**
	* Paint the interface for config rules of automatic stream selection
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintStreamSelectionRules(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$prefix .= '_automatic_stream';
		$o->openField($wa ? translate('Current Rules') : '');

			$current_rules = is_array($asset->attr('stream_selection_rules')) ? $asset->attr('stream_selection_rules') : unserialize($asset->attr('stream_selection_rules'));
			$keyword_datas = $current_rules;

			if (!empty($keyword_datas)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th><?php echo translate('Keyword'); ?></th>

						<th><?php echo translate('Pattern') ?></th>

					<?php
					if ($wa) {
						?>
						<th style="width: 12ex; text-align:center;"><?php echo translate('Delete?'); ?></th>

						<?php
					}
					?>
					</tr>
				<?php
				foreach ($keyword_datas as $id => $info) {
					?>
					<tr>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][keyword]', array_get_index($info,'keyword',''), '', '', FALSE, 'style="width:95%"');
							} else {
								echo array_get_index($info, 'name', '');
							}
							?>
						</td>
						<td>
							<?php
							if ($wa) {
								text_box($prefix.'_current['.$id.'][pattern]', array_get_index($info,'pattern',''), '', '', FALSE, 'style="width:95%"');
							} else {
								echo array_get_index($info, 'pattern', '');
							}
							?>
						</td>
					<?php
					if ($wa) {
						?>
						<td style="text-align:center;">
							<?php
							check_box($prefix.'_current['.$id.'][remove]', '1');
							?>
						</td>
						<?php
					}
					?>
					</tr>
					<?php
				}//end foreach
				?>
				</table>
				<?php
			} else {
				echo ('<em>'.translate('There are no rules set.').'</em>');

			}
		$o->closeField();
		if ($wa) {
			$o->openField(translate('New Rule'));

				?>
				<table class="sq-backend-table">
					<tr>
						<th><?php echo translate('Keyword'); ?></th>

						<td><?php text_box($prefix.'_new[keyword]', '', '', '', FALSE, 'style="width:95%"'); ?><div class="sq-backend-smallprint"><?php echo translate('An asset based keyword or global keyword to use as the value for matching with the pattern value below. Keyword modifiers are also supported.'); ?></div></td>

					</tr>
					<tr>
						<th><?php echo translate('Pattern'); ?></th>

						<td><?php text_box($prefix.'_new[pattern]', '', '', '', FALSE, 'style="width:95%"'); ?><div class="sq-backend-smallprint"><?php echo translate('A value to use for checking a match with the keyword value above. If there is a match, the rule will be evaluated as true. Regular Expression is supported. Keywords are not supported.'); ?></div></td>

					</tr>
				</table>
				<?php
			$o->closeField();
		}

		$o->openField(translate('Rules Logic'));

		$query_types = Array('all' => translate('Match On All Rules'), 'any' => translate('Match On Any Rule'));


		$current_logic = $asset->attr('rules_logic');
		if ($wa) {
			combo_box($prefix.'_logic', $query_types, FALSE, $current_logic);
 		}
 		else {
 			echo $query_types[$current_logic];
 		}
 		$o->closeField();


	}//end paintStreamSelectionRules()


	/**
	* Processes the interface for config rules of automatic stream selection
	*
	* @param Workflow_Schema	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processStreamSelectionRules(Workflow_Stream $asset, Backend_Outputter $o, $prefix)
	{
		$current_rules = is_array($asset->attr('stream_selection_rules')) ? $asset->attr('stream_selection_rules') : unserialize($asset->attr('stream_selection_rules'));
		$prefix .= '_automatic_stream';
		$res = FALSE;

		if (!empty($_POST[$prefix.'_current'])) {
			foreach ($_POST[$prefix.'_current'] as $i => $details) {
				if (!empty($details['remove'])) {
					unset($current_rules[$i]);
				} else {
					$current_rules[$i]['pattern'] = $details['pattern'];
					if (!empty($details['keyword'])) {
						$current_rules[$i]['keyword'] = $details['keyword'];
					}
				}
			}
			$res = TRUE;
		}

		if (!empty($_POST[$prefix.'_new']['pattern']) && !empty($_POST[$prefix.'_new']['keyword'])) {
			$option = Array(
						'pattern'		=> $_POST[$prefix.'_new']['pattern'],
						'keyword'	=> $_POST[$prefix.'_new']['keyword'],
					  );
			$current_rules[] = $option;
			$res = TRUE;
		}

		if (!empty($_POST[$prefix.'_logic']) && !empty($_POST[$prefix.'_logic'])) {
			$grouping_logic = $_POST[$prefix.'_logic'];
			$res = TRUE;
		}

		if ($res) {
			$asset->setAttrValue('stream_selection_rules', $current_rules);
			$asset->setAttrValue('rules_logic', $grouping_logic);
			$asset->saveAttributes();
			return TRUE;
		}
		return FALSE;

	}//end processStreamSelectionRules()



}//end class

?>
