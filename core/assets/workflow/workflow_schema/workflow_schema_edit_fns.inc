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
* $Id: workflow_schema_edit_fns.inc,v 1.24 2006/11/20 05:20:52 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Workflow_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Schema_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Workflow_Schema_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);


	}//end constructor


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewStep(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		echo translate('create_new_step_called').' ';
		text_box($prefix.'_new_step', '', 20);
		return TRUE;

	}//end paintNewStep()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewStep(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		if (isset($_POST[$prefix.'_new_step']) && trim($_POST[$prefix.'_new_step'])) {
			$new_step_name = trim($_POST[$prefix.'_new_step']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step');
			$new_step =& new Workflow_Step();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_step->setAttrValue('name', $new_step_name);
			if (!$new_step->create($schema_link)) return FALSE;
		}

		return TRUE;

	}//end processNewStep()


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSteps(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('links');
		$ok = $write_access;

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		$i = 1;
		foreach ($current_links as $link_data) {
			$step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;

			$o->openSection('Step '.$i.' - '.$step->name);
				if ($write_access) {
					$o->openField('Delete ?');
						check_box($prefix.'_delete_steps['.$link_data['linkid'].']');
						label(translate('delete_step_and_conditions_question'), $prefix.'_delete_steps['.$link_data['linkid'].']');
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
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSteps(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;
		$updated = FALSE;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
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
	* @param object	$asset	the asset we are generating the array for
	*
	* @return array
	* @access public
	*/
	function generateWorkflowArray($asset)
	{
		$workflow_array['steps']        = Array();
		$workflow_array['current_step'] = Array('1');
		$workflow_array['complete']     = FALSE;
		$workflow_array['schema_name']  = $asset->name;
		$workflow_array['started']      = time();

		$i = 1;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', TRUE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;
			$edit_fns = $step->getEditFns();
			$workflow_array['steps'][$i] = Array();
			if (!$edit_fns->generateWorkflowArray($step, $workflow_array['steps'][$i])) {
				return FALSE;
			}
			if ($i == 1) {
				$workflow_array['steps'][$i]['started'] = time();
			}
			$i++;
		}
		return $workflow_array;

	}//end generateWorkflowArray()


}//end class

?>