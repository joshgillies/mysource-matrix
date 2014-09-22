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
* $Id: hipo_job_quick_hipo.inc,v 1.5 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to acquire locks
*
* <pre>
* Initial Running Vars :
*    array(int)           assetids          - the assetids with whose children we are going to attempt to lock
*    array(string)|string lock_type         - the type of lock(s) to get (see Asset_Manager::acquireLock())
*    bool                 dependants_only   - whether dependants only or all children, defaults to true
*    bool                 forceably_acquire - whether to attempt to forceably acquire the lock or not,  defaults to false
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Quick_HIPO extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Quick_HIPO($code_name='')
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
	* @access public
	*/
	function getCodeName()
	{
		$items = Array();
		foreach ($this->_running_vars['items'] as $key => $val) {
			$items[] = $key.'='.$val;
		}

		return parent::getCodeName().'-'.$this->_running_vars['callback_assetid'].'-'.md5(implode(',', $items));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Quick');


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
					'name'			=> translate('Processing Assets'),

					'function_call'	=> Array(
										'process_function'	=> 'processQuickHipo',
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
		while (!empty($this->_running_vars['todo_items'])) {
			if (!$this->processQuickHipo($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Returns the callback asset if the asset and callback function are valid
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getCallbackAsset()
	{
		$null = NULL;
		$callback_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['callback_assetid']);

		if (is_null($callback_asset)) {
			trigger_localised_error('HIPO0080', E_USER_ERROR, $this->_running_vars['callback_assetid']);
			return $NULL;
		}

		if (!method_exists($callback_asset, $this->_running_vars['callback_function'])) {
			trigger_localised_error('HIPO0081', E_USER_ERROR, $this->_running_vars['callback_function']);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($callback_asset);
			return $NULL;
		}

		return $callback_asset;

	}//end _getCallbackAsset()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start acquiring locks, we need to have an asset id to acquire a lock on
		if (empty($this->_running_vars['items'])) {
			trigger_localised_error('HIPO0079', E_USER_WARNING, translate('Quick'));

			return FALSE;
		}

		// check that the callback asset is valid, and the callback function exists
		$callback_asset = $this->_getCallbackAsset();
		if (is_null($callback_asset)) return FALSE;

		$this->_running_vars['total_count'] = count($this->_running_vars['items']);
		$this->_running_vars['todo_items']  = array_keys($this->_running_vars['items']);
		$this->_running_vars['done_items']  = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Acquire a lock on a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processQuickHipo(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_items'])) {

			// check that the callback asset is valid, and the callback function exists
			$callback_asset = $this->_getCallbackAsset();
			if (is_null($callback_asset)) return FALSE;

			$current_item_index = reset($this->_running_vars['todo_items']);
			$callback_function  = $this->_running_vars['callback_function'];

			if (!$callback_asset->$callback_function($current_item_index, $this->_running_vars['items'][$current_item_index], $this->_running_vars['settings'])) {
				if ($this->_running_vars['fail_on_error']) {
					return FALSE;
				}
			}

			// add this item to the done array so we dont do it again, then remove the item from the todo list
			$this->_running_vars['done_items'][] = $current_item_index;
			array_shift($this->_running_vars['todo_items']);
		}

		if (empty($this->_running_vars['todo_items'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_items']) / $this->_running_vars['total_count']) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processQuickHipo()


}//end class

?>
