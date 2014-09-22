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
* $Id: report_asset_counter_edit_fns.inc,v 1.19 2013/01/23 04:57:22 cupreti Exp $
*
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
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Asset_Counter_Edit_Fns extends Report_Edit_Fns
{

	public $report_name = 'report.xml';
	public $temp_report = 'report.tmp';


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Generate this report until complete
	*
	* @param HIPO_Job	$job		the HIPO job running this report
	* @param array		&$step_data	a reference to the array of information about
	*								the current step of the HIPO job that is running this report
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	public function generateReport(HIPO_Job $job, Array &$step_data, $prefix)
	{

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$owner = $am->getAsset($job->_running_vars['assetid'], 'report_asset_counter');
		$job_vars =& $job->_running_vars;

		$report_contents = '';
		$report_dir = $owner->data_path;

		$xml_file_name = $report_dir.'/'.$this->report_name;
		$types = $owner->attr('types');

		if (empty($job_vars['unique_types'])) {
			// setup initial report parameters

			$step_data['message'] = translate('Obtaining and preparing the list of assets ...');


			$children = Array();
			if (!empty($types)) {
				foreach ($types as $type => $inherit_bool) {
					$children_one = $am->getChildren($this->getRootAssetid($owner), $type, !$inherit_bool, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE);
					$children = array_merge($children, $children_one);
				}
			} else {
				// if types are not provided we search for all type codes
				$children = $am->getChildren($this->getRootAssetid($owner), '', TRUE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE);
			}

			// Get the missing types for shadow assets if any
			foreach($children as $child_id => $child_info) {
				$id_parts = explode(':', $child_id);
				// If its a shadow asset and has missing type info, get it from the bridge
				if (isset($id_parts[1]) && empty($child_info[0]['type_code'])) {
					$shdw_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($child_id, Array(), TRUE, 'type_code');
					if (!empty($shdw_asset_info[$child_id])) {
						$children[$child_id][0]['type_code'] = $shdw_asset_info[$child_id];
						if (!isset($types[$shdw_asset_info[$child_id]])) {
							unset($children[$child_id]);
						}
					} else {
						$children[$child_id][0]['type_code'] = 'unknown_asset_type';
					}
				}//end if
			}//end foreach

			// check if there is anything to do
			if (empty($children)) {
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
				$job->_addError(translate('This report has not been generated'),TRUE);

				return FALSE;
			}

			// get count and type code
			foreach ($children as $i => $info) {
				if (!isset($job_vars['type_counts'][$info[0]['type_code']])) {
					$job_vars['type_counts'][$info[0]['type_code']] = 1;
				} else {
					$job_vars['type_counts'][$info[0]['type_code']]++;
				}
			}
			$job_vars['unique_types'] = array_keys($job_vars['type_counts']);

			if (!is_dir($report_dir)) {
				if (!create_directory($report_dir)) {
					trigger_localised_error('CORE0011', translate('Unable to create data directory, report generation aborted'), E_USER_WARNING)
					return FALSE;
				}
			}

			$step_data['percent_done'] = 50;
			$step_data['complete']     = FALSE;

		} else {
			// provide human descriptions for each type of Asset
			// and save the report

			$step_data['message'] = translate('Getting readable asset type descriptions ...');


			$report_contents .= '<?xml version="1.0" encoding="'.SQ_CONF_DEFAULT_CHARACTER_SET.'"?>'."\n";
			// generate the report
			$report_contents .= '<asset_types>';

			foreach ($job_vars['unique_types'] as $asset_id => $type_code) {
				$name = $am->getTypeInfo($type_code, 'name');
				$count = $job_vars['type_counts'][$type_code];

				$report_contents .= '<asset_type code="'.$type_code.'">';
				$report_contents .= '<name>'.$name.'</name>';
				$report_contents .= '<count>'.$count.'</count>';
				$report_contents .= '</asset_type>';
			}

			$report_contents .= '</asset_types>';

			// file manipulation section
			$file_handler = fopen($xml_file_name, 'w');
			if ($file_handler === FALSE) {
				trigger_localised_error('CORE0017', translate('Unable to open report file, report generation aborted'), E_USER_WARNING)
				return FALSE;
			}
			fwrite($file_handler, $report_contents);
			fclose($file_handler);

			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;

		}//end else

		return TRUE;

	}//end generateReport()


	/**
	* Paint the interface for viewing a report
	*
	* @param Report_Asset_Counter	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(Report_Asset_Counter $asset, Backend_Outputter $o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$report_path = $asset->data_path.'/'.$this->report_name;
		if (!is_file($report_path)) {
			echo translate('This report has not been generated');

			return;
		}

		// @see: unit_tests/by_file/core/assets/reports/asset_counter/test_asset_counter_report.test
		// use simple xml to parse the report file
		try {
			$dom = new DOMDocument();
			// use DOMDocument::loadXML() to see if there is any entity  reference loop
			// which may exhaust us of the memory limit, if not then continue as normal
			if ($dom->loadXML(file_get_contents($report_path), LIBXML_NOENT)) {
				$root = simplexml_import_dom($dom);
			} else {
				// don't worry about any warning, if we are here then PHP has already raised the warning for us
				return;
			}
		} catch (Exception $e) {
			throw new Exception('Unable to parse report file "'.$report_path.'": '.$e->getMessage());
			return;
		}

		// build the info array for printing
		$asset_types = Array();
		foreach ($root->asset_type as $node) {
			$type = (string)$node->attributes()->code;
			$asset_types[$type] = Array(
									'name'	=> (string)$node->name,
									'count'	=> (int)$node->count,
								  );
		}

		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {
			echo '<b>Note</b> The current asset counter report is shown below. You can regenerate this report on the details screen.';
		}

		$o->closeSection();

		$o->openSection(translate('Asset Type Counts'));

			$o->openField('');
				if (empty($asset_types)) {
					echo 'Report is empty';
					return;
				}

				$sort_by_attr = $asset->getAttribute('sort_by');
				$sort_by = $sort_by_attr->value;

				$row_format =
				'
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
				';

				$rows_string = '';
				$rows = Array();
				foreach ($asset_types as $typecode => $data) {
					$counts[$typecode] = $data['count'];
					$names[$typecode]  = $data['name'];
					$types[$typecode]  = $typecode;

					$rows[$typecode]   = sprintf($row_format,$am->getAssetIconURL($typecode),$data['count'],$typecode,$data['name']);
				}


				// sort the table
				switch ($sort_by) {
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

				$table_format =
				'
					<h3>Sorted by: <span style="color: red">'.$sort_by_name.'</span></h3>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" style="width: 10ex">Icon</td>
							<td class="sq-backend-table-header" style="width: 10ex">Count</td>
							<td class="sq-backend-table-header">Code</td>
							<td class="sq-backend-table-header" width="70%%">Type</td>
						</tr>
						%s
					</table>
				';

				foreach ($target_array as $typecode => $data) {
					$rows_string .= $rows[$typecode];
				}
				$table = sprintf($table_format,$rows_string);

				echo $table;

	}//end paintReport()


	/**
	* Paint the current asset types that have been added
	*
	* @param Report_Asset_Counter	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentAssetTypes(Report_Asset_Counter $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('content');

		$types = $asset->attr('types');
		if (empty($types)) {
			echo translate('There are no Types Defined');

			return FALSE;
		}

		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header"><?php echo translate('Asset Type'); ?></td>

			<td class="sq-backend-table-header"><?php echo translate('Inherit Types'); ?></td>

			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header"><?php echo translate('Remove?'); ?></td>

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
							echo ($inherit) ? translate('Yes') : translate('No');


						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix."_remove[$type]", $type, FALSE);
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

		return TRUE;

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param Report_Asset_Counter	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processCurrentAssetTypes(Report_Asset_Counter $asset, Backend_Outputter $o, $prefix)
	{
		$types = $asset->attr('types');

		// we are looking to see if any types have been removed
		if (isset($_POST[$prefix.'_remove'])) {

			foreach ($_POST[$prefix.'_remove'] as $type) {
				if (isset($types[$type])) unset($types[$type]);
			}
		}

		$inherit_types = array_get_index($_POST, $prefix.'_inherit_types', Array());
		foreach ($types as $key => $val) {
			$types[$key] = isset($inherit_types[$key]) ? TRUE : FALSE;
		}

		$asset->setAttrValue('types', $types);

		return TRUE;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param Report_Asset_Counter	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintAddAssetType(Report_Asset_Counter $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('content')) return FALSE;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));
		asort($types);

		$uc_types = Array('' => '');
		foreach ($types as $type) {
			$uc_types[$type] = ucwords(str_replace('_',' ', $type));
		}
		combo_box($prefix.'_new_type', $uc_types, FALSE, '');

		return TRUE;

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param Report_Asset_Counter	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the backend outputter object
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processAddAssetType(Report_Asset_Counter $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_type'])) {
			$type = trim($_POST[$prefix.'_new_type']);

			if (empty($type)) return FALSE;

			$types = $asset->attr('types');

			if (!isset($types[$type])) {
				$types[$type] = TRUE;
				$asset->setAttrValue('types', $types);
				return TRUE;
			}
		}
		return FALSE;

	}//end processAddAssetType()


}//end class

?>
