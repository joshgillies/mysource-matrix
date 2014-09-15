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
* $Id: report_safe_trash_edit_fns.inc,v 1.9 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
/**
* Report_Safe_Trash_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Safe_Trash_Edit_Fns extends Report_Edit_Fns
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
		$owner = $GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_safe_trash');

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

			if (!is_null($asset) && !($asset instanceof Root_Folder)) {
				// If asset type list is empty, then none were set and therefore we do all asset
				// types. If set, then it needs to be a key in the $asset_types
				if (empty($asset_types) || isset($asset_types[$asset->type()])) {
					$job_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, 'updating', 'minor');
					if ($job_link) {

						$cron_job = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_link['majorid']);
						$step_data['message'] = sprintf(translate('core_checking_for_metadata'), $asset->name);

						$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						$field_values = $mm->getMetadataFieldValues($asset->id);
						$trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($asset->id);

						flush();

						$content = "\t".'<asset assetid="'.$asset->id.'" name="'.htmlspecialchars($asset->name).'" short_name="'.htmlSpecialChars($asset->short_name).'" type_code="'.$asset->type().'" status="'.$asset->status.'" marked_since="'.$cron_job->created.'">';

						if (!empty($trash_errors['links'])) {
							$content .= '<unsafe_links>';
							foreach ($trash_errors['links'] as $link) {
								$content .= '	<assetid>'.$link['majorid'].'</assetid>';
							}
							$content .= '</unsafe_links>';
						}

						if (!empty($trash_errors['children'])) {
							$content .= '<unsafe_children>';
							foreach ($trash_errors['children'] as $link) {
								$content .= '	<assetid>'.$link['minorid'].'</assetid>';
							}
							$content .= '</unsafe_children>';
						}

						$content .= '</asset>'."\n";

						// write out the XML if there were any trash errors
						if (!empty($trash_errors)) fwrite($temp_file, $content);
					} else {
						$step_data['message'] = sprintf(translate('hipo_skipping_asset'), $assetid).' - NO JOB';
					}

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					$step_data['message'] = sprintf(translate('hipo_skipping_asset'), $assetid);
				}
			} else {
				$step_data['message'] = sprintf(translate('hipo_skipping_asset'), $assetid);
				$job->_addError(sprintf(translate('core_cannot_generate_incomplete_metadata_report'), $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$job->_running_vars['done'][] = $assetid;

		}//end if

		if (empty($job->_running_vars['todo']) && !$job->_running_vars['complete']) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;

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
	* @param Report_Safe_Trash	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(Report_Safe_Trash $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$report_path = $asset->data_path.'/report.xml';
		if (!is_file($report_path)) {
			echo translate('report_not_generated');
			return;
		}

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

		$missing_fields = Array();

		foreach ($root->asset as $result) {
			$id = (int) $result->attributes()->assetid;
			$missing_fields[$id]['since'] = (string) $result->attributes()->marked_since;

			foreach ($result->unsafe_children as $unsafe_children) {
				foreach ($unsafe_children->assetid as $unsafe_assetid) {
					$missing_fields[$id]['children'][] = (int) $unsafe_assetid;
				}
			}
			foreach ($result->unsafe_links as $unsafe_links) {
				foreach ($unsafe_links->assetid as $unsafe_assetid) {
					$missing_fields[$id]['links'][] = (int) $unsafe_assetid;
				}
			}
		}//end foreach

		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {
			echo '<b>Note</b> The current safe trash report is shown below. You can regenerate this report on the <a href="'.$asset->getBackendHref('details').'">details screen</a>.';
		}
		$o->closeSection();

		$o->openSection(translate('report_safe_trash_section_status'));
			$o->openField('');
				if (empty($missing_fields)) {
					echo 'NO ASSET FOUND';
					return;
				}
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><?php echo translate('asset'); ?></td>
						<td class="sq-backend-table-header" style="width: 180px"><?php echo translate('report_safe_trash_marked_since'); ?></td>
						<td class="sq-backend-table-header" style="width: 150px"><?php echo translate('status'); ?></td>
						<td class="sq-backend-table-header" style="width: 150px"><?php echo translate('report_safe_trash_unsafe_links'); ?></td>
						<td class="sq-backend-table-header" style="width: 150px"><?php echo translate('report_safe_trash_unsafe_children'); ?></td>
					</tr>
				<?php
				foreach ($missing_fields as $assetid => $fields) {
					$asset_tmp = NULL;
					if ($assetid) {
						$asset_tmp = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
					}
					if (is_null($asset_tmp)) {
						$asset_type = '';
						$asset_string = 'Unknown asset #'.$assetid;
						$asset_bg = 'FFFFFF';
						$asset_status = '';
					} else {
						$asset_type = $asset_tmp->type();
						$asset_bg = get_asset_status_icon($asset_tmp->status);
						$asset_status = get_status_description($asset_tmp->status);
						$asset_url = $asset_tmp->getBackendHref('metadata');
						$asset_string = '<a href="'.$asset_tmp->getBackendHref('details').'" title="'.$asset_tmp->name.'">'.ellipsisize($asset_tmp->name, 40).'</a>';
					}

					?>
					<tr>
						<td class="sq-backend-table-cell" style="padding-right: 1px;">
							<?php
							if (!empty($asset_type)) {
								echo get_asset_tag_line($asset_tmp->id, 'details');
							}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo easy_datetime((int)$missing_fields[$asset_tmp->id]['since']); ?>
						</td>
						<td class="sq-backend-table-cell" style="padding-right: 1px;">
						<?php
						if ($asset_tmp->status >= SQ_STATUS_LIVE) {
							?>
							<span class="sq-backend-warning">
							<?php
						}
						?>
						<?php echo $asset_bg; ?><?php echo $asset_status; ?>
						<?php
						if ($asset_tmp->status >= SQ_STATUS_LIVE) {
							?>
							</span>
							<?php
						}
						?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							if (empty($missing_fields[$asset_tmp->id]['links'])) {
								echo translate('none');
							} else {
								?>
								<ul style="list-style-type: none; padding-left: 0; margin-left: 0;">
								<?php
								foreach ($missing_fields[$asset_tmp->id]['links'] as $linked_assetid) {
									?>
									<li class="sq-backend-warning"><?php echo get_asset_tag_line($linked_assetid, 'details'); ?></li>
									<?php
								}
								?>
								</ul>
								<?php
							}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							if (empty($missing_fields[$asset_tmp->id]['children'])) {
								echo translate('none');
							} else {
								?>
								<ul style="list-style-type: none; padding-left: 0; margin-left: 0;">
								<?php
								foreach ($missing_fields[$asset_tmp->id]['children'] as $linked_assetid) {
									?>
									<li class="sq-backend-warning"><?php echo get_asset_tag_line($linked_assetid, 'details'); ?></li>
									<?php
								}
								?>
								</ul>
								<?php
							}
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
	* @param Report_Safe_Trash	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentAssetTypes(Report_Safe_Trash $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('content');

		$types = $asset->attr('types');
		if (empty($types)) {
			echo translate('core_no_types_defined');
			return FALSE;
		}

		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header"><?php echo translate('asset_type'); ?></td>
			<td class="sq-backend-table-header"><?php echo translate('inherit_types'); ?></td>
			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header"><?php echo translate('remove_question'); ?></td>
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
							echo ($inherit) ? translate('yes') : translate('no');
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
	* @param Report_Safe_Trash	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processCurrentAssetTypes(Report_Safe_Trash $asset, Backend_Outputter $o, $prefix)
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
	* @param Report_Safe_Trash	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintAddAssetType(Report_Safe_Trash $asset, Backend_Outputter $o, $prefix)
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
	* @param Report_Safe_Trash	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processAddAssetType(Report_Safe_Trash $asset, Backend_Outputter $o, $prefix)
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
