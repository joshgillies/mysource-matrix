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
* $Id: report_incomplete_metadata_edit_fns.inc,v 1.4 2004/10/25 01:31:17 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
/**
* Report_Incomplete_Metadata_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Incomplete_Metadata_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Report_Incomplete_Metadata_Edit_Fns()
	{
		$this->Report_Edit_Fns();

	}//end constructor


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
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$owner = &$GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_incomplete_metadata');

		if (!isset($job->_running_vars['results'])) {
			$job->_running_vars['todo'] = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->getRootAssetid($owner));
			$job->_running_vars['done'] = Array();
			$job->_running_vars['results'] = Array();

			if (!is_dir($owner->data_path)) {
				if (!create_directory($owner->data_path)) {
					trigger_error('Could not create data directory, report generation aborted', E_USER_WARNING);
					return false;
				}
			}
			$temp_file = fopen($owner->data_path.'/report.tmp', 'w');
			if ($temp_file === false) {
				trigger_error('Could not open temporary file, report generation aborted', E_USER_WARNING);
				return false;
			}
			fwrite($temp_file, "<assets>\n");
		} else {
			$temp_file = fopen($owner->data_path.'/report.tmp', 'a');
			if ($temp_file === false) {
				trigger_error('Could not open temporary file, report generation aborted', E_USER_WARNING);
				return false;
			}
		}

		if (!empty($job->_running_vars['todo'])) {

			$assetid = array_shift(array_keys($job->_running_vars['todo']));
			$asset_type = $job->_running_vars['todo'][$assetid];
			unset($job->_running_vars['todo'][$assetid]);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				$step_data['message'] = 'Checking metadata for '.$asset->name.' ...';

				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$field_values = $mm->getMetadataFieldValues($asset->id);

				$empty_fields = false;
				ob_start();
					echo "\t<asset assetid=\"".$asset->id."\">\n";
					foreach ($field_values as $field_name => $value) {
						$value = trim($value);
						if (empty($value)) {
							echo "\t\t<field>$field_name</field>\n";
							$empty_fields = true;
						}
					}
					echo "\t</asset>\n";
					$content = ob_get_contents();
				ob_end_clean();

				// write out the XML if there were empty fields
				if ($empty_fields) fwrite($temp_file, $content);

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$job->_addError('Could not generate incomplete metadata report for assetd #'.$assetid.', asset does not exist');
			}

			// add this assetid to the done array so we dont do it again
			$job->_running_vars['done'][] = $assetid;

		}//end if

		if (empty($job->_running_vars['todo'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;

			// move the temp file to become the real report XML file
			fwrite($temp_file, "</assets>\n");
			if (!copy($owner->data_path.'/report.tmp', $owner->data_path.'/report.xml')) {
				trigger_error('Could not move temporary file, report generation aborted', E_USER_WARNING);
				return false;
			}
			if (!unlink($owner->data_path.'/report.tmp')) {
				trigger_error('Could not delete temporary file, report generation continues', E_USER_WARNING);
			}
		} else {
			$total = count($job->_running_vars['todo']) + count($job->_running_vars['done']);
			$step_data['percent_done'] = (count($job->_running_vars['done']) / $total) * 100;
			$step_data['complete']     = false;
		}

		fclose($temp_file);
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
		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$report_path = $asset->data_path.'/report.xml';
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

		$missing_fields = Array();
		foreach ($root->children as $result) {
			if ($result->name != 'asset') continue;
			foreach ($result->children as $field) {
				if ($field->name != 'field') continue;
				$missing_fields[$result->attributes['assetid']][] = $field->content;
			}
		}

		echo '<b>Note</b> The current incomplete metadata report is shown below. You can regenerate this report on the details screen.';
		$o->closeSection();

		$o->openSection('Missing Metadata Fields');
			$o->openField('');
				if (empty($missing_fields)) {
					echo 'No missing metadata fields were found';
					return;
				}

				?>
				<table class="sq-backend-table">
					<tr>
						<td colspan="4" class="sq-backend-table-header" style="width: 226px;">Found In</td>
						<td class="sq-backend-table-header">Empty Fields</td>
					</tr>
				<?php
				foreach ($missing_fields as $assetid => $fields) {
					$asset = null;
					if ($assetid) {
						$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);
					}
					if (is_null($asset)) {
						$asset_type = '';
						$asset_string = 'Unkown asset #'.$assetid;
						$asset_bg = 'FFFFFF';
						$asset_status = '';
					} else {
						$asset_type = $asset->type();
						$asset_bg = get_status_colour($asset->status);
						$asset_status = get_status_description($asset->status);
						$asset_url = $asset->getBackendHref('metadata');
						$asset_string = '<a href="#" onclick="Javscript: return false;" target="_blank" title="'.$asset->name.'">'.ellipsisize($asset->name, 40).'</a>';
					}

					?>
					<tr>
						<td class="sq-backend-table-cell" style="width: 10px; padding-right: 1px; text-align: right;">
							<?php
							if (!empty($asset_type)) {
								?>
								<a href="<?php echo $asset_url; ?>" title="Edit this asset's metadata">#<?php echo (int) $asset->id; ?></a>
								<?php
							}
							?>
						</td>
						<td class="sq-backend-table-cell" style="width: 10px; padding-right: 1px;">
							<div title="<?php echo $asset_status; ?>" style="border: 1px solid #CCCCCC; padding: 4px; background-color: #<?php echo $asset_bg; ?>; width: 8px; height: 8px;">&nbsp;</div>
						</td>
						<td class="sq-backend-table-cell" style="width: 16px; padding-right: 2px; padding-left: 2px;">
							<?php
							if (!empty($asset_type)) {
								?>
								<span title="<?php echo ucwords(str_replace('_', ' ', $asset_type)); ?>">
									<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($asset_type); ?>", "16", "16", "");</script>
								</span>
								<?php
							}
							?>
						</td>
						<td class="sq-backend-table-cell" style="padding-left: 2px;">
							<?php echo $asset_string; ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							echo implode('<br />', $fields);
							?>
						</td>
					</tr>
					<?php

				}//end foreach assets

				?></table><?php

	}//end paintReport()


}//end class

?>
