<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: internal_message.inc,v 1.21 2003/10/10 04:47:57 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


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
	* Type of the message eg. asset.workflow
	* @var string
	*/
	var $type = '';

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
	* An array of params that may be used by the message depending on its type
	* @var array
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
	* @param int	$messageid	id of the message the you want to get
	*
	* @return boolean
	* @access public
	*/
	function load($messageid)
	{
		if (!$messageid) return false;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql   = 'SELECT messageid, userto, userfrom, subject, body, type, sent, priority, status, parameters
				    FROM '.SQ_TABLE_RUNNING_PREFIX.'internal_message ';
		$where = 'messageid = '.$db->quote($messageid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		
		$result = $db->getRow($sql.$where);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->id         = $result['messageid'];
		$this->to         = Array($result['userto']);
		$this->from       = $result['userfrom'];
		$this->subject    = $result['subject'];
		$this->body       = $result['body'];
		$this->type       = $result['type'];
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
		// lets work out what action should be conducted on this type of message
		// ie. should it be sent to the flash, logged, mailed etc
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		foreach (Array('show_in_flash', 'log_to_file', 'log_to_db', 'send_mail') as $main_type_code) {
			if (!isset($ms->_tmp['message_type_logs'][$main_type_code][$this->type])) {
				$def_name_w = 'SQ_MS_'.strtoupper($main_type_code).'_WHITE_LIST';
				$def_name_b = 'SQ_MS_'.strtoupper($main_type_code).'_BLACK_LIST';
				$types_w = explode("\n", constant($def_name_w));
				$types_b = explode("\n", constant($def_name_b));

				$type_code_list = explode('.', $this->type);

				if (in_array($this->type, $types_w)) {
					// this exact type is white listed
					$allow_log = true;
				} else if (in_array($this->type, $types_b)) {
					// this exact type is black listed
					$allow_log = false;
				} else {
					$allow_log = false;
					if (in_array('*', $types_w) && !in_array('*', $types_b)) {
						// all messages of this type are white listed and not black listed
						$allow_log = true;
					}

					if (!$allow_log) {
						// keep checking the virtual types to see if we should log this
						$check_code = '';
						foreach ($type_code_list as $type_code) {
							if (!empty($check_code)) {
								$check_code = trim($check_code, '.* ');
								$check_code .= '.';
							}
							$check_code .= $type_code.'.*';
							if (in_array($check_code, $types_w) && !in_array($check_code, $types_b)) {
								// this virtual type is white listed and not black listed
								$allow_log = true;
								break;
							}
						}
					}
				}

				// cache this for this script execution
				$ms->_tmp['message_type_logs'][$main_type_code][$this->type] = $allow_log;
			}

			$$main_type_code = $ms->_tmp['message_type_logs'][$main_type_code][$this->type];

		}//end foreach


		$user_from = null;
		$from_name = '';
		if ($this->from) {
			$user_from = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->from);
			if (!is_null($user_from)) {
				$from_name = $user_from->name;
			} else {
				$from_name = 'Unkown User #'.$this->from;
			}
		} else {
			// message from the system
			$from_name = SQ_SYSTEM_SHORT_NAME.' System';
		}

		$param_string = '';
		foreach ($this->parameters as $key => $value) $param_string .= "$key:$value;";


		  ///////////////////////////
		 // LOG MESSAGES TO FLASH //
		///////////////////////////
		if ($show_in_flash) {

			// we require the file to use the message constants that are defined at the top
			require_once(SQ_INCLUDE_PATH.'/backend_outputter.inc');

			// add the message to the session so the backend outputter picks it up next time
			// it prints the messages - we cant reply on the backend outputter being us=
			$_SESSION['backend_outputter_msgs'][] = Array('type' => SQ_BO_MSG_NOTICE, 'msg' => $this->body);

		}//end if logging message to flash


		  //////////////////////////////
		 // LOG MESSAGES TO LOG FILE //
		//////////////////////////////
		if ($log_to_file) {

			if (!$fp = fopen (SQ_DATA_PATH.'/private/logs/system.log', 'a')) {
				trigger_error('Unable to open log file for writing ['.SQ_DATA_PATH.'/private/logs/system.log'.']', E_USER_WARNING);
			}

			$log_line = '['.$this->sent.'] ['.$this->from.':'.$from_name.'] ['.$this->type.' - '.$param_string.'] ('.$this->subject.') - '.str_replace("\n", '<newline/>', $this->body)."\n";

			if (!fwrite($fp, $log_line)) {
				trigger_error('Unable to write log to file ['.SQ_DATA_PATH.'/private/logs/system.log'.']', E_USER_WARNING);
			}

		}//end if logging message to file


		if ($log_to_db || $send_mail) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			// work out the email address the the email will be sent from
			$from_email = SQ_CONF_DEFAULT_EMAIL;
			$from_string = '';
			if ($this->from) {
				if (!is_null($user_from)) {
					$from_string = $user_from->name.' <'.$user_from->attr('email').'>';
					$from_email = $user_from->attr('email');
				}
			}
			if (empty($from_email)) $from_email = 'webmaster@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']);
			if (empty($from_string)) $from_string = '"'.SQ_SYSTEM_SHORT_NAME.' System" <'.$from_email.'>';
		}


		  //////////////////////////////
		 // LOG MESSAGES TO DATABASE //
		//////////////////////////////
		if ($log_to_db) {

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// log a message for the system so it is not deleted
			$messageid = $db->nextId('sq_sequence_internal_message');
			if (DB::isError($messageid)) {
				trigger_error($messageid->getMessage().'<br/>'.$messageid->getUserInfo(), E_USER_ERROR);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$values = Array('messageid'  => $messageid,
							'userto'     => 0,
							'userfrom'   => $this->from,
							'subject'    => $this->subject,
							'body'       => $this->body,
							'type'       => $this->type,
							'priority'   => $this->priority,
							'status'     => $this->status,
							'sent'       => $this->sent,
							'parameters' => $param_string
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('internal_message', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}//end if logging message to db


		  ///////////////////////////
		 // SEND MESSAGE TO USERS //
		///////////////////////////
		if ($send_mail) {

			$send_to = $this->expandUsersTo();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// send each user a message - via email and into their internal message inbox
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
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				$values = Array('messageid'  => $messageid,
								'userto'     => $userid,
								'userfrom'   => $this->from,
								'subject'    => $this->subject,
								'body'       => $this->body,
								'type'       => $this->type,
								'priority'   => $this->priority,
								'status'     => $this->status,
								'sent'       => $this->sent,
								'parameters' => $param_string
								);
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

		}//end if sending message via email

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
		$info = $ms->getMessages($this->to[0], $this->type, Array(), Array($this->id), $this->from, null, 'name');
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
		if (empty($this->to)) {
			// this message is not going to anyone so lets see if the type
			// can shead some light on who this is supposed to be going to
			$type = explode('.', $this->type);
			switch ($type[0]) {

				case 'asset' :
					if (isset($type[1])) {
						switch ($type[1]) {
							case 'linking' :
								if (!isset($this->parameters['majorid'])) {
									trigger_error('Failed sending internal message, no majorid specified (type: '.$this->type.')', E_USER_WARNING);
								}
								if (!isset($this->parameters['minorid'])) {
									trigger_error('Failed sending internal message, no minorid specified (type: '.$this->type.')', E_USER_WARNING);
								}

								$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->parameters['majorid']);
								if (is_null($major)) {
									trigger_error('Failed sending internal message, invalid majorid ['.$this->parameters['majorid'].'] specified (type: '.$this->type.')', E_USER_WARNING);
								}
								$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->parameters['minorid']);
								if (is_null($minor)) {
									trigger_error('Failed sending internal message, invalid minorid ['.$this->parameters['minorid'].'] specified (type: '.$this->type.')', E_USER_WARNING);
								}
							break(2);
						}
					}
					if (!isset($this->parameters['assetid'])) {
						// cant send an asset message if we dont know which asset is affected
						trigger_error('Failed sending internal message, no assetid specified (type: '.$this->type.')', E_USER_WARNING);
					}
					$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->parameters['assetid']);
					if (is_null($asset)) {
						// cant send an asset message if we dont know which asset is affected
						trigger_error('Failed sending internal message, invalid assetid ['.$this->parameters['assetid'].'] specified (type: '.$this->type.')', E_USER_WARNING);
					}

					$this->to = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset->id, SQ_PERMISSION_ADMIN, true, false);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					break;

				case 'config' :
				case 'hipo'   :
					// send config changes to the DEFAULT and TECH emails and to the Root User
					$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
					$this->to = Array(0, $root_user->id);
					break;

			}//end switch

		}// end if

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
