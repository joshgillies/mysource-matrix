<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Workflow_Step_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Workflow_Step_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for creating and deleting step conditions
	*
	* @param object Workflow_Step		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintInlineOptionInterface(&$asset, &$o)
	{
		$write_access = $asset->writeAccess();
		$prefix = $asset->getPrefix();
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$o->openField('Step Logic');
			$step_logic = $asset->attr('logic');
			$step_logic_num = $asset->attr('logic_num');
			if (!$step_logic_num) $step_logic_num = '';

			if (!$write_access) {
				echo '<i>'.$wfm->_valid_step_logic[$step_logic];
				if ($step_logic_num) echo " $step_logic_num condition".(($step_logic_num > 1) ? 's' : '')." must be met</i>";
			} else {
				combo_box($prefix.'_logic', $wfm->_valid_step_logic, false, $step_logic);
				echo '&nbsp;';
				text_box($prefix.'_logic_num', $step_logic_num, 4);
				echo ' condition(s) must be met in this step';
			}
		$o->closeField();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', true, 'major', null, true, true);
		if (!empty($current_links)) {
			$o->openField('Conditions');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">&nbsp;</td>
						<td class="sq-backend-table-header" width="270"><b>Group Logic</b></td>
						<?php
						if ($write_access) {
							?><td align="center" width="70" class="sq-backend-table-header"><b>Delete ?</b></td><?php
						}
						?>
					</tr>
				<?php
				foreach ($current_links as $link_data) {
					$condition = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					if (is_null($condition)) continue;
					$edit_fns = $condition->getEditFns();
					echo '<tr>';
					if (!$edit_fns->paintInlineOptionInterface($condition, $o)) return false;
					if ($write_access) {
						?><td align="center" class="sq-backend-table-cell"><?php
						check_box($prefix.'_delete_conditions['.$link_data['linkid'].']')
						?></td><?php
					}
					echo '</tr>';
				}
				?></table><?php
			$o->closeField();
		}

		if ($write_access) {
			$o->openField('New Condition?');
				asset_finder($prefix.'_new_condition', 0, Array('user' => 'D', 'user_group' => 'D'));
			$o->closeField();
		}

		return true;

	}//end paintInlineOptionInterface()


	/**
	* Process the interface for creating and deleting step conditions
	*
	* @param object Workflow_Step		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processInlineOptionInterface(&$asset, &$o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;

		$prefix = $asset->getPrefix();
		$updated = false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set the step logic
		$step_logic = '';
		$step_logic_num = '';
		if (isset($_POST[$prefix.'_logic'])) {
			$step_logic = trim($_POST[$prefix.'_logic']);
		}
		if (isset($_POST[$prefix.'_logic_num']) && $step_logic != 'all') {
			$step_logic_num = trim($_POST[$prefix.'_logic_num']);
		}
		if (!$asset->setAttrValue('logic', $step_logic)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$asset->setAttrValue('logic_num', $step_logic_num)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$asset->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// process all our current conditions
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', true, 'major', null, true, true);
		if (!empty($current_links)) {
			foreach ($current_links as $link_data) {
				$condition = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
				if (is_null($condition)) continue;
				$edit_fns = $condition->getEditFns();
				if ($edit_fns->processInlineOptionInterface($condition, $o)) $updated = true;
			}
		}

		// delete any conditions that need deleting
		if (isset($_POST[$prefix.'_delete_conditions'])) {
			$delete_conds = $_POST[$prefix.'_delete_conditions'];
			foreach ($delete_conds as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = true;
			}
		}

		// create a new condition?
		if (isset($_POST[$prefix.'_new_condition']) && $_POST[$prefix.'_new_condition'] > 0) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step_condition');
			$new_cond =& new Workflow_Step_Condition();
			$step_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'dependant' => 1, 'exclusive' => 1);
			$new_cond->setAttrValue('userid', $_POST[$prefix.'_new_condition']);
			if ($new_cond->create($step_link)) $updated = true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $updated;

	}//end paintInlineOptionInterface()



	function generateWorkflowArray($asset, &$workflow_array)
	{
		$workflow_array = Array();
		$workflow_array['conditions'] = Array();
		$workflow_array['step_name'] = $asset->name;

		// work out the logic expression
		$logic = $asset->attr('logic');
		$logic_num = $asset->attr('logic_num');
		$logic = $logic.(($logic_num) ? " $logic_num" : '');
		$workflow_array['logic'] = $logic;

		$i = 1;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step_condition', true, 'major', null, true, true);
		foreach ($current_links as $link_data) {
			$condition = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($condition)) continue;
			$edit_fns = $condition->getEditFns();
			if (!$edit_fns->generateWorkflowArray($condition, $workflow_array['conditions'])) return false;
			$i++;
		}
		return true;
	}


}//end class

?>