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
* $Id: hipo_job_clear_cache.inc,v 1.2 2004/09/16 11:01:00 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to clear cache
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Clear_Cache extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Clear Cache';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
				Array(
						'name'             => 'Clearing Cache',
						'function_call'    => Array(
												'process_function' => 'processClearCache',
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
		if (isset($this->_running_vars['linkid'])) return parent::getCodeName().$this->_running_vars['linkid'];
		else return parent::getCodeName().$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Prepares the running vars and gets a list of the cache directories in the system
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Clear Cache HIPO, no start asset ID specified', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Clear Cache HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
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

		$this->_running_vars['done_assetids'] = Array();

		// find the cache dirs that are currently in the cache repository
		$this->_running_vars['cache_dirs'] = Array();
		$dh = opendir(SQ_CACHE_PATH);
		while (false !== ($file = readdir($dh))) {
			if ($file == '.' || $file == '..') continue;
			if (!is_dir(SQ_CACHE_PATH.'/'.$file)) continue;

			// cache directories should only be 4 digits long
			if (!preg_match('|\d{4}|', $file)) continue;

			// just add the relative path to the cache dir so
			// we can compare the name with the asset hash
			$this->_running_vars['cache_dirs'][] = $file;
		}
		closedir($dh);

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
		while (!empty($this->_running_vars['todo_assetids']) || !$this->_running_vars['done_root']) {
			if (!$this->processClearCache($this->_steps[0], get_class($this))) return false;
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
		if (!empty($this->_running_vars['todo_assetids'])) {

			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

			// get the group that is used as the first 2 digits of the
			// cache directory names, and get the asset id code whics is used as
			// the first string on each of the cache files

			$assetid_code = md5($assetid);
			$group = $cm->getAssetHash($assetid_code);

			foreach ($this->_running_vars['cache_dirs'] as $dir) {
				if (substr($dir, 0, strlen($group)) == $group) {

					// this directory contains cache entries for this particular asset
					// now files the files in this cache dir that belong to this asset
					$dh = opendir(SQ_CACHE_PATH.'/'.$dir);

					while (false !== ($file = readdir($dh))) {
						$abs_path = SQ_CACHE_PATH.'/'.$dir.'/'.$file;
						if (is_dir($abs_path)) continue;
						// if the file starts with the assetid_code for the asset
						// then back up in the unlinking with the resurection
						if (substr($file, 0, strlen($assetid_code)) == $assetid_code) unlink($abs_path);
					}
					closedir($dh);
				}
			}//end foreach

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;

		}//end if

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete'] = false;
		}
		return true;

	}//end processClearCache()


}//end class

?>
