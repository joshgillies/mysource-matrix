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
* $Id: bulkmail_job_edit_fns.inc,v 1.18 2005/11/17 01:46:09 rong Exp $
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
* @version $Revision: 1.18 $
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
			$bm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$details_info = $bm->generateJobDetails($asset, $post_office);
			$errors = $bm->isValidJob($details_info, true);
			$o->openField(translate('bm_job_validation_result'));
				if (empty($errors)) {
					echo translate('bm_job_validation_successful');
				} else {
					?><span class="sq-backend-warning"><?php
					foreach ($errors as $error) {
						echo $error;
						?><br /><?php
					}
					?></span><?php
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
		return true;

	}//end processValidation()


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
			check_box($prefix.'_use_post_office_header', true, $use_post_office_header);
		} else {
			?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $use_post_office_header ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
			<?php
		}
		$post_office =& $asset->getPostOffice();
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
		if (!$asset->writeAccess('attributes')) return false;

		// save whether to use post office header details
		$use_header_details = isset($_POST[$prefix.'_use_post_office_header']);
		$asset->setAttrValue('use_post_office_header', $use_header_details);

		return true;

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
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		if (!$write_access) return;

		$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return true;

	}//end processJobQueue()


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
		if (!$asset->writeAccess('attributes')) return false;

		$recipient = array_get_index($_REQUEST, $prefix.'_preview_address', '');
		if (empty($recipient)) return false;

		if ($asset->sendPreviewMail($recipient)) {
			$msg_reps = Array(
							'assetid'		=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
							'email_addr'	=> $recipient,
						);

			$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message = $ms->newMessage(Array(), 'bulkmail.preview', $msg_reps);

			$message->parameters['assetid'] = $asset->id;
			$message->parameters['version'] = substr($asset->version, 0, strrpos($asset->version, '.'));
			$message->send();

			return true;
		} else {
			return false;
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
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$logs = $ms->getMessages(0, 'bulkmail.preview', Array(), Array(), null, null, 'name', Array('assetid' => $asset->id), 5);

		if (!empty($logs)) {
			?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header">Date</th>
						<th class="sq-backend-table-header">Subject</th>
						<th class="sq-backend-table-header">Body</th>
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
		} else {
			echo 'This email has never been previewed';
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
		return true;

	}//end processPreviewLog()


//--        PROGRESS        --//


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
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$job = $bulkmail_manager->getQueuedJobs($asset->id);

		return (empty($job) ? false : true);

	}//end showProgressSection()


}//end class
?>