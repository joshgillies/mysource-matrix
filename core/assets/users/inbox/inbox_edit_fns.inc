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
* @package Resolve_Packages
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
		$messages = $ms->getMessages($user->id, Array(SQ_MSG_UNREAD, SQ_MSG_READ), null, null, 'name');

		if (empty($messages)) {
			echo 'You have no messages';
		} else {
			?>
			<table border="0" cellspacing="1" cellpadding="3" width="100%" bgcolor="#DDDDDD">
			<tr>
				<td class="sq-backend-message-header">From</td>
				<td class="sq-backend-message-header">Subject</td>
				<td class="sq-backend-message-header">Sent</td>
				<td class="sq-backend-message-header">Priority</td>
				<td class="sq-backend-message-header">Read</td>
				<td>&nbsp;</td>
			</tr>
			<?php
			foreach ($messages as $data) {
				?>
				<tr>
					<td class="sq-backend-message-item">
						<a name="msg_<?php echo $data['messageid'];?>"></a>
						<?php echo $data['from_name'];?>
					</td>
					<td class="sq-backend-message-item">
						<?php echo $data['subject'];?>
					</td>
					<td class="sq-backend-message-item" style="white-space: nowrap;">
						<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="sq-backend-message-item" style="text-align: center;">
						<?php echo $data['priority']; ?>
					</td>
					<td class="sq-backend-message-item" style="text-align: center;">
						<input type="checkbox" name="<?php echo $prefix.'_mark_as_read['.$data['messageid'].']'?>" value="1"<?php echo (($data['status'] == SQ_MSG_READ) ? ' checked' : '')?>>
					</td>
					<td>
						<input type="submit" name="<?php echo $prefix.'_delete['.$data['messageid'].']'?>" value="x">
					</td>
				</tr>
				<tr>
					<td class="sq-backend-message-item" colspan="6">
						<?php echo $data['body'];?>
					</td>
				</tr>
				<tr>
					<td colspan="6"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<?php
			}
			?>
			</table>
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
				// change message status (read/unread)
				if (in_array($data['messageid'], $read) && $data['status'] != SQ_MSG_READ) {
					if (!$ms->updateMessageStatus($data['messageid'], SQ_MSG_READ)) {
						trigger_error('Status of message '.$data['messageid'].' was not updated', E_USER_WARNING);
					}
				} else if (!in_array($data['messageid'], $read) && $data['status'] == SQ_MSG_READ) {
					if (!$ms->updateMessageStatus($data['messageid'], SQ_MSG_UNREAD)) {
						trigger_error('Status of message '.$data['messageid'].' was not updated', E_USER_WARNING);
					}
				}

				// delete messages
				if (in_array($data['messageid'], $delete) && $data['status'] != SQ_MSG_DELETED) {
					if (!$ms->updateMessageStatus($data['messageid'], SQ_MSG_DELETED)) {
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
		$messages = $ms->getMessages($user->id, Array(SQ_MSG_DELETED), null, null, 'name');

		if (empty($messages)) {
			echo 'Your trash is empty';
		} else {
			?>
			<table border="0" cellspacing="1" cellpadding="3" width="100%" bgcolor="#DDDDDD">
			<tr>
				<td class="sq-backend-message-header">From</td>
				<td class="sq-backend-message-header">Subject</td>
				<td class="sq-backend-message-header">Sent</td>
				<td class="sq-backend-message-header">Priority</td>
				<td class="sq-backend-message-header">Recover</td>
				<td>&nbsp;</td>
			</tr>
			<?php
			foreach ($messages as $data) {
				?>
				<tr>
					<td class="sq-backend-message-item">
						<a name="msg_<?php echo $data['messageid'];?>"></a>
						<?php echo $data['from_name'];?>
					</td>
					<td class="sq-backend-message-item">
						<?php echo $data['subject'];?>
					</td>
					<td class="sq-backend-message-item" style="white-space: nowrap;">
						<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="sq-backend-message-item" style="text-align: center;">
						<?php echo $data['priority']; ?>
					</td>
					<td class="sq-backend-message-item" style="text-align: center;">
						<input type="checkbox" name="<?php echo $prefix.'_recover['.$data['messageid'].']'?>" value="1">
					</td>
					<td>
						<input type="submit" name="<?php echo $prefix.'_purge['.$data['messageid'].']'?>" value="x">
					</td>
				</tr>
				<tr>
					<td class="sq-backend-message-item" colspan="6">
						<?php echo $data['body'];?>
					</td>
				</tr>
				<tr>
					<td colspan="6"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td>
				</tr>
				<?php
			}
			?>
			</table>
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
				if (!$ms->updateMessageStatus($messageid, SQ_MSG_READ)) {
					trigger_error('Message '.$messageid.' was not recovered', E_USER_WARNING);
				}
			}
		}

		if (!empty($purge)) {
			foreach ($purge as $messageid) {
				// delete messsage FOREVER
				if (!$ms->deleteMessage($messageid)) {
					trigger_error('Message '.$messageid.' was not deleted', E_USER_WARNING);
				}
			}
		}

		return true;
	}//end processTrash()

}//end class
?>
