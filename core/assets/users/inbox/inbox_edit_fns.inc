<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Inbox_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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

		return $message->printBody();

	}//end paintMessage()


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

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_UNREAD, SQ_MSG_READ), Array(), null, null, 'name');

		if (empty($messages)) {
			echo 'You have no messages';
		} else {
			?>
			<script type="text/javascript" language="Javascript">
				function showMessageBody(messageid) {
					var iframe = document.getElementById('sq_message_body');
					if (iframe == null) return;
					
					var url='<?php echo $_SERVER['PHP_SELF']; ?>' + '?a=<?php echo $asset->id; ?>&messageid=' + messageid;
					iframe.src = url;
				}
			</script>

			<table border="0" cellspacing="0" cellpadding="1" width="100%">
			<tr>
				<td colspan="5" background="<?php echo sq_web_path('lib'); ?>/web/images/section_dots.gif" style="height:1px; padding:0px; margin:0px;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">FROM</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<td class="sq-backend-table-header">READ</td>
				<td class="sq-backend-table-header">&nbsp;</td>
			</tr>
			<?php
			$alt = false;
			$num_printed = 0;
			$num_per_page = 10;
			$num_messages = count($messages);
			$start_printing_at = 0;
			if (isset($_REQUEST['sq_internal_message_inbox_start'])) $start_printing_at = (int) $_REQUEST['sq_internal_message_inbox_start'];

			for ($i = 0; $i < $num_messages; $i++) {
				if ($i < $start_printing_at) {
					$i++;
					continue;
				}

				$data = $messages[$i];

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the time of day icon
				if (date('A',$data['sent']) == 'AM') $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_day.png';
				else $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_night.png';
				
				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
				?>
				<tr>
					<td class="<?php echo $class ?>" nowrap>
						<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_<?php echo $data['priority'];?>.png", "16", "16", "");</script>
						<script language="Javascript">sqPrintIcon("<?php echo $sender_img; ?>", "16", "16", "");</script>
						<a name="msg_<?php echo $data['messageid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
					</td>
					<td class="<?php echo $class ?>">
						<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_subject.png", "16", "16", "");</script>
						&nbsp;<a href="#" onClick="Javascript: showMessageBody('<?php echo $data['messageid'];?>'); return false;" style="text-decoration:none;"><?php echo $data['subject'];?></a>
					</td>
					<td class="<?php echo $class ?>" nowrap>
						<script language="Javascript">sqPrintIcon("<?php echo $time_img; ?>", "16", "16", "");</script>
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="<?php echo $class ?>" nowrap>
						<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_<?php echo (($data['status'] == SQ_MSG_UNREAD) ? 'un' : ''); ?>read.png", "16", "16", "");</script>&nbsp;
						<?php
						if ($asset->writeAccess()) {
							?>
							<input type="checkbox" name="<?php echo $prefix.'_mark_as_read['.$data['messageid'].']'?>" value="1"<?php echo (($data['status'] == SQ_MSG_READ) ? ' checked' : '')?>>
							<?php
						}
						?>
					</td>
					<td class="<?php echo $class ?>" nowrap>
						<?php
						if ($asset->writeAccess()) {
							?>
							<input type="checkbox" name="<?php echo $prefix.'_delete['.$data['messageid'].']'?>" value="1">
							<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/delete.png", "16", "16", "");</script>
							&nbsp;&nbsp;Delete<?php
						} else {
							echo '&nbsp;';
						}
						?>
					</td>
				</tr>
				<tr>
					<td colspan="5"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<tr>
					<td colspan="5" background="<?php echo sq_web_path('lib'); ?>/web/images/section_dots.gif" style="height:1px; padding:0px; margin:0px;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<tr>
					<td colspan="5"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<?php
				$num_printed++;
				if ($num_printed >= $num_per_page) break;
			}
			?>
				<tr>
					<td colspan="5" align="center" class="sq-backend-table-cell">
					<input type="hidden" name="sq_internal_message_inbox_start" value="<?php echo $start_printing_at; ?>">
					<?php
					if ($start_printing_at > 0) {
						?><a style="text-decoration: none;" href="#" onClick="Javascript: document.main_form.sq_internal_message_inbox_start.value='<?php echo $start_printing_at - $num_per_page; ?>'; sqSubmitEditForm(true); return false;"><?php
					} else {
						?><span style="color: #BBBBBB"><?php
					}
					?>
					<< Previous Page
					<?php
					if ($start_printing_at > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp;<b>Page <?php echo round(($start_printing_at + $num_per_page) / $num_per_page); ?> of <?php echo round(($num_messages / $num_per_page) + 0.5); ?></b>&nbsp;
					
					<?php
					if (($start_printing_at + $num_per_page) < $num_messages) {
						?><a style="text-decoration: none;" href="#" onClick="Javascript: document.main_form.sq_internal_message_inbox_start.value='<?php echo $start_printing_at + $num_per_page; ?>'; sqSubmitEditForm(true); return false;"><?php
					} else {
						?><span style="color: #BBBBBB"><?php
					}
					?>
					Next Page >>
					<?php
					if (($start_printing_at + $num_per_page) < $num_messages) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					</td>
				</tr>
			</table>
			<p>
			<iframe src="" id="sq_message_body" width="100%" height="300" frameborder="1"></iframe>
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
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processMail(&$asset, &$o, $prefix, $info=Array())
	{
		$read   = (isset($_POST[$prefix.'_mark_as_read'])) ? array_keys($_POST[$prefix.'_mark_as_read']) : Array();
		$delete = (isset($_POST[$prefix.'_delete'])) ? array_keys($_POST[$prefix.'_delete']) : Array();

		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id);

		if (!empty($messages)) {
			foreach ($messages as $data) {
				$message = $ms->getMessageById($data['messageid']);

				// change message status (read/unread)
				if (in_array($data['messageid'], $read) && $data['status'] != SQ_MSG_READ) {
					if (!$message->updateStatus(SQ_MSG_READ)) {
						trigger_error('Status of message '.$data['messageid'].' was not updated', E_USER_WARNING);
					}
				} else if (!in_array($data['messageid'], $read) && $data['status'] == SQ_MSG_READ) {
					if (!$message->updateStatus(SQ_MSG_UNREAD)) {
						trigger_error('Status of message '.$data['messageid'].' was not updated', E_USER_WARNING);
					}
				}

				// delete messages
				if (in_array($data['messageid'], $delete) && $data['status'] != SQ_MSG_DELETED) {
					if (!$message->updateStatus(SQ_MSG_DELETED)) {
						trigger_error('Message '.$data['messageid'].' was not deleted', E_USER_WARNING);
					}
				}
			}
		}

		$o->addOnLoad('if (top.sidenav && top.sidenav.refresh_internal_messages) top.sidenav.refresh_internal_messages();');

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

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_DELETED), Array(), null, null, 'name');

		if (empty($messages)) {
			echo 'Your trash is empty';
		} else {
			?>
			<script type="text/javascript" language="Javascript">
				function showMessageBody(messageid) {
					var iframe = document.getElementById('sq_message_body');
					if (iframe == null) return;
					
					var url='<?php echo $_SERVER['PHP_SELF']; ?>' + '?a=<?php echo $asset->id; ?>&messageid=' + messageid;
					iframe.src = url;
				}
			</script>

			<table border="0" cellspacing="0" cellpadding="1" width="100%">
			<tr>
				<td colspan="5" background="<?php echo sq_web_path('lib'); ?>/web/images/section_dots.gif" style="height:1px; padding:0px; margin:0px;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
			</tr>
			<tr>
				<td class="sq-backend-table-header">FROM</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<td class="sq-backend-table-header"><?php echo ($asset->writeAccess()) ? 'RECOVER' : '&nbsp;';?></td>
				<td class="sq-backend-table-header">&nbsp;</td>
			</tr>
			<?php
			$alt = false;
			$num_printed = 0;
			$num_per_page = 10;
			$num_messages = count($messages);
			$start_printing_at = 0;
			if (isset($_REQUEST['sq_internal_message_trash_start'])) $start_printing_at = (int) $_REQUEST['sq_internal_message_trash_start'];

			for ($i = 0; $i < $num_messages; $i++) {
				if ($i < $start_printing_at) {
					$i++;
					continue;
				}

				$data = $messages[$i];

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the time of day icon
				if (date('A',$data['sent']) == 'AM') $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_day.png';
				else $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_night.png';
				
				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);

				?>
				<tr>
					<td class="<?php echo $class ?>" nowrap>
						<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_<?php echo $data['priority'];?>.png", "16", "16", "");</script>
						<script language="Javascript">sqPrintIcon("<?php echo $sender_img; ?>", "16", "16", "");</script>
						<a name="msg_<?php echo $data['messageid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
					</td>
					<td class="<?php echo $class ?>">
						<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_subject.png", "16", "16", "");</script>
						&nbsp;<a class="sq-backend-table-cell" href="#" onClick="Javascript: showMessageBody('<?php echo $data['messageid'];?>'); return false;" style="text-decoration: none;"><?php echo $data['subject'];?></a>
					</td>
					<td class="<?php echo $class ?>" style="white-space: nowrap;">
						<script language="Javascript">sqPrintIcon("<?php echo $time_img; ?>", "16", "16", "");</script>
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="<?php echo $class ?>" nowrap>
						<?php
						if ($asset->writeAccess()) {
							?><input type="checkbox" name="<?php echo $prefix.'_recover['.$data['messageid'].']'?>" value="1"><?php
						} else {
							echo '&nbsp;';
						}
						?>
					</td>
					<td class="<?php echo $class ?>" nowrap>
						<?php
						if ($asset->writeAccess()) {
							?>
							<input type="checkbox" name="<?php echo $prefix.'_purge['.$data['messageid'].']'?>" value="1">
							<script language="Javascript">sqPrintIcon("<?php echo sq_web_path('lib'); ?>/web/images/icons/delete.png", "16", "16", "");</script>
							&nbsp;&nbsp;Purge<?php
						} else {
							echo '&nbsp;';
						}
						?>
					</td>
				</tr>
				<tr>
					<td colspan="5"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<tr>
					<td colspan="5" background="<?php echo sq_web_path('lib'); ?>/web/images/section_dots.gif" style="height:1px; padding:0px; margin:0px;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<tr>
					<td colspan="5"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<?php
				$num_printed++;
				if ($num_printed >= $num_per_page) break;
			}
			?>
				<tr>
					<td colspan="5" align="center" class="sq-backend-table-cell">
					<input type="hidden" name="sq_internal_message_trash_start" value="<?php echo $start_printing_at; ?>">
					<?php
					if ($start_printing_at > 0) {
						?><a style="text-decoration: none;" href="#" onClick="Javascript: document.main_form.sq_internal_message_trash_start.value='<?php echo $start_printing_at - $num_per_page; ?>'; sqSubmitEditForm(true); return false;"><?php
					} else {
						?><span style="color: #BBBBBB"><?php
					}
					?>
					<< Previous Page
					<?php
					if ($start_printing_at > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp;<b>Page <?php echo round(($start_printing_at + $num_per_page) / $num_per_page); ?> of <?php echo round(($num_messages / $num_per_page) + 0.5); ?></b>&nbsp;
					
					<?php
					if (($start_printing_at + $num_per_page) < $num_messages) {
						?><a style="text-decoration: none;" href="#" onClick="Javascript: document.main_form.sq_internal_message_trash_start.value='<?php echo $start_printing_at + $num_per_page; ?>'; sqSubmitEditForm(true); return false;"><?php
					} else {
						?><span style="color: #BBBBBB"><?php
					}
					?>
					Next Page >>
					<?php
					if (($start_printing_at + $num_per_page) < $num_messages) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					</td>
				</tr>
			</table>
			<p>
			<iframe src="" id="sq_message_body" width="100%" height="300" frameborder="1"></iframe>
			<?php
		}
		return true;

	}//end paintTrash()


	/**
	* Processes the trash interface
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processTrash(&$asset, &$o, $prefix, $info=Array())
	{
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

}//end class

?>