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
* $Id: messaging_service.inc,v 1.39.2.7 2006/03/02 03:26:33 mmcintyre Exp $
*
*/


require_once SQ_DATA_PATH.'/private/conf/messaging_service.inc';
require_once 'Mail.php';
require_once 'Mail/mime.php';

define('SQ_MSG_UNREAD' , 'U');
define('SQ_MSG_READ'   , 'R');
define('SQ_MSG_DELETED', 'D');

define('SQ_MSG_PRIORITY_VERY_LOW'  , 1);
define('SQ_MSG_PRIORITY_LOW'       , 2);
define('SQ_MSG_PRIORITY_NORMAL'    , 3);
define('SQ_MSG_PRIORITY_HIGH'      , 4);
define('SQ_MSG_PRIORITY_VERY_HIGH' , 5);

/**
* Messaging_Service
*
* Purpose
*    Facilitate the sending and management of internal messages
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.39.2.7 $
* @package MySource_Matrix
*/
class Messaging_Service extends MySource_Object
{

	/**
	* Temporary holder for messages added to queues and logs
	*
	* @var Array()
	*/
	var $_levels = Array(
					'queues'	=> Array(),
					'logs'		=> Array(),
				   );


	/**
	* Queued email messages, that will be sent by sendQueuedEmails()
	*
	* @var Array()
	*/
	var $_emails = Array();

	/**
	* Flag that is set when queued messages are sent using send()
	*
	* This is used by the internal messages to work out whether it
	* needs to initiate sending the email itself - if a queue/log is
	* being processed, it'll be handled by that.
	*
	* @var boolean
	*/
	var $sending_queue = false;


	/**
	* Constructor
	*
	*/
	function Messaging_Service()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Returns the log types that are to be used for the passed message type
	* Log Types: 'log_to_file', 'log_to_db', 'send_mail'
	*
	* @param string $msg_type	the message type (eg 'asset.workflow.announce.approve')
	*
	* @return array('string'=>bool)	array of log type => boolen for whether to use or not
	* @public
	*/
	function getLogTypes($msg_type)
	{

		if (!isset($this->_tmp['message_type_logs'][$msg_type])) {

			$this->_tmp['message_type_logs'][$msg_type] = Array();

			$msg_type_list = explode('.', $msg_type);

			foreach (Array('log_to_file', 'log_to_db', 'send_mail') as $log_type) {

				$def_name_w = 'SQ_MS_'.strtoupper($log_type).'_WHITE_LIST';
				$def_name_b = 'SQ_MS_'.strtoupper($log_type).'_BLACK_LIST';
				$types_w = explode("\n", constant($def_name_w));
				$types_b = explode("\n", constant($def_name_b));

				if (in_array($msg_type, $types_w)) {
					// this exact type is white listed
					$allow_log = true;
				} else if (in_array($msg_type, $types_b)) {
					// this exact type is black listed
					$allow_log = false;
				} else {

					// We need to cycle through all the parts for completeness
					// example of why:
					//		msg_type = asset.attributes.fulllog.scalar
					//		WHITE_LIST: asset.attributes.*
					//		BLACK_LIST: asset.attributes.fulllog.*
					// If we didn't go through whole list then we would be allowing this log
					// when we have specifically denied it.

					$allow_log = false;

					if (in_array('*', $types_w)) {
						// all messages of this type are white listed
						$allow_log = true;
					}

					// check the virtual types to see if they clash
					$check_code = '';
					foreach ($msg_type_list as $code_part) {
						if (!empty($check_code)) {
							$check_code = trim($check_code, '* ');
						}
						$check_code .= $code_part.'.*';
						if (in_array($check_code, $types_w)) {
							// this virtual type is white listed
							$allow_log = true;

						} else if (in_array($check_code, $types_b)) {
							// this virtual type is explicitly black listed
							$allow_log = false;

						}

					}

				}

				// cache this for this script execution
				$this->_tmp['message_type_logs'][$msg_type][$log_type] = $allow_log;

			}// end foreach

		}//end if

		return $this->_tmp['message_type_logs'][$msg_type];

	}//end getLogTypes()


//--        QUEUES        --//


	/**
	* Opens a new queue or nested queue
	* Note that this function really just updates how many queues are open
	*
	* @return boolean
	* @access public
	*/
	function openQueue()
	{
		array_push($this->_levels['queues'], Array());

	}//end openQueue()


	/**
	* Closes a queue or nested queue, and add's it messages to the main queue
	* If the last queue is closed, the messages in the queue are sent
	*
	* @return boolean
	* @access public
	*/
	function closeQueue()
	{
		// remove the current levels messages
		$msgs = array_pop($this->_levels['queues']);

		// add them to the level above or the main queue
		if (empty($this->_levels['queues'])) {
			// this is the last queue
			return $this->send($msgs);
		} else {
			foreach ($msgs as $msg) {
				array_push($this->_levels['queues'][count($this->_levels['queues']) - 1], $msg);
			}
			return true;
		}

	}//end closeQueue()


	/**
	* Aborts a queue or nested queue, and removes it's messages from the queue
	*
	* @return boolean
	* @access public
	*/
	function abortQueue()
	{
		// remove the current levels messages
		array_pop($this->_levels['queues']);
		return true;

	}//end abortQueue()


	/**
	* Adds an internal message into the queue
	*
	* @param object Internal_Message	$message	the message to add to the queue
	*
	* @return void
	* @access public
	*/
	function enqueueMessage($message)
	{
		array_push($this->_levels['queues'][count($this->_levels['queues']) - 1], $message);

	}//end enqueueMessage()


	/**
	* Returns the messages in the current queue or nested queue
	*
	* Only the contents of the most inside nested queue are available from this function
	*
	* @return array of object Internal_Message
	* @access public
	*/
	function queueContents()
	{
		return $this->_levels['queues'][count($this->_levels['queues']) - 1];

	}//end queueContents()


//--        LOGS        --//


	/**
	* Opens a new log or nested log
	* Note that this function really just updates how many logs are open
	*
	* @return boolean
	* @access public
	*/
	function openLog()
	{
		array_push($this->_levels['logs'], Array());

	}//end openLog()


	/**
	* Closes a log or nested log, and add's it messages to the main log
	* If the last log is closed, the messages in the log are sent
	*
	* @return boolean
	* @access public
	*/
	function closeLog()
	{
		// remove the current logs messages
		$msgs = array_pop($this->_levels['logs']);

		// add them to the level above or the main queue
		if (empty($this->_levels['logs'])) {
			// this is the last queue
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			foreach ($msgs as $msg) {
				if (!$msg->send()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;

		} else {
			foreach ($msgs as $msg) {
				array_push($this->_levels['logs'][count($this->_levels['logs']) - 1], $msg);
			}
			return true;

		}//end if

	}//end closeLog()


	/**
	* Aborts a log or nested log, and removes it's messages from the log
	*
	* @return boolean
	* @access public
	*/
	function abortLog()
	{
		// remove the current levels messages
		array_pop($this->_levels['logs']);
		return true;

	}//end abortLog()


	/**
	* Adds an internal message into the log
	*
	* @param object Internal_Message	$message	the message to add to the log
	*
	* @return void
	* @access public
	*/
	function logMessage($message)
	{
		array_push($this->_levels['logs'][count($this->_levels['logs']) - 1], $message);

	}//end logMessage()


	/**
	* Returns the messages in the current log or nested log
	*
	* Only the contents of the most inside nested log are available from this function
	*
	* @return array of object Internal_Message
	* @access public
	*/
	function logContents()
	{
		return $this->_levels['logs'][count($this->_levels['logs']) - 1];

	}//end logContents()


//--        MESSAGES        --//


	/**
	* Creates and returns a new internal message
	*
	* Some variables of a message can be passed in for simple messages
	* but messages that want to utilise other options need to set these manually
	* so as to not clutter this functions
	*
	* @param array(int)	$to			an array of user or group IDs to send this message to
	* @param string		$type		the log type of this message (if any)
	* @param Array		$reps		array of keywords replacements
	*
	* @return object Internal_Message
	* @access public
	*/
	function newMessage($to=Array(), $type='', $reps=Array())
	{
		require_once SQ_INCLUDE_PATH.'/internal_message.inc';

		$new_message =& new Internal_Message();
		$new_message->sent         = ts_iso8601(time());
		$new_message->from         = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$new_message->to           = $to;
		$new_message->type         = $type;
		$new_message->replacements = $reps;
		return $new_message;

	}//end newMessage()


	/**
	* Sends all messages passed in, doing some amalgamation of msg bodies for same user and same subject
	*
	* @param array	$msgs	an array of Internal_Message to send
	*
	* @return void
	* @access public
	*/
	function send($msgs)
	{
		// lets try and work out if any of the messages to be
		// sent can actually be amalgamated based on who the message is being
		// sent to, who it is from, the message type, and the subject line of the message
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$this->sending_queue = true;

		foreach ($msgs as $msg) {
			$msg->send();
		}

		$this->sending_queue = false;
		$this->sendQueuedEmails();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end send()


	/**
	* Gets all messages for the passed userid
	*
	* @param string			$userid			id of the user you are fetching messages for (0 = system, null = any user except system)
	* @param string			$type			the type of the message eg. asset.workflow or asset.attributes.*
	* @param array(string)	$statii			can limit returned messages to those with a status
	*										in this array
	* @param array(int)		$messageids		can limit returned messages to those with a messageid
	*										in this array
	* @param int			$from			timestamp that every msg returned must be this time
	*										or after (NULL to ignore)
	* @param int			$to				timestamp that every msg returned must be this time
	*										or before (NULL to ignore)
	* @param string			$get_user_name	[name|short_name], used to return the name/short_name
	*										of the from user with the return, which will be returned
	*										in the field 'from_name'. (NULL to ignore)
	* @param array			$parameters		can limit returned messages to those with these params set
	*
	* @return array
	* @access public
	*/
	function getMessages($userid, $type=null, $statii=Array(), $messageids=Array(), $from=null, $to=null, $get_user_name=null, $parameters=Array())
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if (!is_null($get_user_name) && $get_user_name != 'name') {
			$get_user_name = 'short_name';
		}

		$sql = 'SELECT m.msgid, m.userto, m.userfrom, m.subject, m.body, m.sent, m.priority, m.status, m.type, m.parameters';
		if (is_string($get_user_name)) {
			$sql .= ',
					CASE
						WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = '.$db->quoteSmart('0').' THEN '.$db->quote(SQ_SYSTEM_SHORT_NAME.' System').'
						ELSE a.'.$get_user_name.'
					END as from_name';
			$sql  .=	', CASE WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = '.$db->quoteSmart('0').' THEN \'root_user\''.
					' ELSE a.type_code END as type_code';
		}

		$sql  .= ' FROM sq_internal_msg m';
		if (is_null($userid)) {
			$where = 'm.userto != '.$db->quoteSmart('0');
		} else {
			$where = 'm.userto = '.$db->quoteSmart($userid);
		}

		if (is_string($get_user_name)) {
			$sql .= ' LEFT OUTER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON m.userfrom = a.assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		} else {
			$where = 'WHERE '.$where;
		}

		if (!is_null($type)) {
			if (substr($type, -2) == '.*') {
				$where .= ' AND m.type LIKE '.$db->quoteSmart(substr($type, 0, -1).'%');
			} else {
				$where .= ' AND m.type = '.$db->quoteSmart($type);
			}
		}
		if (!empty($statii)) {
			for ($i = 0; $i < count($statii); $i++) {
				$statii[$i] = $db->quoteSmart($statii[$i]);
			}
			$where .= ' AND m.status IN ('.implode(', ', $statii).')';
		}
		if (!empty($messageids)) {
			for ($i = 0; $i < count($messageids); $i++) {
				$messageids[$i] = $db->quoteSmart($messageids[$i]);
			}
			$where .= ' AND m.msgid IN ('.implode(', ', $messageids).')';
		}
		if (!empty($parameters)) {
			foreach ($parameters as $param_key => $param_value) {
				$where .= ' AND m.parameters LIKE '.$db->quoteSmart('%'.$param_key.':'.$param_value.';%');
			}
		}
		if (!is_null($from)) {
			$where .= ' AND m.sent >= '.$db->quoteSmart(ts_iso8601($from));
		}
		if (!is_null($to)) {
			$where .= ' AND m.sent <= '.$db->quoteSmart(ts_iso8601($to));
		}

		$sql .= ' '.$where.' ORDER BY m.sent DESC';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		for ($i = 0, $total = count($result); $i < $total; $i++) {
			$result[$i]['sent'] = iso8601_ts($result[$i]['sent']);

			// if we are showing the from name, check that any messages from shadow
			// assets are actually displaying the correct name
			if (is_string($get_user_name)) {
				$id_parts = explode(':', $result[$i]['userfrom']);
				if (isset($id_parts[1])) {
					$bridgeid = $id_parts[0];
					$bridge = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
					$shadow = &$bridge->getAsset($id_parts[1]);
					if (!is_null($shadow)) {
						$result[$i]['from_name'] = $shadow->$get_user_name;
						$result[$i]['type_code'] = $shadow->type();
					}
				}

				// if we couldnt find the name of the user, set a default
				if (empty($result[$i]['from_name'])) {
					$result[$i]['from_name'] = 'Unknown User';
				}

				// if we couldnt find the type code user, set a default
				if (empty($result[$i]['type_code'])) {
					$result[$i]['type_code'] = 'user';
				}
			}
		}

		return $result;

	}//end getMessages()


	/**
	* Get logged messages of the passed typed
	*
	* This is just a cut down version of getMessages so you can get logs without
	* having to call getMessages will a bunch of array and NULL arguments
	*
	* @param string	$type		the type of the message eg. asset.attributes
	* @param array	$parameters	can limit returned messages to those with these params set
	*
	* @return array
	* @access public
	* @see getMessages()
	*/
	function getLogs($type, $parameters=Array())
	{
		return $this->getMessages(0, $type, Array(), Array(), null, null, 'name', $parameters);

	}//end getLogs()


	/**
	* Returns the message object for the passed id
	*
	* @param int	$messageid	id of the message the you want to get
	*
	* @return object Internal_Message	NULL on error or Internal_Message
	* @access public
	*/
	function getMessageById($messageid)
	{
		$message = $this->newMessage();
		$message->load($messageid);
		if (!$message->id) return null;
		return $message;

	}//end getMessageById()


	/**
	* Sorts two messages based on the sent date (newest message first)
	*
	* @param array	$a	the first message to compare
	* @param array	$b	the second message to compare
	*
	* @return int
	* @access public
	*/
	function sortMessages($a, $b)
	{
		if ($a['sent'] == $b['sent']) return 0;
		return ($a['sent'] > $b['sent']) ? -1 : 1;

	}//end sortMessages()


	/**
	* queue up an email to send at the conclusion of the script
	*
	* @param string	$from		the assetid of the user to send from (0 for system)
	* @param string	$to			the assetid of the user to send to (0 for system)
	* @param string $subject	subject of the internal message
	* @param string $body		body of the internal message
	*
	* @return void
	* @access public
	*/
	function queueEmail($from, $to, $subject, $body)
	{
		if (!isset($this->_emails[$from])) {
			$this->_emails[$from] = Array();
		}

		if (!isset($this->_emails[$from][$to])) {
			$this->_emails[$from][$to] = Array();
		}

		if (!isset($this->_emails[$from][$to][$subject])) {
			$this->_emails[$from][$to][$subject] = Array();
		}

		$this->_emails[$from][$to][$subject][] = $body;

	}//end queueEmail()


	/**
	* Sends queued-up emails
	*
	* @return boolean
	* @access public
	*/
	function sendQueuedEmails()
	{
		$crlf = "\n";
		$mime =& new Mail_mime($crlf);

		foreach ($this->_emails as $from => $from_emails) {
			// work out the email address the the email will be sent from
			$from_email = SQ_CONF_DEFAULT_EMAIL;
			$from_string = '';
			if ($from) {
				$user_from = &$GLOBALS['SQ_SYSTEM']->am->getAsset($from);
				if (!is_null($user_from)) {
					$from_string = $user_from->name.' <'.$user_from->attr('email').'>';
					$from_email = $user_from->attr('email');
				}
			}
			if (empty($from_email)) {
				if (isset($_SERVER['HOSTNAME']) && isset($_SERVER['HTTP_HOST'])) {
					$from_email = 'webmaster@'.((SQ_PHP_CLI) ? $_SERVER['HOSTNAME'] : $_SERVER['HTTP_HOST']);
				} else {
					$from_email = SQ_CONF_DEFAULT_EMAIL;
				}
			}
			if (empty($from_string)) {
				$from_string = '"'.SQ_SYSTEM_SHORT_NAME.' System" <'.$from_email.'>';
			}

			foreach ($from_emails as $to => $to_emails) {
				if (!$to) {
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
					$user = SQ_CONF_SYSTEM_OWNER;
				} else {
					$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($to);
					if (is_null($user)) continue;
					$to_email = trim($user->attr('email'));
				}

				if (empty($to_email)) continue;
				foreach ($to_emails as $subject => $bodies) {

					$body = implode("\n\n".str_repeat('-', 50)."\n\n", $bodies);
					$mail =& Mail::factory('mail', "-f$from_email");

					$hdrs = Array(
								'From'		=> $from_string,
								'Subject'	=> $subject,
							);

					$text_body = $this->formatEmail($body, $user, 'text');
					$html_body = $this->formatEmail($body, $user, 'html');

					$mime->setTXTBody(strip_tags($text_body));
					$mime->setHTMLBody($html_body);

					// get() must be called before headers()
					$body = $mime->get();
					$hdrs = $mime->headers($hdrs);

					$mail->send($to_email, $hdrs, $body);
				}
			}
		}

		// Remove sent emails
		unset($this->_emails);
		$this->_emails = Array();

	}//end sendQueuedEmails()


	/**
	* formats the body of an email with a header and a footer
	*
	* @param mixed	$user	either the user object or a string containing the user's name that the email is intended for
	* @param string	$body	the email body
	*
	* @access public
	* @return string the body
	*/
	function formatEmail($body, &$user, $type='text')
	{
		$term = ($type=='text') ? "\n" : '<br />';

		$header = '';
		if (!is_null($user) && is_object($user) && is_a($user, 'user')) {
			$header = $user->name.','.$term.$term;
		} else if (is_string($user) && $user != '') {
			$header = $user.','.$term.$term;
		}
		// replace separator line for multiple-bodied message with a h-rule
		if ($type == 'html') {
			$body = nl2br(str_replace("\n\n".str_repeat('-', 50)."\n\n",'<hr />',$body));
		}

		$line = ($type == 'text') ? str_repeat('-', 20).$term : '<hr />';

		$footer = $term.$term.$line
				.'This is an automated message from '.SQ_CONF_SYSTEM_NAME.' running '.SQ_SYSTEM_LONG_NAME.$term
				.'System Root URLs : '.$term.str_replace("\n", $term, SQ_CONF_SYSTEM_ROOT_URLS).$term
				.$term
				.'Contact '.SQ_CONF_SYSTEM_OWNER.' ('.SQ_CONF_TECH_EMAIL.') for support.';
		return $header.$body.$footer;

	}//end formatEmail()


}//end class

?>
