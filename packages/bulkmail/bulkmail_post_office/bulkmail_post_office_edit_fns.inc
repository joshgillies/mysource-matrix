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
* $Id: bulkmail_post_office_edit_fns.inc,v 1.17 2005/11/14 23:41:11 ndvries Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Bulkmail_Post_Office_Edit_Fns
*
* Purpose
*
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Post_Office_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


//--        DETAILS        --//


	/**
	* Paints the recipients which will recieve the bulkmail
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
					$group_asset = null;
					if (!empty($assetid)) {
						$group_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					}

					if (!is_null($group_asset)) {
						?><li><?php echo get_asset_tag_line($group_asset->id); ?></li><?php
					} else {
						echo translate('bulkmail_recipient_group_asset_not_found');
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
		if (!$asset->writeAccess('attributes')) return false;

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

		$field_name = translate('bulkmail_server_detail_mail_method_field_name');
		$o->openField($field_name);
			$options = Array(
						'smtp'		=> 'SMTP',
						'sendmail'	=> 'Sendmail',
					   );

			$onChange = 'onChange="switchDriver(this.value)"';
			$value = array_get_index($server_details, 'driver', 'smtp');
			if ($write_access) {
				combo_box($prefix.'_server_det_driver', $options, false, $value, 0, $onChange);
			} else {
				echo $options[$value];
			}
		$o->closeField();

		$smtp = !($value == 'smtp');

		$field_name = translate('bulkmail_server_detail_host_field_name');
		$name = $prefix.'_server_detail_smtp_host';
		$o->openField($field_name, '', '', $smtp, $name.'_field');
			$value = array_get_index($server_details, 'host', '');
			if ($write_access) {
				text_box($name, array_get_index($server_details, 'host', ''));
			} else {
				echo $value;
			}
		$o->closeField();

		$field_name = translate('bulkmail_server_detail_port_field_name');
		$name = $prefix.'_server_detail_smtp_port';
		$o->openField($field_name, '', '', $smtp, $name.'_field');
			$value = array_get_index($server_details, 'port', '');
			if ($write_access) {
				int_text_box($name, $value, false, 4);
			} else {
				echo $value;
			}
		$o->closeField();

		$field_name = translate('bulkmail_server_detail_auth_field_name');
		$name = $prefix.'_server_detail_smtp_auth';
		$o->openField($field_name, '', 'Server requires authentication?', $smtp, $name.'_field');
			$auth = array_get_index($server_details, 'auth', false);

			if ($write_access) {
				check_box($name, true, $auth, 'switchAuthentication(this.checked)');
			} else {
				?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $auth ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
				<?php
			}
		$o->closeField();

		$disabled = ($auth) ? '' : 'disabled=true';

		$name = $prefix.'_server_detail_smtp_auth_username';
		$o->openField(translate('username'), '', '', $smtp, $name.'_field');
			$value = array_get_index($server_details, 'username', '');
			if ($write_access) {
				//text_box($name, $value, '', '', false, 'disabled='.$disabled);
				text_box($name, $value, '', '', false, $disabled);
			} else {
				echo $value;
			}
		$o->closeField();

		$name = $prefix.'_server_detail_smtp_auth_password';
		$o->openField(translate('password'), '', '', $smtp, $name.'_field');
			$value = array_get_index($server_details, 'password', '');
			if ($write_access) {
				password_box($name, $value, '', '', $disabled);
			} else {
				echo str_repeat('*', strlen($value));
			}
		$o->closeField();

		$field_name = translate('bulkmail_server_detail_path_field_name');
		$name = $prefix.'_server_detail_sendmail_sendmail_path';
		$o->openField($field_name, '', 'The path to your sendmail binary. Eg. /usr/sbin/sendmail', !$smtp, $name.'_field');
			$value = array_get_index($server_details, 'sendmail_path', '');
			if ($write_access) {
				text_box($name, $value);
			} else {
				echo $value;
			}
		$o->closeField();

		$field_name = translate('bulkmail_server_detail_arg_field_name');
		$name = $prefix.'_server_detail_sendmail_sendmail_args';
		$o->openField($field_name, '', 'Eg. -t will run sendmail as \'sendmail -t\'', !$smtp, $name.'_field');
			$value = array_get_index($server_details, 'sendmail_args', '');
			if ($write_access) {
				text_box($name, $value);
			} else {
				echo $value;
			}
		//$o->closeField(); This field is closed in the edit interface

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
		if (!$asset->writeAccess('attributes')) return false;

		if (isset($_REQUEST[$prefix.'_server_detail_mph'])) {
			$server_details['mph'] = $_REQUEST[$prefix.'_server_detail_mph'];
		}

		$driver = array_get_index($_REQUEST, $prefix.'_server_det_driver', 'smtp');
		foreach ($_REQUEST as $key => $value) {
			if (preg_match('/'.$prefix.'_server_detail_'.$driver.'_(auth_)?([^^]*)/', $key, $matches)) {
				$server_details[$matches[2]] = $value;
			}
		}
		$server_details['driver'] = $driver;


		if (array_get_index($server_details, 'driver', 'smtp') == 'smtp') {
			if (array_get_index($server_details, 'auth', false) == false) {
				$server_details['auth'] = false;
				$server_details['username'] = '';
				$server_details['password'] = '';
			}
		}
		return $asset->setAttrValue('server_details', $server_details);

	}//end processServerDetails()


	/**
	* Paints the header details interface for the bmail
	*
	* This paint function is used by bulkmail job too
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
		$width = 30;
		$write_access = $asset->writeAccess('attributes');

		// paint extra header fields
		$header_details = $asset->attr('header_details');
		//$o->openField(translate('bulkmail_header_detail_extra_fields'));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><b><?php echo translate('bulkmail_header_detail_field_name'); ?></b></td>
					<td class="sq-backend-table-header"><b><?php echo translate('bulkmail_header_detail_field_body'); ?></b></td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header"><b><?php echo translate('delete_question'); ?></b></td><?php
					}
					?>
				</tr>
				<?php
					$index = 0;
					foreach ($header_details as $field_name => $field_body) {
						if ($write_access) {
							// field_name[i] is not a valid xhtml id for form fields
							?><tr>
								<td class="sq-backend-table-cell"><?php echo text_box($prefix.'_header_detail_fields_name_'.$index, $field_name);?></td>
								<td class="sq-backend-table-cell"><?php echo text_box($prefix.'_header_detail_fields_body_'.$index, $field_body);?></td>
								<td class="sq-backend-table-cell"><?php check_box($prefix.'_header_detail_fields_delete_'.$index, true, false);?></td>
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
		$o->closeField();

		// text box for new field entry
		if ($write_access) {
			$o->openField(translate('bulkmail_header_new_field_name'));
				text_box($prefix.'_header_detail_field_new_name', '', $width);
			$o->closeField();
			$o->openField(translate('bulkmail_header_new_field_body'));
				text_box($prefix.'_header_detail_field_new_body', '', $width);
			//$o->closeField();
		}

		return $write_access;

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
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		// update and delete fields
		$header_details = Array();
		$index = 0;
		// changed naming convention from name[i] to name_i to comply with xhtml standards
		while (isset($_POST[$prefix.'_header_detail_fields_name_'.$index])) {
			if (!isset($_POST[$prefix.'_header_detail_fields_delete_'.$index])) {
				$name = $_POST[$prefix.'_header_detail_fields_name_'.$index];
				$body = $_POST[$prefix.'_header_detail_fields_body_'.$index];
				$header_details[$name] = $body;
			}
			$index++;
		}

		// create a new field or overwrite an existing field
		$field_status = (isset($_POST[$prefix.'_header_detail_field_new_name']) && trim($_POST[$prefix.'_header_detail_field_new_name']) != '');
		$body_status  = (isset($_POST[$prefix.'_header_detail_field_new_body']) && trim($_POST[$prefix.'_header_detail_field_new_body']) != '');
		if ($field_status && $body_status) {
			$new_field_name = trim($_POST[$prefix.'_header_detail_field_new_name']);
			$new_field_body = trim($_POST[$prefix.'_header_detail_field_new_body']);
			$header_details[$new_field_name] = $new_field_body;
		}

		$asset->setAttrValue('header_details', $header_details);
		return true;

	}//end processHeaderDetails()


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
		$threshold = $asset->attr('threshold');
		if (empty($threshold)) {
			$threshold = Array(
							'type'	=> 'chunk',
							'value'	=> '0',
						 );
		}
		$chunk_value = ($threshold['type'] == 'chunk') ? $threshold['value'] : 0;
		$spread_value = ($threshold['type'] == 'spread') ? $threshold['value'] : 0;

		ob_start();
			int_text_box($prefix.'_threshold_chunk', $chunk_value, false, 3);
			$chunk_box = ob_get_contents();
		ob_end_clean();
		ob_start();
			int_text_box($prefix.'_threshold_spread', $spread_value, false, 3);
			$spread_box = ob_get_contents();
		ob_end_clean();

		$threshold_types = Array(
							'chunk'		=> translate('bulkmail_threshold_detail_chunk', $chunk_box),
							'spread'	=> translate('bulkmail_threshold_detail_spread', $spread_box),
							'asap'		=> translate('bulkmail_threshold_detail_asap'),
						   );

		$o->openField(translate('bulkmail_threshold_detail_type'));
			if ($write_access) {
				$o->note(translate('bulkmail_threshold_detail_note'));
				foreach ($threshold_types as $threshold_type => $threshold_desc) {
					if (isset($threshold['type']) && $threshold_type == $threshold['type']) {
						$checked = true;
						$value = $threshold['value'];
					} else {
						$checked = false;
						$value = '';
					}
					radio_button($prefix.'_threshold_type', $threshold_type, $checked);
					echo $threshold_desc;
					?><br /><?php
				}
			} else {
				$threshold_desc = translate('bulkmail_threshold_detail_'.$threshold['type'], $threshold['value']);
				echo translate('bulkmail_threshold_detail_report', $threshold_desc);
			}
		$o->closeField();
		return true;

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
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		if (isset($_REQUEST[$prefix.'_threshold_type'])) {
			$type = $_REQUEST[$prefix.'_threshold_type'];
			$value = ($type == 'asap') ? 0 : $_REQUEST[$prefix.'_threshold_'.$type];
			$threshold = Array(
							'type'	=> $type,
							'value'	=> $value,
						 );
			$asset->setAttrValue('threshold', $threshold);
		}
		return true;

	}//end processThresholds()


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
			$jobs = $bulkmail_manager->getQueuedJobs(null, $asset->id);
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

		$jobs = $bulkmail_manager->getQueuedJobs(null, $asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return true;

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
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$jobs = $bulkmail_manager->getQueuedJobs(null, $asset->id);

		return (empty($jobs) ? false : true);

	}//end showProgressSection()


}//end class
?>
