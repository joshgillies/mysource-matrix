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
* $Id: hipo_job_generate_report.inc,v 1.1 2004/07/06 03:31:54 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to generate reports
*
* <pre>
* Initial Running Vars :
*    int    assetid  -   the assetid of the report being generated
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Generate_Report extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Generate Report';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Generating Report',
						'function_call'    => Array(
												'process_function' => 'processReport',
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
	function HIPO_Job_Generate_Report($code_name='')
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
		return parent::getCodeName().'-'.$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Prepares the running vars
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Generate Report HIPO, no report asset ID specified', E_USER_WARNING);
			return false;
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		$this->_running_vars['type_code'] = $asset->type();

		return parent::prepare();
	
	}//end prepare()


	/**
	* Generate the report
	*
	* We pass the work off to the report asset to do for us
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processReport(&$step_data, $prefix)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid'], $this->_running_vars['type_code']);
		$edit_fns = $asset->getEditFns();
		
		return $edit_fns->generateReport($this, $step_data, $prefix);

	}//end processReport()


}//end class

?>