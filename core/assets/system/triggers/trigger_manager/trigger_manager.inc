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
* $Id: trigger_manager.inc,v 1.17 2005/04/27 00:14:43 lwright Exp $
*
*/

// determines if tree_location refers to all assets under itself, or itself only
define('SQ_TRIG_TREE_PROPAGATE',   '01');
define('SQ_TRIG_TREE_NO_PROPAGATE','00');

// trigger status
define('SQ_TRIG_STATUS_ACTIVE',    '1');
define('SQ_TRIG_STATUS_INACTIVE',  '0');
define('SQ_TRIG_STATUS_DEFAULT',   SQ_TRIG_STATUS_INACTIVE);

// used to determine if a trigger has failed, and if so, how
define('SQ_TRIG_RESULT_SUCCESS',  1);
define('SQ_TRIG_RESULT_FALSE',   -1);
define('SQ_TRIG_RESULT_INVALID', -2);
define('SQ_TRIG_RESULT_FAILURE',  0);

define('SQ_TRIG_TABLE_MAIN', 'trig');
define('SQ_TRIG_TABLE_HASH', 'trig_hash');


require_once 'hash.inc';

/**
* Trigger_Manager
*
* Purpose
*    Manages the triggering system
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Trigger_Manager($assetid=0)
	{
		$this->Asset($assetid);

		// include base classes for conditions, actions, events
		$this->_loadComponent('trigger_condition');
		$this->_loadComponent('trigger_action');
		$this->_loadComponent('trigger_event');

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return int|false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Return an array of all the permitted link types, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Return FALSE always because we don't want to be able to clone a trigger manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Returns the name of a component (an action or a condition)
	*
	* @param string	$type	component type_code
	*
	* @access private
	* @return string
	*/
	function _getComponentName($type)
	{
		$name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
		return $name;

	}//end _getComponentName()


	/**
	* Returns the name of a component (an action or a condition)
	*
	* @param string	$type	component type_code
	*
	* @access private
	* @return string
	*/
	function _getComponentDescription($type)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'description');

	}//end _getComponentDescription()


//--        TRIGGER EXECUTION        --//


	/**
	* Broadcasts an event, and run all the triggers associated with it
	*
	* @param string	$event_name		the name of the event to broadcast
	* @param Asset	&$broadcaster	The firing asset
	*
	* @access public
	* @return boolean
	*/
	function broadcastEvent($event_name, &$broadcaster, $parameters=null)
	{
		// triggers do not work in ROLLBACK mode
		if (SQ_ROLLBACK_VIEW) return true;
		if (!$this->attr('enabled')) return true;

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// include the event asset
		if (!$this->_loadComponent($event_name)) return false;

		// calculate state, prepare state hash
		$state = $this->_calculateState($event_name, $broadcaster, $parameters);
		$state_hash = $this->_calculateStateHash($state);

		// lookup triggers
		$candidate_trigger_ids = $this->_lookupCandidateTriggers($state_hash);

		// execute each trigger
		foreach ($candidate_trigger_ids as $trigger_id) {

			$status_message = null;
			$msg_type = null;
			// re-create a trigger
			$trigger = $this->_loadTrigger($trigger_id);
			$status  = $this->_executeTrigger($trigger['data'], $state);

			$msg_reps = Array(
							'trigger_name'	=> $trigger['name'],
							'triggerid'		=> $trigger['id'],
							'event'			=> $event_name,
							'asset_name'	=> $broadcaster->name,
							'assetid'		=> $broadcaster->id,
						);
			switch ($status) {
				case SQ_TRIG_RESULT_FAILURE:
					trigger_localised_error("CORE0127", E_USER_WARNING, $trigger_info_line);
				break;

				case SQ_TRIG_RESULT_SUCCESS:
					$msg_type = 'trigger.success';
				break;

				case SQ_TRIG_RESULT_INVALID:
					$msg_type = 'trigger.invalid';
				break;

				default:
			}

			if (!empty($status_message)) {
				// log message using messaging service
				$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
				
				$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
				$message->send();
			}
		}

		return true;

	}//end broadcastEvent()


	/**
	* Returns a state calculated from the event.
	*
	* @param string	$event_type		the name of the event to calculate the state from
	* @param asset	&$broadcaster	the firing asset
	*
	* @access private
	* @return array
	*/
	function _calculateState($event_type, &$event_broadcaster, $parameters=null)
	{
		// calculate the environment state
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// for use by the trigger
		$state['event']['name'] = $event_type;
		$state['event']['data'] = $parameters;

		$state['asset']         = &$event_broadcaster;
		$state['assetid']       = $event_broadcaster->id;
		$state['asset_type']    = $event_broadcaster->type();

		//// parent links
		$parent_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, null, true, 'minor');
		$new_parent_links = Array();
		$parent_assets    = Array();
		$parent_link_map  = Array();
		foreach ($parent_links as $link) {
			$new_parent_links[$link['linkid']] = $link;
			$parent_assets[$link['majorid']] = $link['major_type_code'];
			$parent_link_map[$link['majorid']][] = $link['linkid'];
		}
		$state['immediate_parent_link_map'] = $parent_link_map;
		$state['immediate_parent_links']    = $new_parent_links;
		$state['immediate_parents']         = $parent_assets;


		//// child links
		$child_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, null, true, 'major');
		$new_child_links = Array();
		$child_assets = Array();
		$child_link_map  = Array();
		foreach ($child_links as $link) {
			$new_child_links[$link['linkid']] = $link;
			$child_assets[$link['minorid']] = $link['minor_type_code'];
			$child_link_map[$link['minorid']][] = $link['linkid'];
		}
		$state['immediate_child_link_map'] = $child_link_map;
		$state['immediate_child_links'] = $new_child_links;
		$state['immediate_children'] = $child_assets;

		// uses type 1,2 and 3 only
		$tmp_treeid = $am->getAssetTreeids($state['assetid']);

		if ($tmp_treeid) $state['treeid'] = $tmp_treeid;

		return $state;

	}//end _calculateState()


	/**
	* Returns a state_hash calculated from the state
	*
	* @param array	&$state	the state to calculate the hash from
	*
	* @access private
	* @return array
	*/
	function _calculateStateHash(&$state)
	{
		$state_hash['event']        = $state['event']['name'];
		$state_hash['assetid']      = $state['assetid'];
		$state_hash['asset_type']   = $state['asset_type'];
		$state_hash['treeid']       = $state['treeid'];
		$state_hash['parents']      = $state['immediate_parents'];
		$state_hash['children']     = $state['immediate_children'];

		return $state_hash;

	}//end _calculateStateHash()


	/**
	* Lookup the State hash table for all triggers that might need executing
	*
	* @param Array(mixed)   $state_hash     The hash used to check the database with
	*
	* @access private
	* @return Array(mixed)  Trigger data (array of arrays)
	*/
	function _lookupCandidateTriggers($state_hash=null)
	{
		// if no state_hash, then fail
		if (empty($state_hash)) return Array();
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$PROPAGATE      = $db->quoteSmart(SQ_TRIG_TREE_PROPAGATE);
		$NO_PROPAGATE   = $db->quoteSmart(SQ_TRIG_TREE_NO_PROPAGATE);

		// compose the link_tree SQL block
		$treeid_sql_array[] = "treeid IS NULL";
		if (empty($state_hash['treeid'])) {
			$state_hash['treeid'] = Array();
		}

		foreach ($state_hash['treeid'] as $current_treeid) {
			if (empty($current_treeid)) continue;

			$tree_length = strlen($current_treeid) - SQ_CONF_ASSET_TREE_SIZE;
			$tree_length = $db->quoteSmart($tree_length);
			$current_treeid = $db->quoteSmart($current_treeid);

			$treeid_sql_array[] = '
						(
							(treeid != '.$current_treeid.' AND SUBSTR('.$current_treeid.', 1, LENGTH(treeid)) = treeid)
							AND (
								tree_propagate = '.$PROPAGATE.'
								OR (tree_propagate = '.$NO_PROPAGATE.' AND LENGTH(treeid) = '.$tree_length.')
							)
						)';

		}
		$treeid_sql = implode($treeid_sql_array, " OR ");


		//// PARENTS
		$parent_id_array = Array();
		$parent_type_array = Array();

		$parents = $state_hash['parents'];
		foreach ($parents as $id => $type) {
			$parent_id_array[]   = $db->quoteSmart($id);
			$parent_type_array[$type] = $db->quoteSmart($type);
		}

		// parent IDs
		$parent_id_sql = 'parentid IS NULL';
		if (!empty($parent_id_array)) {
			$parent_id_sql .= ' OR parentid IN ('.implode(',', $parent_id_array).')';
		}

		// parent types
		$parent_type_sql = 'parent_type IS NULL';
		if (!empty($parent_type_array)) {
			$parent_type_sql .= ' OR parent_type IN ('.implode(',', $parent_type_array).')';
		}


		//// CHILDREN
		$child_id_array = Array();
		$child_type_array = Array();

		$children = $state_hash['children'];
		foreach ($children as $id => $type) {
			$child_id_array[]   = $db->quoteSmart($id);
			$child_type_array[$type] = $db->quoteSmart($type);
		}

		// child IDs
		$child_id_sql = 'childid IS NULL';
		if (!empty($child_id_array)) {
			$child_id_sql .= ' OR childid IN ('.implode(',', $child_id_array).')';
		}

		// child types
		$child_type_sql = 'child_type IS NULL';
		if (!empty($child_type_array)) {
			$child_type_sql .= ' OR child_type IN ('.implode(',', $child_type_array).')';
		}


		// event
		$event_sql = 'event IS NULL';
		if (!is_null($state_hash['event'])) {
			$event_sql .= ' OR event = '.$db->quoteSmart($state_hash['event']);
		}

		// broadcaster's asset ID
		$assetid_sql = 'assetid IS NULL';
		if (!is_null($state_hash['assetid'])) {
			$assetid_sql .= ' OR assetid = '.$db->quoteSmart($state_hash['assetid']);
		}

		// broadcaster's asset Type
		$asset_type_sql = 'asset_type IS NULL';
		if (!is_null($state_hash['asset_type'])) {
			$asset_type_sql .= ' OR asset_type = '.$db->quoteSmart($state_hash['asset_type']);
		}

		// compose query
		$sql = 'SELECT DISTINCT triggerid
				FROM sq_trig_hash
				WHERE   ('.$event_sql.')
					AND ('.$assetid_sql.')
					AND ('.$asset_type_sql.')
					AND ('.$treeid_sql.')
					AND ('.$parent_id_sql.')
					AND ('.$parent_type_sql.')
					AND ('.$child_id_sql.')
					AND ('.$child_type_sql.')';

		// query the database
		$result = $db->getCol($sql);

		// check result
		assert_valid_db_result($result);

		return $result;

	}//end _lookupCandidateTriggers()


	/**
	* Execute one trigger, using the passed trigger data and state info
	*
	* Returns the status of the executed trigger
	*
	* @param array	$trigger_data	details (id, name, etc.) and data (actions, conditions)
	* 								associated with the trigger
	* @param array	$state_data		aata relating to the asset the trigger is running on
	*
	* @access private
	* @return boolean
	*/
	function _executeTrigger($trigger_data, &$state_data)
	{
		$status = SQ_TRIG_RESULT_SUCCESS;

		$conditions = $trigger_data['conditions'];
		$actions    = $trigger_data['actions'];

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// check each condition
		foreach ($conditions as $condition) {

			// include condition code
			if (!$this->_loadComponent($condition['type'])) {
				return SQ_TRIG_RESULT_INVALID;
			}

			// check condition
			// dynamic substitution of static condition class
			eval('$condition_success = '.$condition['type'].'::evaluate($condition["data"], $state_data);');

			// if condition fails return with Success
			// (trigger does not need execution, false alarm but trigger did not fail)
			if (!$condition_success) return SQ_TRIG_RESULT_FALSE;
		}


		// all conditions pass. now perform the actions.
		// NOTE: this is not thread-safe
		// for thread safety all the conditions and actions need to be a part of one transaction
		// we rely on the fact that this function will be wrappend in a transaction

		// this relies on the state remaining constant throughout trigger execution
		// this assumption will be the first to blame for unpredictable behaviour

		// include action code in one go, in case one of the middle actions are missing
		foreach ($actions as $action) {
			if (!$this->_loadComponent($action['type'])) {
				return SQ_TRIG_RESULT_INVALID;
			}
		}

		// init locks - this will be used to store locks to release after executing all the actions
		$locks_held = Array();

		// perform actions
		foreach ($actions as $action) {
			$not_required = array_get_index($action, 'not_required', false);
			$ignore_permissions = array_get_index($action, 'ignore_permissions', false);

			if ($ignore_permissions) {

				// execute action
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				eval('$action_success = '.$action['type'].'::execute($action["data"], $state_data);');
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			} else {
				$lock_success = true;
				// acquire locks
				// dynamic substitution of static action class
				eval('$action_locks = '.$action['type'].'::getLocks($action["data"], $state_data);');

				foreach ($action_locks as $lock_assetid => $lock_types) {
					foreach ($lock_types as $lock_type) {
						$lock_success = $am->acquireLock($lock_assetid, $lock_type);
						if ($lock_success) {
							$locks_held[$lock_assetid][$lock_type] = true;
						} else {
							break 2;
						}
					}
				}

				if (!$lock_success) {
					if (!$not_required) {
						continue; // next action, please
					} else {
						$status = SQ_TRIG_RESULT_FAILURE;
						break;
					}
				}

				// execute action
				eval('$action_success = '.$action['type'].'::execute($action["data"], $state_data);');
			}

			if ($action_success === false) {
				if (!$not_required) {
					trigger_localised_error("CORE0134", E_USER_NOTICE, $action['type']);
					continue;
				} else {
					$status = SQ_TRIG_RESULT_FAILURE;
					trigger_localised_error("CORE1035", E_USER_NOTICE, $action['type']);
					break;
				}
			}

			$status = SQ_TRIG_RESULT_SUCCESS;

		}

		// release the locks
		foreach ($locks_held as $lock_assetid => $lock_types) {
			foreach ($lock_types as $lock_type => $lock_true) {
				// can't do anything if locks aren't released, so don't take result
				$am->releaseLock($lock_assetid, $lock_type);
			}
		}

		return $status;

	}//end _executeTrigger()


//--        TRIGGER CREATION        --//


	/**
	* Create and return an instance of hash object
	*
	* @access private
	* @return Hash
	*/
	function _getHashObject()
	{
		return new Hash();

	}//end _getHashObject()


	/**
	* Return a blank trigger in preparation for editing
	*
	* @access private
	* @return array
	*/
	function _getEmptyTrigger()
	{
		$trigger['name'] = '';
		$trigger['description'] = '';
		$trigger['data'] = null;
		$trigger['active'] = SQ_TRIG_STATUS_INACTIVE;
		$trigger['state_hash'] = $this->_getHashObject();

		return $trigger;

	}//end _getEmptyTrigger()


	/**
	* Prepare the trigger and state hash for saving, and ship them off to be saved
	*
	* Returns trigger ID on success or FALSE on failure
	*
	* @param array	$trigger	the trigger to be saved
	*
	* @access private
	* @return int | FALSE
	*/
	function _saveTrigger($trigger)
	{
		// triggers do not work in ROLLBACK mode
		if (SQ_ROLLBACK_VIEW) return true;
		if (empty($trigger)) return false;

		// start transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$status = true;
		$new    = false;

		// lack of id means it is a new trigger
		// this bit gets all ids to come from the sequence, manual entries will make db inconsistent
		if (!isset($trigger['id'])) {
			$new = true;
			$tr_id = $db->nextId('sq_trig_id');
			assert_valid_db_result($tr_id);
			$trigger['id'] = $tr_id;
		}

		if ($new) {

			$sql = 'INSERT INTO
						sq_trig
						(
							id,
							data,
							active,
							description,
							name
						)
						VALUES
						(
							?,
							?,
							?,
							?,
							?
						)';

			$prepared = $db->prepare($sql);
			assert_valid_db_result($prepared);

			$values = Array(
						$trigger['id'],
						serialize($trigger['data']),
						array_get_index($trigger,'active', SQ_TRIG_STATUS_DEFAULT),
						$trigger['description'],
						$trigger['name'],
					  );

			$result = $db->execute($prepared, $values);
			assert_valid_db_result($result);

		} else {

			$sql = 'UPDATE
						sq_trig
					SET
						data		= ?,
						active		= ?,
						description	= ?,
						name		= ?
					WHERE
						id = ?';

			$prepared = $db->prepare($sql);
			assert_valid_db_result($prepared);

			$values = Array(
						serialize($trigger['data']),
						array_get_index($trigger,'active', SQ_TRIG_STATUS_DEFAULT),
						$trigger['description'],
						$trigger['name'],
						$trigger['id'],
					  );

			$result = $db->execute($prepared, $values);
			assert_valid_db_result($result);
		}

		$this->_deleteTriggerHash($trigger['id']);

		// prepare and save the hash
		// the edit interface can set the optional 'save_hash' index to false, which will cause
		// the hash not to be saved for this trigger, for whatever reason
		if ($status && array_get_index($trigger, 'save_hash', true) && $trigger['active']) {
			$hash = array_get_index($trigger, 'state_hash');

			if (empty($hash)) $hash = $this->_getHashObject();
			$hash->setTriggerId($trigger['id']);
			$status = $status && $this->_saveTriggerHash($hash);
		}

		// now try to save the trigger itself
		if ($status) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return $trigger['id'];
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end _saveTrigger()


	/**
	* Saves the trigger hash in the database
	*
	* @param Object Hash	$hash	the Hash object
	*
	* @access private
	* @return boolean
	*/
	function _saveTriggerHash($hash=null)
	{
		if (empty($hash)) return false;

		$hash_combinations = $hash->getHashCombinations();

		if (empty($hash_combinations)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		foreach ($hash_combinations as $combination) {

			$field_array = Array();
			$value_array = Array();

			foreach ($combination as $hash_id => $hash_data) {
				$field_array[] = $hash_id;
				$value_array[] = $db->quoteSmart($hash_data);
			}

			$sql = 'INSERT INTO sq_trig_hash
					('.implode($field_array, ', ').')
					VALUES
					('.implode($value_array, ', ').')';

			$result = $db->query($sql);

			// check result
			if (!assert_valid_db_result($result, null, false, false)) {
				return false;
			}

		}//end foreach

		return true;

	}//end _saveTriggerHash()


	/**
	* Format and return the state_hash
	*
	* @param array	$state_hash	the state_hash to check
	*
	* @access private
	* @return array
	*/
	function _prepareRawStateHash($state_hash)
	{
		// if the state_hash is empty or not an array, make it an array so we
		// can set defaults later
		if (empty($state_hash) || !is_array($state_hash)) {
			$state_hash = Array();
		}

		// define the defaults
		$state_hash_defaults = Array(
								'event'				=> '',
								'assetid'			=> '',
								'asset_type'		=> '',
								'treeid'			=> '',
								'tree_propagate'	=> SQ_TRIG_TREE_NO_PROPAGATE,
								'triggerid'			=> '',
							   );

		// make sure every field has data, and that no fields are null
		foreach ($state_hash_defaults as $defaults_id => $defaults_data) {
			if (empty($state_hash[$defaults_id])) {
				$state_hash[$defaults_id] = $defaults_data;
			}
		}

		return $state_hash;

	}//end _prepareRawStateHash()


//--        TRIGGER DELETION        --//


	/**
	* Deletes the trigger from trigger table and clears the hash entries
	*
	* @param int	$trigger_id	id of trigger to be deleted
	*
	* @access private
	* @return boolean
	*/
	function _deleteTrigger($trigger_id)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM
					sq_trig
				WHERE
					id = '.$db->quoteSmart($trigger_id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$success = $this->_deleteTriggerHash($trigger_id);

		return $success;

	}//end _deleteTrigger()


	/**
	* Removes all hash entries for a particular trigger ID
	*
	* @param int	$trigger_id	id of trigger whose hash is be deleted
	*
	* @access private
	* @return boolean
	*/
	function _deleteTriggerHash($trigger_id)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM
					sq_trig_hash
				WHERE
					triggerid = '.$db->quoteSmart($trigger_id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		return true;

	}//end _deleteTriggerHash()


//--        GETTERS/SETTERS, LOAD FUNCTIONS        --//


	/**
	* Load Trigger Data
	*
	* @param integer	$id	the ID of the trigger to load
	*
	* @access private
	* @return array
	*/
	function _loadTrigger($trigger_id=null)
	{
		if (!isset($trigger_id)) return null;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$select = 'SELECT
					id,
					name,
					data,
					description,
					active
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'trig';

		$where  = 'id = '.$db->quoteSmart($trigger_id);
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$query = $select.' '.$where;
		$trigger_db = $db->getRow($query);
		assert_valid_db_result($trigger_db);

		if (empty($trigger_db)) {
			if (!$mute_errors) {
				trigger_localised_error("CORE0133", E_USER_WARNING, $trigger_id);
			}

			return null;
		}

		$trigger_db['data'] = (isset($trigger_db['data']) && !empty($trigger_db['data'])) ? unserialize($trigger_db['data']) : '';

		return $trigger_db;

	}//end _loadTrigger()


	/**
	* Obtain a list of trigger info
	*
	* @access private
	* @return array
	*/
	function _getTriggerInfoList()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$select = 'SELECT
						id,
						name,
						description,
						active
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'trig';

		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause();
		$order  = 'ORDER BY active DESC , name ASC';

		$result = $db->getAll($select.' '.$where.' '.$order);

		assert_valid_db_result($result);

		return $result;

	}//end _getTriggerInfoList()


	/**
	* Returns a list of descendants of a certain type, along with type info
	*
	* @param string	$type	the type_code to get the descendants of
	*
	* @access private
	* @return array
	*/
	function _getComponentList($type=null)
	{
		if (empty($type)) return Array();

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$component_types = $am->getTypeDescendants($type);
		$component_info  = $am->getTypeInfo($component_types, 'name');

		return $component_info;

	}//end _getComponentList()


	/**
	* Get A list of all installed events
	*
	* @access private
	* @return array
	*/
	function _getEventList()
	{
		return $this->_getComponentList('trigger_event');

	}//end _getEventList()


	/**
	* Get a list of installed conditions
	*
	* @access private
	* @return array
	*/
	function _getConditionList()
	{
		return $this->_getComponentList('trigger_condition');

	}//end _getConditionList()


	/**
	* Get a list of installed actions
	*
	* @access private
	* @return array
	*/
	function _getActionList()
	{
		return $this->_getComponentList('trigger_action');

	}//end _getActionList()


	/**
	* Load a component
	*
	* NOTE: logic of this function can be pushed onto asset manager
	* includeAsset could be modified to not throw an exception and return status
	* if that change is ever made, conversion should be simple. Currently asset
	* manager makes an assertion, which kills the system when trying to include
	* something that is not installed
	*
	* @param string	$type_code	the type_code of the component to load
	*/
	function _loadComponent($type_code)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			return true;
		} else {
			return false;
		}

	}//end _loadComponent()


	/**
	* Returns a string containing the buffered output of a component's edit interface
	*
	* @param string	$type_code	the type_code of the component to get the interface of
	* @param array	$data		the components settings data
	* @param string	$prefix		the temporary unique ID for the component
	*
	* @access private
	* @return string
	*/
	function _getComponentInterface($type_code, $data, $prefix, $write_access=false)
	{
		if (!$this->_loadComponent($type_code)) return '';

		eval('$interface = '.$type_code.'::getInterface($data, $prefix, $write_access);');

		return $interface;

	}//end _getComponentInterface()


	/**
	* Determines if more than one instance of a condition is allowed in the one trigger
	*
	* @param string	$type_code	the type_code of the condition to check
	*
	* @access private
	* @return boolean
	*/
	function _isMultipleConditionAllowed($type_code)
	{
		$this->_loadComponent($type_code);
		eval('$status = '.$type_code.'::allowMultiple();');
		return $status;

	}//end _isMultipleConditionAllowed()


}//end class


?>
