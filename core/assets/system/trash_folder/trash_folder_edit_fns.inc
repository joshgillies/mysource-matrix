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
* $Id: trash_folder_edit_fns.inc,v 1.44 2012/08/30 01:09:12 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Trash_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trash_Folder_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trash_Folder_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

		$this->static_screens['details']['name'] = 'Purge Trash';
		$this->static_screens['details']['lock_type'] = 'none';
		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Prints the interface for purging the trash
	*
	* @param object	&$asset	the trash asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPurgeTrash(&$asset, &$o, $prefix)
	{
		if ($asset->purgeAccess()) {
			$num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id);
			if (!$num_links) {
				$o->openField('');
					echo translate('The trash is currently empty');
				$o->closeField();
				return TRUE;
			} else {
				$o->openField(translate('Purge Root Node'));

					asset_finder($prefix.'_purge_root');
					$o->note(translate('Only this asset and its children will be purged. If no purge root is selected, all assets will be purged.'));

				$o->closeField();

				$o->openField(translate('Link Age'));

				text_box($prefix.'_link_age_units', 0, 3, '', TRUE, 'min="0" max="999"', 'number');
				echo ' ';
				$periods = Array(
							'-'			=> '--Select One --',
							'minutes'	=> translate('minutes'),
							'hours'		=> translate('hours'),
							'days'		=> translate('days'),
							'weeks'		=> translate('weeks'),
							'months'	=> translate('months'),
							'years'		=> translate('years'),
						   );

				combo_box($prefix.'_link_age_period', $periods, FALSE, '-', NULL);

				$o->note(translate('If set, only links in the trash that are older than the specified time period will be purged.'));

				$o->closeField();

				$o->openField(translate('Purge Trash?'));

				combo_box($prefix.'_purge_trash', Array(0 => translate('No'), 1 => translate('Yes')), FALSE, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm(js_translate('Are you sure you want to purge the selected assets from the trash?'))) { this.value = 0; } } \"");

				$o->note(translate('This will purge the selected assets from the trash and make them unavailable for restoring or editing.'));

				$o->closeField();

				return TRUE;
			}
		}
		return FALSE;

	}//end paintPurgeTrash()


	/**
	* Process function for paintPurgeTrash, but doesnt actually do anything
	* All the real work is done in paintBackend for the trash
	*
	* @param object	&$asset	the trash asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* @see Trash_Folder::paintBackend()
	*/
	function processPurgeTrash(&$asset, &$o, $prefix)
	{
		if (!$asset->purgeAccess()) return FALSE;

		if (isset($_POST[$prefix.'_purge_trash']) && $_POST[$prefix.'_purge_trash']) {
			//// START HIPO PROCESSING ////
			$on_complete = $_SERVER['REQUEST_URI'].'&'.$prefix.'_purge_complete=1';
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$purge_root_linkid  = $_POST[$prefix.'_purge_root']['linkid'];
			$purge_root_assetid = $_POST[$prefix.'_purge_root']['assetid'];

			$link_age_time   = array_get_index($_POST, $prefix.'_link_age_units', 0);
			$link_age_period = array_get_index($_POST, $prefix.'_link_age_period', '-');

			$vars = Array();
			// check to see if the link age or the period is set but not the other
			// for this we need to abort the purge process as it seems like user
			// didn't set it up correctly and we don't want ot purge the whole trash
			if ((int) $link_age_time !== 0 && $link_age_period === '-') {
				trigger_error(translate('Link age was configured on the screen but the period was not selected. Aborting Purge Trash process.'), E_USER_WARNING);
				return FALSE;
			} else if ((int) $link_age_time === 0 && $link_age_period !== '-') {
				trigger_error(translate('Link age was not configured on the screen but the period was selected. Aborting Purge Trash process.'), E_USER_WARNING);
				return FALSE;
			} else if ((int) $link_age_time !== 0 && $link_age_period !== '-') {
				$vars['link_age'] = $link_age_time.' '.$link_age_period;
			}

			// Bug #3663 - If the assetid is typed in, try and pick it up
			if (empty($purge_root_linkid) && !empty($purge_root_assetid)) {
				// handle non valid and non existant assetids gracefully Squizmap #7978
				if (assert_valid_assetid($purge_root_assetid, '', FALSE, FALSE)) {
					$purge_root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($purge_root_assetid, '', TRUE);
				} else {
					$purge_root_asset = NULL;
					return FALSE;
				}
				
				if (!is_null($purge_root_asset)) {
					$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $purge_root_asset->id);
					
					// if no link in the trash abort the purge Squizmap #7861
					if (empty($link_info)) {
						trigger_error(translate('Purge root needs to be a direct child of the Trash Folder. Aborting Purge Trash process.'), E_USER_WARNING);
						return FALSE;
					}

					$purge_root_linkid = array_get_index($link_info, 'linkid', '');
				} else {
					// entered assetid is too long or is invalid, abort the purge Squizmap #7861
					trigger_error(translate('Purge root does not exist. Aborting Purge Trash process.'), E_USER_WARNING);
					return FALSE;
				}//end if
			}//end if

			// Let's go trashing!
			$vars = array_merge(Array('purge_root_linkid' => $purge_root_linkid), $vars);

			$hh->queueHipo('hipo_job_purge_trash', $vars, $on_complete);

			////  END HIPO PROCESSING  ////
			return TRUE;
		}
		return FALSE;

	}//end processPurgeTrash()


	/**
	* Prints the interface for configuring the scheduled job for purging the trash
	*
	* @param object	&$asset	the trash asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPurgeTrashScheduling(&$asset, &$o, $prefix)
	{
		if ($asset->purgeAccess()) {
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';

			$scheduling_status = (boolean) $asset->attr('scheduling_status');
			$o->openField(translate('Schedule Trash Purging'));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-cell-alt" style="width:120px">
						<?php 
							echo translate('Scheduled Trash Purging');
						?> 
					</td>
					<td class="sq-backend-table-cell">
						<?php
							$options = Array(
										1 => translate('Enabled'),
										0 => translate('Disabled'),
									   );

							combo_box($prefix.'_scheduling_status', $options, FALSE, $scheduling_status);
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell-alt" style="width:120px">
						<?php 
							echo translate('Purge Root Node');
						?> 
					</td>
					<td class="sq-backend-table-cell">
						<?php
							$selected_asset_root_node = $asset->attr('job_purge_node');
							asset_finder($prefix.'_job_purge_root', $selected_asset_root_node);
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell-alt" style="width:120px">
						<?php 
							echo translate('Link Age');
						?> 
					</td>
					<td class="sq-backend-table-cell">
						<?php
							echo translate('Only purge links older than &nbsp;');
							$job_link_age = $asset->attr('job_link_age');
							$link_age_bits = explode(' ', $job_link_age);
							$link_age_units = (array_get_index($link_age_bits, 0, '') == '') ? 0 : $link_age_bits[0];

							text_box($prefix.'_sched_link_age_units', $link_age_units, 3, '', TRUE, 'min="0" max="999"', 'number');
							echo ' ';
							$periods = Array(
										'-'			=> '--Select One --',
										'minutes'	=> translate('minutes'),
										'hours'		=> translate('hours'),
										'days'		=> translate('days'),
										'weeks'		=> translate('weeks'),
										'months'	=> translate('months'),
										'years'		=> translate('years'),
									   );

							combo_box($prefix.'_sched_link_age_period', $periods, FALSE, array_get_index($link_age_bits, 1, '-'), NULL);

						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell-alt" style="width:120px">
						<?php 
							echo translate('Schedule');
						?> 
					</td>
					<td class="sq-backend-table-cell" style="vertical-align:middle;">
						<?php
							$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
							$purge_job = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_job_purge_trash');
							$cron_epoc = (int)$cron_mgr->attr('epoch');
							$cron_refresh = (int)$cron_mgr->attr('refresh_time');

							if ($cron_epoc > 0 && $cron_refresh > 0) {
								$edit_fns = $purge_job->getEditFns();
								$when = $purge_job->attr('when');

								ob_start();
									$edit_fns->paintInLineBackend($purge_job, $o, $prefix.'_new_pt', $when);
								$contents = ob_get_contents();
								ob_end_clean();

								echo $contents;
							} else {
								echo translate('The Scheduled Jobs Manager has not been configured. No automated purging can be set.');

							}

							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);
						?>
					</td>
				</tr>
			</table>
			<?php

			$o->note(sprintf(translate('The Scheduled Jobs Manager runs every <b>%1$s</b>. It will next run on the <b>%2$s</b>.'), $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));
			$o->closeField();
		}
		return FALSE;

	}//end paintPurgeTrashScheduling()


	/**
	* Processes the interface for configuring the scheduled job for purging the trash
	*
	* @param object	&$asset	the trash asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processPurgeTrashScheduling(&$asset, &$o, $prefix)
	{
		$job_scheduled = array_get_index($_POST, $prefix.'_scheduling_status', FALSE);

		$purge_root_assetid = $_POST[$prefix.'_job_purge_root']['assetid'];
		$link_age_time   = array_get_index($_POST, $prefix.'_sched_link_age_units', 0);
		$link_age_period = array_get_index($_POST, $prefix.'_sched_link_age_period', '-');

		// check to see if the link age or the period is set but not the other
		// for this we need to abort the purge process as it seems like user
		// didn't set it up correctly and we don't want ot purge the whole trash
		if ((int) $link_age_time !== 0 && $link_age_period === '-') {
			trigger_error(translate('Link age was configured for scheduled job but the period was not selected. Job scheduling not saved.'), E_USER_WARNING);
			return $asset->setAttrValue('scheduling_status', FALSE);
		} else if ((int) $link_age_time === 0 && $link_age_period !== '-') {
			trigger_error(translate('Link age was not configured for scheduled job but the period was selected. Job scheduling not saved.'), E_USER_WARNING);
			return $asset->setAttrValue('scheduling_status', FALSE);
		} else if ((int) $link_age_time !== 0 && $link_age_period !== '-') {
			$link_age = $link_age_time.' '.$link_age_period;
		} else {
			$link_age = '';
		}

		$asset->setAttrValue('job_link_age', $link_age);
		$asset->setAttrValue('job_purge_node', $purge_root_assetid);

		$purge_job = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_job_purge_trash');
		$edit_fns = $purge_job->getEditFns();

		$edit_fns->processInLineBackend($purge_job, $o, $prefix.'_new_pt');
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED + SQ_SECURITY_LINK_INTEGRITY);
			$purge_job->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return $asset->setAttrValue('scheduling_status', $job_scheduled);

	}//end processPurgeTrashScheduling()


	/**
	* Returns TRUE if the current user can purge the trash
	*
	* @param object	&$asset	the trash asset
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function purgeAccess(&$asset, $prefix)
	{
		return $asset->purgeAccess();

	}//end purgeAccess()


}//end class

?>
