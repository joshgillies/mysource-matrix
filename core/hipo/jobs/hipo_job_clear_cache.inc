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
* $Id: hipo_job_clear_cache.inc,v 1.15 2006/04/18 03:27:51 rong Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clear cache
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Clear_Cache extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Clear_Cache($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		if (isset($this->_running_vars['linkid'])) {
			return parent::getCodeName().$this->_running_vars['linkid'];
		} else {
			return parent::getCodeName().$this->_running_vars['assetid'];
		}

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_clear_cache');

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
					'name'			=> translate('hipo_clearing_cache'),
					'function_call'	=> Array(
										'process_function'	=> 'processClearCache',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Prepares the running vars and gets a list of the cache directories in the system
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assetid'])) {
			trigger_localised_error('HIPO0033', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_localised_error('HIPO0032', E_USER_WARNING, $this->_running_vars['assetid']);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

		switch ($this->_running_vars['level']) {
			case 'single':
				$this->_running_vars['todo_assetids'] = Array($this->_running_vars['assetid']);
			break;
			case 'dependants':
				$this->_running_vars['todo_assetids'] = array_keys($GLOBALS['SQ_SYSTEM']->am->getDependantChildren($this->_running_vars['assetid'], $this->_running_vars['type_codes']));
				$this->_running_vars['todo_assetids'][] = $this->_running_vars['assetid'];
			break;
			case 'children':
				$this->_running_vars['todo_assetids'] = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->_running_vars['assetid'], $this->_running_vars['type_codes']));
				$this->_running_vars['todo_assetids'][] = $this->_running_vars['assetid'];
			break;
		}

		// get all the cache file path to be cleared
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$this->_running_vars['todo_filepaths'] = $cm->getFilePaths($this->_running_vars['todo_assetids']);
		$this->_running_vars['done_filepaths'] = Array();
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cm);

		// set the number of filepath to process per step, max is 100
		$count = count($this->_running_vars['todo_filepaths']) / 5;
		$this->_running_vars['count'] = min(100, $count);
		return parent::prepare();

	}//end prepare()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_filepaths'])) {
			if (!$this->processClearCache($this->_steps[0], get_class($this))) {
				return false;
			}
		}
		return true;

	}//end freestyle()


	/**
	* Processes the clearing of the actual cache files
	*
	* foreach of the todo assetids, the groups containing the cache files are
	* traversed and the cache files belonging to that assetid are deleted
	*
	* @param &Array 	$step_		data the data for the steps
	* @param string 	$prefix 	the unique prefix
	*/
	function processClearCache(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_filepaths'])) {

			// clear all related db entries
			if (!empty($this->_running_vars['todo_assetids'])) {
				$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$cm->clearCache($this->_running_vars['todo_assetids']);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cm);
				$this->_running_vars['todo_assetids'] = Array();
			}

			$index = 0;
			// process some number of filepaths in this step
			while ($index < $this->_running_vars['count'] && !empty($this->_running_vars['todo_filepaths'])) {
				$path = array_shift($this->_running_vars['todo_filepaths']);
				$path_name = SQ_CACHE_PATH.'/'.$path['path'];
				unlink($path_name);
				$this->_running_vars['done_filepaths'][] = $path;
				$index++;
			}

		}//end if

		if (empty($this->_running_vars['todo_filepaths'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_filepaths']) + count($this->_running_vars['done_filepaths']);
			$step_data['percent_done'] =(count($this->_running_vars['done_filepaths']) / $total) * 100;
			$step_data['complete'] = false;
		}
		return true;

	}//end processClearCache()


}//end class

?>
