<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: hipo_job_acquire_lock.inc,v 1.14 2003/10/21 00:14:18 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to acquire locks
*
* <pre>
* Initial Running Vars :
*    int    assetid             -   the assetid with whose children we are going to attempt to lock
*    string lock_type           -   the type of lock to get see Asset_Manager::acquireLock()
*    bool   [dependants_only]   -   whether dependants only or all children, defaults to 
*    bool   [forceably_acquire] -   whether to attempt to forceably acquire the lock or not, defaults to false
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage lib
*/
class HIPO_Job_Acquire_Lock extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Acquire Lock';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Acquiring Lock',
						'function_call'    => Array(
												'process_function' => 'processLocking',
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
	function HIPO_Job_Acquire_Lock($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Acquire Lock HIPO appends the ID of the asset that the lock is being acquired on
	* (the top most parent) so that any other locking jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		return parent::_getCodeName().'-'.$this->_running_vars['assetid'].'-'.implode(',',$this->_running_vars['lock_types']);

	}//end _getCodeName()


	/**
	* Resets the vars that the HIPO steps use
	*
	* @param Array	$vars
	*
	* @return void
	* @access public
	*/
	function setRunningVars($vars)
	{
		if (isset($vars['lock_type']) && isset($vars['lock_type'])) {
			// OK, the main reason that we are getting the lock types here is so that we don't end up 
			// with lock types in a child being locked that the parent doesn't have, because if they do then
			// we end up with a source lockid that is not actually creatable
			// EG. 'mail' lock is available in 'Inbox', but not 'User' and when you locked the user you would end
			//     up with a lock 'asset.5.mail' and a source lock 'asset.4.mail' which is not available
			// -- If this doesn't make sense ask me -- BCR
			$vars['lock_types'] = $GLOBALS['SQ_SYSTEM']->am->getLockTypes($vars['assetid'], $vars['lock_type']);
		}

		return parent::setRunningVars($vars);

	}//end setRunningVars()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Acquire Lock HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		// to start acquiring locks, we need to have an asset id to acquire a lock on
		if (empty($this->_running_vars['assetid']) || empty($this->_running_vars['lock_type'])) {
			trigger_error('Cannot initialise Acquire Locks HIPO, no start asset ID and/or lock type specified', E_USER_WARNING);
			return '';
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Acquire Locks HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return '';
		}

		// we were passed an invalid lock type, bugger off
		if (empty($this->_running_vars['lock_types'])) {
			trigger_error('Cannot initialise Acquire Locks HIPO, lock type "'.$this->_running_vars['lock_type'].'" invalid', E_USER_WARNING);
			return '';
		}

		// we don't need this anymore, we use the array 'lock_types'
		unset($this->_running_vars['lock_type']);

		// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
		// we will be acquiring locks on

		// if there is no dependants setting, or it is true, get only dependants

		if (!isset($this->_running_vars['dependants_only']) || !empty($this->_running_vars['dependants_only'])) {
			$this->_running_vars['dependants_only'] = false;
			$this->_running_vars['todo_assetids'] = array_merge(Array($asset->id), $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id));
		// else get locks for all children
		} else {
			$this->_running_vars['dependants_only'] = true;
			$this->_running_vars['todo_assetids'] = array_merge(Array($asset->id), $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id));

		}// endif

		if (!isset($this->_running_vars['forceably_acquire'])) {
			$this->_running_vars['forceably_acquire'] = false;
		} else {
			$this->_running_vars['forceably_acquire'] = (bool) $this->_running_vars['forceably_acquire'];
		}

		$this->_running_vars['done_assetids'] = Array();

		unset($asset);

		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Acquire a lock on a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processLocking(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				foreach($this->_running_vars['lock_types'] as $lock_type) {
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, $lock_type, $this->_running_vars['assetid'], $this->_running_vars['forceably_acquire']);
				}
				$step_data['message'] = 'Acquiring Lock on "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not Acquire Lock for asset #'.$assetid.', asset does not exist');
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

	}//end processLocking()

}//end class

?>