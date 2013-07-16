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
* $Id: workflow_manager.inc,v 1.183.2.1 2013/07/16 00:51:18 lwright Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/datetime.inc';


/**
* Workflow_Manager
*
* Purpose
*    Facilitate the managment of workflow for assets
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.183.2.1 $
* @package MySource_Matrix
*/
class Workflow_Manager extends MySource_Object
{

	/**
	* Logic operators require the right operands
	*
	* @var array
	*/
	var $_valid_logic_with_right_operand = Array('>=');

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
	* Check if workflow can be set on an asset
	*
	* @param string	$assetid	the ID of the asset to check
	*
	* @return boolean
	* @access public
	*/
	function allowsWorkflow($assetid)
	{
		if (preg_match('/:/',$assetid)) return FALSE;
		return TRUE;

	}//end allowsWorkflow()


	/**
	* Get an array of schemaIDs for all schemas applied and denied on an asset
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
	function getSchemas($assetid, $granted=NULL, $running=FALSE, $cascades=NULL)
	{
		$storage_access = ((is_null($granted)) ? 2 : (int) $granted);
		if (!isset($this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running])) {

			$sql = '  SELECT DISTINCT schemaid, granted, cascades
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_wflow ';
			$where = 'assetid = :assetid';
			if ($running) {
				if (MatrixDAL::getDbType() === 'oci') {
					$where .= ' AND DBMS_LOB.GETLENGTH(wflow) > 0';
				} else {
					$where .= ' AND wflow IS NOT NULL';
				}
			}
			if (!is_null($granted) || $running) {
				$where .= ' AND granted = :granted';
			}
			if (!is_null($cascades)) {
				$where .= ' AND cascades = :cascades';
			}
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid, PDO::PARAM_STR);
				if (!is_null($granted) || $running) {
					if ($running) {
						$granted_bind = '1';
					} else {
						$granted_bind = $granted ? '1' : '0';
					}
					MatrixDAL::bindValueToPdo($query, 'granted', $granted_bind, PDO::PARAM_STR);
				}
				if (!is_null($cascades)) {
					MatrixDAL::bindValueToPdo($query, 'cascades', $cascades ? '1' : '0', PDO::PARAM_STR);
				}
				$result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get workflow schemas for asset ID #'.$assetid.' due to database error: '.$e->getMessage());
			}

			$schemas = Array();
			foreach ($result as $data) {
				if (is_null($granted)) {
					$schemas[$data['schemaid']] = $data['granted'];
				} else {
					$schemas[] = $data['schemaid'];
				}
			}

			$this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running] = $schemas;
		}//end if

		return $this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running];

	}//end getSchemas()


	/**
	* Get list of schemas for a certain asset, optionally filtered by cascading setting
	*
	* This is primarily used for showing settings on the Metadata Schemas
	* screen.
	*
	* @param int		$assetid					the ID of the asset
	* @param int		$schemaid					the ID of the schema
	* @param boolean 	$cascades					limit to cascades setting : null = all, true = on, false = off
	* @param boolean 	$include_cascades			include schema cascades info
	* @param boolean 	$include_last_started_by	include schema last_started_by info
	*
	* @return multitype:
	*/
	function getAssetSchemaInfo($assetid, $schemaid=NULL, $cascades=NULL, $include_cascades=TRUE, $include_last_started_by=FALSE, $include_last_stream=FALSE)
	{
		$sql = '  SELECT DISTINCT schemaid, granted, cascades, last_started_by, last_stream
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_wflow ';
		$where = 'assetid = :assetid';
		if (!is_null($schemaid)) {
			$where .= ' AND schemaid = :schemaid';
		}
		if (!is_null($cascades)) {
			$where .= ' AND cascades = :cascades';
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid, PDO::PARAM_STR);
			if (!is_null($schemaid)) {
				MatrixDAL::bindValueToPdo($query, 'schemaid', (int)$schemaid, PDO::PARAM_INT);
			}
			if (!is_null($cascades)) {
				MatrixDAL::bindValueToPdo($query, 'cascades', $cascades ? '1' : '0', PDO::PARAM_STR);
			}
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get asset workflow schemas of asset ID #'.$assetid.' due to database error: '.$e->getMessage());
		}

		$schemas = Array();
		foreach ($result as $data) {
			if ($include_cascades || $include_last_started_by || $include_last_stream) {
				$schemas[$data['schemaid']] = Array(
												'granted'	=> $data['granted'],
											  );
				if ($include_cascades) $schemas[$data['schemaid']]['cascades'] = $data['cascades'];
				if ($include_last_started_by) $schemas[$data['schemaid']]['last_started_by'] = $data['last_started_by'];
				if($include_last_stream) $schemas[$data['schemaid']]['last_stream'] = $data['last_stream'];

			} else {
				$schemas[$data['schemaid']] = $data['granted'];
			}
		}

		// If a schemaid is specified, flatten out array
		if (!is_null($schemaid)) {
			if (array_key_exists($schemaid, $schemas)) {
				$schemas = $schemas[$schemaid];
			} else {
				$schemas = Array();
			}
		}

		return $schemas;

	}//end getAssetSchemaInfo()


	/**
	* Apply a schema to or Deny from, an asset
	*
	* @param int		$assetid	the ID of the asset to set the schema on
	* @param int		$schemaid	the ID of the schema to set
	* @param boolean	$granted	is this schema applied (TRUE) or denied (FALSE)?
	* @param boolean	$force_set	if true, existing workflow schemas will be updated regardless of whether or not the new workflow schemas are found to conflict with them
	*
	* @return boolean
	* @access public
	*/
	function setSchema($assetid, $schemaid, $granted, $cascades=TRUE, $force_set=FALSE)
	{
		$assetid  = (int)  $assetid;
		$schemaid = (int)  $schemaid;
		$granted  = (bool) $granted;
		$db_action = 'insert';

		$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_localised_error('SYS0175', E_USER_WARNING, $schemaid);
			return FALSE;
		}

		if (!($schema instanceof Workflow_Schema)) {
			trigger_localised_error('SYS0173', E_USER_WARNING, $schema->name, $schemaid);
			return FALSE;
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0174', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_localised_error('SYS0117', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// check for any running schemas - because if the asset is in workflow we cant do anything
		$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);
		if (!empty($running_schemas)) {
			trigger_localised_error('SYS0118', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// get the current schemas that are set
		$schema_info = $this->getAssetSchemaInfo($assetid, $schemaid);

		// check if this schema is already set
		if (!empty($schema_info)) {

			if ((bool)$schema_info['granted'] == $granted) {
				// schema is set with same access level
				if ((bool)$schema_info['cascades'] == $cascades) {
					// same cascade level too, so no update needed
					return TRUE;
				} else {
					// same access level, but different cascade, so update it
					$db_action = 'update';
				}
			} else {
				if ($force_set) {
					$db_action = 'update';
				} else {
					// schema is set but with the opposite access level
					$new_access     = ($granted) ? 'apply'  : 'deny';
					$current_access = ($granted) ? 'denied' : 'applied';

					trigger_localised_error('SYS0124', E_USER_WARNING, $new_access, $schema->name, $asset->name, $current_access);
					return FALSE;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($db_action == 'insert') {
			$query_name = 'setWorkflowSchema';
		} else if ($db_action == 'update') {
			$query_name = 'updateWorkflowSchema';
		}

		try {
			$bind_vars = Array(
							'assetid'	=> (string)$assetid,
							'schemaid'	=> (string)$schemaid,
							'wflow'		=> NULL,
							'granted'	=> $granted ? '1' : '0',
							'cascades'	=> $cascades ? '1' : '0',
						 );
			MatrixDAL::executeQuery('core', $query_name, $bind_vars);
		} catch (DALException $e) {
			throw new Exception ('Unable to set workflow schema for "'.$asset->name.'" (#'.$asset->id.') due to database error: '.$e->getMessage());
		}

		unset($this->_tmp['schemas'][(int)$assetid]);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setSchema()


	/**
	* Remove a set schema from an asset
	*
	* @param int		$assetid	the ID of the asset to delete the schema from
	* @param int		$schemaid	the ID of the schema to delete
	* @param boolean	$running	whether to check if the asset has running schemas
	*
	* @return boolean
	* @access public
	*/
	function deleteSchema($assetid, $schemaid, $running = TRUE)
	{
		$assetid  = (int) $assetid;
		$schemaid = (int) $schemaid;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0153', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_localised_error('SYS0105', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		if ($running) {
			// check for any running schemas - because if the asset is in workflow we cant do anything
			$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);
			if (!empty($running_schemas)) {
				trigger_localised_error('SYS0106', E_USER_WARNING, $asset->name);
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars = Array(
							'assetid'	=> (string)$asset->id,
							'schemaid'	=> (string)$schemaid,
						 );
			MatrixDAL::executeQuery('core', 'deleteWorkflowSchema', $bind_vars);
		} catch (DALException $e) {
			throw new Exception ('Unable to delete workflow schema for "'.$asset->name.'" (#'.$asset->id.') due to database error: '.$e->getMessage());
		}

		unset($this->_tmp['schemas'][(int)$assetid]);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteSchema()


	/**
	* Get the workflow values set for an asset on a particular schema or all schemas
	*
	* Return value: Array( schemaid => workflow_array )
	*
	* @param int	$assetid	the ID of the asset to get workflow for
	* @param int	$schemaid	the ID of the schema to get workflow for ('all' or omit for all workflows)
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
			$workflows = Array();
			$bind_vars = Array();

			$sql = 'SELECT
						schemaid, wflow
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_wflow ';

			$where = 'assetid = :assetid
						AND granted = \'1\'';
			$bind_vars['assetid'] = $assetid;

			if ($schemaid != 'all') {
				$where .= ' AND schemaid = :schemaid';
				$bind_vars['schemaid'] = $schemaid;
			}
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				$results = MatrixDAL::executePdoGroupedAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get workflow schema(s) for asset: '.$assetid.' due to database error: '.$e->getMessage());
			}

			foreach ($results as $assetid => $serialized_wflow) {
				$workflows[$assetid] = (empty($serialized_wflow[0]['wflow']) ? '' : unserialize($serialized_wflow[0]['wflow']));
			}

			if ($schemaid == 'all') {
				$this->_tmp['schema_workflows'][$assetid]['all'] = $workflows;
			} else {
				$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflows[$schemaid];
			}
		}//end if

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

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0177', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_localised_error('SYS0117', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		$schemas = $this->getSchemas($asset->id, TRUE);
		if (empty($schemas)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($schemas as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0174', E_USER_WARNING, $schemaid);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (!($schema instanceof Workflow_Schema)) {
				trigger_localised_error('SYS0176', E_USER_WARNING, $schema->name, $schemaid);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (!isset($workflow[$schema->name])) continue;

			try {
				$bind_vars = Array(
								'wflow'		=> serialize($workflow[$schema->name]),
								'assetid'	=> $asset->id,
								'schemaid'	=> $schema->id,
							 );
				MatrixDAL::executeQuery('core', 'updateWorkflow', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to update workflow table for asset: '.$assetid.' due to database error: '.$e->getMessage());
			}

			$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflow[$schema->name];
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

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
	* @return array
	* @access public
	*/
	function whoCanPublish($assetid, $schemaid=NULL, $group=FALSE)
	{
		// if there are no current workflows running, we are not waiting for anyone
		$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);
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
		if ($this->silentWorkflowParty($assetid)) return TRUE;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0180', E_USER_WARNING, $assetid);
			return FALSE;
		}

		// if there are no workflows, we dont notify anyone
		$schemas = $this->getAssetSchemaInfo($assetid, NULL, NULL, FALSE, TRUE, TRUE);
		if (empty($schemas)) return TRUE;

		foreach ($schemas as $schemaid => $schema_info) {
			if (!$schema_info['granted']) continue;

			// these will be the people who to notify (some may be groups)
			$notify = Array();

			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
			if ($schema && $schema->attr('notify_starter_on_live')){
				if (!empty($schema_info['last_started_by'])){
					$started_by_user = $GLOBALS['SQ_SYSTEM']->am->getAsset($schema_info['last_started_by'], '', TRUE);
					if ($started_by_user){
						// see bug #4986 Edit User doesnt get workflow emails initiated via EES
						// and see simple_edit_user::canAccessBackend()
						if ($started_by_user instanceof Simple_Edit_User) $started_by_user->_tmp['starter_of_workflow'] = TRUE;
						$notify[] = $schema_info['last_started_by'];
					}
				}
			}
			
			// only notify conditions in current stream
			$stream_exists = $GLOBALS['SQ_SYSTEM']->am->assetExists($schema_info['last_stream']);
			if($stream_exists) {
			    $conditions = $GLOBALS['SQ_SYSTEM']->am->getChildren($schema_info['last_stream'], 'workflow_step_condition');
			}
			else {
			    $conditions = $GLOBALS['SQ_SYSTEM']->am->getChildren($schemaid, 'workflow_step_condition');
			}
			
			foreach ($conditions as $cond_id => $type_code) {
				$condition = $GLOBALS['SQ_SYSTEM']->am->getAsset($cond_id, 'workflow_step_condition');
				if (is_null($condition)) continue;
				if ($condition->attr('notify')) {
					// check if this is a roleid rather than a user_groupid or userid
					if (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1') {
						$fetch_global_roles = (SQ_CONF_ENABLE_GLOBAL_ROLES == '1');
						$role = $GLOBALS['SQ_SYSTEM']->am->getRole($assetid, $condition->attr('userid'), NULL, FALSE, $fetch_global_roles);
						if (!empty($role)) {
							foreach ($role as $roleid => $userids) {
								for ($i=0; $i<count($userids); $i++) {
									$notify[] = $userids[$i];
								}
							}
						} else {
							// the user or group in this condition wants to be notified
							$notify[] = $condition->attr('userid');
						}
					} else {
						// the user or group in this condition wants to be notified
						$notify[] = $condition->attr('userid');
					}
				}
			}//end foreach

			if (!empty($notify)) {
				$notify = array_unique($notify);
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
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
					$msg_reps['asset_url'] = current_url().$asset->getBackendHref('preview', FALSE);
				} else {
					$msg_reps['asset_url'] = current_url().$asset->getBackendHref('details', FALSE);
				}

				$msg->replacements  = $msg_reps;

				if ($schema){
					// Does this schema have a custom message?
					if (trim($schema->attr('message_notify_on_live')) !== '' ) {
						$msg->body = trim($schema->attr('message_notify_on_live'));
					}//end if

					// Does this schema have a custom subject?
					if (trim($schema->attr('subject_notify_on_live')) !== '') {
						$subject = trim($schema->attr('subject_notify_on_live'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Made Live';
					}//end if

					if ($schema->attr('schema_reply_to_email_address') != '') {
						$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
					} else {
						$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
					}
					if ($schema->attr('schema_from_email_address') != '') {
						$msg->from = $schema->attr('schema_from_email_address');
					}
				}//end if

				$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);

				$msg->parameters['assetid'] = $asset->id;
				$ms->enqueueMessage($msg);
			}
		}//end foreach

		return TRUE;

	}//end notifyOnLive()


	/**
	* Determines whether commenting is mandatory for a particular user
	*
	* @param string	$assetid	the ID of the asset to check
	* @param string	$userid		the ID of the user to check
	*
	* @return boolean
	* @access public
	*/
	function requiresComment($assetid, $userid)
	{
		if ($this->silentWorkflowParty($assetid)) return FALSE;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			// TODO: Check error
			trigger_localised_error('SYS0180', E_USER_WARNING, $assetid);
			return FALSE;
		}

		// if there are no workflows, we can't require comments
		$schemas = $this->getSchemas($assetid, TRUE);
		if (empty($schemas)) return FALSE;

		$schema_workflows       = $this->getSchemaWorkflows($assetid);
		$current_workflow_steps = $this->getWorkflowCurrentSteps($assetid);

		foreach ($schemas as $schemaid) {
			// nothing in this schema, try the next
			if (empty($schema_workflows[$schemaid])) continue;

			// check to see if the step exists
			if (empty($current_workflow_steps[$schemaid])) {
				continue;
			}

			// get the current step for this schema
			$current_step =& $this->getCurrentStep($schema_workflows[$schemaid]);
			// is the user listed in the conditions?
			$listed_user = FALSE;

			// loop through the conditions for the current ste
			$condition_type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo(array_keys($current_step['conditions']));
			foreach ($current_step['conditions'] as $conditionid => $condition_details) {
				if (!isset($condition_type_info[$conditionid])) {
					continue;
				}
				// is a user_group
				if (in_array('user_group', $condition_type_info[$conditionid])) {
					$group_users = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($conditionid, Array('user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE));
					if (in_array($userid, $group_users)) {
						$listed_user = TRUE;
					}
				} else {
					// assume user
					if ($conditionid == $userid) $listed_user = TRUE;
				}

				if (array_get_index($condition_details, 'require_comment', FALSE)) {
					// no point going through the rest if we know we need to comment
					return TRUE;
				}

			}

			// if the user wasn't listed, then check if unlisted users need to comment
			// if so, return: there's no point checking the rest
			if (!$listed_user && array_get_index($current_step, 'require_comment_from_unlisted_users', FALSE)) {
				return TRUE;
			}
		}//end foreach

		return FALSE;

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
	* @return array
	* @access private
	*/
	function _whoCanPublishWorkflow($assetid, $schemaid)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		// these will be the people who can approve this asset at this point in the workflow
		$can_publish = Array();
		$workflow = $this->getSchemaWorkflows($assetid, $schemaid);
		if (!isset($workflow['steps'])) return Array();

		$step_data =& $this->getCurrentStep($workflow);
		if (empty($step_data)) return Array();

		for (reset($step_data['conditions']);
				NULL !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
			$cond_data =& $step_data['conditions'][$pub];

			// safety code
			if (!isset($cond_data['complete'])) {
				$cond_data['complete'] = FALSE;
			}
			if (!isset($cond_data['published_by'])) {
				$cond_data['published_by'] = Array();
			}

			if ($cond_data['complete']) {
				$complete = TRUE;
			} else {
				$complete = FALSE;
				$can_publish_cond = Array();

				// work out who can publish right now
				if (!$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($pub, Array('user', 'user_group'), FALSE)) {
					continue;
				}
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
				if ($asset->id) {
					$asset_class = get_class($asset);
					// Note that the order of IF statements is important, because roles are
					// a descendent of groups but should be treated differently
					if ($am->isTypeDecendant($asset_class, 'user')) {
						$can_publish_cond[] = $asset->id;
					} else if ($am->isTypeDecendant($asset_class, 'role')) {
						// expand global users
						if (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1') {
							$fetch_global_roles = (SQ_CONF_ENABLE_GLOBAL_ROLES == '1');
							$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($assetid, $asset->id, NULL, FALSE, TRUE);
							foreach ($roles as $roleid => $userids) {
								foreach ($userids as $userid) {
									$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), Array(), FALSE, 'type_code');
									if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($info[$userid], 'user_group')) {
										$can_publish_cond = array_merge($can_publish_cond, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($userid, Array('user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE)));
									} else {
										$can_publish_cond[] = $userid;
									}
								}
							}
						}
					} else if ($am->isTypeDecendant($asset_class, 'user_group')) {
						// getChildren with all the shadow asset children
						$can_publish_cond = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE));
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

				if ($cond_data['logic'] == 'all') {
					$cond_string = 'if ('.count($cond_data['published_by']).
								   ' >= '.count($can_publish_cond).') { $complete = true; }';
				} else {
					$cond_string = 'if ('.count($cond_data['published_by']).
								   ' '.$cond_data['logic'].') { $complete = true; }';
				}
				eval($cond_string);

			}//end else

			if (!$complete) {
				$can_publish = array_merge($can_publish, array_diff($can_publish_cond, array_keys($cond_data['published_by'])));
			}
		}//end for

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
		$workflow['current_step'] = Array();

		$current_step_found = FALSE;
		$step_array =& $workflow['steps'];

		while (!$current_step_found) {
			$current_step_found = TRUE;
			for (reset($step_array); NULL !== ($stepid = key($step_array)); next($step_array)) {
				$step_data =& $step_array[$stepid];

				// 'load' the current step

				$step_keys = array_keys($workflow['current_step']);
				$last_key = end($step_keys);
				$workflow['current_step'][$last_key] = $stepid;

				if ($step_data['expired']) {
					if ($step_data['completed']) {

						// expired and complete means that it expired, but was completed by virtue of its escalation workflow being completed
						$current_step_found = FALSE;
						continue;
					} else {
						// we need to move to this asset's substeps if there are any, otherwise to the sibling.
						$step_array =& $step_data['escalation_steps'];

						// add an element to the current step array cos we're going deeper.
						$workflow['current_step'][] = $stepid;
						$current_step_found = FALSE;

						// make the new asset the step array and keep running
						continue(2);
					}
				}

				$current_step_found = TRUE;
				$completed_conds = 0;

				for (reset($step_data['conditions']);
						NULL !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
					$current_step_found = TRUE;
					$cond_data =& $step_data['conditions'][$pub];

					// safety code
					if (!isset($cond_data['complete'])) {
						$cond_data['complete'] = FALSE;
					}
					if (!isset($cond_data['published_by'])) {
						$cond_data['published_by'] = Array();
					}

					if (empty($cond_data['published_by'])) {
						// nobody has approved this, so it cant be finished
						if ($step_data['logic'] == 'all') {
							$current_step_found = TRUE;
							break(2);
						} else {
							continue;
						}
					}
					$complete = FALSE;

					if ($cond_data['complete']) {
						$current_step_found = FALSE;
						$complete = TRUE;
					} else {

						$current_step_found = TRUE;
						if ($cond_data['logic'] == 'all') {
							// lets work out what number 'ALL' represents
							$can_publish_cond = Array();
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
							if ($asset->id) {
								if ($asset instanceof User) {
									$can_publish_cond[] = $asset->id;
								} else if ($asset instanceof User_Group) {
									$can_publish_cond = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE));
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
						$cond_data['complete'] = TRUE;
						$current_step_found = FALSE;
						$completed_conds++;
						if ($step_data['logic'] == 'all') continue;

						$step_completed = FALSE;
						$logic_string = 'if ('.$completed_conds.' '.$step_data['logic'].' ) { $step_completed = true; }';
						eval($logic_string);

						if ($step_completed) {
							// this step is completed so move to the next one
							break;
						} else {
							// step is not finished yet so move to the next condition
							$current_step_found = TRUE;
							continue;
						}
					}
				}//end for

				// is the current step complete? If not, we are up to this step and can return

				if ($step_data['logic'] == 'all' && $completed_conds < count($step_data['conditions'])) {
					$current_step_found = TRUE;
				}
				if ($current_step_found) break;
			}//end for


			// we've been through all the siblings at the current level, and they're all complete.
			$step_keys = array_keys($workflow['current_step']);
			$last_key = end($step_keys);
			if ($workflow['current_step'][$last_key] == count($step_array) && $current_step_found == FALSE) {

				// if we're at the top level of workflow, we can't unescalate, we're cooked
				if (count($workflow['current_step']) == 1) {
					$workflow['current_step'] = Array();
					$current_step_found = TRUE;
					continue;
				}

				// get the index of the current step at this level. We should be at the bottom level, so take the last token from current_step.

				$completed_step_id = array_pop($workflow['current_step']);

				$step_array[$completed_step_id]['completed'] = time();


				// ideally we don't want to set complete and start times for steps in here, but we
				// need to indicate somehow that we've just checked out the escalation workflow and it's all done
				$step_array =& $this->getCurrentStepArray($workflow);
				$current_step_id = end($workflow['current_step']);
				$step_array[$current_step_id]['completed'] = time();

			}
		}//end while !current step found

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
		$asset   = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0160', E_USER_WARNING, $assetid);
			return FALSE;
		}

		// if there are no current workflows running, how can someone publish (you do the math)?
		$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);
		if (empty($running_schemas)) return FALSE;
		$schema_workflows = $this->getSchemaWorkflows($assetid);

		// we'll need these for later to check if any workflow has changed steps
		if (!$this->silentWorkflowParty($asset->id)) {
			$schema_publishers_before = Array();
			$steps_before = $this->getWorkflowCurrentSteps($asset->id);
			foreach ($running_schemas as $schemaid) {
				$schema_publishers_before[$schemaid] = $this->whoCanPublish($asset->id, $schemaid);
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for (reset($schema_workflows); NULL !== ($schemaid = key($schema_workflows)); next($schema_workflows)) {
			if (!in_array($schemaid, $running_schemas)) continue;
			$workflow =& $schema_workflows[$schemaid];
			if ($workflow['complete'] == TRUE) continue;
			if ($this->_recordPublish($workflow, $publisher, $assetid)) {
				$new_workflow = serialize($workflow);

				try {
					$bind_vars = Array(
									'wflow'		=> $new_workflow,
									'assetid'	=> $assetid,
									'schemaid'	=> $schemaid,
								 );
					MatrixDAL::executeQuery('core', 'updateWorkflow', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to update workflow table for asset: '.$assetid.' due to database error: '.$e->getMessage());
				}

				// updated cached version of this workflow
				$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflow;
				if (isset($this->_tmp['schema_workflows'][$assetid]['all'][$schemaid])) {
					$this->_tmp['schema_workflows'][$assetid]['all'][$schemaid] = $workflow;
				}
			}
		}

		// send an internal message to let people know someone has approved
		$asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');
		$user       = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$ms         = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		// workflow logs since the start of this workflow
		$log_message = '';
		$comments = $ms->getMessages(0, 'asset.workflow.userlog', Array(), Array(), $workflow['started'], NULL, 'name', Array('assetid' => $asset->id));
		foreach ($comments as $comment) {
			 $log_message  .= '"'.$comment['body'].'"'.', by '.$comment['from_name'].', '.ts_iso8601($comment['sent'])."\n";
		}

		$log = $ms->newMessage();
		$msg_reps = Array(
						'workflow_user'	=> $user->name,
						'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
						'asset_name'	=> $asset->name,
						'assetid'		=> $asset->id,
						'workflow_url'	=> current_url().$asset->getBackendHref('workflow', FALSE),
						'asset_url'		=> $asset->getDependantParentsURL(),
						'asset_version'	=> $asset->version,
						'log_message'	=> $log_message,
					);
		$log->replacements = $msg_reps;
		$log->type = 'asset.workflow.log.approve';
		$log->parameters['assetid'] = $asset->id;
		$log->parameters['version'] = substr($asset->version, 0, strrpos($asset->version, '.'));
		$ms->enqueueMessage($log);

		if (!$this->silentWorkflowParty($asset->id)) {
			// send internal messages to everyone who could approve before to let them know that someone did
			$base_msg = $ms->newMessage();
			$base_msg->type    = 'asset.workflow.announce.approve';
			$base_msg->parameters['assetid'] = $asset->id;
			$base_msg->replacements = $msg_reps;

			// Send custom messages to anything that the publisher is in the workflow of
			$generic_publishers_before = Array();
			foreach ($schema_publishers_before as $schemaid => $publishers_step) {
				$workflow =& $schema_workflows[$schemaid];
				$current_step = $this->getCurrentStep($workflow, $steps_before[$schemaid]);
				if (in_array($publisher, $publishers_step)) {
					// Does this step have a custom message?
					if (isset($current_step['message_approve']) && trim($current_step['message_approve']) !== '' ) {
						$msg = clone $base_msg;
						$msg->to = $publishers_step;
						// see if we have the 'from' field set
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
						if ($schema->attr('schema_reply_to_email_address') != '') {
							$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
						} else {
							$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
						}
						if ($schema->attr('schema_from_email_address') != '') {
							$msg->from = $schema->attr('schema_from_email_address');
						}

						$subject = trim(array_get_index($current_step, 'subject_approve', 'Asset Changes Approved'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Changes Approved';
						$msg->body = trim($current_step['message_approve']);
						$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
						$ms->enqueueMessage($msg);
					} else if (isset($current_step['subject_approve']) && trim($current_step['subject_approve']) !== '') {
						$msg = clone $base_msg;
						$msg->to = $publishers_step;
						// see if we have the 'from' field set
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
						if ($schema->attr('schema_reply_to_email_address') != '') {
							$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
						} else {
							$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
						}
						if ($schema->attr('schema_from_email_address') != '') {
							$msg->from = $schema->attr('schema_from_email_address');
						}

						$subject = trim(array_get_index($current_step, 'subject_approve', 'Asset Changes Approved'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Changes Approved';
						$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
						$ms->enqueueMessage($msg);
					} else {
						$generic_publishers_before = array_merge($generic_publishers_before, $publishers_step);
					}
				} else {
					$generic_publishers_before = array_merge($generic_publishers_before, $publishers_step);
				}
			}

			// Any other current publishers where the publisher is out of the workflow,
			// or was in the workflow but there is no custom message?
			// Send a generic workflow approval message  in this case.
			if (count($generic_publishers_before) > 0) {
				$msg = clone $base_msg;
				$msg->to = $generic_publishers_before;

				$ms->enqueueMessage($msg);
			}

			// fire the 'Workflow Approval' event
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_workflow_approval', $asset);
		}

		// now check if any workflows have progressed to the next step
		// and send messages to the new approvers if they have
		if (!$this->silentWorkflowParty($asset->id)) {
			$steps_after = $this->getWorkflowCurrentSteps($asset->id);
			foreach ($steps_after as $schemaid => $step) {
				// Are there actually no steps left?
				if (empty($step)) continue;

				if (!($step == $steps_before[$schemaid])) {
					$publishers = $this->whoCanPublish($asset->id, $schemaid);

					// Bug fix #2636, add the slash to the URL in case the trigger fire this chain of action.
					$workflow_backend_href = $asset->getBackendHref('workflow', FALSE);
					if (!empty($workflow_backend_href) && substr($workflow_backend_href, 0, 2) != './') {
						$workflow_backend_href  = '/'.$workflow_backend_href;
					}//end if

					$asset_edt_fns = $asset->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$preview_backend_href = $asset->getBackendHref('preview', FALSE);
						if (!empty($preview_backend_href) && substr($preview_backend_href, 0, 2) != './') {
							$preview_backend_href   = '/'.$preview_backend_href;
						}//end if
						$preview_url = current_url().$preview_backend_href;
					} else {
						$details_backend_href = $asset->getBackendHref('preview', FALSE);
						if (!empty($details_backend_href) && substr($details_backend_href, 0, 2) != './') {
							$details_backend_href   = '/'.$details_backend_href;
						}//end if
						$preview_url = current_url().$details_backend_href;
					}

					if ($asset->status == SQ_STATUS_LIVE_APPROVAL){
						$msg = $ms->newMessage($publishers);

						$workflow = $schema_workflows[$schemaid];
						$current_step = $this->getCurrentStep($workflow, $step);

						$msg_reps = Array(
										'workflow_user'	=> $user->name,
										'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
										'asset_name'	=> $asset->name,
										'assetid'		=> $asset->id,
										'workflow_url'	=> current_url().$workflow_backend_href,
										'preview_url'   => $preview_url,
									);

						$msg->type = 'asset.workflow.review';
						$msg->parameters['assetid'] = $asset->id;
						// see if we have the 'from' field set
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
						if ($schema->attr('schema_reply_to_email_address') != '') {
							$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
						} else {
							$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
						}
						if ($schema->attr('schema_from_email_address') != '') {
							$msg->from = $schema->attr('schema_from_email_address');
						}

						// Does this step have a custom message (for inviting users to the next step)?
						if (isset($current_step['message_review_invitation']) && trim($current_step['message_review_invitation']) !== '' ) {
							$msg->body = trim($current_step['message_review_invitation']);
						}

						// Does this step have a custom subject (for inviting users to the next step)?
						if (isset($current_step['subject_review_invitation']) && trim($current_step['subject_review_invitation']) !== '' ) {
							$subject = trim(array_get_index($current_step, 'subject_review_invitation', 'Asset Up For Review'));
							$msg->subject = (!empty($subject)) ? $subject : 'Asset Up For Review';
						}

						$msg->replacements = $msg_reps;
						$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
						$ms->enqueueMessage($msg);
					} else {
						// send internal messages to everyone in the next step who can now publish the asset
						$msg = $ms->newMessage($publishers);

						$workflow =& $schema_workflows[$schemaid];
						$previous_step =& $this->getCurrentStep($workflow, $steps_before[$schemaid]);
						$previous_step_name =& $previous_step['step_name'];
						$current_step =& $this->getCurrentStep($workflow, $step);
						$current_step_name =& $current_step['step_name'];

						$accept_url = $workflow_backend_href.'&asset_version='.$asset->version.'&workflow_link_action=approve';
						$reject_url = $workflow_backend_href.'&asset_version='.$asset->version.'&workflow_link_action=reject';

						$msg_reps = Array(
										'workflow_user'			=> $user->name,
										'type_code'				=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
										'asset_name'			=> $asset->name,
										'assetid'				=> $asset->id,
										'previous_step_id'		=> implode('.', $steps_before[$schemaid]),
										'previous_step_name'	=> $previous_step_name,
										'current_step_id'		=> implode('.', $step),
										'current_step_name'		=> $current_step_name,
										'workflow_url'			=> current_url().$workflow_backend_href,
										'accept_url'			=> current_url().$accept_url,
										'reject_url'			=> current_url().$reject_url,
										'asset_url'				=> $asset->getDependantParentsURL(),
										'asset_version'			=> $asset->version,
										'log_message'			=> $log_message,
										'preview_url'   		=> $preview_url,
									);

						$msg->type    = 'asset.workflow.invitation.progress';
						$msg->parameters['assetid'] = $asset->id;
						$msg->replacements = $msg_reps;
						// see if we have the 'from' field set
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
						if ($schema->attr('schema_reply_to_email_address') != '') {
							$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
						} else {
							$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
						}
						if ($schema->attr('schema_from_email_address') != '') {
							$msg->from = $schema->attr('schema_from_email_address');
						}

						// Does this step have a custom message?
						if (isset($current_step['message_invitation']) && trim($current_step['message_invitation']) !== '' ) {
							$msg->body = trim($current_step['message_invitation']);
						}//end if

						// Does this step have a custom subject?
						if (isset($current_step['subject_invitation']) && trim($current_step['subject_invitation']) !== '' ) {
							$subject = trim(array_get_index($current_step, 'subject_invitation', 'Workflow Approval Required'));
							$msg->subject = (!empty($subject)) ? $subject : 'Workflow Approval Required';
						}//end if

						$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
						$ms->enqueueMessage($msg);
					}//end else

				}//end if step has advanced
			}//end foreach $steps_after

		}//end if not silent workflow party

		// if the user who has completed the step on this workflow is also able to
		// approve the next part of the workflow, let their action to approve workflow
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$list_of_publishers = $this->whoCanPublish($asset->id);
		// Test they are the next user in workflow
		if (in_array($current_userid, $list_of_publishers)) {
			if (!$this->recordPublish($asset->id, $current_userid)) {
				trigger_localised_error('SYS0078', E_USER_WARNING, $current_userid);
				// dont die here because they can try to approve again later, we just want to warn them
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end recordPublish()


	/**
	* Pretend this user has approved the workflow to see if the workflow process will be completed
	*
	* Use this to find out if the user is allowed to live edit.
	*
	* If workflow has not started, it will test whether a user can approve a
	* workflow simply by starting it. In this case, usually workflow can be
	* bypassed completely (notwithstanding other roadblocks, such as incomplete
	* metadata).
	*
	* If a workflow schema contains multiple streams, it can test either a single
	* stream or ALL of them (again, presuming workflow has not started). By
	* default, it will test the default stream. If all streams are tested, it
	* will only return TRUE if ALL of them are able to be completed.
	*
	* @param int	$assetid		the ID of the asset to set the approval for
	* @param int	$publisher		the ID of the user who we are testing approval for
	* @param string	$stream_name	the name of the stream to test. NULL to test ALL streams.
	*								'' to test default stream. Only applicable when workflow has not started.
	*
	* @return boolean
	* @access public
	*/
	function testPublish($assetid, $publisher, $stream_name='')
	{
		$schema_workflows = $this->getSchemaWorkflows($assetid);
		$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);

		if (empty($running_schemas)) {
			// we dont actually have any running workflows, so lets pretend we do
			$schemas = $this->getSchemas($assetid, TRUE);
			$streamids = Array();

			// foreach of our schemas check to make sure they are still on system
			// see bug 5678 enhancement: better error handling for non exist workflow schema
			foreach ($schemas as $index => $schema) {
				if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($schema)) {
					trigger_error("Workflow Schema Id #$schema is applied to Asset Id #$assetid but cannot be found in system anymore.", E_USER_WARNING);
					unset($schemas[$index]);
				}
			}
			if ($stream_name === NULL) {
				// ALL Streams
				foreach ($schemas as $schemaid) {
					$streamids[$schemaid] = $this->getStreams($schemaid);
				}
			} else if ($stream_name === '') {
				// Default Stream
				$default_streamids = $this->getDefaultStream($schemas);
				// Make it an array to fit it with the all-schemas option
				// (name doesn't even matter)
				foreach ($default_streamids as $schemaid => $streamid) {
					$streamids[$schemaid] = Array($streamid => 'Default Stream');
				}
			} else {
				// Named Stream
				$named_streamids = $this->getStreamByName($schemas, $stream_name);
				// Make it an array to fit it with the all-schemas option
				// (name doesn't even matter)
				foreach ($named_streamids as $schemaid => $streamid) {
					$streamids[$schemaid] = Array($streamid => $stream_name);
				}
			}

			foreach ($schemas as $schemaid) {
				foreach ($streamids[$schemaid] as $streamid => &$stream_name) {
					$schema_workflows[$schemaid][$streamid] = $this->generateWorkflowArray($assetid, $streamid);
				}
				$running_schemas[] = $schemaid;
			}
		} else {
			// Keep the schema workflow format consistent
			foreach($schema_workflows as $schemaid => &$schema_workflow) {
				// Handle pre-3.26 schema without a stream ID
				$streamid = array_get_index($schema_workflow, 'stream_assetid', 0);
				$schema_workflow = Array($streamid => $schema_workflow);
			}
		}

		for (reset($schema_workflows); NULL !== ($schemaid = key($schema_workflows)); next($schema_workflows)) {
			if (!in_array($schemaid, $running_schemas)) continue;

			foreach ($schema_workflows[$schemaid] as $streamid => &$workflow) {
				// already finished the workflow?
				if ($workflow['complete']) continue;

				// Try to push workflow forward until we can't publish no more -
				// either our effort is not enough, or we've completed it
				while (!$workflow['complete']) {
					if (!$this->_recordPublish($workflow, $publisher, $assetid)) {
						break;
					}
				}

				// We haven't finished the workflow?
				if (!$workflow['complete']) return FALSE;
			}
		}

		return TRUE;

	}//end testPublish()


	/**
	* Changes the passed workflow to indicate that a user has approved the asset
	*
	* Returns TRUE if the publisher was able to publish and thus that the passed workflow
	* has actually changed - FALSE otherwise. This code is abstracted from RecordPublish so
	* it can be used by testPublish
	*
	* @param array	&$workflow		the workflow add approval to
	* @param int	$publisherid	the ID of the user who has approved
	* @param int	$assetid		the ID of the asset being published
	*
	* @return boolean
	* @access public
	*/
	function _recordPublish(&$workflow, $publisherid, $assetid)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$workflow_updated = FALSE;
		// safety code for blank workflows
		if (empty($workflow['steps'])) {
			$workflow_updated = TRUE;
			$workflow['complete'] = TRUE;
			$workflow['steps'] = Array();
		} else {
			$step_data =& $this->getCurrentStep($workflow);
			for (reset($step_data['conditions']);
					NULL !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {

				$cond_data =& $step_data['conditions'][$pub];
				// work out who can publish this condition
				$can_publish = Array();

				if (!$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($pub, Array('user', 'user_group'), FALSE)) {
					continue;
				}
				$publisher = $GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
				if ($publisher->id) {
					$asset_class = get_class($publisher);

					// Note that the order of IF statements is important, because roles are
					// a descendent of groups but should be treated differently
					if ($am->isTypeDecendant($asset_class, 'user')) {
						$can_publish[] = $publisher->id;
					} else if ($am->isTypeDecendant($asset_class, 'role')) {
						if (SQ_CONF_ENABLE_ROLES_WF_SYSTEM == '1') {
							$fetch_global_roles = (SQ_CONF_ENABLE_GLOBAL_ROLES == '1');
							$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($assetid, $publisher->id, NULL, FALSE, $fetch_global_roles, TRUE);
							foreach ($roles as $roleid => $userids) {
								foreach ($userids as $userid) {
									$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), Array(), FALSE, 'type_code');
									if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($info[$userid], 'user_group')) {
										$can_publish = array_merge($can_publish, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($userid, Array('user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE)));
									} else {
										$can_publish[] = $userid;
									}
								}//end foreach
							}//end foreach
						}//end if
					} else if ($am->isTypeDecendant($asset_class, 'user_group')) {
						$can_publish = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($publisher->id, Array('user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE));
					}
				}

				if (in_array($publisherid, $can_publish)) {
					if (!isset($cond_data['published_by'])) {
						$cond_data['published_by'] = Array();
					}
					// if the publisherid has not already published
					if (!isset($cond_data['published_by'][$publisherid])) {
						$cond_data['published_by'][$publisherid] = time();
						$workflow_updated = TRUE;
					}

				}
			}//end for

			if ($workflow_updated) {
				// load the current step into the workflow
				$this->_loadCurrentStep($workflow);

				// if the current step returns nothing, workflow is complete
				if ($workflow['current_step'] === Array()) {
					$workflow['complete'] = TRUE;
					// mark the last step as completed
					$num_steps = count($workflow['steps']);
					if ($num_steps > 0) {
						$workflow['steps'][$num_steps]['completed'] = time();
					}
				// else start the next workflow step
				} else {
					$current_step =& $this->getCurrentStep($workflow);
					if (!$current_step['started']) {
						$current_step['started'] = time();
					}
					// if this is not the first step in this workflow,
					$step_keys = array_keys($workflow['current_step']);
					$last_key = end($step_keys);
					if ($workflow['current_step'][$last_key] > 1) {
						$previous_step_address = $workflow['current_step'];
						$previous_step_address[$last_key]--;
						$previous_step =& $this->getCurrentStep($workflow, $previous_step_address);
						if (!$previous_step['completed']) {
							$previous_step['completed'] = time();
						}
					}
				}//end if
			}
		}//end else

		return $workflow_updated;

	}//end _recordPublish()


	/**
	* Generate an array representing the workflow structure for a STREAM at this point in time
	*
	* The generated array is a snap shot of the schema at a point in time that can get written
	* to the database and used for publishing etc.
	*
	* @param int	$assetid	the ID of the asset to generate the file for
	* @param int	$streamid	the ID of the stream
	*
	* @return boolean
	* @access public
	*/
	function generateWorkflowArray($assetid, $streamid)
	{
		$assetid  = (int) $assetid;
		$streamid = (int) $streamid;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0155', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$stream = $GLOBALS['SQ_SYSTEM']->am->getAsset($streamid);
		if (is_null($stream)) {
			trigger_localised_error('SYS0156', E_USER_WARNING, $streamid);
			return FALSE;
		}

		if (!($stream instanceof Workflow_Stream)) {
			trigger_localised_error('SYS0154', E_USER_WARNING, $stream->name, $streamid);
			return FALSE;
		}

		// check that the schema we are generating an array for is actually set
		$asset_schemas = $this->getSchemas($assetid, TRUE);
		$stream_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($streamid, 'workflow_schema', TRUE);

		if (count(array_intersect($asset_schemas, $stream_parents)) === 0) {
			return Array();
		}

		$edit_fns = $stream->getEditFns();
		return $edit_fns->generateWorkflowArray($stream);

	}//end generateWorkflowArray()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int		$assetid		the ID of the asset to start workflow for
	* @param boolean	$auto_approve	register that the current user has approed the workflow	once it has been started
	* @param object		$msg			an internal message to be sent instead of the default
	*
	* @return boolean
	* @access public
	*/
	function startWorkflow($assetid, $auto_approve=TRUE, $base_msg=NULL, $stream_name=NULL)
	{
		$assetid  = (int) $assetid;
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0179', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$schemas = $this->getSchemas($assetid, TRUE, FALSE);

		if (empty($schemas)) return FALSE;
		if ($stream_name === NULL) {
			$stream_name = $this->getStartingStream($assetid);
			$this->setStartingStream($assetid, NULL);
		}

		if ($stream_name === NULL) {
			$streams = $this->getDefaultStream($schemas);
			$stream_name = 'Default Stream';
		} else {
			$streams = $this->getStreamByName($schemas, $stream_name);
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$workflow_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($schemas);

		foreach ($schemas as $schemaid) {
			// Grab the workflow array from the appropriate stream, and then
			// make that our workflow for this
			$workflow = $this->generateWorkflowArray($assetid, $streams[$schemaid]);
			$workflow['schema_name'] = $workflow_names[$schemaid]['name'];

			$stream_names = $this->getStreams($schemaid);
			$workflow['stream_assetid'] = $streams[$schemaid];
			$workflow['stream_name'] = $stream_names[$streams[$schemaid]];
			if (empty($workflow['steps'])) continue;
			$new_workflow = serialize($workflow);

			$sql = 'UPDATE
						'.SQ_TABLE_RUNNING_PREFIX.'ast_wflow
					SET
						wflow = :wflow,
						last_started_by = :last_started_by,
						last_stream = :last_stream
					WHERE
						assetid	= :assetid
						AND schemaid = :schemaid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				$bind_vars = Array(
								'wflow'				=> $new_workflow,
								'assetid'			=> $assetid,
								'schemaid'			=> $schemaid,
								'last_started_by' 	=> $workflow['started_by'],
								'last_stream'	=> $workflow['stream_assetid'],
							 );
				foreach ($bind_vars as $bind_name => $bind_value){
					MatrixDAL::bindValueToPdo($query, $bind_name, $bind_value);
				}
				MatrixDAL::execPdoQuery($query);
			} catch (DALException $e) {
				throw new Exception('Unable to update workflow table for asset: '.$assetid.' due to database error: '.$e->getMessage());
			}

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
				} else {
					// bug fix #3807 Workflow email send to second reviewer contains wrong content and link
					// we we have sucessfully completed recordPublish() call, then the email to all the
					// approvers in the following step condition has already been safe. It will be safe
					// to return from here.
					return TRUE;
				}
			}
		}

		if (!$this->silentWorkflowParty($asset->id)) {
			// send internal messages to everyone who can now publish the asset
			$generic_msg = NULL;

			$ms   = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$schema_workflows = $this->getSchemaWorkflows($asset->id);
			foreach ($schema_workflows as $schemaid => $schema_workflow) {
				$current_step = $this->getCurrentStep($schema_workflow);
				$publishers = $this->whoCanPublish($asset->id, $schemaid);

				// we don't want to be sending asset.workflow.invitation emails to users who have admin
				// permissions on the asset in workflow, see expandUsersTo() in internal_message.inc
				if (is_null($base_msg)) {
					if (empty($publishers)) {
						continue;
					} else {
						// create a new internal message if we have not been supplied with one
						$msg = $ms->newMessage();
					}
				} else {
					// Clone the base internal message
					$msg = clone $base_msg;
					if (empty($msg->to) && empty($msg->type) && empty($publishers)) {
						continue;
					}
				}

				// see if we have the 'from' field set
				$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
				if ($schema->attr('schema_reply_to_email_address') != '') {
					$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
				} else {
					$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
				}
				if ($schema->attr('schema_from_email_address') != '') {
					$msg->from = $schema->attr('schema_from_email_address');
				}

				// added the publishers to the list of people that are to be notified
				$msg->to = array_merge($msg->to, $publishers);
				// complete any missing internal message fields that have not been supplied
				if (empty($msg->replacements)) {
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
					// Bug fix #2636, add the slash to the URL in case the trigger fire this chain of action.
					$workflow_backend_href = $asset->getBackendHref('workflow', FALSE);
					if (!empty($workflow_backend_href) && substr($workflow_backend_href, 0, 2) != './') {
						$workflow_backend_href  = '/'.$workflow_backend_href;
					}//end if

					$accept_url = $workflow_backend_href.'&asset_version='.$asset->version.'&workflow_link_action=approve';
					$reject_url = $workflow_backend_href.'&asset_version='.$asset->version.'&workflow_link_action=reject';


					$msg_reps = Array(
									'workflow_user'			=> $user->name,
									'type_code'				=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
									'asset_name'			=> $asset->name,
									'assetid'				=> $asset->id,
									'workflow_url'			=> current_url().$workflow_backend_href,
									'accept_url'			=> current_url().$accept_url,
									'reject_url'			=> current_url().$reject_url,
									'previous_step_id'		=> '',
									'previous_step_name'	=> '',
									'current_step_id'		=> implode('.', $schema_workflow['current_step']),
									'current_step_name'		=> $current_step['step_name'],
									'stream'				=> $schema_workflow['stream_name'],
									'asset_url'				=> $asset->getDependantParentsURL(),
									'asset_version'			=> $asset->version,
									'log_message'			=> '',
								);
					$asset_edt_fns = $asset->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$preview_backend_href = $asset->getBackendHref('preview', FALSE);
						if (!empty($preview_backend_href) && substr($preview_backend_href, 0, 2) != './') {
							$preview_backend_href   = '/'.$preview_backend_href;
						}//end if
						$msg_reps['preview_url'] = current_url().$preview_backend_href;
					} else {
						$details_backend_href = $asset->getBackendHref('preview', FALSE);
						if (!empty($details_backend_href) && substr($details_backend_href, 0, 2) != './') {
							$details_backend_href   = '/'.$details_backend_href;
						}//end if
						$msg_reps['preview_url'] = current_url().$details_backend_href;
					}
					$msg->replacements = $msg_reps;

					// Does this step have a custom message?
					if (isset($current_step['message_invitation']) && trim($current_step['message_invitation']) !== '' ) {
						// This message has a custom body. Just send it.
						$subject = trim(array_get_index($current_step, 'subject_invitation', 'Workflow Approval Required'));
						$msg->subject = (!empty($subject)) ? $subject : 'Workflow Approval Required';
						$msg->body = trim($current_step['message_invitation']);
					} else if (isset($current_step['subject_invitation']) && trim($current_step['subject_invitation']) !== '') {
						// This message has a custom subject. Just send it.
						$subject = trim(array_get_index($current_step, 'subject_invitation', 'Workflow Approval Required'));
						$msg->subject = (!empty($subject)) ? $subject : 'Workflow Approval Required';
					} else {
						// This is the generic message. Make sure only one of these are sent out,
						// regardless of how many schemas trigger it.
						if ($generic_msg === NULL) {
							$generic_msg = $msg;
						} else {
							$generic_msg->to = array_merge($generic_msg->to, $msg->to);
							continue;
						}
					}

				}//end if
				if (empty($msg->type)) {
					$msg->type = 'asset.workflow.invitation';
				} else if ($msg->type == 'asset.workflow.review'){
					// Does this step have a custom message?
					if (isset($current_step['message_review_invitation']) && trim($current_step['message_review_invitation']) !== '' ) {
						$msg->body = trim($current_step['message_review_invitation']);
					}//end if

					// Does this step have a custom subject?
					if (isset($current_step['subject_review_invitation']) && trim($current_step['subject_review_invitation']) !== '' ) {
						$subject = trim(array_get_index($current_step, 'subject_review_invitation', 'Asset Up For Review'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Up For Review';
					}//end if
				}

				$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);

				$msg->parameters['assetid'] = $asset->id;
				$ms->enqueueMessage($msg);
			}//end foreach
		}//end if

		return TRUE;

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
		$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);
		if (empty($running_schemas)) return TRUE;
		$schema_workflows = $this->getSchemaWorkflows($assetid);

		foreach ($schema_workflows as $schemaid => $workflow) {
			if (!in_array($schemaid, $running_schemas)) continue;
			if (empty($workflow['steps'])) continue;
			if (!$workflow['complete']) return FALSE;
		}

		return TRUE;

	}//end isWorkflowComplete()


	/**
	* Get the currently running steps of the workflow array, keyed by schema ID
	*
	* @param int	$assetid	the ID of the asset to check workflow completness for
	*
	* @return array
	* @access public
	*/
	function getWorkflowCurrentSteps($assetid)
	{
		$running_schemas = $this->getSchemas($assetid, TRUE, TRUE);
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
	* @param int	$assetid	the ID of the asset to cancel workflow for
	* @param object	$msg		an internal message to be sent instead of the default
	*
	* @return boolean
	* @access public
	*/
	function cancelWorkflow($assetid, $base_msg=NULL)
	{
		$assetid = (int) $assetid;
		$asset   = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0146', E_USER_WARNING, $assetid);
			return FALSE;
		}

		// we have to get the publishers before the workflow is cleared, because
		// this function depends on having a workflow running
		$publishers = $this->whoCanPublish($asset->id);
		$workflow_values = $this->getSchemaWorkflows($assetid);
		$workflow = reset($workflow_values);
		$started_by = isset($workflow['started_by']) ? $workflow['started_by'] : '';
		if ($started_by && $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($started_by, Array('user', 'user_group'), FALSE)) {
			$started_by_user = $GLOBALS['SQ_SYSTEM']->am->getAsset($started_by, '', TRUE);
			if ($started_by_user){
				// see bug #4986 Simple Edit User doesnt get workflow emails initiated via EES
				// and see simple_edit_user::canAccessBackend()
				if ($started_by_user instanceof Simple_Edit_User) $started_by_user->_tmp['starter_of_workflow'] = TRUE;
			}
			$publishers = array_unique(array_merge($publishers, Array($started_by)));
		}

		$current_steps = $this->getWorkflowCurrentSteps($assetid);
		if (!$this->_clearWorkflow($assetid)) return FALSE;

		// send internal messages to everyone who could approve before to let them know that someone cancelled
		if (!$this->silentWorkflowParty($asset->id)) {
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

			// fire the 'Workflow Rejection' event
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_workflow_rejection', $asset);
			$generic_msg = NULL;

			// workflow logs since the start of this workflow
			$log_message = '';
			$comments = $ms->getMessages(0, 'asset.workflow.userlog', Array(), Array(), $workflow['started'], NULL, 'name', Array('assetid' => $asset->id));
			foreach ($comments as $comment) {
				 $log_message  .= '"'.$comment['body'].'"'.', by '.$comment['from_name'].', '.ts_iso8601($comment['sent'])."\n";
			}

			foreach ($current_steps as $schemaid => $current_step_num) {
				$current_step = $this->getCurrentStep($workflow_values[$schemaid], $current_step_num);

				// we don't want to be sending asset.workflow.announce.reject emails to users who have admin
				// permissions on the asset in workflow, see expandUsersTo() in internal_message.inc
				if (is_null($base_msg)) {
					if (empty($publishers)) {
						continue;
					} else {
						// create a new internal message if we have not been supplied with one
						$msg = $ms->newMessage();
					}
				} else {
					$msg = clone $base_msg;
					if (empty($msg->to) && empty($msg->type) && empty($publishers)) {
						continue;
					}
				}

				// see if we have the 'from' field set
				$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
				if ($schema->attr('schema_reply_to_email_address') != '') {
					$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
				} else {
					$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
				}
				if ($schema->attr('schema_from_email_address') != '') {
					$msg->from = $schema->attr('schema_from_email_address');
				}

				// added the publishers to the list of people that are to be notified
				// (note: the publishers get emails from each workflow - they should be
				// combined into one at the Messaging Service end)
				$msg->to = array_merge($msg->to, $publishers);

				// complete any missing internal message fields that have not been supplied
				if (empty($msg->replacements)) {
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
					$msg_reps = Array(
									'workflow_user'	=> $user->name,
									'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
									'asset_name'	=> $asset->name,
									'assetid'		=> $asset->id,
									'workflow_url'	=> current_url().$asset->getBackendHref('workflow', FALSE),
									'asset_url'		=> $asset->getDependantParentsURL(),
									'asset_version'	=> $asset->version,
									'log_message'	=> $log_message,
								);
					$asset_edt_fns = $asset->getEditFns();
					if (isset($asset_edt_fns->static_screens['preview'])) {
						$msg_reps['preview_url'] = current_url().$asset->getBackendHref('preview', FALSE);
					} else {
						$msg_reps['preview_url'] = current_url().$asset->getBackendHref('details', FALSE);
					}
					$msg->replacements = $msg_reps;

					// Does this step have a custom message?
					if (isset($current_step['message_reject']) && trim($current_step['message_reject']) !== '' ) {
						$subject = trim(array_get_index($current_step, 'subject_reject', 'Asset Changes Rejected'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Changes Rejected';
						$msg->body = trim($current_step['message_reject']);
					} else if (isset($current_step['subject_reject']) && trim($current_step['subject_reject']) !== '') {
						$subject = trim(array_get_index($current_step, 'subject_reject', 'Asset Changes Rejected'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Changes Rejected';
					} else {
						// This is the generic message. Make sure only one of these are sent out,
						// regardless of how many schemas trigger it.
						if ($generic_msg === NULL) {
							$generic_msg = $msg;
						} else {
							$generic_msg->to = array_merge($generic_msg->to, $msg->to);
							continue;
						}
					}
				}
				if (empty($msg->type)) {
					$msg->type = 'asset.workflow.announce.reject';
				} else if ($msg->type == 'asset.workflow.review.cancel'){
					// Does this step have a custom message?
					if (isset($current_step['message_review_cancel']) && trim($current_step['message_review_cancel']) !== '' ) {
						$subject = trim(array_get_index($current_step, 'subject_review_cancel', 'Asset Review Cancelled'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Review Cancelled';
						$msg->body = trim($current_step['message_review_cancel']);
					} else if (isset($current_step['subject_review_cancel']) && trim($current_step['subject_review_cancel']) !== '') {
						$subject = trim(array_get_index($current_step, 'subject_review_cancel', 'Asset Review Cancelled'));
						$msg->subject = (!empty($subject)) ? $subject : 'Asset Review Cancelled';
					} else {
						// This is the generic message. Make sure only one of these are sent out,
						// regardless of how many schemas trigger it.
						if ($generic_msg === NULL) {
							$generic_msg = $msg;
						} else {
							$generic_msg->to = array_merge($generic_msg->to, $msg->to);
							continue;
						}
					}
				}

				$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
				$msg->parameters['assetid'] = $asset->id;
				$ms->enqueueMessage($msg);

			}//end foreach

		}//end if

		return TRUE;

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
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0150', E_USER_WARNING, $assetid);
			return FALSE;
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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'UPDATE
					sq_ast_wflow
				SET
					wflow = NULL
				WHERE
					assetid	 = :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			MatrixDAL::execPdoQuery($query);
		} catch (DALException $e) {
			throw new Exception ('Unable to clear workflow for asset: '.$assetid.' due to database error: '.$e->getMessage());
		}

		unset($this->_tmp['schemas'][$assetid]);
		unset($this->_tmp['schema_workflows'][$assetid]);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _clearWorkflow()


	/**
	* Deletes any workflow entries that this asset has
	*
	* @param string	$assetid	the asset id
	*
	* @return boolean
	* @access public
	*/
	function purgeWorkflow($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars = Array(
							'assetid'	=> (string)$assetid,
						 );
			MatrixDAL::executeQuery('core', 'purgeWorkflow', $bind_vars);
		} catch (DALException $e) {
			throw new Exception ('Unable to purge workflow schema for "'.$asset->name.'" (#'.$asset->id.') due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

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
	* @return boolean
	* @access public
	*/
	function silentWorkflowParty($assetid)
	{
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((int) $assetid);
		if (is_null($asset)) return TRUE;
		$edit_fns = $asset->getEditFns();
		return !isset($edit_fns->static_screens['workflow']);

	}//end silentWorkflowParty()


	/**
	* Escalates workflow
	*
	* If there are escalation steps, move to them. If not, move to sibling.
	*
	* @param int	$assetid	the ID of the asset who's workflow is being escalated
	* @param int	$schemaid	The ID of the schema being escalated
	*
	* @return boolean
	* @access public
	*/
	function escalateWorkflow($assetid, $schemaid)
	{
		$assetid = (int) $assetid;
		$asset   = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0160', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$schema_workflows = $this->getSchemaWorkflows($assetid);
		$workflow =& $schema_workflows[$schemaid];

		// workflow logs since the start of this workflow
		$log_message = '';
		$comments = $ms->getMessages(0, 'asset.workflow.userlog', Array(), Array(), $workflow['started'], NULL, 'name', Array('assetid' => $asset->id));
		foreach ($comments as $comment) {
			 $log_message  .= '"'.$comment['body'].'"'.', by '.$comment['from_name'].', '.ts_iso8601($comment['sent'])."\n";
		}

		// mark the current step as escalated
		$current_step =& $this->getCurrentStep($workflow);

		// we'll need these for later to check if any workflow has changed steps
		$step_before = $current_step;
		$step_before_id = $workflow['current_step'];
		$publishers_before = $this->whoCanPublish($assetid);


		$current_step['expired'] = TRUE;

		// if there are no escalation steps for the current step, we can't escalate to substeps, so defer to the next step
		if (empty($current_step['escalation_steps'])) {
			$current_step['completed'] = time();
		}

		// load the current step into the workflow
		$this->_loadCurrentStep($workflow);

		if ($workflow['current_step'] == Array()) {
			if ($this->silentWorkflowParty($asset->id)) return TRUE;
			// We can't complete workflow by escalating
			$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'workflow'), FALSE);
			$href = str_replace('./', '', current($hrefs));
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
			$url = 'http://'.current($root_urls).'/'.$href;
			$type = 'asset.workflow.stale';
			$admins = $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, SQ_PERMISSION_ADMIN, TRUE, FALSE, TRUE);



			$msg_reps = Array(
							'asset_name'	=> $asset->attr('name'),
							'assetid'		=> $assetid,
							'step_id'		=> implode('.', $step_before_id),
							'step_name'		=> $current_step['step_name'],
							'schema'		=> $workflow['schema_name'],
							'stream'		=> $workflow['stream_name'],
							'started_time'	=> easy_datetime($current_step['started']),
							'expiry_time'	=> easy_time_total($current_step['expiry_time']),
							'workflow_url'	=> $url,
							'asset_url'		=> $asset->getDependantParentsURL(),
							'asset_version'	=> $asset->version,
							'log_message'	=> $log_message,
						);

			// If there are no admins, send it to system admins instead
			if (empty($admins)) $admins = Array(0);

			$msg = $ms->newMessage($admins, $type, $msg_reps);

			// see if we have the 'from' field set
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
			if ($schema->attr('schema_reply_to_email_address') != '') {
				$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
			} else {
				$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
			}
			if ($schema->attr('schema_from_email_address') != '') {
				$msg->from = $schema->attr('schema_from_email_address');
			}

			// Does this step have a custom message for stale workflow?
			if (isset($current_step['message_stale']) && trim($current_step['message_stale']) !== '' ) {
				$msg->body = trim($current_step['message_stale']);
			}

			// Does this step have a custom subject for stale workflow?
			if (isset($current_step['subject_stale']) && trim($current_step['subject_stale']) !== '' ) {
				$subject = trim(array_get_index($current_step, 'subject_stale', 'Stale Workflow Step'));
				$msg->subject = (!empty($subject)) ? $subject : 'Stale Workflow Step';
			}//end if

			$msg->parameters['assetid'] = $asset->id;
			$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
			$ms->enqueueMessage($msg);

			return TRUE;
		}

		// we may have gone deeper in workflow. If so, mark the current step as started.,
		$current_step =& $this->getCurrentStep($workflow);
		if (!$current_step['started']) {
			$current_step['started'] = time();
		}


		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$new_workflow = serialize($workflow);
		try {
			$bind_vars = Array(
							'wflow'		=> $new_workflow,
							'assetid'	=> $assetid,
							'schemaid'	=> $schemaid,
						 );
			MatrixDAL::executeQuery('core', 'updateWorkflow', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to update workflow table for asset: '.$assetid.' due to database error: '.$e->getMessage());
		}

		$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflow;
		if (isset($this->_tmp['schema_workflows'][$assetid]['all'][$schemaid])) {
			$this->_tmp['schema_workflows'][$assetid]['all'][$schemaid] = $workflow;
		}


		// send an internal message to let people know workflow has been escalated
		$asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');
		$user       = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());

		$log = $ms->newMessage();

		$url_parts = explode('://' , $asset->getURL());

		if (count($url_parts) > 1) {
			$site_url = $GLOBALS['SQ_SYSTEM']->am->getRootURL($url_parts[1]);
			$root_url = $url_parts[0].'://'.$site_url['url'];
		} else {
			// Asset doesn't has url, so ...
			$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'workflow'), FALSE);
			$href = str_replace('./', '', current($hrefs));
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
			$root_url = 'http://'.current($root_urls).'/'.$href;
		}

		$msg_reps = Array(
						'workflow_user'			=> $user->name,
						'type_code'				=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
						'asset_name'			=> $asset->name,
						'workflow_url'			=> $root_url.'/'.$asset->getBackendHref('workflow', FALSE),
						'assetid'				=> $assetid,
						'previous_step_id'		=> implode('.', $step_before_id),
						'previous_step_name'	=> $step_before['step_name'],
						'current_step_id'		=> implode('.', $workflow['current_step']),
						'current_step_name'		=> $current_step['step_name'],
						'schema'				=> $workflow['schema_name'],
						'stream'				=> $workflow['stream_name'],
						'started_time'			=> easy_datetime($step_before['started']),
						'expiry_time'			=> easy_time_total($step_before['expiry_time']),
						'asset_url'				=> $asset->getDependantParentsURL(),
						'asset_version'			=> $asset->version,
						'log_message'			=> $log_message,
					);

		// see if we have the 'from' field set
		$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'workflow_schema');
		if ($schema->attr('schema_reply_to_email_address') != '') {
			$log->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
		} else {
			$log->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}
		if ($schema->attr('schema_from_email_address') != '') {
			$log->from = $schema->attr('schema_from_email_address');
		}

		$log->replacements = $msg_reps;
		$log->type = 'asset.workflow.log.escalated';

		$log->parameters['assetid'] = $asset->id;
		$log->parameters['version'] = substr($asset->version, 0, strrpos($asset->version, '.'));
		$ms->enqueueMessage($log);


		// send internal messages to everyone who could approve before to let them know they have lost their chance

		if (!$this->silentWorkflowParty($asset->id)) {
			$msg = $ms->newMessage($publishers_before);
			// see if we have the 'from' field set
			if ($schema->attr('schema_reply_to_email_address') != '') {
				$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
			} else {
				$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
			}
			if ($schema->attr('schema_from_email_address') != '') {
				$msg->from = $schema->attr('schema_from_email_address');
			}

			$msg->type    = 'asset.workflow.announce.escalated';
			$msg->parameters['assetid'] = $asset->id;
			$msg->replacements = $msg_reps;

			// Does this step have a custom message for escalation?
			if (isset($step_before['message_escalated']) && trim($step_before['message_escalated']) !== '' ) {
				$msg->body = trim($step_before['message_escalated']);
			}

			// Does this step have a custom subject for escalation?
			if (isset($step_before['subject_escalated']) && trim($step_before['subject_escalated']) !== '' ) {
				$subject = trim(array_get_index($step_before, 'subject_escalated', 'Workflow Escalated'));
				$msg->subject = (!empty($subject)) ? $subject : 'Workflow Escalated';
			}

			$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
			$ms->enqueueMessage($msg);

			// send messages to the new approvers if they have

			$publishers = $this->whoCanPublish($asset->id, $schemaid);
			$url_parts = explode('://' , $asset->getURL());
			if (count($url_parts) > 1) {
				$site_url = $GLOBALS['SQ_SYSTEM']->am->getRootURL($url_parts[1]);
				$root_url = $url_parts[0].'://'.$site_url['url'];
			} else {
				// Asset doesn't has url, so ...
				$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => 'workflow'), FALSE);
				$href = str_replace('./', '', current($hrefs));
				$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
				$root_url = 'http://'.current($root_urls).'/'.$href;
			}

			if ($asset->status == SQ_STATUS_LIVE_APPROVAL){
				$msg = $ms->newMessage($publishers);

				$msg_reps = Array(
								'workflow_user'	=> $user->name,
								'type_code'		=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
								'asset_name'	=> $asset->name,
								'assetid'		=> $asset->id,
								'workflow_url'	=> $root_url.'/'.$asset->getBackendHref('workflow', FALSE),
							);

				$asset_edt_fns = $asset->getEditFns();
				if (isset($asset_edt_fns->static_screens['preview'])) {
					$msg_reps['preview_url'] = $root_url.'/'.$asset->getBackendHref('preview', FALSE);
				} else {
					$msg_reps['preview_url'] = $root_url.'/'.$asset->getBackendHref('details', FALSE);
				}
				// see if we have the 'from' field set
				if ($schema->attr('schema_reply_to_email_address') != '') {
					$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
				} else {
					$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
				}
				if ($schema->attr('schema_from_email_address') != '') {
					$msg->from = $schema->attr('schema_from_email_address');
				}

				$msg->type = 'asset.workflow.review';
				$msg->parameters['assetid'] = $asset->id;

				// Does this step have a custom message (for inviting users to the next step)?
				if (isset($current_step['message_review_invitation']) && trim($current_step['message_review_invitation']) !== '' ) {
					$msg->body = trim($current_step['message_review_invitation']);
				}

				// Does this step have a custom subject (for inviting users to the next step)?
				if (isset($current_step['subject_review_invitation']) && trim($current_step['subject_review_invitation']) !== '' ) {
					$subject = trim(array_get_index($current_step, 'subject_review_invitation', 'Asset Up For Review'));
					$msg->subject = (!empty($subject)) ? $subject : 'Asset Up For Review';
				}

				$msg->replacements = $msg_reps;
				$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
				$ms->enqueueMessage($msg);

			} else {
				// send internal messages to everyone in the next step who can now publish the asset
				$msg = $ms->newMessage($publishers);

				$workflow_backend_href = $asset->getBackendHref('workflow', FALSE);
				$accept_url = $workflow_backend_href.'&asset_version='.$asset->version.'&workflow_link_action=approve';
				$reject_url = $workflow_backend_href.'&asset_version='.$asset->version.'&workflow_link_action=reject';

				$msg_reps = Array(
								'workflow_user'			=> $user->name,
								'type_code'				=> $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'),
								'asset_name'			=> $asset->name,
								'assetid'				=> $asset->id,
								'previous_step_id'		=> implode('.',$step_before_id),
								'previous_step_name'	=> $step_before['step_name'],
								'current_step_id'		=> implode('.',$workflow['current_step']),
								'current_step_name'		=> $current_step['step_name'],
								'stream'				=> $workflow['stream_name'],
								'workflow_url'			=> $root_url.'/'.$workflow_backend_href,
								'accept_url'			=> $root_url.'/'.$accept_url,
								'reject_url'			=> $root_url.'/'.$reject_url,
								'asset_url'				=> $asset->getDependantParentsURL(),
								'asset_version'			=> $asset->version,
								'log_message'			=> $log_message,
							);

				$asset_edt_fns = $asset->getEditFns();
				if (isset($asset_edt_fns->static_screens['preview'])) {
					$msg_reps['preview_url'] = $root_url.'/'.$asset->getBackendHref('preview', FALSE);
				} else {
					$msg_reps['preview_url'] = $root_url.'/'.$asset->getBackendHref('details', FALSE);
				}
				// see if we have the 'from' field set
				if ($schema->attr('schema_reply_to_email_address') != '') {
					$msg->parameters['reply_to'] = $schema->attr('schema_reply_to_email_address');
				} else {
					$msg->parameters['reply_to'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
				}
				if ($schema->attr('schema_from_email_address') != '') {
					$msg->from = $schema->attr('schema_from_email_address');
				}

				$msg->type    = 'asset.workflow.invitation.progress';
				$msg->parameters['assetid'] = $asset->id;

				// Does this step have a custom message (for inviting users to the next step)?
				if (isset($current_step['message_invitation']) && trim($current_step['message_invitation']) !== '' ) {
					$msg->body = trim($current_step['message_invitation']);
				}

				// Does this step have a custom subject (for inviting users to the next step)?
				if (isset($current_step['subject_invitation']) && trim($current_step['subject_invitation']) !== '' ) {
					$subject = trim(array_get_index($current_step, 'subject_invitation', 'Workflow Approval Required'));
					$msg->subject = (!empty($subject)) ? $subject : 'Workflow Approval Required';
				}

				$msg->replacements = $msg_reps;
				$this->addMessageReplacements($asset, $msg->subject, $msg->body, $msg->replacements);
				$ms->enqueueMessage($msg);
			}//end else
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end escalateWorkflow()


	/**
	* Returns an array of workflow steps containing the current or specified step and it's siblings
	*
	* @param array	&$workflow		the workflow we are searching in
	* @param array	$current_step	the address of the desired step
	*
	* @return array
	* @access public
	*/
	function &getCurrentStepArray(&$workflow, $current_step=NULL)
	{
		// go through workflow and return the current and sibling steps based on the current step
		if ($current_step == NULL) {
			$current_step = $workflow['current_step'];
		}

		array_pop($current_step);
		$steps =& $workflow['steps'];
		foreach ($current_step as $step_id) {
			$steps =& $steps[$step_id]['escalation_steps'];
		}
		return $steps;

	}//end getCurrentStepArray()


	/**
	* Returns a reference to the current or specified step
	*
	* @param array	&$workflow		the workflow we are searching in
	* @param array	$current_step	the address of the desired step
	*
	* @return array
	* @access public
	*/
	function &getCurrentStep(&$workflow, $current_step=NULL)
	{
		if ($current_step == NULL) {
			$current_step = $workflow['current_step'];
		}
		$steps =& $this->getCurrentStepArray($workflow, $current_step);

		return $steps[end($current_step)];

	}//end getCurrentStep()


	/**
	* Returns a list of streams attached to a certain schema
	*
	* Returned array is in format (streamid => stream name)
	*
	* @param string		$schemaid		Schema ID to search
	* @param boolean	$ignore_default	TRUE = ignore the default stream: only
	*									alternate streams are returned
	*
	* @return array
	*/
	public function getStreams($schemaid, $ignore_default=FALSE)
	{
		$stream_names = Array();
		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($schemaid)) {
			trigger_error("Workflow Schema Id #$schemaid is applied to this asset but cannot be found in system anymore.", E_USER_WARNING);
			return $stream_names;
		}
		$stream_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($schemaid, SQ_LINK_TYPE_2, 'workflow_stream', FALSE);
		$streamids = Array();

		foreach ($stream_links as $stream_link) {
			if (($ignore_default === FALSE) || ($stream_link['value'] !== 'default_stream')) {
				$streamids[] = $stream_link['minorid'];
			}
		}

		$stream_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($streamids, 'workflow_stream', TRUE, 'name');
		return $stream_names;

	}//end getStreams()


	/**
	* Get a list of stream IDs that could bypass workflow for the passed asset
	* and user
	*
	* @param string	$assetid		ID of the asset being tested
	* @param string	$schemaid		ID of the schema whose streams are being tested
	* @param string	$current_userid	Test with this user, or NULL for current user
	*/
	public function getBypassableStreams($assetid, $schemaid, $current_userid=NULL)
	{
		if ($current_userid === NULL) {
			$current_userid = $GLOBALS['SQ_SYSTEM']->user->id;
		}

		$bypassable_streams = Array();
		$streams = $this->getStreams($schemaid);
		foreach ($streams as $streamid => $stream_name) {
			$test_result = $this->testPublish($assetid, $current_userid, $stream_name);
			if ($test_result === TRUE) {
				$bypassable_streams[] = $streamid;
			}
		}

		return $bypassable_streams;

	}//end getBypassableStreams()


	/**
	* Returns the default stream for a certain schema or set of schemas
	*
	* If sent an array of schema IDs, the return value will be an array of
	* (schema ID => stream ID). If a single schema ID, the return will be a
	* scalar asset ID (string) value.
	*
	* @param mixed array|string	$schemaids	Schema ID(s) to search
	*
	* @return mixed array|string
	*/
	public function getDefaultStream($schemaids)
	{
		// Did we receive a single schema?
		$return_array = TRUE;
		if (is_array($schemaids) === FALSE) {
			$return_array = FALSE;
			$schemaids = Array($schemaids);
		}

		$return_value = Array();
		foreach ($schemaids as $schemaid) {
			$stream_link = $GLOBALS['SQ_SYSTEM']->am->getLink($schemaid, SQ_LINK_TYPE_2, 'workflow_stream', FALSE, 'default_stream');

			if (empty($stream_link) === FALSE) {
				$return_value[$schemaid] = $stream_link['minorid'];
			}
		}

		if ($return_array === FALSE) {
			$return_value = reset($return_value);
		}

		return $return_value;

	}//end getDefaultStream()


	/**
	* Returns the list of streams matching a defined stream name
	*
	* If a stream name does not exist for a passed schema, it will use the
	* default stream for these.
	*
	* If sent an array of schema IDs, the return value will be an array of
	* (schema ID => stream ID). If a single schema ID, the return will be a
	* scalar asset ID (string) value.
	*
	* @param mixed array|string	$schemaids		Schema ID(s) to search
	* @param string				$stream_name	Stream name to search for
	*
	* @return mixed array|string
	*/
	public function getStreamByName($schemaids, $stream_name)
	{
		// Did we receive a single schema?
		$return_array = TRUE;
		if (is_array($schemaids) === FALSE) {
			$return_array = FALSE;
			$schemaids = Array($schemaids);
		}

		// Find the stream name if it exists
		$return_value = Array();
		foreach ($schemaids as $schemaid) {
			$schema_stream_names = $this->getStreams($schemaid);
			$found_streamid = array_search($stream_name, $schema_stream_names);
			if ($found_streamid !== FALSE) {
				$return_value[$schemaid] = $found_streamid;
			}
		}

		// Now add the default stream for these
		$unfound_streams = array_diff($schemaids, array_keys($return_value));
		if (count($unfound_streams) > 0) {
			$unfound_streamids = $this->getDefaultStream($unfound_streams);
			$return_value += $unfound_streamids;
		}

		if ($return_array === FALSE) {
			$return_value1 = reset($return_value);
		}

		return $return_value;

	}//end getStreamByName()


	/**
	* Set an stream to use when starting the workflow for a certain asset
	*
	* This is to get around coupling issues - since the POST var handling should
	* be at the Details static screen level, but the starting of workflow
	* happens at status change level - and there is no way to pass extra data to
	* a status change.
	*
	* It is intended to be called immediately before starting workflow. This
	* only has effect for the request, and is cleared when workflow is
	* actually started.
	*
	* It can still be overridden by sending a stream ID to startWorkflow().
	*
	* If streamid is NULL, the set stream will be cleared, and the default
	* stream will be used instead.
	*
	* @param string	$assetid		The asset to set a starting stream for
	* @param string	$stream_name	The stream to set as "active". NULL to clear.
	*
	* @return void
	*/
	public function setStartingStream($assetid, $stream_name)
	{
		$this->_tmp['starting_streams'][$assetid] = $stream_name;

	}//end setStartingStream()


	/**
	* Return a set starting stream name
	*
	* @param string	$schemaid	The asset to get the starting stream name for
	*
	* @return string
	*/
	public function getStartingStream($assetid)
	{
		$starting_streams = array_get_index($this->_tmp, 'starting_streams', Array());
		return array_get_index($starting_streams, $assetid, NULL);

	}//end getStartingStream()


	/**
	* Add keyword values for the given $asset to the $replacements array
	* by extracting any extra keywords found in the $subject and $body.
	*
	* @param object $asset			The asset in workflow to get keyword replacements from
	* @param string $subject		The message subject
	* @param string $body			The message body
	* @param array  $replacements	The keywords and their replacement values
	*
	* @return void
	*/
	public function addMessageReplacements($asset, $subject, $body, &$replacements)
	{

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = retrieve_keywords_replacements($subject);
		foreach ($keywords as $keyword) {
			if (isset($replacements[$keyword])) continue;
			$replacement = $asset->getKeywordReplacement($keyword);
			if (!empty($replacement) && trim($replacement, '%') != $keyword) $replacements[$keyword] = $replacement;
		}

		$keywords = retrieve_keywords_replacements($body);
		foreach ($keywords as $keyword) {
			if (isset($replacements[$keyword])) continue;
			$replacement = $asset->getKeywordReplacement($keyword);
			if (!empty($replacement) && trim($replacement, '%') != $keyword) $replacements[$keyword] = $replacement;
		}

	}//end addMessageReplacements()


	/**
	* When the user is directly approving the asset changes without having to 
	* initiate the workflow, we need to set this user id as the value for 
	* 'last_started_by' col in the sq_ast_wflow table to prevent the issue 
	* cause in Bug #5871 Matrix emailing the workflow initiator from previous 
	* workflow process when asset pushed to Live.
	*
	* @param string	$userid		userid to set 'last_started_by' col value in db
	* @param string $assetid	id of the asset we are changing the status
	*
	* @return void
	*/
	public function setCurrentUserAsLastStarted($userid, $assetid)
	{
		// we need the workflow on this asset that aren't running
		$schemas = $this->getSchemas($assetid, TRUE, FALSE);

		$sql = "UPDATE sq_ast_wflow 
				SET last_started_by = :last_started_by 
				WHERE schemaid = :schemaid AND assetid = :assetid";

		foreach ($schemas as $schemaid) {
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'last_started_by', $userid);
				MatrixDAL::bindValueToPdo($query, 'schemaid', $schemaid);
				MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				trigger_localised_error('CORE0320', E_USER_NOTICE, $assetid, $schemaid, $e->getMessage());
				continue;
			}//end try/catch
		}//end foreach

	}//end setCurrentUserAsLastStarted()


}//end class

?>
