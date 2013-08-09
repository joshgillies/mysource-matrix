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
* $Id: tool_clear_matrix_cache.inc,v 1.9.8.1 2013/08/09 06:15:35 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Clear_Matrix_Cache
*
* Purpose
*
* Tool to clear Matrix cache, and to schedule a clear cache cron job
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.9.8.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Tool_Clear_Matrix_Cache extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid  the asset id to be loaded
	*
	*/
	function Tool_Clear_Matrix_Cache($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	public static function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);

		// caching is not enabled
		$cache_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		if (!$cache_manager->attr('enabled')) {
			$o->openSection($tool_info['name']);
			$o->openField('Note');
				$o->note('<span class="sq-backend-warning">Matrix caching status is currently "Off".</span><br />');
			$o->closeField();
			$o->closeSection();
			return;
		}

		$o->openSection($tool_info['name']);

		// interface to get to asset using asset finder
		$o->openField(translate('choose_asset'));
			asset_finder($type_code.'_assetid');
		$o->closeField();

		// level option
		$o->openField(translate('level'));
			$options = Array(
						'single'		=> translate('this_asset'),
						'dependants'	=> translate('this_asset_and_dependants'),
						'children'		=> translate('this_asset_and_children'),
					   );
			combo_box($type_code.'_level', $options, '', FALSE);
		$o->closeField();

		// asset types dropdown
		$o->openField(translate('asset_types'));
			$type_options = Array();
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			foreach ($asset_types as $code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$type_options[$code] = $info['name'];
				}
			}
			asort($type_options);
			$type_options = array_reverse($type_options, TRUE);
			$type_options[''] = '-- '.translate('all_asset_types').' --';
			$type_options = array_reverse($type_options, TRUE);
			combo_box($type_code.'_type_codes', $type_options, TRUE, '', 8);
		$o->closeField();

		// if repopulate box is checked, will request page using CURL
		$o->openField(translate('tool_matrix_cache_populate_question'));
			$disabled = (function_exists('curl_init')) ? '' : 'disabled';
			check_box($type_code.'_repopulate', TRUE, FALSE, '', $disabled);
			echo translate('tool_matrix_cache_populate_note');
			if ($disabled) {
				// we cannot repopulate if libcurl is not installed
				$o->note('<span class="sq-backend-warning">Note: PHP libcurl package is not installed</span>');
			}
			echo '<br /><br />';
			$o->note(translate('tool_matrix_cache_note'));
		$o->closeField();

		// if clear now box is checked, cache is cleared at commit
		$o->openField(translate('tool_matrix_cache_clear_now_field'));
			check_box($type_code.'_clear_now', TRUE);
		$o->closeField();

		$o->closeSection();


		// scheduled jobs section
		$o->openSection(translate('matrix_cache_schedule_section'));

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('Cron_Job_Clear_Matrix_Cache');
		$purge_jobs = Cron_Job_Clear_Matrix_Cache::getActiveJobs();

		// print scheduled jobs, one_off or repeating
		if (!empty($purge_jobs)) {
			$o->openField(translate('matrix_cache_scheduled_job_field'), 'new_line');
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('asset'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('when'); ?></td>
					<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('delete_question'); ?></td>
				</tr>
				<?php
					for ($i = 0; $i < count($purge_jobs); $i++) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo get_asset_tag_line($purge_jobs[$i]->attr('ownerid')); ?>

						</td>
						<td class="sq-backend-table-cell">
							<?php echo $purge_jobs[$i]->readableWhen(); ?>
						</td>
						<td align="center" class="sq-backend-table-cell">
						<?php
							check_box($type_code.'_remove_fs[]', $purge_jobs[$i]->id);
						?>
						</td>
					</tr>
					<?php
				}//end for
				?>
			</table><br />
			<?php
			$o->closeField();
		}//end if

		$cron_epoc = (int)$cron_mgr->attr('epoch');
		$cron_refresh = (int)$cron_mgr->attr('refresh_time');
		if ($cron_epoc > 0 && $cron_refresh > 0) {

			// prints the interface for adding a new one-off job
			$o->openField(translate('matrix_cache_one_off_field'), 'new_line');
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$new_job = new Cron_Job_Clear_Matrix_Cache();
				$edit_fns = $new_job->getEditFns();
				$edit_fns->paintInLineBackend($new_job, $o, $type_code.'_one_off');
				echo '&nbsp;<br /><br />';
			$o->closeField();

			// prints the interface for adding a new repeating job
			$o->openField(translate('matrix_cache_repeating_field'), 'new_line');
				$new_job = new Cron_Job_Clear_Matrix_Cache();
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$new_job->setAttrValue('type', 'repeating');
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$edit_fns = $new_job->getEditFns();
				$edit_fns->paintInLineBackend($new_job, $o, $type_code.'_repeating');
				$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));
			$o->closeField();

		} else {
			$o->openField('Note');
				$o->note(translate('cron_manager_not_configured'));
			$o->closeField();
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cache_manager);

		$o->closeSection();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	public static function processTool(&$o, $type_code)
	{
		$cache_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		if (!$cache_manager->attr('enabled')) return;

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('Cron_Job_Clear_Matrix_Cache');

		// remove cron jobs
		if (!empty($_POST[$type_code.'_remove_fs'])) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('Cron_Job_Clear_Matrix_Cache');
			$purge_jobs = Cron_Job_Clear_Matrix_Cache::getActiveJobs();
			if (!empty($purge_jobs)) {
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for ($i = 0; $i < count($purge_jobs); $i++) {
						if (in_array($purge_jobs[$i]->id, $_POST[$type_code.'_remove_fs'])) {
							$cron_mgr->removeJob($purge_jobs[$i]);
						}
					}

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');
				} else {
					trigger_localised_error('SYS0232', E_USER_NOTICE);
				}
			}
			// remove job only, do not print error msg
			if (!$_REQUEST[$type_code.'_assetid']['assetid']) {
				return FALSE;
			}
		}

		if (isset($_POST[$type_code.'_assetid']) && !empty($_POST[$type_code.'_assetid']['assetid'])) {
			// running vars
			$assetid = $_POST[$type_code.'_assetid']['assetid'];
			$level = (isset($_POST[$type_code.'_level'])) ? $_POST[$type_code.'_level'] : 'single';
			$type_codes = Array();
			if (isset($_POST[$type_code.'_type_codes']) && !empty($_POST[$type_code.'_type_codes'][0])) {
				$type_codes = $_POST[$type_code.'_type_codes'];
			}
			$repopulate = FALSE;
			if (isset($_POST[$type_code.'_repopulate']) && $_POST[$type_code.'_repopulate']) {
				$repopulate = TRUE;
			}
			$run_now = isset($_POST[$type_code.'_clear_now']) && $_POST[$type_code.'_clear_now'];
			// run HIPO job if the 'clear now' box is checked
			if ($run_now) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array(
							'assetid'		=> Array ($assetid),
							'level'			=> $level,
							'type_codes'	=> $type_codes,
							'delete_all'	=> 'no',
							'repopulate'	=> $repopulate,
						);
				$hh->queueHipo('hipo_job_clear_cache', $vars, $_SERVER['REQUEST_URI'].'&tool_type_code=tool_clear_matrix_cache');
				$url = $hh->runQueuedJobs();
				if (!empty($url)) $o->setRedirect($url);
				return TRUE;
			}

			// one off
			$new_job = new Cron_Job_Clear_Matrix_Cache();
			$edit_fns = $new_job->getEditFns();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$new_job->setAttrValue('ownerid', $assetid);
				$new_job->setAttrValue('level', $level);
				$new_job->setAttrValue('type_codes', $type_codes);
				$new_job->setAttrValue('repopulate', $repopulate);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			if ($edit_fns->processInLineBackend($new_job, $o, $type_code.'_one_off')) {
				$cron_mgr->addJob($new_job, $GLOBALS['SQ_SYSTEM']->user);
			}

			// repeating
			$new_job = new Cron_Job_Clear_Matrix_Cache();
			$edit_fns = $new_job->getEditFns();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$new_job->setAttrValue('type', 'repeating');
				$new_job->setAttrValue('ownerid',$assetid);
				$new_job->setAttrValue('level', $level);
				$new_job->setAttrValue('type_codes', $type_codes);
				$new_job->setAttrValue('repopulate', $repopulate);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			if ($edit_fns->processInLineBackend($new_job, $o, $type_code.'_repeating')) {
				$cron_mgr->addJob($new_job, $GLOBALS['SQ_SYSTEM']->user);
			}


		}//end if asset id is set

		return FALSE;

	}//end processTool()


}//end class
?>
