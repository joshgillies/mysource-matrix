<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Workflow_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Workflow_Schema_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Workflow_Schema_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end construtor()


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param object Workflow_Schema		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewStep(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		echo 'Create a new step called ';
		text_box($prefix.'_new_step', '', 20);
		return true;

	}//end paintNewStep()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param object Workflow_Schema		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewStep(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;

		if (isset($_POST[$prefix.'_new_step']) && trim($_POST[$prefix.'_new_step'])) {
			$new_step_name = trim($_POST[$prefix.'_new_step']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_step');
			$new_step =& new Workflow_Step();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'dependant' => 1);
			$new_step->setAttrValue('name', $new_step_name);
			if (!$new_step->create($schema_link)) return false;
		}

		return true;

	}//end processNewStep()


	/**
	* Paint the interface for creating a new workflow step
	*
	* @param object Workflow_Schema		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSteps(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', true, 'major', null, true, false);
		$i = 1;
		foreach ($current_links as $link_data) {
			$step = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;

			$o->openSection('Step '.$i.' - '.$step->name);
				if ($write_access) {
					$o->openField('Delete ?');
						check_box($prefix.'_delete_steps['.$link_data['linkid'].']');
						echo '&nbsp; Delete this step and all its conditions?';
					$o->closeField();
				}
				$edit_fns = $step->getEditFns();
				if (!$edit_fns->paintInlineEditInterface($step, $o)) return false;
			$o->closeSection();

			$i++;
		}

		return true;

	}//end paintSteps()


	/**
	* Processes the interface for creating a new workflow step
	*
	* @param object Workflow_Schema		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSteps(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;
		$updated = false;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', true, 'major', null, true, false);
		foreach ($current_links as $link_data) {
			$step = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;
			$edit_fns = $step->getEditFns();
			if($edit_fns->processInlineEditInterface($step, $o)) $updated = true;
		}

		// delete any steps that need deleting
		if (isset($_POST[$prefix.'_delete_steps'])) {
			$delete_steps = $_POST[$prefix.'_delete_steps'];
			foreach ($delete_steps as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = true;
			}
		}

		return $updated;

	}//end processSteps()


	function generateWorkflowArray($asset)
	{
		$workflow_array['steps'] = Array();
		$workflow_array['current_step'] = 1;
		$workflow_array['complete'] = false;
		$workflow_array['schema_name'] = $asset->name;

		$i = 1;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'workflow_step', true, 'major', null, true, false);
		foreach ($current_links as $link_data) {
			$step = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($step)) continue;
			$edit_fns = $step->getEditFns();
			$workflow_array['steps'][$i] = Array();
			if (!$edit_fns->generateWorkflowArray($step, $workflow_array['steps'][$i])) return false;
			$i++;
		}
		return $workflow_array;
	}


}//end class

?>