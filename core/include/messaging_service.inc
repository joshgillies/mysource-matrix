<?php

define('SQ_MSG_UNREAD' , 'U');
define('SQ_MSG_READ'   , 'R');
define('SQ_MSG_DELETED', 'D');

define('SQ_MSG_PRIORITY_LOW'    , 'L');
define('SQ_MSG_PRIORITY_NORMAL' , 'N');
define('SQ_MSG_PRIORITY_HIGH'   , 'H');

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
	* Constructor
	*
	*/
	function Messaging_Service()
	{
		$this->Resolve_Object();
	}//end constructor

	
	/**
	* Sends an internal message
	*
	* @param int	$to			userid of the recipient
	* @param int	$from		userid of the sender
	* @param string	$subject	subject of the message
	* @param string	$body		main body of the message
	* @param string	$priority	message priority (see constants for SQ_MSG_PRIOIRTY_*)
	* @param string	$status		status of the message (see constants for SQ_MSG_*)
	* @param array	$parameters	array of parameters to use for special messages
	*
	* @return boolean
	* @access public
	*/
	function sendMessage($to, $from, $subject, $body, $priority=SQ_MSG_PRIORITY_NORMAL, $status=SQ_MSG_UNREAD, $parameters=Array())
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$messageid = $db->nextId('sq_sequence_internal_message');
		if (DB::isError($messageid)) trigger_error($messageid->getMessage().'<br/>'.$messageid->getUserInfo(), E_USER_ERROR);

		$now = time();
		$parameters = serialize($parameters);

		$sql = 'INSERT INTO sq_internal_message
				(messageid, userto, userfrom, subject, body, priority, status, sent, parameters)
				VALUES
				('.$db->quote($messageid).','.$to.', '.$from.', '.$db->quote($subject).', '.$db->quote($body).', '.$db->quote($priority).', '.$db->quote($status).', '.$db->quote(ts_iso8601($now)).', '.$db->quote($parameters).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end sendMessage()


	/**
	* Deletes an internal message FOREVER - be careful
	*
	* @param int	$messageid	id of the message to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteMessage($messageid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM sq_internal_message WHERE messageid='.$db->quote($messageid);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end deleteMessage()


	/**
	* Gets all messages for the passed userid
	*
	* @param int			$userid	id of the user you are fetching messages for
	* @param array(string)	$stati	can limit returned messages to those with a
	*								status in this array
	*
	* @return array
	* @access public
	*/
	function getMessages($userid, $stati=Array())
	{
		if (!$userid) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT * FROM sq_internal_message WHERE userto='.$db->quote($userid);
		if (!empty($stati)) {
			$status_string = implode("', '", $stati);
			$sql .= " AND status IN ('".$status_string."')";
		}
		$sql .= ' ORDER BY status DESC, sent DESC';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return $result;
	}//end getMessages()


	/**
	* Update the status of a message
	*
	* @param int	$messageid	id of the message to update
	* @param string	$status		the new status (see constants for SQ_MSG_*)
	*
	* @return boolean
	* @access public
	*/
	function updateMessageStatus($messageid, $status)
	{
		if (!$messageid) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'UPDATE sq_internal_message SET status='.$db->quote($status).' WHERE messageid='.$db->quote($messageid);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}
		return true;
	}//end updateMessageStatus()

}//end class
?>
