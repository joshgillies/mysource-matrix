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
	* @param array(int)	$to			array of users or user groups
	* @param int		$from		userid of the sender
	* @param string		$subject	subject of the message
	* @param string		$body		main body of the message
	* @param string		$priority	message priority (see constants for SQ_MSG_PRIOIRTY_*)
	* @param string		$status		status of the message (see constants for SQ_MSG_*)
	* @param array		$parameters	array of parameters to use for special messages
	*
	* @return boolean
	* @access public
	*/
	function sendMessage($to=Array(), $from, $subject, $body, $priority=SQ_MSG_PRIORITY_NORMAL, $status=SQ_MSG_UNREAD, $parameters=Array())
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$now = time();
		$parameters = serialize($parameters);

		$send_to = Array();
		$to_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo($to, Array('user', 'user_group'), false);
		foreach ($to_types as $assetid => $types) {
			if (in_array('user_group', $types)) {
				$group = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if ($group->id) $send_to = array_merge($send_to, $group->getChildren(Array('user'), false));
			} else {
				$send_to[] = $assetid;
			}
		}
		$send_to = array_unique($send_to);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($send_to as $userid) {
			$messageid = $db->nextId('sq_sequence_internal_message');
			if (DB::isError($messageid)) {
				trigger_error($messageid->getMessage().'<br/>'.$messageid->getUserInfo(), E_USER_ERROR);
			}

			$sql = 'INSERT INTO sq_internal_message
					(messageid, userto, userfrom, subject, body, priority, status, sent, parameters)
					VALUES
					('.$db->quote($messageid).','.$userid.', '.$from.', '.$db->quote($subject).', 
					 '.$db->quote($body).', '.$db->quote($priority).', '.$db->quote($status).', 
					 '.$db->quote(ts_iso8601($now)).', '.$db->quote($parameters).')';

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
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
	* @param int			$userid			id of the user you are fetching messages for
	* @param array(string)	$stati			can limit returned messages to those with a status 
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
	function getMessages($userid, $stati=Array(), $from=null, $to=null, $get_user_name=null)
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
		if (!empty($stati)) {
			for($i = 0; $i < count($stati); $i++) $stati[$i] = $db->quote($stati[$i]);
			$sql .= ' AND m.status IN ('.implode(', ', $stati).')';
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
