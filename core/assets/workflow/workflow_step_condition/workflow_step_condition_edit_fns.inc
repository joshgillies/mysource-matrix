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
* $Id: workflow_step_condition_edit_fns.inc,v 1.30 2013/02/26 01:38:19 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Workflow_Step_Condition_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Condition_Edit_Fns extends Asset_Edit_Fns
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
	* @param Workflow_Step_Condition	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the Backend Outputter object
	*
	* @return boolean
	* @access public
	*/
	function paintStepCondition(Workflow_Step_Condition $asset, Backend_Outputter $o)
	{
		$write_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$cond_logic = $asset->attr('logic');
		$cond_logic_num = $asset->attr('logic_num');
		if (!$cond_logic_num) $cond_logic_num = '';

		$userid = $asset->attr('userid');

		?>
		<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($userid); ?></td>
		<td class="sq-backend-table-cell">
			<?php
			$notify = $asset->attr('notify');
			if (!$write_access) {
				echo ($notify) ? translate('yes') : translate('no');
			} else {
				check_box($prefix.'_notify', '1', $notify);
			}
			?>
		</td>
		<td class="sq-backend-table-cell">
			<?php
			$require_comment = $asset->attr('require_comment');
			if (!$write_access) {
				echo ($require_comment) ? translate('yes') : translate('no');
			} else {
				check_box($prefix.'_require_comment', '1', $require_comment);
			}
			?>
		</td>
		<?php
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), Array('user', 'user_group'), FALSE);
			if (empty($user_info)) {
				echo '<td>&nbsp;</td>';
				return TRUE;
			}
		?>
		<td class="sq-backend-table-cell">
			<?php
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
			if (!$write_access) {
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
				if (($user instanceof User)) {
					echo '&nbsp;';
				} else {
					if (isset($wfm->_valid_cond_logic[$cond_logic])) {
						echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
						if ($cond_logic_num) {
							echo ' '.$cond_logic_num.' group member'.(($cond_logic_num > 1) ? 's' : '').' must approve</i>';
						}
					}
				}
			} else {
				if (($user instanceof User)) {
					hidden_field($prefix.'_logic', '>=');
					hidden_field($prefix.'_logic_num', '1');
				} else {
					combo_box($prefix.'_logic', $wfm->_valid_cond_logic, FALSE, $cond_logic);
					echo '&nbsp;';

					ob_start();
						text_box($prefix.'_logic_num', $cond_logic_num, 4);
						$text_box = ob_get_contents();
					ob_end_clean();
					echo translate('members_must_approve', $text_box);
				}
			}
			?>
		</td>
		<?php

		return TRUE;

	}//end paintStepCondition()


	/**
	* Process the interface for editing this step condition
	*
	* @param Workflow_Step_Condition	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the Backend Outputter object
	*
	* @return boolean
	* @access public
	*/
	function processStepCondition(Workflow_Step_Condition $asset, Backend_Outputter $o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$prefix = $asset->getPrefix();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$cond_logic = array_get_index($_POST, $prefix.'_logic');
		if (!$asset->setAttrValue('logic', $cond_logic)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (isset($_POST[$prefix.'_logic_num']) && $cond_logic != 'all') {
			$cond_logic_num = (!empty($_POST[$prefix.'_logic_num'])) ? (int) $_POST[$prefix.'_logic_num'] : 0;
		} else {
			$cond_logic_num = 0;
		}
		if (!$asset->setAttrValue('logic_num', $cond_logic_num)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$notify = (bool) array_get_index($_POST, $prefix.'_notify', FALSE);
		if (!$asset->setAttrValue('notify', $notify)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$require_comment = (bool) array_get_index($_POST, $prefix.'_require_comment', FALSE);
		if (!$asset->setAttrValue('require_comment', $require_comment)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!$asset->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processStepCondition()


	/**
	* Generate an array representing the workflow structure for the step condition at this point in time
	*
	* The generated array is a snap shot of the schema at a point in time that can get written
	* to the database and used for publishing etc.
	*
	* @param Asset	$asset				the asset we are generating the array for
	* @param array	&$workflow_array	the workflow array that we will modify
	*
	* @return array
	* @access public
	*/
	function generateWorkflowArray(Asset $asset, &$workflow_array)
	{
		// work out the logic expression
		$logic = $asset->attr('logic');
		$logic_num = $asset->attr('logic_num');

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		if (in_array($logic, $wfm->_valid_logic_with_right_operand)) {
			$logic = $logic.(($logic_num) ? " $logic_num" : ' 0');
		} else {
			$logic = $logic.(($logic_num) ? " $logic_num" : '');
		}

		$publisher = $asset->attr('userid');
		$workflow_array[$publisher] = Array('logic' => $logic, 'published_by' => Array());

		// commenting
		$workflow_array[$publisher]['require_comment'] = $asset->attr('require_comment');

		return TRUE;

	}//end generateWorkflowArray()


}//end class

?>
