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
* $Id: hipo_job_edit_status.inc,v 1.15 2004/01/22 00:35:35 diarandine Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to change the status of an asset
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Status extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Edit Status';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Editing Status',
						'function_call'    => Array(
												'process_function' => 'processStatus',
												),
						'running_mode'     => 'server',
						'auto_step'        => true,
						'percent_done'     => 0,
						'complete'         => false,
						'message'          => '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Status($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	function paintConfig(&$o, &$class)
	{
			$write_access = $this->writeAccess();
			
			// metadata regeneration threshhold HIPO config entry
			$o->openField('Edit Status Threshold');

			if($write_access) {
				text_box($class.'[SQ_HIPO_EDIT_STATUS_THRESHOLD]', SQ_HIPO_EDIT_STATUS_THRESHOLD, 5);
			} else {
				echo SQ_HIPO_EDIT_STATUS_THRESHOLD;
			}

			$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	function getConfigVars()
	{
		return Array(
			'SQ_HIPO_EDIT_STATUS_THRESHOLD' => Array('editable' => 1, 'default' => 1),
		);

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Status HIPO appends the ID of the asset that the status is being edited for
	* (the top most parent) so that any other status jobs on this asset do not start while it
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
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	* 
	* @return float
	* @access public
	*/
	function getThreshold()
	{
		if (SQ_HIPO_EDIT_STATUS_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) return 0;
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_EDIT_STATUS_THRESHOLD) * 100);
	
	}//end getThreshold()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start editing a status, we need to have an asset id to edit
		if (!isset($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Edit Status HIPO, no start asset ID specified', E_USER_WARNING);
			return '';
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Edit Status HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return '';
		}

		// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
		// we will be edting the status of
		$child_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);
		$child_assets[$asset->id] = $asset->type();
		$this->_running_vars['todo_assetids'] = $child_assets;
		$this->_running_vars['done_assetids'] = Array();

		unset($asset);

		return parent::prepare();
	
	}//end prepare()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Edit Status HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		if (!$this->is_prepared) {
			$this->prepare();
		}
		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	* 
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processStatus($this->_steps[0], get_class($this))) return false;
		}
		return true;

	}//end freestyle()


	/**
	* Process the current step of a HIPO in a server environment
	*
	* This function manages the message queue during the status changes
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processServer(&$step_data, $prefix)
	{
		// we are probably going to be sending some internal messages during this
		// process, so we'll open a new queue here
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();
		
		if (!parent::processServer($step_data, $prefix)) {
			$ms->abortQueue();
			return false;
		}

		// close the internal message queue and send the messages that
		// were generated during this status change
		$ms->closeQueue();

		return true;

	}//end processServer()


	/**
	* Edit the status of a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processStatus(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift(array_keys($this->_running_vars['todo_assetids']));
			$asset_type = $this->_running_vars['todo_assetids'][$assetid];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				$new_status = $this->_running_vars['new_status'];
				$asset->processStatusChange($new_status);
				$step_data['message'] = 'Editing status of "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not Edit Status for asset #'.$assetid.', asset does not exist');
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

	}//end processStatus()

}//end class

?>