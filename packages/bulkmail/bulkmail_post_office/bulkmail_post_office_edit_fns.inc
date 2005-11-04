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
* $Id: bulkmail_post_office_edit_fns.inc,v 1.10 2005/11/04 02:50:23 rong Exp $
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
* @version $Revision: 1.10 $
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
						?><li><?php echo $group_asset->name, '(Id: #', $group_asset->id, ')'; ?></li><?php
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
	* Paint the content generation method, either user or group based
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintContentDetails(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$content_details = $asset->attr('content_details');

		// content page
		$page_id = array_get_index($content_details, 'content_page', 0);
		if ($page_id) {
			$page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($page_id);
		}
		$name = translate('bulkmail_content_detail_page_name');
		$note = translate('bulkmail_content_detail_page_note');
		$o->openField($name, '', $note);
			if ($write_access) {
				asset_finder($prefix.'_content_detail_page', $page_id, Array('page' => 'D'));
			} else {
				if (!$page_id) {
					echo translate('no_asset_selected');
				} else {
					if (is_null($page)) {
						echo translate('invalid_assetid');
					} else {
						echo $page->name.' (#'.$page->id.')';
					}
				}
			}
		$o->closeField();

		// generation method
		$generation_method = array_get_index($content_details, 'type', 'each_user');
		$options = Array(
					'each_user'			=> translate('bulkmail_content_detail_generation_method_use_each_user'),
					'pre_selected_user'	=> translate('bulkmail_content_detail_generation_method_preselected_user'),
				   );
		$name = translate('bulkmail_content_detail_method_name');
		$note = translate('bulkmail_content_detail_method_note');
		$o->openField($name, '', $note);
			if ($write_access) {
				combo_box($prefix.'_content_det_type', $options, false, $generation_method);
			} else {
				echo $options[$generation_method];
			}
		$o->closeField();

		// pre-selected user
		$user_id = array_get_index($content_details, 'pre_selected_user', '0');
		if ($user_id) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($user_id);
		}
		$name = translate('bulkmail_content_detail_generate_as_field_name');
		$note = translate('bulkmail_content_detail_generate_as_field_note');
		$o->openField($name, '', $note);
			if ($write_access) {
				asset_finder($prefix.'_content_detail_pre_selected_user', $user_id, Array('user' => 'D'));
			} else {
				if (!$user_id) {
					echo translate('no_asset_selected');
				} else {
					if (is_null($user)) {
						echo translate('invalid_assetid');
					} else {
						echo $user->name.' (#'.$user->id.')';
					}
				}
			}
		$o->closeField();

		// design
		$design_id = array_get_index($content_details, 'design', '0');
		if ($design_id) {
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($design_id);
		}
		$name = translate('bulkmail_content_detail_design_field_name');
		$note = translate('bulkmail_content_detail_design_field_note');
		$o->openField($name, '', $note);
			if ($write_access) {
				asset_finder($prefix.'_content_detail_design', $design_id, Array('design' => 'I'));
			} else {
				if (!$design_id) {
					echo translate('no_asset_selected');
				} else {
					if (is_null($design)) {
						echo translate('invalid_assetid');
					} else {
						echo $design->name.' (#'.$design->id.')';
					}
				}
			}
		$o->closeField();

		// layout
		$layout_id = array_get_index($content_details, 'layout', '0');
		if ($layout_id) {
			$layout = &$GLOBALS['SQ_SYSTEM']->am->getAsset($layout_id);
		}
		$name = translate('bulkmail_content_detail_layout_field_name');
		$note = translate('bulkmail_content_detail_layout_field_note');
		$o->openField($name, '', $note);
			if ($write_access) {
				asset_finder($prefix.'_content_detail_layout', $layout_id, Array('layout' => 'I'));
			} else {
				if (!$layout_id) {
					echo translate('no_asset_selected');
				} else {
					if (is_null($layout)) {
						echo translate('invalid_assetid');
					} else {
						echo $layout->name.' (#'.$layout->id.')';
					}
				}
			}
		$o->closeField();

		return $write_access;

	}//end paintContentDetails()


	/**
	* Process the fields related to the content generation method
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processContentDetails(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;
		// once live, content details are not changeable
		if ($asset->status == SQ_STATUS_LIVE) return false;

		$content_details = $asset->attr('content_details');
		// save content page id
		if (isset($_POST[$prefix.'_content_detail_page'])) {
			$content_details['content_page'] = $_POST[$prefix.'_content_detail_page']['assetid'];
		}
		// save content generation type (each user/pre-selected user)
		if (isset($_POST[$prefix.'_content_det_type'])) {
			$content_details['type'] = $_POST[$prefix.'_content_det_type'];
		}
		// save bulkmail user for group-based content generation
		if (isset($_POST[$prefix.'_content_detail_pre_selected_user'])) {
			$content_details['pre_selected_user'] = $_POST[$prefix.'_content_detail_pre_selected_user']['assetid'];
		}
		// design
		if (isset($_POST[$prefix.'_content_detail_design'])) {
			$content_details['design'] = $_POST[$prefix.'_content_detail_design']['assetid'];
		}
		// layout
		if (isset($_POST[$prefix.'_content_detail_layout'])) {
			$content_details['layout'] = $_POST[$prefix.'_content_detail_layout']['assetid'];
		}

		$asset->setAttrValue('content_details', $content_details);
		return true;

	}//end processContentDetails()


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
	* @param object		&$asset	the asset to which we belong
	* @param object		&$o		the outputter class
	* @param string		$prefix	prefix for the form element
	* @param boolean	$hide	hide the header details?
	*
	* @return boolean
	* @access public
	*/
	function paintHeaderDetails(&$asset, &$o, $prefix, $hide=false)
	{
		$width = 60;
		$write_access = $asset->writeAccess('attributes');
		$header_details = $asset->attr('header_details');

		// paint existing header fields
		$o->openField(translate('fields'), '', '', $hide, $prefix.'_header_detail');
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><b><?php echo translate('bulkmail_header_detail_field_name'); ?></b></td>
					<td align="center" class="sq-backend-table-header"><b><?php echo translate('bulkmail_header_detail_field_body'); ?></b></td>
					<?php
					if ($write_access) {
						?><td align="center" width="70" class="sq-backend-table-header"><b><?php echo translate('delete_question'); ?></b></td><?php
					}
					?>
				</tr>
				<?php
					$index = 0;
					foreach ($header_details as $field_name => $field_body) {
						if ($write_access) {
							// field_name[i] is not a valid xhtml id for form fields
							?><tr>
								<td align="left" class="sq-backend-table-cell"><?php echo text_box($prefix.'_header_detail_fields_name_'.$index, $field_name);?></td>
								<td align="left" class="sq-backend-table-cell"><?php echo text_box($prefix.'_header_detail_fields_body_'.$index, $field_body, $width);?></td>
								<td align="center" class="sq-backend-table-cell"><?php check_box($prefix.'_header_detail_fields_delete_'.$index, true, false);?></td>
							</tr><?php
						} else {
							?><tr>
								<td align="left" class="sq-backend-table-cell"><?php echo $field_name;?></td>
								<td align="left" class="sq-backend-table-cell" colspan="2"><?php echo htmlspecialchars($field_body);?></td>
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
			$o->openField(translate('bulkmail_header_new_field_name'), '', '', $hide, $prefix.'_header_detail');
				text_box($prefix.'_header_detail_field_new_name', '');
			$o->closeField();
			$o->openField(translate('bulkmail_header_new_field_body'), '', '', $hide, $prefix.'_header_detail');
				text_box($prefix.'_header_detail_field_new_body', '', $width);
			$o->closeField();
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


//--        PREVIEW        --//


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

		$current_user = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_user);
		$email = $user->attr('email');

		if (empty($email)) $email = 'none';

		$field_name = translate('bulkmail_preview_address');
		$field_note = translate('bulkmail_preview_address_note');
		$o->openField($field_name, '', $field_note);
			if ($write_access) {
				echo $email.'&nbsp;';
				// do not print the checkbox if no email is provided for current user
				if ($email != 'none') {
					check_box($prefix.'_use_default', true, true);
					echo translate('bulkmail_preview_use_default');
				}
			} else {
				echo $email;
			}
		$o->closeField();
		return $write_access;

	}//end paintPreviewAddress()


	/**
	* Sends the preview bmail to the default preview address
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
		// this is done in processOptionalAddresses()
		return true;

	}//end processPreviewAddress()


	/**
	* Paints alternative preview address to receive the bmail
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAlternativeAddresses(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$field_name = translate('bulkmail_alternative_address');
			$o->openField($field_name);
				text_box($prefix.'_alternative_address', '');
			$o->closeField();
		}

		return $write_access;

	}//end paintAlternativeAddresses()


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
	function processAlternativeAddresses(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$recipient = Array();
		if (isset($_REQUEST[$prefix.'_alternative_address'])) {
			if (!empty($_REQUEST[$prefix.'_alternative_address'])) {
				$recipient[] = $_REQUEST[$prefix.'_alternative_address'];
			}
		}

		// if use_default box is checked, send the preview to default address too
		if (isset($_POST[$prefix.'_use_default'])) {
			$current_user = $GLOBALS['SQ_SYSTEM']->currentUserId();
			$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_user);
			$recipient[] = $user->attr('email');
		}

		// no duplicate copies to the same address
		$recipient = array_unique($recipient);

		if (!empty($recipient)) {

			// update log
			if (isset($_POST[$prefix.'_clear_log'])) {
				// clear the log if checkbox is checked
				$asset->setAttrValue('preview_log', Array());
			} else {
				$new_log = Array($asset->generateLog($recipient));
				$preview_log = array_merge($new_log, $asset->attr('preview_log'));
				// only keep the last 5 entries
				if (count($preview_log) > 5) array_pop($preview_log);
				$asset->setAttrValue('preview_log', $preview_log);

				// send the actual preview mail
				$asset->sendPreviewMail($recipient);
			}
			return true;
		} else {
			return false;
		}

	}//end processAlternativeAddresses()


	/**
	* Paints the log of last 5 preview records
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
		$preview_log = $asset->attr('preview_log');

		$field_name = translate('bulkmail_preview_log');
		$o->openField($field_name);
			?><ul id="<?php echo $prefix; ?>_preview_log"><?php
			foreach ($preview_log as $index => $log) {
				?><li><?php echo $log; ?></li><?php
			}
			?></ul><?php
		$o->closeField();

		if ($write_access) {
			// checkbox for clearing the preview log
			$field_name = translate('bulkmail_clear_log');
			$o->openField($field_name);
				check_box($prefix.'_clear_log', true, false);
			$o->closeField();
		}

		return $write_access;

	}//end paintPreviewLog()


	/**
	* Processes the log of last 5 preview records
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
		// this is done in processOptionalAddresses()
		return true;

	}//end processPreviewLog()


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

		$jobs = $bulkmail_manager->getQueuedJobs(null, $asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return true;

	}//end processJobQueue()


}//end class
?>
