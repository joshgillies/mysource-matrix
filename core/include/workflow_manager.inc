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
	var $_steps = Array();

	/**
	* The current state of this workflow
	*
	* @var array
	*/
	var $_state = '';

	/**
	* An array of workflow objects for all workflows we inherit
	*
	* @var array
	*/
	var $_inherited_workflows = Array();

	/**
	* An array of workflows that are currently in progress
	*
	* @var array
	*/
	var $_current_workflows = Array();

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


	function load($and_inherited)
	{
		// get the direct workflow if it has been set
		$direct = $this->_asset->getPermission(SQ_PERMISSION_WORKFLOW, false, false);

		if (empty($direct) || !isset($direct[0][$this->_asset->id])) {
			$this->_state = 'new';
		} else {
			$this->_state = 'idle';
			$this->_steps = unserialize($direct[0][$this->_asset->id]);
		}

		if ($and_inherited) {
			// now lets do the inherited workflows
			$users    = $this->_asset->getPermission(SQ_PERMISSION_WORKFLOW, false);
			$inherits = $this->_asset->getInheritedPermission(SQ_PERMISSION_WORKFLOW, array_keys($users));
			$inherited = $inherits + $users;
			if (isset($inherited[0])) {
				foreach ($inherited[0] as $assetid => $workflow) {
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					$this->_inherited_workflows[$assetid] =& new Workflow_Manager($asset, false);
				}
			}
		}

		return true;
	}


	function loadRunning()
	{
		// lets take a look in the workflow table to see
		// if this workflow has started and what its status is
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT * FROM sq_asset_workflow
					WHERE workflowid LIKE '.$db->quote($this->_asset->id.'~%');

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (empty($result)) {
			return false;
		}

		// we have a current workflow in progress
		$this->_state = 'current';

		// run through and work out what steps we are up to
		foreach ($result as $workflow) {
			$steps = unserialize($workflow['workflow']);
			$storage_name = (($workflow['complete']) ? 'complete' : 'running');
			$this->_current_workflows[$storage_name][$workflow['workflowid']] = Array('steps' => $steps, 'current_step' => 0);

			// if the workflow is not complete, work out what step we are up to
			if (!$workflow['complete']) $this->_loadCurrentStep($workflow['workflowid']);
		}

		return true;
	}//end loadRunning()


	function _loadCurrentStep($workflowid)
	{
		// if there are no current workflows running, how can someone publish?
		if (!isset($this->_current_workflows['running'][$workflowid])) return false;

		// run through and work out what step we are up to
		$workflow =& $this->_current_workflows['running'][$workflowid];
		$continue = false;
		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			$step_data =& $workflow['steps'][$stepid];
			
			$workflow['current_step'] = $stepid;
			$continue = false;
			$completed_conds = 0;

			for (reset($step_data['conditions']); null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
				$cond_data =& $step_data['conditions'][$pub];

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

				if ($complete) {
					// this condition has been completed
					$continue = true;
					$completed_conds++;
					if ($step_data['logic'] == 'all') continue;

					$completed = false;
					$logic_string = 'if ('.$completed_conds.' '.$step_data['logic'].' ) { $completed = true; }';
					eval($logic_string);

					if ($completed) break;
					else continue;
				}
			}
			if (!$continue) break;
		}
		if ($continue) {
			$completed = false;
			$logic_string = 'if ('.$completed_conds.' '.$step_data['logic'].' ) { $completed = true; }';
			eval($logic_string);
			if ($completed) $workflow['current_step'] = 0;
		}
	}


	function whoCanPublish($group=false)
	{
		// if there are no current workflows running, we are not waiting for anyone
		if (!isset($this->_current_workflows['running']) || empty($this->_current_workflows['running'])) return Array();

		// these will be the people who can approve this asset
		// at this point in the workflow
		$can_publish = Array();

		foreach ($this->_current_workflows['running'] as $workflowid => $workflow) {
			if ($group) {
				$can_publish[$workflowid] = $this->_whoCanPublishWorkflow($workflowid);
				array_unique($can_publish[$workflowid]);
			} else {
				$can_publish = array_merge($can_publish, $this->_whoCanPublishWorkflow($workflowid));
			}
		}

		if (!$group) $can_publish = array_unique($can_publish);
		return $can_publish;
	}


	function _whoCanPublishWorkflow($workflowid)
	{
		// if this workflow is not running, we are not waiting for anyone
		if (!isset($this->_current_workflows['running'][$workflowid])) return Array();

		// these will be the people who can approve this asset
		// at this point in the workflow
		$can_publish = Array();
		$workflow =& $this->_current_workflows['running'][$workflowid];

		for (reset($workflow['steps']); null !== ($stepid = key($workflow['steps'])); next($workflow['steps'])) {
			$step_data =& $workflow['steps'][$stepid];
			if ($stepid != $workflow['current_step']) continue;

			for (reset($step_data['conditions']); null !== ($pub = key($step_data['conditions'])); next($step_data['conditions'])) {
				$cond_data =& $step_data['conditions'][$pub];

				// these are the people who have already approved this asset for publishing
				if (!isset($cond_data['published_by'])) $cond_data['published_by'] = Array();

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

				if (!$complete) {
					$can_publish = array_merge($can_publish, array_diff($can_publish_cond, $cond_data['published_by']));
				}
			}
		}

		return array_unique($can_publish);
	}


	function recordPublish($publisher)
	{
		// if there are no current workflows running, how can someone publish?
		if (!isset($this->_current_workflows['running']) || empty($this->_current_workflows['running'])) return false;
		$cw =& $this->_current_workflows['running'];
		$updated = false;

		for (reset($cw); null !== ($workflowid = key($cw)); next($cw)) {
			$workflow =& $cw[$workflowid];
			$workflow_updated = false;

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
						if (!isset($cond_data['published_by']))  $cond_data['published_by'] = Array();
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
					$this->_current_workflows['completed'][$workflowid] = $this->_current_workflows['running'][$workflowid];
					unset($this->_current_workflows['running'][$workflowid]);
					if (empty($this->_current_workflows['running'])) {
						// we finished all our workflows.. YAY!!
						// now we can publish this asset
						bam('ALL WORKFLOWS COMPLETED :)');
						return true;
					}
					prev($cw);
				}
				$updated = true;
			}
		}

		return $updated;

	}


	function start()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		
		$workflows_todo = Array();
		if (!empty($this->_inherited_workflows)) {
			foreach ($this->_inherited_workflows as $assetid => $workflow) {
				$workflows_todo[$assetid] = $workflow->_steps;
			}
		}
		$workflows_todo[$this->_asset->id] = $this->_steps;
		
		foreach ($workflows_todo as $assetid => $steps) {
			$workflowid = "{$this->_asset->id}~{$assetid}";
			$workflow = serialize($steps);
			$sql = 'INSERT INTO sq_asset_workflow
					(workflowid, complete, workflow)
					VALUES 
					('.$db->quote($workflowid).', 0, '.$db->quote($workflow).')';

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
		}
	}


	function save()
	{
		$steps = serialize($this->_steps);
		switch ($this->_state) {
			case 'new'      :
				// a brand new direct workflow to save
				return $this->_asset->grantPermission('0', SQ_PERMISSION_WORKFLOW, $steps);
				break;
			case 'idle'       :
				// updating an existing direct workflow
				return $this->_asset->editPermission('0', SQ_PERMISSION_WORKFLOW, '0', $steps);
				break;
			case 'current' :
				// updating existing running workflows
				$db = &$GLOBALS['SQ_SYSTEM']->db;
				foreach ($this->_current_workflows['running'] as $workflowid => $workflow) {
					$steps = serialize($workflow['steps']);
					$sql = 'UPDATE sq_asset_workflow
							SET complete = 0, workflow = '.$db->quote($steps).'
							WHERE workflowid = '.$db->quote($workflowid);

					$result = $db->query($sql);
					if (DB::isError($result)) {
						trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
						return false;
					}
				}
				break;
		}
	}


	function addStep()
	{
		$stepid = count($this->_steps) + 1;

		$this->_steps[$stepid] = Array();
		$this->_steps[$stepid]['logic'] = '>= 1';
		$this->_steps[$stepid]['conditions'] = Array();

		return $stepid;
	}


	function deleteStep($stepid)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->_steps[$stepid])) {
			trigger_error("Cannot delete workflow step, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		unset($this->_steps[$stepid]);

		// move all the other steps up one position
		$new_steps = Array();
		foreach ($this->_steps as $step => $step_data) {
			if ($step < $stepid) $new_stepid = $step;
			else $new_stepid = $step -1;
			$new_steps[$new_stepid] = $step_data;
		}
		$this->_steps = $new_steps;

		return true;
	}


	function setStepLogic($stepid, $logic='>=', $logic_num=1)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->_steps[$stepid])) {
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
		$this->_steps[$stepid]['logic'] = $logic;

		return true;
	}


	function addCondition($stepid, $publisher, $logic='>=', $logic_num=1)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->_steps[$stepid])) {
			trigger_error("Cannot add step condition, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$logic = trim((string)$logic);
		if (!isset($this->_valid_cond_logic[$logic])) {
			trigger_error("Cannot add step condition, '$logic' is not a valid condition logic", E_USER_WARNING);
			return false;
		}

		$publisher = (int)$publisher;
		if (isset($this->_steps[$stepid]['conditions'][$publisher])) {
			trigger_error("Cannot add step condition, publisher $publisher is already set in step $stepid", E_USER_WARNING);
			return false;
		}

		// work out the logic expression
		$logic_num = (int)$logic_num;
		$require = $logic.(($logic_num) ? " $logic_num" : '');
		$this->_steps[$stepid]['conditions'][$publisher] = Array('require' => $require, 'published_by' => Array());
	
		return true;
	}


	function setConditionLogic($stepid, $publisher, $logic='>=', $logic_num=1)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->_steps[$stepid])) {
			trigger_error("Cannot edit step condition logic, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$logic = trim((string)$logic);
		if (!isset($this->_valid_cond_logic[$logic])) {
			trigger_error("Cannot edit step condition logic, '$logic' is not a valid condition logic", E_USER_WARNING);
			return false;
		}

		$publisher = (int)$publisher;
		if (!isset($this->_steps[$stepid]['conditions'][$publisher])) {
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
		$this->_steps[$stepid]['conditions'][$publisher]['require'] = $require;
	
		return true;
	}


	function deleteCondition($stepid, $publisher)
	{
		$stepid = (int)$stepid;
		if (!$stepid) return false;

		if (!isset($this->_steps[$stepid])) {
			trigger_error("Cannot delete step condition, step $stepid does not exist", E_USER_WARNING);
			return false;
		}

		$publisher = (int)$publisher;
		if (!isset($this->_steps[$stepid]['conditions'][$publisher])) {
			trigger_error("Cannot delete step condition, publisher $publisher does not exist in step $stepid", E_USER_WARNING);
			return false;
		}

		unset($this->_steps[$stepid]['conditions'][$publisher]);
	
		return true;
	}


}//end class
?>
