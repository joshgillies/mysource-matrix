<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_tool_delete_bmail_users.inc,v 1.3 2008/11/24 22:28:21 mbrydon Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to delete Bulkmail Users
* by email address as specified from a text file
*
* Initial Running Vars :
*    file_info				-	the text file info
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
* @subpackage bulkmail
*/
class HIPO_Job_Tool_Delete_Bmail_Users extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string $code_name              a unique codename the HIPO
	*/
	function __construct($code_name='')
	{
		$this->uses_trans = FALSE;
		parent::__construct($code_name);
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
		return translate('bm_hipo_name_delete_bmail_user');

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
					'name'			=> translate('bm_hipo_deleting_bmail_users'),
					'function_call'	=> Array(
										'process_function'	=> 'processDelete',
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
			if (!$this->processDelete($this->_steps[0], get_class($this))) {
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
		$csv->import();

		// add users into the 'todo' array
		$i = 0;
		foreach ($csv->values as $line_value) {
			$this->_running_vars['todo_asset_users'][$i]['email'] = trim($line_value[0]);
			$i++;
		}
		$this->_running_vars['done_asset_users'] = Array();
		return parent::prepare();

	}//end prepare()


	/**
	* Delete Bulkmail Users matching those specified in a text file
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processDelete(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_asset_users'])) {

			// import one bmail user
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bulkmail_user');
			$index = array_shift(array_keys($this->_running_vars['todo_asset_users']));
			$email = $this->_running_vars['todo_asset_users'][$index]['email'];

			// find the existing bmail user, and link here
			$user = $this->_getExistingBmailUser($email);
			if ($user != NULL) {
				$this->_deleteAsset($user->id);
				$step_data['message'] = "Deleting Bulkmail User \"$user->name\" (#$user->id)";
			}

			$step_data['message'] = translate('bm_deleting_bmail_user', $email);

			// add this user to the done array
			$this->_running_vars['done_asset_users'][] = $email;

			// clean up
			unset($this->_running_vars['todo_asset_users'][$index]);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

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

	}//end processDelete()


	/**
	* Find existing bmail user with the same email address.
	*
	* Returns the bmail user or null if not found.
	*
	* @param string $email  the email address we are finding
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
			throw new Exception("Unable to get existing bulkmail user due to database error: ".$e->getMessage());
		}

		$user = NULL;
		if (!is_null($assetid) && !empty($assetid)) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		}

		return $user;

	}//end _getExistingBmailUser()


	/**
	* Deletes an asset for good - bypassing the trash
	*
	* @param int	$asset_id	the asset ID of the asset to delete
	*
	* @return void
	* @access private
	*/
	function _deleteAsset($asset_id)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Delete the attribute values
		$sql = 'DELETE FROM sq_ast_attr_val WHERE assetid = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
		MatrixDAL::execPdoQuery($query);

		// Delete the unique attribute values for this asset
		$sql = 'DELETE FROM sq_ast_attr_uniq_val WHERE assetid = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
		MatrixDAL::execPdoQuery($query);

		// Delete the asset itself
		$sql = 'DELETE FROM sq_ast WHERE assetid = :assetid';
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
		MatrixDAL::execPdoQuery($query);

		// Find all existing links for the asset...
		$db = MatrixDAL::getDb();
		$sql = 'SELECT linkid
				FROM sq_ast_lnk
				WHERE minorid = :minorid
				OR majorid = :majorid';

		$result = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'minorid', $asset_id);
			MatrixDAL::bindValueToPdo($query, 'majorid', $asset_id);
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}

		// ...and delete them too
		if ($result != NULL) {
			$links_array = Array();
			foreach ($result as $value) {
				array_push($links_array, $value['linkid']);
			}
			$links_set = '('.implode(', ', $links_array).')';

			// Delete links
			$sql = 'DELETE FROM sq_ast_lnk WHERE linkid IN '.$links_set;
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::execPdoQuery($query);

			// Delete link tree entry
			$sql = 'DELETE FROM sq_ast_lnk_tree WHERE linkid IN '.$links_set;
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::execPdoQuery($query);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end _deleteAsset()


}//end class

?>
