<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_job_edit_fns.inc,v 1.34 2008/11/24 22:28:19 mbrydon Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';


/**
* Bulkmail_Job_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.34 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Edit_Fns extends Bulkmail_Post_Office_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		unset($this->static_screens['preview']);
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


//--        VALIDATION        --//


	/**
	* Paints the section for user to check if a job is valid
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintValidation(&$asset, &$o, $prefix)
	{
		// set a hidden field with initial value of '0'
		// set value to '1' when the validate button is clicked
		 $o->openRaw();
			?><p style="margin-top: 5px; margin-bottom: 5px"><?php
				echo translate('bm_job_validation_validate_job');
				?><br /><?php
				echo normal_button($prefix.'_validation_button', translate('bm_job_validation_button'), 'onClick= validate(); this.form.submit();');
				echo hidden_field($prefix.'_validation_hidden', '0');
			?></p><?php
		$o->closeRaw();

		// report errors if the the hidden field is set to '1'
		if (isset($_REQUEST[$prefix.'_validation_hidden']) && $_REQUEST[$prefix.'_validation_hidden'] == 1) {
			$post_office = $asset->getPostOffice();
			$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$details_info = $bm->generateJobDetails($asset, $post_office);
			$errors = $bm->isValidJob($details_info, TRUE);
			$o->openField(translate('bm_job_validation_result'));
				if (empty($errors)) {
					echo translate('bm_job_validation_successful');
				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header"><?php echo translate('bm_configuration'); ?></th>
							<th class="sq-backend-table-header"><?php echo translate('bm_warning'); ?></th>
						</tr>
					<?php
					foreach ($errors as $error) {
						$error_msg = explode(':', $error, 2);
						?><tr>
							<td><?php echo $error_msg[0]; ?></td>
							<td><span class="sq-backend-warning"><?php echo $error_msg[1]; ?></span></td>
						</tr><?php
					}
					?></table><?php
				}
			$o->closeField();
		}

		// javascript to update the value of hidden validation form element
		?>
		<script language="JavaScript" type="text/javascript">
			function validate() {
				var prefix = '<?php echo $prefix; ?>';
				var id = prefix + '_validation_hidden';
				var element = document.getElementById(id);
				element.value = '1';
			}
		</script>
		<?php

	}//end paintValidation()


	/**
	* Process the section for user to check if a job is valid
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processValidation(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processValidation()


//--        WARNING        --//


	/**
	* Paint the 'changes in settings will not affect job-in-progress' warning message
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintWarning(&$asset, &$o, $prefix)
	{
		?><span class="sq-backend-warning"><?php echo translate('bm_warning_changes_no_effect'); ?></span><?php
		return TRUE;

	}//end paintWarning()


	/**
	* Process the 'changes in settings will not affect job-in-progress' warning message
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processWarning(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processWarning()


	/**
	* Returns TRUE when the changes in settings will not affect job-in-progress
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function showWarningSection(&$asset, $prefix)
	{
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$job = $bulkmail_manager->getQueuedJobs($asset->id);

		return (empty($job)) ? FALSE : TRUE;

	}//end showWarningSection()


//--        HEADER        --//


	/**
	* Paints the header detail checkbox, whether to use the post office header or not
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintHeaderDetailsCheckBox(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$use_post_office_header = $asset->attr('use_post_office_header');

		// paint checkbox
		if ($write_access) {
			check_box($prefix.'_use_post_office_header', TRUE, $use_post_office_header);
		} else {
			?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $use_post_office_header ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
			<?php
		}
		$post_office = $asset->getPostOffice();
		echo get_asset_tag_line($post_office->id);

		return $write_access;

	}//end paintHeaderDetailsCheckBox()


	/**
	* Process the header detail checkbox
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processHeaderDetailsCheckBox(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		// save whether to use post office header details
		$use_header_details = isset($_POST[$prefix.'_use_post_office_header']);
		$asset->setAttrValue('use_post_office_header', $use_header_details);

		return TRUE;

	}//end processHeaderDetailsCheckBox()


	/**
	* Check if we should use the post office header
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function showJobHeader(&$asset, $prefix)
	{
		return !($asset->attr('use_post_office_header'));

	}//end showJobHeader()


//--        PROGRESS        --//


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
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		$o->openField('');
			$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
			$bulkmail_manager_edit_fns->_paintJobQueue($asset, $jobs);
		$o->closeField();

				$o->openField('');
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$logs = $ms->getMessages(0, 'bulkmail.job.*', Array(), Array(), NULL, NULL, 'name', Array('assetid' => $asset->id));
			$logs = array_slice($logs, 0, 5);

			if (!empty($logs)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header"><?php echo translate('date'); ?></th>
						<th class="sq-backend-table-header"><?php echo translate('subject'); ?></th>
						<th class="sq-backend-table-header"><?php echo translate('body'); ?></th>
					</tr>
				<?php
				foreach ($logs as $log_entry) {
					?>
						<tr>
							<td align="left" class="sq-backend-table-cell"><?php echo ts_iso8601($log_entry['sent']); ?></td>
							<td align="left" class="sq-backend-table-cell"><?php echo $log_entry['subject']; ?></td>
							<td align="left" class="sq-backend-table-cell"><?php echo $log_entry['body']; ?></td>
						</tr>
					<?php
				}
				?></table><?php
			}
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
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		if (!$write_access) return;

		$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return TRUE;

	}//end processJobQueue()


	/**
	* Function that determine whether to hide or show the progress section for this job
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function showProgressSection(&$asset, $prefix)
	{
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$job = $bulkmail_manager->getQueuedJobs($asset->id);

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$logs = $ms->getMessages(0, 'bulkmail.job.*', Array(), Array(), NULL, NULL, 'name', Array('assetid' => $asset->id));
		$logs = array_slice($logs, 0, 5);

		return ((empty($job) && empty($logs)) ? FALSE : TRUE);

	}//end showProgressSection()


//--        PREVIEWING        --//


	/**
	* Paints the screen to send preview mail to the current user
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintPreviewAddress(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		$email = $current_user->attr('email');

		if ($write_access) {
			text_box($prefix.'_preview_address', $email);
		} else {
			echo $email;
		}

		return $write_access;

	}//end paintPreviewAddress()


	/**
	* Sends the preview bmail to default/alternative address
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPreviewAddress(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$recipient = array_get_index($_REQUEST, $prefix.'_preview_address', '');
		if (empty($recipient) || !valid_email($recipient)) {
			trigger_localised_error('BML0013', E_USER_WARNING, $recipient);
			return FALSE;
		}

		if ($asset->sendPreviewMail($recipient)) {
			$msg_reps = Array(
							'assetid'		=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
							'email_addr'	=> $recipient,
						);

			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message = $ms->newMessage(Array(), 'bulkmail.preview', $msg_reps);

			$message->parameters['assetid'] = $asset->id;
			$message->parameters['version'] = substr($asset->version, 0, strrpos($asset->version, '.'));
			$message->send();

			return TRUE;
		} else {
			return FALSE;
		}

	}//end processPreviewAddress()


	/**
	* Paints the log of preview records
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintPreviewLog(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$logs = $ms->getMessages(0, 'bulkmail.preview', Array(), Array(), NULL, NULL, 'name', Array('assetid' => $asset->id));
		$logs = array_slice($logs, 0, 5);

		if (!empty($logs)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('date'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('subject'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('body'); ?></th>
				</tr>
			<?php
			foreach ($logs as $log_entry) {
				?>
					<tr>
						<td align="left" class="sq-backend-table-cell"><?php echo ts_iso8601($log_entry['sent']); ?></td>
						<td align="left" class="sq-backend-table-cell"><?php echo $log_entry['subject']; ?></td>
						<td align="left" class="sq-backend-table-cell"><?php echo $log_entry['body']; ?></td>
					</tr>
				<?php
			}
			?></table><?php
		} else {
			echo translate('bm_job_preview_never');
		}

		return $write_access;

	}//end paintPreviewLog()


	/**
	* Processes the log of preview records
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPreviewLog(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processPreviewLog()


	/**
	* Processes the log of preview records
	*
	* @param Bulkmail_Job		&$asset	the asset to which we belong
	* @param Backend_Outputter	&$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintContentBodyFilter(Bulkmail_Job $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$field_prefix = $prefix.'_content_body_filter';

		$enabled     = $asset->attr('content_body_filter');
		$match       = $asset->attr('content_body_filter_match');
		$filter_text = $asset->attr('content_body_filter_text');

		if ($write_access) {
			// Enable/disable drop-down
			// Match (drop-down) \ disable when "disable" selected
			// Text  (text)      /
			$contents = Array(
							1	=> translate('enabled'),
							0	=> translate('disabled'),
						);

			combo_box($field_prefix.'_enabled', $contents, FALSE, $enabled, FALSE);

			echo '<div id="'.$field_prefix.'_settings" style="padding: 1em 0em;">';
				$contents = Array(
								1	=> translate('matches'),
								0	=> translate('does_not_match'),
							);
				echo translate('bulkmail_job_filter_explanation').'<br/>';
				combo_box($field_prefix.'_match', $contents, FALSE, $match, FALSE);
				echo ' ';
				text_box($field_prefix.'_text', $filter_text, 30);
			echo '</div>';

		} else {
			if ($enabled) {
				if (!empty($filter_text)) {
					echo translate('enabled').'<br/>';
					echo translate('bulkmail_job_filter_explanation').'<br/>';
					if ($match) {
						echo translate('bulkmail_job_filter_enabled_format_true', $filter_text);
					} else {
						echo translate('bulkmail_job_filter_enabled_format_false', $filter_text);
					}
				} else {
					echo '<span class="sq-backend-warning">';
					echo translate('bulkmail_job_enabled_but_not_valid');
					echo '</span><br/>';
					echo translate('bulkmail_job_enabled_but_not_valid_explanation');
				}
			} else {
				echo translate('disabled');
			}
		}

		return TRUE;

	}//end paintContentBodyFilter()


	/**
	* Processes the log of preview records
	*
	* @param Bulkmail_Job		&$asset	the asset to which we belong
	* @param Backend_Outputter	&$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processContentBodyFilter(Bulkmail_Job $asset, Backend_Outputter $o, $prefix)
	{
		// If no access to attributes then rack off
		if (!$asset->writeAccess('attributes')) return FALSE;

		$field_prefix = $prefix.'_content_body_filter';
		$asset->setAttrValue('content_body_filter',       (bool)$_POST[$field_prefix.'_enabled']);
		$asset->setAttrValue('content_body_filter_match', (bool)$_POST[$field_prefix.'_match']);
		$asset->setAttrValue('content_body_filter_text',  $_POST[$field_prefix.'_text']);

		return $asset->saveAttributes();

	}//end processContentBodyFilter()


}//end class
?>