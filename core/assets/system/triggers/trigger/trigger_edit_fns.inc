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
* $Id: trigger_edit_fns.inc,v 1.1 2006/03/30 23:19:15 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Trigger_Edit_Fns
*
* @author Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trigger_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		// disable all static screens
		$this->static_screens = Array();

		// enable just the details
		$this->static_screens['details']['name'] = translate('details');
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint Subscreen for Edit Triggers
	*
	* @param object		&$asset			the asset being printed
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param boolean	$write_access	flag whether it is
	*
	* @return void
	* @access private
	*/
	function paintTriggerScreen(&$asset, &$o, $prefix, $write_access=FALSE)
	{
		// adapted from trigger manager
		$prefix .= '[edit]';
		$trigger =& $asset;

		$tm =& $this->_getTriggerManager($trigger);
		if (is_null($tm)) return FALSE;

		$write_access = $asset->writeAccess('attributes');

		// id of the trigger
		$trigger_id = $asset->id;

		$o->openField('');
		$o->sectionNote(translate('trigger_single_subscreen_explanation'));

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

			// do not display when creating a new trigger
			if ($trigger->id) {
				$o->openField(translate('id'));
					echo($trigger->id);
					hidden_field($prefix.'[id]', $trigger->id);
				$o->closeField();
			}


			$o->openField(translate('status'));
				$active_status = $trigger->attr('active');
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


			$o->openField(translate('blocking'));
				$trigger_data = $trigger->attr('data');
				if (isset($trigger_data['settings'])) {
					$blocking = array_get_index($trigger_data['settings'], 'blocking', FALSE);
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


			$o->openField(translate('name'));
				$name = $trigger->attr('name');
				if ($write_access) {
					text_box($prefix.'[info][name]', $name, 40);
				} else {
					echo $name;
				}
			$o->closeField();


			$o->openField(translate('description').':');
				$description = $trigger->attr('description');
				if ($write_access) {
					text_area($prefix.'[info][description]', $description, 40, 5);
				} else {
					echo $description;
				}
			$o->closeField();

		$o->closeSection();

		// initialization
		$trigger_data = $trigger->attr('data');

		$events     = array_get_index($trigger_data, 'events', Array());
		$conditions = array_get_index($trigger_data, 'conditions', Array());
		$actions    = array_get_index($trigger_data, 'actions', Array());

		// information from the trigger manager
		$event_type_list     = $tm->_getEventList();
		$condition_type_list = $tm->_getConditionList();
		$action_type_list    = $tm->_getActionList();

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
					if ($checked) echo '<li>'.$e_name.'</li>';
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

		$o->closeField();

		return TRUE;

	}//end paintTriggerScreen()


	/**
	* Process input submitted from the Edit subscreen
	*
	* @param object	&$asset	trigger
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access private
	*/
	function processTriggerScreen(&$asset, &$o, $prefix)
	{
		// these functions have been adapted from the
		// trigger_manger edit interface
		$trigger =& $asset;

		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;

		$request_data = array_get_index($_REQUEST, $prefix, FALSE);

		$edit_data = array_get_index($request_data, 'edit', Array());
		if (empty($edit_data)) return FALSE;

		// get the trigger manager
		$tm =& $this->_getTriggerManager($trigger);
		if (is_null($tm)) return FALSE;

		$trigger_data = Array();
		$edit_info = array_get_index($edit_data, 'info');

		// setup basic trigger info
		$trigger->setAttrValue('name', array_get_index($edit_info, 'name'));
		$trigger->setAttrValue('description', array_get_index($edit_info, 'description'));
		$trigger->setAttrValue('active', array_get_index($edit_info, 'active'));

		$trigger_data['settings']['blocking'] = array_get_index($edit_info, 'blocking');

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

		// compile trigger data
		$trigger_data['events']     = $processed_events;
		$trigger_data['conditions'] = $processed_conditions;
		$trigger_data['actions']    = $processed_actions;

		// save trigger data
		$trigger->setAttrValue('data', $trigger_data);

		return TRUE;

	}//end processTriggerScreen()


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
	* Gets the trigger manager for the given asset.
	*
	* @param object	&$asset	The asset to get the trigger manager for
	*
	* @return object
	* @access private
	*/
	function &_getTriggerManager(&$asset)
	{
		$tm = NULL;

		$tmid = 0;
		if (isset($asset->tmid) && !empty($asset->tmid)) {
			// asset already has a trigger manager
			$tmid = $asset->tmid;
		} else {
			// maybe being created under a trigger manager
			$parent_assetid = array_get_index($_REQUEST, 'parent_assetid');
			if (!is_null($parent_assetid)) {
				$tmid = $parent_assetid;
			}
		}

		$tm =& $GLOBALS['SQ_SYSTEM']->am->getAsset($tmid, 'trigger_manager');
		if (!is_null($tm) && empty($asset->tmid)) {
			$asset->setTriggerManager($tmid);
		}
		return $tm;

	}//end _getTriggerManager()


}//end class

?>
