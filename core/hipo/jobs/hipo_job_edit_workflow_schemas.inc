<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: hipo_job_edit_workflow_schemas.inc,v 1.5 2003/11/18 15:37:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit workflow schemas
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage lib
*/
class HIPO_Job_Edit_Workflow_Schemas extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Edit Workflow Schemas';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'			=> 'Acquiring Workflow Locks',
						'hipo_job'		=> Array(
												'init_details_function' => 'getLockAssetDetails',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array(
						'name'             => 'Updating Workflow Schemas',
						'function_call'    => Array(
												'process_function' => 'processSchemas',
												),
						'running_mode'     => 'server',
						'auto_step'        => true,
						'percent_done'     => 0,
						'complete'         => false,
						'message'          => '',
					),
					Array(
						'name'			=> 'Releasing Workflow Locks',
						'function_call'    => Array(
												'process_function' => 'releaseLocks',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Workflow_Schemas($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Workflow Schema HIPO appends the ID of the asset that the schemas are being set at
	* (the top most parent) so that any other schema jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		return parent::_getCodeName().$this->_running_vars['assetid'];

	}//end _getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Edit Workflow Schemas HIPO also checks that there are schemas to edit and that
	* there is an assetid to start this action on (the top most parent). It also saves a
	* list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		// to start editing schemas, we need to have an asset id to start editing schemas at
		if (!isset($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Edit Workflow Schemas HIPO, no start asset ID specified', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Edit Workflow Schemas HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return false;
		}

		$this->_running_vars['done_assetids'] = Array();
		if (empty($this->_running_vars['schema_changes'])) {
			// no schema changes, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
			// we will be affecting out list of schema changes on
			$child_assets = Array();
			$child_assets = array_merge(Array($asset->id), $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id));
			$this->_running_vars['todo_assetids'] = $child_assets;
		}

		unset($asset);

		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Edit schemas for a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processSchemas(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				foreach ($this->_running_vars['schema_changes'] as $set_data) {
					if ($set_data['schemaid']) {
						$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($set_data['schemaid']);
						if (is_null($schema)) $schema_name = 'Unknown Schema';
						else                  $schema_name = $schema->name;
					} else {
						continue;
					}

					$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();

					if ($set_data['previous_access'] === null) {
						// to edit this schema, there must not currently be a schema set
						// at all - either grant OR deny - for this asset
						$schemas = $wfm->getSchemas($asset->id, $set_data['access']);
						if (isset($schemas[$set_data['schemaid']])) {
							$new_access     = ($set_data['access']) ? 'apply' : 'deny';
							$current_access = ($schemas[$set_data['schemaid']]) ? 'applied' : 'denied';
							$this->_addError('Could not '.$new_access.' schema "'.$schema_name.'" on "'.$asset->name.'" because this asset already has this schema '.$current_access.'. The current schema setting for "'.$asset->name.'" has been retained.', true);
							continue;
						}
					} else {
						// we need to have a schema set and the access level be the same
						$schemas = $wfm->getSchemas($asset->id);
						if (!isset($schemas[$set_data['schemaid']])) {
							continue;
						} elseif ($schemas[$set_data['schemaid']] != $set_data['previous_access']) {
							$new_access     = ($set_data['previous_access']) ? 'revoke'  : 'apply';
							$current_access = ($set_data['previous_access']) ? 'applied' : 'denied';
							$this->_addError('Could not '.$new_access.' schema "'.$schema_name.'" on "'.$asset->name.'" because this asset already has this schema '.$current_access.'. The current schema setting for "'.$asset->name.'" has been retained.', true);
							continue;
						}
					}

					switch ($set_data['access']) {
						case -1 :
							// delete a schema
							$wfm->deleteSchema($asset->id, $set_data['schemaid']);
						break;

						case 0  :
						case 1  :
							// deny a schema
							// apply a permission
							$wfm->setSchema($asset->id, $set_data['schemaid'], $set_data['access']);
						break;
					}
				}

				$step_data['message'] = 'Editing Workflow Schemas for "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not edit schemas for asset #'.$assetid.', asset does not exist');
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processSchemas()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_lock';

		$options['auto_complete'] = true;

		$running_vars['assetid']         = $this->_running_vars['assetid'];
		$running_vars['lock_type']       = 'workflow';
		$running_vars['dependants_only'] = false;

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the schemas
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (!is_null($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'workflow');
			$step_data['message'] = 'Releasing Locks';
		}
		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end releaseLocks()


}//end class

?>