<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: hipo_job_update_lookups.inc,v 1.12 2003/10/15 23:42:27 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to update lookups
* 
* <pre>
* Initial Running Vars : 
*    assetids	-	an array of id's of assets that we will be starting the update from
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage lib
*/
class HIPO_Job_Update_Lookups extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Update Lookups';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array(
						'name'             => 'Updating Lookups',
						'function_call'    => Array(
												'process_function' => 'processLookups',
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
	function HIPO_Job_Update_Lookups($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Update Lookups HIPO appends the ID of the asset that the lookups are being updated at
	* (the top most parent) so that any other update lookup jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		return parent::_getCodeName().'-'.implode(',', $this->_running_vars['assetids']);

	}//end _getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Update Lookups HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		// to start updating lookups, we need to have an asset id to start updating lookups at
		if (empty($this->_running_vars['assetids'])) {
			trigger_error('Cannot initialise Update Lookups HIPO, no start asset IDs specified', E_USER_WARNING);
			return false;
		}

		$todo_assetids = $this->_running_vars['assetids'];
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		
		foreach($this->_running_vars['assetids'] as $assetid) {
			
			$where = 'l.minorid = '.$db->quote($assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql = 'SELECT t.treeid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
					'.$where.' LIMIT 1';

			$treeid = $db->getOne($sql);
			if (DB::isError($treeid)) {
				trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			}

			$sql = 'SELECT l.minorid, MAX(CHAR_LENGTH(t.treeid)) as length
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
					         INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid
					';
			$where = 't.treeid LIKE '.$db->quote($treeid.'%').'
					  GROUP BY l.minorid ORDER BY length';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			
			$todo_assets = $db->getCol($sql.$where);
		
		} // end foreach
		
		$this->_running_vars['todo_assetids'] = array_unique($todo_assets);
		$this->_running_vars['done_assetids'] = Array();

		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Update lookups for a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processLookups(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				if (!$asset->updateLookups()) return false;
				$step_data['message'] = 'Updating Lookups for "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not update lookups for asset #'.$assetid.', asset does not exist');
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

	}//end processLookups()

}//end class

?>