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
* $Id: hipo_job_tool_delete_bmail_users.inc,v 1.8 2012/08/30 00:57:13 ewang Exp $
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
* @version $Revision: 1.8 $
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
		// check if email list supplied, if not look for file
		if (!isset($this->_running_vars['email_list'])) {
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
		} else {
			$emails = $this->_running_vars['email_list'];
			$i = 0;
			foreach ($emails as $email) {
				$this->_running_vars['todo_asset_users'][$i]['email'] = $email;
				$i++;
			}
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
			$am = $GLOBALS['SQ_SYSTEM']->am;
			$am->includeAsset('bulkmail_user');
			$index = array_shift(array_keys($this->_running_vars['todo_asset_users']));
			$email = $this->_running_vars['todo_asset_users'][$index]['email'];

			// find the existing bmail user, and link here
			$user = $this->_getExistingBmailUser($email);
			if ($user != NULL) {
				if (empty($this->_running_vars['root_nodes'])) {
					// If no root node specified big delete
					$am->purgeAsset($user->id);
					$step_data['message'] = "Deleting Bulkmail User \"$user->name\" (#$user->id)";
				} else {
					// Otherwise delete by root node
					$root_nodes = $this->_running_vars['root_nodes'];
					for (reset($root_nodes); NULL !== ($k = key($root_nodes)); next($root_nodes)) {
						$root_nodes[$k] = MatrixDAL::quote((string) $root_nodes[$k]);
					}//end for
					$treeid_sql = 'SELECT rt.treeid AS treeid FROM sq_ast_lnk rl INNER JOIN sq_ast_lnk_tree rt ON rl.linkid=rt.linkid WHERE rl.minorid IN ('.implode(',', $root_nodes).')';
					$treeid_result = NULL;
					try {
						$treeid_query  = MatrixDAL::preparePdoQuery($treeid_sql);
						$treeid_result = MatrixDAL::executePdoAssoc($treeid_query);
					} catch (Exception $e) {
						throw new Exception($e->getMessage());
					}//end try-catch

					$under_tree = '';
					if (!empty($treeid_result)) {
						$under_tree .= '(';
						$bind_vars = Array();
						foreach ($treeid_result as $treeid_index => $each_treeid) {
							if (!empty($each_treeid['treeid'])) {
								$bind_vars['treeid_'.$treeid_index] = $each_treeid['treeid'].'%';
								if (!empty($treeid_index)) $under_tree .= ' OR ';
								$under_tree .= 't.treeid LIKE :treeid_'.$treeid_index;
							}//end if
						}//end foreach
						$under_tree .= ')';
					}//end if

					// Something went wrong :(
					if (!empty($under_tree)) {
						$db = MatrixDAL::getDb();
						$sql = Array(
								'select'	=> 'SELECT l.linkid',
								'from'		=> 'FROM sq_ast_lnk l',
								'join'		=> 'INNER JOIN sq_ast_lnk_tree t ON l.linkid=t.linkid',
								'where'		=> 'WHERE l.minorid=:bulkmail_user_id',
							   );
						$sql2 = Array(
									'select'	=> 'SELECT l.linkid',
									'from'		=> 'FROM sq_ast_lnk l',
									'join'		=> 'INNER JOIN sq_ast_lnk_tree t ON l.linkid=t.linkid',
									'where'		=> 'WHERE l.minorid=:bulkmail_user_id AND '.$under_tree,
								);
						$result_all_links = NULL;
						$result_wanted_links = NULL;
						try {
							$query  = MatrixDAL::preparePdoQuery(implode(' ', $sql));
							$query2 = MatrixDAL::preparePdoQuery(implode(' ', $sql2));
							MatrixDAL::bindValueToPdo($query, 'bulkmail_user_id', $user->id);
							MatrixDAL::bindValueToPdo($query2, 'bulkmail_user_id', $user->id);
							foreach ($bind_vars as $bind_name => $bind_value) {
								MatrixDAL::bindValueToPdo($query2, $bind_name, $bind_value);
							}//end foreach
							$result_all_links = MatrixDAL::executePdoAssoc($query);
							$result_wanted_links = MatrixDAL::executePdoAssoc($query2);
						} catch (Exception $e) {
							throw new Exception($e->getMessage());
						}//end try-catch

						if (!empty($result_all_links) && !empty($result_wanted_links)) {
							// Cleaning up the array to make easier to work with
							$all_links = Array();
							foreach ($result_all_links as $link_info) {
								if (!empty($link_info['linkid'])) {
									$all_links[] = $link_info['linkid'];
								}//end if
							}//end foreach
							$wanted_links = Array();
							foreach ($result_wanted_links as $wanted_link_info) {
								if (!empty($wanted_link_info['linkid'])) {
									$wanted_links[] = $wanted_link_info['linkid'];
								}//end if
							}//end foreach

							// check if we are going to link in diffrent place or delete
							if (!empty($this->_running_vars['link_asset']['assetid'])) {
								$to_parentid = $this->_running_vars['link_asset']['assetid'];
								$link_type = $this->_running_vars['link_asset']['link_type'];
								foreach ($wanted_links as $linkid) {
									$am->moveLink($linkid, $to_parentid, $link_type, -1);
								}
								$step_data['message'] = "Moving Bulkmail User \"$user->name\" (#$user->id)";
							} else {
								// Check if this is the last link or going to be
								if ((count($all_links) == 1) || (count($wanted_links) == count($all_links))) {
									// Last link, so delete
									$am->purgeAsset($user->id);
									$step_data['message'] = "Deleting Bulkmail User \"$user->name\" (#$user->id)";
								} else {
									// Otherwise just delete the requested linkids
									foreach ($wanted_links as $linkid) {
										$am->deleteAssetLink($linkid);
									}//end foreach
									$step_data['message'] = "Deleting Bulkmail User \"$user->name\" (#$user->id)";
								}//end if
							}//end else
						}//end if
					}//end if
				}//end else
			}//end if

			$step_data['message'] = sprintf(translate('bm_deleting_bmail_user'), $email);

			// add this user to the done array
			$this->_running_vars['done_asset_users'][] = $email;

			// clean up
			unset($this->_running_vars['todo_asset_users'][$index]);
			$am->forgetAsset($user);

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


}//end class

?>
