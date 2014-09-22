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
* $Id: bulkmail_job_edit_fns.inc,v 1.37 2012/08/30 00:57:13 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
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
* @version $Revision: 1.37 $
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
		$o->openField(translate('Validate Job'));

			?> <?php
				echo normal_button($prefix.'_validation_button', translate('Validate Job'), 'onClick= validate(); this.form.submit();');

				echo hidden_field($prefix.'_validation_hidden', '0');
				echo $o->note(translate('Press the Validate Job button to check for setting errors'));

			// javascript to update the value of hidden validation form element
			?>
			<script>
				function validate() {
					var prefix = '<?php echo $prefix; ?>';
					var id = prefix + '_validation_hidden';
					var element = document.getElementById(id);
					element.value = '1';
				}
			</script>
			<?php
		$o->closeField();

		// report errors if the the hidden field is set to '1'
		if (isset($_REQUEST[$prefix.'_validation_hidden']) && $_REQUEST[$prefix.'_validation_hidden'] == 1) {
			$post_office = $asset->getPostOffice();
			$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
			$details_info = $bm->generateJobDetails($asset, $post_office);
			$errors = $bm->isValidJob($details_info, TRUE);
			$o->openField(translate('Result'));

				if (empty($errors)) {
					echo translate('Job is valid.');

				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header"><?php echo translate('Configuration'); ?></th>

							<th class="sq-backend-table-header"><?php echo translate('Warning'); ?></th>

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
		?><span class="sq-backend-warning"><?php echo translate('This job has already been started. Configuration changes will not affect the current mailout unless the job is cancelled and restarted'); ?></span><?php

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
			if (isset($asset->_tmp['bulkmail.job.*'])){ //try using result from showProgressSection()
				$logs = $asset->_tmp['bulkmail.job.*'];
				unset($asset->_tmp['bulkmail.job.*']);
			} else {
				$logs = $ms->getMessages(0, 'bulkmail.job.*', Array(), Array(), NULL, NULL, 'name', Array('assetid' => $asset->id));
				$logs = array_slice($logs, 0, 5);
			}

			if (!empty($logs)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header"><?php echo translate('Date'); ?></th>

						<th class="sq-backend-table-header"><?php echo translate('Subject'); ?></th>

						<th class="sq-backend-table-header"><?php echo translate('Body'); ?></th>

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
		$asset->_tmp['bulkmail.job.*'] = $logs;

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
					<th class="sq-backend-table-header"><?php echo translate('Date'); ?></th>

					<th class="sq-backend-table-header"><?php echo translate('Subject'); ?></th>

					<th class="sq-backend-table-header"><?php echo translate('Body'); ?></th>

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
			echo translate('This email has never been previewed');

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
							1	=> translate('Enabled'),

							0	=> translate('Disabled'),

						);

			combo_box($field_prefix.'_enabled', $contents, FALSE, $enabled, FALSE);

			echo '<div id="'.$field_prefix.'_settings" style="padding: 1em 0em;">';
				$contents = Array(
								1	=> translate('Matches'),

								0	=> translate('Does Not Match'),

							);
				echo translate('Emails will be sent if the filter is enabled, and the body text of each bulkmail:').'<br/>';

				combo_box($field_prefix.'_match', $contents, FALSE, $match, FALSE);
				echo ' ';
				text_box($field_prefix.'_text', $filter_text, 30);
			echo '</div>';

		} else {
			if ($enabled) {
				if (!empty($filter_text)) {
					echo translate('Enabled').'<br/>';

					echo translate('Emails will be sent if the filter is enabled, and the body text of each bulkmail:').'<br/>';

					if ($match) {
						echo sprintf(translate('Matches: %s'), $filter_text);

					} else {
						echo sprintf(translate('Does Not Match: %s'), $filter_text);

					}
				} else {
					echo '<span class="sq-backend-warning">';
					echo translate('Enabled, but not valid');

					echo '</span><br/>';
					echo translate('The body filter is set to be enabled, however it cannot run without text to match against.');

				}
			} else {
				echo translate('Disabled');

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


	/**
	* Paints the user subscriptions
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintUserSubscriptions(&$asset, &$o, $prefix)
	{
		$write_access         = $asset->writeAccess('attributes');
		$subscription_enabled = $asset->attr('user_subscriptions_status');
		$subscriptions        = $asset->attr('subscriptions');

		$option_type = Array(
						'matches'		=> 'Matches',
						'contains'		=> 'Contains',
					   );

		$new_rule_type = Array(
							'0'			=> '-- Add New Rule --',
							'metadata'	=> 'Asset Metadata field',
						 );

		if ($subscription_enabled) {
			$o->openField(translate('Categories'));

				echo '<table class ="sq-backend-table" border="1" style="margin-bottom: 7px">';
				echo '<tr><td rowspan =2 width="12%">Name</td><td colspan="3">Rules</td></tr>';
				echo '<tr><td width="15%">Type</td><td>Details</td><td>Delete?</td></tr>';

				foreach ($subscriptions as $name => $subscription) {
					hidden_field($prefix.'_subscriptions['.$name.']', '');
					$tr = FALSE;
					echo "<tr>";
					$rowspan = ($write_access) ? (count($subscription) + 1) : count($subscription);
					if (empty($subscription)) $rowspan++;
					echo '<th rowspan='.$rowspan.'>'.$name.'</th>';
					if (!empty($subscription)) {
						foreach ($subscription as $index => $rules) {
							if ($tr) echo "<tr>";
							$type_data = array_keys($rules);
							echo '<td>'.$new_rule_type[$type_data[0]].'</td>';
							echo '<td>';
							if ($write_access) {
								asset_finder($prefix.'_subscriptions['.$name.']['.$index.']['.$type_data[0].'][assetid]', $rules[$type_data[0]]['assetid'], Array('metadata_field' => 'D'));
							} else {
								echo get_asset_tag_line($rules[$type_data[0]]['assetid']);
							}
							echo '&nbsp;&nbsp;&nbsp;';
							if ($write_access) {
								combo_box($prefix.'_subscriptions['.$name.']['.$index.']['.$type_data[0].'][rule_cond]', $option_type, FALSE, $rules[$type_data[0]]['rule_cond']);
							} else {
								echo $rules[$type_data[0]]['rule_cond'];
							}
							echo '&nbsp;&nbsp;&nbsp;';
							if ($write_access) {
								text_box($prefix.'_subscriptions['.$name.']['.$index.']['.$type_data[0].'][rule_text]', $rules[$type_data[0]]['rule_text'], 30);
							} else {
								echo $rules[$type_data[0]]['rule_text'];
							}

							echo '</td><td>';
							if ($write_access) check_box($prefix.'_subscriptions['.$name.']['.$index.'][delete]');
							echo "</td></tr>";

							$tr = TRUE;
						}//end foreach
					} else {
						echo '<td colspan=3></td></tr>';
					}
					if ($write_access) {
						?>
							<tr>
								<td colspan=4>
									<?php
										combo_box($prefix.'_subscriptions['.$name.'][new_rule]', $new_rule_type, FALSE);
									?>
								</td>
							</tr>
						<?php
					}
				}//end foreach
				echo "</table>";
				if ($write_access) {
					label(translate('New Category'), $prefix.'_new_category_name');

					echo '&nbsp;';
					text_box($prefix.'_new_category_name', '', 15);
				}
			$o->closeField();

		}//end if

		return $write_access;

	}//end paintUserSubscriptions()


	/**
	* Processes the User subscriptions
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUserSubscriptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$subscription_enabled = $asset->attr('user_subscriptions_status');

		if ($write_access) {
			$new_subscriptions = Array();
			if ($subscription_enabled) {
				if (isset($_POST[$prefix.'_subscriptions'])) {
					foreach ($_POST[$prefix.'_subscriptions'] as $sub_name => $subscription) {
						foreach ($subscription as $index => $subscription_data) {
							if ($index === 'new_rule') {
								if ($subscription['new_rule'] !== '0')  {
									$new_subscriptions[$sub_name][][$subscription['new_rule']] = Array(
																									'assetid'	=> 0,
																									'rule_cond'	=> 'contains',
																									'rule_text'	=> '',
																								);
								}
								continue;
							}

							if (!isset($subscription_data['delete'])) {
								$category = array_keys($subscription_data);
								$new_subscriptions[$sub_name][][$category[0]] = Array(
																					'assetid'	=> $subscription_data[$category[0]]['assetid']['assetid'],
																					'rule_cond'	=> array_get_index($subscription_data[$category[0]], 'rule_cond', 'contains'),
																					'rule_text'	=> array_get_index($subscription_data[$category[0]], 'rule_text',''),
																				);
							}
						}
					}
				}
				$new_category_name = array_get_index($_POST, $prefix.'_new_category_name', '');
				if (!empty($new_category_name)) {
					$new_subscriptions[$new_category_name] = Array();
				}

				$asset->setAttrValue('subscriptions',  $new_subscriptions);
				return $asset->saveAttributes();
			}
		}

		return $write_access;

	}//end processUserSubscriptions()


	/**
	* Paints the user Scheduling screen
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintScheduling(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$scheduling   = $asset->attr('scheduled_time');
		$interval_opt = Array(
							'empty_val'	=> 'None Selected',
							'hourly'	=> 'Hourly',
							'daily'		=> 'Daily',
							'weekly'	=> 'Weekly',
							'monthly'	=> 'Monthly',
							'yearly'	=> 'Yearly',
						);
		$interval     = array_get_index(array_keys($scheduling), 0, 'empty_val');
		$start_date   = array_get_index($scheduling, $interval, date('Y-m-d H:i').':00');
		?>
		<table>
		<tr>
			<td>
				<?php
					echo '<b>'.translate('Start Date').'</b>';

				?>
			</td>
			<td>
			<?php
				if ($write_access) {
						$parameters = Array(
										'min'			=> '2012-01-01 00:00:00',
										'max'			=> ts_iso8601(time() + (60 * 60 * 24 * 365 * 4)),
										'allow_circa'	=> '0',
										'show'			=> Array('y', 'm', 'd', 'h', 'i'),
										'null'			=> Array(),
										'style'			=> Array(
															'y'	=> 's',
															'm'	=> 's',
															'd'	=> 's',
															'h'	=> 's',
															'i'	=> 's',
														   ),
									  );

						$parameters['print_format'] = 'j M Y H:i';
						$from = new Datetime_Field($prefix.'_from', $start_date, $parameters);

						$from->printField();
				} else {
					if ($start_date == date('Y-m-d H:i').':00') {
						echo translate('Not Configured');

					} else {
						echo $start_date;
					}
				}
			?>
			</td>
		</tr>
		<tr>
			<td>
				<?php
					echo '<b>'.translate('Repeating').'</b>';

				?>
			</td>
			<td>
			<?php
				if ($write_access) {
					combo_box($prefix.'_repeating', $interval_opt, FALSE, $interval);
				} else {
					echo $interval_opt[$interval];
				}
			?>
			</td>
		</tr>
		</table>
		<?php

		return $write_access;

	}//end paintScheduling()


	/**
	* Processes the Scheduling screen
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processScheduling(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if ($write_access) {
			$interval   = array_get_index($_POST, $prefix.'_repeating', 'empty_val');
			$parameters = Array(
							'min'			=> '2012-01-01 00:00:00',
							'max'			=> ts_iso8601(time() + (60 * 60 * 24 * 365 * 4)),
							'allow_circa'	=> '0',
							'show'			=> Array('y', 'm', 'd', 'h', 'i'),
							'null'			=> Array(),
							'style'			=> Array(
												'y'	=> 's',
												'm'	=> 's',
												'd'	=> 's',
												'h'	=> 's',
												'i'	=> 's',
											   ),
						  );

			$parameters['print_format'] = 'j M Y H:i';
			$value = NULL;
			$from = new Datetime_Field($prefix.'_from', $value, $parameters);
			$from->processField();

			$scheduling = Array();
			if ($interval != 'empty_val') {
				$scheduling = Array(
								$interval => $from->value,
							  );
			}
			$asset->setAttrValue('scheduled_time', $scheduling);
			$asset->setAttrValue('last_scheduled_run', 0);
		}

		return $write_access;

	}//end processScheduling()


}//end class

?>
