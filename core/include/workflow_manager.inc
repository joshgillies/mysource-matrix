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
* $Id: workflow_manager.inc,v 1.83 2006/01/26 22:34:08 lwright Exp $
*
*/


/**
* Workflow_Manager
*
* Purpose
*    Facilitate the managment of workflow for assets
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.83 $
* @package MySource_Matrix
*/
class Workflow_Manager extends MySource_Object
{

	/**
	* Logic strings that are valid for steps
	*
	* @var array
	*/
	var $_valid_step_logic = Array('>='  => 'At least', 'all' => 'All');

	/**
	* Logic strings that are valid for conditions
	*
	* @var array
	*/
	var $_valid_cond_logic = Array('>=' => 'At least', 'all' => 'All');


	/**
	* Constructor
	*
	*/
	function Workflow_Manager()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Get an array of schemaIDs for all schemas applied and denied on an asset
	*
	* Can also optionally get running workflows only.
	*
	* Return value: Array( schemaid => granted[1|0] ) if $granted is null
	*	OR array(schemaid) if it is set
	*
	* @param int		$assetid	the ID of the asset to get schemas for
	* @param boolean	$granted	type of access : null = all, true = applied, false = denied
	* @param boolean	$running	only getting running workflows
	*
	* @return array
	* @access public
	*/
	function getSchemas($assetid, $granted=null, $running=false)
	{
		$storage_access = ((is_null($granted)) ? 2 : (int) $granted);
		if (!isset($this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = '  SELECT DISTINCT schemaid, granted
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_wflow ';
			$where = 'assetid = '.$db->quote($assetid);
			if ($running) $where .= ' AND wflow IS NOT NULL';
			if (!is_null($granted) || $running) {
				$where .= ' AND granted = '.$db->quote((($granted) ? '1' : '0'));
			}
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			assert_valid_db_result($result);

			$schemas = Array();
			foreach ($result as $data) {
				if (is_null($granted)) {
					$schemas[$data['schemaid']] = $data['granted'];
				} else {
					$schemas[] = $data['schemaid'];
				}
			}

			$this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running] = $schemas;
		}

		return $this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running];

	}//end getSchemas()


	/**
	* Apply a schema to or Deny from, an asset
	*
	* @param int		$assetid	the ID of the asset to set the schema on
	* @param int		$schemaid	the ID of the schema to set
	* @param boolean	$granted	is this schema applied (TRUE) or denied (FALSE)?
	*
	* @return boolean
	* @access public
	*/
	function setSchema($assetid, $schemaid, $granted)
	{
		$assetid  = (int)  $assetid;
		$schemaid = (int)  $schemaid;
		$granted  = (bool) $granted;

		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_localised_error('SYS0175', E_USER_WARNING, $schemaid);
			return false;
		}

		if (!is_a($schema, 'workflow_schema')) {
			trigger_localised_error('SYS0173', E_USER_WARNING, $schema->name, $schemaid);
			return false;
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0174', E_USER_WARNING, $assetid);
			return false;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_localised_error('SYS0117', E_USER_WARNING, $asset->name);
			return false;
		}

		// check for any running schemas - because if the asset is in workflow we cant do anything
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (!empty($running_schemas)) {
			trigger_localised_error('SYS0118', E_USER_WARNING, $asset->name);
			return false;
		}

		// get the current schemas that are set
		$schemas = $this->getSchemas($assetid);

		// check if this schema is already set
		if (isset($schemas[$schemaid])) {
			if ((bool)$schemas[$schemaid] == $granted) {
				// schema is set with same access level
				return true;
			} else {
				// schema is set but with the opposite access level
				$new_access     = ($granted) ? 'apply'  : 'deny';
				$current_access = ($granted) ? 'denied' : 'applied';

				trigger_localised_error('SYS0124', E_USER_WARNING, $new_access, $schema->name, $asset->name, $current_access);
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'INSERT INTO
					sq_ast_wflow
					(
						assetid,
						schemaid,
						wflow,
						granted
					)
					VALUES
					(
						?,
						?,
						?,
						?
					)';

		$prepared = $db->prepare($sql);
		assert_valid_db_result($prepared);

		$values = Array(
					$asset->id,
					$schemaid,
					null,
					(($granted) ? '1' : '0'),
				  );

		$result = $db->execute($prepared, $values);
		assert_valid_db_result($result);

		unset($this->_tmp['schemas'][(int)$assetid]);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end setSchema()


	/**
	* Remove a set schema from an asset
	*
	* @param int	$assetid	the ID of the asset to delete the schema from
	* @param int	$schemaid	the ID of the schema to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteSchema($assetid, $schemaid)
	{
		$assetid  = (int) $assetid;
		$schemaid = (int) $schemaid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0153', E_USER_WARNING, $assetid);
			return false;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_localised_error('SYS0105', E_USER_WARNING, $asset->name);
			return false;
		}

		// check for any running schemas - because if the asset is in workflow we cant do anything
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (!empty($running_schemas)) {
			trigger_localised_error('SYS0106', E_USER_WARNING, $asset->name);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_ast_wflow
				WHERE
					schemaid	= '.$db->quoteSmart($schemaid).'
				AND	assetid		= '.$db->quoteSmart($asset->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		unset($this->_tmp['schemas'][(int)$assetid]);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end deleteSchema()


	/**
	* Get the workflow values set for an asset on a particular schema or all schemas
	*
	* The asset and schema DO NOT get loaded in this function. This level of checking is
	* not really needed here but that is not the real reason. In the useSystemVersion()
	* function of asset.inc publishers of the workflow are asked for - resulting in this
	* function being called. If the asset is loaded again, an infinite loop will occur because
	* this function is called BEFORE the original asset has been stored in the factory.
	*
	* Return value: Array( schemaid => workflow_array )
	*
	* @param int	$assetid	the ID of the asset to get workflow for
	* @param int	$schemaid	the ID of the schema to get workflow for (NULL for all workflows)
	*
	* @return array
	* @access public
	* @see Asset::useSystemVersion()
	*/
	function getSchemaWorkflows($assetid, $schemaid='all')
	{
		$assetid = (int) $assetid;

		if ($schemaid != 'all') {
			// we are looking for a specific schema, but we might already have
			// the information we need in the 'all' array
			if (isset($this->_tmp['schema_workflows'][$assetid]['all'][$schemaid])) {
				$this->_tmp['schema_workflows'][$assetid][$schemaid] = $this->_tmp['schema_workflows'][$assetid]['all'][$schemaid];
			}
		}

		if (!isset($this->_tmp['schema_workflows'][$assetid][$schemaid])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT
						schemaid, wflow
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_wflow ';

			$where = 'assetid = '.$db->quote($assetid).'
						AND granted = 1';

			if ($schemaid != 'all') {
				$where .= ' AND schemaid = '.$db->quote($schemaid);
			}
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$workflows = $db->getAssoc($sql.$where, false, Array(), DB_FETCHMODE_ASSOC);
			assert_valid_db_result($workflows);

			for (reset($workflows); null !== ($k = key($workflows)); next($workflows)) {
				$workflows[$k] = (is_null($workflows[$k])) ? '' : unserialize($workflows[$k]);
			}

			if ($schemaid == 'all') {
				$this->_tmp['schema_workflows'][$assetid]['all'] = $workflows;
			} else {
				$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflows[$schemaid];
			}
		}

		if (!isset($this->_tmp['schema_workflows'][$assetid][$schemaid])) {
			return Array();
		}

		return $this->_tmp['schema_workflows'][$assetid][$schemaid];

	}//end getSchemaWorkflows()


	/**
	* Set workflow values for an asset
	*
	* @param int	$assetid	the ID of the asset to set the values for
	* @param array	$workflow	the values to set
	*
	* @return boolean
	* @access public
	*/
	function setWorkflow($assetid, $workflow)
	{
		$assetid = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0177', E_USER_WARNING, $assetid);
			return false;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_localised_error('SYS0117', E_USER_WARNING, $asset->name);
			return false;
		}

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return false;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE
					sq_ast_wflow
				SET
					wflow	= ?
				WHERE
						assetid		= ?
					AND	schemaid	= ?';

		$prepared = $db->prepare($sql);
		assert_valid_db_result($prepared);

		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0174', E_USER_WARNING, $schemaid);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}

			if (!is_a($schema, 'workflow_schema')) {
				trigger_localised_error('SYS0176', E_USER_WARNING, $schema->name, $schemaid);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}

			if (!isset($workflow[$schema->name])) continue;

			$values = Array(
						serialize($workflow[$schema->name]),
						$asset->id,
						$schema->id,
					  );

			$result = $db->execute($prepared, $values);
			assert_valid_db_result($result);

			$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflow[$schema->name];
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end setWorkflow()


	/**
	* Returns an array of all users who can publish one of the running workflows
	*
	* The publishers are based on the current step in the workflow
	* so the users returned are only users who will affect the workflow at this point.
	*
	* @param int		$assetid	the ID of the asset to get publishers for
	* @param int		$schemaid	the schema to get publishers for
	* @param boolean	$group		group the return value by workflowid
	*
	* @return array(int)
	* @access public
	*/
	function whoCanPublish($assetid, $schemaid=null, $group=false)
	{
		// if there are no current workflows running, we are not waiting for anyone
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) return Array();

		// these will be the people who can approve this asset
		// at this point in the workflow
		$can_publish = Array();

		foreach ($running_schemas as $sid) {
			if (!is_null($schemaid) && $sid != $schemaid) {
				continue;
			}
			if ($group) {
				$can_publish[$sid] = $this->_whoCanPublishWorkflow($assetid, $sid);
				$can_publish[$sid] = array_unique($can_publish[$sid]);
			} else {
				$can_publish = array_merge($can_publish, $this->_whoCanPublishWorkflow($assetid, $sid));
			}
		}

		if (!$group) $can_publish = array_unique($can_publish);
		return $can_publish;

	}//end whoCanPublish()


	/**
	* Notifies people in the workflow that want to know when the asset goes live
	*
	* @param int	$assetid	the ID of the asset that has gone live
	* @param int	$old_status	the status the asset is changing from
	*
	* @return boolean
	* @access public
	*/
	function notifyOnLive($assetid, $old_status)
	{
		// we dont have to notify people for silent workflow assets
		if ($this->silentWorkflowParty($assetid)) return true;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0180', E_USER_WARNING, $assetid);
			return false;
		}

		// if there are no workflows, we dont notify anyone
		$schemas = $this->getSchemas($assetid, true);
		if (empty($schemas)) return true;

		// these will be the people who to notify (some may be groups)
		$notify = Array();

		foreach ($schemas as $schemaid) {
			$conditions = $GLOBALS['SQ_SYSTEM']->am->getChildren($schemaid, 'workflow_step_condition');
			foreach ($conditions as $cond_id => $type_code) {
				$condition = &$GLOBALS['SQ_SYSTEM']->am->getAsset($cond_id, 'workflow_step_condition');
				if (is_null($condition)) continue;
				if ($condition->attr('notify')) {
					// the user or group in this condition wants to be notified
					$notify[] = $condition->attr('userid');
				}
			}
		}

		if (!empty($notify)) {
			$notify = array_unique($notify);
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

			$msg = $ms->newMessage($notify);
			$msg->type    = 'asset.status.notify';
			$msg_reps     = Array(
								'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
								'asset_name'	=> $asset->name,
								'status'		=> get_status_description($old_status),
							);

			// create a link to the preview screen, or to the details screen if preview doesn't exist
			$asset_edt_fns = $asset->getEditFns();
			if (isset($asset_edt_fns->static_screens['preview'])) {
				$msg_reps['asset_url'] = current_url().$asset->getBackendHref('preview');
			} else {
				$msg_reps['asset_url'] = current_url().$asset->getBackendHref('details');
			}

			$msg->parameters['assetid'] = $asset->id;
			$msg->replacements          = $msg_reps;
			$ms->enqueueMessage($msg);
		}

		return true;

	}//end notifyOnLive()


	/**
	* Determines whether commenting is mandatory for a particular user
	*
	* @param string		$assetid	the ID of the asset to check
	* @param string		$userid		the ID of the user to check
	* @param string		$mode		either 'approval' or 'rejection'
	*
	* @return boolean
	* @access public
	*/
	function requiresComment($assetid, $userid)
	{
		if ($this->silentWorkflowParty($assetid)) return false;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			// TODO: Check error
			trigger_localised_error('SYS0180', E_USER_WARNING, $assetid);
			return false;
		}

		// if there are no workflows, we can't require comments
		$schemas = $this->getSchemas($assetid, true);
		if (empty($schemas)) return false;

		$schema_workflows       = $this->getSchemaWorkflows($assetid);
		$current_workflow_steps = $this->getWorkflowCurrentSteps($assetid);

		foreach ($schemas as $schemaid) {
			// nothing in this schema, try the next
			if (empty($schema_workflows[$schemaid])) {
				continue;
			}

			// get the current step for this schema
			$current_step =& $schema_workflows[$schemaid]['steps'][$current_workflow_steps[$schemaid]];

			// is the user listed in the conditions?
			$listed_user = false;

			// loop through the conditions for the current ste
			$condition_type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo(array_keys($current_step['conditions']));
			foreach ($current_step['conditions'] as $conditionid => $condition_details) {
				// is a user_group
				if (in_array('user_group', $condition_type_info[$conditionid])) {
					$group_users = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($conditionid, Array('user'), false));
					if (in_array($userid, $group_users)) {
						$listed_user = true;
					}
				} else {
					// assume user
					if ($conditionid == $userid) {
						$listed_user = true;
					}
				}

				if ($condition_details['require_comment']) {
					// no point going through the rest if we know we need to comment
					return true;
				}

			}

			// if the user wasn't listed, then check if unlisted users need to comment
			// if so, return: there's no point checking the rest
			if (!$listed_user && $current_step['require_comment_from_unlisted_users']) {
				return true;
			}
		}

		return false;

	}//end requiresComment()


	/**
	* Returns an array of all users who can publish the running workflow for the passed schemaID
	*
	* The publishers are based on the current step in the workflow
	* so the users returned are only users who will affect the workflow at this point
	*
	* @param int	$assetid	the ID of the asset to get publishers for
	* @param int	$schemaid	the id of the schema to find publishers for
	*
	* @return array(int)
	* @access private
	*/
	function _whoCanPublishWorkflow($assetid, $schemaid)
	{
		// these will be the people who can approve this asset at this point in the workflow
		$can_publish = Array();
		$workflow = $this->getSchemaWorkflows($assetid, $schemaid);
		if (!isset($workflow['steps'])) return Array();

		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			$step_data =& $workflow['steps'][$stepid];
			if ($stepid != $workflow['current_step']) continue;

			for (reset($step_data['conditions']);
					null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
				$cond_data =& $step_data['conditions'][$pub];

				// safety code
				if (!isset($cond_data['complete'])) {
					$cond_data['complete'] = false;
				}
				if (!isset($cond_data['published_by'])) {
					$cond_data['published_by'] = Array();
				}

				if ($cond_data['complete']) {
					$complete = true;
				} else {
					$complete = false;
					$can_publish_cond = Array();

					// work out who can publish right now
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
					if ($asset->id) {
						if (is_a($asset, 'user')) {
							$can_publish_cond[] = $asset->id;
						} else if (is_a($asset, 'user_group')) {
							$can_publish_cond = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false));
						}
					}

					if ($cond_data['logic'] == 'all') {
						$cond_string = 'if ('.count($cond_data['published_by']).
									   ' >= '.count($can_publish_cond).') { $complete = true; }';
					} else {
						$cond_string = 'if ('.count($cond_data['published_by']).
									   ' '.$cond_data['logic'].') { $complete = true; }';
					}
					eval($cond_string);
				}

				if (!$complete) {
					$can_publish = array_merge($can_publish, array_diff($can_publish_cond, array_keys($cond_data['published_by'])));
				}
			}
		}

		return array_unique($can_publish);

	}//end _whoCanPublishWorkflow()


	/**
	* Goes through the passed workflow array and sets what step we are up to
	*
	* @param array	&$workflow	the workflow we are working out the current step for
	*
	* @return void
	* @access private
	*/
	function _loadCurrentStep(&$workflow)
	{
		// run through and work out what step we are up to
		$workflow['current_step'] = 0;
		$continue = false;
		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			$step_data =& $workflow['steps'][$stepid];

			$workflow['current_step'] = $stepid;
			$continue = false;
			$completed_conds = 0;

			for (reset($step_data['conditions']);
					null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
				$continue = false;
				$cond_data =& $step_data['conditions'][$pub];

				// safety code
				if (!isset($cond_data['complete'])) {
					$cond_data['complete'] = false;
				}
				if (!isset($cond_data['published_by'])) {
					$cond_data['published_by'] = Array();
				}

				if (empty($cond_data['published_by'])) {
					// nobody has approved this, so it cant be finished
					if ($step_data['logic'] == 'all') {
						$continue = false;
						break(2);
					} else {
						continue;
					}
				}

				if ($cond_data['complete']) {
					$complete = true;
				} else {
					$complete = false;
					if ($cond_data['logic'] == 'all') {
						// lets work out what number 'ALL' represents
						$can_publish_cond = Array();
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
						if ($asset->id) {
							if (is_a($asset, 'user')) {
								$can_publish_cond[] = $asset->id;
							} else if (is_a($asset, 'user_group')) {
								$can_publish_cond = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false));
							}
						}
						$cond_string = 'if ('.count($cond_data['published_by']).
									   ' >= '.count($can_publish_cond).') { $complete = true; }';
					} else {
						$cond_string = 'if ('.count($cond_data['published_by']).
									   ' '.$cond_data['logic'].') { $complete = true; }';
					}
					eval($cond_string);
				}

				if ($complete) {
					// this condition has been completed
					$cond_data['complete'] = true;
					$continue = true;
					$completed_conds++;
					if ($step_data['logic'] == 'all') continue;

					$completed = false;
					$logic_string = 'if ('.$completed_conds.' '.$step_data['logic'].' ) { $completed = true; }';
					eval($logic_string);

					if ($completed) {
						// this step is completed so move to the next one
						break;
					} else {
						// step is not finished yet so move to the next condition
						$continue = false;
						continue;
					}
				}
			}
			if ($step_data['logic'] == 'all' && $completed_conds < count($step_data['conditions'])) {
				$continue = false;
			}
			if (!$continue) break;
		}

		if ($continue) $workflow['current_step'] = 0;

	}//end _loadCurrentStep()


	/**
	* Changes the running workflows for an asset to indicate that a user has approved the asset
	*
	* @param int	$assetid	the ID of the asset to set the approval for
	* @param int	$publisher	the ID of the user who has approved
	*
	* @return boolean
	* @access public
	*/
	function recordPublish($assetid, $publisher)
	{
		$assetid = (int) $assetid;
		$asset   = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0160', E_USER_WARNING, $assetid);
			return false;
		}

		// if there are no current workflows running, how can someone publish (you do the math)?
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) return false;
		$schema_workflows = $this->getSchemaWorkflows($assetid);

		// we'll need these for later to check if any workflow has changed steps
		if (!$this->silentWorkflowParty($asset->id)) {
			$steps_before = $this->getWorkflowCurrentSteps($asset->id);
			$publishers_before = $this->whoCanPublish($asset->id);
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE
					sq_ast_wflow
				SET
					wflow	= ?
				WHERE
						assetid		= ?
					AND	schemaid	= ?';

		$prepared = $db->prepare($sql);
		assert_valid_db_result($prepared);

		for (reset($schema_workflows); null !== ($schemaid = key($schema_workflows)); next($schema_workflows)) {
			if (!in_array($schemaid, $running_schemas)) continue;
			$workflow =& $schema_workflows[$schemaid];
			if ($this->_recordPublish($workflow, $publisher)) {
				$new_workflow = serialize($workflow);

				$values = Array(
							$new_workflow,
							$assetid,
							$schemaid,
						  );

				$result = $db->execute($prepared, $values);

				assert_valid_db_result($result);

				// updated cached version of this workflow
				$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflow;
				if (isset($this->_tmp['schema_workflows'][$assetid]['all'][$schemaid])) {
					$this->_tmp['schema_workflows'][$assetid]['all'][$schemaid] = $workflow;
				}
			}
		}

		// send an internal message to let people know someone has approved
		$asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');
		$user       = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$ms         = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		$log = $ms->newMessage();
		$msg_reps = Array(
						'user_name'		=> $user->name,
						'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
						'asset_name'	=> $asset->name,
						'workflow_url'	=> current_url().$asset->getBackendHref('workflow'),
					);
		$log->replacements = $msg_reps;
		$log->type = 'asset.workflow.log.approve';
		$log->parameters['assetid'] = $asset->id;
		$log->parameters['version'] = substr($asset->version, 0, strrpos($asset->version, '.'));
		$log->send();

		// send internal messages to everyone who could approve before to let them know that someone did
		if (!$this->silentWorkflowParty($asset->id)) {
			$msg = $ms->newMessage($publishers_before);
			$msg->type    = 'asset.workflow.announce.approve';
			$msg->parameters['assetid'] = $asset->id;
			$msg->replacements = $msg_reps;
			$msg->send();
		}

		// now check if any workflows have progressed to the next step
		// and send messages to the new approvers if they have
		if (!$this->silentWorkflowParty($asset->id)) {
			$steps_after = $this->getWorkflowCurrentSteps($asset->id);
			foreach ($steps_after as $schemaid => $step) {
				if ($step > $steps_before[$schemaid]) {
					$publishers = $this->whoCanPublish($asset->id, $schemaid);
					// send internal messages to everyone in the next step who can now publish the asset
					$msg = $ms->newMessage($publishers);

					$msg_reps = Array(
									'user_name'		=> $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
									'asset_name'	=> $asset->name,
									'old_step'		=> (int)$steps_before[$schemaid],
									'new_step'		=> (int)$step,
									'workflow_url'	=> current_url().$asset->getBackendHref('workflow'),
								);

					$asset_edt_fns = $asset->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$msg_reps['preview_url'] = current_url().$asset->getBackendHref('preview');
					} else {
						$msg_reps['preview_url'] = current_url().$asset->getBackendHref('details');
					}

					$msg->type    = 'asset.workflow.invitation.progress';
					$msg->parameters['assetid'] = $asset->id;
					$msg->replacements = $msg_reps;
					$msg->send();
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end recordPublish()


	/**
	* Pretend this user has approved the workflow to see if the workflow process will be completed
	*
	* Use this to find out if the user is allowed to live edit.
	*
	* @param int	$assetid	the ID of the asset to set the approval for
	* @param int	$publisher	the ID of the user who we are testing approval for
	*
	* @return boolean
	* @access public
	*/
	function testPublish($assetid, $publisher)
	{
		$schema_workflows = $this->getSchemaWorkflows($assetid);
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) {
			// we dont actually have any running workflows, so lets pretend we do
			$schemas = $this->getSchemas($assetid, true);
			foreach ($schemas as $schemaid) {
				$schema_workflows[$schemaid] = $this->generateWorkflowArray($assetid, $schemaid);
				$running_schemas[] = $schemaid;
			}
		}

		for (reset($schema_workflows); null !== ($schemaid = key($schema_workflows)); next($schema_workflows)) {
			if (!in_array($schemaid, $running_schemas)) continue;
			$workflow =& $schema_workflows[$schemaid];
			if ($workflow['complete']) return true;
			if (!$this->_recordPublish($workflow, $publisher)) {
				return false;
			}
			if (!$workflow['complete']) return false;
		}

		return true;

	}//end testPublish()


	/**
	* Changes the passed workflow to indicate that a user has approved the asset
	*
	* Returns TRUE if the publisher was able to publish and thus that the passed workflow
	* has actually changed - FALSE otherwise.
	*
	* @param array	&$workflow	the workflow add approval to
	* @param int	$publisher	the ID of the user who has approved
	*
	* @return boolean
	* @access public
	*/
	function _recordPublish(&$workflow, $publisher)
	{
		$workflow_updated = false;

		// safety code for blank workflows
		if (empty($workflow['steps'])) {
			$workflow_updated = true;
		}

		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			if (!isset($workflow['current_step'])) {
				$workflow['current_step'] = 1;
			}
			if ($stepid != $workflow['current_step']) continue;
			$step_data =& $workflow['steps'][$stepid];

			for (reset($step_data['conditions']);
					null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {

				$cond_data =& $step_data['conditions'][$pub];
				// work out who can publish this condition
				$can_publish = Array();
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
				if ($asset->id) {
					if (is_a($asset, 'user')) {
						$can_publish[] = $asset->id;
					} else if (is_a($asset, 'user_group')) {
						$can_publish = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false));
					}
				}

				if (in_array($publisher, $can_publish)) {
					if (!isset($cond_data['published_by'])) {
						$cond_data['published_by'] = Array();
					}
					// if the publisher has not already published
					if (!isset($cond_data['published_by'][$publisher])) {
						$cond_data['published_by'][$publisher] = time();
						$workflow_updated = true;
					}
				}
			}
		}

		if ($workflow_updated) {
			// load the current step into the workflow
			$this->_loadCurrentStep($workflow);
			if ($workflow['current_step'] === 0) {
				$workflow['complete'] = true;
				$num_steps = count($workflow['steps']);
				if ($num_steps > 0) {
					$workflow['steps'][$num_steps]['completed'] = time();
				}
			} else {
				if (!$workflow['steps'][$workflow['current_step']]['started']) {
					$workflow['steps'][$workflow['current_step']]['started'] = time();
				}
				if ($workflow['current_step'] > 1 && !$workflow['steps'][$workflow['current_step'] -1]['completed']) {
					$workflow['steps'][$workflow['current_step'] -1]['completed'] = time();
				}
			}
			$updated = true;
		}

		return $workflow_updated;

	}//end _recordPublish()


	/**
	* Generate an array representing the workflow structure for a schema at this point in time
	*
	* The generated array is a snap shot of the schema at a point in time that can get written
	* to the database and used for publishing etc.
	*
	* @param int	$assetid	the ID of the asset to generate the fle for
	*
	* @access public
	* @return boolean
	*/
	function generateWorkflowArray($assetid, $schemaid)
	{
		$assetid  = (int) $assetid;
		$schemaid = (int) $schemaid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0155', E_USER_WARNING, $assetid);
			return false;
		}

		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_localised_error('SYS0156', E_USER_WARNING, $schemaid);
			return false;
		}

		if (!is_a($schema, 'workflow_schema')) {
			trigger_localised_error('SYS0154', E_USER_WARNING, $schema->name, $schemaid);
			return false;
		}

		// check that the schema we are generating an array for is actually set
		$schemas = $this->getSchemas($assetid, true);
		if (!in_array($schemaid, $schemas)) return Array();

		$edit_fns = $schema->getEditFns();
		return $edit_fns->generateWorkflowArray($schema);

	}//end generateWorkflowArray()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int						$assetid		the ID of the asset to start workflow for
	* @param boolean					$auto_approve	register that the current user has approed the workflow
	*													once it has been started
	* @param object Internal_Message	$msg			an internal message to be sent instead of the default
	*
	* @return boolean
	* @access public
	*/
	function startWorkflow($assetid, $auto_approve=true, $msg=null)
	{
		$assetid  = (int) $assetid;
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0179', E_USER_WARNING, $assetid);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$schemas = $this->getSchemas($assetid, true, false);

		if (empty($schemas)) return false;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE
					sq_ast_wflow
				SET
					wflow	= ?
				WHERE
						assetid		= ?
					AND	schemaid	= ?';

		$prepared = $db->prepare($sql);
		assert_valid_db_result($prepared);

		foreach ($schemas as $schemaid) {
			$workflow = $this->generateWorkflowArray($assetid, $schemaid);
			if (empty($workflow['steps'])) continue;
			$workflow = serialize($workflow);

			$values = Array(
						$workflow,
						$assetid,
						$schemaid,
					  );

			$result = $db->execute($prepared, $values);
			assert_valid_db_result($result);
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$this->_tmp = Array();

		if ($auto_approve) {
			// if the user who has started this workflow is also able to
			// approve it as part of workflow, let their action to start workflow
			// signal their intention to approve it as well
			$publishers = $this->whoCanPublish($asset->id);
			$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
			if (in_array($current_userid, $publishers)) {
				if (!$this->recordPublish($asset->id, $current_userid)) {
					trigger_localised_error('SYS0078', E_USER_WARNING, $current_userid);
					// dont die here because they can try to approve again later, we just want to warn them
				}
			}
		}

		// send internal messages to everyone who can now publish the asset
		$ms   = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$publishers = $this->whoCanPublish($asset->id);

		if (!$this->silentWorkflowParty($asset->id)) {
			// create a new internal message if we have not been supplied with one
			if (is_null($msg)) $msg = $ms->newMessage();
			// added the publishers to the list of people that are to be notified
			$msg->to = array_merge($msg->to, $publishers);

			// complete any missing internal message fields that have not been supplied
			if (empty($msg->replacements)) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
				$msg_reps = Array(
								'user_name'		=> $user->name,
								'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
								'asset_name'	=> $asset->name,
								'workflow_url'	=> current_url().$asset->getBackendHref('workflow'),
							);
				$asset_edt_fns = $asset->getEditFns();
				if (isset($asset_edt_fns->static_screens['preview'])) {
					$msg_reps['preview_url'] = current_url().$asset->getBackendHref('preview');
				} else {
					$msg_reps['preview_url'] = current_url().$asset->getBackendHref('details');
				}
				$msg->replacements = $msg_reps;
			}
			if (empty($msg->type)) {
				$msg->type = 'asset.workflow.invitation';
			}

			$msg->parameters['assetid'] = $asset->id;
			$ms->enqueueMessage($msg);
		}

		return true;

	}//end startWorkflow()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int	$assetid	the ID of the asset to check workflow completness for
	*
	* @return boolean
	* @access public
	*/
	function isWorkflowComplete($assetid)
	{
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) return true;
		$schema_workflows = $this->getSchemaWorkflows($assetid);

		foreach ($schema_workflows as $schemaid => $workflow) {
			if (!in_array($schemaid, $running_schemas)) continue;
			if (empty($workflow['steps'])) continue;
			if (!$workflow['complete']) return false;
		}

		return true;

	}//end isWorkflowComplete()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int	$assetid	the ID of the asset to check workflow completness for
	*
	* @return Array(schemaid=>step_no)
	* @access public
	*/
	function getWorkflowCurrentSteps($assetid)
	{
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) return Array();
		$schema_workflows = $this->getSchemaWorkflows($assetid);

		$steps = Array();
		foreach ($schema_workflows as $schemaid => $workflow) {
			if (!in_array($schemaid, $running_schemas)) continue;
			if (empty($workflow['steps'])) continue;
			$steps[$schemaid] = $workflow['current_step'];
		}

		return $steps;

	}//end getWorkflowCurrentSteps()


	/**
	* Cancel the currently running workflows for the passed asset
	*
	* @param int						$assetid	the ID of the asset to cancel workflow for
	* @param object Internal_Message	$msg		an internal message to be sent instead of the default
	*
	* @return boolean
	* @access public
	*/
	function cancelWorkflow($assetid, $msg=null)
	{
		$assetid = (int) $assetid;
		$asset   = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0146', E_USER_WARNING, $assetid);
			return false;
		}

		// we have to get the publishers before the workflow is cleared, because
		// this function depends on having a workflow running
		$publishers = $this->whoCanPublish($asset->id);

		if (!$this->_clearWorkflow($assetid)) return false;

		// send internal messages to everyone who could approve before to let them know that someone cancelled
		if (!$this->silentWorkflowParty($asset->id)) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

			// create a new internal message if we have not been supplied with one
			if (is_null($msg)) $msg = $ms->newMessage();
			// added the publishers to the list of people that are to be notified
			$msg->to = array_merge($msg->to, $publishers);

			// complete any missing internal message fields that have not been supplied
			if (empty($msg->replacements)) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
				$msg_reps = Array(
								'user_name'		=> $user->name,
								'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
								'asset_name'	=> $asset->name,
								'workflow_url'	=> current_url().$asset->getBackendHref('workflow'),
							);
				$asset_edt_fns = $asset->getEditFns();
				if (isset($asset_edt_fns->static_screens['preview'])) {
					$msg_reps['preview_url'] = current_url().$asset->getBackendHref('preview');
				} else {
					$msg_reps['preview_url'] = current_url().$asset->getBackendHref('details');
				}
				$msg->replacements = $msg_reps;
			}
			if (empty($msg->type)) {
				$msg->type = 'asset.workflow.announce.reject';
			}

			$msg->parameters['assetid'] = $asset->id;
			$ms->enqueueMessage($msg);
		}

		return true;

	}//end cancelWorkflow()


	/**
	* Completes the workflow by clearing out all the running workflows from the DB
	*
	* @param int	$assetid	the ID of the asset to complete workflow for
	*
	* @return boolean
	* @access public
	*/
	function completeWorkflow($assetid)
	{
		$assetid = (int) $assetid;
		$asset   = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0150', E_USER_WARNING, $assetid);
			return false;
		}

		return $this->_clearWorkflow($assetid);

	}//end completeWorkflow()


	/**
	* Clears all generated workflow arrays from the workflow table for the passed assetID
	*
	* @param int	$assetid	the ID of the asset to clear workflow for
	*
	* @return boolean
	* @access public
	*/
	function _clearWorkflow($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE
					sq_ast_wflow
				SET
					wflow = NULL
				WHERE
					assetid	 = ?';

		$prepared = $db->prepare($sql);
		assert_valid_db_result($prepared);

		$values = Array($assetid);

		$result = $db->execute($prepared, $values);
		assert_valid_db_result($result);

		unset($this->_tmp['schemas'][$assetid]);
		unset($this->_tmp['schema_workflows'][$assetid]);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _clearWorkflow()


	/**
	* Deletes any workflow entries that this asset has
	*
	* @param string	$assetid	the asset id
	*
	* @return boolean
	* @access void
	*/
	function purgeWorkflow($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_ast_wflow
				WHERE
					assetid	= '.$db->quoteSmart($assetid);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end purgeWorkflow()


	/**
	* Check if the passed assetid is a silent party in the worflow process
	*
	* For example, when pulling a standard page through workflow, the bodycopy
	* is a silent party because it cannot be shared and so has the same admins
	* as the standard page. However, any tables can be shared and so they are not
	* silent parties.
	*
	* @param int	$assetid	the ID of the asset to check
	*
	* @access public
	* @return boolean
	*/
	function silentWorkflowParty($assetid)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset((int) $assetid);
		if (is_null($asset)) return true;
		$edit_fns = $asset->getEditFns();
		return !isset($edit_fns->static_screens['workflow']);

	}//end silentWorkflowParty()


}//end class

?>
