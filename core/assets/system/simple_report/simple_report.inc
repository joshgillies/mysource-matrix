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
* $Id: simple_report.inc,v 1.1 2008/05/07 05:54:04 mbrydon Exp $
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
* @version $Revision: 1.1 $
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

                $table_format = '
                                        <table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header" width="30%%">Last Generated</th>
                                                        <td class="sq-backend-table-cell" id="'.$type_code.'_regen" align="right">%s &nbsp;&nbsp;( %s )</td>
                                                </tr>
                                        </table>';

                $table = sprintf($table_format, 'This report has not been generated', $generate_link);
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

                $table_format = '
                                        <table class="sq-backend-table">
                                                <tr>
                                                        <th class="sq-backend-table-header" width="30%%">Last Generated</th>
                                                        <td class="sq-backend-table-cell" id="'.$type_code.'_regen" align="right">%s &nbsp;&nbsp;( %s )</td>
                                                </tr>
                                        </table>';

                $table = sprintf($table_format, $date, $regenerate_link);
                echo $table;

        }//end paintLastGeneratedDate()


}//end class


?>
