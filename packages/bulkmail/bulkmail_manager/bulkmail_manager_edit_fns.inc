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
* $Id: bulkmail_manager_edit_fns.inc,v 1.24 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office_edit_fns.inc';


/**
* Bulkmail_Manager_Edit_Fns
*
* Purpose
*
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/


class Bulkmail_Manager_Edit_Fns extends Bulkmail_Post_Office_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		
		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Paint the job queue
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintJobQueue(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField('');
			$jobs = $asset->getQueuedJobs();
			$this->_paintJobQueue($asset, $jobs);
		$o->closeField();

		return $write_access;

	}//end paintJobQueue()


	/**
	* Process the job queue
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processJobQueue(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if (!$write_access) return;

		$jobs = $asset->getQueuedJobs();
		$this->_processJobQueue($asset, $jobs);

		return TRUE;

	}//end processJobQueue()


	/**
	* Process the job queue
	*
	* @param object	&$asset	the asset to which we belong
	* @param array	$jobs	an array of jobs
	*
	* @return boolean
	* @access public
	*/
	function _processJobQueue(&$asset, $jobs)
	{
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');

		foreach ($jobs as $job_id => $job_info) {
			$prefix = $asset->getPrefix();
			if (!empty($_REQUEST[$prefix.'_'.$job_id])) {
				switch ($_REQUEST[$prefix.'_'.$job_id]) {
					case 'resume':
						$bulkmail_manager->updateJob($job_id, BML_JOB_STATE_NOT_RUNNING);
					break;
					case 'pause':
						$bulkmail_manager->updateJob($job_id, BML_JOB_STATE_PAUSED);
					break;
					case 'cancel':
						// log a message if this is not an ad-hoc job
						$job_id_parts = explode(':', $job_id);
						if (!isset($job_id_parts[1])) {
							$msg_reps = Array(
											'user_id'	=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
										);
							$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
							$message = $ms->newMessage(Array(), 'bulkmail.job.cancelled', $msg_reps);
							$message->parameters['assetid'] = $job_id;
							$message->send();
						}
						$bulkmail_manager->deleteJob($job_id);
					break;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bulkmail_manager);
			}
		}

		return;

	}//end _processJobQueue()


	/**
	* Prints out a list of jobs in the queue depending on the current context
	*
	* @param object	&$asset	the asset to which we belong
	* @param array	$jobs	an array of jobs
	*
	* @return string
	* @access public
	* @see Bulkmail_Manager_Edit_Fns::printJobQueue()
	* @see Bulkmail_Manager::getQueuedJobs()
	*/
	function _paintJobQueue(&$asset, $jobs)
	{
		if (empty($jobs)) return;

		$write_access = $asset->writeAccess('attributes');

		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');

		?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('bm_job_table_header_id'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('bm_job_table_header_job'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('bm_job_table_header_post_office'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('bm_job_table_header_state'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('bm_job_table_header_progress'); ?></th>
					<?php
						if ($write_access) {
					?><th class="sq-backend-table-header"><?php echo translate('bm_job_table_header_control'); ?></th><?php
						}
					?>
				<tr>
		<?php
			foreach ($jobs as $jobid => $job_info) {
				$jobid_parts = explode(':', $jobid);
				if (isset($jobid_parts[1])) {
					$name = translate('bm_adhoc_job_name');
				} else {
					$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($jobid), Array(), TRUE, 'name');
					$name = array_shift($asset_info);
				}

				$progress = $job_info['progress'];
				$progress_string = translate('bm_job_progress_none');
				if (!empty($progress) && $progress['total_count'] != 0) {
					$percent_done = round(($progress['current_count'] / $progress['total_count']) * 100, 2).'%';
					$progress_string = translate('bm_job_progress', number_format($progress['current_count']), number_format($progress['total_count']), $percent_done);
				}
		?>
				<tr>
					<td align="left" class="sq-backend-table-cell"><?php echo $jobid; ?></td>
					<td align="left" class="sq-backend-table-cell"><?php echo $name; ?></td>
					<td align="left" class="sq-backend-table-cell"><?php echo get_asset_tag_line($job_info['po_id']); ?></td>
					<td align="left" class="sq-backend-table-cell">
					<?php
						switch ($job_info['status']) {
							case BML_JOB_STATE_RUNNING:
								echo translate('bm_state_running');
							break;
							case BML_JOB_STATE_NOT_RUNNING:
								echo translate('bm_state_not_running');
							break;
							case BML_JOB_STATE_PAUSED:
								echo translate('bm_state_paused');
							break;
							case BML_JOB_STATE_SLEEPING:
								echo translate('bm_state_sleeping');
							break;
							default:
								echo translate('bm_state_unknown');
							break;
						}
					?>
					</td>
					<td align="left" class="sq-backend-table-cell"><?php echo $progress_string; ?></td>
					<?php
						if ($write_access) {
					?>
					<td align="left" class="sq-backend-table-cell">
					<?php
						$options = Array(
									''			=> '',
									'cancel'	=> translate('bm_state_action_cancel'),
								   );
						switch ($job_info['status']) {
							case BML_JOB_STATE_RUNNING:
							case BML_JOB_STATE_NOT_RUNNING:
								$options['pause'] = translate('bm_state_action_pause');
							break;
							case BML_JOB_STATE_PAUSED:
								$options['resume'] = translate('bm_state_action_resume');
							break;
							default:
						}

						combo_box($asset->getPrefix().'_'.$jobid, $options, FALSE, '');
					?>
					</td>
					<?php
						}
					?>
				<tr>
		<?php
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($post_office);
			}//end foreach
		?>
			</table>
		<?php

	}//end _paintJobQueue()


	/**
	* Function that determine whether to hide or show the progress section for bulkmail manager
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function showProgressSection(&$asset, $prefix)
	{
		$jobs = $asset->getQueuedJobs();
		return (empty($jobs) ? FALSE : TRUE);

	}//end showProgressSection()


	/**
	* Paints the threshold setting checkbox, whether to override all settings or not
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintThresholdSettingCheckBox(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$use_bm_threshold = $asset->attr('use_bm_threshold');

		// paint checkbox
		if ($write_access) {
			check_box($prefix.'_use_bm_threshold', TRUE, $use_bm_threshold);
		} else {
			?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $use_bm_threshold ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
			<?php
		}

		return $write_access;

	}//end paintThresholdSettingCheckBox()


	/**
	* Process the threshold setting checkbox
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processThresholdSettingCheckBox(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		// save whether to use bulkmail manager threshold settings
		$use_bm_threshold = isset($_POST[$prefix.'_use_bm_threshold']);
		$asset->setAttrValue('use_bm_threshold', $use_bm_threshold);

		return TRUE;

	}//end processThresholdSettingCheckBox()


}//end class
?>
