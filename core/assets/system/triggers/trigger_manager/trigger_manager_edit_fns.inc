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
* $Id: trigger_manager_edit_fns.inc,v 1.6 2005/02/25 05:28:09 rhoward Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Trigger_Manager_Edit_Fns
*
* Purpose
*   To handle the creation, modification, and the batch-firing of triggers
*
* @author  Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Manager_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Trigger_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		// disable all static screens
		$this->static_screens = Array();

		// enable just the details
		$this->static_screens['details']['name'] ='Details';
		$this->static_screens['details']['force_unlock'] = false;

	}//end Trigger_Manager_Edit_Fns()


	/**
	* Paint the interface for creating a new metadata section
	*
	* @param object Metadata_Schema		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTriggerCount(&$asset, &$o, $prefix)
	{
		$count = $asset->_countTriggers();
		echo $count;
		return true;

	}//end paintTriggerCount()


	/**
	* Main Trigger Screen Display Controller
	*
	* Determines which Subscreen to display based on state info

	* @param object Trigger_Manager &$tm trigger manager
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTriggerScreen(&$tm, &$o, $prefix)
	{
		$write_access = $tm->writeAccess('attributes');

		$o->openField("");

		$screen = 'list';

		// if there is a trigger id to edit, go to edit screen
		if (isset($this->_tmp['edit_id'])) {
			$screen = 'edit';
		}

		// determine which subscreen to display
		switch ($screen) {
			case 'list':
				$o->sectionNote('This subscreen allows you to add new trigger or select an existing one to edit');
				$this->_paintSubScreenList($tm, $o, $prefix, $write_access);
			break;

			case 'edit':
				$o->sectionNote('This subscreen is for editing a single trigger. A trigger consists of descriptive information and a set of events, conditions and actions. Use the sections below to change settings of this trigger. In order to speed trigger execution, MySource Matrix tries to make some pre-calculations. There is an overhead to determining whether a trigger needs to be run. Upon loading, the conditions are checked and if all pass, the actions are executed. It can be rewarding to put the fastest conditions first. As a rule of the thumb any condition that works on a whole range of values, like "anything that extends this asset type" will be relatively slow and should not be placed first. On the other hand system conditions that work on exact values of asset type (not inheritance), status and tree location should be placed first.<br /><br /><strong>ATTENTION:</strong> Any trigger modification cannot be undone once saved. An invalid trigger is capable of bringing the whole system down.');
				$this->_paintSubScreenEdit($tm, $o, $prefix.'[edit]', $write_access);
			break;

			default:
				// unknown error
				trigger_error("I'm lost in the matrix", E_USER_ERROR);
		}

		$o->closeField();
		return true;

	}//end paintTriggerScreen()


	/**
	* Paint Subscreen for Edit Triggers
	*
	* @param object Trigger_Manager     &$tm trigger manager
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	* @param boolean					$write_access	flag whether it is
	*
	* @access private
	*/
	function _paintSubScreenEdit(&$tm, &$o, $prefix, $write_access=false)
	{
		if (!$write_access){
			return;
		}

		$trigger_id = $this->_tmp['edit_id'];

		$trigger = $tm->_loadTrigger($trigger_id);
		if (empty($trigger)) {
			// paint error message
			trigger_error("Trigger not found", E_USER_WARNING);
			return false;
		}

		// the styles below can be moved to edit.css in the future
		?>
		<style type="text/css">
			.sq-backend-triggers-component-header {
				background-color: #666;
				color: #fff;
				padding: 2px;
			}

			.sq-backend-triggers-component-global-options {
				background-color: #eee;
				padding: 2px;
				border-bottom: 1px dotted black;
				border-top: 1px dotted black;
			}

			.sq-backend-triggers-component-type-info{
				float: left;
				font-weight: bold;
				padding: 1px;
			}

			.sq-backend-triggers-component-controls {
				text-align: right;
			}

			.sq-backend-triggers-component-block-wrapper {
				padding: 1px;
			}

			.sq-backend-triggers-component-body {
				padding: 3px;
				margin: 1px;
				background-color: #fff;
			}

			.sq-backend-triggers-new-component-selector {
				font-weight: bold;
				padding: 3px;
				margin: 0 0 10px 0;
			}

			.sq-backend-triggers-component-wrapper {
				background-color: #eee;
				border: 1px solid black;
				margin: 10px 0;
			}

		</style>

		<script type="text/javascript">
			//<![CDATA[
			/**
			* Moves a div up (if possible) relative to the div with the same class name
			*/
			function moveUp(div) {
				previous = div.previousSibling;
				if (previous == null) {
					return;
				}
				while (previous.className != div.className) {
					previous = previous.previousSibling;
					if (previous == null) {
						return;
					}
				}
				div.parentNode.insertBefore(div, previous);
			}

			/**
			* Moves a div down
			*/
			function moveDown(div) {
				next = div.nextSibling;
				while (next.className != div.className) {
					next = next.nextSibling;
					if (next == null) {
						return;
					}
				}
				div.parentNode.insertBefore(next, div);
			}

			/**
			* Deletes a div, with a confirmation box
			*/
			function deleteDiv(div) {
				if(confirm("Deleting a component cannot be undone.\nAre you sure you want to proceed?")) {
					div.parentNode.removeChild(div);
				}
			}

			/**
			* Object that handles the ordering of elements. Used for accounting purposes only,
			* i.e does not move any objects around. After initialisation, the methods of this object
			* are used to record the movement of an element up or down, can also handle the deletion
			* of an element. After each operation the field identified by field_id is updated to
			* contain a list of ids of elements, ordered properly.
			*
			* This object is useful because if the elements that get moved around contain forms,
			* some browsers get confused and ignore the dynamic order of the elements.
			* It appears that browsers take a snapshot of the form and operate on it, regardless of
			* what modifications the form might undertake by JavaScript
			*/
			componentOrderer = function(field_id, count)
			{
				this.field_id = field_id;

				this.values = Array();
				this.keys = Array();

				count = parseInt(count);
				for (i=0; i<count; i++) {
					this.values.push(i);
					this.keys.push(i);
				}

				this.moveUp = function(value)
				{
					var key = this.keys[value];
					if (key == 0) {
						return;
					}

					var new_key = key-1;

					this.swap(key, new_key);
					this.update()
				}

				this.moveDown = function(value)
				{
					var key = this.keys[value];
					if (key == this.values.length-1) {
						return;
					}

					var new_key = key+1;

					this.swap(key, new_key);
					this.update()
				}

				this.remove = function(value)
				{
					var key = this.keys[value];
					this.values.splice(key,1);
					this.keys[value] = null;

					for (var i=0; i<this.values.length; i++) {
						this.keys[this.values[i]] = i;
					}
					this.update()
				}

				// this function assumes that keys are valid and can be swapped
				this.swap = function(key1, key2)
				{
					var val1 = this.values[key1];
					var val2 = this.values[key2];

					this.values[key1] = val2;
					this.values[key2] = val1;

					this.keys[val1] = key2;
					this.keys[val2] = key1;

				}

				this.update = function()
				{
					var joined = this.values.join();
					document.getElementById(this.field_id).value = joined;
				}

			}
			//]]>
		</script>
		<?php
		$o->closeSection();

		$o->openSection('Trigger Info');

		$o->openField('ID:');
			echo(array_get_index($trigger, 'id', 'N/A'));
			hidden_field($prefix.'[id]', array_get_index($trigger, 'id', ''));
		$o->closeField();

		$o->openField('Status:');
			check_box($prefix.'[info][active]','1',array_get_index($trigger, 'active', SQ_TRIG_STATUS_DEFAULT));
			echo 'Active?';
		$o->closeField();

		$o->openField('Name:');
			text_box($prefix.'[info][name]', array_get_index($trigger, 'name', ''), 40);
			hidden_field($prefix.'[id]', array_get_index($trigger, 'id', ''));
		$o->closeField();

		$o->openField('Description:');
			text_area($prefix.'[info][description]', array_get_index($trigger, 'description', ''), 40, 5);
		$o->closeField();

		// initialization
		$trigger_data = array_get_index($trigger, 'data', Array());

		$events     = array_get_index($trigger_data, 'events', Array());
		$conditions = array_get_index($trigger_data, 'conditions', Array());
		$actions    = array_get_index($trigger_data, 'actions', Array());

		$event_type_list     = $tm->_getEventList();
		$condition_type_list = $tm->_getConditionList();
		$action_type_list    = $tm->_getActionList();

		$o->closeSection();
		$o->openSection('Events');
		$o->openField('');
		$o->sectionNote('This is a list of all the events installed in this system. Select the ones you want this trigger to respond to. If you do not specify any events, the trigger will be fired when ANY event is fired, which can negatively affect the speed of the system.');

		// paint a tickbox list of all the events in the system
		$event_prefix = $prefix.'[events]';
		foreach ($event_type_list as $e_type => $e_name) {
			if (isset($events[$e_type])) {
				$checked = true;
			} else {
				$checked = false;
			}
			check_box($event_prefix.'[]', $e_type, $checked);
			echo $e_name;
			echo "<br />";
		}

		$o->closeSection();
		$o->openSection('Conditions');
		$o->openField('');
		$o->sectionNote("Here you specify the conditions that must be satisfied before the actions are allowed to perform. Note that order is important. New conditions are added to the bottom of the list with default settings. If you add a condition, but do not change it's settings it will automatically fail. Some condition types can only be used once in a trigger.");
		?>
		<div class="sq-backend-triggers-component-block-wrapper">
			<div class="sq-backend-triggers-component-block">
			<?php
			// process the condition list to detemine which ones allow multiple instances
			$multiple_status = Array();
			$type_instance_count = Array();
			foreach ($condition_type_list as $type => $name) {
				$multiple_status[$type] = $tm->_isMultipleConditionAllowed($type);
				$type_instance_count[$type] = 0;
			}

			$condition_prefix = $prefix.'[conditions]';
			$condition_order_prefix = $prefix.'[condition_order]';
			$i = 0;
			foreach ($conditions as $condition) {
				$type = $condition['type'];

				// check if condition is allowed to be displayed
				// even if somehow it got into the settings, if it is not multiple,
				// it will not be displayed
				if (!$multiple_status[$type]  && $type_instance_count[$type] != 0) {
					continue;
				}

				$type_instance_count[$type]++;

				$data = $condition['data'];

				$data_prefix   = $condition_prefix.'['.$i.'][data]';
				$type_prefix   = $condition_prefix.'['.$i.'][type]';
				$delete_prefix = $condition_prefix.'['.$i.'][delete]';

				$interface = $tm->_getComponentInterface($type, $data, $data_prefix);
				$type_name = $tm->_getComponentName($type);
				?>
				<div class="sq-backend-triggers-component-wrapper">
					<div class="sq-backend-triggers-component-header">
						<div class="sq-backend-triggers-component-type-info">
							<?php echo $type_name; ?>
						</div>
						<div class="sq-backend-triggers-component-controls">
							<input type="checkbox" name="<?php echo $delete_prefix; ?>" >
							Delete
								&nbsp;&nbsp;&nbsp;&nbsp;
							<a href="#" onclick="moveUp(this.parentNode.parentNode.parentNode); conditionOrdererObject.moveUp(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a>
							<a href="#" onclick="moveDown(this.parentNode.parentNode.parentNode); conditionOrdererObject.moveDown(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a>
						</div>
					</div>
					<div class="sq-backend-triggers-component-body">
						<input type="hidden" name="<?php echo $type_prefix; ?>" value="<?php echo $type; ?>" />
						<?php echo $interface; ?>
					</div>
				</div>
				<?php
				$i++;
			}
			?>
			</div>
			<script type="text/javascript">var conditionOrdererObject = new componentOrderer('conditionOrderer',<?php echo $i; ?>)</script>
			<input type="hidden" id="conditionOrderer" name="<?php echo $condition_order_prefix; ?>" />
			<!-- Adding a New Condition -->
			<div class="sq-backend-triggers-new-component-selector">
				Add new Condition type:
					<?php
					$select_list = Array(' '=>' ---- Select to Add ---- ');
					$removed = false;
					foreach ($condition_type_list as $type => $name) {
						if (!$multiple_status[$type] && $type_instance_count[$type] != 0) {
							$removed = true;
							continue;
						}
						$select_list[$type] = $name;
					}

					combo_box($prefix.'[new_condition]', $select_list, false, ' ');
					if ($removed) {
						echo ' <span class="sq-backend-warning">Some conditions can only be added once.</span>';
					}
					?>
			</div>
		</div>

		<?php
		// actions are handled the same way as conditions
		$o->closeSection();
		$o->openSection('Actions');
		$o->openField('');
		$o->sectionNote("Actions are performed in the order they are specified. If one action fails, the trigger will fail. Unless 'Ignore Permissions' is selected, the action is run as a user that caused the trigger to be executed (currently logged in user). This means that some actions might fail if they user doesn't have appropriate access privileges. To make sure that the actions are performed regardles of user's access rights, select 'Ignore Permissions'. Actions can be marked as 'Not Required' which means that if that action fails for some reason, the trigger will not fail. If an action fails and it is required, then the system will try to undo all the previous actions in this trigger.");
		?>
		<div class="sq-backend-triggers-component-block-wrapper">
			<div class="sq-backend-triggers-component-block">
			<?php
			$action_prefix = $prefix.'[actions]';
			$action_order_prefix = $prefix.'[action_order]';
			$i = 0;
			foreach ($actions as $action) {
				$this_prefix   = $action_prefix.'['.$i.']';

				$data_prefix               = $this_prefix.'[data]';
				$type_prefix               = $this_prefix.'[type]';
				$delete_prefix             = $this_prefix.'[delete]';
				$ignore_permissions_prefix = $this_prefix.'[ignore_permissions]';
				$not_required_prefix       = $this_prefix.'[not_required]';

				$type = $action['type'];
				$data = $action['data'];
				$not_required = array_get_index($action, 'not_required', false);
				$ignore_permissions = array_get_index($action, 'ignore_permissions', false);

				$interface = $tm->_getComponentInterface($type, $data, $data_prefix);
				$type_name = $tm->_getComponentName($type);
				?>
				<div class="sq-backend-triggers-component-wrapper">
					<div class="sq-backend-triggers-component-header">
						<div class="sq-backend-triggers-component-type-info">
							<?php echo $type_name; ?>
						</div>
						<div class="sq-backend-triggers-component-controls">
							<input type="checkbox" name="<?php echo $delete_prefix; ?>" >
							Delete
								&nbsp;&nbsp;&nbsp;&nbsp;
							<a href="#" onclick="moveUp(this.parentNode.parentNode.parentNode); actionOrdererObject.moveUp(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a>
							<a href="#" onclick="moveDown(this.parentNode.parentNode.parentNode); actionOrdererObject.moveDown(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a>
						</div>
					</div>
					<div class="sq-backend-triggers-component-global-options">
						<?php check_box($ignore_permissions_prefix, 1, $ignore_permissions); ?>Ignore Permissions
						<?php check_box($not_required_prefix, 1, $not_required); ?>NOT required
					</div>
					<div class="sq-backend-triggers-component-body">
						<input type="hidden" name="<?php echo $type_prefix; ?>" value="<?php echo $type; ?>" />
						<?php echo $interface; ?>
					</div>
				</div>
				<?php
				$i++;
			}
			?>
			<script type="text/javascript">var actionOrdererObject = new componentOrderer('actionOrderer',<?php echo $i; ?>)</script>
			<input type="hidden" id="actionOrderer" name="<?php echo $action_order_prefix; ?>" />
			<!-- Add a New Action -->
			<div class="sq-backend-triggers-new-component-selector">
				Add new Action type:
				<select name="<?php echo $prefix; ?>[new_action]" >
					<option value="" > ---- Select to Add ---- </option>
					<?php
					foreach ($action_type_list as $a_type => $a_name) {
						echo '<option value="'.$a_type.'" >'.$a_name.'</option>';
					}
					?>
				</select>
			</div>

			</div>
		</div>
		<?php

		$o->closeSection();
		$o->openSection('Other');
		$o->openField('');
		?>
		<div>
			<?php echo check_box($prefix.'[finish]'); ?>Finish Editing (save and display the trigger list)
		</div>
		<?php
		$o->closeSection();

		return true;

	}//end _paintSubScreenEdit()



	/**
	* Process Triggers Screen Main Controller
	*
	* This function determines how to process the input from the backend interface
	*
	* @param object Trigger_Manager     &$tm    trigger manager
	* @param object	Backend_Outputter	&$o     the outputter class
	* @param string                     $prefix prefix for the form elements
	*
	* @access public
	*/
	function processTriggerScreen(&$tm, &$o, $prefix)
	{
		$status = false;

		$request_data = array_get_index($_REQUEST, $prefix, false);
		if (!$request_data) {
			// Can't do anything without the REQUEST vars, fail
			return $status;
		}

		$screen = array_get_index($request_data, 'sub_screen', 'list');
		if (isset($request_data['edit'])) {
			$screen = 'edit';
		}

		switch ($screen) {
			case 'list':
				$status = $this->_processSubScreenList($tm, $request_data);
			break;

			case 'edit':
				$status = $this->_processSubScreenEdit($tm, $request_data);
			break;
			default:
				return $status;
		}

		return $status;

	}//end processTriggerScreen()


	/**
	* Process input submitted from the List subscreen
	*
	* This function determines how to process the input from the backend interface
	*
	* @param object Trigger_Manager     &$tm trigger manager
	* @param array		                &$request_data		data extracted from the request variables
	*
	* @access private
	*/
	function _processSubScreenList(&$tm, $request_data)
	{
		// edit request
		$edit_id = array_get_index($request_data, 'edit_id', false);

		// add new trigger
		$new_name = array_get_index($request_data, 'add_name', false);
		if ($new_name && !empty($new_name)) {
			$trigger = $tm->_getEmptyTrigger();
			$trigger['name'] = $new_name;

			$new_id = $tm->_saveTrigger($trigger);
			if (!empty($new_id)) {
				// make sure the newly added trigger is to be edited next
				$edit_id = $new_id;
			}
		}

		if ($edit_id) {
			$this->_tmp['edit_id'] = $edit_id;
		}

		// delete the requested triggers
		$to_delete = array_get_index($request_data, 'delete', false);
		if (!empty($to_delete)) {
			$i=0;
			foreach ($to_delete as $trigger_id => $junk) {
				// edit request takes precence over delete
				if (!empty($edit_id) && $trigger_id == $edit_id) {
					continue;
				}
				$tm->_deleteTrigger($trigger_id);
				$i++;
			}
		}

		//return true;

	}//end _processSubScreenList()


	/**
	* Process input submitted from the Edit subscreen
	*
	* This function determines how to process the input from the backend interface
	*
	* @param object Trigger_Manager     &$tm                trigger manager
	* @param array                      &$request_data      data extracted from the request variables
	*
	* @access private
	*/
	function _processSubScreenEdit(&$tm, $request_data)
	{
		$edit_data = array_get_index($request_data, 'edit', Array());
		$edit_id = array_get_index($edit_data, 'id');

		if (empty($edit_data) || empty($edit_id)) {
			return false;
		}

		if (array_get_index($edit_data, 'reload', false)) {
			$this->_tmp['edit_id'] = $edit_id;
			return;
		}

		$trigger = $tm->_loadTrigger($edit_id);
		if (empty($trigger)) {
			return false;
		}

		$hash = $tm->_getHashObject();

		$edit_info = array_get_index($edit_data, 'info');

		// setup basic trigger info
		$trigger['name']        = array_get_index($edit_info, 'name');
		$trigger['description'] = array_get_index($edit_info, 'description');
		$trigger['active']      = array_get_index($edit_info, 'active');
		$hash->setActive($trigger['active']);

		// events
		$raw_events = array_get_index($edit_data, 'events', Array());
		$processed_events = Array();
		foreach ($raw_events as $one_event_type) {
			$processed_events[$one_event_type] = $one_event_type;
			$hash->setEvent($one_event_type);
		}

		// conditions
		$raw_conditions = array_get_index($edit_data, 'conditions');
		$condition_order = array_get_index($edit_data, 'condition_order');
		if (!empty($condition_order)) {
			$order_array = explode(",", $condition_order);
			foreach ($order_array as $position) {
				$new_raw_conditions[] = $raw_conditions[$position];
			}
			$raw_conditions = $new_raw_conditions;
		}
		$new_condition_type = array_get_index($edit_data, 'new_condition');
		if (!empty($new_condition_type)) {
			$raw_conditions[] = Array(
				'type' => $new_condition_type,
				'new'  => true,
			);
		}
		$processed_conditions = $this->_processRawConditionSet($tm, $raw_conditions, $hash);

		// actions
		$raw_actions = array_get_index($edit_data, 'actions');
		$action_order = array_get_index($edit_data, 'action_order');
		if (!empty($action_order)) {
			$order_array = explode(",", $action_order);
			foreach ($order_array as $position) {
				$new_raw_actions[] = $raw_actions[$position];
			}
			$raw_actions = $new_raw_actions;
		}
		$new_action_type = array_get_index($edit_data, 'new_action');
		if (!empty($new_action_type)) {
			$raw_actions[] = Array(
				'type' => $new_action_type,
				'new'  => true,
			);
		}

		$processed_actions = $this->_processRawActionSet($tm, $raw_actions);

		if (empty($processed_events) || empty($processed_actions)) {
			$trigger['save_hash'] = false;
		}

		$trigger['data']['events']     = $processed_events;
		$trigger['data']['conditions'] = $processed_conditions;
		$trigger['data']['actions']    = $processed_actions;
		$trigger['state_hash']         = &$hash;

		$tm->_saveTrigger($trigger);

		// set this to make sure we return to edit screen of the same trigger
		if (!isset($edit_data['finish'])) {
			$this->_tmp['edit_id'] = $edit_id;
		}

		return true;

	}//end _processSubScreenEdit()


	/**
	* Process a set of raw conditions
	*
	* Conditions that were supplied by the interface, plus the newly added one
	*
	* @param object Trigger_Manager     &$tm trigger manager
	* @param array                      $raw_components a list of submitted conditions
	* @param object Hash		        &$hash		hash object to populate
	*
	* @access private
	*/
	function _processRawConditionSet(&$tm, $raw_components, &$hash)
	{
		$processed_components = Array();

		if (empty($raw_components)) {
			return $processed_components;
		}

		$i=1;
		foreach ($raw_components as $component) {
			$status = true;
			if (isset($component['delete'])) {
				continue;
			}
			$settings = Array();

			$type = array_get_index($component, 'type');
			if (empty($type) || !$tm->_loadComponent($type)) {
				continue; // invalid type is silently ignored
			}

			$data = array_get_index($component, 'data', Array());
			if (!array_get_index($component, 'new', false)) {
				eval('$status = '.$type.'::processInterface($settings, $hash, $data);');
			}

			$processed_components[] = Array(
				'type' => $type,
				'data' => $settings,
			);

			if (!$status) {
				trigger_error("Settings for Condition number $i ($type) could not be saved. Please make sure the input is complete.", E_USER_WARNING);
			}
			$i++;
		}

		return $processed_components;

	}//end _processRawConditionSet()


	/**
	* Process a set of raw actions
	*
	* Action settings that were supplied by the interface, plus the newly added one are
	* converted to the form that can be saved and used later
	*
	* @param object Trigger_Manager     &$tm trigger manager
	* @param array                      $raw_components a list of submitted actions
	* @param object Hash		        &$hash		hash object to populate
	*
	* @access private
	*/
	function _processRawActionSet(&$tm, $raw_components)
	{
		$processed_components = Array();

		if (empty($raw_components)) {
			return $processed_components;
		}

		$i=1;
		foreach ($raw_components as $component) {
			$status = true;
			if (isset($component['delete'])) {
				continue;
			}

			$settings = Array();

			$type = array_get_index($component, 'type');
			if (empty($type) || !$tm->_loadComponent($type)) {
				continue; // invalid type is silently ignored
			}


			$data = array_get_index($component, 'data', Array());
			if (!array_get_index($component, 'new', false)) {
				eval('$status = '.$type.'::processInterface($settings, $data);');
			}

			$processed_components[] = Array(
				'type' => $type,
				'data' => $settings,
				'not_required'       => isset($component['not_required']),
				'ignore_permissions' => isset($component['ignore_permissions']),
			);

			if (!$status) {
				trigger_error("Settings for Action number $i ($type) could not be saved. Please make sure the input is complete.", E_USER_WARNING);
			}
			$i++;
		}

		return $processed_components;

	}//end _processRawActionSet()


	/**
	* Paint Trigger List sub screen
	*
	* @param object Trigger_Manager     &$tm trigger manager
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	* @param boolean                    $write_access a flag denoting whether we're editing or not
	*
	* @access private
	*/
	function _paintSubScreenList(&$tm, &$o, $prefix, $write_access)
	{
		$o->closeSection();
		$list = $tm->_getTriggerInfoList();


		if ($write_access) {
			$o->openSection('Add New Trigger');
			$o->openField("New Trigger Name: ");
			text_box($prefix.'[add_name]', '');
			echo "<br />To add a new trigger, enter the name in this box and submit the form";
			$o->closeSection();
		}

		$o->openSection('Trigger List');
		$o->openField("");
		?>
		<script type="text/javascript">
			//<![CDATA[
			var deleteCheckedCount = 0;

			function confirmDelete()
			{
				var result = true;

				if (deleteCheckedCount > 0) {
					result = confirm ("Are you sure you want to delete the selected triggers?\n\nItems to delete: "+deleteCheckedCount);
				}

				return result;
			}

			function checkDelete(checked)
			{
				if (checked) {
					deleteCheckedCount++;
				} else {
					deleteCheckedCount--;
				}
			}
			//]]>
		</script>
		<table class="sq-backend-table">
			<tr>
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;">ID</td>
				<td class="sq-backend-table-header" style="font-weight: bold;">Name</td>
				<td class="sq-backend-table-header" style="font-weight: bold;">Description</td>
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;">Status</td>
				<?php if ($write_access) { ?>
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;">Edit</td>
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;">Delete?</td>
				<?php } ?>
			</tr>

		<?php
		foreach ($list as $trigger) {
			$active_text = $trigger['active']?'<strong>Active</strong>':'<span style="color: red">Inactive</strike>';
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo $trigger['id']; ?></td>
				<td class="sq-backend-table-cell"><?php echo $trigger['name']; ?></td>
				<td class="sq-backend-table-cell"><?php echo $trigger['description']; ?></td>
				<td class="sq-backend-table-cell"><strong><?php echo $active_text; ?></strong></td>
				<?php if ($write_access) { ?>
				<td class="sq-backend-table-cell"><input type="radio" name="<?php echo $prefix; ?>[edit_id]" value="<?php echo $trigger['id']; ?>"></td>
				<td class="sq-backend-table-cell"><input type="checkbox" name="<?php echo $prefix; ?>[delete][<?php echo $trigger['id']; ?>]"  onClick="checkDelete(this.checked)" ></td>
				<?php } ?>
			</tr>
		<?php
		}

		if ($write_access) { ?>
			<tr style="background-color: white">
				<td class="sq-backend-table-cell">&nbsp;</td>
				<td class="sq-backend-table-cell">&nbsp;</td>
				<td class="sq-backend-table-cell">&nbsp;</td>
				<td class="sq-backend-table-cell">&nbsp;</td>
				<td class="sq-backend-table-cell"><input type="radio" name="<?php echo $prefix; ?>[edit_id]" value="" checked="checked" >None</td>
				<td class="sq-backend-table-cell">&nbsp;</td>
			</tr>
		<?php } ?>
		</table>
		<?php

		$o->closeSection();
		$o->addOnsubmit("return confirmDelete();");

		return true;

	}//end _paintSubScreenList()


	/**
	* Display the Batching Triggers Screen
	*
	* @param object Trigger_Manager     &$tm    trigger manager
	* @param object	Backend_Outputter   &$o     the outputter class
	* @param string                     $prefix prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBatching(&$tm, &$o, $prefix)
	{
		$write_access = $tm->writeAccess('attributes');
		if ($write_access) {
			$o->openField('Target Node');
			asset_finder($prefix.'[target_node]', array_get_index($this->_tmp, 'target_id'));

			$o->openField('Children');
			check_box($prefix.'[include_children]', 'yes', array_get_index($this->_tmp, 'include_children', true));
			echo "Include Children?";

			$o->openField('Events');
			$event_type_list = $tm->_getEventList();
			$events = array_get_index($this->_tmp, 'selected_events');
			// paint a tickbox list of all the events in the system
			$event_prefix = $prefix.'[selected_events]';
			foreach ($event_type_list as $e_type => $e_name) {
				if (isset($events[$e_type])) {
					$checked = true;
				} else {
					$checked = false;
				}
				check_box($event_prefix.'['.$e_type.']', $e_type, $checked);
				echo $e_name;
				echo "<br />";
			}
			return true;
		} else {
			return false;
		}

	}//end paintBatching()


	/**
	* Process Batching Triggers Screen
	*
	* Prepares and fires the trigger batching HIPO job
	*
	* @param object Trigger_Manager     &$tm    trigger manager
	* @param object	Backend_Outputter   &$o     the outputter class
	* @param string                     $prefix prefix for the form elements
	*
	* @access public
	*/
	function processBatching(&$tm, &$o, $prefix)
	{
		$error = false;

		$request_data = array_get_index($_REQUEST, $prefix, Array());

		$target_node = array_get_index($request_data, 'target_node');
		$target_id   = array_get_index($target_node,  'assetid');
		$events      = array_get_index($request_data, 'selected_events', Array());
		$propagate   = isset($request_data['include_children']);

		$this->_tmp['target_id']        = $target_id;
		$this->_tmp['selected_events']  = $events;
		$this->_tmp['include_children'] = $propagate;

		if (empty($target_id)) {
			trigger_error('Target asset not specified. Trigger batching cannot proceed.', E_USER_WARNING);
			$error = true;
		}

		if (empty($events)) {
			trigger_error('Events not specified. Trigger batching cannot proceed.', E_USER_WARNING);
			$error = true;
		}

		if ($error) {
			return false;
		}

		// grab all the children of the target if applicable, then add the target itself
		$all_targets = Array();
		if ($propagate) {
			$all_targets = $GLOBALS['SQ_SYSTEM']->am->getChildren($target_id);
		}
		$target_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($target_id);
		$all_targets[$target_id] = $target_asset->type();

		// convert from id->type to index->id
		$all_targets = array_keys($all_targets);

		// start up the HIPO
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					  'assets'             => $all_targets,
					  'events'             => $events,
					  'codename_target'    => $target_id,
					  'codename_propagate' => ($propagate ? '1' : '0')
					  );
		$hh->queueHipo('hipo_job_trigger_batch_events', $vars);

		return true;

	}//end processBatching()


}//end class

?>
