<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: messaging_service.inc,v 1.21 2003/09/30 02:25:42 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_DATA_PATH.'/private/conf/messaging_service.inc';

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
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Messaging_Service extends MySource_Object
{
	/**
	* Temporary holder for messages added to queues and logs
	*
	* @var Array()
	*/
	var $_levels = Array(	'queues' => Array(),
							'logs'   => Array()
						);


	/**
	* Constructor
	*
	*/
	function Messaging_Service()
	{
		$this->MySource_Object();

	}//end constructor


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
			foreach($msgs as $msg) {
				array_push($this->_levels['queues'][count($this->_levels['queues']) - 1], $msg);
			}
			return true;

		}// end if

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
			foreach($msgs as $msg) {
				array_push($this->_levels['logs'][count($this->_levels['logs']) - 1], $msg);
			}
			return true;

		}// end if

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
	* Adds an internal message into the queue
	*
	* @param object Internal_Message	$message	the message to add to the queue
	*
	* @return void
	* @access public
	*/
	function logMessage($message)
	{
		array_push($this->_levels['logs'][count($this->_levels['logs']) - 1], $message);

	}//end enqueueMessage()


//--        MESSAGES        --//


	/**
	* Creates and returns a new internal message
	*
	* Some variables of a message can be passed in for simple messages
	* but messages that want to utilise other options need to set these manually
	* so as to not clutter this functions
	*
	* @param Array(int)	$to			an array of user or group IDs to send this message to
	* @param string		$subject	the subject of the message (keep it short)
	* @param string		$body		the main body of the message
	* @param string		$type		the log type of this message (if any)
	*
	* @return object Interal_Message
	* @access public
	*/
	function newMessage($to=Array(), $subject='', $body='', $type='')
	{
		require_once SQ_INCLUDE_PATH.'/internal_message.inc';
		$new_message = new Internal_Message();
		$new_message->sent    = ts_iso8601(time());
		$new_message->from    = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$new_message->to      = $to;
		$new_message->subject = $subject;
		$new_message->body    = $body;
		$new_message->type    = $type;
		return $new_message;

	}//end newMessage()


	/**
	* Sends all messages passed in, doing some amalgamation of msg bodies for same user and same subject
	*
	* @param Array()	$msgs	an array of Internal_Message to send
	*
	* @return void
	* @access public
	*/
	function send($msgs)
	{
		// lets try and work out if any of the messages to be
		// sent can actually be amalgamated based on who the message is being
		// sent to, who it is from, the message type, and the subject line of the message
		$send_queue = Array();
		foreach ($msgs as $msg) {
			$to_ids = $msg->expandUsersTo();
			foreach ($to_ids as $assetid) {
				$send_queue[$assetid][$msg->type][$msg->subject][$msg->from]['bodies'][] = $msg->body;
				// make sure we set the highest priority in the group
				if (!isset($send_queue[$assetid][$msg->type][$msg->subject][$msg->from]['priority'])) {
					$send_queue[$assetid][$msg->type][$msg->subject][$msg->from]['priority'] = $msg->priority;
				} else {
					if ($msg->priority > $send_queue[$assetid][$msg->type][$msg->subject][$msg->from]['priority']) {
						$send_queue[$assetid][$msg->type][$msg->subject][$msg->from]['priority'] = $msg->priority;
					}
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($send_queue as $userid => $message_data) {
			foreach ($message_data as $type => $type_data) {
				foreach ($type_data as $subject => $subject_data) {
					foreach ($subject_data as $from => $from_data) {
						$message = $this->newMessage();
						$message->to = Array($userid);
						$message->from = $from;
						$message->subject = $subject;
						$message->body = implode("\n\n".str_repeat('-', 50)."\n\n", $from_data['bodies']);
						$message->send();
					}
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end send()


	/**
	* Gets all messages for the passed userid
	*
	* @param int			$userid			id of the user you are fetching messages for
	* @param string			$type			the type of the message eg. asset.attributes
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

		if (!is_null($get_user_name) && $get_user_name != 'name') 
			$get_user_name = 'short_name';

		$sql = 'SELECT m.messageid, m.userto, m.userfrom, m.subject, m.body, m.sent, m.priority, m.status, m.type, m.parameters';
		if (is_string($get_user_name)) {
			$sql .= ',
					CASE
						WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = 0 THEN '.$db->quote(SQ_SYSTEM_SHORT_NAME.' System').'
						ELSE a.'.$get_user_name.'
					END as from_name';
			$sql  .=	', CASE WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = 0 THEN \'root_user\''.
					' ELSE a.type_code END as type_code';
		}

		$sql  .= ' FROM '.SQ_TABLE_RUNNING_PREFIX.'internal_message m';
		$where = 'm.userto = '.$db->quote($userid);

		if (is_string($get_user_name)) {
			$sql .= ' LEFT OUTER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON m.userfrom = a.assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		}

		if (!is_null($type)) $where .= ' AND m.type = '.$db->quote($type);
		if (!empty($statii)) {
			for($i = 0; $i < count($statii); $i++) $statii[$i] = $db->quote($statii[$i]);
			$where .= ' AND m.status IN ('.implode(', ', $statii).')';
		}
		if (!empty($messageids)) {
			for($i = 0; $i < count($messageids); $i++) $messageids[$i] = $db->quote($messageids[$i]);
			$where .= ' AND m.messageid IN ('.implode(', ', $messageids).')';
		}
		if (!empty($parameters)) {
			foreach ($parameters as $param_key => $param_value) {
				$where .= ' AND m.parameters LIKE '.$db->quote('%'.$param_key.':'.$param_value.';%');
			}
		}
		if (!is_null($from)) $where .= ' AND m.sent >= '.$db->quote(ts_iso8601($from));
		if (!is_null($to))   $where .= ' AND m.sent <= '.$db->quote(ts_iso8601($to));

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'm');
		$sql .= ' '.$where.' ORDER BY m.sent DESC';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		for($i = 0, $total = count($result); $i < $total; $i++) {
			$result[$i]['sent'] = iso8601_ts($result[$i]['sent']);
		}

		return $result;

	}//end getMessages()


	/**
	* Get logged messages of the passed typed
	*
	* This is just a cut down version of getMessages so you can get logs without
	* having to call getMessages will a bunch of Array() and NULL arguments
	*
	* @param string			$type			the type of the message eg. asset.attributes
	* @param array			$parameters		can limit returned messages to those with these params set
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
	* @param int	$messageid		id of the message the you want to get
	*
	* @return object Internal_Message
	* @access public
	*/
	function getMessageById($messageid)
	{
		$message = $this->newMessage();
		if ($messageid) $message->load($messageid);
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

}//end class

?>