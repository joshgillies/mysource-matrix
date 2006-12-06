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
* $Id: log_report_edit_fns.inc,v 1.4 2006/12/06 01:07:34 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';

/**
* Report_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Report_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Paint the interface for regenerating the report
	*
	* Leave this up to the kids to override for now
	*
	* @param object	&$asset	the report whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintRegenerate(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end paintRegenerate()


	/**
	* Paint the interface for updating the report
	*
	* @param object	&$report	the report whose interface we are painting
	* @param object	&$out		the Backend Outputter object
	* @param string	$prefix		prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function _processRegenerate(&$report, &$out, $prefix)
	{
		return TRUE;

	}//end _processRegenerate()


	/**
	* Paint the interface for updating the report
	*
	* @param object	&$report	the report whose interface we are painting
	* @param object	&$out		the Backend Outputter object
	* @param string	$prefix		prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$report, &$out, $prefix)
	{
		$body = $report->attr('report_body');
		echo $body;

	}//end paintReport()


	/**
	* Generate this report until complete
	*
	* @param object	&$job		a reference to the HIPO job generating this report
	* @param array	&$step_data	a reference to the array of information about
	*							the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end generateReport()


}//end class

?>