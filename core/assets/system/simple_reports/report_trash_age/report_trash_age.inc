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
* $Id: report_trash_age.inc,v 1.3 2008/10/02 07:02:50 mbrydon Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/simple_report/simple_report.inc';

/**
* Trash Age Report
*
* Purpose
* 	Provides a report of the top twenty assets in the trash which have not been updated recently
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Report_Trash_Age extends Simple_Report
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
		$report = Report_Trash_Age::loadReport($type_code);

		if (empty($report)) {
			Report_Trash_Age::paintReportNotGenerated($type_code);
			return TRUE;
		}

		$generated = $report['generated'];
		$report = $report['report_data'];

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$row_format = '<tr>
				<td class="sq-backend-table-cell" style="width: 10px">
					<img src="%s">
				</td>
				<td class="sq-backend-table-cell">
					%s (Id: %s)
				</td>
				<td class="sq-backend-table-cell" align="right">
					%s
				</td>
				</tr>';

		$rows_string = '';
		foreach ($report as $asset_info) {
			$rows_string .= sprintf($row_format, $am->getAssetIconURL($asset_info['type_code']), $asset_info['name'], $asset_info['assetid'], date('d M Y, H:i:s', strtotime($asset_info['updated'])));
		}

		$table_format = '<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header" colspan="2">Name</th>
					<th class="sq-backend-table-header" align="right">Last Updated</th>
				</tr>
				%s
				</table>';

		$table = sprintf($table_format, $rows_string);

		Report_Trash_Age::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));
		echo '<br>';
		if (empty($rows_string)) {
			$table = '<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-cell">There are currently no assets in the trash</td>
				</tr>
				</table>';
		}

		echo $table;

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

		$asset_report = Report_Trash_Age::_getOldestAssetsInTrash();

		$report = Array(
				'generated'	=> $today,
				'report_data'	=> $asset_report,
			  );

		Report_Trash_Age::saveReport($type_code, $report);

		return TRUE;

	}//end processReport()


	/**
	* Returns a structure of the top twenty assets that reside in the trash which have not been updated recently
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getOldestAssetsInTrash()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$asset_info = Array();

		$db = MatrixDAL::getDb();
		$sql = 'SELECT a.assetid, a.type_code, a.name, a.updated '.
			'FROM sq_ast_lnk l, sq_ast_lnk_tree t, sq_ast a, sq_ast_typ p '.
			'WHERE a.type_code = p.type_code AND p.allowed_access != \'system\' AND p.instantiable = 1 AND t.treeid LIKE '.
			'(SELECT t.treeid FROM sq_ast_lnk_tree t, sq_ast_lnk l, sq_ast a '.
			'WHERE l.minorid = a.assetid AND t.linkid = l.minorid AND a.type_code = \'trash_folder\')||\'%\' '.
			'AND l.linkid = t.linkid AND a.assetid = l.minorid AND a.type_code != \'trash_folder\' '.
			'ORDER BY updated';
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 20);

		$rows = MatrixDAL::executeSqlAssoc($sql);

		return $rows;

	}//end _getOldestAssetsInTrash()


}//end class
?>
