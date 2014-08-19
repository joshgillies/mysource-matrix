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
* $Id: inbox_edit_fns.inc,v 1.65 2013/09/24 01:20:42 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Inbox_Edit_Fns
*
* Purpose
*	Functions to print and process the backend interface of the user's inbox
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @version $Revision: 1.65 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Inbox_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Inbox_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array ('details' => Array ('name' => translate('inbox'), 'use_tab' => TRUE, 'force_unlock' => FALSE, 'lock_type' => 'mail',),);

	}//end constructor


	/**
	* Prints the new message interface
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewMessage(&$asset, &$o, $prefix)
	{
		$user = $asset->_getUser();

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, NULL, Array (SQ_MSG_UNREAD, SQ_MSG_READ), Array (), NULL, NULL, 'name');
		$write_access = $asset->writeAccess('mail');
		if ($write_access) {
?>
			<style type="text/css">
				@import url(<?php echo sq_web_path('lib')?>/web/css/inbox.css);
			</style>

			<table class="sq-new-message-table" width="100%">
				<tr>
					<td valign="top"><?php echo translate('to'); ?>:</td>
					<td>
						<?php multiple_asset_finder($prefix.'_To', Array(), Array('user' => 'D')); ?>
					</td>
				</tr>
				<tr>
					<td valign="top"><?php echo translate('subject'); ?>:</td>
					<td>
						<input type="text" id="<?php echo $prefix; ?>_Subject" name="<?php echo $prefix; ?>_Subject" style="width:300px" />
					</td>
				</tr>
				<tr>
					<td valign="top"><?php echo translate('priority'); ?>:</td>
					<td>
						<select id="<?php echo $prefix; ?>_Priority" name="<?php echo $prefix; ?>_Priority" style="float:left; width: 100px;">
							<option value="1"><?php echo translate('highest'); ?></option>
							<option value="2"><?php echo translate('high'); ?></option>
							<option value="3" selected="selected"><?php echo translate('standard'); ?></option>
							<option value="4"><?php echo translate('low'); ?></option>
							<option value="5"><?php echo translate('lowest'); ?></option>
						</select>
					</td>
				</tr>
				<tr>
					<td colspan="2">
						<textarea class="sq-new-message-box" style="clear:both; width: 95%; height: 250px" id="<?php echo $prefix; ?>_Content" name="<?php echo $prefix; ?>_Content"></textarea>
					</td>
				</tr>
			</table>
			<?php

		} else {
?>
			<div style="text-align: center"><?php echo translate('inbox_lock'); ?></div>
			<?php

		}

		return $write_access;

	}//end paintNewMessage()


	/**
	* Processes the new message interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewMessage(&$asset, &$o, $prefix)
	{
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		// send the message
		$noaction = TRUE;
		if (isset ($_POST[$prefix.'_To'])) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			$recepients = Array ();
			foreach ($_POST[$prefix.'_To'] as $key => $sender) {
				$recepients[] = $sender['assetid'];
			}

			$msg_reps = Array(
							'subject'	=> $_POST[$prefix.'_Subject'],
							'body'		=> $_POST[$prefix.'_Content'],
						);

			$message = $ms->newMessage($recepients, 'inbox.message', $msg_reps);
			$message->priority = $_POST[$prefix.'_Priority'];
			$message->send();

			foreach ($recepients as $key => $sender) {
				if (empty ($sender)) continue;
				$noaction = TRUE;
				$message = $ms->newMessage(Array ($userid), 'inbox.sent', $msg_reps);
				$message->from = $sender;
				$message->status = 'R';
				$message->priority = $_POST[$prefix.'_Priority'];
				$message->send();
			}

		}
		if ($noaction) echo '<!-- ###No Action### -->';

	}//end processNewMessage()


	/**
	* Prints the sent box interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSent(&$asset, &$o, $prefix)
	{
		$user = $asset->_getUser();

		$priority = Array ('', 'Highest', 'High', 'Normal', 'Low', 'Lowest');

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, NULL, Array (SQ_MSG_UNREAD, SQ_MSG_READ), Array (), NULL, NULL, 'name');
		$write_access = $asset->writeAccess('mail');
		$num_messages = count($messages);
?>
			<style type="text/css">
				@import url(<?php echo sq_web_path('lib')?>/web/css/inbox.css);
			</style>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
			<script type="text/javascript">
			//<![CDATA[
				var prefix = "<?php echo $prefix; ?>";
				var Bodies = new Object();
			//]]>
			</script>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/inbox/js/inbox.js"></script>
			<div id="inbox_container" class="sq-inbox-list-container">
			<table id="inbox_table" class="sq-backend-table">
			<tr>
				<th class="sq-backend-table-header"><?php echo translate('to'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('subject'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('sent'); ?></th>
				<?php

		if ($write_access) {
?>
					<th class="sq-backend-table-header">
						<?php check_box($prefix.'_delete_all', '1', FALSE, 'changeAllDelStatuses(this.checked)'); ?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon" class="trash" onclick="changeAllStatuses()" />
						<?php echo translate('delete'); ?>
					</th>
					<?php

		}
?>
			</tr>
			<?php


		for ($i = 0; $i < $num_messages; $i ++) {

			$data = $messages[$i];
			if ($data['type'] != 'inbox.sent') continue;

			// work out the sender icon
			$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
			// work out the message body
			$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
			$body = str_replace('"', '&quot;', $body);
?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="sq-backend-table-cell" nowrap="nowrap" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16, '', $priority[$data['priority']].' priority') ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="sq-backend-table-cell" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo htmlspecialchars($data['subject'], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET); ?>
					</td>
					<td class="sq-backend-table-cell" nowrap="nowrap" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<?php

			if ($write_access) {
?>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php check_box($prefix.'_delete['.$data['msgid'].']', '1', FALSE, 'changeStatus('.$data['msgid'].')'); ?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon<?php echo $data['msgid'];?>" class="trash" onclick="changeStatus('<?php echo $data['msgid'];?>')" />
						</td>
						<?php

			}
?>
				</tr>
				<?php

		}//end for - each message
?>
			</table>
			</div>
			<div id="sq_message_body"></div>
		<?php


		return $write_access;

	}//end paintSent()


	/**
	* Processes the inbox sent interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSent(&$asset, &$o, $prefix)
	{
		$delete = (isset ($_POST[$prefix.'_delete'])) ? array_keys($_POST[$prefix.'_delete']) : Array();

		if (empty($delete)) {
			return;
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		foreach ($delete as $messageid) {
			// delete message
			$message = $ms->getMessageById($messageid);
			if (!$message->updateStatus(SQ_MSG_DELETED)) {
				trigger_localised_error('CORE0063', E_USER_WARNING, $messageid);
			}
		}

		return TRUE;

	}//end processSent()


	/**
	* Prints the inbox interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInbox(&$asset, &$o, $prefix)
	{
		$user = $asset->_getUser();

		$priority = Array ('', 'Highest', 'High', 'Normal', 'Low', 'Lowest');

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, NULL, Array (SQ_MSG_UNREAD, SQ_MSG_READ), Array (), NULL, NULL, 'name');
		$write_access = $asset->writeAccess('mail');

		$allread = TRUE;
		$num_messages = count($messages);
		$num_sent = 0;
		$num_inbox = 0;
		for ($i = 0; $i < $num_messages; $i ++) {
			$data = $messages[$i];
			if ($data['status'] == SQ_MSG_UNREAD) {
				$allread = FALSE;
			}
			if ($data['type'] == 'inbox.sent') {
				$num_sent ++;
			} else {
				$num_inbox ++;
			}
		}
?>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
			<script type="text/javascript">
			//<![CDATA[
				var prefix = "<?php echo urlencode($prefix); ?>";
				var Bodies = new Object();
			//]]>
			</script>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/inbox/js/inbox.js"></script>
			<style type="text/css">
				@import url(<?php echo sq_web_path('lib')?>/web/css/inbox.css);
			</style>
			<div id="inbox_container">
			<?php

		if ($num_inbox == 0) {
			echo '<div class="sq-inbox-notification">Your inbox is empty.</div></div>';
		} else {
			$num_per_page = $GLOBALS['SQ_SYSTEM']->getUserPrefs('inbox', 'SQ_INBOX_MAIL_PER_PAGE');
			if (empty($num_per_page)) $num_per_page = 20;
			$total_assets = $num_inbox;
			$num_pages = ceil($total_assets / $num_per_page);

			$link_offset = array_get_index($_POST, $prefix.'_link_offset', 0);

			// if the page number is greater to the total of pages
			if ( (round(($link_offset + $num_per_page) / $num_per_page)) > (ceil($total_assets / $num_per_page)) ) {
				$link_offset = (ceil($total_assets / $num_per_page)-1)*$num_per_page;
			}
?>
			<!-- start print pagination bar top -->
			<table id="inbox_table" class="sq-backend-table">
				<tr><th colspan="5" class="sq-backend-table-header-header sq-pagination-links" style="text-align: center;">
					<?php
					hidden_field($prefix.'_link_offset', $link_offset);
					if ($link_offset > 0) {
					?>
						<a title="<?php echo translate('pagination_go_to_first') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;">
					<?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" ><?php
					}
					?>
					&lt;&lt;
					<?php
					if ($link_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if ($link_offset > 0) {
						?><a title="<?php echo translate('pagination_go_to_previous') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" ><?php
					}
					?>
					&lt;
					<?php
					if ($link_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp; &nbsp; <?php echo translate('page_number', round(($link_offset + $num_per_page) / $num_per_page), ceil($total_assets / $num_per_page)); ?> &nbsp; &nbsp;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('pagination_go_to_next') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" ><?php
					}
					?>
					&gt;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('pagination_go_to_last') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" ><?php
					}
					?>
					&gt;&gt;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
				</th><tr>
				<!-- end print pagination bar -->
			<tr>
				<th class="sq-backend-table-header"><?php echo translate('from'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('subject'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('sent'); ?></th>
				<th class="sq-backend-table-header">
					<?php

			if ($write_access) {
?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="letterIcon" class="read" />
					<?php

			}
?>
					&nbsp;<?php echo translate('read'); ?>
				</th>
				<?php

			if ($write_access) {
?>
					<th class="sq-backend-table-header">
						<?php check_box($prefix.'_delete_all', '1', FALSE, 'changeAllDelStatuses(this.checked)'); ?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon" class="trash" />
						<?php echo translate('delete'); ?>
					</th>
					<?php

			}
?>
			</tr>
			<?php

			$total_assets = 0;
			for ($i = 0; $i < $num_messages; $i ++) {

				$data = $messages[$i];
				if ($data['type'] == 'inbox.sent') continue;

				if (($total_assets < $link_offset) || (($link_offset+$num_per_page-1) < $total_assets)) {
					$total_assets++;
					continue;
				}

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
				$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
				$body = str_replace('"', '\"', $body);
?>
				<script type="text/javascript">
				//<![CDATA[
					// adding body text of the messages into JS array.
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="sq-backend-table-cell" nowrap="nowrap" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16, '', $priority[$data['priority']].' priority') ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="sq-backend-table-cell" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo htmlspecialchars($data['subject'], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET); ?>
					</td>
					<td class="sq-backend-table-cell" nowrap="nowrap" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="sq-backend-table-cell" nowrap="nowrap">
						<?php

				check_box(urlencode($prefix).'_mark_as_read['.$data['msgid'].']', '1', ($data['status'] == SQ_MSG_READ), 'readClick(this)', ($write_access) ? '' : 'style="display:none"');
?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="letterIcon[<?php echo $data['msgid'];?>]" class="<?php echo (($data['status'] == SQ_MSG_UNREAD) ? 'un' : ''); ?>read" onclick="readClick(this)" />
					</td>
					<?php

				if ($write_access) {
?>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php check_box($prefix.'_delete['.$data['msgid'].']', '1', FALSE, 'changeStatus('.$data['msgid'].')'); ?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon[<?php echo $data['msgid'];?>]" class="trash" onclick="changeStatus('<?php echo $data['msgid'];?>', 'delete')" />
						</td>
						<?php

				}
?>
				</tr>
				<?php
				$total_assets++;
			}//end for - each message
?>
			</table>
			</div>
			<script type="text/javascript">
			//<![CDATA[
				refreshAllStatus();
			//]]>
			</script>
			<div id="sq_message_body"></div>
			<?php

		}//end else - if empty inbox

		$this->_processReadStatus($asset, $prefix);

		return $write_access;

	}//end paintInbox()


	/**
	* Processes the read statuses of the messages
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access private
	*/
	function _processReadStatus(&$asset, $prefix)
	{
		$msgs = (isset ($_REQUEST[$prefix.'_messages'])) ? array_keys($_REQUEST[$prefix.'_messages']) : Array ();
		$read = (isset ($_REQUEST[$prefix.'_mark_as_read'])) ? array_keys($_REQUEST[$prefix.'_mark_as_read']) : Array ();

		if (empty($msgs)) {
			return;
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		foreach ($msgs as $msgid) {
			$message = $ms->getMessageById($msgid);
			if (in_array($msgid, $read) && $message->status == SQ_MSG_UNREAD) {
				if (!$message->updateStatus(SQ_MSG_READ)) {
					trigger_localised_error('CORE0076', E_USER_WARNING, $msgid);
				}
				continue;
			}
			if (!in_array($msgid, $read) && $message->status == SQ_MSG_READ) {
				if (!$message->updateStatus(SQ_MSG_UNREAD)) {
					trigger_localised_error('CORE0076', E_USER_WARNING, $msgid);
				}
			}
		}

	}//end _processReadStatus()


	/**
	* Processes the inbox interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInbox(&$asset, &$o, $prefix)
	{
		$msgs = (isset ($_POST[$prefix.'_messages'])) ? array_keys($_POST[$prefix.'_messages']) : Array ();
		$delete = (isset ($_POST[$prefix.'_delete'])) ? array_keys($_POST[$prefix.'_delete']) : Array ();

		if (empty($msgs)) {
			return;
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		foreach ($msgs as $msgid) {
			$message = $ms->getMessageById($msgid);
			if (in_array($msgid, $delete) && $message->status != SQ_MSG_DELETED) {
				if (!$message->updateStatus(SQ_MSG_DELETED)) {
					trigger_localised_error('CORE0063', E_USER_WARNING, $data['msgid']);
				}
			}
		}

		$o->addOnLoad('if (parent.frames["sidenav"] && parent.frames["sidenav"].refresh_internal_messages) parent.frames["sidenav"].refresh_internal_messages();');

		return TRUE;

	}//end processInbox()


	/**
	* Prints the trash interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTrash(&$asset, &$o, $prefix)
	{
		$user = $asset->_getUser();

		$priority = Array('', 'Highest', 'High', 'Normal', 'Low', 'Lowest');

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, NULL, Array(SQ_MSG_DELETED), Array(), NULL, NULL, 'name');
		$write_access = $asset->writeAccess('mail');
?>
			<style type="text/css">
				@import url(<?php echo sq_web_path('lib')?>/web/css/inbox.css);
			</style>
			<script type="text/javascript" >
			//<![CDATA[
				var Bodies = new Object();
				var prefix = "<?php echo $prefix; ?>";
			//]]>
			</script>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/inbox/js/inbox.js"></script>
		<?php

		if (empty($messages)) {
			?><div style="text-align: center"><?php echo translate('inbox_trash_empty'); ?></div><?php
			return FALSE;
		} else {
?>
			<div id="inbox_container" class="sq-inbox-list-container">

			<table id="inbox_table" class="sq-backend-table">
			<tr>
				<th class="sq-backend-table-header"><?php echo translate('from'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('subject'); ?></th>
				<th class="sq-backend-table-header"><?php echo translate('sent'); ?></th>
				<?php

			if ($write_access) {
?>
					<th class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_recover', '1', FALSE, 'check_all(document.main_form, "'.$prefix.'_recover", this.checked);'); ?>
						<?php echo translate('recover'); ?>
					</th>
					<th class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_purge', '1', FALSE, 'check_all(document.main_form, "'.$prefix.'_purge", this.checked);'); ?>
						<?php echo translate('purge'); ?>
					</th>
					<?php

			}
?>
			</tr>
			<?php

			$num_messages = count($messages);

			for ($i = 0; $i < $num_messages; $i ++) {

				$data = $messages[$i];

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);

				$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
				$body = str_replace('"', '\"', $body);
?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="sq-backend-table-cell" nowrap="nowrap" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16, '', $priority[$data['priority']].' priority') ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="sq-backend-table-cell" onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo htmlspecialchars($data['subject'], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET); ?>
					</td>
					<td class="sq-backend-table-cell" onclick="showBody('<?php echo $data['msgid'];?>', this);" style="cursor: pointer;white-space: nowrap;">
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>

					<?php

				if ($write_access) {
?>
						<td class="sq-backend-table-cell" nowrap>
							<?php

					if ($asset->writeAccess('mail')) {
						check_box($prefix.'_recover['.$data['msgid'].']');
					} else {
						echo '&nbsp;';
					}

?>
						</td>
						<td class="sq-backend-table-cell" nowrap>
							<?php check_box($prefix.'_purge['.$data['msgid'].']'); ?>
						</td>
						<?php

				}
?>
				</tr>
				<?php

			}//end for - each message
?>
			</table>
			</div>
			<div id="sq_message_body"></div>
		<?php

		}//end else - if trash empty

		return TRUE;

	}//end paintTrash()


	/**
	* Processes the trash interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processTrash(&$asset, &$o, $prefix)
	{
		$msgs = (isset ($_POST[$prefix.'_messages'])) ? array_keys($_POST[$prefix.'_messages']) : Array();
		$recover = (isset ($_POST[$prefix.'_recover'])) ? array_keys($_POST[$prefix.'_recover']) : Array();
		$purge = (isset ($_POST[$prefix.'_purge'])) ? array_keys($_POST[$prefix.'_purge']) : Array();

		if (empty($msgs)) {
			return;
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		if (!empty ($recover)) {
			foreach ($recover as $messageid) {
				// recover messages to inbox
				$message = $ms->getMessageById($messageid);
				if (!$message->updateStatus(SQ_MSG_READ)) {
					trigger_localised_error('CORE0064', E_USER_WARNING, $messageid);
				}
			}
		}

		if (!empty ($purge)) {
			foreach ($purge as $messageid) {
				// delete messsage FOREVER
				$message = $ms->getMessageById($messageid);
				if (!$message->delete()) {
					trigger_localised_error('CORE0063', E_USER_WARNING, $data['msgid']);
				}
			}
		}
		return TRUE;

	}//end processTrash()


}//end class

?>
