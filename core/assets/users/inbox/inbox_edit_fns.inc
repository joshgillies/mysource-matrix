<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: inbox_edit_fns.inc,v 1.40 2005/02/23 06:01:43 dbaranovskiy Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Inbox_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.40 $
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

		$this->static_screens = Array(	'details'		=> Array(
															'name'			=> 'Inbox',
															'use_tab'		=> true,
															'force_unlock'	=> false,
															'lock_type'		=> 'mail',
															),
									);

	}//end constructor


	/**
	* Prints the message with the passed ID
	*
	* @param int	$messageid	the ID of the message to print
	*
	* @return boolean
	* @access public
	*/
	function paintMessage($messageid)
	{
		// get the message we are printing
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message = $ms->getMessageById($messageid);

		// mark the message as read
		if ($message->status == SQ_MSG_UNREAD) {
			$message->updateStatus(SQ_MSG_READ);
		}

		return $message->printBody();

	}//end paintMessage()


	/**
	* Prints the new message interface
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewMessage(&$asset, &$o, $prefix)
	{
		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_UNREAD, SQ_MSG_READ), Array(), null, null, 'name');
		$write_access = $asset->writeAccess('mail');
		if ($write_access) { ?>
			<style type="text/css">
				@import url(<?php echo sq_web_path('data')?>/asset_types/inbox/css/inbox.css.php);
			</style>

			<div>
				<label style="width: 10ex;float: left;">To:</label>
				<div style="float:left;">
					<?php multiple_asset_finder($prefix.'_To', Array(), Array('user' => 'D')); ?>
				</div>
			</div>
			<br style="clear:both;" />
			<label style="width: 10ex;float: left;clear: left;">Subject:</label>
			<input type="text" id="<?php echo $prefix; ?>_Subject" name="<?php echo $prefix; ?>_Subject" style="width:300px" />
			<label style="width: 10ex;float: left;">Priority:</label>
			<select id="<?php echo $prefix; ?>_Priority" name="<?php echo $prefix; ?>_Priority" style="float:left; width: 100px;">
				<option value="1">Highest</option>
				<option value="2">High</option>
				<option value="3" selected="selected">Normal</option>
				<option value="4">Low</option>
				<option value="5">Lowest</option>
			</select>
			<br />
			<textarea cols="3" rows="3" style="clear:both; width: 95%; height: 250px" id="<?php echo $prefix; ?>_Content" name="<?php echo $prefix; ?>_Content"></textarea>
		<?php } else { ?>
			<div style="text-align: center">Lock inbox to send messages.</div>
		<?php }

		return true;

	}



	/**
	* Prints the sent box interface
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSent(&$asset, &$o, $prefix)
	{
		$user = &$asset->_getUser();

		$priority = Array('', 'Highest', 'High', 'Normal', 'Low', 'Lowest');

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_UNREAD, SQ_MSG_READ), Array(), null, null, 'name');
		$write_access = $asset->writeAccess('mail');
		$num_messages = count($messages);
		?>
			<style type="text/css">
				@import url(<?php echo sq_web_path('data')?>/asset_types/inbox/css/inbox.css.php);
			</style>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
			<script type="text/javascript">
			//<![CDATA[
				var prefix = "<?php echo $prefix; ?>";
				var Bodies = new Object();
			//]]>
			</script>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/inbox/js/inbox.js"></script>
			<div id="inbox_container">
			<table border="0" cellspacing="0" cellpadding="1" width="100%" id="inbox_table">
			<tr>
				<td class="sq-backend-table-header">TO</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_delete_all', '1', false, 'changeAllDelStatuses(this.checked)'); ?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon" class="trash" onclick="changeAllStatuses()" />
						DELETE
					</td>
					<?php
				}
				?>
			</tr>
			<?php
			$alt = false;

			for ($i = 0; $i < $num_messages; $i++) {

				$data = $messages[$i];
				if ($data['type'] != 'hipo.sent') {
					continue;
				}

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
				// work out the message body
				$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
				$body = str_replace("\"", '&quot;', $body);
				?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16, '', $priority[$data['priority']].' priority') ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="<?php echo $class ?>">
						<span onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo $data['subject'];?>
						</span>
					</td>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="<?php echo $class ?>" nowrap="nowrap">
							<?php check_box($prefix.'_delete['.$data['msgid'].']', '1', false, 'changeStatus('.$data['msgid'].')'); ?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon<?php echo $data['msgid'];?>" class="trash" onclick="changeStatus('<?php echo $data['msgid'];?>')" />
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
			?>
			</table>
			</div>
			<?php if ($write_access) { ?>
				<div id="sq_message_body"></div>
			<?php } else { ?>
				<div id="sq_message_body_disabled">Acquire the lock on the inbox to read messages.</div>
			<?php } ?>
			</div>
		<?php
		return true;

	}


	/**
	* Prints the inbox interface
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMail(&$asset, &$o, $prefix)
	{
		$user = &$asset->_getUser();

		$priority = Array('', 'Highest', 'High', 'Normal', 'Low', 'Lowest');

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_UNREAD, SQ_MSG_READ), Array(), null, null, 'name');
		$write_access = $asset->writeAccess('mail');

			$allread = true;
			$num_messages = count($messages);
			$num_sent = 0;
			$num_inbox = 0;
			for ($i = 0; $i < $num_messages; $i++) {
				$data = $messages[$i];
				if ($data['status'] == SQ_MSG_UNREAD) {
					$allread = false;
				}
				if ($data['type'] == 'hipo.sent') {
					$num_sent++;
				} else {
					$num_inbox++;
				}
			}
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
			<script type="text/javascript">
			//<![CDATA[
				var prefix = "<?php echo $prefix; ?>";
				var Bodies = new Object();
			//]]>
			</script>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/inbox/js/inbox.js"></script>
			<style type="text/css">
				@import url(<?php echo sq_web_path('data')?>/asset_types/inbox/css/inbox.css.php);
			</style>
			<div id="inbox_container">
			<?php
				if ($num_inbox == 0) {
					echo '<div style="text-align: center">Your inbox is empty.</div></div>';
				} else {
			?>
			<table border="0" cellspacing="0" cellpadding="1" id="inbox_table">
			<tr>
				<td class="sq-backend-table-header">FROM</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<td class="sq-backend-table-header">
					<?php
					if ($write_access) { ?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="letterIcon" class="read" />
					<?php }
					?>
					READ
				</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_delete_all', '1', false, 'changeAllDelStatuses(this.checked)'); ?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon" class="trash" />
						DELETE
					</td>
					<?php
				}
				?>
			</tr>
			<?php
			$alt = false;

			for ($i = 0; $i < $num_messages; $i++) {

				$data = $messages[$i];
				if ($data['type'] == 'hipo.sent') {
					continue;
				}

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
				if ($write_access) {
				$body = str_replace("\n", '<br />', $data['body']);
				$body = str_replace("\"", '&quot;', $body);
				?>
				<script type="text/javascript">
				//<![CDATA[
					// adding body text of the messages into JS array.
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<?php } ?>
				<tr>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16, '', $priority[$data['priority']].' priority') ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="<?php echo $class ?>">
						<span onclick="showBody('<?php echo $data['msgid'];?>', this)" style="cursor: pointer">
						&nbsp;<?php echo $data['subject'];?>
						</span>
					</td>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php
						if ($asset->writeAccess('mail')) {
							check_box($prefix.'_mark_as_read['.$data['msgid'].']', '1', ($data['status'] == SQ_MSG_READ), 'readClick(this)');
						}
						?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="letterIcon[<?php echo $data['msgid'];?>]" class="<?php echo (($data['status'] == SQ_MSG_UNREAD) ? 'un' : ''); ?>read" onclick="readClick(this)" />
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="<?php echo $class ?>" nowrap="nowrap">
							<?php check_box($prefix.'_delete['.$data['msgid'].']', '1', false, 'changeStatus('.$data['msgid'].')'); ?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="trashIcon[<?php echo $data['msgid'];?>]" class="trash" onclick="changeStatus('<?php echo $data['msgid'];?>', 'delete')" />
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
			?>
			</table>
			</div>
			<?php if ($write_access) { ?>
				<div id="sq_message_body"></div>
			<?php } else { ?>
				<div id="sq_message_body_disabled">Acquire the lock on the inbox to read messages.</div>
			<?php } ?>
			<?php
			}

		return true;

	}//end paintMail()


	/**
	* Processes the inbox interface
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processMail(&$asset, &$o, $prefix)
	{
		$msgs   = (isset($_POST[$prefix.'_messages'])) ? array_keys($_POST[$prefix.'_messages']) : Array();
		$read   = (isset($_POST[$prefix.'_mark_as_read'])) ? array_keys($_POST[$prefix.'_mark_as_read']) : Array();
		$delete = (isset($_POST[$prefix.'_delete'])) ? array_keys($_POST[$prefix.'_delete']) : Array();

		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id);

		if (!empty($messages)) {
			foreach ($messages as $data) {
				if (!in_array($data['msgid'], $msgs)) continue;
				$message = $ms->getMessageById($data['msgid']);

				// change message status (read/unread)
				if (in_array($data['msgid'], $read) && $data['status'] != SQ_MSG_READ) {
					if (!$message->updateStatus(SQ_MSG_READ)) {
						trigger_error('Status of message '.$data['msgid'].' was not updated', E_USER_WARNING);
					}
				} else if (!in_array($data['msgid'], $read) && $data['status'] == SQ_MSG_READ) {
					if (!$message->updateStatus(SQ_MSG_UNREAD)) {
						trigger_error('Status of message '.$data['msgid'].' was not updated', E_USER_WARNING);
					}
				}

				// delete messages
				if (in_array($data['msgid'], $delete) && $data['status'] != SQ_MSG_DELETED) {
					if (!$message->updateStatus(SQ_MSG_DELETED)) {
						trigger_error('Message '.$data['msgid'].' was not deleted', E_USER_WARNING);
					}
				}
			}
		}

		$o->addOnLoad('if (parent.frames["sidenav"] && parent.frames["sidenav"].refresh_internal_messages) parent.frames["sidenav"].refresh_internal_messages();');

		return true;

	}//end processMail()


	/**
	* Prints the trash interface
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTrash(&$asset, &$o, $prefix)
	{
		$user = &$asset->_getUser();

		$priority = Array('', 'Highest', 'High', 'Normal', 'Low', 'Lowest');

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_DELETED), Array(), null, null, 'name');
		$write_access = $asset->writeAccess('mail');

			?>
			<style type="text/css">
				@import url(<?php echo sq_web_path('data')?>/asset_types/inbox/css/inbox.css.php);
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
			echo '<div style="text-align: center">Your trash is empty.</div>';
			return false;
		} else {
		?>
			<div id="inbox_container">

			<table border="0" cellspacing="0" cellpadding="1" id="inbox_table">
			<tr>
				<td class="sq-backend-table-header">FROM</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_recover', '1', false, "check_all(document.main_form, '{$prefix}_recover', this.checked);"); ?>
						RECOVER
					</td>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_purge', '1', false, "check_all(document.main_form, '{$prefix}_purge', this.checked);"); ?>
						PURGE
					</td>
					<?php
				}
				?>
			</tr>
			<?php
			$alt = false;
			$num_messages = count($messages);

			for ($i = 0; $i < $num_messages; $i++) {

				$data = $messages[$i];

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);

				$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
				$body = str_replace("\"", '&quot;', $body);
				?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16, '', $priority[$data['priority']].' priority') ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="<?php echo $class ?>">
						&nbsp;<span class="sq-backend-table-cell" onclick="showBody('<?php echo $data['msgid'];?>', this);" style="cursor: pointer"><?php echo $data['subject'];?></span>
					</td>
					<td class="<?php echo $class ?>" style="white-space: nowrap;">
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>

					<?php
					if ($write_access) {
						?>
						<td class="<?php echo $class ?>" nowrap>
							<?php
							if ($asset->writeAccess('mail')) check_box($prefix.'_recover['.$data['msgid'].']');
							else echo '&nbsp;';
							?>
						</td>
						<td class="<?php echo $class ?>" nowrap>
							<?php check_box($prefix.'_purge['.$data['msgid'].']'); ?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
			?>
			</table>
			</div>
			<div id="sq_message_body"></div>
		<?php }
		?>
		<?php
		return true;

	}//end paintTrash()


	/**
	* Processes the trash interface
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processTrash(&$asset, &$o, $prefix)
	{
		$msgs    = (isset($_POST[$prefix.'_messages'])) ? array_keys($_POST[$prefix.'_messages']) : Array();
		$recover = (isset($_POST[$prefix.'_recover'])) ? array_keys($_POST[$prefix.'_recover']) : Array();
		$purge   = (isset($_POST[$prefix.'_purge'])) ? array_keys($_POST[$prefix.'_purge']) : Array();

		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id);

		if (!empty($recover)) {
			foreach ($recover as $messageid) {
				// recover messages to inbox
				$message = $ms->getMessageById($messageid);
				if (!$message->updateStatus(SQ_MSG_READ)) {
					trigger_error('Message '.$messageid.' was not recovered', E_USER_WARNING);
				}
			}
		}

		if (!empty($purge)) {
			foreach ($purge as $messageid) {
				// delete messsage FOREVER
				$message = $ms->getMessageById($messageid);
				if (!$message->delete()) {
					trigger_error('Message '.$messageid.' was not deleted', E_USER_WARNING);
				}
			}
		}

		return true;

	}//end processTrash()


	/**
	* Processes the new message interface
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewMessage(&$asset, &$o, $prefix)
	{
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		// send the message
		$noaction = true;
		if (isset($_POST[$prefix.'_To'])) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			$recepients = Array();
			foreach ($_POST[$prefix.'_To'] as $key => $sender) {
				$recepients[] = $sender['assetid'];
			}

			$message = $ms->newMessage($recepients, $_POST[$prefix.'_Subject'], $_POST[$prefix.'_Content'], 'hipo.some');
			$message->priority = $_POST[$prefix.'_Priority'];
			$message->send();

			foreach($recepients as $key => $sender) {
				if (empty($sender)) {
					continue;
				}
				$noaction = true;
				$message = $ms->newMessage(Array($userid), $_POST[$prefix.'_Subject'], $_POST[$prefix.'_Content'], 'hipo.sent');
				$message->from = $sender;
				$message->status = 'R';
				$message->priority = $_POST[$prefix.'_Priority'];
				$message->send();
			}

		}
		if ($noaction) {
			echo '<!-- ###No Action### -->';
		}

	}


}//end class

function sq_print_icon($path, $width, $height, $alt = '', $title = '')
{
	echo '<img src="'.sq_web_path('lib').'/web/images/blank.gif" alt="'.$alt.'" title="'.$title.'" style="background:url('.$path.'); background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''.$path.'\', sizingMethod=\'crop\');width: '.$width.'px; height: '.$height.'px" />';
}

?>
