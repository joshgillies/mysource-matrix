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
* $Id: workflow_step_edit_fns.inc,v 1.41 2013/02/26 01:39:37 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Workflow_Step_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.41 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);

	}//end constructor


	/**
	* Paint the interface for creating and deleting step conditions
	*
	* @param Workflow_Step		$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	*
	* @return boolean
	* @access public
	*/
	function paintStep(Workflow_Step $asset, Backend_Outputter $o)
	{
		$write_access = ($asset->writeAccess('attributes') && $asset->writeAccess('links'));
		$prefix = $asset->getPrefix();
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$o->openField(translate('step_logic'));
			$step_logic = $asset->attr('logic');
			$step_logic_num = $asset->attr('logic_num');
			if (!$step_logic_num) $step_logic_num = '';

			if (!$write_access) {
				echo '<i>'.$wfm->_valid_step_logic[$step_logic];
				if ($step_logic_num) {
					echo ' '.$step_logic_num.' condition'.(($step_logic_num > 1) ? 's' : '').' must be met</i>';
				}
			} else {
				combo_box($prefix.'_logic', $wfm->_valid_step_logic, FALSE, $step_logic);
				echo '&nbsp;';
				text_box($prefix.'_logic_num', $step_logic_num, 4);
				echo ' '.translate('conditions_must_be_met_in_this_step');
			}
		$o->closeField();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE, TRUE);
		if (!empty($current_links)) {
			$o->openField('Conditions');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">&nbsp;</td>
						<td class="sq-backend-table-header"><?php echo translate('notify_when_live'); ?></td>
						<td class="sq-backend-table-header"><?php echo translate('require_comment'); ?></td>
						<td class="sq-backend-table-header" width="270"><?php echo translate('group_logic'); ?></td>
						<?php
						if ($write_access) {
							?><td align="center" width="70" class="sq-backend-table-header"><b><?php echo translate('delete_question'); ?></b></td><?php
						}
						?>
					</tr>
				<?php
				foreach ($current_links as $link_data) {
					$condition = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					if (is_null($condition)) continue;
					$edit_fns = $condition->getEditFns();
					echo '<tr>';
					if (!$edit_fns->paintStepCondition($condition, $o)) {
						return FALSE;
					}
					if ($write_access) {
						?><td align="center" class="sq-backend-table-cell"><?php
						check_box($prefix.'_delete_conditions['.$link_data['linkid'].']')
						?></td><?php
					}
					echo '</tr>';
				}
				?></table><?php
			$o->closeField();
		}//end if


		if ($write_access) {
			$o->openField(translate('new_condition_question'));
				asset_finder($prefix.'_new_condition', 0, Array('user' => 'D', 'user_group' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
			$o->closeField();
		}
		if ($write_access) {
			$o->openField(translate('new_dynamic_condition_question'));
				text_box($prefix.'_new_dynamic_condition', '', 80);
				$o->note(translate('new_dynamic_condition_question_note'));
			$o->closeField();
		}
		$o->openField(translate('require_comment_from_unlisted_users'));
			$require_comment_attr = $asset->getAttribute('require_comment_from_unlisted_users');
			$xml = simplexml_load_string('<boolean true_text="yes" false_text="no" />');
			$require_comment_attr->setEditParams($xml);
			$require_comment_attr->paint($prefix.'_require_comment_from_unlisted_users', !$write_access);
			$o->note(translate('require_comment_from_unlisted_users_explanation'));
		$o->closeField();

		$o->openField(translate('expiry_time'));
			$comparator_attr = $asset->getAttribute('expiry_time');
			$comparator_attr->paint($prefix.'_expiry_time', !$write_access);
			$o->note(translate('step_not_completed_in_time_explanation'));
		$o->closeField();
		$o->openField('Escalate?');
			$comparator_attr = $asset->getAttribute('escalate');
			$xml = simplexml_load_string('<boolean true_text="yes" false_text="no" />');
			$comparator_attr->setEditParams($xml);
			$comparator_attr->paint($prefix.'_escalate', !$write_access);
			$o->note(translate('escalate_note'));
			if ($asset->attr('escalate') == TRUE) {
				$o->openSection('Escalation Steps');
					// look for all the children of this step and list a bunch of links with delete buttons
					$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
					if (!empty($current_links)) {
						$assetids = Array();

						foreach ($current_links as $link) {
							$assetids[$link['minorid']] = 'details';
						}
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));
						$asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($assetids);

						$o->openField('Escalation Steps');
							?>
							<table class="sq-backend-table">
								<tr>
									<td class="sq-backend-table-header"><b><?php echo translate('step_name'); ?></b></td>
									<td width="150" class="sq-backend-table-header"><b><?php echo translate('edit_step'); ?></b></td>
									<?php
									if ($write_access) {
										?><td align="center" width="70" class="sq-backend-table-header"><b><?php echo translate('delete_question'); ?></b></td><?php
									}
									?>
								</tr>
							<?php
							foreach ($current_links as $link_data) {
								$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
								if (is_null($field)) continue;
								?>
								<tr>
									<td align="left" class="sq-backend-table-cell"><?php echo get_asset_tag_line($field->id);?></td>
									<td align="left" class="sq-backend-table-cell">[ <a href="<?php echo $asset_href[$field->id];?>"><?php echo translate('edit_step'); ?></a> ]</td>
								<?php
								if ($write_access) {
									?><td align="center" class="sq-backend-table-cell"><?php
									check_box($prefix.'_delete_steps['.$link_data['linkid'].']')
									?></td><?php
								}
								echo '</tr>';
							}
							?></table><?php
						$o->closeField();
					}//end if

					if ($write_access) {
						$o->openField('Create New Escalation Step');
							text_box($prefix.'_new_escalation_step', '', 20);
						$o->closeField();
					}
				$o->closeSection();
			}//end if escalate
		$o->closeField();

		return $write_access;

	}//end paintStep()


	/**
	* Process the interface for creating and deleting step conditions
	*
	* @param Workflow_Step		$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	*
	* @return boolean
	* @access public
	*/
	function processStep(Workflow_Step $asset, Backend_Outputter $o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes') || !$asset->writeAccess('links')) {
			return FALSE;
		}

		$prefix = $asset->getPrefix();
		$updated = FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set the step logic
		$step_logic = '';
		$step_logic_num = 0;
		if (isset($_POST[$prefix.'_logic'])) {
			$step_logic = trim($_POST[$prefix.'_logic']);
		}
		if (isset($_POST[$prefix.'_logic_num']) && $step_logic != 'all') {
			// Added condition to ensure a number is entered, and is greater than 0
			if (is_numeric(trim($_POST[$prefix.'_logic_num']))) {
				$step_logic_num = (int) trim($_POST[$prefix.'_logic_num']);
			} else {
				$step_logic_num = 1;
			}
			if ($step_logic_num <= 0) $step_logic_num = 1;
		}
		if (!$asset->setAttrValue('logic', $step_logic)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		if (!$asset->setAttrValue('logic_num', $step_logic_num)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// require comment from unlisted users
		$require_comment_attr = $asset->getAttribute('require_comment_from_unlisted_users');
		$require_comment_attr->process($prefix.'_require_comment_from_unlisted_users');
		if ($require_comment_attr->processed) {
			$asset->setAttrValue('require_comment_from_unlisted_users', $require_comment_attr->value);
			$updated = TRUE;
		}

		// update expiry date
		$expiry_time_attr = $asset->getAttribute('expiry_time');
		$expiry_time_attr->process($prefix.'_expiry_time');
		if ($expiry_time_attr->processed) {
			$asset->setAttrValue('expiry_time', $expiry_time_attr->value);
			$updated = TRUE;
		}

		// update escalation
		$escalate_attr = $asset->getAttribute('escalate');
		$escalate_attr->process($prefix.'_escalate');
		if ($escalate_attr->processed) {
			$asset->setAttrValue('escalate', $escalate_attr->value);
			$updated = TRUE;
		}

		if (!$asset->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// delete any steps that need deleting
		if (isset($_POST[$prefix.'_delete_steps'])) {
			$delete_steps = $_POST[$prefix.'_delete_steps'];
			foreach ($delete_steps as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = TRUE;
			}
		}

		if (isset($_POST[$prefix.'_new_escalation_step']) && trim($_POST[$prefix.'_new_escalation_step'])) {
			$new_step_name = trim($_POST[$prefix.'_new_escalation_step']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step');
			$new_step = new Workflow_Step();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_step->setAttrValue('name', $new_step_name);

			if ($new_step->create($schema_link)) $updated = TRUE;
		}

		// process all our current conditions
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE, TRUE);
		if (!empty($current_links)) {
			foreach ($current_links as $link_data) {
				$condition = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
				if (is_null($condition)) continue;
				$edit_fns = $condition->getEditFns();
				if ($edit_fns->processStepCondition($condition, $o)) {
					$updated = TRUE;
				}
			}
		}

		// delete any conditions that need deleting
		if (isset($_POST[$prefix.'_delete_conditions'])) {
			$delete_conds = $_POST[$prefix.'_delete_conditions'];
			foreach ($delete_conds as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = TRUE;
			}
		}

		// create a new condition?
		if (isset($_POST[$prefix.'_new_condition']['assetid']) && $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($_POST[$prefix.'_new_condition']['assetid'], Array('user', 'user_group'), FALSE)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step_condition');
			$new_cond = new Workflow_Step_Condition();
			$step_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1, 'is_exclusive' => 1);
			$new_cond->setAttrValue('userid', $_POST[$prefix.'_new_condition']['assetid']);
			if ($new_cond->create($step_link)) $updated = TRUE;
		}
		// create a new dynamic condition?
		else if (isset($_POST[$prefix.'_new_dynamic_condition']) && !empty($_POST[$prefix.'_new_dynamic_condition'])) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step_condition');
			$new_cond = new Workflow_Step_Condition();
			$step_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1, 'is_exclusive' => 1);
			$new_cond->setAttrValue('dynamic_userid', $_POST[$prefix.'_new_dynamic_condition']);
			if ($new_cond->create($step_link)) $updated = TRUE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $updated;

	}//end processStep()


	/**
	* Generate an array representing the workflow structure for the step at this point in time
	*
	* The generated array is a snap shot of the schema at a point in time that can get written
	* to the database and used for publishing etc.
	*
	* @param Asset	$asset				the asset we are generating the array for
	* @param array	&$workflow_array	the workflow array that we will modify
	* @param int	$top_parent	the dependant top parent of current asset
	*
	* @return array
	* @access public
	*/
	function generateWorkflowArray(Asset $asset, Array &$workflow_array, $top_parent='')
	{
		$workflow_array = Array();
		$workflow_array['conditions']							= Array();
		$workflow_array['escalation_steps']						= Array();
		$workflow_array['step_name']							= $asset->name;
		$workflow_array['started']								= 0;
		$workflow_array['expiry_time']							= $asset->attr('expiry_time');
		$workflow_array['escalate']						= $asset->attr('escalate');
		$workflow_array['completed']							= 0;
		$workflow_array['expired']								= 0;
		$workflow_array['require_comment_from_unlisted_users']	= $asset->attr('require_comment_from_unlisted_users');

		$workflow_array['message_invitation']					= $asset->attr('message_invitation');
		$workflow_array['message_review_invitation']			= $asset->attr('message_review_invitation');
		$workflow_array['message_approve']						= $asset->attr('message_approve');
		$workflow_array['message_reject']						= $asset->attr('message_reject');
		$workflow_array['message_review_cancel']				= $asset->attr('message_review_cancel');
		$workflow_array['message_stale']						= $asset->attr('message_stale');
		$workflow_array['message_escalated']					= $asset->attr('message_escalated');

		$workflow_array['subject_invitation']					= $asset->attr('subject_invitation');
		$workflow_array['subject_review_invitation']			= $asset->attr('subject_review_invitation');
		$workflow_array['subject_approve']						= $asset->attr('subject_approve');
		$workflow_array['subject_reject']						= $asset->attr('subject_reject');
		$workflow_array['subject_review_cancel']				= $asset->attr('subject_review_cancel');
		$workflow_array['subject_stale']						= $asset->attr('subject_stale');
		$workflow_array['subject_escalated']					= $asset->attr('subject_escalated');

		// work out the logic expression
		$logic = $asset->attr('logic');
		$logic_num = $asset->attr('logic_num');
		$logic = $logic.(($logic_num) ? " $logic_num" : '');
		$workflow_array['logic'] = $logic;

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE, TRUE);
		foreach ($current_links as $link_data) {
			$condition = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($condition)) continue;
			$edit_fns = $condition->getEditFns();
			if (!$edit_fns->generateWorkflowArray($condition, $workflow_array['conditions'], $top_parent)) {
				return FALSE;
			}

		}

		// work out the escalation steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		$i = 1;
		foreach ($current_links as $link_data) {
			$step = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;
			$workflow_array['escalation_steps'][$i] = Array();
			$edit_fns = $step->getEditFns();
			if (!$edit_fns->generateWorkflowArray($step, $workflow_array['escalation_steps'][$i], $top_parent)) {
				return FALSE;
			}
			$i++;
		}

		return TRUE;

	}//end generateWorkflowArray()


}//end class

?>
