<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: report_triggers.inc,v 1.1 2008/05/07 05:54:05 mbrydon Exp $
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
* @version $Revision: 1.1 $
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
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this tool asset
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintReport(&$o, $type_code)
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

		foreach ($report as $trigger_info) {
			foreach ($trigger_info as $trigger_weighted_info) {
				$rows_string .= sprintf($row_format, $trigger_weighted_info['name'].' (Id: '.$trigger_manager->id.':'.$trigger_weighted_info['id'].')', $trigger_weighted_info['events'], $trigger_weighted_info['conditions'], $trigger_weighted_info['actions']);
			}
		}

		$table_format = '<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header">Trigger</th>
					<th class="sq-backend-table-header" align="right">Events</th>
					<th class="sq-backend-table-header" align="right">Conditions</th>
					<th class="sq-backend-table-header" align="right">Actions</th>
				</tr>
				%s
				</table>';

		$table = sprintf($table_format, $rows_string);

		Report_Triggers::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));
		echo $table;

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
	public static function processReport(&$o, $type_code)
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
                                                                'id'            => $trigger['id'],
                                                                'name'          => $trigger['name'],
                                                                'events'        => $num_events,
                                                                'conditions'    => $num_conditions,
                                                                'actions'       => $num_actions,
                                                             );
                }

                ksort($trigger_info_sorted);

                return $trigger_info_sorted;

        }//end _getTriggerInfo()


}//end class
?>
