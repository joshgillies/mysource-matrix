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
* $Id: workflow_manager.inc,v 1.49.2.3 2004/11/03 23:57:53 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Workflow_Manager
*
* Purpose
*    Facilitate the managment of workflow for assets
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* @param int		$assetid	the ID of the asset to get schemas for
	* @param boolean	$access		type of access : null = all, true = applied, false = denied
	* @param boolean	$running	only getting running workflows
	*
	* @access public
	* @return array( schemaid => access[1|0] ) if $access is null OR array(schemaid) if it is set
	*/
	function getSchemas($assetid, $access=null, $running=false)
	{
		$storage_access = ((is_null($access)) ? 2 : (int) $access);
		if (!isset($this->_tmp['schemas'][(int)$assetid][$storage_access][(int)$running])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = '  SELECT DISTINCT schemaid, access
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_workflow ';
			$where = 'assetid = '.$db->quote($assetid);
			if ($running) $where .= ' AND workflow IS NOT NULL';
			if (!is_null($access) || $running) $where .= ' AND access = '.$db->quote((($access) ? '1' : '0'));
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			$schemas = Array();
			foreach ($result as $data) {
				if (is_null($access)) $schemas[$data['schemaid']] = $data['access'];
				else $schemas[] = $data['schemaid'];
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
	* @param boolean	$access		is this schema applied (TRUE) or denied (FALSE)?
	*
	* @access public
	* @return boolean
	*/
	function setSchema($assetid, $schemaid, $access)
	{
		$assetid  = (int)  $assetid;
		$schemaid = (int)  $schemaid;
		$access   = (bool) $access;

		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_error('Failed setting workflow schema, Schema Asset #'.$schemaid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!is_a($schema, 'workflow_schema')) {
			trigger_error('Failed setting workflow schema, "'.$schema->name.'" (Asset #'.$schemaid.') is not a workflow schema', E_USER_WARNING);
			return false;
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed setting workflow schema, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_error('Cannot set workflow schema for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		// check for any running schemas - because if the asset is in workflow we cant do anything
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (!empty($running_schemas)) {
			trigger_error('Cannot set workflow schema for "'.$asset->name.'", workflow is currently running', E_USER_WARNING);
			return false;
		}

		// get the current schemas that are set
		$schemas = $this->getSchemas($assetid);

		// check if this schema is already set
		if (isset($schemas[$schemaid])) {
			if ((bool)$schemas[$schemaid] == $access) {
				// schema is set with same access level
				return true;
			} else {
				// schema is set but with the opposite access level
				$new_access     = ($access) ? 'apply'  : 'deny';
				$current_access = ($access) ? 'denied' : 'applied';

				trigger_error('Could not '.$new_access.' workflow schema "'.$schema->name.'" on "'.$asset->name.'" because this asset already has this schema '.$current_access, E_USER_WARNING);
				return false;
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the schema to the database
		$values = Array('assetid'  => $asset->id,
						'schemaid' => $schemaid,
						'workflow' => NULL,
						'access'   => (($access) ? '1' : '0')
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_workflow', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		unset($this->_tmp['schemas'][(int)$assetid]);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end setSchema()


	/**
	* Remove a set schema from an asset
	*
	* @param int	$assetid	the ID of the asset to delete the schema from
	* @param int	$schemaid	the ID of the schema to delete
	*
	* @access public
	* @return boolean
	*/
	function deleteSchema($assetid, $schemaid)
	{
		$assetid  = (int) $assetid;
		$schemaid = (int) $schemaid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed deleting workflow schema, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_error('Cannot delete workflow schema for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}
		
		// check for any running schemas - because if the asset is in workflow we cant do anything
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (!empty($running_schemas)) {
			trigger_error('Cannot delete workflow schema for "'.$asset->name.'", workflow is currently running', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete and add rollback entries
		$where = 'schemaid    = '.$db->quote($schemaid).'
				  AND assetid = '.$db->quote($asset->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_workflow', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		unset($this->_tmp['schemas'][(int)$assetid]);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
	* @param int	$assetid	the ID of the asset to get workflow for
	* @param int	$schemaid	the ID of the schema to get workflow for (NULL for all workflows)
	*
	* @access public
	* @return array( schemaid => workflow_array )
	* @see Asset::useSystemVersion
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

			$sql = 'SELECT schemaid, workflow
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_workflow ';
			$where = 'assetid = '.$db->quote($assetid).'
						AND access = 1';
			if ($schemaid != 'all') $where .= ' AND schemaid = '.$db->quote($schemaid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			
			$workflows = $db->getAssoc($sql.$where, false, Array(), DB_FETCHMODE_ASSOC);
			if (DB::isError($workflows)) {
				trigger_error($workflows->getMessage().'<br />'.$workflows->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			for(reset($workflows); null !== ($k = key($workflows)); next($workflows)) {
				$workflows[$k] = (is_null($workflows[$k])) ? '' : unserialize($workflows[$k]);
			}

			if ($schemaid == 'all') $this->_tmp['schema_workflows'][$assetid]['all'] = $workflows;
			else					$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflows[$schemaid];
		}

		if (!isset($this->_tmp['schema_workflows'][$assetid][$schemaid])) return Array();
		return $this->_tmp['schema_workflows'][$assetid][$schemaid];

	}//end getSchemaWorkflows()


	/**
	* Set workflow values for an asset
	*
	* @param int	$assetid	the ID of the asset to set the values for
	* @param array	$workflow	the values to set
	*
	* @access public
	* @return boolean
	*/
	function setWorkflow($assetid, $workflow)
	{
		$assetid = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed setting workflow, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$asset->adminAccess('workflow')) {
			trigger_error('Cannot set workflow for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_error('Failed setting workflow, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if (!is_a($schema, 'workflow_schema')) {
				trigger_error('Failed setting workflow, "'.$schema->name.'" (Asset #'.$schemaid.') is not a workflow schema', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if (!isset($workflow[$schema->name])) continue;

			$values = Array('workflow' => $db->quote(serialize($workflow[$schema->name])));
			$where = 'assetid = '.$db->quote($asset->id).'
					    AND schemaid = '.$db->quote($schema->id);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$this->_tmp['schema_workflows'][$assetid][$schemaid] = $workflow[$schema->name];
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
			if (!is_null($schemaid) && $sid != $schemaid) continue;
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
			trigger_error('Failed to notify, asset #'.$assetid.' does not exist', E_USER_WARNING);
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
			$msg->subject = 'Asset Made Live';
			$msg->type    = 'asset.status.notify';
			$msg->body    = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name').' "'.$asset->name.'" has gone live from '.get_status_description($old_status);
			
			// create a link for navigation to the workflow screen for this asset
			$asset_url = $asset->getURL();
			if (!empty($asset_url)) {
				$msg->body .= "\n\n".'You can view the asset <SQ_MSG_LINK href="'.$asset_url.'">here</SQ_MSG_LINK>.';
			}

			$msg->parameters['assetid'] = $asset->id;
			$ms->enqueueMessage($msg);
		}
		
		return true;

	}//end notifyOnLive()


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
				if (!isset($cond_data['complete'])) $cond_data['complete'] = false;
				if (!isset($cond_data['published_by'])) $cond_data['published_by'] = Array();

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
				if (!isset($cond_data['complete'])) $cond_data['complete'] = false;
				if (!isset($cond_data['published_by'])) $cond_data['published_by'] = Array();

				if(empty($cond_data['published_by'])) {
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
			if ($step_data['logic'] == 'all' && $completed_conds < count($step_data['conditions'])) $continue = false;
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
			trigger_error('Failed recording workflow publish, Asset #'.$assetid.' does not exist', E_USER_WARNING);
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

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for (reset($schema_workflows); null !== ($schemaid = key($schema_workflows)); next($schema_workflows)) {
			if (!in_array($schemaid, $running_schemas)) continue;
			$workflow =& $schema_workflows[$schemaid];
			if ($this->_recordPublish($workflow, $publisher)) {
				$new_workflow = serialize($workflow);
				$values = Array('workflow' => $db->quote($new_workflow));
				$where = 'assetid = '.$db->quote($assetid).'
							AND schemaid = '.$db->quote($schemaid);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
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
		$user       = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$ms         = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		$log = $ms->newMessage(); 
		$log->subject = 'Asset Changes Approved';
		$log->body    = 'User "'.$user->name.'" has approved '.$asset_type.' "'.$asset->name.'" to go live.';
		// create a link for navigation to the workflow screen for this asset
		$log->body .= "\n\n".'A full report for the Workflow of "'.$asset->name.'" can be found on the <SQ_MSG_LINK href="'.current_url().$asset->getBackendHref('workflow').'">Workflow Screen</SQ_MSG_LINK>';
		$log->parameters['assetid'] = $asset->id;
		$log->parameters['version'] = substr($asset->version, 0, strrpos($asset->version, '.'));
		$log->send();

		// send internal messages to everyone who could approve before to let them know that someone did
		if (!$this->silentWorkflowParty($asset->id)) {
			$msg = $ms->newMessage($publishers_before);
			$msg->subject = $log->subject;
			$msg->body    = $log->body;
			$msg->type    = 'asset.workflow.announce';
			$msg->parameters['assetid'] = $asset->id;
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
					$msg->subject = 'Workflow Approval Required';
					$msg->body    = $asset_type.' "'.$asset->name.'" is currently in workflow and has progressed from step '.(int)$steps_before[$schemaid].' to step '.(int)$step.'.'."\n".'You are now required to approve or reject the changes made to "'.$asset->name.'" so the workflow process can continue.';
					// create a link for navigation to the workflow screen for this asset
					$msg->body .= "\n\n".'A full report for the Workflow of "'.$asset->name.'" can be found on the <SQ_MSG_LINK href="'.current_url().$asset->getBackendHref('workflow').'">Workflow Screen</SQ_MSG_LINK>';
					$msg->type    = 'asset.workflow.invitation';
					$msg->parameters['assetid'] = $asset->id;
					$msg->send();
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
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
			if (!$this->_recordPublish($workflow, $publisher)) return false;
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
		if (empty($workflow['steps'])) $workflow_updated = true;

		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			if (!isset($workflow['current_step'])) $workflow['current_step'] = 1;
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
					if (!isset($cond_data['published_by'])) $cond_data['published_by'] = Array();
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
			trigger_error('Failed getting workflow, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_error('Failed getting workflow, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!is_a($schema, 'workflow_schema')) {
			trigger_error('Failed getting workflow, "'.$schema->name.'" (Asset #'.$schemaid.') is not a workflow schema', E_USER_WARNING);
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
	* @param int						$assetid	the ID of the asset to start workflow for
	* @param object Internal_Message	$msg		an internal message to be sent instead of the default
	*
	* @return boolean
	* @access public
	*/
	function startWorkflow($assetid, $msg=null)
	{
		$assetid  = (int) $assetid;
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed starting workflow, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$schemas = $this->getSchemas($assetid, true, false);

		if (empty($schemas)) return false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($schemas as $schemaid) {
			$workflow = $this->generateWorkflowArray($assetid, $schemaid);
			if (empty($workflow['steps'])) continue;
			$workflow = serialize($workflow);

			$values = Array('workflow' => $db->quote($workflow));
			$where = 'assetid = '.$db->quote($assetid).'
					    AND schemaid = '.$db->quote($schemaid);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$this->_tmp = Array();

		// send internal messages to everyone who can now publish the asset
		$ms   = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$publishers = $this->whoCanPublish($asset->id);

		if (!$this->silentWorkflowParty($asset->id)) {
			// create a new internal message if we have not been supplied with one
			if (is_null($msg)) $msg = $ms->newMessage();
			// added the publishers to the list of people that are to be notified
			$msg->to = array_merge($msg->to, $publishers);

			// complete any missing internal message fields that have not been supplied
			if (empty($msg->subject)) $msg->subject = 'Workflow Approval Required';
			if (empty($msg->body)) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
				$asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');

				$msg->body = $asset_type.' "'.$asset->name.'" is currently in workflow.'."\n".'You are now required to approve or reject the changes made to "'.$asset->name.'" so the workflow process can continue.';

				// create a link for navigation to the workflow screen for this asset
				$msg->body .= "\n\n".'A full report for the Workflow of "'.$asset->name.'" can be found on the <SQ_MSG_LINK href="'.current_url().$asset->getBackendHref('workflow').'">Workflow Screen</SQ_MSG_LINK>';
				$asset_url = $asset->getURL();
				if (!empty($asset_url)) {
					$msg->body .= ' You can also <SQ_MSG_LINK href="'.$asset_url.'">preview this asset.</SQ_MSG_LINK>';
				}
			}
			if (empty($msg->type)) $msg->type = 'asset.workflow.invitation';

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
	* @return boolean
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

	}//end isWorkflowComplete()


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
			trigger_error('Failed cancelling workflow, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$this->_clearWorkflow($assetid)) return false;

		// send internal messages to everyone who could approve before to let them know that someone did
		if (!$this->silentWorkflowParty($asset->id)) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$publishers = $this->whoCanPublish($asset->id);
			
			// create a new internal message if we have not been supplied with one
			if (is_null($msg)) $msg = $ms->newMessage();
			// added the publishers to the list of people that are to be notified
			$msg->to = array_merge($msg->to, $publishers);

			// complete any missing internal message fields that have not been supplied
			if (empty($msg->subject)) $msg->subject = 'Asset Changes Rejected';
			if (empty($msg->body)) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
				$asset_type = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');

				$msg->body = 'User "'.$user->name.'" has rejected the changes made to '.$asset_type.' "'.$asset->name.'". The workflow process has been cancelled.';
				// create a link for navigation to the workflow screen for this asset
				$msg->body .= "\n\n".'A full report for the Workflow of "'.$asset->name.'" can be found on the <SQ_MSG_LINK href="'.current_url().$asset->getBackendHref('workflow').'">Workflow Screen</SQ_MSG_LINK>';
				$asset_url = $asset->getURL();
				if (!empty($asset_url)) {
					$msg->body .= ' You can also <SQ_MSG_LINK href="'.$asset_url.'">preview this asset.</SQ_MSG_LINK>';
				}
			}
			if (empty($msg->type)) $msg->type = 'asset.workflow.announce';
			
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
			trigger_error('Failed completing workflow, Asset #'.$assetid.' does not exist', E_USER_WARNING);
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$values = Array('workflow' => 'NULL');
		$where = 'assetid = '.$db->quote($assetid);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
			trigger_error('Failed clearing workflow for asset #'.$assetid, E_USER_WARNING);
			return false;
		}
		
		unset($this->_tmp['schemas'][$assetid]);
		unset($this->_tmp['schema_workflows'][$assetid]);
		return true;

	}//end cancelWorkflow()


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

	}//end allowsWorkflow()

}//end class

?>
