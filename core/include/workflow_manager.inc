<?php

/**
* Workflow_Manager
*
* Purpose
*    Facilitate the managment of workflow for assets
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Workflow_Manager extends Resolve_Object
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
		$this->Resolve_Object();

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

		return $schemas;

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

		if (!$asset->adminAccess()) {
			trigger_error('Cannot set workflow schema for "'.$asset->name.'", permission denied', E_USER_WARNING);
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

		if (!$asset->adminAccess()) {
			trigger_error('Cannot delete workflow schema for "'.$asset->name.'", permission denied', E_USER_WARNING);
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteSchema()


	/**
	* Get the workflow values set for an asset on a particular schema or all schemas
	*
	* @param int	$assetid	the ID of the asset to get workflow for
	* @param int	$schemaid	the ID of the schema to get workflow for (NULL for all workflows)
	*
	* @access public
	* @return array( schemaid => workflow_array )
	*/
	function getSchemaWorkflows($assetid, $schemaid=null)
	{
		$assetid  = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed getting workflow, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!is_null($schemaid)) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_error('Failed getting workflow, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
				return false;
			}

			if (!is_a($schema, 'workflow_schema')) {
				trigger_error('Failed getting workflow, "'.$schema->name.'" (Asset #'.$schemaid.') is not a workflow schema', E_USER_WARNING);
				return false;
			}

			// check that the schema we are getting workflow for is actually set
			$schemas = $this->getSchemas($assetid, true);
			if (!in_array($schemaid, $schemas)) return Array();
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '  SELECT schemaid, workflow
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_workflow ';
		$where = 'assetid = '.$db->quote($asset->id).'
				    AND access = 1';
		if (!is_null($schemaid)) $where .= ' AND schemaid = '.$db->quote($schemaid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$workflows = $db->getAssoc($sql.$where);
		if (DB::isError($workflows)) {
			trigger_error($workflows->getMessage().'<br />'.$workflows->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		for(reset($workflows); null !== ($k = key($workflows)); next($workflows)) {
			$workflows[$k] = (is_null($workflows[$k])) ? '' : unserialize($workflows[$k]);
		}

		if (!is_null($schemaid)) return $workflows[$schemaid];
		return $workflows;

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

		if (!$asset->adminAccess()) {
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
				return false;
			}

			if (!is_a($schema, 'workflow_schema')) {
				trigger_error('Failed setting workflow, "'.$schema->name.'" (Asset #'.$schemaid.') is not a workflow schema', E_USER_WARNING);
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
	* @param boolean	$group		group the return value by workflowid
	*
	* @return array(int)
	* @access public
	*/
	function whoCanPublish($assetid, $group=false)
	{
		// if there are no current workflows running, we are not waiting for anyone
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) return Array();

		// these will be the people who can approve this asset
		// at this point in the workflow
		$can_publish = Array();

		foreach ($running_schemas as $schemaid) {
			if ($group) {
				$can_publish[$schemaid] = $this->_whoCanPublishWorkflow($assetid, $schemaid);
				array_unique($can_publish[$schemaid]);
			} else {
				$can_publish = array_merge($can_publish, $this->_whoCanPublishWorkflow($assetid, $schemaid));
			}
		}

		if (!$group) $can_publish = array_unique($can_publish);
		return $can_publish;

	}//end whoCanPublish()


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
		// if this workflow is not running, we are not waiting for anyone
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (!in_array($schemaid, $running_schemas)) return Array();

		// these will be the people who can approve this asset at this point in the workflow
		$can_publish = Array();
		$workflow = $this->getSchemaWorkflows($assetid, $schemaid);

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
							$can_publish_cond = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false);
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
					$can_publish = array_merge($can_publish, array_diff($can_publish_cond, $cond_data['published_by']));
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
								$can_publish_cond = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false);
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
		// if there are no current workflows running, how can someone publish (you do the math)?
		$running_schemas = $this->getSchemas($assetid, true, true);
		if (empty($running_schemas)) return false;
		$schema_workflows = $this->getSchemaWorkflows($assetid);

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for (reset($schema_workflows); null !== ($schemaid = key($schema_workflows)); next($schema_workflows)) {
			if (!in_array($schemaid, $running_schemas)) continue;
			$workflow =& $schema_workflows[$schemaid];
			if (!$this->_recordPublish($workflow, $publisher)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$new_workflow = serialize($workflow);
			$values = Array('workflow' => $db->quote($new_workflow));
			$where = 'assetid = '.$db->quote($assetid).'
					    AND schemaid = '.$db->quote($schemaid);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
				return false;
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
						$can_publish = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false);
					}
				}

				if (in_array($publisher, $can_publish)) {
					if (!isset($cond_data['published_by'])) $cond_data['published_by'] = Array();
					// if the publisher has not already published
					if (!in_array($publisher, $cond_data['published_by'])) {
						$cond_data['published_by'][$publisher] = time();
						$workflow_updated = true;
					}
				}
			}
		}

		if ($workflow_updated) {
			// load the current step into the workflow
			$this->_loadCurrentStep($workflow);
			if ($workflow['current_step'] === 0) $workflow['complete'] = true;
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
	* @param int	$assetid	the ID of the asset to start workflow for
	*
	* @return boolean
	* @access public
	*/
	function startWorkflow($assetid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$schemas = $this->getSchemas($assetid, true, false);

		if (empty($schemas)) return false;

		foreach ($schemas as $schemaid) {
			$workflow = $this->generateWorkflowArray($assetid, $schemaid);
			if (empty($workflow['steps'])) continue;
			$workflow = serialize($workflow);

			$values = Array('workflow' => $db->quote($workflow));
			$where = 'assetid = '.$db->quote($assetid).'
					    AND schemaid = '.$db->quote($schemaid);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
				return false;
			}
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
	* Clears all generated workflow arrays from the workflow table for the passed assetID
	*
	* @param int	$assetid	the ID of the asset to cancel workflow for
	*
	* @return boolean
	* @access public
	*/
	function cancelWorkflow($assetid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$values = Array('workflow' => 'NULL');
		$where = 'assetid = '.$db->quote($assetid);
		return $GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where);

	}//end cancelWorkflow()

}//end class

?>