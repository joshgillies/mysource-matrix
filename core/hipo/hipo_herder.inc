<?php

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) Herder (Manager ;P)
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage HIPO
*/
class HIPO_Herder extends Resolve_Object
{

	/**
	* @var array of jobs that have been loaded
	*/
	var $_jobs = Array();

	/**
	* Constructor
	*
	*/
	function HIPO_Herder()
	{
	}//end constructor


	/**
	* Returns tha stack of jobs that are running for passed user and where initiated because
	* of the hipo job represented by source_code_name
	*
	* @param string	$code_name	the codename of the job
	* @param string	$job_type	the type of job (supply if known for speedier access)
	*
	* @return object HIPO_Job|null	returns the requested hipo job OR null on error
	* @access public
	*/
	function &getJob($code_name, $job_type='')
	{
		if (empty($this->_jobs[$code_name])) {
			if (!$job_type) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$db = &$GLOBALS['SQ_SYSTEM']->db;
				$sql = 'SELECT job_type
						FROM '.SQ_TABLE_PREFIX.'hipo_job
						WHERE code_name = '.$db->quote($code_name);
				$job_type = $db->getOne($sql);
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				if (DB::isError($job_type)) {
					trigger_error($job_type->getMessage().'<br/>'.$job_type->getUserInfo(), E_USER_WARNING);
					$null = null; // needed for ref
					return $null;
				}
				// no job exists ? 
				if (!$job_type) {
					$null = null; // needed for ref
					return $null;
				}

			}// end if

			require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/'.$job_type.'.inc';
			$this->_jobs[$code_name] = new $job_type($code_name);

			// something buggered up, not our problem
			if (!$this->_jobs[$code_name]->code_name) {
				$null = null; // needed for ref
				return $null;
			}

		}// endif
		return $this->_jobs[$code_name];

	}//end getJobStack()


	/**
	* Called by the web script to inform the hipo manager that it can start processing
	*
	* @return boolean
	* @access public
	*/
	function processWeb()
	{
		if (empty($_GET['hipo_source_code_name'])) {
			trigger_error('HIPO source codename not specified', E_USER_WARNING);
			return false;
		}

		$source_job = &$this->getJob($_GET['hipo_source_code_name']);
		if (is_null($source_job)) {
			trigger_error('Unable to find a source job with the codename "'.$_GET['hipo_source_code_name'].'"', E_USER_WARNING);
			return false;
		}

		if (!$source_job->process()) {
			trigger_error('HIPO failed to process', E_USER_WARNING);
			return false;

		}
		return true;

	}//end processWeb()


	/**
	* Called by the web script to paint the current process bar
	*
	* @return void
	* @access public
	*/
	function paintWeb()
	{
		if (empty($_GET['hipo_source_code_name'])) {
			trigger_error('HIPO source codename not specified', E_USER_WARNING);
			return false;
		}

		$source_job = &$this->getJob($_GET['hipo_source_code_name']);
		if (is_null($source_job)) {
			trigger_error('Unable to find a source job with the codename "'.$_GET['hipo_source_code_name'].'"', E_USER_WARNING);
			return false;
		}

		$source_job->paint();

	}//end paintWeb()


	/**
	* Called by the daemon script to inform the hipo manager that it is about to start processing
	* This is needed so that we can check the the input from the web script and
	* return an ACK or an error to it before we start processing
	*
	* @param object XML_Tree	$input_root		root node of the data sent from the web script
	* @param object XML_Tree	$output_root	root node of the data to be sent back to the web script
	*
	* @return boolean	indicating whether processing should start or not
	* @access public
	*/
	function prepareProcessServer(&$input_root, &$output_root)
	{
		require_once 'XML/Tree.php';

		$this->_tmp['prepared_server'] = Array();

		if ($input_root->name != 'hipo_job') {
			$output_root->name = 'error';
			$output_root->contents = 'Input Root Node not named "hipo_job"';
			return false;
		}

		if (empty($input_root->attributes['userid'])) {
			$output_root->name     = 'error';
			$output_root->contents = 'userid not supplied on Input Root Node';
			return false;
		}

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input_root->attributes['userid']);
		if (is_null($user) || !$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)) {
			$output_root->name     = 'error';
			$output_root->contents = 'Unable to find User #'.$userid.' to run as or unable to set them to the current user';
			return false;
		}

		if (empty($input_root->attributes['source_code_name'])) {
			$output_root->name     = 'error';
			$output_root->contents = 'source_code_name not supplied on Input Root Node';
			return false;
		}


		$source_job = &$this->getJob($input_root->attributes['source_code_name']);
		if (is_null($source_job)) {
			trigger_error('Unable to find a source job with the codename "'.$input_root->attributes['source_code_name'].'"', E_USER_WARNING);
			return false;
		}

		$this->_tmp['prepared_server']['source_code_name'] = $source_job->code_name;

		$output_root->name = 'ack';
		return true;

	}//end prepareProcessServer()


	/**
	* Called by the daemon script to inform the hipo manager that it can start processing
	* Process the current step of a HIPO in a server environment
	*
	* This function should only be called by a HIPO running on the HIPO server, so we assume
	* no timeouts here and continually call the process function of the current step until
	* it is completed.
	*
	* @access public
	*/
	function processServer()
	{
		if (empty($this->_tmp['prepared_server'])) {
			return false;
		}

		$source_job = &$this->getJob($this->_tmp['prepared_server']['source_code_name']);
		if (is_null($source_job)) {
			trigger_error('Unable to find a source job with the codename "'.$input_root->attributes['source_code_name'].'"', E_USER_WARNING);
			return false;
		}
		unset($this->_tmp['prepared_server']);

		while(!$source_job->complete()) {
			if (!$hipo->process()) return false;
		}// end while

		return true;

	}//end processServer()

}//end class

?>