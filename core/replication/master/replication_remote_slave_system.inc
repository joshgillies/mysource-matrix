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
* $Id: replication_remote_slave_system.inc,v 1.1.2.1 2004/02/14 12:38:13 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replication_remote_system.inc';

/**
* Replication_Remote_Slave_System
*
* Purpose
*
*    To represent a remote system in the replication framework of the master system
*    Includes taking care of Authentication and Managing the connections between the systems
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_Remote_Slave_System extends Replication_Remote_System
{
	/**
	* The timestamp for when this system was created
	* @var int
	*/
	var $created = null;

	/**
	* The timestamp for when this system was retired
	* @var int
	*/
	var $retired = null;


	/**
	* Constructor
	*
	* @param string	$systemid	the id of the system that we are representing
	*
	*/
	function Replication_Remote_Slave_System($systemid='')
	{
		$this->Replication_Remote_System();
		if($systemid) $this->load($systemid);

	}//end constructor


	/**
	* Creates a new Remote System by creating a config file for it
	*
	* @param string	$name				the text name of the system that we will be representing
	* @param string	$auth_type			the authentication type that is going to be used
	* @param string	$transport_method	the transport method that is going to be used
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function create($name, $auth_type, $transport_method)
	{
		if (true !== ($err_msg = Replicator::validSystemName($name))) {
			trigger_error('Unable to create Replication Remote System "'.$name.'" : System Name Invalid ('.$err_msg.')', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT COUNT(*) 
				FROM '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				WHERE name = '.$db->quote($name);

		$result = $db->getOne($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (!empty($result)) {
			trigger_error('Unable to create Replication Remote System "'.$name.'" : System Name has already been used', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$systemid = $db->nextId(SQ_TABLE_PREFIX.'sequence_replication_slave');
		if (DB::isError($systemid)) {
			trigger_error($systemid->getMessage().'<br/>'.$systemid->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$now = time();

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				(systemid, name, created, retired)
				VALUES
				('.$db->quote($systemid).', '.$db->quote($name).', '.$db->quote(ts_iso8601($now)).', NULL)';
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->id      = $systemid;
		$this->name    = $name;
		$this->created = $now;
		$this->retired = null;

		if (!$this->setAuthType($auth_type)) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : Unable to set Authentication Type', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		if (!$this->setTransportMethod($transport_method)) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : Unable to set Transport Method', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		if (!$this->save()) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : unable to save', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end create()


	/**
	* Returns the conf file for this remote system
	*
	* @return string
	* @access public
	* @see save(), load()
	*/
	function confFile()
	{
		if (is_null($this->id)) {
			trigger_error('Unable to get config filename : No System Id', E_USER_WARNING);
			return '';
		}
		return SQ_DATA_PATH.'/private/conf/replication/'.$this->id.'_'.$this->name.'.inc';

	}// end confFile()


	/**
	* Loads a remote system into this instance
	*
	* @param string	$systemid	the id of the system to load
	*
	* @return boolean
	* @access public
	* @see save()
	*/
	function load($systemid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT systemid, name, created, retired
				FROM '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				WHERE systemid = '.$db->quote($systemid);

		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) return;

		list($this->id,
			 $this->name,
			 $created,
			 $retired) = $result;
		$this->created = iso8601_ts($created);
		$this->retired = ($retired != '') ? iso8601_ts($retired) : null;
		unset($result);

		if (is_null($this->retired)) {
			return parent::load();
		} else {
			return true;
		}// end if

	}// end load()


	/**
	* Retires a remote system out of server
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function retire()
	{
		if (is_null($this->id)) return false;

		if ($this->isRetired()) {
			trigger_error('Unable to retire Replication Remote System "'.$this->id.'" : System Already Retired', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$now = time();
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				  SET retired = '.$db->quote(ts_iso8601($now)).'
				WHERE systemid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$conf_file = $this->confFile();
		if ($conf_file != '' && file_exists($conf_file) && !unlink($conf_file)) {
			trigger_error('Unable to retire Replication Remote System "'.$this->id.'" : unable to remove config file', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->retired = $now;

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end retire()


	/**
	* Is this remote system retired ?
	*
	* @return boolean	true if retired, false if not
	* @access public
	*/
	function isRetired()
	{
		return !is_null($this->retired);
	}// end isRetired()

}//end class

?>