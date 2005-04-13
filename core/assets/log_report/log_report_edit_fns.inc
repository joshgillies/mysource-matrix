<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: log_report_edit_fns.inc,v 1.1 2005/04/13 04:44:59 rhoward Exp $
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
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Report_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Paint the interface for regenerating the report
	*
	* @param object Report				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintRegenerate(&$asset, &$o, $prefix)
	{
		// $report_path = $asset->data_path.'/report.xml';
		if (false) {//{!is_file($report_path)) {
			echo 'This report has not yet been generated<br />';
		} else {
			echo '<p>';
			echo '    <b>This report has been generated</b><br />';
			echo '    Date: '.$asset->attr('generated_date');
			echo '    <br />';
			echo '    User: '.$asset->attr('generated_user');
			echo '</p>';
		}

		return false;

	}//end paintRegenerate()


	/**
	* Paint the interface for updating the report
	*
	* @param object Log_Report			$report	the asset whose interface we are painting
	* @param object	Backend_Outputter	$out	the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function _processRegenerate(&$report, &$out, $prefix)
	{
		return true;

	}//end _processRegenerate()


	/**
	* Paint the interface for updating the report
	*
	* @param object Log_Report			$report	the asset whose interface we are painting
	* @param object	Backend_Outputter	$out	the outputter class
	* @param string						$prefix	prefix for the form element
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
	* @param object HIPO_Job	&$job		a reference to the array of information about
	* @param array				&$step_data	a reference to the array of information about
	*										the current step of the HIPO job that is running this report
	* @param string				$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;
		return true;

	}//end generateReport()


}//end class

?>