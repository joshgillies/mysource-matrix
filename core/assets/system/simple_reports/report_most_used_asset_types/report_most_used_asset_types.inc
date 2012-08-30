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
* $Id: report_most_used_asset_types.inc,v 1.5 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/simple_report/simple_report.inc';

/**
* Most Used Asset Types Report
*
* Purpose
* 	Provides a report of the top ten asset types in use on the system
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Report_Most_Used_Asset_Types extends Simple_Report
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
		$report = Report_Most_Used_Asset_Types::loadReport($type_code);

		if (empty($report)) {
			Report_Most_Used_Asset_Types::paintReportNotGenerated($type_code);
			return TRUE;
		}

		$generated = $report['generated'];
		$report = $report['report_data'];

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Lovingly borrowed from the Asset Counter Report
		$row_format = '<tr>
				<td class="sq-backend-table-cell" style="width: 10px">
					<img src="%s">
				</td>
				<td class="sq-backend-table-cell">
					%s
				</td>
				<td class="sq-backend-table-cell" align="right">
					%s
				</td>
				</tr>';

		$rows_string = '';

		foreach ($report['user_assets'] as $asset_type_code => $user_asset_stats) {
			$rows_string .= sprintf($row_format, $am->getAssetIconURL($asset_type_code), $user_asset_stats['asset_type_name'], $user_asset_stats['total']);
		}

		$table_format = '       <b>%s</b><br><br>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header" colspan="2">Asset Type</th>
						<th class="sq-backend-table-header" width="70" align="right">Total</th>
					</tr>
					%s
				</table>
				<br>';

		$user_assets_table = sprintf($table_format, 'User Asset Types', $rows_string);

		$rows_string = '';

		foreach ($report['system_assets'] as $asset_type_code => $system_asset_stats) {
			$rows_string .= sprintf($row_format, $am->getAssetIconURL($asset_type_code), $system_asset_stats['asset_type_name'], $system_asset_stats['total']);
		}

		$system_assets_table = sprintf($table_format, 'System Asset Types', $rows_string);

		Report_Most_Used_Asset_Types::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));
		echo '<br><br>';
		echo $user_assets_table;
		echo '<br>'.$system_assets_table;

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

		$report = Array(
				'generated'	=> $today,
				'report_data'	=> Array(
							'user_assets'	=> Report_Most_Used_Asset_Types::_getTopAssetTypes('user'),
							'system_assets'	=> Report_Most_Used_Asset_Types::_getTopAssetTypes('system'),
						   ),
			  );

		Report_Most_Used_Asset_Types::saveReport($type_code, $report);

		return TRUE;

	}//end processReport()


	/**
	* Return the number and type of the ten most used asset types on the system
	*
	* @param string	$asset_access_type	the asset 'domain' to return
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getTopAssetTypes($asset_access_type='all')
	{
		$access_type_query = '';

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$db = MatrixDAL::getDb();
		$sql = 'SELECT type_code, count(*) AS total '.
			'FROM sq_ast '.
			'GROUP BY type_code '.
			'ORDER BY total DESC';

		// Now for our "Top Ten"
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 10);

		if (($asset_access_type == 'system') || ($asset_access_type == 'user')) {
			if ($asset_access_type == 'system') {
				$access_type_query = 't.allowed_access = '.MatrixDAL::quote('system').' ';
			} else if ($asset_access_type == 'user') {
				$access_type_query = 't.allowed_access != '.MatrixDAL::quote('system').' ';
			}

			$sql = 'SELECT a.type_code, count(*) AS total '.
				'FROM sq_ast a, sq_ast_typ t '.
				'WHERE a.type_code = t.type_code AND t.instantiable = '.MatrixDAL::quote('1').' AND '.$access_type_query.
				'GROUP BY a.type_code '.
				'ORDER BY total DESC';
			$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 10);
		}

		$rows = MatrixDAL::executeSqlAssoc($sql);
		$asset_types = Array();

		foreach ($rows as $row) {
			$asset_type_name = $am->getTypeInfo($row['type_code'], 'name');

			$asset_types[$row['type_code']] = Array(
								'asset_type_name'	=> $asset_type_name,
								'total'			=> $row['total'],
							  );
		}		

		return $asset_types;

	}//end _getTopAssetTypes()


}//end class

?>
