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
* $Id: bulkmail_post_office_edit_fns.inc,v 1.38 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Bulkmail_Post_Office_Edit_Fns
*
* Purpose
*
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.38 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


//--        DETAILS        --//


	/**
	* Paints the recipients which will receive the bulkmail
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintRecipients(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$recipients = $asset->attr('recipients');

		if ($write_access) {
			multiple_asset_finder($prefix.'_groups', $recipients);
		} else {
			?><ul><?php
				foreach ($recipients as $assetid) {
					$group_asset = NULL;
					if (!empty($assetid)) {
						$group_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					}

					if (!is_null($group_asset)) {
						?><li><?php echo get_asset_tag_line($group_asset->id); ?></li><?php
					} else {
						echo translate('Error retrieving group asset.');

					}
				}
			?></ul><?php
		}

		return $write_access;

	}//end paintRecipients()


	/**
	* Processes the value input from group addressee selection widget
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processRecipients(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$groups = Array();
		if (is_array($_POST[$prefix.'_groups'])) {
			foreach ($_POST[$prefix.'_groups'] as $group_node) {
				if (!empty($group_node['assetid'])) {
					$groups[] = $group_node['assetid'];
				}
			}
		}

		return $asset->setAttrValue('recipients', $groups);

	}//end processRecipients()


	/**
	* Paint the email server details
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintServerDetails(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$server_details = $asset->attr('server_details');

		$field_name = translate('Mail Method');

		$o->openField($field_name);
			$options = Array(
						'smtp'		=> 'SMTP',
						'sendmail'	=> 'Sendmail',
					   );

			$onChange = 'onChange="switchDriver(this.value)"';
			$value = array_get_index($server_details, 'driver', 'smtp');
			if ($write_access) {
				combo_box($prefix.'_server_det_driver', $options, FALSE, $value, 0, $onChange);
			} else {
				echo $options[$value];
			}
		$o->closeField();

		$smtp = !($value == 'smtp');

		$field_name = translate('Host');

		$name = $prefix.'_server_detail_smtp_host';
		$o->openField($field_name, '', '', $smtp, $name.'_field');
			$value = array_get_index($server_details, 'host', '');
			if ($write_access) {
				text_box($name, array_get_index($server_details, 'host', ''));
			} else {
				echo $value;
			}
		$o->closeField();

		$field_name = translate('Port');

		$name = $prefix.'_server_detail_smtp_port';
		$o->openField($field_name, '', '', $smtp, $name.'_field');
			$value = array_get_index($server_details, 'port', '');
			if ($write_access) {
				int_text_box($name, $value, FALSE, 4);
			} else {
				echo $value;
			}
		$o->closeField();

		$field_name = translate('Auth');

		$name = $prefix.'_server_detail_smtp_auth';
		$o->openField($field_name, '', 'Server requires authentication?', $smtp, $name.'_field');
			$auth = array_get_index($server_details, 'auth', FALSE);

			if ($write_access) {
				check_box($name, TRUE, $auth, 'switchAuthentication(this.checked)');
			} else {
				?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $auth ? 'tick' : 'cross'; ?>.png" width="15" height="15" />
				<?php
			}
		$o->closeField();

		$disabled = ($auth) ? '' : 'disabled=TRUE';

		$name = $prefix.'_server_detail_smtp_auth_username';
		$o->openField(translate('Username'), '', '', $smtp, $name.'_field');

			$value = array_get_index($server_details, 'username', '');
			if ($write_access) {
				text_box($name, $value, '', '', FALSE, $disabled);
			} else {
				echo $value;
			}
		$o->closeField();

		$name = $prefix.'_server_detail_smtp_auth_password';
		$o->openField(translate('Password'), '', '', $smtp, $name.'_field');

			$value = array_get_index($server_details, 'password', '');
			if ($write_access) {
				password_box($name, $value, '', '', $disabled);
			} else {
				echo str_repeat('*', strlen($value));
			}
		$o->closeField();

		$field_name = translate('Path');

		$name = $prefix.'_server_detail_sendmail_sendmail_path';
		$o->openField($field_name, '', 'The path to your sendmail binary. Use config file data/private/conf/tools.inc.', !$smtp, $name.'_field');
			echo SQ_TOOL_SENDMAIL_PATH;
		$o->closeField();

		$field_name = translate('Arguments');

		$name = $prefix.'_server_detail_sendmail_sendmail_args';
		$o->openField($field_name, '', 'Use config file data/private/conf/tools.inc.', !$smtp, $name.'_field');
			echo SQ_TOOL_SENDMAIL_ARGS;
		// $o->closeField(); This field is closed in the edit interface

		?>
			<script type="text/javascript">
				function switchDriver(driver) {
					var prefix = '<?php echo $prefix; ?>';
					var regex_string1 = prefix + '_server_detail.*_field';
					var regex_string2 = prefix + '_server_detail_' + driver + '.*_field';
					var re1 = new RegExp(regex_string1);
					var re2 = new RegExp(regex_string2);
					var elements = document.getElementsByTagName('tr');

					for (i = 0; i < elements.length; i++) {
						if (elements[i].id.match(re1)) {
							if (elements[i].id.match(re2)) {
								elements[i].style['display'] = '';
							} else {
								elements[i].style['display'] = 'none';
							}
						}
					}
				}

				function switchAuthentication(checked) {
					var prefix = '<?php echo $prefix; ?>';
					var regex_string1 = prefix + '_server_detail_smtp_auth_.*';
					var re1 = new RegExp(regex_string1);
					var elements = document.main_form.elements;

					for (i = 0; i < elements.length; i++) {
						if (elements[i].name.match(re1)) {
							elements[i].disabled = !checked;
						}
					}
				}
			</script>
		<?php

		return $write_access;

	}//end paintServerDetails()


	/**
	* Process the email server details
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processServerDetails(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$driver = array_get_index($_REQUEST, $prefix.'_server_det_driver', 'smtp');
		foreach ($_REQUEST as $key => $value) {
			if (preg_match('/'.$prefix.'_server_detail_'.$driver.'_(auth_)?([^^]*)/', $key, $matches)) {
				// do not store empty string so that we can use PEAR mail default params
				if ($value) $server_details[$matches[2]] = $value;
			}
		}
		$server_details['driver'] = $driver;

		if (array_get_index($server_details, 'driver', 'smtp') == 'smtp') {
			if (array_get_index($server_details, 'auth', FALSE) == FALSE) {
				$server_details['auth'] = FALSE;
				$server_details['username'] = '';
				$server_details['password'] = '';
			}
		}
		return $asset->setAttrValue('server_details', $server_details);

	}//end processServerDetails()


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
	function paintCurrentHeaders(&$asset, &$o, $prefix)
	{
		$width = 30;
		$write_access = $asset->writeAccess('attributes');

		$header_details = $asset->attr('header_details');
		if (!empty($header_details)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><b><?php echo translate('Field Name'); ?></b></th>

					<th class="sq-backend-table-header"><b><?php echo translate('Field Body'); ?></b></th>

					<?php
					if ($write_access) {
						?><th class="sq-backend-table-header"><b><?php echo translate('Delete?'); ?></b></th><?php

					}
					?>
				</tr>
				<?php
					$index = 0;
					foreach ($header_details as $field_name => $field_body) {
						if ($write_access) {
							?><tr>
								<td class="sq-backend-table-cell"><?php echo text_box($prefix.'_header_key_'.$index, $field_name);?></td>
								<td class="sq-backend-table-cell"><?php echo text_box($prefix.'_header_value_'.$index, $field_body);?></td>
								<td class="sq-backend-table-cell"><?php check_box($prefix.'_header_delete_'.$index, TRUE, FALSE);?></td>
							</tr><?php
						} else {
							?><tr>
								<td class="sq-backend-table-cell"><?php echo $field_name;?></td>
								<td class="sq-backend-table-cell" colspan="2"><?php echo htmlspecialchars($field_body);?></td>
							</tr><?php
						}
						$index++;
					}
				?>
			</table>
			<?php

		} else {
			echo translate('There are no additional headers set.');

		}

		return $write_access;

	}//end paintCurrentHeaders()


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
	function processCurrentHeaders(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$header_details = $asset->attr('header_details');

		$index = 0;
		$new_header_details = Array();
		foreach ($header_details as $field_name => $field_body) {
			$key = trim(array_get_index($_REQUEST, $prefix.'_header_key_'.$index));
			$value = trim(array_get_index($_REQUEST, $prefix.'_header_value_'.$index));
			$delete = array_get_index($_REQUEST, $prefix.'_header_delete_'.$index, FALSE);

			if (!empty($key) && !$delete) {
				$new_header_details[$key] = $value;
			}
			$index++;
		}

		$asset->setAttrValue('header_details', $new_header_details);
		return TRUE;

	}//end processCurrentHeaders()


	/**
	* Paints the interface to add a new header field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNewHeader(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		// text box for new field entry
		if ($write_access) {
			echo translate('Key'), ': ';

			text_box($prefix.'_header_new_key', '', 30);
			echo '&nbsp;', translate('Value'), ': ';

			text_box($prefix.'_header_new_value', '', 30);
		}

		$write_access;

	}//end paintNewHeader()


	/**
	* Process the interface to add a new header field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewHeader(&$asset, &$o, $prefix)
	{
		$header_key = array_get_index($_REQUEST, $prefix.'_header_new_key', '');
		$header_value = array_get_index($_REQUEST, $prefix.'_header_new_value', '');

		$header_details = $asset->attr('header_details');

		if ( (!empty($header_key)) && (!empty($header_value)) ) {
			$header_details[$header_key] = $header_value;
		}

		$asset->setAttrValue('header_details', $header_details);

		return TRUE;

	}//end processNewHeader()


//--        THRESHOLD        --//


	/**
	* Paint the threshold settings
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintThresholds(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		// check if bulkmail manager's default's post office overrides threshold settings
		$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$use_bm_threshold = $bm->attr('use_bm_threshold');
		if ($use_bm_threshold && !($asset instanceof Bulkmail_Manager)) {
			$threshold = $bm->attr('threshold');
			$write_access = FALSE;
		} else {
			$threshold = $asset->attr('threshold');
		}

		// paint existing rules
		if (!empty($threshold)) {
			$o->openField(translate('Current Rules'));

			?><table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('Type') ?></th>

					<th class="sq-backend-table-header"><?php echo translate('Allow Multiple ?') ?></th>

					<th class="sq-backend-table-header"><?php echo translate('Rule') ?></th>

					<?php
						if ($write_access) {
							?><th class="sq-backend-table-header"><?php echo translate('Delete Rule') ?></th><?php

						}
					?>
				</tr><?php
				foreach ($threshold as $key => $rule) {
					?><tr>
						<td class="sq-backend-table-cell"><?php echo $rule['type']; ?></td>
						<td class="sq-backend-table-cell"><?php echo ($rule['multiple']) ? translate('Yes') : translate('No'); ?></td>


					<?php
						switch ($rule['type']) {
							case translate('Excluded Time') :

								$this->_paintExcludedTimeRule($asset, $o, $prefix.'_threshold_rule_'.$key, $rule, $write_access);
							break;
							case translate('Send Rate') :

								$this->_paintSendRateRule($asset, $o, $prefix.'_threshold_rule_'.$key, $rule, $write_access);
							break;
							default:
							break;
						}
					?></tr><?php
				}
			?></table><?php
				if ($use_bm_threshold && !($asset instanceof Bulkmail_Manager)) {
					$o->note(sprintf(translate('Using Default Post Office\'s threshold settings in %s'), get_asset_tag_line($bm->id)));

				}
			$o->closeField();
		}//end if

		// the dropdown box that allows user to add new rules
		if (!($use_bm_threshold && !($asset instanceof Bulkmail_Manager))) {
			$this->paintAddRule($asset, $o, $prefix, $write_access);
		}

		return TRUE;

	}//end paintThresholds()


	/**
	* Process the threshold settings
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processThresholds(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		// check if bulkmail manager's default's post office overrides threshold settings
		$bm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$use_bm_threshold = $bm->attr('use_bm_threshold');
		if ($use_bm_threshold && !($asset instanceof Bulkmail_Manager)) {
			$write_access = FALSE;
		}
		if (!$write_access) return FALSE;

		$threshold = $asset->attr('threshold');
		if (!empty($threshold)) {
			foreach ($threshold as $key => $rule) {
				switch ($rule['type']) {
					case translate('Excluded Time') :

						$threshold = $this->_processExcludedTimeRule($prefix.'_threshold_rule_'.$key, $threshold, $key);
					break;
					case translate('Send Rate') :

						$threshold = $this->_processSendRateRule($prefix.'_threshold_rule_'.$key, $threshold, $key);
					break;
					default:
					break;
				}
			}
		}
		$threshold = $this->processAddRule($asset, $o, $prefix, $threshold, $write_access);
		$asset->setAttrValue('threshold', $threshold);
		return TRUE;

	}//end processThresholds()


	/**
	* Paints the dropdown to add a rule to this section
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param boolean	$write_access	write access to the asset attribute
	*
	* @return void
	* @access public
	*/
	function paintAddRule(&$asset, &$o, $prefix, $write_access)
	{
		$o->openField(translate('Add Rule'), '', translate('Please select one of the available threshold rules, and fill in appropriate value(s)'));


		if ($write_access) {
			$rules = Array('' => '-- '.translate('Choose Type').' --');

			$rules += Array(
						translate('Excluded Time'),

						translate('Send Rate'),

					  );
			combo_box($prefix.'_rule_type', $rules, FALSE, '');
		}
		$o->closeField();

	}//end paintAddRule()


	/**
	* Processes the dropdown to add a rule to this section
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param array		$threshold		the post office's threshold attribute
	* @param boolean	$write_access	write access to the asset attribute
	*
	* @return array
	* @access public
	*/
	function processAddRule(&$asset, &$o, $prefix, $threshold, $write_access)
	{
		if ($write_access) {
			$rules = Array(
						translate('Excluded Time'),

						translate('Send Rate'),

					 );
			if (isset($_POST[$prefix.'_rule_type'])) {
				$type = (isset($rules[$_POST[$prefix.'_rule_type']])) ? $rules[$_POST[$prefix.'_rule_type']] : '';
			}
			// multiple entries not allowed for this rule type
			foreach ($threshold as $key => $val) {
				if ($val['type'] == $type) {
					if (!$val['multiple']) return $threshold;
				}
			}
			$threshold_types = $asset->attr('threshold_types');
			switch ($type) {
					case translate('Excluded Time') :

						$threshold_types[0]['type'] = $type;
						$threshold[] = $threshold_types[0];
					break;
					case translate('Send Rate') :

						$threshold_types[1]['type'] = $type;
						$threshold[] = $threshold_types[1];
					break;
					default:
					break;
			}
		}

		return $threshold;

	}//end processAddRule()


	/**
	* Paints the 'excluded time' rule
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param array		$rule			the rule that we are painting
	* @param boolean	$write_access	write access to the asset attribute
	*
	* @return void
	* @access private
	*/
	function _paintExcludedTimeRule(&$asset, &$o, $prefix, $rule, $write_access)
	{
		$hours_array = Array(
						0	=> '00',
						1	=> '01',
						2	=> '02',
						3	=> '03',
						4	=> '04',
						5	=> '05',
						6	=> '06',
						7	=> '07',
						8	=> '08',
						9	=> '09',
						10	=> '10',
						11	=> '11',
						12	=> '12',
						13	=> '13',
						14	=> '14',
						15	=> '15',
						16	=> '16',
						17	=> '17',
						18	=> '18',
						19	=> '19',
						20	=> '20',
						21	=> '21',
						22	=> '22',
						23	=> '23',
					   );
		$days_array = Array(
						0	=> translate('Sunday'),

						1	=> translate('Monday'),

						2	=> translate('Tuesday'),

						3	=> translate('Wednesday'),

						4	=> translate('Thursday'),

						5	=> translate('Friday'),

						6	=> translate('Saturday'),

					  );

		if ($write_access) {
			ob_start();
				combo_box($prefix.'_from', $hours_array, FALSE, $rule['params']['from']);
				$from = ob_get_contents();
			ob_end_clean();
			ob_start();
				combo_box($prefix.'_to', $hours_array, FALSE, $rule['params']['to']);
				$to = ob_get_contents();
			ob_end_clean();
			ob_start();
				combo_box($prefix.'_days', $days_array, TRUE, $rule['params']['days']);
				$days = ob_get_contents();
			ob_end_clean();
			?><td class="sq-backend-table-cell"><?php
				echo sprintf(translate('Do not process jobs between %1$s:00 and %2$s:00'), $from, $to);

				echo sprintf(translate('on %s'), $days);

			?></td><td class="sq-backend-table-cell"><?php
				check_box($prefix.'_delete', TRUE, FALSE);?></td><?php
		} else {
			$from = $hours_array[$rule['params']['from']];
			$to = $hours_array[$rule['params']['to']];
			if (!empty($rule['params']['days'])) {
				$selected_days = Array();
				foreach ($rule['params']['days'] as $day) {
					$selected_days[] = $days_array[$day];
				}
				$days = implode(', ', $selected_days);
			}
			?><td class="sq-backend-table-cell" colspan="2"><?php
				echo sprintf(translate('Do not process jobs between %1$s:00 and %2$s:00'), $from, $to);

				if (!empty($days)) {
					if (count($selected_days) == 7) {
						echo translate('on every day');

					} else {
						echo sprintf(translate('on %s'), $days);

					}
				}
			?></td><?php
		}

	}//end _paintExcludedTimeRule()


	/**
	* Processes the 'excluded time' rule
	*
	* @param string	$prefix		prefix for the form elements
	* @param array	$threshold	the post office's threshold attribute
	* @param int	$key		the index of this rule in the threshold array
	*
	* @return array
	* @access private
	*/
	function _processExcludedTimeRule($prefix, $threshold, $key)
	{
		// make changes to threshold attribute accordingly and pass it back to the process function
		$remove = FALSE;
		if (isset($_POST[$prefix.'_from'])) {
			$params['from'] = $_POST[$prefix.'_from'];
			$params['to']   = $_POST[$prefix.'_to'];
			// check for valid range
			if ($_POST[$prefix.'_to'] == $params['from']) {
				trigger_localised_error('BML0014', translate('Exclude FROM and TO times cannot be equal.'), E_USER_NOTICE);

				if ($threshold[$key]['params']['to'] == $_POST[$prefix.'_to'] && $threshold[$key]['params']['from'] == $_POST[$prefix.'_from']) {
					$remove = TRUE;
				} else {
					$params['from'] = $threshold[$key]['params']['from'];
					$params['to']   = $threshold[$key]['params']['to'];
				}
			}

			if (!isset($_POST[$prefix.'_days']) && !isset($threshold[$key]['params']['days'])) {
				trigger_localised_error('BML0015', translate('No days were selected.'), E_USER_NOTICE);
				$remove = TRUE;
			} else if (!isset($_POST[$prefix.'_days']) && isset($threshold[$key]['params']['days'])) {
				trigger_localised_error('BML0015', translate('No days were selected.'), E_USER_NOTICE);
				$params['days'] = $threshold[$key]['params']['days'];
			} else {
				$params['days'] = (isset($_POST[$prefix.'_days'])) ? $_POST[$prefix.'_days'] : Array();
			}
		}
		$threshold[$key]['params'] = $params;
		// delete box is checked
		if (isset($_POST[$prefix.'_delete']) || $remove) {
			unset($threshold[$key]);
		}

		return $threshold;

	}//end _processExcludedTimeRule()


	/**
	* Paints the 'Send Rate' rule
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param array		$rule			the rule that we are painting
	* @param boolean	$write_access	write access to the asset attribute
	*
	* @return void
	* @access private
	*/
	function _paintSendRateRule(&$asset, &$o, $prefix, $rule, $write_access)
	{
		$time_unit = Array(
						60		=> translate('minute'),

						3600	=> translate('hour'),

						86400	=> translate('day'),

					 );
		// make sure the send rate is within this range
		$min_send_rate = 1;
		$max_send_rate = 600;
		$send_rate = ($rule['params']['value']/$rule['params']['unit']) * 60;
		if ($write_access) {
			ob_start();
				int_text_box($prefix.'_value', $rule['params']['value'], FALSE);
				$value = ob_get_contents();
			ob_end_clean();
			ob_start();
				combo_box($prefix.'_unit', $time_unit, FALSE, $rule['params']['unit']);
				$unit = ob_get_contents();
			ob_end_clean();
			?><td class="sq-backend-table-cell"><?php
				echo sprintf(translate('Send %1$s emails every %2$s'), $value, $unit);

				if ($send_rate < $min_send_rate) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('Warning: Minimum send rate is %1$s emails every minute'), $min_send_rate); ?></span><?php

				} else if ($send_rate > $max_send_rate) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('Warning: Maximum send rate is %1$s emails every minute'), $max_send_rate); ?></span><?php

				}
			?></td><td class="sq-backend-table-cell"><?php
				check_box($prefix.'_delete', TRUE, FALSE);?></td><?php
		} else {
			$value = $rule['params']['value'];
			$unit = $time_unit[$rule['params']['unit']];
			?><td class="sq-backend-table-cell" colspan="2"><?php
				echo sprintf(translate('Send %1$s emails every %2$s'), $value, $unit);

				if ($send_rate < $min_send_rate) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('Warning: Minimum send rate is %1$s emails every minute'), $min_send_rate); ?></span><?php

				} else if ($send_rate > $max_send_rate) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('Warning: Maximum send rate is %1$s emails every minute'), $max_send_rate); ?></span><?php

				}
			?></td><?php
		}

	}//end _paintSendRateRule()


	/**
	* Processes the 'Send Rate' rule
	*
	* @param string	$prefix		prefix for the form elements
	* @param array	$threshold	the post office's threshold attribute
	* @param int	$key		the index of this rule in the threshold array
	*
	* @return array
	* @access private
	*/
	function _processSendRateRule($prefix, $threshold, $key)
	{
		if (isset($_POST[$prefix.'_value'])) {
			$params['value'] = $_POST[$prefix.'_value'];
			$params['unit'] = $_POST[$prefix.'_unit'];
			// do not allow zero rate
			if (!$params['value']) $params['value'] = 1;
		}
		$threshold[$key]['params'] = $params;
		if (isset($_POST[$prefix.'_delete'])) {
			unset($threshold[$key]);
		}

		return $threshold;

	}//end _processSendRateRule()


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
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		$o->openField('');
			$jobs = $bulkmail_manager->getQueuedJobs(NULL, $asset->id);
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
		$bulkmail_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		if (!$write_access) return;

		$jobs = $bulkmail_manager->getQueuedJobs(NULL, $asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return TRUE;

	}//end processJobQueue()


	/**
	* Function that determine whether to hide or show the progress section for this post office
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
		$jobs = $bulkmail_manager->getQueuedJobs(NULL, $asset->id);

		return (empty($jobs) ? FALSE : TRUE);

	}//end showProgressSection()


}//end class


?>
