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
* $Id: report_asset_counter_edit_fns.inc,v 1.2 2004/10/26 01:31:01 arailean Exp $
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
* @version $Revision: 1.2 $
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

		$owner = &$am->getAsset($job->_running_vars['assetid'], 'report_asset_counter');

		$job_vars = & $job->_running_vars;

		$report_contents = '';

		$report_dir = $owner->data_path;

		$xml_file_name = $report_dir.'/'.$this->report_name;
		$types = $owner->attr('types');

		if (empty($job_vars['unique_types'])) {
			// setup initial report parameters

			$step_data['message'] = 'Obtaining and preparing the list of assets ...';

			$children = Array();
			if (!empty($types)) {
				foreach ($types as $type => $inherit_bool) {
					$children_one = $am->getChildren($this->getRootAssetid($owner), $type, !$inherit_bool);
					$children = array_merge($children, $children_one);
				}
			} else {
				// if types are not provided we search for all type codes
				$children = $am->getChildren($this->getRootAssetid($owner));

			}

			// check if there is anything to do
			if (empty($children)) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = true;
				$job->_addError('Report not generated because no assets were found matching your criteria',true);
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

		$o->openSection('Asset Type Counts');
			$o->openField('');
				if (empty($asset_types)) {
					echo 'Report is empty';
					return;
				}

				$sort_by_attr = $asset->getAttribute('sort_by');
				$sort_by = $sort_by_attr->value;

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
					$names[$typecode]  = $data['name'];
					$types[$typecode]  = $typecode;

					$rows[$typecode]   = sprintf($row_format,$am->getAssetIconURL($typecode),$data['count'],$typecode,$data['name']);
				}


				// sort the table
				switch($sort_by) {
					case('count'):
						$target_array = $counts;
					break;

					case('name'):
						$target_array = $names;
					break;

					case('type'):
					default:
						$sort_by      = 'type';
						$target_array = $types;

				}

				if ($asset->attr('reverse_sort')) {
					arsort($target_array);
				} else {
					asort($target_array);
				}

				$sort_by_name = $sort_by_attr->_params['options'][$sort_by];

				$table_format =<<<HEREDOC
					<h3>Sorted by: <span style="color: red">$sort_by_name</span></h3>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 10ex">Icon</td>
							<td class="sq-backend-table-header" style="width: 10ex">Count</td>
							<td class="sq-backend-table-header">Code</td>
							<td class="sq-backend-table-header" width="70%%">Type</td>
						</tr>
						%s
					</table>

HEREDOC;

				foreach ($target_array as $typecode => $data) {
					$rows_string .= $rows[$typecode];
				}
				$table = sprintf($table_format,$rows_string);

				echo $table;

	}//end paintReport()


	/**
	* Paint the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');

		$types = $asset->attr('types');
		if (empty($types)) {
			echo 'There are no Types Defined';
			return false;
		}

		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header">Asset Type</td>
			<td class="sq-backend-table-header">Inherit Types</td>
			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header">Remove?</td>
				<?php
			}
			?>
		</tr>
			<?php
			foreach ($types as $type => $inherit) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php
						echo ucwords(str_replace('_',' ', $type));
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) {
							check_box($prefix."_inherit_types[$type]", $type, $inherit);
						} else {
							echo ($inherit) ? 'Yes' : 'No';
						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix."_remove[$type]", $type, false);
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
		?>
		</table>
		<?php

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');

		// we are looking to see if any types have been removed
		if (isset($_POST[$prefix.'_remove'])) {

			foreach ($_POST[$prefix.'_remove'] as $type) {
				if (isset($types[$type])) {
					unset($types[$type]);
				}
			}
		}

		$inherit_types = array_get_index($_POST, $prefix.'_inherit_types', Array());
		foreach ($types as $key => $val) {
			$types[$key] = isset($inherit_types[$key]) ? true : false;
		}

		$asset->setAttrValue('types', $types);

		return true;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('content')) return false;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));
		asort($types);

		$uc_types = Array('' => '');
		foreach ($types as $type) {
			$uc_types[$type] = ucwords(str_replace('_',' ', $type));
		}
		combo_box($prefix.'_new_type', $uc_types, false, '');

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_type'])) {
			$type = trim($_POST[$prefix.'_new_type']);

			if (empty($type)) {
				return false;
			}

			$types = $asset->attr('types');

			if (!isset($types[$type])) {
				$types[$type] = true;
				$asset->setAttrValue('types', $types);
				return true;
			}
		}
		return false;

	}//end paintAddFormat()


}//end class

?>