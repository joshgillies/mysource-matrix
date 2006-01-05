<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_tool_import_bmail_user.inc,v 1.2 2006/01/05 22:39:05 rong Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to import bmail users from a CSV file
*
* Initial Running Vars :
*    create_in_assetid	-	the ID of user group to create bmail users in
*    file_info			-	the CSV file info
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Tool_Import_Bmail_User extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Tool_Import_Bmail_User($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('bulkmail');

	}//end constructor


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
		return parent::getCodeName().'-'.$this->_running_vars['file_info']['name'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('bm_hipo_name_import_bmail_user');

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
					'name'			=> translate('bm_hipo_importing_bmail_users'),
					'function_call'	=> Array(
										'process_function'	=> 'processImport',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
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
		while (!empty($this->_running_vars['todo_asset_users'])) {
			if (!$this->processImport($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// import the csv file
		require_once SQ_FUDGE_PATH.'/csv/csv.inc';
		$csv =& new CSV($this->_running_vars['file_info']['tmp_name']);
		$csv->import();

		// add users into the 'todo' array
		$i = 0;
		foreach ($csv->values as $line_value) {
			$this->_running_vars['todo_asset_users'][$i]['name'] = trim($line_value[0]);
			$this->_running_vars['todo_asset_users'][$i]['email'] = trim($line_value[1]);
			$i++;
		}
		$this->_running_vars['done_asset_users'] = Array();
		return parent::prepare();

	}//end prepare()


	/**
	* Import Bulkmail Users from a CSV file
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processImport(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_asset_users'])) {

			// import one bmail user
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bulkmail_user');
			$index = array_shift(array_keys($this->_running_vars['todo_asset_users']));

			// create this user in the selected user group
			$create_in =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['create_in_assetid']);
			$user =& new Bulkmail_User();
			$user->setAttrValue('name', $this->_running_vars['todo_asset_users'][$index]['name']);
			$user->setAttrValue('email', $this->_running_vars['todo_asset_users'][$index]['email']);
			$link = Array(
						'asset'		=> $create_in,
						'link_type'	=> SQ_LINK_TYPE_1,
					);
			// $user->processStatusChange(SQ_STATUS_LIVE, FALSE);
			$status = $user->create($link);

			// add this user to the done array
			$step_data['message'] = translate('bm_importing_bmail_user', $user->attr('email'));
			$this->_running_vars['done_asset_users'][] = $user->attr('email');

			// clean up
			unset($this->_running_vars['todo_asset_users'][$index]);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($create_in);
		}

		// update progress
		if (empty($this->_running_vars['todo_asset_users'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_asset_users']) + count($this->_running_vars['done_asset_users']);
			$step_data['percent_done'] =(count($this->_running_vars['done_asset_users']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processImport()


}//end class

?>
