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
* $Id: hipo_job_reindex.inc,v 1.11 2004/05/13 04:13:01 lwright Exp $
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
* @subpackage HIPO
*/
class HIPO_Job_ReIndex extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Re-index Assets';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Reindexing Assets',
						'function_call'    => Array(
												'process_function' => 'processReIndex',
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
	function HIPO_Job_ReIndex($code_name='')
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
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();
		
		// metadata regeneration threshhold HIPO config entry
		$o->openField('Reindexing Threshold');

		if($write_access) {
			text_box($class.'[SQ_HIPO_REINDEX_THRESHOLD]', SQ_HIPO_REINDEX_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_REINDEX_THRESHOLD;
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
			'SQ_HIPO_REINDEX_THRESHOLD' => Array('editable' => 1, 'default' => 1),
		);

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other 
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['root_assetid'];

	}//end getCodeName()


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
		if (SQ_HIPO_REINDEX_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) return 0;
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_REINDEX_THRESHOLD) * 100);
	
	}//end getThreshold()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	* 
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processReIndex($this->_steps[0], get_class($this))) return false;
		}
		return true;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start regenerating metadata, we need to have a root assetid - default to root folder if none is supplied
		if (is_null($this->_running_vars['root_assetid']) || $this->_running_vars['root_assetid'] <= 0) {
			$this->_running_vars['root_assetid'] = 1;	// the root folder
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['root_assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Search Reindexing HIPO, invalid root asset ID specified ('.$this->_running_vars['root_assetid'].')', E_USER_WARNING);
			return false;
		}

		$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id);
		$child_assets[$asset->id] = $asset->type();
		$this->_running_vars['todo_assetids'] = $child_assets;
		$this->_running_vars['done_assetids'] = Array();

		return parent::prepare();
	
	}//end prepare()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Reindex HIPO also checks that there is an assetid to start this action on
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
	* Re-index asset keydata for a single asset in the todo list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processReIndex(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			$sm =& $GLOBALS['SQ_SYSTEM']->getSystemAsset('search_manager');
			if (!$sm) return false;

			// get next asset to reindex
			$assetid = array_shift(array_keys($this->_running_vars['todo_assetids']));
			$asset_type = $this->_running_vars['todo_assetids'][$assetid];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			$step_data['message'] = 'Reindexing '. $asset->name;
			$sm->changed($assetid);

			// add this assetid to the done array 
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

	}//end processReIndex()

}//end class

?>