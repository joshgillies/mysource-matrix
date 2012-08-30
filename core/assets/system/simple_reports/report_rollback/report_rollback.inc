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
* $Id: report_rollback.inc,v 1.7 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/simple_report/simple_report.inc';

/**
* Rollback Report
*
* Purpose
* 	Provides a report of assets with rollback data
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Report_Rollback extends Simple_Report
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
		$report = Report_Rollback::loadReport($type_code);

		if (empty($report)) {
			Report_Rollback::paintReportNotGenerated($type_code);
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
				</tr>';

		$current_asset_rollback_table = '<br /><table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header" width="45%%">Rollback asset entries currently in the system</th>
							<td align="right">'.$report['num_asset_rb_entries_today'].'</td>
						</tr>
						</table>';

		$current_link_rollback_table = '<br /><table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header" width="45%%">Rollback link entries currently in the system</th>
							<td align="right">'.$report['num_link_rb_entries_today'].'</td>
						</tr>
						</table>';

		$row_string = '';
		$asset_report = $report['asset_date_report'];
		foreach ($asset_report as $date_description => $date_data) {
			$row_string .= sprintf($row_format, $date_description, $date_data['num_assets']);
		}

		$table_format = '<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header">Period</th>
					<th class="sq-backend-table-header" width="35%%" align="right">Rollback %s entries in the system</th>
				</tr>
				%s
				</table>';

		$asset_report_table = sprintf($table_format, 'asset', $row_string);

		$row_string = '';
		$link_report = $report['link_date_report'];
		foreach ($link_report as $date_description => $date_data) {
			$row_string .= sprintf($row_format, $date_description, $date_data['num_assets']);
		}

		$link_report_table = sprintf($table_format, 'link', $row_string);

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$table_format = '<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header">Icon</th>
					<th class="sq-backend-table-header">Asset ID</th>
					<th class="sq-backend-table-header">Name</th>
					<th class="sq-backend-table-header">First Rollback Entry</th>
					<th class="sq-backend-table-header">Last Rollback Entry</th>
					<th class="sq-backend-table-header">Total Entries</th>
				</tr>
				%s
				</table>';

		$row_format = '<tr>
				<td class="sq-backend-table-cell">
					<image src="%s">
				</td>
				<td class="sq-backend-table-cell">
					%s
				</td>
				<td class="sq-backend-table-cell">
					%s
				</td>
				<td class="sq-backend-table-cell">
					%s
				</td>
				<td class="sq-backend-table-cell">
					%s
				</td>
				<td class="sq-backend-table-cell" align="right">
					%s
				</td>
				</tr>';


		$row_string = '';
		$most_asset_rollback_entries_report = $report['most_asset_rb_entries_report'];
		foreach ($most_asset_rollback_entries_report as $asset_data) {
			$row_string .= sprintf($row_format, $am->getAssetIconURL($asset_data['type_code']), $asset_data['assetid'],
						$asset_data['name'], $asset_data['first_rb_entry'], $asset_data['last_rb_entry'],
						$asset_data['total']);
		}

		$most_asset_rollback_entries_table = sprintf($table_format, $row_string);

		Report_Rollback::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));
		Report_Rollback::paintRecommendedTasks($o, $type_code, Array('tool_rollback_management'));

		echo '<br />'.$current_asset_rollback_table;
		echo $asset_report_table;

		if (!empty($rows_string)) {
			echo $most_asset_rollback_entries_table;
		}

		echo '<br />'.$current_link_rollback_table;
		echo $link_report_table;

	}//end paintReport()


	/**
	* Process this report
	*
	* @param object	&$o		reference to the outputter object
	* @param string	$type_code	type code of this asset
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processReport(Backend_Outputter &$o, $type_code)
	{
		$today = time();

		$year = date('Y', $today);
		$month = date('m', $today);
		$date = date('d', $today);

		$last_month = str_pad(date('m', $today) - 1, 2, '0', STR_PAD_LEFT);

		$date_yesterday = date('Y-m-d', $today - (24*3600));
		$last_week_date = date('Y-m-d', $today - (24*3600*7));
		$last_month_date = date('Y-m-d', $today - (30*24*3600));
		$last_quarter_date = date('Y-m-d', $today - (90*24*3600));
		$last_semester_date = date('Y-m-d', $today - (180*24*3600));
		$last_year_date = date('Y-m-d', $today - (365*24*3600));

		$asset_date_report = Array(
                                                'Yesterday'                           => Array(
                                                                                                'date'          => $date_yesterday,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Last week'  => Array(
                                                                                                'date'          => $last_week_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Last month' => Array(
                                                                                                'date'          => $last_month_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Last three months'      => Array(
                                                                                                'date'          => $last_quarter_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Last six months'        => Array(
                                                                                                'date'          => $last_semester_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Last year'          => Array(
                                                                                                'date'          => $last_year_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                        );

		$link_date_report = $asset_date_report;

		$assets_since_last_period = Array();

		$num_asset_rb_entries_today = Report_Rollback::_getTotalAssetRollbackEntries();
		$num_link_rb_entries_today = Report_Rollback::_getTotalLinkRollbackEntries();

		// Asset Rollback report
		$total = 0;
		$last_period = 0;
		$remove_date_entries = Array();
		foreach ($asset_date_report as $date_description => $date_data) {
			if ($last_period == 1) {
				$last_period++;
			} else if ($last_period > 1) {
				$remove_date_entries[] = $date_description;
				continue;
			}

			$num_assets = Report_Rollback::_getTotalAssetRollbackEntries($date_data['date']);
			$asset_date_report[$date_description]['num_assets'] = $num_asset_rb_entries_today - $num_assets;
			$total += $asset_date_report[$date_description]['num_assets'];
			if ($total == $num_asset_rb_entries_today) {
				$last_period++;
			}
		}

		foreach ($remove_date_entries as $date_description) {
			unset($asset_date_report[$date_description]);
		}


		// Links Rollback report
		$total = 0;
		$last_period = 0;
		$remove_date_entries = Array();
		foreach ($link_date_report as $date_description => $date_data) {
			if ($last_period == 1) {
				$last_period++;
			} else if ($last_period > 1) {
				$remove_date_entries[] = $date_description;
				continue;
			}

			$num_assets = Report_Rollback::_getTotalLinkRollbackEntries($date_data['date']);
			$link_date_report[$date_description]['num_assets'] = $num_link_rb_entries_today - $num_assets;
			$total += $link_date_report[$date_description]['num_assets'];
			if ($total == $num_link_rb_entries_today) {
				$last_period++;
			}
		}

		foreach ($remove_date_entries as $date_description) {
			unset($link_date_report[$date_description]);
		}

		// Most Asset Rollback entries

		$most_asset_rb_entries_report = Report_Rollback::_getMostAssetRollbackEntries();
			
		$report = Array(
				'generated'     => $today,
				'report_data'   => Array(
							'num_asset_rb_entries_today'	=> $num_asset_rb_entries_today,
							'num_link_rb_entries_today'	=> $num_link_rb_entries_today,
							'asset_date_report'		=> $asset_date_report,
							'link_date_report'		=> $link_date_report,
							'most_asset_rb_entries_report'	=> $most_asset_rb_entries_report,
						   ),
			  );

		Report_Rollback::saveReport($type_code, $report);

		return TRUE;

	}//end processReport()


	/**
	* Returns the top ten assets with the most asset rollback entries
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getMostAssetRollbackEntries()
	{
		$db = MatrixDAL::getDb();
		$sql = 'SELECT r.assetid, a.name, a.type_code, MIN(r.sq_eff_from) AS first_rb_entry, MAX(r.sq_eff_to) AS last_rb_entry, '.
			'COUNT(*) AS total '.
			'FROM sq_rb_ast r, sq_ast a '.
			'WHERE a.assetid = r.assetid '.
			'GROUP BY r.assetid, a.name, a.type_code '.
			'ORDER BY total DESC, first_rb_entry DESC, last_rb_entry DESC';
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 10);

		$rows = MatrixDAL::executeSqlAssoc($sql);

		return $rows;

	}//end _getMostAssetRollbackEntries()


	/**
	* Returns the number of asset rollback entries on or after the specified date
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getTotalAssetRollbackEntries($after_date='')
	{
		$after_date_query = '';
		if ($after_date != '') {
			$after_date_query = ' WHERE sq_eff_from >= '.MatrixDAL::quote($after_date);
		}

		$db = MatrixDAL::getDb();
		$sql = 'SELECT COUNT(*) AS total '.
			'FROM sq_rb_ast'.$after_date_query;

		$num_entries = MatrixDAL::executeSqlOne($sql);

		return $num_entries;

	}//end _getNumAssetRollbackEntries()


	/**
	* Returns the number of link rollback entries on or after the specified date
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getTotalLinkRollbackEntries($after_date='')
	{
		$after_date_query = '';
		if ($after_date != '') {
			$after_date_query = ' WHERE sq_eff_from >= '.MatrixDAL::quote($after_date);
		}

		$db = MatrixDAL::getDb();
		$sql = 'SELECT COUNT(*) AS total '.
			'FROM sq_rb_ast_lnk'.$after_date_query;

		$num_entries = MatrixDAL::executeSqlOne($sql);

		return $num_entries;

	}//end _getTotalLinkRollbackEntries()


}//end class
?>
