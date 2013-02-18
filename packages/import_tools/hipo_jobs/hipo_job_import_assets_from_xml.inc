<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_import_assets_from_xml.inc,v 1.7.2.1 2013/02/18 00:24:41 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/import_export/import.inc';


/**
* HIPO for importing a large HTML file into Matrix assets
*
* Initial Running Vars :
*			'file_info' 			- Full path to the file to import
*			'create_under_assetid'	- Root asset ID to import to
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.7.2.1 $
* @package MySource_Matrix
* @subpackage import_tools/hipo_jobs
*
*/
class HIPO_Job_Import_Assets_From_Xml extends HIPO_Job
{


	public $import_action_outputs = Array();

	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Import_Assets_From_Xml($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique id for this job
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return 'HIPO_Job_Import_Assets_From_Xml-'.md5($this->_running_vars['create_under_assetid'].$this->_running_vars['file_info']);

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('tool_import_assets_from_xml_hipo_name');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('tool_import_assets_from_xml_hipo_name'),
					'function_call'	=> Array(
										'process_function'	=> 'processImportAsset',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			  );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$step_data['complete'] = FALSE;
		while(!$step_data['complete']) {
			if(!$this->processImportAsset($step_data, '')) {
				return FALSE;
			}
		}

		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$this->_running_vars['import_action_outputs'] = Array();
		$this->_running_vars['nest_content_to_fix'] = Array();
		$file_info = $this->_running_vars['file_info'];
		$import_actions = get_import_actions($file_info);

		// if we want to explictily target the root node under which the structure is imported , do it
		if(isset($this->_running_vars['create_under_assetid']) && $this->_running_vars['create_under_assetid'] != 0) {
			$import_actions['actions'][0]['action'][0]['parentid'][0] = $this->_running_vars['create_under_assetid'];
		}
		
		// fix the file import path, it should be relative to the export dir
		foreach ($import_actions['actions'][0]['action'] as $id => $action) {
			if(isset($action['file_path'][0])) {
				$action['file_path'][0] = dirname($file_info).'/'.$action['file_path'][0];
				$import_actions['actions'][0]['action'][$id] = $action;
			}
			// remember nest content to fix
			if($action['action_type'][0] === 'create_asset' && $action['type_code'][0] === 'Content_Type_Nest_Content') {
				$this->_running_vars['nest_content_to_fix'] [] = $action['action_id'][0];
			}
			
			// check if there is hard coded assetid reference which doesn't exist in the target system
			if(!$this->checkAssetExists($action, 'parentid') || !$this->checkAssetExists($action, 'assetid') || !$this->checkAssetExists($action, 'asset')) {
				$this->_addError('Action ID "'.$action['action_id'][0].'" contains non-exist assetid reference. Action skipped.', E_USER_WARNING);
				unset($import_actions['actions'][0]['action'][$id]);
			}
		}
		
		$this->_running_vars['import_actions'] = $import_actions;
		$this->_running_vars['total_count'] = count($import_actions['actions'][0]['action']);
		
		
		if (empty($file_info)) {
			trigger_localised_error('HIPO0095', E_USER_NOTICE);
			return FALSE;
		}

		$root_node_id 	= $this->_running_vars['create_under_assetid'];
		// if value of root node is 0 then we are good. It just means we are not selecting root node
		// but if we have a value, then make sure it is a proper asset
		if($root_node_id != 0) {
			$root_node 		= $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node_id);
			if (is_null($root_node)) {
				trigger_localised_error('HIPO0096', E_USER_NOTICE);
				return FALSE;
			}
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Import Bulkmail Users from a CSV file
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processImportAsset(&$step_data, $prefix)
	{
		$import_actions = $this->_running_vars['import_actions'];
		

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$action = array_shift($import_actions['actions'][0]['action']);
		// Loop through the Actions from the XML File
		if(!empty($action)) {

			// Execute the action
			if (!execute_import_action($action, $this->_running_vars['import_action_outputs'])) {
				trigger_error('Action ID "'.$action['action_id'][0].'" could not be executed', E_USER_WARNING);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$this->_running_vars['import_actions'] = $import_actions;
		
		// update progress
		if (count($import_actions['actions'][0]['action']) == 0) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			 
			// fix nest content type, regenerate the bodycopy
			foreach ($this->_running_vars['nest_content_to_fix'] as $actionid) {
			   if(isset($this->_running_vars['import_action_outputs'][$actionid])) {
				$nest_content_id = $this->_running_vars['import_action_outputs'][$actionid]['assetid'];
				$nest_content = $GLOBALS['SQ_SYSTEM']->am->getAsset($nest_content_id);
				$nest_content->_tmp['edit_fns'] = NULL;
				$nest_content->linksUpdated();
			   }
			}
			
			// done with everything ? delete the file
			if ($this->_running_vars['delete_after_import']) {
				// unlink the import dir
				$command = "rm -rf ".dirname($this->_running_vars['file_info']); 
				system($command);
			}
		}
		else {
			$step_data['percent_done'] = (($this->_running_vars['total_count'] - count($import_actions['actions'][0]['action'])) / $this->_running_vars['total_count']) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processImport()


	/**
	* Check if hard coded assetid reference is a valid assetid in target system
	*
	* @param array	$action	the action xml
	* @param string	$type	the type of asset id field asset | assetid | parentid
	*
	* @return void
	* @access public
	*/
	function checkAssetExists($action, $type='asset')
	{
		if(isset($action[$type][0]) && preg_match('/^[0-9]+$/', $action[$type][0])){
		    return ($GLOBALS['SQ_SYSTEM']->am->assetExists ($action[$type][0]));
		}	
		return TRUE;
	}

}//end class

?>
