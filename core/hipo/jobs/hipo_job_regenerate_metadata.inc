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
* $Id: hipo_job_regenerate_metadata.inc,v 1.2 2003/11/18 15:37:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to regenerate pre-processed metadata files
* for all assets that use the supplied schema
* 
* Initial Running Vars : 
*    schemaid	-	the ID of the schema that has been updated
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage lib
*/
class HIPO_Job_Regenerate_Metadata extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Regenerate Metadata';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Generating Metadata Files',
						'function_call'    => Array(
												'process_function' => 'processMetadata',
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
	function HIPO_Job_Regenerate_Metadata($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other 
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		return parent::_getCodeName().'-'.$this->_running_vars['schemaid'];

	}//end _getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Regenerate Metdata HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		// to start regenerating metadata, we need to have a schema that has been updated
		if (empty($this->_running_vars['schemaid'])) {
			trigger_error('Cannot initialise Regenerate Metdata HIPO, no schemaID specified', E_USER_WARNING);
			return false;
		}

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$this->_running_vars['todo_assetids'] = $mm->getSchemaAssets($this->_running_vars['schemaid'], true);
		$this->_running_vars['done_assetids'] = Array();

		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Regenerate Metatdata for a single asset in the todo list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processMetadata(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				if (!$mm->generateContentFile($asset->id)) return false;
				$step_data['message'] = 'Generating metadata file for "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not regenerate metatdata file for asset #'.$assetid.', asset does not exist');
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

	}//end processMetadata()

}//end class

?>