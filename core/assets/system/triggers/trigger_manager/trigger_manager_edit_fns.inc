<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_manager_edit_fns.inc,v 1.27 2012/08/30 01:09:16 ewang Exp $
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
* @version $Revision: 1.27 $
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

			$o->sectionNote(translate('trigger_subscreen_add_or_edit'));

			// we now only display a list of triggers for editing
			$this->_paintSubScreenList($tm, $o, $prefix, $write_access);

		$o->closeField();
		return $write_access;

	}//end paintTriggerScreen()


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
		return $tm->writeAccess('attributes');

	}//end processTriggerScreen()


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
			</tr>

		<?php

		foreach ($list as $trigger) {
			$trigger_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(implode(':', Array($tm->id, $trigger['id'])));
			if (is_null($trigger_asset)) continue;

			$trigger_url = $trigger_asset->getBackendHref('details');
			$active_text = $trigger['active'] ? '<strong>'.translate('active').'</strong>' : '<span style="color: red">'.translate('inactive').'</span>';
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo $trigger['id']; ?></td>
				<td class="sq-backend-table-cell"><a href="<?php echo $trigger_url; ?>"><?php echo $trigger['name']; ?></a></td>
				<td class="sq-backend-table-cell"><?php echo $trigger['description']; ?></td>
				<td class="sq-backend-table-cell"><strong><?php echo $active_text; ?></strong></td>
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
			$o->openField(translate('trigger_ids'));
			multiple_asset_finder($prefix.'[trigger_ids]', array_get_index($this->_tmp, 'trigger_ids', Array()), Array('trigger' => 'I'));
			$o->note(translate('trigger_ids_note'));

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

		$triggers = array_get_index($request_data, 'trigger_ids');
		$trigger_ids = Array();
		foreach ($triggers as $trigger_id) {
			if ($trigger_id['assetid']) {
				$trigger_ids[] = substr($trigger_id['assetid'], strpos($trigger_id['assetid'], ':') + 1);
			}
		}

		$events      = array_get_index($request_data, 'selected_events', Array());
		$propagate   = isset($request_data['include_children']);

		$this->_tmp['target_id']        = $target_id;
		$this->_tmp['trigger_ids'] = $trigger_ids;
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
		$target_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($target_id);
		$all_targets[$target_id] = $target_asset->type();

		// convert from id->type to index->id
		$all_targets = array_keys($all_targets);

		// start up the HIPO
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'				=> $all_targets,
					'events'				=> $events,
					'event_parameters'		=> Array('specified_trigger_ids' => $trigger_ids),
					'codename_target'		=> $target_id,
					'codename_propagate'	=> ($propagate ? '1' : '0'),
				);
		$hh->queueHipo('hipo_job_trigger_batch_events', $vars);

		return TRUE;

	}//end processBatching()


}//end class

?>
