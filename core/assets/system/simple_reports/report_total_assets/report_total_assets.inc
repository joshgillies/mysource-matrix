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
* $Id: report_total_assets.inc,v 1.1 2008/05/07 05:54:04 mbrydon Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/simple_report/simple_report.inc';

/**
* Total Assets Report
*
* Purpose
* 	Provides a total asset count for seven points in time, from today to one year ago
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Report_Total_Assets extends Simple_Report
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
		$report = Report_Total_Assets::loadReport($type_code);

		if (empty($report)) {
			Report_Total_Assets::paintReportNotGenerated($type_code);
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

		$current_day_assets_table = '<table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header" width="30%%">Assets currently in the system</th>
							<td align="right">'.$report['num_assets_today'].'</td>
						</tr>
					</table>';

		$row_string = '';
		$report = $report['date_report'];
		foreach ($report as $date_description => $date_data) {
			$row_string .= sprintf($row_format, $date_description, $date_data['num_assets']);
		}

		$table_format = '<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header">Date</th>
						<th class="sq-backend-table-header" width="35%%" align="right">New assets since last timeframe</th>
					</tr>
					%s
				</table>';

		$table = sprintf($table_format, $row_string);
		Report_Total_Assets::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));
		Report_Total_Assets::_paintSystemInstallDate();
		echo $current_day_assets_table;
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

		$year = date('Y', $today);
		$month = date('m', $today);
		$date = date('d', $today);

		$last_month = str_pad(date('m', $today) - 1, 2, '0', STR_PAD_LEFT);

		$date_yesterday = date('Y-m-d', $today - (24*3600));
		$last_week_date = date('Y-m-d', $today - (24*3600*7));
		$last_month_date = date('Y-m-d', $today - (30*24*3600*7));
		$last_quarter_date = date('Y-m-d', $today - (90*24*3600*7));
		$last_semester_date = date('Y-m-d', $today - (180*24*3600*7));
		$last_year_date = date('Y-m-d', $today - (365*24*3600*7));

		$date_report = Array(
                                                'One day ago'                           => Array(
                                                                                                'date'          => $date_yesterday,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'One week ago'  => Array(
                                                                                                'date'          => $last_week_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'One month ago' => Array(
                                                                                                'date'          => $last_month_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Three months ago'      => Array(
                                                                                                'date'          => $last_quarter_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'Six months ago'        => Array(
                                                                                                'date'          => $last_semester_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                                'One year ago'          => Array(
                                                                                                'date'          => $last_year_date,
                                                                                                'num_assets'    => 0,
                                                                                           ),
                                        );

		$assets_since_last_period = Array();

		$num_assets_today = Report_Total_Assets::_getTotalAssets();

		foreach ($date_report as $date_description => $date_data) {
			$num_assets = Report_Total_Assets::_getTotalAssets($date_data['date']);
			$date_report[$date_description]['num_assets'] = $num_assets;
			$assets_since_last_period[$date_description] = $num_assets;
		}

		$num_assets_last_period = $num_assets_today;
		foreach ($assets_since_last_period as $date_description => $num_assets) {
			$num_assets_in_period = $num_assets_last_period - $num_assets;
			$num_assets_last_period = $num_assets;
			$date_report[$date_description]['num_assets'] = $num_assets_in_period;
		}

		$report = Array(
				'generated'		=> $today,
				'report_data'		=> Array(
								'num_assets_today'      => $num_assets_today,
								'date_report'           => $date_report,
							   ),
			  );

		Report_Total_Assets::saveReport($type_code, $report);

		return TRUE;

	}//end processReport()


	private static function _getTotalAssets($before_date='')
	{
		$before_date_query = '';
		if ($before_date != '') {
			$before_date_query = " WHERE created <= '".$before_date."'";
		}

		$db = MatrixDAL::getDb();
		$sql = 'SELECT count(*) AS total '.
			'FROM sq_ast'.$before_date_query;

		$num_assets = MatrixDAL::executeSqlOne($sql);

		return $num_assets;

	}//end _getTotalAssets()


	private static function _paintSystemInstallDate()
	{
		$table_format = '<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header" width="30%%">System Install Date</th>
						<td class="sq-backend-table-cell" align="right">%s</td>
					</tr>
				</table>';

		$time = strtotime(Report_Total_Assets::_getSystemInstallDate());
		$date = date('d M Y, H:i:s', $time);

		$table = sprintf($table_format, $date);
		echo $table;

	}//end _paintSystemInstallDate()


	private static function _getSystemInstallDate()
	{
		$db = MatrixDAL::getDb();
		$sql = 'SELECT MIN(created) '.
			'FROM sq_ast';

		$first_asset_date = MatrixDAL::executeSqlOne($sql);

		return $first_asset_date;

	}//end _getSystemInstallDate()


}//end class
?>
