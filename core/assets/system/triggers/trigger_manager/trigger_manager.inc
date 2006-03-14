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
* $Id: trigger_manager.inc,v 1.32 2006/03/14 01:32:43 bcaldwell Exp $
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

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'hash.inc';

/**
* Trigger_Manager
*
* Purpose
*    Manages the triggering system
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.32 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @access public
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
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
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
		return FALSE;

	}//end canClone()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Returns the name of a component (an action or a condition)
	*
	* @param string	$type	component type_code
	*
	* @return string
	* @access private
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
	* @return string
	* @access private
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
	* @param object	&$broadcaster	The firing asset
	* @param array	$parameters		extra parameters
	*
	* @return boolean
	* @access public
	*/
	function broadcastEvent($event_name, &$broadcaster, $parameters=NULL)
	{
		// triggers do not work in ROLLBACK mode
		if (SQ_ROLLBACK_VIEW) return TRUE;
		if (!$this->attr('enabled')) return TRUE;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// include the event asset
		if (!$this->_loadComponent($event_name)) {
			trigger_localised_error('CORE0219', E_USER_WARNING, $event_name);
			return FALSE;
		}

		// check whether any triggers listen for this event
		if (!$this->_isEventListenedFor($event_name)) {
			return TRUE;
		}

		// calculate state, prepare state hash
		$state = $this->_calculateState($event_name, $broadcaster, $parameters);
		$state_hash = $this->_calculateStateHash($state);

		$broadcast_status = TRUE;

		// lookup triggers
		$candidate_trigger_ids = $this->_lookupCandidateTriggers($state_hash);

		// execute each trigger
		foreach ($candidate_trigger_ids as $trigger_id) {

			$status_message = NULL;
			$msg_type = NULL;
			// re-create a trigger
			$trigger = $this->_loadTrigger($trigger_id);
			$status  = $this->_executeTrigger($trigger['data'], $state);

			// get settings
			$trigger_settings =& $trigger['data']['settings'];
			$is_blocking      = array_get_index($trigger['data']['settings'], 'blocking', FALSE);

			// prepare message data
			$msg_reps = Array(
							'trigger_name'	=> $trigger['name'],
							'triggerid'		=> $trigger['id'],
							'event'			=> $event_name,
							'asset_name'	=> $broadcaster->name,
							'assetid'		=> $broadcaster->id,
						);

			switch ($status) {
				case SQ_TRIG_RESULT_FAILURE:
					if ($is_blocking) {
						// if the trigger is set to block, then drop out of the
						// loop to prevent the other triggers from running
						// only allowing blocking triggers to affect the broadcast status
						$broadcast_status = FALSE;
						break 2;
					} else {
						trigger_localised_error('CORE0127', E_USER_WARNING, $trigger['id'], $trigger['name'], $event_name, $broadcaster->id, $broadcaster->name);
					}

				break;

				case SQ_TRIG_RESULT_SUCCESS:
					$msg_type = 'trigger.success';
				break;

				case SQ_TRIG_RESULT_INVALID:
					$msg_type = 'trigger.invalid';
				break;

				default:
			}

			if (!empty($msg_type)) {
				// log message using messaging service
				$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();

				$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
				$message->send();
			}

		}//end foreach

		// clear out the record of running triggers
		$this->_tmp['running_triggers'] = Array();

		return $broadcast_status;

	}//end broadcastEvent()


	/**
	* Returns a state calculated from the event.
	*
	* @param string	$event_type			the name of the event to calculate the state from
	* @param object	&$event_broadcaster	the firing asset
	* @param array	$parameters			extra parameters
	*
	* @return array
	* @access private
	*/
	function _calculateState($event_type, &$event_broadcaster, $parameters=NULL)
	{
		// calculate the environment state
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// for use by the trigger
		$state['event']['name'] = $event_type;
		$state['event']['data'] = $parameters;

		$state['asset']         =& $event_broadcaster;
		$state['assetid']       = $event_broadcaster->id;
		$state['asset_type']    = $event_broadcaster->type();

		//// parent links
		$parent_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, NULL, TRUE, 'minor');
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
		$child_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, NULL, TRUE, 'major');
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

		if ($tmp_treeid) {
			$state['treeid'] = $tmp_treeid;
		} else {
			$state['treeid'] = Array();
		}

		return $state;

	}//end _calculateState()


	/**
	* Returns a state_hash calculated from the state
	*
	* @param array	&$state	the state to calculate the hash from
	*
	* @return array
	* @access private
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
	* @param array	$state_hash	The hash used to check the database with
	*
	* @return array
	* @access private
	*/
	function _lookupCandidateTriggers($state_hash=NULL)
	{
		// if no state_hash, then fail
		if (empty($state_hash)) return Array();
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$PROPAGATE      = $db->quoteSmart(SQ_TRIG_TREE_PROPAGATE);
		$NO_PROPAGATE   = $db->quoteSmart(SQ_TRIG_TREE_NO_PROPAGATE);

		// compose the link_tree SQL block
		$treeid_sql_array[] = 'treeid = '.$db->quoteSmart('-1');
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
		$treeid_sql = implode($treeid_sql_array, ' OR ');


		//// PARENTS
		$parent_id_array = Array();
		$parent_type_array = Array();

		$parents = $state_hash['parents'];
		foreach ($parents as $id => $type) {
			$parent_id_array[]   = $db->quoteSmart($id);
			$parent_type_array[$type] = $db->quoteSmart($type);
		}

		// parent IDs
		$parent_id_sql = 'parentid = '.$db->quoteSmart('-1');
		if (!empty($parent_id_array)) {
			$parent_id_sql .= ' OR parentid IN ('.implode(',', $parent_id_array).')';
		}

		// parent types
		$parent_type_sql = 'parent_type = '.$db->quoteSmart('-1');
		if (!empty($parent_type_array)) {
			$parent_type_sql .= ' OR parent_type IN ('.implode(',', $parent_type_array).')';
		}


		//// CHILDREN
		$child_id_sql   = 'childid = '.$db->quoteSmart('-1');
		$child_type_sql = 'child_type = '.$db->quoteSmart('-1');

		if (!empty($state_hash['treeid'])) {
			// children will always be the same, regardless which treeid is used, so just choose the first one off the top
			$treeid        = reset($state_hash['treeid']);
			$quoted_treeid = $db->quoteSmart($treeid);

			// get all the immediate children
			$child_fromwhere_sql = '
				FROM
					(sq_ast_lnk_tree t INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid)
					INNER JOIN sq_ast a ON l.minorid = a.assetid
				WHERE
					t.treeid LIKE '.$db->quoteSmart($treeid.'%').'
					AND t.treeid > '.$quoted_treeid.'
					AND LENGTH(t.treeid) <= LENGTH('.$quoted_treeid.') + 4
			';

			$child_id_sql .= '
				OR childid IN
				(
				SELECT DISTINCT l.minorid
				'.$child_fromwhere_sql.'
				)
			';

			$child_type_sql .= '
				OR child_type IN
				(
				SELECT DISTINCT a.type_code
				'.$child_fromwhere_sql.'
				)
			';
		}//end if

		// event
		$event_sql = 'event = '.$db->quoteSmart('-1');
		if (!is_null($state_hash['event'])) {
			$event_sql .= ' OR event = '.$db->quoteSmart($state_hash['event']);
		}

		// broadcaster's asset ID
		$assetid_sql = 'assetid = '.$db->quoteSmart('-1');
		if (!is_null($state_hash['assetid'])) {
			$assetid_sql .= ' OR assetid = '.$db->quoteSmart($state_hash['assetid']);
		}

		// broadcaster's asset Type
		$asset_type_sql = 'asset_type = '.$db->quoteSmart('-1');
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
	* @param array	$trigger_data	details (id, name, etc.) and data (actions, conditions) associated with the trigger
	* @param array	&$state_data	data relating to the asset the trigger is running on
	*
	* @return boolean
	* @access private
	*/
	function _executeTrigger($trigger_data, &$state_data)
	{
		$status = SQ_TRIG_RESULT_SUCCESS;

		$conditions  = $trigger_data['conditions'];
		$actions     = $trigger_data['actions'];

		if (isset($trigger_data['settings'])) {
			$is_blocking = array_get_index($trigger_data['settings'], 'blocking', FALSE);
		} else {
			$is_blocking = FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// check each condition
		foreach ($conditions as $condition) {

			// include condition code
			if (!$this->_loadComponent($condition['type'])) {
				return SQ_TRIG_RESULT_INVALID;
			}

			// check condition
			// dynamic substitution of static condition class
			$condition_success = call_user_func_array(Array($condition['type'], 'evaluate'), Array($condition['data'], &$state_data));

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
			$not_required = array_get_index($action, 'not_required', FALSE);
			$ignore_permissions = array_get_index($action, 'ignore_permissions', FALSE);

			if ($ignore_permissions || !$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {

				// execute action
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$action_success = call_user_func_array(Array($action['type'], 'execute'), Array($action['data'], &$state_data));
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			} else {
				$lock_success = TRUE;
				// acquire locks
				// dynamic substitution of static action class
				$action_locks = call_user_func_array(Array($action['type'], 'getLocks'), Array($action['data'], &$state_data));

				foreach ($action_locks as $lock_assetid => $lock_types) {
					foreach ($lock_types as $lock_type) {
						$lock_success = $am->acquireLock($lock_assetid, $lock_type);
						if ($lock_success) {
							$locks_held[$lock_assetid][$lock_type] = TRUE;
						} else {
							// bail if we can't get all of the locks
							break 2;
						}
					}
				}

				if (!$lock_success) {
					if ($not_required) {
						// next action, please
						continue;
					} else {
						$status = SQ_TRIG_RESULT_FAILURE;
						break;
					}
				}

				// execute action
				$action_success = call_user_func_array(Array($action['type'], 'execute'), Array($action['data'], &$state_data));
			}

			if ($action_success === FALSE) {
				if ($not_required) {
					if (!$is_blocking) {
						trigger_localised_error('CORE0134', E_USER_NOTICE, $action['type']);
					}
					continue;
				} else {
					$status = SQ_TRIG_RESULT_FAILURE;
					if (!$is_blocking) {
						trigger_localised_error('CORE0135', E_USER_NOTICE, $action['type']);
					}
					break;
				}
			}

			$status = SQ_TRIG_RESULT_SUCCESS;

		}//end foreach

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
	* @return object
	* @access private
	*/
	function _getHashObject()
	{
		$hash =& new Hash();
		return $hash;

	}//end _getHashObject()


	/**
	* Return a blank trigger in preparation for editing
	*
	* @return array
	* @access private
	*/
	function _getEmptyTrigger()
	{
		$trigger['name'] = '';
		$trigger['description'] = '';
		$trigger['data'] = NULL;
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
	* @return int
	* @access private
	*/
	function _saveTrigger($trigger)
	{
		// triggers do not work in ROLLBACK mode
		if (SQ_ROLLBACK_VIEW) return TRUE;
		if (empty($trigger)) return FALSE;

		// start transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$status = TRUE;
		$new    = FALSE;

		// lack of id means it is a new trigger
		// this bit gets all ids to come from the sequence, manual entries will make db inconsistent
		if (!isset($trigger['id'])) {
			$new = TRUE;
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
		if ($status && array_get_index($trigger, 'save_hash', TRUE) && $trigger['active']) {

			$hash = $this->_getHashObject();
			$hash->setTriggerId($trigger['id']);

			// update the hash
			if (isset($trigger['data'])) {
				if (isset($trigger['data']['events'])) {
					foreach ($trigger['data']['events'] as $event) {
						$hash->setEvent($event);
					}
				}
				if (isset($trigger['data']['conditions'])) {
					foreach ($trigger['data']['conditions'] as $condition) {
						if ($GLOBALS['SQ_SYSTEM']->am->installed(array_get_index($condition, 'type', ''))) {
							if ($this->_loadComponent($condition['type'])) {
								if (isset($condition['data'])) {
									$settings =& $condition['data'];
									call_user_func_array(Array($condition['type'], 'setHash'), Array($settings, $hash));
								}
							}
						}
					}
				}
			}

			$status = $status && $this->_saveTriggerHash($hash);
		}

		// now try to save the trigger itself
		if ($status) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $trigger['id'];
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

	}//end _saveTrigger()


	/**
	* Saves the trigger hash in the database
	*
	* @param object	$hash	the Hash object
	*
	* @return boolean
	* @access private
	*/
	function _saveTriggerHash($hash=NULL)
	{
		if (empty($hash)) return FALSE;

		$hash_combinations = $hash->getHashCombinations();

		if (empty($hash_combinations)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($hash_combinations as $combination) {

			$field_array = Array();
			$value_array = Array();

			foreach ($combination as $hash_id => $hash_data) {
				$field_array[] = $hash_id;
				$value_array[] = $db->quoteSmart($hash_data);
			}

			$sql = 'INSERT INTO
						sq_trig_hash
						(
							'.implode($field_array, ', ').'
						)
						VALUES
						(
							'.implode($value_array, ', ').'
						)';

			$result = $db->query($sql);

			// check result
			if (!assert_valid_db_result($result, NULL, FALSE, FALSE)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _saveTriggerHash()


	/**
	* Format and return the state_hash
	*
	* @param array	$state_hash	the state_hash to check
	*
	* @return array
	* @access private
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
	* @return boolean
	* @access private
	*/
	function _deleteTrigger($trigger_id)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_trig
				WHERE
					id = '.$db->quoteSmart($trigger_id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$success = $this->_deleteTriggerHash($trigger_id);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $success;

	}//end _deleteTrigger()


	/**
	* Removes all hash entries for a particular trigger ID
	*
	* @param int	$trigger_id	id of trigger whose hash is be deleted
	*
	* @return boolean
	* @access private
	*/
	function _deleteTriggerHash($trigger_id)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_trig_hash
				WHERE
					triggerid = '.$db->quoteSmart($trigger_id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _deleteTriggerHash()


//--        GETTERS/SETTERS, LOAD FUNCTIONS        --//


	/**
	* Returns a list of all the triggers, including data
	* See Trigger_Manager::getTrigger() for a complete description of the return format
	*
	* @return array
	* @access public
	* @see Trigger_Manager::getTrigger()
	*/
	function &getTriggerList()
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$query = 'SELECT
					id,
					name,
					data,
					description,
					active
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'trig';

		$trigger_db = $db->getAll($query);
		assert_valid_db_result($trigger_db);

		if (empty($trigger_db)) {
			foreach ($trigger_db as $trigger_data) {
				$trigger_data['data'] = (isset($trigger_data['data']) && !empty($trigger_data['data'])) ? unserialize($trigger_data['data']) : '';
			}
		}

		return $trigger_db;

	}//end getTriggerList()


	/**
	* Returns a trigger with the passed ID
	*
	* <pre>
	* Array(
	*	'id'			=> integer,			// the triggerid
	*	'name'			=> string,			// the name of the trigger
	*	'description'	=> string,			// a description of the trigger
	*	'active'		=> boolean,			// on/off switch for each trigger
	*	'data'			=> Array(
	*						'events'		=> Array(
	*											'event_name'	=> 'event_name',
	*										   ),
	*						'conditions'	=> Array(
	*											int	=> Array(
	*													'type'	=> string							// the type_code of the condition
	*													'data'	=> Array(mixed)						// condition-specific data
	*												   ),
	*										   ),
	*						'actions'		=> Array(
	*											int	=> Array(
	*													'type'					=> string			// the type_code of the condition
	*													'data'					=> Array(mixed)		// condition-specific data
	*													'not_required'			=> boolean			// if true, if the action fails, the trigger will continue to run
	*													'ignore_permissions'	=> boolean			// whether to run with elevated privileges
	*												   ),
	*										   ),
	*						'settings'		=> Array(
	*											'blocking'	=> boolean,		// if true, then if this trigger fails it will halt the
	*																		// execution of any further triggers scheduled to run
	*										   ),
	*					   ),
	* )
	* </pre>
	*
	* @param int	$triggerid	The id of the trigger to retrieve
	*
	* @return array
	* @access public
	*/
	function getTrigger($triggerid)
	{
		if (is_numeric($triggerid)) {
			return $this->_loadTrigger($triggerid);
		} else {
			return NULL;
		}

	}//end getTrigger()


	/**
	* Creates a trigger from data passed, and returns the id of the new trigger
	* See Trigger_Manager::getTrigger() for a complete description of $trigger,
	* except that new triggers do not require $trigger['id']
	*
	* @param int	&$trigger	the id of the trigger to use to create the hash, null for all
	*
	* @return int
	* @access public
	* @see Trigger_Manager::getTrigger()
	*/
	function createTrigger(&$trigger)
	{
		// check the structure of trigger
		// $s contains the status - if any part of $trigger fails the checking,
		// $s will stop further checks from happening
		$s  = TRUE;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// basic elements
		$s = $s && isset($trigger['name']);
		$s = $s && isset($trigger['description']);
		$s = $s && isset($trigger['active']);
		$s = $s && isset($trigger['data']);

		// events
		$s = $s && isset($trigger['data']['events']);

		// conditions
		$s = $s && isset($trigger['data']['conditions']);
		if ($s && !empty($trigger['data']['conditions'])) {
			foreach ($trigger['data']['conditions'] as $condition) {
				$s = $s && isset($condition['type']) && $am->installed($condition['type']);
				$s = $s && isset($condition['data']);
			}
		}

		// actions
		$s = $s && isset($trigger['data']['actions']);
		if ($s && !empty($trigger['data']['actions'])) {
			foreach ($trigger['data']['actions'] as $action) {
				$s = $s && isset($action['type']) && $am->installed($action['type']);
				$s = $s && isset($action['data']);
				$s = $s && isset($action['not_required']);
				$s = $s && isset($action['ignore_permissions']);
			}
		}

		if ($s) {
			return $this->_saveTrigger($trigger);
		} else {
			trigger_localised_error('CORE0232', E_USER_WARNING);
			return FALSE;
		}

	}//end createTrigger()


	/**
	* Load Trigger Data
	*
	* @param int	$trigger_id	the ID of the trigger to load
	*
	* @return array
	* @access private
	*/
	function _loadTrigger($trigger_id=NULL)
	{
		if (!isset($trigger_id)) return NULL;
		$db =& $GLOBALS['SQ_SYSTEM']->db;

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
				trigger_localised_error('CORE0133', E_USER_WARNING, $trigger_id);
			}

			return NULL;
		}

		$trigger_db['data'] = (isset($trigger_db['data']) && !empty($trigger_db['data'])) ? unserialize($trigger_db['data']) : '';

		// set defaults if not present
		if (!isset($trigger_db['data']['events'])) {
			$trigger_db['data']['events'] = Array();
		}
		if (!isset($trigger_db['data']['conditions'])) {
			$trigger_db['data']['conditions'] = Array();
		}
		if (!isset($trigger_db['data']['actions'])) {
			$trigger_db['data']['actions'] = Array();
		}
		if (!isset($trigger_db['data']['settings'])) {
			$trigger_db['data']['settings'] = Array();
		}

		return $trigger_db;

	}//end _loadTrigger()


	/**
	* Obtain a list of trigger info
	*
	* @return array
	* @access private
	*/
	function _getTriggerInfoList()
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

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
	* @return array
	* @access private
	*/
	function _getComponentList($type=NULL)
	{
		if (empty($type)) return Array();

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$component_types = $am->getTypeDescendants($type);
		$component_info  = $am->getTypeInfo($component_types, 'name');

		return $component_info;

	}//end _getComponentList()


	/**
	* Get A list of all installed events
	*
	* @return array
	* @access private
	*/
	function _getEventList()
	{
		return $this->_getComponentList('trigger_event');

	}//end _getEventList()


	/**
	* Get a list of installed conditions
	*
	* @return array
	* @access private
	*/
	function _getConditionList()
	{
		return $this->_getComponentList('trigger_condition');

	}//end _getConditionList()


	/**
	* Get a list of installed actions
	*
	* @return array
	* @access private
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
	*
	* @return boolean
	* @access private
	*/
	function _loadComponent($type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end _loadComponent()


	/**
	* Returns a string containing the buffered output of a component's edit interface
	*
	* @param string		$type_code		the type_code of the component to get the interface of
	* @param array		$data			the components settings data
	* @param string		$prefix			the temporary unique ID for the component
	* @param boolean	$write_access	the write access status of the component
	*
	* @return string
	* @access private
	*/
	function _getComponentInterface($type_code, $data, $prefix, $write_access=FALSE)
	{
		if (!$this->_loadComponent($type_code)) return '';

		$interface = call_user_func_array(Array($type_code, 'getInterface'), Array($data, $prefix, $write_access));

		return $interface;

	}//end _getComponentInterface()


	/**
	* Determines if more than one instance of a condition is allowed in the one trigger
	*
	* @param string	$type_code	the type_code of the condition to check
	*
	* @return boolean
	* @access private
	*/
	function _isMultipleConditionAllowed($type_code)
	{
		$this->_loadComponent($type_code);
		$status = call_user_func(Array($type_code, 'allowMultiple'));
		return $status;

	}//end _isMultipleConditionAllowed()


	/**
	* Returns a list of triggerids for triggers that listen for a given event
	*
	* @param string	$event	the name of the event
	*
	* @return boolean
	* @access private
	*/
	function _isEventListenedFor($event)
	{
		// Check the trigger hash table to see whether any active triggers are listening for this event
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = '
				SELECT
					COUNT(triggerid)
				FROM
					sq_trig_hash
				WHERE
					event = '.$db->quoteSmart($event).'
			';
		$trigger_count = $db->getOne($sql);
		assert_valid_db_result($trigger_count);

		return !empty($trigger_count);

	}//end _isEventListenedFor()


}//end class


?>
