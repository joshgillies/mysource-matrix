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
* $Id: hipo_job_tool_import_bmail_user.inc,v 1.11 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to import bmail users from a CSV file
*
* Initial Running Vars :
*    create_in_assetid	-	the ID of user group to create bmail users in
*    file_info			-	the CSV file info
*    new_link			-	whether to add a new link for existing bmail user (bool)
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.11 $
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
	function __construct($code_name='')
	{
		$this->uses_trans = FALSE;
		parent::__construct($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('bulkmail');

	}//end constructor

	
	/**
	* Paints backend of this job's configuration
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		$o->openField(translate('bm_tool_import_bmail_user_hipo_job_import_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_BULKMAIL_USER_IMPORT_THRESHOLD]', SQ_HIPO_BULKMAIL_USER_IMPORT_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_BULKMAIL_USER_IMPORT_THRESHOLD;
		}
		echo ' '.translate('assets');
		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_BULKMAIL_USER_IMPORT_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
			   );

	}//end getConfigVars()

	
	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in
	* freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_BULKMAIL_USER_IMPORT_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_asset_users'])) {
			return 0;
		}
		$user_count = count($this->_running_vars['todo_asset_users']);
		
		return (($user_count / SQ_HIPO_BULKMAIL_USER_IMPORT_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()
	

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
		$csv = new CSV($this->_running_vars['file_info']['tmp_name']);

		$csv_start = 1;
		if (isset($this->_running_vars['csv_header']) && $this->_running_vars['csv_header'] == TRUE) {
			$csv_start = 2;
		}
		$csv->import($csv_start);

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
			$array_keys = array_keys($this->_running_vars['todo_asset_users']);
			$index = array_shift($array_keys);
			$create_in = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['create_in_assetid']);
			$name = $this->_running_vars['todo_asset_users'][$index]['name'];
			$email = $this->_running_vars['todo_asset_users'][$index]['email'];

			$user = new Bulkmail_User();
			$user->setAttrValue('name', $name);

			// turn off error reporting if we are linking user with same address
			if ($this->_running_vars['new_link']) {
				$old_reporting = error_reporting(0);
			}
			$status = $user->setAttrValue('email', $email);
			if ($this->_running_vars['new_link']) {
				error_reporting($old_reporting);
				if (!$status) {
					// find the existing bmail user, and link here
					$user = $this->_getExistingBmailUser($email);
					if ($name != $user->name) {
						trigger_error("(Bulkmail User #$user->id) \"$name\" does not match existing name \"$user->name\"");
						$step_data['message'] = "Skipping \"$user->name\" (#$user->id)";
					} else {
						$create_in->createLink($user, SQ_LINK_TYPE_1);
						$step_data['message'] = "Linking \"$user->name\" (#$user->id)";
					}
				}
			}

			// create new user as usual
			if ($status) {
				$link = Array(
							'asset'		=> $create_in,
							'link_type'	=> SQ_LINK_TYPE_1,
						);
				$status = $user->create($link);
			}
			$step_data['message'] = translate('bm_importing_bmail_user', $email);

			// add this user to the done array
			$this->_running_vars['done_asset_users'][] = $email;

			// clean up
			unset($this->_running_vars['todo_asset_users'][$index]);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($create_in);

		}//end if not empty todo

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


	/**
	* Find existing bmail user with the same email address.
	*
	* Returns the bmail user or null if not found.
	*
	* @param string	$email	the email address we are finding
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getExistingBmailUser($email)
	{
		try {
			$bind_vars['email'] = $email;
			$assetid = MatrixDAL::executeOne('bulkmail_package', 'getExistingBmailUser', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get existing bulkmail user due to database error: '.$e->getMessage());
		}

		$user = NULL;
		if (!is_null($assetid)) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		}

		return $user;

	}//end _getExistingBmailUser()


}//end class


?>
