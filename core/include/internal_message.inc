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
* $Id: internal_message.inc,v 1.106 2013/07/25 23:23:49 lwright Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/www.inc';

/**
* Internal_Message
*
* Purpose
*    An internal message to be sent around the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.106 $
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
	 * The assetid the message relates to (if it is about an asset).
	 * @var string
	 */
	var $assetid = NULL;

	/**
	* An array of params that may be used by the message depending on its type
	* @var array
	*/
	var $parameters = Array();

	/**
	 * A set of keyword replacements (separate from params) that are used to localise messages
	 * Note that these are temporary and is not saved in localised messages
	 * @var array
	 */
	var $replacements = Array();


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
		$db = MatrixDAL::getDb();

		try {
			$bind_vars	= Array (
							'messageid'	=> $messageid,
						  );
			$result_all	= MatrixDAL::executeAll('core', 'getMessageInfoByMessageId', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get message with messageid #'.$messageid.' due to the following database error:'.$e->getMessage());
		}

		$result = Array();
		if (!empty($result_all)) {
			$result	= $result_all[0];
		} else {
			return false;
		}

		$this->id         = $result['msgid'];
		$this->to         = Array($result['userto']);
		$this->from       = $result['userfrom'];
		$this->subject    = $result['subject'];
		$this->body       = $result['body'];
		$this->type       = $result['type'];
		$this->sent       = iso8601_ts($result['sent']);
		$this->priority   = $result['priority'];
		$this->status     = $result['status'];
		$this->parameters = $result['parameters'];
		$this->assetid    = $result['assetid'];

		return true;

	}//end load()


	/**
	* Sends an internal message
	*
	* @param boolean $user_inbox_only	If TRUE, sends internal message to User's Matrix inbox only
	*									This will bypass the Message Service logs config settings
	*									
	*
	* @return boolean
	* @access public
	*/
	function send($user_inbox_only=FALSE)
	{
		// make sure we have the required fields
		assert_not_empty(trim($this->type), 'Could not send internal message without a type set');

		// lets work out what action should be conducted on this type of message
		// ie. should it logged, mailed etc
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$lm = $GLOBALS['SQ_SYSTEM']->lm;

		foreach ($ms->getLogTypes($this->type) as $main_type_code => $allow_log) {
			$$main_type_code = $allow_log;
		}//end foreach

		$user_from = null;
		$from_name = '';
		if ($this->from && assert_valid_assetid($this->from, '', TRUE, FALSE)) {
			$user_from = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->from);
			if (!is_null($user_from)) {
				$from_name = $user_from->name;
			} else {
				$from_name = 'Unknown User #'.$this->from;
			}
		} else {
			// message from the system
			$from_name = SQ_SYSTEM_SHORT_NAME.' System';
		}

		$param_string = '';
		foreach ($this->parameters as $key => $value) {
			$param_string .= "$key:$value;";
		}

		$assetid = NULL;
		if (isset($this->parameters['assetid']) === TRUE) {
			$assetid = $this->parameters['assetid'];
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		// generic prepared statement for inserting
		$sql = 'INSERT INTO
					sq_internal_msg
					(
						msgid,
						userto,
						userfrom,
						subject,
						body,
						type,
						priority,
						status,
						sent,
						parameters,
						assetid
					)
					VALUES
					(
						:msgid,
						:userto,
						:userfrom,
						:subject,
						:body,
						:type,
						:priority,
						:status,
						:sent,
						:parameters,
						:assetid
					)';

		  //////////////////////////////
		 // LOG MESSAGES TO LOG FILE //
		//////////////////////////////
		if ($log_to_file && !$user_inbox_only) {
			$log_body = preg_replace('|<SQ_MSG_LINK[^>]*?>(.*?)</SQ_MSG_LINK>|', '\\1', $this->body);

			$log_line = '['.$this->type.' - '.$param_string.'] ('.
				$lm->getInternalMessageSubject($this->type, $this->replacements, SQ_CONF_DEFAULT_BACKEND_LOCALE).') - '.
				$lm->getInternalMessageBody($this->type, $this->replacements, SQ_CONF_DEFAULT_BACKEND_LOCALE);

			$tagline_reg = '|<SQ_MSG_TAGLINE[^>]*?>(.*?)</SQ_MSG_TAGLINE>|';
			if (preg_match($tagline_reg, $log_line, $matches)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[1]);
				$log_line = preg_replace($tagline_reg, $asset->name.' (#'.$asset->id.')', $log_line);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			log_write($log_line);

		}//end if logging message to file

		if ($log_to_db || $send_mail) {
			// work out the email address the the email will be sent from
			$from_email = SQ_CONF_DEFAULT_EMAIL;
			$from_string = '';
			if ($this->from && !is_null($user_from)) {
				$from_string = $user_from->name.' <'.$user_from->attr('email').'>';
				$from_email = $user_from->attr('email');
			} else if ($this->from && valid_email($this->from)) {
				$from_email = $this->from;
			}

			if (empty($from_email)) {
				$from_email = SQ_CONF_DEFAULT_EMAIL;
			}
			if (empty($from_string)) {
				$from_string = '"'.SQ_SYSTEM_SHORT_NAME.' System" <'.$from_email.'>';
			}
		}


		  //////////////////////////////
		 // LOG MESSAGES TO DATABASE //
		//////////////////////////////
		if ($log_to_db) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// log a message for the system so it is not deleted
			$messageid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_internal_msg_seq'));

			$log_body = preg_replace('|<SQ_MSG_LINK ([^>]*?)>(.*?)</SQ_MSG_LINK>|', '<a \\1>\\2</a>', $this->body);

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'msgid',      $messageid,      PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'userto',     0,               PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'userfrom',   $this->from,     PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'subject',    (empty($this->subject) ? $lm->getInternalMessageSubject($this->type, $this->replacements, SQ_CONF_DEFAULT_BACKEND_LOCALE) : $this->subject), PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'body',       (empty($this->body) ? $lm->getInternalMessageBody($this->type, $this->replacements, SQ_CONF_DEFAULT_BACKEND_LOCALE) : $this->body), PDO::PARAM_LOB);
				MatrixDAL::bindValueToPdo($query, 'type',       $this->type,     PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'priority',   $this->priority, PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'status',     $this->status,   PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'sent',       $this->sent,     PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'parameters', $param_string,   PDO::PARAM_LOB);
				MatrixDAL::bindValueToPdo($query, 'assetid',    $assetid,        PDO::PARAM_STR);
				MatrixDAL::execPdoQuery($query);
			} catch (DALException $e) {
				throw new Exception('Creation of internal message failed due to database error: '.$e->getMessage());
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}//end if logging message to db

		  ///////////////////////////
		 // SEND MESSAGE TO USERS //
		///////////////////////////
		if ($send_mail || (0 === strpos($this->type, 'inbox')) || $user_inbox_only) {

			require_once 'Mail.php';
			require_once 'Mail/mime.php';

			$send_to = $this->expandUsersTo();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// new line is \n
			$crlf = "\n";
			$mime = new Mail_mime($crlf);

			// workaround to fix the strict standard "non-static method"
			// $mail = Mail::factory('mail', "-f$from_email");
			$mf = new Mail();
			$mail = $mf->factory('mail', "-f$from_email");

			// send each user a message - via email and into their internal message inbox
			foreach ($send_to as $userid) {

				$reply_to_email  = isset($this->parameters['reply_to']) ? $this->parameters['reply_to'] : '';
				if (!$userid) {
					// The userid is empty, which means we are sending a message to the system
					// so we send to the default email and tech email

					$default = SQ_CONF_DEFAULT_EMAIL;
					$tech = SQ_CONF_TECH_EMAIL;
					$to_email = '';

					$user_locale = SQ_CONF_DEFAULT_BACKEND_LOCALE;

					$subject = $this->subject;
					if (empty($subject)) {
						$subject = $lm->getInternalMessageSubject($this->type, $this->replacements, $user_locale);
					} else {
						$subject = replace_keywords($subject, $this->replacements);
					}
					$hdrs = Array(
								'From'		=> $from_string,
								'Subject'	=> $subject,
							);

					if (!empty($default)) $to_email .= $default;
					if (!empty($tech)) {
						if (!empty($to_email)) $to_email .= ',';
						$to_email .= $tech;
					}

					// can't come up with a To: email address - bail for this address
					if (empty($to_email)) {
						// We want to skip this user.
						continue;
					}

					// get the body with a header and a footer attached
					$owner = SQ_CONF_SYSTEM_OWNER;
					$log_body = $this->body;
					if (empty($log_body)) {
						$log_body = $lm->getInternalMessageBody($this->type, $this->replacements, $user_locale);
					} else {
						$log_body = replace_keywords($log_body, $this->replacements);
					}
					$log_body = preg_replace('|<SQ_MSG_LINK ([^>]*?)>(.*?)</SQ_MSG_LINK>|', '<a \\1>\\2</a>', $log_body);

					$tagline_reg = '|<SQ_MSG_TAGLINE[^>]*?>(.*?)</SQ_MSG_TAGLINE>|';
					if (preg_match($tagline_reg, $log_body, $matches)) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[1]);
						$log_body = preg_replace($tagline_reg, $asset->name.' (#'.$asset->id.')', $log_body);
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					}

					$ms->queueEmail($this->from, 0, $subject, $log_body, $reply_to_email);
					if (!$ms->sending_queue) $ms->sendQueuedEmails();

				} else {
					// we are sending a message to user(s)

					// prepare the message content
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
					if (is_null($user)  || !($user instanceof User) || (!$user_inbox_only && !$user->canAccessBackend()) || !$user->canLogin()) continue;
					$user_locale = $user->attr('locale');
					if (empty($user_locale)) {
						$user_locale = SQ_CONF_DEFAULT_BACKEND_LOCALE;
					}

					if (!isset($this->replacements['user_name'])){
						$this->replacements['user_name'] = $user->name;
					}
		
					$subject = empty($this->subject) ? $lm->getInternalMessageSubject($this->type, $this->replacements, $user_locale) : replace_keywords($this->subject, $this->replacements);
					$hdrs = Array(
								'From'		=> $from_string,
								'Subject'	=> $subject,
							);
					$body = $this->formatBody($user, TRUE);
					$log_body = preg_replace('|<SQ_MSG_LINK ([^>]*?)>(.*?)</SQ_MSG_LINK>|', '<a \\1>\\2</a>', $body);

					// log the internal message to the DB
					// TODO: replace with seqNextVal() in queries.xml
					try {
						$messageid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_internal_msg_seq'));
					} catch (Exception $e) {
						throw new Exception("Unable to get next internal message ID due to database error:".$e->getMessage());
					}//end try catch

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'msgid',      $messageid,      PDO::PARAM_INT);
						MatrixDAL::bindValueToPdo($query, 'userto',     $userid,         PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'userfrom',   $this->from,     PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'subject',    $subject,        PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'body',       $log_body,       PDO::PARAM_LOB);
						MatrixDAL::bindValueToPdo($query, 'type',       $this->type,     PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'priority',   $this->priority, PDO::PARAM_INT);
						MatrixDAL::bindValueToPdo($query, 'status',     $this->status,   PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'sent',       $this->sent,     PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'parameters', $param_string,   PDO::PARAM_LOB);
						MatrixDAL::bindValueToPdo($query, 'assetid',    $assetid,        PDO::PARAM_STR);
						MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('Creation of internal message failed due to database error: '.$e->getMessage());
					}

					if ($send_mail && !$user_inbox_only) {
						// send an email as well
						$to_email = trim($user->attr('email'));
						//reload the $body again, since it might be different from internal message.

						$body = $this->formatBody($user);

						// get the body with a header and a footer attached
						$log_body = preg_replace('|<SQ_MSG_LINK ([^>]*?)>(.*?)</SQ_MSG_LINK>|', '<a \\1>\\2</a>', $body);

						$tagline_reg = '|<SQ_MSG_TAGLINE[^>]*?>(.*?)</SQ_MSG_TAGLINE>|';
						if (preg_match($tagline_reg, $log_body, $matches)) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[1]);
							$log_body = preg_replace($tagline_reg, $asset->name.' (#'.$asset->id.')', $log_body);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						}

						$ms->queueEmail($this->from, $userid, $subject, $log_body, $reply_to_email);
						if (!$ms->sending_queue) $ms->sendQueuedEmails();
					}
				}

			}//end foreach

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}//end if sending message via email

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// Clear internal message cache for the current user
		if (!is_null($GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->user->clearUserMessageLocalCache();
		}

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
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
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
						<td valign="top" class="sq-message-item"><b><?php strtoupper(translate('subject')) ?>:</b></td>
						<td valign="top" class="sq-message-item" width="100%"><?php echo $info['subject']; ?></td>
					</tr>
					<tr>
						<td valign="top" class="sq-message-item"><b><?php strtoupper(translate('from')) ?>:</b></td>
						<td valign="top" class="sq-message-item" width="100%"><?php echo $info['from_name'] ?></td>
					</tr>
					<tr>
						<td valign="top" class="sq-message-item"><b><?php strtoupper(translate('date')) ?>:</b></td>
						<td valign="top" class="sq-message-item" width="100%"><?php echo readable_datetime($info['sent']); ?></td>
					</tr>
					<tr height="100%">
						<td valign="top" class="sq-message-item-msg" bgcolor="#FFFFFF" width="100%" colspan="2"><br/><pre><?php echo $info['body']; ?></pre></td>
					</tr>
				</table>
			</body>
		</html>
		<?php

	}//end printBody()


	/**
	* Replaces keywords in the body of the message
	*
	* @param object		&$user				the user that the message is being sent to
	* @param boolean	$isInternalMessage	Determine whether this message is internal or to be sent to email.
	*
	* @return boolean
	* @access public
	*/
	function formatBody(&$user, $isInternalMessage=FALSE)
	{
		$tmpReplacements = $this->replacements;
		$lm = $GLOBALS['SQ_SYSTEM']->lm;

		// If it is an internal message, check if the messages are of workflow related.
		if ($isInternalMessage == TRUE) {
			if (strpos($this->type, 'asset.workflow.') !== FALSE) {
				// Add SQ_BACKEND_PAGE flag so the internal message will know that it is going to be opened
				// within a frame.
				if (isset($tmpReplacements['workflow_url'])) {
					$tmpReplacements['workflow_url'] .= '&SQ_BACKEND_PAGE=main';
				}
				if (isset($tmpReplacements['preview_url'])) {
					$tmpReplacements['preview_url']  .= '&SQ_BACKEND_PAGE=main';
				}
				if (isset($tmpReplacements['accept_url'])) {
					$tmpReplacements['accept_url']  .= '&SQ_BACKEND_PAGE=main';
				}
				if (isset($tmpReplacements['reject_url'])) {
					$tmpReplacements['reject_url']  .= '&SQ_BACKEND_PAGE=main';
				}
			}
		}

		if (!($user instanceof User)) {
			if (trim($this->body) === '') {
				return $lm->getInternalMessageBody($this->type, $tmpReplacements, SQ_CONF_DEFAULT_BACKEND_LOCALE);
			} else {
				$body = $this->body;
				return replace_keywords($body, $tmpReplacements);
			}
		}
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$replacements = $tmpReplacements;
		if (!isset($replacements['user_name'])){
			$replacements['user_name'] = $user->name;
		}

		$user_locale = $user->attr('locale');
		if (empty($user_locale)) {
			$user_locale = SQ_CONF_DEFAULT_BACKEND_LOCALE;
		}
		if (trim($this->body) === '') {
			return $lm->getInternalMessageBody($this->type, $replacements, $user_locale);
		} else {
			$body = $this->body;
			return replace_keywords($body, $replacements);
		}
		return replace_keywords($body, $replacements);


	}//end formatBody()


	/**
	* Returns an array of users this message will be sent to
	*
	* That is, it expands the groups ids and returns a unique array of userids
	*
	* @return array
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
									trigger_localised_error('SYS0168', E_USER_WARNING, $this->type);
								} else if (!isset($this->parameters['minorid'])) {
									trigger_localised_error('SYS0168', E_USER_WARNING, $this->type);
								} else {
									$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->parameters['majorid']);
									if (is_null($major)) {
										trigger_localised_error('SYS0167', E_USER_WARNING, $this->parameters['majorid'], $this->type);
									}
									$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->parameters['minorid']);
									if (is_null($minor)) {
										trigger_localised_error('SYS0167', E_USER_WARNING, $this->parameters['minorid'], $this->type);
									}
								}
							break(2);

							case 'locking' :
								if (isset($type[2]) && $type[2] == 'forced') {
									$this->to = array_merge($this->to, Array($this->parameters['former_userid']));
								}
							break;
						}//end switch
					}

					if (!isset($this->parameters['assetid'])) {
						// cant send an asset message if we dont know which asset is affected
						trigger_localised_error('SYS0168', E_USER_WARNING, $this->type);
					} else {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->parameters['assetid']);
						if (is_null($asset)) {
							// cant send an asset message if we dont know which asset is affected
							trigger_localised_error('SYS0167', E_USER_WARNING, $this->parameters['assetid'], $this->type);
						}

						$this->to = array_merge($this->to, $GLOBALS['SQ_SYSTEM']->am->getPermission($asset->id, SQ_PERMISSION_ADMIN, true, false));
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					}
				break;

				case 'config' :
				case 'hipo'   :
					// send config changes to the DEFAULT and TECH emails and to the Root User
					$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
					$this->to = array_merge($this->to, Array(0, $root_user->id));
				break;

			}//end switch

		}// end if

		$send_to = Array();
		if (in_array('0', $this->to)) {
			// this message is being sent to the system
			// so we need to keep the ZERO id in the array
			$send_to[] = 0;
		}

		foreach ($this->to as $assetid) {
			// skip the ZERO (system) assetid as it's already included
			if ($assetid == 0) continue;

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) continue;

			if ($asset instanceof User_Group) {
				$send_to = array_merge($send_to, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('user'), false)));
			} else if ($asset instanceof User) {
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
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$bind_vars = Array('msgid' => $this->id);
			MatrixDAL::executeQuery('core', 'deleteInternalMessage', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete this message #'.$this->id);
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		
		// Clear the current user's local cache for internal message
		if (!is_null($GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->user->clearUserMessageLocalCache();
		}

		return TRUE;

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

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$bind_vars = Array('status' => $status, 'msgid' => $this->id);
		MatrixDAL::executeQuery('core', 'updateInternalMessage', $bind_vars);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		
		// Clear the current user's local cache for internal message
		if (!is_null($GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->user->clearUserMessageLocalCache();
		}

		return true;

	}//end updateStatus()


}//end class

?>
