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
* $Id: messaging_service.inc,v 1.85 2013/09/17 00:07:51 cupreti Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_DATA_PATH.'/private/conf/messaging_service.inc';
require_once SQ_DATA_PATH.'/private/conf/main.inc';
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
* @version $Revision: 1.85 $
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
	var $sending_queue = FALSE;


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
	* ('string'=>bool)	array of log type => boolen for whether to use or not
	*
	* @param string	$msg_type	the message type (eg 'asset.workflow.announce.approve')
	*
	* @return array
	* @access public
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
					$allow_log = TRUE;
				} else if (in_array($msg_type, $types_b)) {
					// this exact type is black listed
					$allow_log = FALSE;
				} else {

					// We need to cycle through all the parts for completeness
					// example of why:
					//    msg_type = asset.attributes.fulllog.scalar
					//    WHITE_LIST: asset.attributes.*
					//    BLACK_LIST: asset.attributes.fulllog.*
					// If we didn't go through whole list then we would be allowing this log
					// when we have specifically denied it.

					$allow_log = FALSE;

					if (in_array('*', $types_w)) {
						// all messages of this type are white listed
						$allow_log = TRUE;
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
							$allow_log = TRUE;

						} else if (in_array($check_code, $types_b)) {
							// this virtual type is explicitly black listed
							$allow_log = FALSE;

						}

					}

				}//end else

				// cache this for this script execution
				$this->_tmp['message_type_logs'][$msg_type][$log_type] = $allow_log;

			}//end foreach

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
			return TRUE;
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
		return TRUE;

	}//end abortQueue()


	/**
	* Adds an internal message into the queue
	*
	* It takes Internal_Message object as an argument
	*
	* @param object	$message	the message to add to the queue
	*
	* @return void
	* @access public
	*/
	function enqueueMessage($message)
	{
		if (empty($this->_levels['queues'])) {
			$this->openQueue();
		}
		array_push($this->_levels['queues'][count($this->_levels['queues']) - 1], $message);

	}//end enqueueMessage()


	/**
	* Returns the messages in the current queue or nested queue (array of object Internal_Message)
	*
	* Only the contents of the most inside nested queue are available from this function
	*
	* @return array
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
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			foreach ($msgs as $msg) {
				if (!$msg->send()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;

		} else {
			foreach ($msgs as $msg) {
				array_push($this->_levels['logs'][count($this->_levels['logs']) - 1], $msg);
			}
			return TRUE;

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
		return TRUE;

	}//end abortLog()


	/**
	* Adds an internal message into the log
	*
	* It takes Internal_Message object as an argument
	*
	* @param object	$message	the message to add to the log
	*
	* @return void
	* @access public
	*/
	function logMessage($message)
	{
		array_push($this->_levels['logs'][count($this->_levels['logs']) - 1], $message);

	}//end logMessage()


	/**
	* Returns the messages in the current log or nested log (array of object Internal_Message)
	*
	* Only the contents of the most inside nested log are available from this function
	*
	* @return array
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
	* @param array	$to		an array of user or group IDs to send this message to (integers)
	* @param string	$type	the log type of this message (if any)
	* @param array	$reps	the main body replacement
	*
	* @return object
	* @access public
	*/
	function newMessage($to=Array(), $type='', $reps=Array(), $from=NULL)
	{
		require_once SQ_INCLUDE_PATH.'/internal_message.inc';

		$new_message = new Internal_Message();
		$new_message->sent			= ts_iso8601(time());
		$new_message->from			= !is_null($from) ? $from : $GLOBALS['SQ_SYSTEM']->currentUserId();
		$new_message->to			= $to;
		$new_message->type			= $type;
		$new_message->replacements	= $reps;
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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$this->sending_queue = TRUE;

		foreach ($msgs as $msg) {
			$msg->send();
		}

		$this->sending_queue = FALSE;
		$this->sendQueuedEmails();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end send()


	/**
	* Gets all messages for the passed userid
	*
	* @param string		$userid			id of the user you are fetching messages for (0 = system, NULL = any user except system)
	* @param string		$type			the type of the message eg. asset.workflow or asset.attributes.*
	* @param array		$statii			can limit returned messages to those with a status
	*									in this array
	* @param array		$messageids		can limit returned messages to those with a messageid
	*									in this array
	* @param int		$from			timestamp that every msg returned must be this time
	*									or after (NULL to ignore)
	* @param int		$to				timestamp that every msg returned must be this time
	*									or before (NULL to ignore)
	* @param string		$get_user_name	[name|short_name], used to return the name/short_name
	*									of the from user with the return, which will be returned
	*									in the field 'from_name'. (NULL to ignore)
	* @param array		$parameters		can limit returned messages to those with these params set
	* @param int		$max_msgs		the maximum number of messages
	* @param boolean $count_only		whether to only get a count of the number of messages or not. defaults to FALSE
	*
	* @return array
	* @access public
	*/
	function getMessages($userid, $type=NULL, $statii=Array(), $messageids=Array(), $from=NULL, $to=NULL, $get_user_name=NULL, $parameters=Array(), $max_msgs=0, $count_only=FALSE)
	{
		$db = DAL::getDb();

		if (!is_null($get_user_name) && $get_user_name != 'name') {
			$get_user_name = 'short_name';
		}

		if (!$count_only) {
			$select_clause = 'm.msgid, m.userto, m.userfrom, m.subject, m.body, m.sent, m.priority, m.status, m.type, m.parameters';
			if (is_string($get_user_name)) {
				$select_clause .= ',
						CASE
							WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = \'0\' THEN '.MatrixDAL::quote(SQ_SYSTEM_SHORT_NAME.' System').'
							ELSE a.'.$get_user_name.'
						END as from_name';
				$select_clause .=	', CASE WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = \'0\' THEN \'root_user\''.
						' ELSE a.type_code END as type_code';
			}
		} else {
			$select_clause = 'COUNT(msgid) AS message_count';
		}

		$bindVars = array();
		$sql = 'SELECT ' . $select_clause;

		$sql  .= ' FROM sq_internal_msg m';
		if (is_null($userid)) {
			$where = 'm.userto != \'0\'';
		} else {
			# See bug 5455 for why this is a bind var and not quoted.
			# Basically - MatrixDAL::quote and Active directory DN's with backslashes in them
			# don't play nicely. Changing it to be a bindvar makes them work together.
			$where = 'm.userto = :userto';
			$bindVars['userto'] = $userid;
		}

		// If we are doing a count only, regardless of whether user_name is a string or not
		// we don't need to hit the ast table.
		if (is_string($get_user_name) && $count_only === FALSE) {
			$sql .= ' LEFT OUTER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON m.userfrom = a.assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		} else {
			$where = 'WHERE '.$where;
		}

		if (!is_null($type)) {
			if (substr($type, -2) == '.*') {
				$where .= ' AND m.type LIKE '.MatrixDAL::quote(substr($type, 0, -1).'%');
			} else {
				$where .= ' AND m.type = '.MatrixDAL::quote($type);
			}
		}
		if (!empty($statii)) {
			$statii_strs = Array();
			for ($i = 0; $i < count($statii); $i++) {
				$statii[$i] = '\''.$statii[$i].'\'';
			}
			$where .= ' AND m.status IN ('.implode(', ', $statii).')';
		}
		if (!empty($messageids)) {
			for ($i = 0; $i < count($messageids); $i++) {
				$messageids[$i] = '\''.$messageids[$i].'\'';
			}
			$where .= ' AND m.msgid IN ('.implode(', ', $messageids).')';
		}
		if (!empty($parameters)) {
			foreach ($parameters as $param_key => $param_value) {
				// skip the from and to date params for now
				if ($param_key == 'from_date' || $param_key == 'to_date') continue;
				$where .= ' AND m.parameters LIKE '.MatrixDAL::quote('%'.$param_key.':'.$param_value.';%');
			}
			if (isset($parameters['assetid']) === TRUE) {
				$where .= ' AND m.assetid='.MatrixDAL::quote($parameters['assetid']);
			}
			// from_date and to_date are to be included in the where clause
			if (isset($parameters['from_date']) && $parameters['from_date'] != '') {
				$where .= ' AND sent >= '.MatrixDAL::quote($parameters['from_date']);
			}
			if (isset($parameters['to_date']) && $parameters['to_date'] != '') {
				$where .= ' AND sent <= '.MatrixDAL::quote($parameters['to_date']);
			}
		}
		if (!is_null($from)) {
			$where .= ' AND m.sent >= '.MatrixDAL::quote(ts_iso8601($from));
		}
		if (!is_null($to)) {
			$where .= ' AND m.sent <= '.MatrixDAL::quote(ts_iso8601($to));
		}

		$sql .= ' '.$where;

		if (!$count_only) {
			$sql .= ' ORDER BY m.sent DESC';
		}

		if ($max_msgs) {
			$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), $max_msgs);
		}

		// Begin try block
		try {
			if (empty($bindVars) === TRUE) {
				$result = MatrixDAL::executeSqlAll($sql);
			} else {
				$prepare = MatrixDAL::preparePdoQuery($sql);
				foreach ($bindVars as $bindVar => $bindValue) {
					MatrixDAL::bindValueToPdo($prepare, $bindVar, $bindValue);
				}
				$result = MatrixDAL::executePdoAll($prepare);
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get message information due to database error: '.$e->getMessage());
		}//end try catch

		if ($count_only) {
			return $result[0]['message_count'];
		}

		for ($i = 0, $total = count($result); $i < $total; $i++) {
			$result[$i]['sent'] = iso8601_ts($result[$i]['sent']);

			// if we are showing the from name, check that any messages from shadow
			// assets are actually displaying the correct name
			if (is_string($get_user_name)) {
				$id_parts = explode(':', $result[$i]['userfrom']);
				if (isset($id_parts[1])) {
					$bridgeid = $id_parts[0];
					$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid, '', TRUE);
					if (!is_null($bridge)) {
						$shadow = $bridge->getAsset($result[$i]['userfrom']);
						if (!is_null($shadow)) {
							$result[$i]['from_name'] = $shadow->$get_user_name;
							$result[$i]['type_code'] = $shadow->type();
						}
					}
				}

				// if we couldnt find the name of the user, set a default
				if (empty($result[$i]['from_name']) && valid_email($result[$i]['userfrom'])) {
					if ($result[$i]['userfrom'] === SQ_CONF_DEFAULT_EMAIL) {
						$result[$i]['from_name'] = SQ_CONF_SYSTEM_NAME.' <'.$result[$i]['userfrom'].'>';
					} else {
						$result[$i]['from_name'] = $result[$i]['userfrom'];
					}
				} else if (empty($result[$i]['from_name'])) {
					$result[$i]['from_name'] = 'Unknown User';
				}

				// if we couldnt find the type code user, set a default
				if (empty($result[$i]['type_code'])) {
					$result[$i]['type_code'] = 'user';
				}
			}

			// We want to do the asset tagline replacements
			$body = $result[$i]['body'];
			$tagline_reg = '|<SQ_MSG_TAGLINE[^>]*?>(.*?)</SQ_MSG_TAGLINE>|';
			if (preg_match($tagline_reg, $body, $matches)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[1]);
				if (is_null($asset)) {
					$body = preg_replace($tagline_reg, 'AssetID #'.$matches[1], $body);
				} else {
					$body = preg_replace($tagline_reg, get_asset_tag_line($asset->id), $body);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				}
				$result[$i]['body'] = $body;
			}

		}//end for

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
		return $this->getMessages(0, $type, Array(), Array(), NULL, NULL, 'name', $parameters);

	}//end getLogs()


	/**
	* Returns the message object for the passed id
	*
	* It returns NULL on error or Internal_Message
	*
	* @param int	$messageid	id of the message the you want to get
	*
	* @return object
	* @access public
	*/
	function getMessageById($messageid)
	{
		$message = $this->newMessage();
		$message->load($messageid);
		if (!$message->id) return NULL;
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
	public static function sortMessages($a, $b)
	{
		if ($a['sent'] == $b['sent']) return 0;
		return ($a['sent'] > $b['sent']) ? -1 : 1;

	}//end sortMessages()


	/**
	* queue up an email to send at the conclusion of the script
	*
	* @param string	$from		the assetid of the user to send from (0 for system)
	* @param string	$to			the assetid of the user to send to (0 for system)
	* @param string	$subject	subject of the internal message
	* @param string	$body		body of the internal message
	*
	* @return void
	* @access public
	*/
	function queueEmail($from, $to, $subject, $body, $reply_to='')
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

		if (!isset($this->_emails['reply_to']) && !empty($reply_to)) {
			$this->_emails['reply_to'] = $reply_to;
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
		$mime_param = Array(
						'head_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'text_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
						'html_charset'	=> SQ_CONF_DEFAULT_CHARACTER_SET,
					  );
		$crlf = "\n";
		$mime = new Mail_mime($crlf);

		foreach ($this->_emails as $from => $from_emails) {
			if ($from === 'reply_to') continue;
			// work out the email address the the email will be sent from
			$from_email = SQ_CONF_DEFAULT_EMAIL;
			$from_string = '';
			if ($from && assert_valid_assetid($from, '', TRUE, FALSE)) {
				$user_from = $GLOBALS['SQ_SYSTEM']->am->getAsset($from);
				if (!is_null($user_from)) {
					$from_name = preg_replace('/[\[\]\(\)\<\>\@\,\;\:\\\"\.]+/','', $user_from->name);
					$from_email = $user_from->attr('email');
					$from_string = $from_name.' <'.$from_email.'>';
				}
			} else {
				$from_email = $from;
				// if the from name is specified, we should respect the format, e.g My Name <myemail@squiz.com>
				if(preg_match('/(.*) <([^ ]*)>/', $from_email, $matches) && isset($matches[1]) && isset($matches[2])) {
					$from_email = $matches[2];
					$from_string = $matches[1].' <'.$from_email.'>';
				}
			}
			if (empty($from_email)) {
				$from_email = SQ_CONF_DEFAULT_EMAIL;
			}
			if (empty($from_string)) {
				$from_string = '"'.SQ_SYSTEM_SHORT_NAME.' System" <'.$from_email.'>';
			}

			if (isset($this->_emails['reply_to']) && !isset($reply_to_str)) {
				$reply_to = $this->_emails['reply_to'];
				if (assert_valid_assetid($reply_to, '', TRUE, FALSE)) {
					$user_reply_to = $GLOBALS['SQ_SYSTEM']->am->getAsset($reply_to);
					if (!is_null($user_reply_to)) {
						$reply_to_name = preg_replace('/[\[\]\(\)\<\>\@\,\;\:\\\"\.]+/','', $user_reply_to->name);
						$reply_to_email = $user_reply_to->attr('email');
						$reply_to_str = $reply_to_name.' <'.$reply_to_email.'>';
					}
				} else {
					$reply_to_str = $reply_to;
				}
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

					// can't come up with a To: email address - bail for this address
					if (empty($to_email)) {
						// We want to skip this user
						continue;
					}

					$user = SQ_CONF_SYSTEM_OWNER;
				} else {
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($to);
					if (is_null($user) || !$user->canAccessBackend()) continue;
					$to_email = trim($user->attr('email'));
				}

				if (empty($to_email)) continue;

				foreach ($to_emails as $subject => $bodies) {

					$body = implode("\n\n".str_repeat('-', 50)."\n\n", $bodies);

					// workaround to fix the php5 strict warning
					// $mail = Mail::factory('mail', "-f$from_email");
					$mf = new Mail();
					$mail = $mf->factory('mail', "-f$from_email");

					$mime = new Mail_mime($crlf);

					$hdrs = Array(
								'From'		=> $from_string,
								'Subject'	=> $subject,
							);

					if (isset($reply_to_str) && $reply_to_str != '') $hdrs['Reply-To'] = $reply_to_str;

					$text_body = $this->formatEmail($body, $user, 'text');
					$html_body = $this->formatEmail($body, $user, 'html');

					$mime->setTXTBody(strip_tags($text_body));
					$mime->setHTMLBody($html_body);

					// get() must be called before headers()
					$body = $mime->get($mime_param);
					$hdrs = $mime->headers($hdrs);
					@$mail->send($to_email, $hdrs, $body);
					unset($mime);
					unset($mail);
				}
			}//end foreach
		}//end foreach

		// Remove sent emails
		unset($this->_emails);
		$this->_emails = Array();

	}//end sendQueuedEmails()


	/**
	* formats the body of an email with a header and a footer
	*
	* @param string	$body	the email body
	* @param mixed	&$user	either the user object or a string containing the user's name that the email is intended for
	* @param string	$type	the type of the email (text or html)
	*
	* @return string
	* @access public
	*/
	function formatEmail($body, &$user, $type='text')
	{
		$term = ($type=='text') ? "\n" : '<br />';

		$header = '';
		if (!is_null($user) && is_object($user) && ($user instanceof User)) {
			$header = $user->name.','.$term.$term;
		} else if (is_string($user) && $user != '') {
			$header = $user.','.$term.$term;
		}
		// replace separator line for multiple-bodied message with a h-rule
		if ($type == 'html') {
			$body = str_replace("\n\n".str_repeat('-', 50)."\n\n",'<hr />',$body);
		}

		// give it newlines
		if ($type == 'html') $body = nl2br($body);

		$line = ($type == 'text') ? str_repeat('-', 20).$term : '<hr />';

		if (!SQ_MS_EMAIL_FOOTER) {
			$footer = sprintf(translate('This is an automated message from %s running %s'), SQ_CONF_SYSTEM_NAME, SQ_SYSTEM_LONG_NAME).$term
					.translate('System Root URLs').' : '.$term.str_replace("\n", $term, SQ_CONF_SYSTEM_ROOT_URLS).$term
					.$term
					.sprintf(translate('Contact %s (%s) for support.'), SQ_CONF_SYSTEM_OWNER, SQ_CONF_TECH_EMAIL);
		} else {
			if ($type == 'html') {
				$footer = nl2br(SQ_MS_EMAIL_FOOTER);
			} else {
				$footer = preg_replace('/<br\s*\/?\s*>/i', "\n", SQ_MS_EMAIL_FOOTER);
			}

			// These keyword replacments are available
			$replacements = Array(
							'%system_name%' => SQ_CONF_SYSTEM_NAME,
							'%system_long_name%' => SQ_SYSTEM_LONG_NAME,
							'%system_root_urls_list%' => str_replace("\n", $term, SQ_CONF_SYSTEM_ROOT_URLS),
							'%system_owner_name%' => SQ_CONF_SYSTEM_OWNER,
							'%system_tech_email%' => SQ_CONF_TECH_EMAIL,
						);
			$footer = str_replace(array_keys($replacements), $replacements, $footer);
		}

		$footer = $term.$term.$line.$footer;

		return $header.$body.$footer;

	}//end formatEmail()


	/**
	* Gets all messages for the passed userid with user's "username", "first name" and "last name" info
	*
	* NOTE: This function is used by User::getMessages()
	*
	* @param string		$userid			Asset id of the user to get messages for
	* @param array		$statii			can limit returned messages to those with a status in this array
	* @param int		$max_msgs		the maximum number of messages
	* @param string		$type			the type of the message eg. asset.workflow or asset.attributes.*
	* @param array		$messageids		can limit returned messages to those with a messageid in this array
	* @param int		$from			timestamp that every msg returned must be this time or after (NULL to ignore)
	* @param int		$to				timestamp that every msg returned must be this time	or before (NULL to ignore)
	* @param boolean	$msgid_only		returns message id field only if set to true
	*
	* @return array
	* @access public
	*/
	function getInternalMessages($userid, $statii=Array(), $max_msgs=NULL, $type=NULL, $messageids=Array(), $from=NULL, $to=NULL, $msgid_only=FALSE)
	{
		$sql = 'SELECT
					msgid'.(!$msgid_only ? (', userto as to_assetid, userfrom as from_assetid, subject, body, sent, priority, type,
					CASE
						WHEN status=\'U\' THEN \'0\' ELSE \'1\'
					END as read') : '').'
				FROM
					sq_internal_msg
				WHERE';

		$where = ' userto = :userto';

		if (!is_null($type)) {
			if (substr($type, -2) == '.*') {
				$where .= ' AND type LIKE '.MatrixDAL::quote(substr($type, 0, -1).'%');
			} else {
				$where .= ' AND type = '.MatrixDAL::quote($type);
			}
		} else {
			// Trashed will contain both previous 'sent' and 'inbox' messages
			if (!in_array(SQ_MSG_DELETED, $statii)) {
				$where .= ' AND type <> \'inbox.sent\'';
			}
		}
		if (!empty($statii)) {
			$statii_strs = Array();
			for ($i = 0; $i < count($statii); $i++) {
				$statii[$i] = '\''.$statii[$i].'\'';
			}
			$where .= ' AND status IN ('.implode(', ', $statii).')';
		}
		if (!empty($messageids)) {
			for ($i = 0; $i < count($messageids); $i++) {
				$messageids[$i] = '\''.$messageids[$i].'\'';
			}
			$where .= ' AND msgid IN ('.implode(', ', $messageids).')';
		}
		if (!is_null($from)) {
			$where .= ' AND sent >= '.MatrixDAL::quote(ts_iso8601($from));
		}
		if (!is_null($to)) {
			$where .= ' AND sent <= '.MatrixDAL::quote(ts_iso8601($to));
		}
		$sql .= ' '.$where;
		$sql .= ' ORDER BY sent DESC';

		// '-1' size means get all the messages
		if ($max_msgs != '-1') {
			// Number of messages to return if this parameter is not supplied
			$max_msgs = !$max_msgs ? 100 : $max_msgs;
			$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), $max_msgs);
		}

		try {
			$prepare = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($prepare, 'userto', $userid);
			$message_result = MatrixDAL::executePdoAssoc($prepare);
		} catch (Exception $e) {
			throw new Exception('Unable to get message information due to database error: '.$e->getMessage());
		}//end try catch

		// If just message id field is required then we are done here
		if ($msgid_only) {
			$msg_ids = Array();
			foreach($message_result as $row) {
				if (isset($row['msgid'])) {
					$msg_ids[] = $row['msgid'];
				}
			}
			return $msg_ids;
		}

		// Userids in the messages above. Both "to" and "from" user
		$userids = Array();
		foreach ($message_result as $index => $val) {

			// We dont get details for shadow assets for the performance reasons
			// TODO: If this is required later
			if (strpos($message_result[$index]['to_assetid'], ':') === FALSE) {
				$userids[$message_result[$index]['to_assetid']] = MatrixDAL::quote($message_result[$index]['to_assetid']);
			}
			if (strpos($message_result[$index]['from_assetid'], ':') === FALSE) {
				$userids[$message_result[$index]['from_assetid']] = MatrixDAL::quote($message_result[$index]['from_assetid']);
			}

			// Do the asset tagline replacements
			$body = $message_result[$index]['body'];
			$tagline_reg = '|<SQ_MSG_TAGLINE[^>]*?'.'>(.*?)</SQ_MSG_TAGLINE>|';
			if (preg_match($tagline_reg, $body, $matches)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[1]);
				if (is_null($asset)) {
					$body = preg_replace($tagline_reg, 'AssetID #'.$matches[1], $body);
				} else {
					$body = preg_replace($tagline_reg, get_asset_tag_line($asset->id), $body);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				}
				$message_result[$index]['body'] = $body;
			}
		}//end foreach

		if (!empty($userids)) {
			// Get the user detials from the db
			$sql = 'SELECT
						v.assetid, a.name, v.custom_val
					FROM
						sq_ast_attr a, sq_ast_attr_val v
					WHERE
						a.attrid=v.attrid AND
						a.name IN (\'username\', \'first_name\', \'last_name\') AND
						v.assetid IN ('.implode(',',$userids).') AND
						contextid = :contextid';

			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
			try {
				$prepare = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($prepare, 'contextid', $contextid);
				$result = MatrixDAL::executePdoAssoc($prepare);
			} catch (Exception $e) {
				throw new Exception('Unable to get message information due to database error: '.$e->getMessage());
			}//end try catch

			$user_info = Array();
			foreach($result as $row) {
				$user_info[$row['assetid']][$row['name']] = $row['custom_val'];
			}//end foreach

			foreach($message_result as $index => $row) {
				if (isset($user_info[$row['to_assetid']])) {
					$message_result[$index]['to_username'] = isset($user_info[$row['to_assetid']]['username']) ? $user_info[$row['to_assetid']]['username'] : '';
					$message_result[$index]['to_first_name'] = isset($user_info[$row['to_assetid']]['first_name']) ? $user_info[$row['to_assetid']]['first_name'] : '';
					$message_result[$index]['to_last_name'] = isset($user_info[$row['to_assetid']]['last_name']) ? $user_info[$row['to_assetid']]['last_name'] : '';
				} else {
					$message_result[$index]['to_username'] = '';
					$message_result[$index]['to_first_name'] = '';
					$message_result[$index]['to_last_name'] = '';
				}

				if (isset($user_info[$row['from_assetid']])) {
					$message_result[$index]['from_username'] = isset($user_info[$row['from_assetid']]['username']) ? $user_info[$row['from_assetid']]['username'] : '';
					$message_result[$index]['from_first_name'] = isset($user_info[$row['from_assetid']]['first_name']) ? $user_info[$row['from_assetid']]['first_name'] : '';
					$message_result[$index]['from_last_name'] = isset($user_info[$row['from_assetid']]['last_name']) ? $user_info[$row['from_assetid']]['last_name'] : '';
				} else {
					$message_result[$index]['from_username'] = '';
					$message_result[$index]['from_first_name'] = '';
					$message_result[$index]['from_last_name'] = '';
				}
			}//end foreach

			unset($user_info);

		}//end if !empty(userids)

		unset($userids);

		return $message_result;

	}//end getInternalMessages()


}//end class

?>
