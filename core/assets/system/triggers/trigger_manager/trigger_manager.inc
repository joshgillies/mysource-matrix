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
* $Id: trigger_manager.inc,v 1.3 2005/02/18 07:30:28 amiller Exp $
*
*/

// determines if tree_location refers to all assets under itself, or itself only
define ('SQ_TRIG_TREE_PROPAGATE',   '01');
define ('SQ_TRIG_TREE_NO_PROPAGATE','00');

// trigger status
define ('SQ_TRIG_STATUS_ACTIVE',    '1');
define ('SQ_TRIG_STATUS_INACTIVE',  '0');
define ('SQ_TRIG_STATUS_DEFAULT',   SQ_TRIG_STATUS_INACTIVE);

// used to determine if a trigger has failed, and if so, how
define ('SQ_TRIG_RESULT_SUCCESS', 1);
define ('SQ_TRIG_RESULT_FALSE',  -1);
define ('SQ_TRIG_RESULT_INVALID',-2);
define ('SQ_TRIG_RESULT_FAILURE', 0);


require_once 'hash.inc';

/**
* Trigger_Manager
*
* Purpose
*    Manages the triggering system
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Manager extends Asset
{

//--        MATRIX REQUIREMENTS        --//

	// Constructor
	function Trigger_Manager($assetid=0)
	{
		$this->Asset($assetid);

		// include base classes for conditions, actions, events
		$this->_loadComponent('trigger_condition');
		$this->_loadComponent('trigger_action');
		$this->_loadComponent('trigger_event');

	}//end Trigger_Manager()


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
			SQ_LINK_TYPE_1 => Array(),
			SQ_LINK_TYPE_2 => Array(),
			SQ_LINK_TYPE_3 => Array(),
			SQ_LINK_NOTICE => Array(),
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
	* @param bool	$short_name	whether or not we are after the shortname or the full name
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
	* @param string		$event_name		The name of the event to broadcast
	* @param Asset		&$broadcaster	The firing asset
	*
	* @access public
	* @return boolean
	*/
	function broadcastEvent($event_name, &$broadcaster, $parameters=null)
	{
		if (!$this->attr('enabled')) {
			return true;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// include the event asset
		if (!$this->_loadComponent($event_name)) {
			return false;
		}


		// calculate state, prepare state hash
		$state = $this->_calculateState($event_name, $broadcaster, $parameters);
		$state_hash = $this->_calculateStateHash($state);

		// lookup triggers
		$raw_triggers = $this->_lookupCandidateTriggers($state_hash);

		// execute each trigger
		foreach ($raw_triggers as $trigger) {
			// re-create a trigger
			$trigger_data = $this->_db2trigger($trigger);
			$status = $this->_executeTrigger($trigger_data['data'], $state);

		}

		return true;

	}//end broadcastEvent()


	/**
	* Returns a state calculated from the event.
	*
	* @param string		$event_type		The name of the event to calculate the state from
	* @param Asset		&$broadcaster	The firing asset
	*
	* @access private
	* @return Array(mixed)
	*/
	function _calculateState($event_type, &$event_broadcaster, $parameters=null)
	{
		// calculate the environment state
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// for use by the trigger
		$state['event']['name']	= $event_type;
		$state['event']['data']	= $parameters;

		$state['asset']			= &$event_broadcaster;
		$state['assetid'] 		= $event_broadcaster->id;
		$state['asset_type'] 	= $event_broadcaster->type();

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
		$tmp_treeid = $am->getRawTreeIdByAsset($state['assetid'], true);
		if ($tmp_treeid) {
			$state['treeid'] = $tmp_treeid;
		} else {
			$state['treeid'] = '';
		}

		return $state;

	}//end _calculateState()


	/**
	* Returns a state_hash calculated from the state.
	*
	* @param Array(mixed)	&$state		The state to calculate the hash from
	*
	* @access private
	* @return Array(mixed)
	*/
	function _calculateStateHash(&$state)
	{
		$state_hash['event'] 		= $state['event']['name'];
		$state_hash['assetid'] 		= $state['assetid'];
		$state_hash['asset_type'] 	= $state['asset_type'];
		$state_hash['treeid'] 		= $state['treeid'];
		$state_hash['parents'] 		= $state['immediate_parents'];
		$state_hash['children'] 	= $state['immediate_children'];

		// assume active triggers only
		$state_hash['active'] = SQ_TRIG_STATUS_ACTIVE;

		return $state_hash;

	}//end _calculateStateHash()


	/**
	* Lookup the State hash table for all triggers that might need executing
	*
	* @param Array(mixed)	$state_hash		The hash used to check the database with
	*
	* @access private
	* @return Array(mixed)	Trigger data (array of arrays)
	*/
	function _lookupCandidateTriggers($state_hash=null)
	{
		// if no state_hash, then fail
		if (empty($state_hash)) {
			return Array();
		}

		// obtain hash
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$PROPAGATE 		= $db->quoteSmart(SQ_TRIG_TREE_PROPAGATE);
		$NO_PROPAGATE 	= $db->quoteSmart(SQ_TRIG_TREE_NO_PROPAGATE);

		// compose the link_tree SQL block
		$treeid_sql_array[] = "treeid = ''";
		if (empty($state_hash['treeid'])) {
			$state_hash['treeid'] = Array();
		}

		foreach ($state_hash['treeid'] as $treeid) {
			if (empty($treeid)) {
				continue;
			}

			$treeid = $db->quoteSmart($treeid);

			$treeid_sql_array[] = '
				(
					tree_propagate = '.$PROPAGATE.'
					AND
					(
						treeid != '.$treeid.' AND
						SUBSTR('.$treeid.', 1, LENGTH(treeid)) = treeid
					)
				)
				OR
				(
					tree_propagate = '.$NO_PROPAGATE.' AND
					treeid = '.$treeid.'
				)
			';

		}
		$treeid_sql = implode($treeid_sql_array, ' OR ');

		// parent hash
		$parent_sql_array[] = '(parentid = \'\' AND parent_type = \'\')';

		$parents = $state_hash['parents'];
		foreach ($parents as $id => $type) {
			$id = $db->quoteSmart($id);
			$type = $db->quoteSmart($type);
			$parent_sql_array[] = '(parentid = \'\' AND parent_type = '.$type.')';

			$parent_sql_array[] = '(parentid = '.$id.' AND parent_type = \'\')';
		}
		$parent_sql = implode($parent_sql_array, ' OR ');

		// child hash
		$child_sql_array[] = '(childid = \'\' AND child_type = \'\')';

		$children = $state_hash['children'];
		foreach ($children as $id => $type) {
			$id   = $db->quoteSmart($id);
			$type = $db->quoteSmart($type);
			$child_sql_array[] = '( childid = \'\' AND child_type = '.$type.')';

			$child_sql_array[] = '(childid = '.$id.' AND child_type = \'\')';
		}
		$child_sql = implode($child_sql_array, ' OR ');

		// compose query
		$sql ='
			SELECT
				id,
				name,
				description,
				data,
				active
			FROM
				!trig
			WHERE
				"id" IN (
					SELECT DISTINCT
						triggerid
					FROM
						!trig_hash
					WHERE
						(active = ?)
					AND
						(
							event = ?		OR
							event = \'\'
						)
					AND
						(
							assetid = ?		OR
							assetid = \'\'
						)
					AND
						(
							asset_type = ?	OR
							asset_type = \'\'
						)
					AND
						(
							!
						)
					AND
						(
							!
						)
					AND
						(
							!
						)
				)
			AND
				"active" = ?
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$state_hash['active'],
			$state_hash['event'],
			$state_hash['assetid'],
			$state_hash['asset_type'],
			$treeid_sql,
			$parent_sql,
			$child_sql,
			$state_hash['active'],
		);

		// query the database
		$result = $db->getAll($sql, $data);

		// check result
		assert_valid_db_result($result);

		return $result;

	}//end _lookupCandidateTriggers()


	/**
	* Execute one trigger, using the passed trigger data and state info.
	*
	* @param Array(mixed)	$trigger_data	The details (id, name, etc.) and data (actions, conditions) associated with the trigger
	* @param Array(mixed)	$state_data		Data relating to the asset the trigger is running on
	*
	* @access private
	* @return boolean		Status
	*/
	function _executeTrigger($trigger_data, &$state_data)
	{
		$status = SQ_TRIG_RESULT_SUCCESS;


		$conditions     = $trigger_data['conditions'];
		$actions        = $trigger_data['actions'];

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// check each condition
		foreach ($conditions as $condition) {

			// include condition code
			if (!$this->_loadComponent($condition['type'])) {
				return SQ_TRIG_RESULT_INVALID;
			}

			// check condition
			// dynamic substitution of static condition class
			eval('$condition_success = '.$condition['type'].'::evaluate($condition[\'data\'], $state_data);');


			// if condition fails return with Success
			// (trigger does not need execution, false alarm but trigger did not fail)
			if (!$condition_success) {
				return SQ_TRIG_RESULT_FALSE;
			}
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

				$previous_install_status = array_get_index($GLOBALS, 'SQ_INSTALL', false);
				$GLOBALS['SQ_INSTALL'] = true;

				// execute action
				eval('$action_success = '.$action['type'].'::execute($action["data"], $state_data);');

				if (!$previous_install_status) {
					unset($GLOBALS['SQ_INSTALL']);
				}

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

			if (!$action_success) {
				if (!$not_required) {
					continue;
				} else {
					$status = SQ_TRIG_RESULT_FAILURE;
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
	* @return Array(string=>mixed)
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
	* @param Array(string=>mixed)	$trigger	The trigger to be saved
	*
	* @access private
	* @return int|false		trigger id or false
	*/
	function _saveTrigger($trigger)
	{
		if (empty($trigger)) {
			return false;
		}

		// start transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// check if trigger exists
		$tr_id = array_get_index($trigger, 'id', null);
		// this bit gets all ids to come from the sequence, manual entries will make db inconsistent
		if (is_null($tr_id)) {
			$tr_id = $db->nextId(SQ_TABLE_PREFIX.'trig_id');
			assert_valid_db_result($tr_id);

			$trigger['id'] = $tr_id;
		} else {
			// remove old trigger
			$this->_deleteTrigger($tr_id);
		}

		$status = true;

		// prepare and save the hash
		// the edit interface can set the optional 'save_hash' index to false, which will cause
		// the hash not to be saved for this trigger, for whatever reason
		if (array_get_index($trigger, 'save_hash', true)) {
			if (empty($trigger['state_hash'])) {
				$trigger['state_hash'] = $this->_getHashObject();
			}

			$trigger['state_hash']->setTriggerId($trigger['id']);

			// prepare hash combinations and update the hash table
			$hash_combinations = $trigger['state_hash']->getHashCombinations();
			$status = $status &&  $this->_saveStateHash($hash_combinations);
		}

		// now try to save the trigger itself
		if ($status && $this->_saveTriggerData($trigger)) {
			// commit
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return $trigger['id'];
		} else {
			// rollback
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end _saveTrigger()


	/**
	* Saves a trigger in the database
	*
	* @param Array(string=>mixed)	$trigger	The trigger to be saved
	*
	* @access private
	* @return boolean		Status
	*/
	function _saveTriggerData($trigger)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// save (new) trigger
		$sql = 'INSERT INTO !trig ("id", "name", "description", "data", "active") VALUES (?,?,?,?,?)';
		$trigger_db = $this->_trigger2db($trigger);
		$data = Array(
			SQ_TABLE_PREFIX,
			$trigger_db['id'],
			$trigger_db['name'],
			$trigger_db['description'],
			$trigger_db['data'],
			$trigger_db['active'],
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);
		assert_valid_db_result($result);

		return true;

	}//end _saveTriggerData()


	/**
	* Saves the state hash in the database
	*
	* @param Array(mixed)	$hash_combinations	The flattened hash to be saved
	*
	* @access private
	* @return boolean		Status
	*/
	function _saveStateHash($hash_combinations)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if (empty($hash_combinations)) {
			return false;
		}

		foreach ($hash_combinations as $combination) {

			$field_array 		= Array();
			$value_array		= Array();

			foreach ($combination as $hash_id => $hash_data) {
				$field_array[] = $db->quoteIdentifier($hash_id);
				$value_array[] = $db->quoteSmart($hash_data);
			}

			$field_string = implode($field_array, ', ');
			$value_string = implode($value_array, ', ');

			$sql = '
				INSERT INTO
					!trig_hash
					(!)
				VALUES
					(!)
			';

			$data = Array(
				SQ_TABLE_PREFIX,
				$field_string,
				$value_string,
			);

			$result = $db->query($sql, $data);

			// check result
			if (!assert_valid_db_result($result, null, false, false)) {
				return false;
			}

		}

		return true;

	}//end _saveStateHash()


	/**
	* Formats the state_hash
	*
	* @param Array(mixed)	$state_hash		The state_hash to check
	*
	* @access private
	* @return Array(mixed)	$state_hash		The formatted state_hash
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
				'active'			=> SQ_TRIG_STATUS_ACTIVE
		);

		// make sure every field has data, and that no fields are null
		foreach ($state_hash_defaults as $defaults_id => $defaults_data) {
			if (empty($state_hash[$defaults_id])) {
				$state_hash[$defaults_id] = $defaults_data;
			}
		}

		return $state_hash;

	}//end _prepareRawStateHash()


//--        TRIGGER MODIFICATION        --//


	/**
	* Set a trigger's active status
	*
	* @param integer	$triggerid		The trigger to set the status of
	* @param string		$status			'1' or '0': the value to set the status to
	*
	* @access private
	* @return
	*/
	function _setTriggerStatus($triggerid, $status='1')
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			UPDATE !trig SET "active" = ? WHERE "id" = ?;
			UPDATE !trig_hash SET "active" = ? WHERE "triggerid" = ?;
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			$status,
			$triggerid,
			SQ_TABLE_PREFIX,
			$status,
			$triggerid,
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);

		assert_valid_db_result($result);

		return true;

	}//end _setTriggerStatus()


	/**
	* Set the statuses of an array of triggers
	*
	* @param array(int)		$triggerids		The triggers to alter
	* @param char(1)		$status			Enable ('1') or Disable ('0') the trigger
	*
	* @access private
	* @return
	*/
	function _setTriggerStatuses($triggerids, $status='1')
	{
		if (empty($triggerids) || !($status == '0' || $status == '1') ) {
			return false;
		}

		if (!is_array($triggerids)) {
			$triggerids = Array($triggerids);
		}

		$result = true;
		foreach ($triggerids as $trigger) {
			$result = $this->_setTriggerStatus($trigger, $status);
			if (!$result) {
				break;
			}
		}

		return $result;

	}//end _setTriggerStatuses()


	/**
	* Enable a trigger
	*
	* @param integer	$id		the ID of the trigger to enable
	*
	* @access public
	* @return
	*/
	function enableTrigger($id)
	{
		$this->_setTriggerStatus($id, '1');

	}//end enableTrigger()


	/**
	* Disable a trigger
	*
	* @param integer	$id		the ID of the trigger to disable
	*
	* @access public
	* @return
	*/
	function disableTrigger($id)
	{
		$this->_setTriggerStatus($id, '0');

	}//end disableTrigger()


//--        TRIGGER DELETION        --//


	/**
	* Deletes the trigger from trigger table and clears the hash entries
	*
	* @param int $trigger_id    id of trigger to be deleted
	*
	* @access private
	* @return boolean	Status
	*/
	function _deleteTrigger($trigger_id)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			DELETE FROM !trig WHERE "id" = ?;
			DELETE FROM !trig_hash WHERE "triggerid" = ?;
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			$trigger_id,
			SQ_TABLE_PREFIX,
			$trigger_id,
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);

		assert_valid_db_result($result);

		return true;

	}//end _deleteTrigger()


//--        TRIGGER/DB CONVERSION        --//


	/**
	* Serialise a trigger in preparation for storing it in the database
	*
	* @param Array(mixed)	$trigger	the trigger to store
	*
	* @access private
	* @return string
	*/
	function _trigger2db($trigger)
	{
		$trigger['data'] = serialize($trigger['data']);
		$trigger['active'] = array_get_index($trigger,'active', SQ_TRIG_STATUS_DEFAULT);
		return $trigger;

	}//end _trigger2db()


	/**
	* Unserialise a trigger retrieved from the database
	*
	* @param Array(mixed)	$trigger	the trigger to unserialise
	*
	* @access private
	* @return Array(mi
	*/
	function _db2trigger($trigger)
	{
		$trigger['data'] = isset($trigger['data'])?unserialize($trigger['data']):'';
		return $trigger;

	}//end _db2trigger()


//--        GETTERS/SETTERS, LOAD FUNCTIONS        --//


	/**
	* Obtain a count of triggers
	*
	* @access private
	* @return integer	Count
	*/
	function _countTriggers()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = "SELECT count(*) FROM !trig";
		$data = Array(
			SQ_TABLE_PREFIX,
		);

		$result = $db->getOne($sql, $data);

		assert_valid_db_result($result);
		return $result;

	}//end _countTriggers()


	/**
	* Load Trigger Data
	*
	* @param integer	$id		the ID of the trigger to load
	*
	* @access private
	* @return array(mixed)
	*/
	function _loadTrigger($id)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// compose sql query
		$sql = 'SELECT * FROM !trig WHERE "id" = ?';
		$data = Array(
			SQ_TABLE_PREFIX,
			$id,
		);

		$result = $db->getAll($sql, $data);
		assert_valid_db_result($result);

		// convert result to trigger data
		if (empty($result)){
			return null;
		}

		$trigger_db = array_pop($result);
		$trigger = $this->_db2trigger($trigger_db);

		return $trigger;

	}//end _loadTrigger()


	/**
	* Obtain a list of trigger info
	*
	* @access private
	* @return array(mixed)	Trigger info
	*/
	function _getTriggerInfoList()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT * FROM !trig order by "active" DESC, "name" ASC';
		$data = Array(
			SQ_TABLE_PREFIX,
		);

		$result = $db->getAll($sql, $data);

		assert_valid_db_result($result);
		return $result;

	}//end _getTriggerInfoList()


	/**
	* Returns a list of descendants of a certain type, along with type info
	*
	* @param string	$type	the type_code to get the descendants of
	*
	* @access private
	* @return array(mixed)
	*/
	function _getComponentList($type=null)
	{
		if (empty($type)) {
			return Array();
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$component_types = $am->getTypeDescendants($type);
		$component_info  = $am->getTypeInfo($component_types, 'name');

		return $component_info;

	}//end _getComponentList()


	/**
	* Get A list of all installed events
	*
	* @access private
	* @return array(mixed)
	*/
	function _getEventList()
	{
		return $this->_getComponentList('trigger_event');

	}//end _getEventList()


	/**
	* Get a list of installed conditions
	*
	* @access private
	* @return array(mixed)
	*/
	function _getConditionList()
	{
		return $this->_getComponentList('trigger_condition');

	}//end _getConditionList()


	/**
	* Get a list of installed actions
	*
	* @access private
	* @return array(mixed)
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
	* if that change is ever made, conversion should be simple
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
	* @param string			$type_code	the type_code of the component to get the interface of
	* @param array(mixed)	$data		the components settings data
	* @param string			$prefix		the temporary unique ID for the component
	*
	* @access private
	* @return string	Interface
	*/
	function _getComponentInterface($type_code, $data, $prefix)
	{
		if (!$this->_loadComponent($type_code)) {
			return '';
		}

		eval('$interface = '.$type_code.'::getInterface($data, $prefix);');

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
