<?php

/**
* Internal_Message
*
* Purpose
*    An internal message to be sent around the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Internal_Message extends MySource_Object
{

	/**
	* The unique ID of the message
	* @var int
	*/
	var $id = 0;

	/**
	* An array of users or user groups to send the message to
	* @var Array(int)
	*/
	var $to = Array();

	/**
	* The userid of the sender
	* Userid will be ZERO if the sender is the MySource system
	* @var int
	*/
	var $from = 0;

	/**
	* When the message was sent (if it has been sent)
	* @var string
	*/
	var $sent = 0;

	/**
	* The subject of the message
	* @var string
	*/
	var $subject = '';

	/**
	* The main body of the message
	* @var string
	*/
	var $body = '';

	/**
	* The message priority (see constants for SQ_MSG_PRIOIRTY_*)
	* @var int
	*/
	var $priority = SQ_MSG_PRIORITY_NORMAL;

	/**
	* The status of the message (see constants for SQ_MSG_*)
	* @var string
	*/
	var $status = SQ_MSG_UNREAD;

	/**
	* An array of parameters to use for special messages
	* @var Array()
	*/
	var $parameters = Array();


	/**
	* Constructor
	*
	*/
	function Internal_Message()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Loads the information about this message
	*
	* @param int	$messageid		id of the message the you want to get
	*
	* @return boolean
	* @access public
	*/
	function load($messageid)
	{
		if (!$messageid) return false;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'messageid = '.$db->quote($messageid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT messageid, userto, userfrom, subject, body, sent, priority, status, parameters 
				FROM '.SQ_TABLE_RUNNING_PREFIX.'internal_message '.$where;
		
		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->id         = $result['messageid'];
		$this->to         = Array($result['userto']);
		$this->from       = $result['userfrom'];
		$this->subject    = $result['subject'];
		$this->body       = $result['body'];
		$this->sent       = iso8601_ts($result['sent']);
		$this->priority   = $result['priority'];
		$this->status     = $result['status'];
		$this->parameters = $result['parameters'];

		return true;

	}//end load()

	
	/**
	* Sends an internal message
	*
	* @return boolean
	* @access public
	*/
	function send()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$now = time();
		$parameters = serialize($this->parameters);

		$send_to = $this->expandUsersTo();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$from_email = SQ_CONF_DEFAULT_EMAIL;
		$from_string = '';
		if ($this->from) {
			$user_from = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->from);
			if (!is_null($user_from)) {
				$from_string = $user_from->name.' <'.$user_from->attr('email').'>';
				$from_email = $user_from->attr('email');
			}
		}
		if (empty($from_email)) $from_email = 'webmaster@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']);
		if (empty($from_string)) $from_string = '"'.SQ_SYSTEM_SHORT_NAME.' System" <'.$from_email.'>';

		foreach ($send_to as $userid) {
			if (!$userid) {
				// if the userid is empty, we are sending a message to the system
				// so we send to the default email and tech email
				$default = SQ_CONF_DEFAULT_EMAIL;
				$tech = SQ_CONF_TECH_EMAIL;
				$to_email = '';

				if (!empty($default)) $to_email .= $default;
				if (!empty($tech)) {
					if (!empty($to_email)) $to_email .= ',';
					$to_email .= $tech;
				}

				if (!empty($to_email)) mail($to_email, $this->subject, $this->body, "From: ".$from_string, "-f$from_email");

				continue;
			}

			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
			if (is_null($user)) continue;

			// log the internal message
			$messageid = $db->nextId('sq_sequence_internal_message');
			if (DB::isError($messageid)) {
				trigger_error($messageid->getMessage().'<br/>'.$messageid->getUserInfo(), E_USER_ERROR);
			}

			$values = Array('messageid'  => $messageid,
							'userto'     => $userid,
							'userfrom'   => $this->from,
							'subject'    => $this->subject,
							'body'       => $this->body,
							'priority'   => $this->priority,
							'status'     => $this->status,
							'sent'       => ts_iso8601($now),
							'parameters' => $parameters);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('internal_message', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// now send an email as well
			require_once SQ_FUDGE_PATH.'/general/www.inc';
			$to_email = trim($user->attr('email'));
			if (!empty($to_email) && valid_email($to_email)) {
				mail($to_email, $this->subject, $this->body, "From: ".$from_string, "-f$from_email");
			}

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end send()


	/**
	* Called by the design to print the body of this message
	*
	* @access public
	*/
	function printBody()
	{
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$info = $ms->getMessages($this->to[0], Array(), Array($this->id), $this->from, null, 'name');
		$info = $info[0];
		?>
		<html>
			<head>
				<style>
					.sq-message-item, .sq-message-item-msg {
						color:           #000000;
						font-family:     Arial, Verdana, Helvetica, sans-serif;
						font-size:       10px;
						text-decoration: none;
					}
					.sq-message-item-msg {
						color:           #000000;
						font-family:     courier new, fixed;
						font-size:       11px;
						white-space:     pre;
						text-decoration: none;
					}
				</style>
			</head>
			<body bgcolor="#FFFFFF">
				<table border="0" cellspacing="1" cellpadding="2" width="100%" height="100%">
					<tr>
						<td valign="top" class="sq-message-item"><b>SUBJECT:</b></td>
						<td valign="top" class="sq-message-item" width="100%"><?php echo $info['subject']; ?></td>
					</tr>
					<tr>
						<td valign="top" class="sq-message-item"><b>FROM:</b></td>
						<td valign="top" class="sq-message-item" width="100%"><?php echo $info['from_name'] ?></td>
					</tr>
					<tr>
						<td valign="top" class="sq-message-item"><b>DATE:</b></td>
						<td valign="top" class="sq-message-item" width="100%"><?php echo readable_datetime($info['sent']); ?></td>
					</tr>
					<tr height="100%">
						<td valign="top" class="sq-message-item-msg" bgcolor="#FFFFFF" width="100%" colspan="2"><br/><pre><?php echo $info['body']; ?></pre></td>
					</tr>
				</table>
			</body>
		</html>
		<?php

	}// end printBody()


	/**
	* Returns an array of users this message will be sent to
	* That is, it expands the groups ids and returns a unique array of userids
	*
	* @return Array(int)
	* @access public
	*/
	function expandUsersTo()
	{
		$send_to = Array();
		if (in_array('0', $this->to)) {
			// this message is being sent to the system
			// so we need to keep the ZERO id in the array
			$send_to[] = 0;
		}
		$to_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo($this->to, Array('user', 'user_group'), false);
		foreach ($to_types as $assetid => $types) {
			if (in_array('user_group', $types)) {
				$group = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if ($group->id) $send_to = array_merge($send_to, $GLOBALS['SQ_SYSTEM']->am->getChildren($group->id, Array('user'), false));
			} else {
				$send_to[] = $assetid;
			}
		}
		return array_unique($send_to);

	}//end expandUsersTo()


	/**
	* Deletes an internal message FOREVER - be careful
	*
	* @return boolean
	* @access public
	*/
	function delete()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$where_cond = 'messageid='.$db->quote($this->id);
		return $GLOBALS['SQ_SYSTEM']->rollbackDelete('internal_message', $where_cond);
		
	}//end delete()


	/**
	* Update the status of a message
	*
	* @param string	$status	the new status (see constants for SQ_MSG_*)
	*
	* @return boolean
	* @access public
	*/
	function updateStatus($status)
	{
		if (!$this->id) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$where = 'messageid='.$db->quote($this->id);
		$values = Array('status' => $db->quote($status));
		return $GLOBALS['SQ_SYSTEM']->rollbackUpdate('internal_message', $values, $where);

	}//end updateStatus()

}//end class

?>