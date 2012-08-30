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
* $Id: form_action_send_email.inc,v 1.2 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/email_format/email_format.inc';

require_once SQ_FUDGE_PATH.'/general/general.inc';

/**
* Form_Action_Send_Email
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/


class Form_Action_Send_Email extends Form_Action
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['email_format'])) {
			$email_settings = serialize(Array());
		} else {
			$email_settings = $settings['email_format'];
		}

		// Set up email format
		$email_format = new Asset_Attribute_Email_Format(0, $email_settings);
		$xml = simplexml_load_string('<email_format with_asset_finder="1" show_inc_attachments="1" />');
		$email_format->setEditParams($xml);

		$email_format->paint($prefix);

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	*
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['email_format'])) {
			$email_settings = serialize(Array());
		} else {
			$email_settings = $settings['email_format'];
		}

		// Set up email format
		$email_format = new Asset_Attribute_Email_Format(0, $email_settings);
		$xml = simplexml_load_string('<email_format with_asset_finder="1" show_inc_attachments="1" />');
		$email_format->setEditParams($xml);

		$email_format->name = 'current action\'s settings';
		$email_format->process($prefix);

		$settings['email_format'] = $email_format->value;

		return $email_format->validateValue($email_format->value);

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	*
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['email_format'])) {
			$email_settings = Array();
		} else {
			$email_settings = unserialize($settings['email_format']);
		}
		self::_fillEmailFormatDefaults($email_settings);

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>To:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					$i = 0;
					$to_array = Array();
					$all_keywords = TRUE;

					// Get manually-entered "to" settings.
					for (; ($i < 6) && !empty($email_settings['to']); $i++) {
						$this_email = array_shift($email_settings['to']);
						$to_array[] = htmlspecialchars($this_email);
						if (!preg_match('/%.*%/', $this_email)) {
							$all_keywords = FALSE;
						}
					}

					// Now the users (by definition, these are not based on keywords).
					// Continue $i's count from previous loop.
					for (; ($i < 6) && !empty($email_settings['to_assetids']); $i++) {
						$to_array[] = get_asset_tag_line(array_shift($email_settings['to_assetids']));
						$all_keywords = FALSE;
					}

					if ($i > 0) {
						echo implode(', ', $to_array);
						$extra_recipients = count($email_settings['to']) + count($email_settings['to_assetids']);
						if ($extra_recipients > 0) {
							?>, <em>plus <?php echo $extra_recipients ?> more recipients</em><?php
						}
						if ($all_keywords) {
							?><br/><em>Note: All of your recipients are based on the results of keywords (for instance, submitted responses). If these are left blank in a submission, this email may not send.</em><?php
						}
					} else {
						?><span class="sq-backend-warning">No recipients specified.</span> This email cannot send without at least one recipient.<?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>From:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($email_settings['from'])) {
						echo htmlspecialchars($email_settings['from']);
					} else {
						?><span class="sq-backend-warning">No "From:" address specified.</span> The system's default email address (<?php echo SQ_CONF_DEFAULT_EMAIL ?>) will be used.<?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Subject:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($email_settings['subject'])) {
						echo htmlspecialchars($email_settings['subject']);
					} else {
						?><span class="sq-backend-warning">No subject specified.</span> Although an email can send without a subject, it is recommended you add one.<?php
					} ?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>HTML Body:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($email_settings['html_format'])) {
						echo ellipsisize(strip_tags(preg_replace('/\\n/', ' ', $email_settings['html_format'])), 512);
					} else {
						?><span class="sq-backend-warning">The HTML body is empty.</span><?php
					} ?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Text Body:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($email_settings['text_format'])) {
						echo ellipsisize(preg_replace('/\\n/', ' ', $email_settings['text_format']), 512);
					} else {
						?><span class="sq-backend-warning">The text-only body is empty.</span><?php
					} ?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Uploads:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p>File uploads are <strong><?php echo $email_settings['inc_attachments'] ? 'attached' : 'not attached' ?></strong>.</p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	*/
	public static function execute(Form $form, $settings)
	{
		$email_settings = $settings['email_format'];

		// Set up email format and let it be the final judge
		$email_format = new Asset_Attribute_Email_Format(0, $email_settings);
		$xml = simplexml_load_string('<email_format with_asset_finder="1" show_inc_attachments="1" />');
		$email_format->setEditParams($xml);

		$form->sendEmail($email_format);
		return TRUE;

	}//end execute()


	/**
	* Execute form action
	*
	*/
	public static function isValid(Form $form, $settings)
	{
		if (!isset($settings['email_format'])) {
			$email_settings = Array();
		} else {
			$email_settings = unserialize($settings['email_format']);
		}
		self::_fillEmailFormatDefaults($email_settings);

		// No-one to send it to!
		if (empty($email_settings['to']) && empty($email_settings['to_assetids'])) {
			return FALSE;
		}

		// Nothing in either body!
		if (empty($email_settings['text_format']) && empty($email_settings['html_format'])) {
			return FALSE;
		}

		// Set up email format and let it be the final judge
		$email_format = new Asset_Attribute_Email_Format(0, serialize($email_settings));
		$xml = simplexml_load_string('<email_format with_asset_finder="1" show_inc_attachments="1" />');
		$email_format->setEditParams($xml);

		return $email_format->validateValue($email_settings);

	}//end isValid()


	/**
	* Fill any missing values from the email format with defaults
	*
	* @param array	&$email_format	The unserialised email format settings
	*
	* @return void
	*/
	private static function _fillEmailFormatDefaults(&$email_format) {
		if (!isset($email_format['to'])) {
			$email_format['to'] = Array();
		}

		if (!isset($email_format['to_assetids'])) {
			$email_format['to_assetids'] = Array();
		}

		if (!isset($email_format['from'])) {
			$email_format['from'] = '';
		}

		if (!isset($email_format['subject'])) {
			$email_format['subject'] = '';
		}

		if (!isset($email_format['html_format'])) {
			$email_format['html_format'] = '';
		}

		if (!isset($email_format['text_format'])) {
			$email_format['text_format'] = '';
		}

		if (!isset($email_format['inc_attachments'])) {
			$email_format['inc_attachments'] = Array();
		}

		if (!isset($email_format['attachments'])) {
			$email_format['attachments'] = Array();
		}


	}//end _fillDefaultValues()


}//end class

?>
