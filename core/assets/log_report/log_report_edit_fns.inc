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
* $Id: log_report_edit_fns.inc,v 1.7 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Report_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for regenerating the report
	*
	* Leave this up to the kids to override for now
	*
	* @param Log_Report			$asset	the report whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintRegenerate(Log_Report $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end paintRegenerate()


	/**
	* Paint the interface for updating the report
	*
	* @param Log_Report			$asset	the report whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function _processRegenerate(Log_Report $asset, Backend_Outputter $o, $prefix)
	{
		return TRUE;

	}//end _processRegenerate()


	/**
	* Paint the interface for updating the report
	*
	* @param Log_Report			$asset	the report whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(Log_Report $asset, Backend_Outputter $o, $prefix)
	{
		$body = $asset->attr('report_body');
		echo $body;

	}//end paintReport()


	/**
	* Generate this report until complete
	*
	* @param HIPO_Job	$job		a reference to the HIPO job generating this report
	* @param array		&$step_data	a reference to the array of information about
	*								the current step of the HIPO job that is running this report
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	public function generateReport(HIPO_Job $job, Array &$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end generateReport()


}//end class

?>