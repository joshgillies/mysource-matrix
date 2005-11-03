<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_job_edit_fns.inc,v 1.8 2005/11/03 00:52:23 ndvries Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Bulkmail_Job_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Edit_Fns extends Bulkmail_Post_Office_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Job_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['preview']);

	}//end constructor


	/**
	* Paints the progress section of the details interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintProgress(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current_state = $asset->attr('current_state');
		$options = Array(
					BML_JOB_STATE_RUNNING	=> translate('bulkmail_progress_state_running'),
					BML_JOB_STATE_PAUSED	=> translate('bulkmail_progress_state_paused'),
					BML_JOB_STATE_NOT_LIVE	=> translate('bulkmail_progress_state_cancel'),
				   );
		$field_name = translate('bulkmail_progress_current_state');
		//$field_note = translate('bulkmail_progress_note');
		$o->openField($field_name); //, '', $field_note);
			if ($asset->status == SQ_STATUS_LIVE) {
				// only allow user to pause/resume if status is live
				if ($write_access) {
					$name = $prefix.'_progress_current_state';
					combo_box($name, $options, false, $current_state);
				} else {
					$options[BML_JOB_STATE_NOT_LIVE] = translate('bulkmail_progress_state_not_live');
					echo $options[$current_state];
				}
			} else {
				echo translate('bulkmail_progress_state_not_live');
			}
		$o->closeField();

		// get progress information from file
		$progress_path = $asset->getProgressPath();
		if (file_exists($progress_path)) {
			include $progress_path;
			$percent_done = round(($progress_info['current_count'] / $progress_info['total_count']) * 100, 2).'%';
			$detailed_progress = $progress_info['current_count'].' of '.$progress_info['total_count'];

			$field_name = translate('bulkmail_progress_current_progress');
			$o->openField($field_name);
				echo $percent_done;
			$o->closeField();

			$field_name = translate('bulkmail_progress_detailed_progress');
			$o->openField($field_name);
				echo $detailed_progress;
			$o->closeField();
		}

		// notify the user that changes will not be saved if the status is live
		if ($asset->status == SQ_STATUS_LIVE && $write_access) {
			$o->openField(translate('bulkmail_warning'));
				$warning = '<span class="sq-backend-warning">'.translate('bulkmail_progress_warning_save').'</span>';
				$o->note($warning);
			$o->closeField();
		}

		// errors
		$errors = $asset->canSetLive();
		if (!empty($errors)) {
			$o->openField(translate('bulkmail_warning'));
			foreach ($errors as $error) {
				$error = '<span class="sq-backend-warning">'.$error.'</span>';
				$o->note($error);
			}
			$o->closeField();
		}

		/*
		// debug
		$o->openField('Last Updated');
			echo date('r', $asset->attr('last_processed'));
		$o->closeField();

		$o->openField('Parent Process Id');
			echo $asset->attr('parent_process_pid');
		$o->closeField();

		$o->openField('Child Process Ids');
			$array = $asset->attr('child_process_pids');
			foreach ($array as $key => $val) {
				echo "$val. ";
			}
		$o->closeField();
*/
		return $write_access;

	}//end paintProgress()


	/**
	* Processes the progress section of the details interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processProgress(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		// TODO: if this job is finished (live but not in db table)
		// prevent the user from changing the state once the job is finished
		//$state = $asset->getAttrDB('current_state');
		//if ($state == BML_JOB_STATE_FINISHED) return false;

		if (isset($_POST[$prefix.'_progress_current_state'])) {

			// load progress info from file
			$progress_path = $asset->getProgressPath();
			if (file_exists($progress_path)) include $progress_path;

			$asset->setAttrValue('current_state', $_POST[$prefix.'_progress_current_state']);

			// job is paused
			if ($_POST[$prefix.'_progress_current_state'] == BML_JOB_STATE_PAUSED) {
				// reset timestamp to prevent timeout issues
				//$asset->setAttrValue('last_processed', 0);
				$progress_info['last_updated'] = 0;
			}
			// job is cancelled, set to under construction
			if ($_POST[$prefix.'_progress_current_state'] == BML_JOB_STATE_NOT_LIVE) {
				// reset the timestamp and process pid to initial state
				//$asset->setAttrValue('last_processed', 0);
				$progress_info['last_updated'] = 0;
				$asset->processStatusChange(SQ_STATUS_UNDER_CONSTRUCTION, false);
			}

			// write the updated progress info to file
			$output = '<?php $progress_info = '.var_export($progress_info, true).'; ?>';
			string_to_file($output, $progress_path);
		}

		return true;

	}//end processProgress()


//--        DETAILS        --//


	/**
	* Paints the header details interface for the bmail
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintHeaderDetails(&$asset, &$o, $prefix)
	{
		$width = 60;
		$write_access = $asset->writeAccess('attributes');
		$post_office =& $asset->getPostOffice();
		$details_setting = $asset->attr('details_setting');

		if (!is_null($post_office)) {
			// if checkbox is checked, use post office's header details
			$checkbox_enabled = '';
			$field_note = translate('bulkmail_header_detail_check_enabled');
			$hide_header =  array_get_index($details_setting, 'use_post_office_header', false);
		} else {
			// no inheritance otherwise
			$checkbox_enabled = 'disabled="true"';
			$field_note = translate('bulkmail_header_detail_check_disabled');
			$hide_header = false;
		}

		// paint checkbox, checkbox is not enabled unless job is linked under a post office
		$field_name = translate('bulkmail_header_detail_check_field_name');
		$o->openField($field_name, '', $field_note);
			if ($write_access) {
				check_box($prefix.'_use_post_office_header', true, $hide_header, 'hideHeader(this.checked)', $checkbox_enabled);
			} else {
				?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $hide_header ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
				<?php
			}
		$o->closeField();

		// javascript to disable input fields when the checkbox is checked
		?>
		<script language="JavaScript" type="text/javascript">
			function hideHeader(checked) {
				var prefix = '<?php echo $prefix; ?>';
				var regex_string1 = prefix + '_header_detail';
				var re1 = new RegExp(regex_string1);
				var elements = document.getElementsByTagName('tr');
				for (i = 0; i < elements.length; i++) {
					if (elements[i].id.match(re1)) {
						elements[i].style['display'] = checked ? 'none' : '';
					}
				}
			}
		</script>
		<?php

		// paint the actual header details
		return parent::paintHeaderDetails($asset, $o, $prefix, $hide_header);

	}//end paintHeaderDetails()


	/**
	* Process the header details for the bmail
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processHeaderDetails(&$asset, &$o, $prefix)
	{
		// once live, header details are not changeable
		if ($asset->status == SQ_STATUS_LIVE) return false;
		if (!$asset->writeAccess('attributes')) return false;

		// save whether to use post office header details
		$use_header_details = isset($_POST[$prefix.'_use_post_office_header']);
		$details_setting = $asset->attr('details_setting');
		$details_setting['use_post_office_header'] = $use_header_details ? true : false;
		$asset->setAttrValue('details_setting', $details_setting);

		// only process this if not inherit from parent
		if (!$use_header_details) {
			return parent::processHeaderDetails($asset, $o, $prefix);
		} else {
			return true;
		}

	}//end processHeaderDetails()


//--        JOB MANAGEMENT        --//


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
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		$o->openField('');
			$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
			$bulkmail_manager_edit_fns->_paintJobQueue($asset, $jobs);
		$o->closeField();

		return $write_access;

	}//end printJobQueue()


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
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		if (!$write_access) return;

		$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return true;

	}//end processJobQueue()


}//end class
?>