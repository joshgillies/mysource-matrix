<?php

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
* @package Resolve
*/
class Messaging_Service extends Resolve_Object
{

	/**
	* An array of message objects to send
	* @var Array(Internal_Message)
	*/
	var $message_queue;

	/**
	* How many levels of queues we are deep
	*
	* @var string()
	*/
	var $_levels;


	/**
	* Constructor
	*
	*/
	function Messaging_Service()
	{
		$this->Resolve_Object();
		$this->message_queue = Array();
		$this->_levels = 0;

	}//end constructor


	/**
	* Creates and returns a new internal message
	*
	* @return object Interal_Message
	* @access public
	*/
	function newMessage()
	{
		require_once SQ_INCLUDE_PATH.'/internal_message.inc';
		$new_message = new Internal_Message();
		return $new_message;

	}//end newMessage()


	/**
	* Opens a new queue or nested queue
	* Note that this function really just updates how many queues are open
	*
	* @return boolean
	* @access public
	*/
	function openQueue()
	{
		return $this->_levels++;

	}//end openQueue()


	/**
	* Closes a queue or nested queue
	* If the last queue is closed, the messages in the queue are sent
	*
	* @return boolean
	* @access public
	*/
	function closeQueue()
	{
		$this->_levels--;
		if ($this->_levels == 0) {
			// this is the last queue
			return $this->sendAll();
		}
		return true;

	}//end closeQueue()


	/**
	* Adds an internal message into the queue
	*
	* @param object Internal_Message	$message	the message to add to the queue
	*
	* @return boolean
	* @access public
	*/
	function enqueueMessage($message)
	{
		return $this->message_queue[] = $message;

	}//end enqueueMessage()


	/**
	* Sends all messages waiting in the queue
	*
	* @return object Interal_Message
	* @access public
	*/
	function sendAll()
	{
		// lets try and work out if any of the messages to be
		// sent can actually be amalgamated based on who the message is being
		// sent to, who it is from, and the subject line of the message
		$send_queue = Array();
		foreach ($this->message_queue as $msg) {
			$to_ids = $msg->expandUsersTo();
			foreach ($to_ids as $assetid) {
				$send_queue[$assetid][$msg->subject][$msg->from]['bodies'][] = $msg->body;
				// make sure we set the highest priority in the group
				if ($msg->priority > $send_queue[$assetid][$msg->subject][$msg->from]['priority']) {
					$send_queue[$assetid][$msg->subject][$msg->from]['priority'] = $msg->priority;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($send_queue as $userid => $message_data) {
			foreach ($message_data as $subject => $subject_data) {
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// clean out the message queue
		$this->message_queue = Array();

		return true;

	}//end sendAll()


	/**
	* Gets all messages for the passed userid
	*
	* @param int			$userid			id of the user you are fetching messages for
	* @param array(string)	$statii			can limit returned messages to those with a status 
	*										in this array
	* @param int			$from			timestamp that every msg returned must be this time 
	*										or after (NULL to ignore)
	* @param int			$to				timestamp that every msg returned must be this time 
	*										or before (NULL to ignore)
	* @param string			$get_from_name	[name|short_name], used to return the name/short_name 
	*										of the from user with the return, which will be returned 
	*										in the field 'from_name'. (NULL to ignore)
	*
	* @return array
	* @access public
	*/
	function getMessages($userid, $statii=Array(), $from=null, $to=null, $get_user_name=null)
	{
		if (!$userid) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if (!is_null($get_user_name) && $get_user_name != 'name') $get_user_name = 'short_name';

		$sql = 'SELECT m.messageid, m.userto, m.userfrom, m.subject, m.body, m.sent, m.priority, m.status, m.parameters';
		if (is_string($get_user_name)) {
			$sql .= ', 
					CASE 
						WHEN a.'.$get_user_name.' IS NULL AND m.userfrom = 0 THEN '.$db->quote(SQ_SYSTEM_SHORT_NAME.' System').'
						ELSE a.'.$get_user_name.'
					END as from_name';
		}
		$sql .= ' FROM sq_internal_message m';
		if (is_string($get_user_name)) $sql .= ' LEFT OUTER JOIN sq_asset a ON m.userfrom = a.assetid';

		$sql .= ' WHERE m.userto = '.$db->quote($userid);
		if (!empty($statii)) {
			for($i = 0; $i < count($statii); $i++) $statii[$i] = $db->quote($statii[$i]);
			$sql .= ' AND m.status IN ('.implode(', ', $statii).')';
		}
		if (!is_null($from)) $sql .= ' AND m.sent >= '.$db->quote(ts_iso8601($from));
		if (!is_null($to))   $sql .= ' AND m.sent <= '.$db->quote(ts_iso8601($to));

		$sql .= ' ORDER BY m.sent DESC';
		
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

}//end class

?>