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
* $Id: workflow_step_condition_edit_fns.inc,v 1.10.2.1 2004/03/26 16:41:50 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Workflow_Step_Condition_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Condition_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Workflow_Step_Condition_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);

	}//end constructor


	/**
	* Paint the interface for creating and deleting step conditions
	*
	* @param object Workflow_Step_Condition	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter		&$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintInlineOptionInterface(&$asset, &$o)
	{
		$write_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$cond_logic = $asset->attr('logic');
		$cond_logic_num = $asset->attr('logic_num');
		if (!$cond_logic_num) $cond_logic_num = '';

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->attr('userid'));
		if (is_null($user)) return false;

		?>
		<td class="sq-backend-table-cell"><?php echo '&nbsp;'.$user->name.' ['.$user->type()." : $user->id]";?></td>
		<td class="sq-backend-table-cell">
			<?php
			if (!$write_access) {
				if (is_a($user, 'user')) {
					echo '&nbsp;';
				} else {
					if (isset($wfm->_valid_cond_logic[$cond_logic])) {
						echo '<i>'.$wfm->_valid_cond_logic[$cond_logic];
						if ($cond_logic_num) echo " $cond_logic_num group member".(($cond_logic_num > 1) ? 's' : '')." must approve</i>";
					}
				}
			} else {
				if (is_a($user, 'user')) {
					hidden_field($prefix.'_logic', '>=');
					hidden_field($prefix.'_logic_num', '1');
				} else {
					combo_box($prefix.'_logic', $wfm->_valid_cond_logic, false, $cond_logic);
					echo '&nbsp;';
					text_box($prefix.'_logic_num', $cond_logic_num, 4);
					echo ' member(s) must approve';
				}
			}
		?></td><?php

		return true;

	}//end paintInlineOptionInterface()


	/**
	* Process the interface for editing this step condition
	*
	* @param object Workflow_Step_Condition	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter		&$o		the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processInlineOptionInterface(&$asset, &$o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$prefix = $asset->getPrefix();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// set the step condition logic
		$cond_logic = '';
		$cond_logic_num = '';
		if (isset($_POST[$prefix.'_logic'])) {
			$cond_logic = trim($_POST[$prefix.'_logic']);
		}
		if (isset($_POST[$prefix.'_logic_num']) && $cond_logic != 'all') {
			$cond_logic_num = trim($_POST[$prefix.'_logic_num']);
		}
		if (!$asset->setAttrValue('logic', $cond_logic)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$asset->setAttrValue('logic_num', $cond_logic_num)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$asset->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end processInlineOptionInterface()


	function generateWorkflowArray($asset, &$workflow_array)
	{
		// work out the logic expression
		$logic = $asset->attr('logic');
		$logic_num = $asset->attr('logic_num');
		$logic = $logic.(($logic_num) ? " $logic_num" : '');

		$publisher = $asset->attr('userid');
		$workflow_array[$publisher] = Array('logic' => $logic, 'published_by' => Array());

		return true;
	}


}//end class

?>