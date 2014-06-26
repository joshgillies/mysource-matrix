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
* $Id: email_format.inc,v 1.57 2013/07/25 23:23:49 lwright Exp $
*
*/

define('SQ_EMAIL_FORMAT_SEND_TO_EMAIL', 0);
define('SQ_EMAIL_FORMAT_SEND_TO_MATRIX_INBOX', 1);
define('SQ_EMAIL_FORMAT_SEND_TO_BOTH', 2);

require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';
require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

/**
* Email Format Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <email_format with_asset_finder="1" show_inc_attachments="1" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.57 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Email_Format extends Asset_Attribute_WYSIWYG
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Email_Format($attributeid=0, $value=NULL)
	{
		$this->Asset_Attribute_WYSIWYG($attributeid, $value);
		$this->_edit_params['with_asset_finder'] = FALSE;
		$this->_edit_params['show_from'] = TRUE;
		$this->_edit_params['show_reply_to'] = TRUE;
		$this->_edit_params['show_to'] = TRUE;
		$this->_edit_params['show_inc_attachments'] = FALSE;
		$this->_edit_params['show_asset_to_send'] = FALSE;
		$this->_edit_params['show_send_mail_to_type'] = FALSE;

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object	&$node	XML_Tree_Node object.
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return FALSE;
		foreach ($this->_edit_params as $field => $val) {
			if (isset($node->attributes()->$field)) {
				// Cannot convert directly to boolean, since (bool) of any non-empty
				// object is TRUE. Convert to int first, then to bool. (Bug #4095)
				$this->_edit_params[$field] = (bool)(int)$node->attributes()->$field;
			}
		}
		return TRUE;

	}//end setEditParams()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		$prefix = str_replace(':', '_', $prefix);

		$current_value = @unserialize($this->value);
		if (!is_array($current_value)) {
			$current_value = Array();
		}
		if (!isset($current_value['to'])) {
			$current_value['to'] = Array();
		}
		if (!isset($current_value['to_assetids'])) {
			$current_value['to_assetids'] = Array();
		}
		if (!isset($current_value['from'])) {
			$current_value['from'] = '';
		}
		if (!isset($current_value['reply_to'])) {
			$current_value['reply_to'] = '';
		}
		if (!isset($current_value['subject'])) {
			$current_value['subject'] = '';
		}
		if (!isset($current_value['html_format'])) {
			$current_value['html_format'] = '';
		}
		if (!isset($current_value['text_format'])) {
			$current_value['text_format'] = '';
		}
		if (!isset($current_value['inc_attachments'])) {
			$current_value['inc_attachments'] = Array();
		}
		if (!isset($current_value['attachments'])) {
			$current_value['attachments'] = Array();
		}
		if (!isset($current_value['asset_to_send'])) {
			$current_value['asset_to_send'] = '';
		}
		if (!isset($current_value['design_to_apply'])) {
			$current_value['design_to_apply'] = '';
		}
		if (!isset($current_value['layout_to_apply'])) {
			$current_value['layout_to_apply'] = '';
		}
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/email_format.js"></script>
		<table border="0" class="sq-backend-table sq-email-format-wrapper">
		<?php
		if ($this->_edit_params['show_send_mail_to_type']) {
		?>
			<tr>
				<th><?php echo 'Send Mail To'; ?></th>
				<td>
					<?php
						$options = Array(
									SQ_EMAIL_FORMAT_SEND_TO_EMAIL => "User's email",
									SQ_EMAIL_FORMAT_SEND_TO_MATRIX_INBOX => "User's Matrix inbox",
									SQ_EMAIL_FORMAT_SEND_TO_BOTH => "Both",
								);
						$selected_value = array_get_index($current_value, 'send_mail_to_type', SQ_EMAIL_FORMAT_SEND_TO_EMAIL);
						combo_box($prefix.'_send_mail_to_type', $options, FALSE,  $selected_value, 0, ($read_only ? ' disabled' : ''));
					?>
				</td>
			</tr>
		<?php }
		if ($this->_edit_params['show_to']) {
			?>
			<tr>
				<th><?php echo translate('to'); ?></th>
				<td>
					<?php
					$ol = new Asset_Attribute_Option_List();
					$ol->value = implode("\r\n", $current_value['to']);
					$ol->setEditParam('width', '50');
					$ol->paint($prefix.'_to_emails', $read_only);

					if ($this->_edit_params['with_asset_finder']) {
						if ($read_only) {
							foreach ($current_value['to_assetids'] as $assetid) {
								echo get_asset_tag_line($assetid).'<br />';
							}
						} else {
							multiple_asset_finder($prefix.'_to_assetids', $current_value['to_assetids'], Array('user' => 'D', 'user_group' => 'I', 'bulkmail_user' => 'I'));
						}
					}
					?>
				</td>
			</tr>
			<?php
		}
		if ($this->_edit_params['show_from']) {
			?>
			<tr>
				<th><?php echo translate('from'); ?></th>
				<td>
					<?php
					if ($read_only) {
						echo htmlspecialchars($current_value['from']);
					} else {
						text_box($prefix.'_from', $current_value['from'], 50);
					}
					?>
				</td>
			</tr>
			<?php
		}
		if ($this->_edit_params['show_reply_to']) {
			?>
			<tr>
				<th><?php echo translate('reply_to'); ?></th>
				<td>
					<?php
					if ($read_only) {
						echo htmlspecialchars($current_value['reply_to']);
					} else {
						text_box($prefix.'_reply_to', $current_value['reply_to'], 50);
					}
					?>
				</td>
			</tr>
			<?php
		}
		?>
			<tr>
				<th><?php echo translate('subject'); ?></th>
				<td>
					<?php
					if ($read_only) {
						echo htmlspecialchars($current_value['subject']);
					} else {
						text_box($prefix.'_subject', $current_value['subject'], 50);
					}
					?>
				</td>
			</tr>
		<?php
		if ($this->_edit_params['show_inc_attachments']) {
			?>
			<tr>
				<th><?php echo translate('attachments'); ?></th>
				<td>
					<?php
					if ($read_only) {
						echo '<img src="'.sq_web_path('lib').'/web/images/'.($current_value['inc_attachments'] ? 'tick' : 'cross').'.gif" width="15" height="15" />&nbsp;';
						echo translate('include_file_attachments');
					} else {
						check_box($prefix.'_inc_attachments', TRUE, $current_value['inc_attachments']);
						label(translate('include_file_attachments'), $prefix.'_inc_attachments');
					}
					?>
				</td>
			</tr>
			<?php
		}
		?>
			<tr>
				<th><?php echo translate('body'); ?></th>
				<td>
					<table class="sq-email-format-body sq-grey-header-wrapper">
						<tr>
							<td class="sq-grey-header-wrapper sq-with-bottom-border">
								<div class="sq-grey-header-row clearfix">
									<span style="float:right;" id="<?php echo $prefix; ?>_format_mode" class="sq-email-format-version">
										<?php echo translate('html_email_version'); ?>
									</span>
									<?php
									if ($read_only) {
										?>
										<strong>View <span class="sq-toggle-option-wrapper">
											<a href="#" onclick="emailFormatSwitchReadingMode('<?php echo $prefix; ?>', '<?php echo addslashes(translate('html_email_version')); ?>', '<?php echo addslashes(translate('text_email_version')); ?>', this); return sq_handle_toggle(this);">Text</a><a class="selected" href="#" onclick="emailFormatSwitchReadingMode('<?php echo $prefix; ?>', '<?php echo addslashes(translate('html_email_version')); ?>', '<?php echo addslashes(translate('text_email_version')); ?>', this); return sq_handle_toggle(this);">HTML</a>
										</span> Version</strong>
										<?php
									} else {
										?>
										<strong>Edit <span class="sq-toggle-option-wrapper">
											<a href="#" onclick="emailFormatSwitchEditingMode('<?php echo $prefix; ?>', '<?php echo addslashes(translate('html_email_version')); ?>', '<?php echo addslashes(translate('text_email_version')); ?>', this); return sq_handle_toggle(this);">Text</a><a class="selected" href="#" onclick="emailFormatSwitchEditingMode('<?php echo $prefix; ?>', '<?php echo addslashes(translate('html_email_version')); ?>', '<?php echo addslashes(translate('text_email_version')); ?>', this); return sq_handle_toggle(this);">HTML</a>
										</span> Version</strong>
										<?php
									}
									?>
								</div>
							</td>
						</tr>
						<tr>
							<td class="sq-email-format-wysiwyg-wrapper">
								<?php
								if ($read_only) {
									?>
									<div id="<?php echo $prefix?>_html_format" class="sq-wysiwyg-preview-wrapper">
										<?php echo $current_value['html_format']; ?>
									</div>
									<div style="display: none" id="<?php echo $prefix?>_text_format" class="raw-text">
										<?php echo nl2br($current_value['text_format']); ?>
									</div>
									<?php
								} else {
									?>
									<div id="<?php echo $prefix?>_html_body_div" class="sq-email-format-html-body-div">
									<?php
									$o = $GLOBALS['SQ_SYSTEM']->backend->out;
									if(!is_null($o) && $o->isViperAvailable()) {
									    $o->enableViperEditor();
									    // Use a hidden input to pass the Viper edited content to Matrix backend form
									    $o->addOnSubmit('document.getElementById("'.$prefix.'_contents_div_viper_input").value = document.getElementById("'.$prefix.'_contents_div_viper").innerHTML;');
									    ?>
									  <div id="<?php echo $prefix?>_contents_div_viper" class="with-viper"><?php echo $current_value['html_format'];?></div>
									  <input  id="<?php echo $prefix?>_contents_div_viper_input" style="display:none;" name="<?php echo $prefix?>_contents_div_viper_input" value=""/>
									  <?php
									}
									else {
									    $wysiwyg = $this->_createEditor($prefix);
									    $wysiwyg->set_contents($current_value['html_format']);
									    echo $wysiwyg->paint();
									    // enable the WYSIWYG editor by default
									    ?><script type="text/javascript">loadWYSIWYG('<?php echo $prefix; ?>');</script>
									    <?php
									}
									?>
									</div>
									<div style="display: none" id="<?php echo $prefix?>_text_body_div" class="sq-email-format-text-body-div raw-text">
										<?php text_area($prefix.'_text_body', $current_value['text_format'], 105, 15); ?>
									</div>
									<?php
								}
								?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<?php if ($this->_edit_params['show_asset_to_send']) {?>
				<tr>
					<th><?php echo translate('asset_to_send'); ?></th>
					<td>
						<?php asset_finder($prefix.'_asset_to_send', $current_value['asset_to_send'], Array('page' => 'D')); ?><br/>
						<?php echo translate('asset_to_send_desc'); ?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('design_to_apply'); ?></th>
					<td>
						<?php asset_finder($prefix.'_design_to_apply', $current_value['design_to_apply'], Array('design' => 'D')); ?><br/>
						<?php echo translate('design_to_apply_desc'); ?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('layout_to_apply'); ?></th>
					<td>
						<?php asset_finder($prefix.'_layout_to_apply', $current_value['layout_to_apply'], Array('paint_layout_page' => 'D')); ?><br/>
						<?php echo translate('layout_to_apply_desc'); ?>
					</td>
				</tr>
			<?php } ?>
		</table>
		<?php

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$prefix = str_replace(':', '_', $prefix);

		if (!isset($_REQUEST[$prefix.'_to_emails']) && !isset($_REQUEST[$prefix.'_to_assetids']) && !isset($_REQUEST[$prefix.'_from']) && !isset($_REQUEST[$prefix.'_subject']) && !isset($_REQUEST[$prefix.'_text_body'])) {
			return FALSE;
		}

		$value = Array();

		if(isset($_REQUEST[$prefix.'_contents_div_viper_input'])) {
		    $value['html_format'] = trim($_REQUEST[$prefix.'_contents_div_viper_input']);
		}
		else {
		    $wysiwyg = $this->_createEditor($prefix);
		    $value['html_format'] = trim($wysiwyg->process());
		}
		$value['text_format'] = (isset($_REQUEST[$prefix.'_text_body'])) ? $_REQUEST[$prefix.'_text_body'] : '';

		$ol = new Asset_Attribute_Option_List();
		$ol->process($prefix.'_to_emails');
		$value['to'] = explode("\r\n", $ol->value);

		// Remove any blanks from the 'to' list
		for (reset($value['to']); NULL !== ($k = key($value['to'])); next($value['to'])) {
			if (trim($value['to'][$k]) == '') {
				unset($value['to'][$k]);
			}
		}

		$value['to_assetids'] = Array();
		if (isset($_REQUEST[$prefix.'_to_assetids']) && is_array($_REQUEST[$prefix.'_to_assetids'])) {
			foreach ($_REQUEST[$prefix.'_to_assetids'] as $new_to_info) {
				if (!empty($new_to_info['assetid']) && assert_valid_assetid($new_to_info['assetid'], 'Invalid Asset Id: #'.$new_to_info['assetid'].' supplied to attribute"'.$this->name.'"', FALSE, FALSE)) {
					$value['to_assetids'][] = $new_to_info['assetid'];
				}
			}
		}

		$value['from']    = (isset($_REQUEST[$prefix.'_from'])) ? $_REQUEST[$prefix.'_from'] : '';
		$value['reply_to']    = (isset($_REQUEST[$prefix.'_reply_to'])) ? $_REQUEST[$prefix.'_reply_to'] : '';
		$value['subject'] = (isset($_REQUEST[$prefix.'_subject'])) ? $_REQUEST[$prefix.'_subject'] : '';
		$value['inc_attachments'] = (isset($_REQUEST[$prefix.'_inc_attachments'])) ? $_REQUEST[$prefix.'_inc_attachments'] : FALSE;
		$value['asset_to_send'] = (isset($_REQUEST[$prefix.'_asset_to_send']['assetid'])) ? $_REQUEST[$prefix.'_asset_to_send']['assetid'] : '0';
		$value['design_to_apply'] = (isset($_REQUEST[$prefix.'_design_to_apply']['assetid'])) ? $_REQUEST[$prefix.'_design_to_apply']['assetid'] : '0';
		$value['layout_to_apply'] = (isset($_REQUEST[$prefix.'_layout_to_apply']['assetid'])) ? $_REQUEST[$prefix.'_layout_to_apply']['assetid'] : '0';
		$value['send_mail_to_type'] = (isset($_REQUEST[$prefix.'_send_mail_to_type'])) ? $_REQUEST[$prefix.'_send_mail_to_type'] : SQ_EMAIL_FORMAT_SEND_TO_EMAIL;
		// from address is not specified, print a warning message
		if (isset($value['to']) && !empty($value['to']) && empty($value['from'])) {
			trigger_localised_error('CORE0251', E_USER_WARNING, $this->name);
		}

		$this->processed = $this->setValue($value);

	}//end process()


	/**
	* Ensure the attribute being set is an array
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		// cant serilise scalar variables
		if (is_scalar($value)) {
			// see if it is already serilised
			$unser = @unserialize(trim((string) $value));
			if (is_scalar($unser)) {
				return FALSE;
			} else {
				$value = $unser;
			}
		}

		if (!is_array($value)) return FALSE;

		if (!isset($value['to'])) {
			$value['to'] = Array();
		}
		if (!isset($value['to_assetids'])) {
			$value['to_assetids'] = Array();
		}
		if (!isset($value['from'])) {
			$value['from'] = '';
		}
		if (!isset($value['reply_to'])) {
			$value['reply_to'] = '';
		}
		if (!isset($value['subject'])) {
			$value['subject'] = '';
		}
		if (!isset($value['html_format'])) {
			$value['html_format'] = '';
		}
		if (!isset($value['text_format'])) {
			$value['text_format'] = '';
		}
		if (!isset($value['inc_attachments'])) {
			$value['inc_attachments'] = Array();
		}
		if (!isset($value['attachments'])) {
			$value['attachments'] = Array();
		}
		if (!isset($value['asset_to_send'])) {
			$value['asset_to_send'] = '';
		}
		if (!isset($value['design_to_apply'])) {
			$value['design_to_apply'] = '';
		}
		if (!isset($value['layout_to_apply'])) {
			$value['layout_to_apply'] = '';
		}

		$value = serialize($value);
		return TRUE;

	}//end validateValue()


	/**
	* Use the values stored in this attribute to send an email
	*
	* @param array	$replacements			an array of keyword replacements
	* @param array	$additional_attachments	additional filepaths to attach to the email
	*										(must be inside data directory)
	*
	* @return void
	* @access public
	*/
	function sendMail($replacements=Array(), $additional_attachments=Array())
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/www.inc';

		$current_value = @unserialize($this->value);

		// Internal message recepient userids
		$to_assetids = Array();

		// replace keywords in all our values
		$assetid_emails = Array();
		if (isset($current_value['to_assetids']) && !empty($current_value['to_assetids'])) {
			foreach ($current_value['to_assetids'] as $assetid) {
				$addressee = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				if (is_null($addressee)) continue;

				if ($addressee->type() == 'user_group') {
					// Include directly and indirectly-linked shadow assets
					$users = $GLOBALS['SQ_SYSTEM']->am->getChildren($addressee->id, 'user', FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL, Array(), TRUE);

					foreach ($users as $user_id => $type_code) {
					    // make sure shadow ldap user group isn't returned
					    if(isset($type_code[0]['type_code'])) {
							$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_id);
							$assetid_emails[] = $user->attr('email');
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

							$to_assetids[] = $user_id;
					    }
					}
				} else {
					$assetid_emails[] = $addressee->attr('email');
					$to_assetids[] = $addressee->id;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($addressee);
			}
		}

		// Attachments
		$attachments = Array();
		if (isset($current_value['inc_attachments']) && $current_value['inc_attachments'] && isset($current_value['attachments']) && !empty($current_value['attachments'])) {
			foreach ($current_value['attachments'] as $file_asset_id) {
				$file_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($file_asset_id);
				$file_info = $file_asset->getExistingFile();
				$attachments[] = $file_info['path'];
			}
		}

		$to 	   = replace_keywords($current_value['to'], 		 $replacements);
		$from      = replace_keywords($current_value['from'],        $replacements);
		$reply_to  = replace_keywords($current_value['reply_to'],    $replacements);
		$subject   = replace_keywords($current_value['subject'],     $replacements);
		$text_body = replace_keywords($current_value['text_format'], $replacements);
		$html_body = replace_keywords($current_value['html_format'], $replacements);

		// Comma seperated emails in the "to" parameter
		$cs_emails = Array();

		// Firefox fix for html body
		if (trim($html_body) == '<br>') $html_body = '';
		if (isset($current_value['asset_to_send']) && $current_value['asset_to_send'] != 0) {
			$html_body = '';
			$text_body = '';
			// make an array of asset_id, design_id, and layout_id obtained from asset_picker in trigger and
			// put then in an array to be passed to generateRawContent()
			$details = Array(
						'content_id'		=> $current_value['asset_to_send'],
						'content_design'	=> $current_value['design_to_apply'],
						'content_layout'	=> $current_value['layout_to_apply'],
					   );
			$html_body = $GLOBALS['SQ_SYSTEM']->am->generateRawContent($details);

			$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $GLOBALS['SQ_SYSTEM']->am->getAsset($current_value['asset_to_send']));

			foreach ($to as $key => $val) {
				replace_global_keywords($to[$key]);
				// If its got comma seperated values
				if (strpos($to[$key], ',') !== FALSE) {
					$cs_emails = array_merge($cs_emails, explode(',', $to[$key]));
					unset($to[$key]);
				}
			}
			replace_global_keywords($from);
			replace_global_keywords($reply_to);
			replace_global_keywords($subject);
			replace_global_keywords($html_body);

			if (is_null($old_current_asset)) {
				$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
			} else {
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
			}

		} else {
			// replace internal links to pages and images with their full URLs
			$matches = Array();
			preg_match_all('|\./\?a=([0-9]+)|', $html_body, $matches);
			$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($matches[1]));
			foreach ($urls as $assetid => $url) {
				$html_body = preg_replace('|\./\?a='.$assetid.'([^0-9])|', $url.'\\1', $html_body);
			}

			foreach ($to as $key => $val) {
				replace_global_keywords($to[$key]);
				if (strpos($to[$key], ',') !== FALSE) {
					// If its got comma seperated values
					$cs_emails = array_merge($cs_emails, explode(',', $to[$key]));
					unset($to[$key]);
				}
			}
			replace_global_keywords($from);
			replace_global_keywords($reply_to);
			replace_global_keywords($subject);
			replace_global_keywords($text_body);
			replace_global_keywords($html_body);

		}

		// If message needs to be sent to email
		if ($current_value['send_mail_to_type'] == SQ_EMAIL_FORMAT_SEND_TO_EMAIL || $current_value['send_mail_to_type'] == SQ_EMAIL_FORMAT_SEND_TO_BOTH) {

			$to = array_unique(array_merge($to, $assetid_emails, $cs_emails));

			// if no 'From:' address (possibly because of empty keyword replacement)
			// send from the system's default email address
			if (trim($from) == '') $from = SQ_CONF_DEFAULT_EMAIL;

			// can't come up with a From: email address - bail for this email
			if (empty($from)) {
				trigger_localised_error('SYS0315', E_USER_WARNING);
				return;
			}

			require_once 'Mail.php';
			require_once 'Mail/mime.php';

			// new line is \n
			$crlf = "\n";
			$mime = new Mail_mime($crlf);

			$from_addr = preg_replace('|.*\<(.*)\>|', '\\1', $from);

			$tmp_mail = new Mail();
			$mail = $tmp_mail->factory('mail', "-f$from_addr");

			$hdrs = Array(
						'From'		=> $from,
						'Subject'	=> $subject,
					);

			$reply_to = trim($reply_to);
			if (!empty($reply_to)) $hdrs['Reply-To'] = $reply_to;

			foreach ($to as $to_email) {

				$to_email = trim($to_email);

				$to_addr = preg_replace('|.*\<(.*)\>|', '\\1', $to_email);

				if (empty($to_email) || !valid_email($to_addr)) {
					continue;
				}

				// Unset the $mime->_parts (which holds all the attachments for the email)
				// This was not done before, causing an incremental attachment of files
				// for multiple recipients (please see Bug #2897)
				if (count($mime->_parts) > 0) unset($mime->_parts);

				foreach ($attachments as $file_path) {
					if (file_exists($file_path)) {
						$mime_type = file_mime_type($file_path);
						@$mime->addAttachment($file_path, $mime_type); // throws PHP5 strict warning (Non-static method PEAR::isError should not be called statically)
					}
				}

				// Additional attachments.
				foreach ($additional_attachments as $file_path) {
					if (file_exists($file_path) && (strpos(realpath($file_path), SQ_DATA_PATH) === 0)) {
						$mime_type = file_mime_type($file_path);
						@$mime->addAttachment($file_path, $mime_type); // throws PHP5 strict warning
					}
				}

				// if it's email verification keyword replace it with a token
				$keywords = retrieve_keywords_replacements($text_body);
				$keywords += retrieve_keywords_replacements($html_body);
				if(!empty($keywords)) {
				    foreach ($keywords as $keyword) {
						if($keyword === 'email_verification_token') {
					    	// generate a random token
	                	    include_once SQ_FUDGE_PATH.'/general/security.inc';
					    	$token = md5(uniqid(security_rand(), TRUE));
						    $_SESSION['SQ_VERIFY_EMAIL_RAW'] = $to_email;
						    $_SESSION['SQ_VERIFY_EMAIL_TOKEN'] = $token;
						    $_SESSION['SQ_VERIFY_EMAIL_VERIFIED'] = '';
						    $replacements = Array($keyword => $token);
						   $text_body = replace_keywords($text_body, $replacements);
						   $html_body = replace_keywords($html_body, $replacements);
						}
			    	}
				}

				$mime->setTXTBody($text_body);
				$mime->setHTMLBody($html_body);

				// get() must be called before headers()
				// set up the charsets, use matrix preferences
				$params = Array(
							'html_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
							'text_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
							'head_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						  );
				$body = $mime->get($params);

				$hdrs = $mime->headers($hdrs);

				$msg = $mail->send($to_email, $hdrs, $body);

			}//end foreach

		}//end if send type = "email"

		// If message needs to be sent to Matrix inbox
		if (!empty($to_assetids) && ($current_value['send_mail_to_type'] == SQ_EMAIL_FORMAT_SEND_TO_MATRIX_INBOX || $current_value['send_mail_to_type'] == SQ_EMAIL_FORMAT_SEND_TO_BOTH)) {

			$to_assetids = array_unique($to_assetids);
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

			$replacements = Array(
								'subject' => $subject,
								'body' => $text_body ? $text_body : $html_body,
							);
			$msg = $ms->newMessage($to_assetids, 'internal.message', $replacements);
			// Asset triggering the send mail process
			$msg->parameters['assetid'] = array_get_index($current_value, 'triggering_assetid', '');
			// Send the message to the user's Matrix inbox
			$msg->send(TRUE);
		}//end if send type = "Matrix inbox"

	}//end sendMail()


	/**
	* Get any keywords that are present in this attr's components
	*
	* @return void
	* @access public
	*/
	function getKeywords()
	{
		$current_value = @unserialize($this->value);
		$keyword_fields = Array(
							'to',
							'from',
							'reply_to',
							'subject',
							'text_format',
							'html_format',
						  );

		$res = Array();
		foreach ($keyword_fields as $field) {
			$field_value = array_get_index($current_value, $field, '');
			if (!empty($field_value)) {
				if (is_array($field_value)) {
					$value = Array();
					foreach ($field_value as $idx => $val) {
						$value[$idx] = retrieve_keywords_replacements($val);
						$res = array_merge($res, $value[$idx]);
					}
				} else {
					$value = retrieve_keywords_replacements($field_value);
					$res = array_merge($res, $value);
				}
			}
		}

		return $res;

	}//end getKeywords()


}//end class

?>
