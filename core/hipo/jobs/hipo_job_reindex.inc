<?php
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
class HIPO_Job_ReIndex extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Re-index Assets';

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
		return parent::_getCodeName().'-'.$this->_running_vars['root_assetid'];

	}//end _getCodeName()


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
		// to start regenerating metadata, we need to have a root assetid - default to root folder if none is supplied
		if (is_null($this->_running_vars['root_assetid']) || $this->_running_vars['root_assetid'] <= 0) {
			$this->_running_vars['root_assetid'] = 1;	// the root folder
		}

		$this->_running_vars['todo_assetids'] = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->_running_vars['root_assetid']);
		$this->_running_vars['done_assetids'] = Array();
		
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
			$im = &$GLOBALS['SQ_SYSTEM']->getIndexingManager();

			// get next asset to reindex
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset =& $am->getAsset($assetid);
			$step_data['message'] = 'Reindexing '. $asset->name;
			$im->changed($assetid);

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