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
* $Id: report_asset_counter_edit_fns.inc,v 1.1 2004/10/25 01:32:33 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Report_Asset_Counter_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Asset_Counter_Edit_Fns extends Report_Edit_Fns
{

	var $report_name = 'report.xml';
	var $temp_report = 'report.tmp';

	// Constructor is not necessary

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

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$owner = &$GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_asset_counter');

		$job_vars = & $job->_running_vars;

		if (empty($job_vars['step'])) {
			$job_vars['step'] = 1;
		}
		$report_contents = '';

		$report_dir = $owner->data_path;

		$xml_file_name = $report_dir.'/'.$this->report_name;

		if (empty($job_vars['unique_types'])) {
			// setup initial report parameters

			$step_data['message'] = 'Obtaining and preparing the list of assets ...';

			$children = $am->getChildren($this->getRootAssetid($owner));

			// check if there is anything to do
			if (empty($children)) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = true;
				return false;
			}

			$job_vars['type_counts']     = array_count_values($children);
			$job_vars['unique_types']    = array_unique($children);

			if (!is_dir($report_dir)) {
				if (!create_directory($report_dir)) {
					trigger_error('Could not create data directory, report generation aborted', E_USER_WARNING);
					return false;
				}
			}

			$step_data['percent_done'] = 50;
			$step_data['complete']     = false;

		} else {
			// provide human descriptions for each type of Asset
			// and save the report

			$step_data['message'] = 'Getting readable asset type descriptions ...';

			// generate the report
			$report_contents .= "<asset_types>";

			foreach ($job_vars['unique_types'] as $asset_id => $type_code) {
				$name = $am->getTypeInfo($type_code, 'name');
				$count = $job_vars['type_counts'][$type_code];

				$report_contents .= '<asset_type code="'.$type_code.'">';
				$report_contents .= "<name>$name</name>";
				$report_contents .= "<count>$count</count>";
				$report_contents .= '</asset_type>';
			}

			$report_contents .= "</asset_types>";

			// file manipulation section
			$file_handler = fopen($xml_file_name, 'w');
			if ($file_handler === false) {
				trigger_error('Could not open report file, report generation aborted', E_USER_WARNING);
				return false;
			}
			fwrite($file_handler, $report_contents);
			fclose($file_handler);

			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;

		}

		return true;

	}//end generateReport()


	/**
	* Paint the interface for viewing a report
	*
	* @param object Report				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$report_path = $asset->data_path.'/'.$this->report_name;

		if (!is_file($report_path)) {
			echo 'This report has not been generated';
			return;
		}

		$input = new XML_Tree($report_path);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error('Unable to read report file : '.$report_path."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		$asset_types = Array();
		foreach ($root->children as $type_node) {
			if ($type_node->name != 'asset_type') continue;

			$type = $type_node->attributes['code'];

			foreach ($type_node->children as $field) {
				$asset_types[$type][$field->name] = $field->content;
			}
		}

		echo '<strong>Asset Counts</strong>';
		$o->closeSection();

		$o->openSection('Asset Counts');
			$o->openField('');
				if (empty($asset_types)) {
					echo 'Report is empty';
					return;
				}

				$table_format =<<<HEREDOC
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 10ex">Icon</td>
							<td class="sq-backend-table-header" style="width: 10ex">Count</td>
							<td class="sq-backend-table-header">Type Code</td>
							<td class="sq-backend-table-header" width="70%%">Type</td>
						</tr>
						%s
					</table>

HEREDOC;

				$row_format =<<<HEREDOC
					<tr>
						<td class="sq-backend-table-cell">
							<script language="JavaScript" type="text/javascript">sq_print_icon("%s", "16", "16", "");</script>
						</td>
						<td class="sq-backend-table-cell" style="width: 10px;">
							%s
						</td>
						<td class="sq-backend-table-cell">
							%s
						</td>
						<td class="sq-backend-table-cell">
							%s
						</td>
					</tr>

HEREDOC;

				$rows_string = '';
				$rows = Array();
				foreach ($asset_types as $typecode => $data) {
					$counts[$typecode] = $data['count'];
					$rows[$typecode]   = sprintf($row_format,$am->getAssetIconURL($typecode),$data['count'],$typecode,$data['name']);
				}

				// built-in sorting by type code
				// dynamic sorting should be implemented in the future

				// uncomment ONE of the following lines to change the sorting mode
				// asort($counts); // use this line to sort by counts
				ksort($counts); // use this line to sort by type

				reset ($counts);

				foreach ($counts as $typecode => $data) {
					$rows_string .= $rows[$typecode];
				}
				$table = sprintf($table_format,$rows_string);

				echo $table;

	}//end paintReport()


}//end class

?>