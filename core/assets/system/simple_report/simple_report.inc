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
* $Id: simple_report.inc,v 1.3 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Simple Report
*
* Purpose
*
*    An asset to execute and display one-shot reports without
*    a form interface.
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Simple_Report extends Asset
{


	/**
	* Constructor
	*
	* @param int  $assetid  the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Paints the report interface.
	*
	* @param object	&$o		reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintReport(Backend_Outputter &$o, $type_code)
	{

	}//end paintReport()


	/**
	* Processes the user input and performs the function of the report.  Returns false if any immediate errors.
	*
	* @param object	&$o		reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processReport(Backend_Outputter &$o, $type_code)
	{
		return TRUE;

	}//end processReport()


	/**
	* Saves the result of a report to the cache directory
	*
	* @param string	$type_code	Asset type code of the report
	* @param array	$report_data	The report data
	*
	* @return boolean
	* @access protected
	* @static
	*/
	protected static function saveReport($type_code, $report_data)
	{
		$fd = FALSE;

		// Final check before using supplied type code
		$am = $GLOBALS['SQ_SYSTEM']->am;
		if ($am->installed($type_code)) {
			$fd = fopen(SQ_CACHE_PATH.'/'.$type_code.'.dat', 'w');
			if ($fd) {
				fwrite($fd, serialize($report_data));
				fclose($fd);
			}
		}

		return $fd;

	}//end saveReport()


	/**
	* Loads a report from the cache directory of the specified type
	*
	* @param string	$type_code	Asset type code of the report
	*
	* @return array
	* @access protected
	* @static
	*/
	protected static function loadReport($type_code)
	{
		$report_data = Array();

		// Final check before using supplied type code
		$am = $GLOBALS['SQ_SYSTEM']->am;
		if ($am->installed($type_code)) {
			$filename = SQ_CACHE_PATH.'/'.$type_code.'.dat';
			if (file_exists($filename)) {
				$fd = fopen($filename, 'r');
				if ($fd) {
					$report_data = fread($fd, filesize($filename));
					$report_data = unserialize($report_data);
					if (!is_array($report_data)) $report_data = Array();
					fclose($fd);
				}
			}
		}

		return $report_data;

	}//end loadReport()


	/**
	* Paints a message to say that the report has not been generated and a link to generate the report
	*
	* @param string	$type_code	Asset type code of the report
	*
	* @return void
	* @access protected
	* @static
	*/
	protected static function paintReportNotGenerated($type_code)
	{
		$generate_link = '<a href="#'.$type_code.'" onclick="regen(\''.$type_code.'\');">'.translate('generate_report').'</a>';

		$table_format = '<table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header" width="35%%">'.translate('last_generated').'</th>
                                                        <td class="sq-backend-table-cell" id="'.$type_code.'_regen" align="right">%s &nbsp;&nbsp;( %s )</td>
                                                </tr>
                                        </table>';

		$table = sprintf($table_format, translate('report_not_generated'), $generate_link);
		echo $table;

	}//end paintReportNotGenerated()


	/**
	* Paints the last report generation date and a link to regenerate the report
	*
	* @param string	$type_code	Asset type code of the report
	* @param string	$date		The date the report was last generated
	*
	* @return void
	* @access protected
	* @static
	*/
	protected static function paintLastGeneratedDate($type_code, $date)
	{
		$regenerate_link = '<a href="#'.$type_code.'" onclick="regen(\''.$type_code.'\');">'.translate('regenerate_report').'</a>';

		$table_format = '<table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header" width="35%%">'.translate('last_generated').'</th>
                                                        <td class="sq-backend-table-cell" id="'.$type_code.'_regen" align="right">%s &nbsp;&nbsp;( %s )</td>
                                                </tr>
                                        </table>';

		$table = sprintf($table_format, $date, $regenerate_link);
		echo $table;

	}//end paintLastGeneratedDate()


	/**
	* Paints a list of recommended maintenance tasks
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code		Asset type code of the report
	* @param array	$report_type_code	Asset type codes of recommended tools
	*
	* @return void
	* @access protected
	* @static
	*/
	protected static function paintRecommendedTasks(Backend_Outputter &$o, $type_code, $report_type_codes)
	{
		$table_format = '<table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header" width="35%%">'.translate('recommended_maintenance_tasks').'</th>
                                                        <td class="sq-backend-table-cell" align="right">%s</td>
                                                </tr>
                                        </table>';

		// Get report name and description and sort reports by name
		$report_list = Array();
		foreach ($report_type_codes as $report_type_code) {
			$report_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($report_type_code);
			$report_name = $report_info['name'];
			$report_list[$report_name] = Array(
							'type_code'     => $report_type_code,
							'description'   => $report_info['description'],
						     );
		}

		ksort($report_list);
		reset($report_list);

		$row_string = '';

		// Link to the relevant tools
		foreach ($report_list as $report_name => $report) {
			$row_string .= '<img src="'.sq_web_path('lib').'/web/images/icons/asset_map/myspace.png"> <a href="./?SQ_BACKEND_PAGE=main&backend_section=tools&tool_type_code='.$report['type_code'].'&redirect_to_sys_maintenance=1">'.$report_name.'</a>';
		}

		$table = sprintf($table_format, $row_string);
		echo $table;

	}//end paintRecommendedTasks()


}//end class


?>
