<?php


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
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/


class Bulkmail_Manager_Edit_Fns extends Bulkmail_Post_Office_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

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

		return true;

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
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');

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
		if (empty($jobs)) {
			//echo translate('bm_job_progress_none');
			return;
		}

		$write_access = $asset->writeAccess('attributes');

		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');

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
				$post_office =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job_info['post_office_id']);
				if (isset($jobid_parts[1])) {
					$data_dir = $post_office->data_path.'/.data/'.$jobid_parts[1];
					$name = translate('bm_adhoc_job_name');
				} else {
					$bulkmail_job =& $GLOBALS['SQ_SYSTEM']->am->getAsset($jobid);
					$data_dir = $bulkmail_job->data_path.'/.data';
					$name = $bulkmail_job->name;
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bulkmail_job);
				}
				// Now that we have the data directory, we can get progress information from it
				$progress = $bulkmail_manager->getJobProgress($jobid);
				$progress_string = translate('bm_job_progress_none');
				if (!empty($progress) && $progress['total_count'] != 0) {
					$percent_done = round(($progress['current_count'] / $progress['total_count']) * 100, 2).'%';
					$progress_string = translate('bm_job_progress', $progress['current_count'], $progress['total_count'], $percent_done);
				}
		?>
				<tr>
					<td align="left" class="sq-backend-table-cell"><?php echo $jobid; ?></td>
					<td align="left" class="sq-backend-table-cell"><?php echo $name; ?></td>
					<td align="left" class="sq-backend-table-cell"><?php echo $post_office->name.' (#'.$post_office->id.')'; ?></td>
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

						combo_box($asset->getPrefix().'_'.$jobid, $options, false, '');
					?>
					</td>
					<?php
						}
					?>
				<tr>
		<?php
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($post_office);
			}
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
		return (empty($jobs) ? false : true);

	}//end showProgressSection()


}//end class
?>
