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
* $Id: report_incomplete_metadata_edit_fns.inc,v 1.21 2012/08/30 01:09:09 ewang Exp $
*
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
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Incomplete_Metadata_Edit_Fns extends Report_Edit_Fns
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
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$owner = $GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_incomplete_metadata');

		$asset_types = $owner->getAllowedAssetTypeMap();

		if (!isset($job->_running_vars['results'])) {
			$job->_running_vars['todo'] = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->getRootAssetid($owner));
			$job->_running_vars['done'] = Array();
			$job->_running_vars['results'] = Array();

			if (!is_dir($owner->data_path)) {
				if (!create_directory($owner->data_path)) {
					trigger_localised_error('CORE0198', E_USER_WARNING);
					return FALSE;
				}
			}
			$temp_file = fopen($owner->data_path.'/report.tmp', 'w');
			if ($temp_file === FALSE) {
				trigger_localised_error('CORE0201', E_USER_WARNING);
				return FALSE;
			}


			fwrite($temp_file, '<?xml version="1.0" encoding="'.SQ_CONF_DEFAULT_CHARACTER_SET.'"?>'."\n");
			fwrite($temp_file, "<assets>\n");
		} else {
			$temp_file = fopen($owner->data_path.'/report.tmp', 'a');
			if ($temp_file === FALSE) {
				trigger_localised_error('CORE0201', E_USER_WARNING);
				return FALSE;
			}
		}

		if (!empty($job->_running_vars['todo'])) {
			$keys = array_keys($job->_running_vars['todo']);
			$assetid = array_shift($keys);
			$asset_type = $job->_running_vars['todo'][$assetid][0]['type_code'];
			unset($job->_running_vars['todo'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				// If asset type list is empty, then none were set and therefore we do all asset
				// types. If set, then it needs to be a key in the $asset_types
				if (empty($asset_types) || isset($asset_types[$asset->type()])) {
					$step_data['message'] = sprintf(translate('Checking metadata for %s ...'), $asset->name);


					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					$field_values = $mm->getMetadataFieldValues($asset->id);

					$empty_fields = FALSE;
					$content = "\t".'<asset assetid="'.$asset->id.'" name="'.htmlSpecialChars($asset->name).'" short_name="'.htmlSpecialChars($asset->short_name).'" type_code="'.$asset->type().'" status="'.$asset->status.'">'."\n";
					foreach ($field_values as $field_name => $value) {
						$value = trim($value);
						if (empty($value)) {
							$content .= "\t\t".'<field>'.$field_name.'</field>'."\n";
							$empty_fields = TRUE;
						}
					}
					$content .= "\t".'</asset>'."\n";

					// write out the XML if there were empty fields
					if ($empty_fields) fwrite($temp_file, $content);

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

				}
			} else {
				$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

				$job->_addError(sprintf(translate('Could not generate incomplete metadata report for asset #%s, asset does not exist'), $assetid));

			}

			// add this assetid to the done array so we dont do it again
			$job->_running_vars['done'][] = $assetid;

		}//end if

		if (empty($job->_running_vars['todo'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$job->_running_vars['complete'] = TRUE;

			// move the temp file to become the real report XML file
			fwrite($temp_file, "</assets>\n");
			if (!copy($owner->data_path.'/report.tmp', $owner->data_path.'/report.xml')) {
				trigger_localised_error('CORE0200', E_USER_WARNING);
				return FALSE;
			}
			if (!unlink($owner->data_path.'/report.tmp')) {
				trigger_localised_error('CORE0199', E_USER_WARNING);
			}
		} else {
			$total = count($job->_running_vars['todo']) + count($job->_running_vars['done']);
			$step_data['percent_done'] = (count($job->_running_vars['done']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}
		fclose($temp_file);
		return TRUE;

	}//end generateReport()


	/**
	* Paint the interface for viewing a report
	*
	* @param Report_Incomplete_Metadata	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the backend outputter object
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(Report_Incomplete_Metadata $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$report_path = $asset->data_path.'/report.xml';
		if (!is_file($report_path)) {
			echo translate('This report has not been generated');

			return;
		}

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
			// trigger_localised_error('CORE0092', E_USER_WARNING, $report_path, $root->getMessage(), $root->getUserInfo());
			return;
		}

		// build the info array for printing
		$missing_fields = Array();
		foreach ($root->asset as $node) {
			$assetid = (int) $node->attributes()->assetid;
			foreach ($node->field as $field) {
				$missing_fields[$assetid][] = (string) $field;
			}
		}

		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {
			echo '<b>Note</b> The current incomplete metadata report is shown below. You can regenerate this report on the details screen.';
		}
		$o->closeSection();

		$o->openSection(translate('Missing Metadata Fields'));

			$o->openField('');
				if (empty($missing_fields)) {
					echo translate('No missing metadata fields were found');

					return;
				}

				?>
				<table class="sq-backend-table">
					<tr>
						<td colspan="4" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('Found In'); ?></td>

						<td class="sq-backend-table-header"><?php echo translate('Empty Fields'); ?></td>

					</tr>
				<?php
				foreach ($missing_fields as $assetid => $fields) {
					$asset = NULL;
					if ($assetid) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
					}
					if (is_null($asset)) {
						$asset_type = '';
						$asset_string = 'Unknown asset #'.$assetid;
						$asset_bg = 'FFFFFF';
						$asset_status = '';
					} else {
						$asset_type = $asset->type();
						$asset_bg = get_asset_status_icon($asset->status);
						$asset_status = get_status_description($asset->status);
						$asset_url = $asset->getBackendHref('metadata');
						$asset_string = '<u title="'.$asset->name.'">'.ellipsisize($asset->name, 40).'</u>';
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

						<td class="sq-backend-table-cell" style="width: 16px; padding-right: 2px; padding-left: 15px;">
							<?php
							if (!empty($asset_type)) {
								echo get_asset_type_icon($asset_type);
							}
							?>
						</td>
						<td class="sq-backend-table-cell" style="padding-left: 2px;">
							<?php echo $asset_string; ?>
						</td>
						<td class="sq-backend-table-cell" style="padding-right: 1px;">
							<?php echo $asset_bg; ?><?php echo $asset_status; ?>
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


	/**
	* Paint the current asset types that have been added
	*
	* @param Report_Incomplete_Metadata	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the backend outputter object
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentAssetTypes(Report_Incomplete_Metadata $asset, Backend_Outputter $o, $prefix)
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

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param Report_Incomplete_Metadata	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the backend outputter object
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processCurrentAssetTypes(Report_Incomplete_Metadata $asset, Backend_Outputter $o, $prefix)
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
	* @param Report_Incomplete_Metadata	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the backend outputter object
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintAddAssetType(Report_Incomplete_Metadata $asset, Backend_Outputter $o, $prefix)
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

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param Report_Incomplete_Metadata	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the backend outputter object
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processAddAssetType(Report_Incomplete_Metadata $asset, Backend_Outputter $o, $prefix)
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
