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
* $Id: report_total_internal_messages.inc,v 1.3 2008/10/02 07:02:50 mbrydon Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/simple_report/simple_report.inc';

/**
* Internal Messages Report
*
* Purpose
* 	Provides a report of Internal Message types and totals for each type
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Report_Total_Internal_Messages extends Simple_Report
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
	public static function paintReport(Backend_Outputter &$o, $type_code)
	{
                $report = Report_Total_Internal_Messages::loadReport($type_code);

		if (empty($report)) {
			Report_Total_Internal_Messages::paintReportNotGenerated($type_code);
			return TRUE;
		}

                $generated = $report['generated'];
                $report = $report['report_data'];
		$total_system_messages_by_type = $report['total_system_messages_by_type'];
		$total_user_messages_by_type = $report['total_user_messages_by_type'];
		$total_messages_by_status = $report['total_messages_by_status'];

                $row_format = '<tr>
                                <td class="sq-backend-table-cell">
                                        %s
                                </td>
                                <td class="sq-backend-table-cell" align="right">
                                        %s
                                </td>
                                </tr>';

                $table_format = '	<br><b>%s</b><br><br>
                                        <table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header">Message Type</th>
                                                        <th class="sq-backend-table-header" width="70" align="right">Total</th>
                                                </tr>
                                                %s
                                        </table>
                                ';

                $rows_string = '';
                foreach ($total_system_messages_by_type as $report) {
                        $rows_string .= sprintf($row_format, $report['type'], $report['total']);
                }

                $table = sprintf($table_format, 'System Messages', $rows_string);

                Report_Total_Internal_Messages::paintLastGeneratedDate($type_code, date('d M Y, H:i:s', $generated));
                echo '<br>'.$table;

                $rows_string = '';
                foreach ($total_user_messages_by_type as $report) {
                        $rows_string .= sprintf($row_format, $report['type'], $report['total']);
                }

                $table = sprintf($table_format, 'User Messages', $rows_string);

		echo '<br>'.$table;

                $rows_string = '';

                $row_format = '<tr>
                                <td class="sq-backend-table-cell" style="width: 10px">
                                        %s
                                </td>
                                <td class="sq-backend-table-cell">
                                        %s
                                </td>
                                <td class="sq-backend-table-cell" align="right">
                                        %s
                                </td>
                                </tr>';

                foreach ($total_messages_by_status as $report) {
			$status = $report['status'];
			$unknown_status = FALSE;

			switch ($status) {
				case 'U':	$status = 'Unread';
				break;

				case 'R':	$status = 'Read';
				break;

				case 'D':	$status = 'Deleted';
				break;

				default:	$unknown_status = TRUE;
				break;
			}

			$icon = '';
			if (!$unknown_status) {
				if ($status == 'Deleted') {
					$icon = '<img src="'.sq_web_path('lib').'/web/images/icons/internal_message/trash.png">';
				} else {
					$icon = '<img src="'.sq_web_path('lib').'/web/images/icons/internal_message/message_'.strtolower($status).'.png">';
				}
			}

                        $rows_string .= sprintf($row_format, $icon, $status, $report['total']);
                }


                $table_format = '	<br>
                                        <table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header" colspan="2">Message Status</th>
                                                        <th class="sq-backend-table-header" width="70" align="right">Total</th>
                                                </tr>
                                                %s
                                        </table>
                                ';

                $table = sprintf($table_format, $rows_string);
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

		$messages_report = Array(
					'total_system_messages_by_type'	=> Report_Total_Internal_Messages::_getTotalInternalMessagesByType('system'),
					'total_user_messages_by_type'	=> Report_Total_Internal_Messages::_getTotalInternalMessagesByType('user'),
					'total_messages_by_status'	=> Report_Total_Internal_Messages::_getTotalInternalMessagesByStatus(),
				   );

		$report = Array(
				'generated'     => $today,
				'report_data'   => $messages_report,
		          );

		Report_Total_Internal_Messages::saveReport($type_code, $report);

		return TRUE;

	}//end processReport()


	/**
	* Returns the number of Internal Messages of the specified type in the system
	*
	* @param string	$message_type	the message type on which to report
	*
	* @return array
	* @access private
	* @static
	*/
	private static function _getTotalInternalMessagesByType($message_type='all')
	{
                $db = MatrixDAL::getDb();

		$where_clause = '';
		if ($message_type == 'system') {
			$where_clause = "WHERE type LIKE 'system.%'";
		} else if ($message_type == 'user') {
			$where_clause = "WHERE type NOT LIKE 'system.%'";
		}

                $sql = 'SELECT type, COUNT(*) AS total '.
			'FROM sq_internal_msg '.$where_clause.
			'GROUP BY type ORDER BY total DESC, type';

                $rows = MatrixDAL::executeSqlAssoc($sql);

                return $rows;

        }//end _getTotalInternalMessagesByType()


	/**
	* Returns the number of Internal Messages for each message state
	*
	* @return array
	* @access private
	* @static
	*/
	private function _getTotalInternalMessagesByStatus()
	{
                $db = MatrixDAL::getDb();
                $sql = 'SELECT status, COUNT(*) AS total '.
			'FROM sq_internal_msg '.
			'GROUP BY status ORDER BY status DESC';

                $rows = MatrixDAL::executeSqlAssoc($sql);

                return $rows;

	}//end _getTotalInternalMessagesByStatus()


}//end class

?>
