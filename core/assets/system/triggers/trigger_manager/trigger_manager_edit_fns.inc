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
* $Id: trigger_manager_edit_fns.inc,v 1.22 2006/02/10 02:03:19 sdanis Exp $
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
* @author  Robert Howard  <rhoward@squiz.net>
* @version $Revision: 1.22 $
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
		$this->static_screens['details']['name'] = translate('details');
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Main Trigger Screen Display Controller
	*
	* Determines which Subscreen to display based on state info
	*
	* @param object	&$tm	trigger manager
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTriggerScreen(&$tm, &$o, $prefix)
	{
		$write_access = $tm->writeAccess('attributes');

		$o->openField('');

		$screen = 'list';

		// if there is a trigger id to edit, go to edit screen
		if (!empty($_REQUEST[$prefix]['edit_id'])) {
			$this->_tmp['edit_id'] = $_REQUEST[$prefix]['edit_id'];
		}

		if (isset($this->_tmp['edit_id'])) $screen = 'edit';

		// determine which subscreen to display
		switch ($screen) {
			case 'edit':
				$o->sectionNote(translate('trigger_single_subscreen_explanation'));
				$this->_paintSubScreenEdit($tm, $o, $prefix.'[edit]', $write_access);
			break;

			case 'list':
			default:
				$o->sectionNote(translate('trigger_subscreen_add_or_edit'));
				$this->_paintSubScreenList($tm, $o, $prefix, $write_access);
			break;
		}

		$o->closeField();
		return $write_access;

	}//end paintTriggerScreen()


	/**
	* Paint Subscreen for Edit Triggers
	*
	* @param object		&$tm			trigger manager
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param boolean	$write_access	flag whether it is
	*
	* @return void
	* @access private
	*/
	function _paintSubScreenEdit(&$tm, &$o, $prefix, $write_access=FALSE)
	{

		$trigger_id = $this->_tmp['edit_id'];

		$trigger = $tm->_loadTrigger($trigger_id);
		if (empty($trigger)) {
			// paint error message
			trigger_localised_error('CORE0136', E_USER_WARNING);
			return FALSE;
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
				if(confirm(js_translate('confirm_delete_component'))) {
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

		$o->openSection(translate('trigger_info'));

		$o->openField(translate('id').':');
			echo(array_get_index($trigger, 'id', 'N/A'));
			hidden_field($prefix.'[id]', array_get_index($trigger, 'id', ''));
		$o->closeField();

		$o->openField(translate('status').':');
			$active_status = array_get_index($trigger, 'active', SQ_TRIG_STATUS_DEFAULT);
			if ($write_access) {
				check_box($prefix.'[info][active]','1', $active_status);
				label(translate('active_question'), $prefix.'[info][active]');
			} else {
				if (SQ_TRIG_STATUS_ACTIVE == $active_status) {
					echo translate('active');
				} else {
					echo translate('inactive');
				}
			}
		$o->closeField();

		$o->openField(translate('blocking').':');
			if (isset($trigger['data']['settings'])) {
				$blocking = array_get_index($trigger['data']['settings'], 'blocking', FALSE);
			} else {
				$blocking = FALSE;
			}
			if ($write_access) {
				check_box($prefix.'[info][blocking]', '1', $blocking, '', 'id="info_blocking"');
				label(translate('block_other_triggers_on_failure'), 'info_blocking');
			} else {
				if ($blocking) {
					echo translate('yes');
				} else {
					echo translate('no');
				}
			}
		$o->closeField();


		$o->openField(translate('name').':');
			$name = array_get_index($trigger, 'name', '');
			if ($write_access) {
				text_box($prefix.'[info][name]', $name, 40);
			} else {
				echo $name;
			}
		$o->closeField();

		$o->openField(translate('description').':');
			$description = array_get_index($trigger, 'description', '');
			if ($write_access) {
				text_area($prefix.'[info][description]', $description, 40, 5);
			} else {
				echo $description;
			}
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
		$o->openSection(translate('events'));
		$o->openField('');
		if ($write_access) {
			$o->sectionNote(translate('trigger_events_list'));
		}

		// paint a tickbox list of all the events in the system
		$events_selected = FALSE;
		$event_prefix = $prefix.'[events]';
		foreach ($event_type_list as $e_type => $e_name) {
			if (isset($events[$e_type])) {
				$checked = TRUE;
			} else {
				$checked = FALSE;
			}

			$events_selected = $checked || $events_selected;

			if ($write_access) {
				check_box($event_prefix.'[]', $e_type, $checked);
				echo $e_name.'<br />';
			} else {
				if ($checked) echo $e_name.'<br />';
			}
		}

		if (!$write_access && !$events_selected) {
			echo '<strong>'.translate('no_event_selected').'</strong>';
		}


		$o->closeSection();
		$o->openSection(translate('conditions'));
		$o->openField('');
		if ($write_access) {
			$o->sectionNote(translate('trigger_conditions_explanation'));
		}
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

				$interface = $tm->_getComponentInterface($type, $data, $data_prefix, $write_access);
				$type_name = $tm->_getComponentName($type);
				?>
				<div class="sq-backend-triggers-component-wrapper">
					<div class="sq-backend-triggers-component-header">
						<div class="sq-backend-triggers-component-type-info">
							<?php echo $type_name; ?>
						</div>
						<div class="sq-backend-triggers-component-controls">
						<?php
						if ($write_access) {
						?>
							<input type="checkbox" name="<?php echo $delete_prefix; ?>" >
							<?php echo label(translate('delete'), $delete_prefix); ?>
								&nbsp;&nbsp;&nbsp;&nbsp;
							<a href="#" onclick="moveUp(this.parentNode.parentNode.parentNode); conditionOrdererObject.moveUp(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a>
							<a href="#" onclick="moveDown(this.parentNode.parentNode.parentNode); conditionOrdererObject.moveDown(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a>
						<?php
						} else {
							echo '&nbsp;';
						}
						?>
						</div>
					</div>
					<div class="sq-backend-triggers-component-body">
						<input type="hidden" name="<?php echo $type_prefix; ?>" value="<?php echo $type; ?>" />
						<?php echo $interface; ?>
					</div>
				</div>
				<?php
				$i++;
			}//end foreach
			?>
			</div>
			<?php  if ($write_access) {
			?>
			<script type="text/javascript">var conditionOrdererObject = new componentOrderer('conditionOrderer',<?php echo $i; ?>)</script>
			<input type="hidden" id="conditionOrderer" name="<?php echo $condition_order_prefix; ?>" />
			<!-- Adding a New Condition -->
			<div class="sq-backend-triggers-new-component-selector">
				<?php echo translate('add_new_condition_type'); ?>:
					<?php
					$select_list = Array(' '=>' ---- '.translate('select_to_add').'  ---- ');
					$removed = FALSE;
					foreach ($condition_type_list as $type => $name) {
						if (!$multiple_status[$type] && $type_instance_count[$type] != 0) {
							$removed = TRUE;
							continue;
						}
						$select_list[$type] = $name;
					}

					combo_box($prefix.'[new_condition]', $select_list, FALSE, ' ');
					if ($removed) {
						echo ' <span class="sq-backend-warning">'.translate('some_conditions_can_only_be_added_once').'</span>';
					}
					?>
			</div>
			<?php
			} else {

				if ($i < 0) echo translate('no_conditions_specified');

			}
			?>
		</div>

		<?php
		// actions are handled the same way as conditions
		$o->closeSection();
		$o->openSection(translate('actions'));
		$o->openField('');
		if ($write_access) {
			$o->sectionNote(translate('trigger_actions_explanation'));
		}

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
				$not_required = array_get_index($action, 'not_required', FALSE);
				$ignore_permissions = array_get_index($action, 'ignore_permissions', FALSE);

				$interface = $tm->_getComponentInterface($type, $data, $data_prefix, $write_access);
				$type_name = $tm->_getComponentName($type);
				?>
				<div class="sq-backend-triggers-component-wrapper">
					<div class="sq-backend-triggers-component-header">
						<div class="sq-backend-triggers-component-type-info">
							<?php echo $type_name; ?>
						</div>
						<div class="sq-backend-triggers-component-controls">
							<?php
							if ($write_access) {
								?>
								<input type="checkbox" name="<?php echo $delete_prefix; ?>"><?php echo label(translate('delete')); ?>
								&nbsp;&nbsp;&nbsp;&nbsp;
								<a href="#" onclick="moveUp(this.parentNode.parentNode.parentNode); actionOrdererObject.moveUp(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a>
								<a href="#" onclick="moveDown(this.parentNode.parentNode.parentNode); actionOrdererObject.moveDown(<?php echo $i; ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a>
								<?php
							} else {
								echo '&nbsp;';
							}
							?>
						</div>
					</div>
					<div class="sq-backend-triggers-component-global-options">
						<?php
						if (!$write_access) {
							$extras = 'disabled="disabled"';
						} else {
							$extras = '';
						}

						check_box($ignore_permissions_prefix, 1, $ignore_permissions, NULL, $extras);
						label(translate('ignore_permissions'), $ignore_permissions_prefix);

						check_box($not_required_prefix, 1, $not_required, NULL, $extras);
						label(translate('not_required'), $not_required_prefix);
						?>
					</div>
					<div class="sq-backend-triggers-component-body">
						<input type="hidden" name="<?php echo $type_prefix; ?>" value="<?php echo $type; ?>" />
						<?php echo $interface; ?>
					</div>
				</div>
				<?php
				$i++;
			}//end foreach
			if ($write_access) {
				?>
				<script type="text/javascript">var actionOrdererObject = new componentOrderer('actionOrderer',<?php echo $i; ?>)</script>
				<input type="hidden" id="actionOrderer" name="<?php echo $action_order_prefix; ?>" />
				<!-- Add a New Action -->
				<div class="sq-backend-triggers-new-component-selector">
					<?php echo translate('add_new_action_type'); ?>:
					<select name="<?php echo $prefix; ?>[new_action]" >
						<option value="" > ---- <?php echo translate('select_to_add'); ?> ---- </option>
						<?php
						foreach ($action_type_list as $a_type => $a_name) {
							echo '<option value="'.$a_type.'" >'.$a_name.'</option>';
						}
						?>
					</select>
				</div>
				<?php
			} else {

				if ($i < 0) echo translate('no_action_specified');

			}
			?>
			</div>
		</div>
		<?php

		$o->closeSection();

		if ($write_access) {
			$o->openSection(translate('other'));
			$o->openField('');
				check_box($prefix.'[finish]');
				label(translate('trigger_finish_editing'), $prefix.'[finish]');
			$o->closeSection();
		}

		return TRUE;

	}//end _paintSubScreenEdit()


	/**
	* Process Triggers Screen Main Controller
	*
	* This function determines how to process the input from the backend interface
	*
	* @param object	&$tm	trigger manager
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTriggerScreen(&$tm, &$o, $prefix)
	{
		$write_access = $tm->writeAccess('attributes');
		$status = FALSE;

		$request_data = array_get_index($_REQUEST, $prefix, FALSE);

		if (!$request_data) {
			// Can't do anything without the REQUEST vars, fail
			return FALSE;
		}

		$screen = array_get_index($request_data, 'sub_screen', 'list');

		if (isset($request_data['edit'])) $screen = 'edit';

		switch ($screen) {
			case 'list':
				$status = $this->_processSubScreenList($tm, $request_data, $write_access);
			break;

			case 'edit':
				$status = $this->_processSubScreenEdit($tm, $request_data, $write_access);
			break;
			default:
				return $status;
		}

		return $status && $write_access;

	}//end processTriggerScreen()


	/**
	* Process input submitted from the List subscreen
	*
	* This function determines how to process the input from the backend interface
	*
	* @param object		&$tm			trigger manager
	* @param array		$request_data	data extracted from the request variables
	* @param boolean	$write_access	Write access
	*
	* @return void
	* @access private
	*/
	function _processSubScreenList(&$tm, $request_data, $write_access=FALSE)
	{
		// edit request
		$edit_id = array_get_index($request_data, 'edit_id', FALSE);

		// add new trigger
		$new_name = array_get_index($request_data, 'add_name', FALSE);
		if ($new_name && !empty($new_name)) {
			$trigger = $tm->_getEmptyTrigger();
			$trigger['name'] = $new_name;

			$new_id = $tm->_saveTrigger($trigger);
			if (!empty($new_id)) {
				// make sure the newly added trigger is to be edited next
				$edit_id = $new_id;
			}
		}

		if ($edit_id) $this->_tmp['edit_id'] = $edit_id;

		// delete the requested triggers
		$to_delete = array_get_index($request_data, 'delete', FALSE);
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

	}//end _processSubScreenList()


	/**
	* Process input submitted from the Edit subscreen
	*
	* This function determines how to process the input from the backend interface
	*
	* @param object		&$tm			trigger manager
	* @param array		$request_data	data extracted from the request variables
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access private
	*/
	function _processSubScreenEdit(&$tm, $request_data, $write_access=FALSE)
	{
		$edit_data = array_get_index($request_data, 'edit', Array());
		$edit_id = array_get_index($edit_data, 'id');

		if (empty($edit_data) || empty($edit_id)) {
			return FALSE;
		}

		$trigger = $tm->_loadTrigger($edit_id);
		if (empty($trigger)) return FALSE;

		// set this to make sure we return to edit screen of the same trigger
		if (!isset($edit_data['finish'])) {
			$this->_tmp['edit_id'] = $edit_id;
		}

		if (!$write_access) return FALSE;

		$edit_info = array_get_index($edit_data, 'info');

		// setup basic trigger info
		$trigger['name']                         = array_get_index($edit_info, 'name');
		$trigger['description']                  = array_get_index($edit_info, 'description');
		$trigger['active']                       = array_get_index($edit_info, 'active');
		$trigger['data']['settings']['blocking'] = array_get_index($edit_info, 'blocking');

		// events
		$raw_events = array_get_index($edit_data, 'events', Array());
		$processed_events = Array();
		foreach ($raw_events as $one_event_type) {
			$processed_events[$one_event_type] = $one_event_type;
		}

		// conditions
		$raw_conditions = array_get_index($edit_data, 'conditions');
		$condition_order = array_get_index($edit_data, 'condition_order');
		if (!empty($condition_order)) {
			$order_array = explode(',', $condition_order);
			foreach ($order_array as $position) {
				$new_raw_conditions[] = $raw_conditions[$position];
			}
			$raw_conditions = $new_raw_conditions;
		}
		$new_condition_type = array_get_index($edit_data, 'new_condition');
		if (!empty($new_condition_type)) {
			$new_condition['type'] = $new_condition_type;
			$new_condition['new']  = TRUE;

			$raw_conditions[] = $new_condition;
		}
		$processed_conditions = $this->_processRawConditionSet($tm, $raw_conditions);

		// actions
		$raw_actions = array_get_index($edit_data, 'actions');
		$action_order = array_get_index($edit_data, 'action_order');
		if (!empty($action_order)) {
			$order_array = explode(',', $action_order);
			foreach ($order_array as $position) {
				$new_raw_actions[] = $raw_actions[$position];
			}
			$raw_actions = $new_raw_actions;
		}
		$new_action_type = array_get_index($edit_data, 'new_action');
		if (!empty($new_action_type)) {
			$new_action['type'] = $new_action_type;
			$new_action['new']  = TRUE;

			$raw_actions[] = $new_action;
		}

		$processed_actions = $this->_processRawActionSet($tm, $raw_actions);

		if (empty($processed_events) || empty($processed_actions) || empty($trigger['active'])) {
			$trigger['save_hash'] = FALSE;
		}

		$trigger['data']['events']     = $processed_events;
		$trigger['data']['conditions'] = $processed_conditions;
		$trigger['data']['actions']    = $processed_actions;

		$tm->_saveTrigger($trigger);

		return TRUE;

	}//end _processSubScreenEdit()


	/**
	* Process a set of raw conditions
	*
	* Conditions that were supplied by the interface, plus the newly added one
	*
	* @param object	&$tm			trigger manager
	* @param array	$raw_components	a list of submitted conditions
	*
	* @return array
	* @access private
	*/
	function _processRawConditionSet(&$tm, $raw_components)
	{
		$processed_components = Array();

		if (empty($raw_components)) {
			return $processed_components;
		}

		$i = 1;
		foreach ($raw_components as $component) {
			$error = FALSE;
			if (isset($component['delete'])) continue;
			$settings = Array();

			$type = array_get_index($component, 'type');
			if (empty($type) || !$tm->_loadComponent($type)) {
				// invalid type is silently ignored
				continue;
			}

			$data = array_get_index($component, 'data', Array());
			if (!array_get_index($component, 'new', FALSE)) {
				eval('$error = '.$type.'::processInterface($settings, $data);');
			}

			if (FALSE !== $error) {
				trigger_localised_error('CORE0217', E_USER_NOTICE, $i, $type, $error);
			}

			$processed_components[] = Array(
										'type'	=> $type,
										'data'	=> $settings,
									  );

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
	* @param object	&$tm			trigger manager
	* @param array	$raw_components	a list of submitted actions
	*
	* @return array
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
			$error = FALSE;
			if (isset($component['delete'])) continue;

			$settings = Array();

			$type = array_get_index($component, 'type');
			if (empty($type) || !$tm->_loadComponent($type)) {
				continue; // invalid type is silently ignored
			}


			$data = array_get_index($component, 'data', Array());
			if (!array_get_index($component, 'new', FALSE)) {
				eval('$error = '.$type.'::processInterface($settings, $data);');
			}

			$processed_components[] = Array(
										'type'					=> $type,
										'data'					=> $settings,
										'not_required'			=> isset($component['not_required']),
										'ignore_permissions'	=> isset($component['ignore_permissions']),
									  );

			if (FALSE !== $error) {
				trigger_localised_error('CORE0130', E_USER_NOTICE, $i, $type, $error);
			}
			$i++;
		}

		return $processed_components;

	}//end _processRawActionSet()


	/**
	* Paint Trigger List sub screen
	*
	* @param object		&$tm			trigger manager
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param boolean	$write_access	a flag denoting whether we're editing or not
	*
	* @return boolean
	* @access private
	*/
	function _paintSubScreenList(&$tm, &$o, $prefix, $write_access)
	{
		$o->closeSection();
		$list = $tm->_getTriggerInfoList();


		if ($write_access) {
			$o->openSection(translate('add_new_trigger'));
				$o->openField(translate('new_trigger_name'));
					text_box($prefix.'[add_name]', '');
					$o->note(translate('trigger_add_new'));
				$o->closeField();
			$o->closeSection();
		}

		$o->openSection(translate('trigger_list'));
		$o->openField('');
		?>
		<script type="text/javascript">
			//<![CDATA[
			var deleteCheckedCount = 0;

			function confirmDelete()
			{
				var result = true;

				if (deleteCheckedCount > 0) {
					result = confirm (js_translate('confirm_delete_trigger', deleteCheckedCount));
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
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('id'); ?></td>
				<td class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('name'); ?></td>
				<td class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('description'); ?></td>
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('status'); ?></td>
				<?php if ($write_access) {
				?>
				<td width="50" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question'); ?></td>
				<?php
					}
				?>
			</tr>

		<?php

		$screen_url = $tm->getBackendHref('triggers');

		foreach ($list as $trigger) {
			$trigger_url = $screen_url.'&'.$prefix.'[edit_id]='.$trigger['id'];
			$active_text = $trigger['active'] ? '<strong>'.translate('active').'</strong>' : '<span style="color: red">'.translate('inactive').'</span>';
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo $trigger['id']; ?></td>
				<td class="sq-backend-table-cell"><a href="<?php echo $trigger_url; ?>"><?php echo $trigger['name']; ?></a></td>
				<td class="sq-backend-table-cell"><?php echo $trigger['description']; ?></td>
				<td class="sq-backend-table-cell"><strong><?php echo $active_text; ?></strong></td>
				<?php if ($write_access) {
				?>
				<td class="sq-backend-table-cell"><input type="checkbox" name="<?php echo $prefix; ?>[delete][<?php echo $trigger['id']; ?>]"  onClick="checkDelete(this.checked)" ></td>
				<?php
					}
				?>
			</tr>
		<?php
			}
		?>
		</table>
		<?php

		$o->closeSection();
		$o->addOnsubmit('return confirmDelete();');

		return TRUE;

	}//end _paintSubScreenList()


	/**
	* Display the Batching Triggers Screen
	*
	* @param object	&$tm	trigger manager
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBatching(&$tm, &$o, $prefix)
	{
		$write_access = $tm->writeAccess('attributes');
		if ($write_access) {
			$o->openField(translate('target_node'));
			asset_finder($prefix.'[target_node]', array_get_index($this->_tmp, 'target_id'));

			$o->openField(translate('children'));
			check_box($prefix.'[include_children]', 'yes', array_get_index($this->_tmp, 'include_children', TRUE));
			label(translate('include_children_question'), $prefix.'[include_children]');

			$o->openField(translate('events'));
			$event_type_list = $tm->_getEventList();
			$events = array_get_index($this->_tmp, 'selected_events');
			// paint a tickbox list of all the events in the system
			$event_prefix = $prefix.'[selected_events]';
			foreach ($event_type_list as $e_type => $e_name) {
				if (isset($events[$e_type])) {
					$checked = TRUE;
				} else {
					$checked = FALSE;
				}
				check_box($event_prefix.'['.$e_type.']', $e_type, $checked);
				label($e_name, $event_prefix.'['.$e_type.']');
				?><br /> <?php
			}
			return TRUE;
		} else {
			return FALSE;
		}

	}//end paintBatching()


	/**
	* Process Batching Triggers Screen
	*
	* Prepares and fires the trigger batching HIPO job
	*
	* @param object	&$tm	trigger manager
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processBatching(&$tm, &$o, $prefix)
	{
		$error = FALSE;

		$request_data = array_get_index($_REQUEST, $prefix, Array());

		$target_node = array_get_index($request_data, 'target_node');
		$target_id   = array_get_index($target_node,  'assetid');
		$events      = array_get_index($request_data, 'selected_events', Array());
		$propagate   = isset($request_data['include_children']);

		$this->_tmp['target_id']        = $target_id;
		$this->_tmp['selected_events']  = $events;
		$this->_tmp['include_children'] = $propagate;

		if (empty($target_id)) {
			trigger_localised_error('CORE0132', E_USER_WARNING);
			$error = TRUE;
		}

		if (empty($events)) {
			trigger_localised_error('CORE0126', E_USER_WARNING);
			$error = TRUE;
		}

		if ($error) return FALSE;

		// grab all the children of the target if applicable, then add the target itself
		$all_targets = Array();
		if ($propagate) {
			$all_targets = $GLOBALS['SQ_SYSTEM']->am->getChildren($target_id);
		}
		$target_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($target_id);
		$all_targets[$target_id] = $target_asset->type();

		// convert from id->type to index->id
		$all_targets = array_keys($all_targets);

		// start up the HIPO
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'				=> $all_targets,
					'events'				=> $events,
					'codename_target'		=> $target_id,
					'codename_propagate'	=> ($propagate ? '1' : '0'),
				);
		$hh->queueHipo('hipo_job_trigger_batch_events', $vars);

		return TRUE;

	}//end processBatching()


}//end class

?>
