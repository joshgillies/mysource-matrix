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
* $Id: report_triggers.inc,v 1.4 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/simple_report/simple_report.inc';

/**
* Triggers Report
*
* Purpose
* 	Provides a report of the total number of active and inactive triggers, and shows a list of triggers
*	with the most events, conditions and actions
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Report_Triggers extends Simple_Report
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Paint the interface for this report
	*
	* @param object	&$o		reference to the outputter object
	* @param string	$type_code	type code of this tool asset
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintReport(Backend_Outputter &$o, $type_code)
	{
		$report = Report_Triggers::loadReport($type_code);

		if (empty($report)) {
			Report_Triggers::paintReportNotGenerated($type_code);
			return TRUE;
		}

		$generated = $report['generated'];
		$report = $report['report_data'];

		$row_format = '<tr>
				<td class="sq-backend-table-cell">
					<img src="%s">
				</td>
				<td class="sq-backend-table-cell">
					%s
				</td>
				<td class="sq-backend-table-cell" align="right">
					%s
				</td>
				<td class="sq-backend-table-cell" align="right">
					%s
				</td>
				<td class="sq-backend-table-cell" align="right">
					%s
				</td>
				</tr>';

		$rows_string = '';

		$trigger_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');

		foreach ($report as $key => $trigger_info) {
			if (($key == 'num_triggers') || ($key == 'num_active_triggers')) continue;

			foreach ($trigger_info as $trigger_weighted_info) {
				$icon = 'exit_limbo';
				if ($trigger_weighted_info['active'] == 1) {
					$icon = 'preview';
				}

				$rows_string .= sprintf($row_format, sq_web_path('lib').'/web/images/icons/header/'.$icon.'.png', $trigger_weighted_info['name'].' (Id: '.$trigger_manager->id.':'.$trigger_weighted_info['id'].')', $trigger_weighted_info['events'], $trigger_weighted_info['conditions'], $trigger_weighted_info['actions']);
			}
		}

		$table_format = '<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header">State</th>
					<th class="sq-backend-table-header">Trigger</th>
					<th class="sq-backend-table-header" align="right">Events</th>
					<th class="sq-backend-table-header" align="right">Conditions</th>
					<th class="sq-backend-table-header" align="right">Actions</th>
				</tr>
				%s
				</table>';

		$table = sprintf($table_format, $rows_string);

		Report_Triggers::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));

		?>
<br>
<table class="sq-backend-table">
		<?php
		if ($report['num_triggers'] > 0) {
		?>
<tr>
	<th class=sq-backend-table-header">Total Active Triggers</th>
	<td class="sq-backend-table-cell" align="right"><?php echo $report['num_active_triggers']; ?></td>
</tr>
<tr>
	<th class=sq-backend-table-header">Total Triggers</th>
	<td class="sq-backend-table-cell" align="right"><?php echo $report['num_triggers']; ?></td>
</tr>
		<?php
		} else {
		?>
<tr>
	<td class="sq-backend-table-cell">There are no Triggers in the system</td>
</tr>
		<?php
		}
		?>
</table>
<br>
		<?php

		if (!empty($rows_string)) echo $table;

	}//end paintReport()


	/**
	* Process this report.  Return false if any errors.
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this asset
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processReport(Backend_Outputter &$o, $type_code)
	{
		$today = time();
		$trigger_report = Report_Triggers::_getTriggerInfo();

		$report = Array(
				'generated'     => $today,
				'report_data'   => $trigger_report,
			  );

		Report_Triggers::saveReport($type_code, $report);

		return TRUE;

	}//end processReport()

	/**
	* Returns a list of the number of events, conditions, and actions for each Trigger in the system
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getTriggerInfo()
	{
		$trigger_info = MatrixDAL::executeAll('core', 'getTriggerList');
		$trigger_info_sorted = Array();

		$num_triggers = count($trigger_info);
		$num_active_triggers = 0;

		foreach ($trigger_info as $trigger) {
			$trigger_data = unserialize($trigger['data']);

			$num_events = count($trigger_data['events']);
			$num_conditions = count($trigger_data['conditions']);
			$num_actions = count($trigger_data['actions']);

			$weighting = ($num_events * 20) + ($num_conditions * 10) + $num_actions;

			$trigger_info_sorted[$weighting][] = Array(
								'id'			=> $trigger['id'],
								'name'			=> $trigger['name'],
								'events'		=> $num_events,
								'conditions'	=> $num_conditions,
								'actions'		=> $num_actions,
								'active'		=> $trigger['active'],
							     );

			if ($trigger['active']) $num_active_triggers++;
		}

		ksort($trigger_info_sorted);

		$trigger_info_sorted['num_active_triggers'] = $num_active_triggers;
		$trigger_info_sorted['num_triggers'] = $num_triggers;

		return $trigger_info_sorted;

	}//end _getTriggerInfo()


}//end class
?>
