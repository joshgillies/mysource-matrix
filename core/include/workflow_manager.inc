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
	* The asset whose workflow we are managing
	*
	* @var object Asset
	*/
	var $_asset = null;

	/**
	* An array of steps to follow for workflow
	*
	* @var array
	*/
	var $steps = Array();

	/**
	* The current state of this workflow
	*
	* @var array
	*/
	var $state = '';

	/**
	* An array of workflow objects for all workflows we inherit
	*
	* @var array
	*/
	var $inherited_workflows = Array();

	/**
	* An array of workflows that are currently in progress
	*
	* @var array
	*/
	var $current_workflows = Array('running' => Array(), 'complete' => Array());

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
	* @param object Asset	&$asset			the asset to manage
	* @param boolean		$and_direct		load the direct workflow for the asset
	* @param boolean		$and_inherited	load all inherited workflows for the asset
	* @param boolean		$and_running	load all running workflows for the asset
	*/
	function Workflow_Manager(&$asset, $and_direct=true, $and_inherited=true, $and_running=true)
	{
		$this->Resolve_Object();
		$this->_asset =& $asset;
		if ($and_direct) {
			// we are going to load this workflows
			// direct workflow (if it has been defined)
			$this->load($and_inherited);
		}
		if ($and_running) {
			// load the currently running workflows
			// so we can determine publish access
			$this->loadRunning();
		}

	}//end constructor


	/**
	* Loads the workflow steps for this asset into the manager
	* Also optionally loads all inherited workflows into the manager as well
	*
	* @param boolean	$and_inherited	load inherited workflow objects as well
	*
	* @return boolean
	* @access public
	*/
	function load($and_inherited)
	{
		// get the direct workflow if it has been set
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'assetid = '.$db->quote($this->_asset->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT workflow FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_workflow '.$where;

		$direct = $db->getOne($sql);
		if (DB::isError($direct)) {
			trigger_error($direct->getMessage().'<br/>'.$direct->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (empty($direct)) {
			$this->state = 'new';
		} else {
			$this->state = 'idle';
			$this->steps = unserialize($direct);
		}

		if ($and_inherited) {
			// now lets do the inherited workflows
			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->_asset->id);
			$above_me = $parents;
			for($i = 0; $i < count($above_me); $i++) $above_me[$i] = $db->quote($above_me[$i]);
			$in_cond = 'IN ('.implode(', ', $above_me).')';
			
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_permission ';
			$where = 'assetid '.$in_cond.'
						AND access = 1
						AND permission = '.$db->quote(SQ_PERMISSION_WORKFLOW);
			$where .= ' AND userid NOT IN (~SQ0~)';

			$sub_where = 'assetid = '.$db->quote($this->_asset->id).'
							AND access = 0
							AND permission = '.$db->quote(SQ_PERMISSION_WORKFLOW);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where);
			$subs = Array( 'SELECT userid
							FROM '.SQ_TABLE_PREFIX.'asset_permission '.$sub_where );

			$where = db_extras_subquery($db, $where, $subs);
			if (DB::isError($where)) trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$inherited = $db->getCol($sql.$where);
			if (DB::isError($inherited)) {
				trigger_error($inherited->getMessage().'<br/>'.$inherited->getUserInfo(), E_USER_WARNING);
				return false;
			}

			foreach ($inherited as $assetid) {
				if ($assetid == $this->_asset->id) continue;
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$this->inherited_workflows[$assetid] =& new Workflow_Manager($asset, true, false, false);
			}

		}

		return true;

	}//end load()


	/**
	* Loads currently running workflow steps into the manager
	*
	* @return boolean
	* @access public
	*/
	function loadRunning()
	{
		// lets take a look in the workflow table to see
		// if this workflow has started and what its status is
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'workflowid LIKE '.$db->quote($this->_asset->id.'~%');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_running_workflow '.$where;

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (empty($result)) {
			return false;
		}

		// we have a current workflow in progress
		$this->state = 'running';

		// run through and work out what steps we are up to
		foreach ($result as $workflow) {
			$steps = unserialize($workflow['workflow']);
			$storage_name = (($workflow['complete']) ? 'complete' : 'running');
			$this->current_workflows[$storage_name][$workflow['workflowid']] = Array('steps' => $steps, 'current_step' => 0);

			// if the workflow is not complete, work out what step we are up to
			if (!$workflow['complete']) $this->_loadCurrentStep($workflow['workflowid']);
		}

		// check all our running workflows
		$cw =& $this->current_workflows['running'];
		for (reset($cw); null !== ($workflowid = key($cw)); next($cw)) {
			$workflow =& $cw[$workflowid];
			if ($workflow['current_step'] === 0) {
				$this->current_workflows['complete'][$workflowid] = $this->current_workflows['running'][$workflowid];
			}
		}

		foreach ($this->current_workflows['running'] as $workflowid => $data) {
			if (isset($this->current_workflows['complete'][$workflowid])) {
				unset($this->current_workflows['running'][$workflowid]);
			}
		}

		if (empty($this->current_workflows['running'])) $this->state = 'complete';

		return true;

	}//end loadRunning()


	/**
	* Goes through the passed running workflow and sets what step we are up to
	*
	* @param int	$workflowid	the workflow we are working out the current step for
	*
	* @return void
	* @access private
	*/
	function _loadCurrentStep($workflowid)
	{
		// if there are no current workflows running, how can someone publish?
		if (!isset($this->current_workflows['running'][$workflowid])) return false;

		// run through and work out what step we are up to
		$workflow =& $this->current_workflows['running'][$workflowid];
		$workflow['current_step'] = 0;
		$continue = false;
		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			$step_data =& $workflow['steps'][$stepid];

			$workflow['current_step'] = $stepid;
			$continue = false;
			$completed_conds = 0;

			for (reset($step_data['conditions']); null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
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
					if ($cond_data['require'] == 'all') {
						// lets work out what number 'ALL' represents
						$can_publish_cond = Array();
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
						if ($asset->id) {
							if (is_a($asset, 'user')) {
								$can_publish_cond[] = $asset->id;
							} else if (is_a($asset, 'user_group')) {
								$can_publish_cond = $asset->getChildren(Array('user'), false);
							}
						}
						$cond_string = 'if ('.count($cond_data['published_by']).' >= '.count($can_publish_cond).') { $complete = true; }';
					} else {
						$cond_string = 'if ('.count($cond_data['published_by']).' '.$cond_data['require'].') { $complete = true; }';
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
						// this step is completed
						// so move to the next one
						break;
					} else {
						// step is not finsihed yet so move
						// to the next condition
						$continue = false;
						continue;
					}
				}
			}
			if (!$continue) break;
		}

		if ($continue) $workflow['current_step'] = 0;

	}//end _loadCurrentStep()


	/**
	* Returns an array of all users who can publish one of the running workflows
	* Note that the publishers are based on the current step in the workflow
	* so the users returned are only users who will affect the workflow at this point
	*
	* @param boolean	$group	group the return value by workflowid
	*
	* @return array(int)
	* @access public
	*/
	function whoCanPublish($group=false)
	{
		// if there are no current workflows running, we are not waiting for anyone
		if (!isset($this->current_workflows['running']) || empty($this->current_workflows['running'])) return Array();

		// these will be the people who can approve this asset
		// at this point in the workflow
		$can_publish = Array();

		foreach ($this->current_workflows['running'] as $workflowid => $workflow) {
			if ($group) {
				$can_publish[$workflowid] = $this->_whoCanPublishWorkflow($workflowid);
				array_unique($can_publish[$workflowid]);
			} else {
				$can_publish = array_merge($can_publish, $this->_whoCanPublishWorkflow($workflowid));
			}
		}

		if (!$group) $can_publish = array_unique($can_publish);
		return $can_publish;

	}//end whoCanPublish()


	/**
	* Returns an array of all users who can publish the passed running workflow
	* Note that the publishers are based on the current step in the workflow
	* so the users returned are only users who will affect the workflow at this point
	*
	* @param int	$workflowid	the id of the running workflow to find publishers for
	*
	* @return array(int)
	* @access private
	*/
	function _whoCanPublishWorkflow($workflowid)
	{
		// if this workflow is not running, we are not waiting for anyone
		if (!isset($this->current_workflows['running'][$workflowid])) return Array();

		// these will be the people who can approve this asset
		// at this point in the workflow
		$can_publish = Array();
		$workflow =& $this->current_workflows['running'][$workflowid];

		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			$step_data =& $workflow['steps'][$stepid];
			if ($stepid != $workflow['current_step']) continue;

			for (reset($step_data['conditions']); null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
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
							$can_publish_cond = $asset->getChildren(Array('user'), false);
						}
					}

					if ($cond_data['require'] == 'all') {
						$cond_string = 'if ('.count($cond_data['published_by']).' >= '.count($can_publish_cond).') { $complete = true; }';
					} else {
						$cond_string = 'if ('.count($cond_data['published_by']).' '.$cond_data['require'].') { $complete = true; }';
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
	* Changes the workflows to indicate that a user has approved the asset
	*
	* @param int	$publisher	the id of the user who has approved
	*
	* @return boolean
	* @access public
	*/
	function recordPublish($publisher)
	{
		// if there are no current workflows running, how can someone publish?
		if (!isset($this->current_workflows['running']) || empty($this->current_workflows['running'])) return false;
		$cw =& $this->current_workflows['running'];
		$updated = false;

		for (reset($cw); null !== ($workflowid = key($cw)); next($cw)) {
			$workflow =& $cw[$workflowid];
			$workflow_updated = false;

			// safety code for blank workflows
			if (empty($workflow['steps'])) $workflow_updated = true;

			for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
				if ($stepid != $workflow['current_step']) continue;
				$step_data =& $workflow['steps'][$stepid];

				for (reset($step_data['conditions']); null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
					$cond_data =& $step_data['conditions'][$pub];
					// work out who can publish this condition
					$can_publish = Array();
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($pub);
					if ($asset->id) {
						if (is_a($asset, 'user')) {
							$can_publish[] = $asset->id;
						} else if (is_a($asset, 'user_group')) {
							$can_publish = $asset->getChildren(Array('user'), false);
						}
					}

					if (in_array($publisher, $can_publish)) {
						if (!isset($cond_data['published_by'])) $cond_data['published_by'] = Array();
						// if the publisher has not already published
						if (!in_array($publisher, $cond_data['published_by'])) {
							$cond_data['published_by'][] = $publisher;
							$workflow_updated = true;
						}
					}
				}
			}

			if ($workflow_updated) {
				$this->_loadCurrentStep($workflowid);
				if ($workflow['current_step'] === 0) {
					$this->current_workflows['complete'][$workflowid] = $this->current_workflows['running'][$workflowid];
				}
				$updated = true;
			}
		}

		foreach ($this->current_workflows['running'] as $workflowid => $data) {
			if (isset($this->current_workflows['complete'][$workflowid])) {
				unset($this->current_workflows['running'][$workflowid]);
			}
		}

		if (empty($this->current_workflows['running'])) {
			// we finished all our workflows.. YAY!!
			$this->state = 'complete';
			return true;
		}

		return $updated;

	}//end recordPublish()


	/**
	* Pretend this user has approved the workflow to see what the resulting state will be
	* Use this to find out if the user is allowed to live edit
	*
	* @param int	$publisher	the id of the user who we are testing approval for
	*
	* @return string
	* @access public
	*/
	function testPublish($publisher)
	{
		// remember what the old vars were
		$old_cw = $this->current_workflows;
		$old_state = $this->state;

		if ($this->state != 'running') {
			// we dont actually have any running
			// workflows, so lets pretend we do
			$workflows_todo = Array();
			if (!empty($this->inherited_workflows)) {
				foreach ($this->inherited_workflows as $assetid => $workflow) {
					if (empty($workflow->steps)) continue;
					$workflows_todo[$assetid] = $workflow->steps;
				}
			}
			$workflows_todo[$this->_asset->id] = $this->steps;

			if (empty($workflows_todo)) return $this->state;

			foreach ($workflows_todo as $assetid => $steps) {
				$workflowid = "{$this->_asset->id}~{$assetid}";
				$this->current_workflows['running'][$workflowid]['steps'] = $steps;
				$this->current_workflows['running'][$workflowid]['current_step'] = 1;
			}
		}

		$this->recordPublish($publisher);
		$new_state = $this->state;

		// set the vars back to their original values
		// because we dont want to change anything
		$this->current_workflows = $old_cw;
		$this->state = $old_state;

		return $new_state;

	}//end testPublish()


	/**
	* Returns an array of ids of assets that have currently running workflows based on this workflow
	*
	* @return boolean
	* @access public
	*/
	function getRunningInheriting()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'workflowid LIKE '.$db->quote('%~'.$this->_asset->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT workflowid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_running_workflow '.$where;

		$result = $db->getCol($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$return_result = Array();
		foreach ($result as $id) {
			list($assetid, $me) = explode('~', $id);
			if ($assetid == $me) continue;
			$return_result[] = $assetid;
		}

		return array_unique($return_result);

	}//end getRunningInheriting()


	/**
	* Starts the workflow by writing out all workflows to the running table
	*
	* @return boolean
	* @access public
	*/
	function start()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$workflows_todo = Array();
		if (!empty($this->inherited_workflows)) {
			foreach ($this->inherited_workflows as $assetid => $workflow) {
				if (empty($workflow->steps)) continue;
				$workflows_todo[$assetid] = $workflow->steps;
			}
		}
		$workflows_todo[$this->_asset->id] = $this->steps;

		if (empty($workflows_todo)) return false;

		foreach ($workflows_todo as $assetid => $steps) {
			if (empty($steps)) continue;
			$workflowid = "{$this->_asset->id}~{$assetid}";
			$workflow = serialize($steps);

			$values = Array('workflowid' => $workflowid,
							'complete'   => 0,
							'workflow'   => $workflow);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_running_workflow', $values)) {
				return false;
			}
		}

		return true;

	}//end start()


	/**
	* Writes the current state of this workflow to the database
	*
	* @return boolean
	* @access public
	*/
	function save()
	{
		$steps = serialize($this->steps);
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		switch ($this->state) {
			case 'new' :
				if (!empty($this->steps)) {
					// a brand new direct workflow to save
					$values = Array('assetid'  => $this->_asset->id,
									'workflow' => $steps
									);
					if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_workflow', $values)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

					if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->_asset->id, $this->_asset->id, SQ_PERMISSION_WORKFLOW, '1')) {
						trigger_error('Failed saving workflow for "'.$this->_asset->name.'", could not grant workflow permission', E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
				}
			break;

			case 'idle' :
				if (empty($this->steps)) {
					// workflow is no more
					$where_cond = 'assetid = '.$db->quote($this->_asset->id);
					if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_workflow', $where_cond)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

					if (!$GLOBALS['SQ_SYSTEM']->am->deletePermission($this->_asset->id, $this->_asset->id, SQ_PERMISSION_WORKFLOW)) {
						trigger_error('Failed saving workflow for "'.$this->_asset->name.'", could not revoke workflow permission', E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
				} else {
					// updating an existing direct workflow
					$where = 'assetid = '.$db->quote($this->_asset->id);
					$values = Array('workflow' => $db->quote($steps));
					if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_workflow', $values, $where)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
				}
			break;

			case 'running' :
				// updating existing running workflows
				if (!empty($this->current_workflows['running'])) {
					foreach ($this->current_workflows['running'] as $workflowid => $workflow) {
						$steps = serialize($workflow['steps']);
						
						$where = 'workflowid = '.$db->quote($workflowid);
						$values = Array('complete' => 0,
										'workflow' => $db->quote($steps)
										);
						if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_running_workflow', $values, $where)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return false;
						}
					}
				}
				if (!empty($this->current_workflows['complete'])) {
					foreach ($this->current_workflows['complete'] as $workflowid => $workflow) {
						$steps = serialize($workflow['steps']);

						$where = 'workflowid = '.$db->quote($workflowid);
						$values = Array('complete' => 1,
										'workflow' => $db->quote($steps)
										);
						if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_running_workflow', $values, $where)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return false;
						}
					}
				}
			break;

			case 'complete' :
				// all workflows are completed
				$where_cond = 'workflowid LIKE '.$db->quote($this->_asset->id.'~%');
				if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_running_workflow', $where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			break;

			default :
				return false;
			break;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end save()


	/**
	* Cancels all currently running workflows
	*
	* @return boolean
	* @access public
	*/
	function cancel()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where_cond = 'workflowid LIKE '.$db->quote($this->_asset->id.'~%');
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_running_workflow', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		return true;

	}//end cancel()


	/**
	* Adds a step to the direct workflow for the asset being managed
	* Returns the new stepid
	*
	* @return int
	* @access public
	*/
	function addStep()
	{
		$stepid = count($this->steps) + 1;

		$this->steps[$stepid] = Array();
		$this->steps[$stepid]['logic'] = '>= 1';
		$this->steps[$stepid]['conditions'] = Array();

		// add a default condition - that the user that set this has to approve
		$this->addCondition($stepid, $GLOBALS['SQ_SYSTEM']->currentUserid());

		return $stepid;

	}//end addStep()


	/**
	* Deletes a step from the direct workflow for the asset being managed
	*
	* @param int	$stepid	the id of the step to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteStep($stepid)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->steps[$stepid])) {
			trigger_error("Cannot delete workflow step, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		unset($this->steps[$stepid]);

		// move all the other steps up one position
		$new_steps = Array();
		foreach ($this->steps as $step => $step_data) {
			if ($step < $stepid) $new_stepid = $step;
			else $new_stepid = $step -1;
			$new_steps[$new_stepid] = $step_data;
		}
		$this->steps = $new_steps;

		return true;

	}//end deleteStep()


	/**
	* Sets how many conditions need to be completed before this step is complete
	*
	* @param int	$stepid		the id of the step to set the logic for
	* @param string	$logic		the logic string (from $this->_valid_step_logic)
	* @param int	$logic_num	how many conditions need to be approved
	*
	* @return boolean
	* @access public
	*/
	function setStepLogic($stepid, $logic='>=', $logic_num=1)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->steps[$stepid])) {
			trigger_error("Cannot edit step logic, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$logic = trim((string)$logic);
		if (!in_array($logic, array_keys($this->_valid_step_logic))) {
			trigger_error("Cannot edit step logic, '$logic' is not a valid step logic", E_USER_WARNING);
			return false;
		}

		$logic_num = (int)$logic_num;
		if (!$logic_num && $logic != 'all') {
			trigger_error("Cannot edit step logic, logic number cannot be zero", E_USER_WARNING);
			return false;
		}

		// work out the logic expression
		$logic = $logic.(($logic_num) ? " $logic_num" : '');
		$this->steps[$stepid]['logic'] = $logic;

		return true;
	}//end setStepLogic()


	/**
	* Adds a condition to the passed step
	*
	* @param int	$stepid		the id of the step the condition is in
	* @param int	$publisher	the id of the publisher / condition
	* @param string	$logic		the logic string (from $this->_valid_cond_logic)
	* @param int	$logic_num	how many users in a group need to approve
	*
	* @return boolean
	* @access public
	*/
	function addCondition($stepid, $publisher, $logic='>=', $logic_num=1)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->steps[$stepid])) {
			trigger_error("Cannot add step condition, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$logic = trim((string)$logic);
		if (!isset($this->_valid_cond_logic[$logic])) {
			trigger_error("Cannot add step condition, '$logic' is not a valid condition logic", E_USER_WARNING);
			return false;
		}

		$publisher = (int)$publisher;
		if (isset($this->steps[$stepid]['conditions'][$publisher])) {
			trigger_error("Cannot add step condition, publisher $publisher is already set in step $stepid", E_USER_WARNING);
			return false;
		}

		// work out the logic expression
		$logic_num = (int)$logic_num;
		$require = $logic.(($logic_num) ? " $logic_num" : '');
		$this->steps[$stepid]['conditions'][$publisher] = Array('require' => $require, 'published_by' => Array());

		return true;
	}//end addCondition()


	/**
	* Sets how many users need to approve before the condition is complete
	*
	* @param int	$stepid		the id of the step the condition is in
	* @param int	$publisher	the id of the publisher / condition
	* @param string	$logic		the logic string (from $this->_valid_cond_logic)
	* @param int	$logic_num	how many users in a group need to approve
	*
	* @return boolean
	* @access public
	*/
	function setConditionLogic($stepid, $publisher, $logic='>=', $logic_num=1)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->steps[$stepid])) {
			trigger_error("Cannot edit step condition logic, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$logic = trim((string)$logic);
		if (!isset($this->_valid_cond_logic[$logic])) {
			trigger_error("Cannot edit step condition logic, '$logic' is not a valid condition logic", E_USER_WARNING);
			return false;
		}

		$publisher = (int)$publisher;
		if (!isset($this->steps[$stepid]['conditions'][$publisher])) {
			trigger_error("Cannot edit step condition logic, publisher $publisher is not set in step $stepid", E_USER_WARNING);
			return false;
		}

		$logic_num = (int)$logic_num;
		if (!$logic_num && $logic != 'all') {
			trigger_error("Cannot edit step condition logic, logic number cannot be zero", E_USER_WARNING);
			return false;
		}

		// work out the logic expression
		$require = $logic.(($logic_num) ? " $logic_num" : '');
		$this->steps[$stepid]['conditions'][$publisher]['require'] = $require;

		return true;
	}//end setConditionLogic()


	/**
	* Deletes a condition from the passed step
	*
	* @param int	$stepid		the id of the step the condition is in
	* @param int	$publisher	the id of the publisher / condition
	*
	* @return boolean
	* @access public
	*/
	function deleteCondition($stepid, $publisher)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->steps[$stepid])) {
			trigger_error("Cannot delete step condition, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$publisher = (int)$publisher;
		if (!isset($this->steps[$stepid]['conditions'][$publisher])) {
			trigger_error("Cannot delete step condition, publisher $publisher does not exist in step $stepid", E_USER_WARNING);
			return false;
		}

		if (count($this->steps[$stepid]['conditions']) == 1) {
			trigger_error("Cannot delete step condition, it is the last condition left in step $stepid", E_USER_WARNING);
			return false;
		}

		unset($this->steps[$stepid]['conditions'][$publisher]);

		return true;
	}//end deleteCondition()


}//end class
?>
