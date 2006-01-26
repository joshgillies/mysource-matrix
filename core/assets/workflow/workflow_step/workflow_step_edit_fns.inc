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
* $Id: workflow_step_edit_fns.inc,v 1.29 2006/01/26 22:34:08 lwright Exp $
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
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Workflow_Step_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);

	}//end constructor


	/**
	* Paint the interface for creating and deleting step conditions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	*
	* @return boolean
	* @access public
	*/
	function paintStep(&$asset, &$o)
	{
		$write_access = ($asset->writeAccess('attributes') && $asset->writeAccess('links'));
		$prefix = $asset->getPrefix();
		$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
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
					$condition =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
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

		$o->openField(translate('require_comment_from_unlisted_users'));
			$require_comment_attr =& $asset->getAttribute('require_comment_from_unlisted_users');
			$require_comment_attr->paint($prefix.'_require_comment_from_unlisted_users', !$write_access);
			$o->note(translate('require_comment_from_unlisted_users_explanation'));
		$o->closeField();

		if ($write_access) {
			$o->openField(translate('new_condition_question'));
				asset_finder($prefix.'_new_condition', 0, Array('user' => 'D', 'user_group' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
			$o->closeField();
		}
		$o->openField(translate('expiry_time'));
			$comparator_attr =& $asset->getAttribute('expiry_time');
			$comparator_attr->paint($prefix.'_expiry_time', !$write_access);
			$o->note(translate('step_not_completed_in_time_explanation'));
		$o->closeField();

		return $write_access;

	}//end paintStep()


	/**
	* Process the interface for creating and deleting step conditions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	*
	* @return boolean
	* @access public
	*/
	function processStep(&$asset, &$o)
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
		$require_comment_attr =& $asset->getAttribute('require_comment_from_unlisted_users');
		$require_comment_attr->process($prefix.'_require_comment_from_unlisted_users');
		if ($require_comment_attr->processed) {
			$asset->setAttrValue('require_comment_from_unlisted_users', $require_comment_attr->value);
			$updated = TRUE;
		}

		// update expiry date
		$expiry_time_attr =& $asset->getAttribute('expiry_time');
		$expiry_time_attr->process($prefix.'_expiry_time');
		if ($expiry_time_attr->processed) {
			$asset->setAttrValue('expiry_time', $expiry_time_attr->value);
			$updated = TRUE;
		}

		if (!$asset->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// process all our current conditions
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE, TRUE);
		if (!empty($current_links)) {
			foreach ($current_links as $link_data) {
				$condition =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
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
		if (isset($_POST[$prefix.'_new_condition']['assetid']) && $_POST[$prefix.'_new_condition']['assetid'] > 0) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step_condition');
			$new_cond =& new Workflow_Step_Condition();
			$step_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1, 'is_exclusive' => 1);
			$new_cond->setAttrValue('userid', $_POST[$prefix.'_new_condition']['assetid']);
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
	* @param object	$asset				the asset we are generating the array for
	* @param array	&$workflow_array	the workflow array that we will modify
	*
	* @return array
	* @access public
	*/
	function generateWorkflowArray($asset, &$workflow_array)
	{
		$workflow_array = Array();
		$workflow_array['conditions']                          = Array();
		$workflow_array['step_name']                           = $asset->name;
		$workflow_array['started']                             = 0;
		$workflow_array['expiry_time']                         = $asset->attr('expiry_time');
		$workflow_array['completed']                           = 0;
		$workflow_array['require_comment_from_unlisted_users'] = $asset->attr('require_comment_from_unlisted_users');

		// work out the logic expression
		$logic = $asset->attr('logic');
		$logic_num = $asset->attr('logic_num');
		$logic = $logic.(($logic_num) ? " $logic_num" : '');
		$workflow_array['logic'] = $logic;

		$i = 1;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', TRUE, 'major', NULL, TRUE, TRUE);
		foreach ($current_links as $link_data) {
			$condition =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($condition)) continue;
			$edit_fns = $condition->getEditFns();
			if (!$edit_fns->generateWorkflowArray($condition, $workflow_array['conditions'])) {
				return FALSE;
			}
			$i++;
		}

		return TRUE;

	}//end generateWorkflowArray()


}//end class

?>