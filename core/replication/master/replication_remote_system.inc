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
* $Id: replication_remote_system.inc,v 1.1.2.1 2004/02/13 17:18:58 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Replication_Remote_System
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
class Replication_Remote_System extends MySource_Object
{
	/**
	* The uniq replication system id that has been given to this remote system
	* @var int
	*/
	var $id = 0;

	/**
	* The uniq test string for this remote system
	* @see Replicator::validSystemName()
	* @var string
	*/
	var $name = '';

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
	* The authentication object used to authenticate this server
	* @var object Replication_System_Authentication
	*/
	var $auth = null;

	/**
	* The transport object used to communicate with this server
	* @var object Replication_System_Transport
	*/
	var $transport = null;

	/**
	* Details for the squiz server on this remote server that we will use to connect to
	* Maybe left blank for servers with dynamic IP
	* @var string
	*/
	var $squiz_server = Array(
							'systemid'	=> '',
							'host'		=> '',
							'port'		=> 0,
						);

	/**
	* Whether the master server normally initiates replication to this server
	* If false, then the slave server is expected to initiate the connection
	* @var boolean
	*/
	var $master_initiates = true;

	/**
	* The amount of time (in seconds) that is be between syncronisation runs
	* If the master server is not the initiator this is only an expected figure
	* @var int
	*/
	var $interval = 0;


	/**
	* Constructor
	*
	* @param string	$systemid	the id of the system that we are representing
	*
	*/
	function Replication_Remote_System($systemid='')
	{
		$this->MySource_Object();
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
				FROM '.SQ_TABLE_PREFIX.'replication_remote_system
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

		$systemid = $db->nextId(SQ_TABLE_PREFIX.'sequence_remote_system');
		if (DB::isError($systemid)) {
			trigger_error($systemid->getMessage().'<br/>'.$systemid->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$now = time();

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'replication_remote_system
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
			return false;
		}

		if (!$this->setTransportMethod($transport_method)) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : Unable to set Transport Method', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$this->save()) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : unable to save', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end create()


	/**
	* Saves this remote system
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save()
	{
		if (!$this->id) return false;

		if (is_null($this->auth) || is_null($this->transport)) {
			trigger_error('Unable to save Remote System, either System Name, Auth Type or Transport Method not defined', E_USER_WARNING);
			return false;
		}

		$config = Array(
					'auth_type'			=> $this->auth->type(),
					'auth_data'			=> $this->auth->extractParams(),
					'transport_method'	=> $this->transport->method(),
					'transport_data'	=> $this->transport->extractParams(),
					'squiz_server'		=> $this->squiz_server,
					'master_initiates'	=> $this->master_initiates,
					'interval'			=> $this->interval,
				);
		$str = '<'."?php\n".'$config = '.var_export($config, true).";\n?".">\n";

		$config_file = SQ_DATA_PATH.'/private/conf/replication/'.$this->id.'_'.$this->name.'.inc';

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory(dirname($config_file))) return false;
		// and save the file
		return string_to_file($str, $config_file);

	}// end save()


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
				FROM '.SQ_TABLE_PREFIX.'replication_remote_system
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
			$config_file = SQ_DATA_PATH.'/private/conf/replication/'.$this->id.'_'.$this->name.'.inc';
			if (!file_exists($config_file)) {
				trigger_error('Config file doesn\'t exist for Remote System "'.$systemid.'"', E_USER_WARNING);
			} else {
				// include the file and hopefully there is a var called $config
				require_once $config_file;

				if (!isset($config) || !is_array($config)) {
					trigger_error('Unable to load Remote System "'.$systemid.'", config var doesn\'t exist', E_USER_WARNING);
					$this->id = 0;
					return false;
				}

				if ($config['auth_type'] != '') {
					require_once SQ_SYSTEM_ROOT.'/core/replication/authentication/replication_system_authentication_'.$config['auth_type'].'.inc';
					$class = 'replication_system_authentication_'.$config['auth_type'];
					$this->auth = new $class();
					if (!$this->auth->loadParams($config['auth_data'])) {
						unset($this->auth);
						$this->auth = null;
						$this->id = 0;
						return false;
					}
				}
				if ($config['transport_method'] != '') {
					require_once SQ_SYSTEM_ROOT.'/core/replication/transport/replication_system_transport_'.$config['transport_method'].'.inc';
					$class = 'replication_system_transport_'.$config['transport_method'];
					$this->transport = new $class();
					if (!$this->transport->loadParams($config['transport_data'])) {
						unset($this->transport);
						$this->transport = null;
						$this->id = 0;
						return false;
					}
				}

				$this->squiz_server		= $config['squiz_server'];
				$this->master_initiates	= (bool) $config['master_initiates'];
				$this->interval			= (int) $config['interval'];

			}// end if file_exists()
		}// end if isnull retired

		return true;

	}// end load()


	/**
	* Retires a remote system out of server
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function retire()
	{
		if ($this->isRetired()) {
			trigger_error('Unable to retire Replication Remote System "'.$this->id.'" : System Already Retired', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$now = time();
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'replication_remote_system
				  SET retired = '.$db->quote(ts_iso8601($now)).'
				WHERE systemid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$conf_file = SQ_DATA_PATH.'/private/conf/replication/'.$this->id.'_'.$this->name.'.inc';
		if (file_exists($conf_file) && !unlink($conf_file)) {
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


	/**
	* Set's the Authentication type for this remote system
	*
	* @param string	$auth_type	the name of the type
	*
	* @return boolean	true on success
	* @access public
	*/
	function setAuthType($auth_type)
	{
		$auth_types = Replicator::getAuthTypes();
		if (!isset($auth_types[$auth_type])) {
			trigger_error('Unable to set Authentication Type for "'.$this->id.'" : Authentication Type "'.$auth_type.'" doesn\'t exist', E_USER_WARNING);
			return false;
		}

		// if we are already set to this, don't override it with a new version
		if (!is_null($this->auth) && $this->auth->type() == $auth_type) return true;

		// create a brand new version
		require_once SQ_SYSTEM_ROOT.'/core/replication/authentication/replication_system_authentication_'.$auth_type.'.inc';
		$class = 'replication_system_authentication_'.$auth_type;
		$this->auth = new $class();

		return true;

	}// end setAuthType()


	/**
	* Set's the Transport method for this remote system
	*
	* @param string	$transport_method	the name of the method
	*
	* @return boolean	true on success
	* @access public
	*/
	function setTransportMethod($transport_method)
	{
		$transport_methods = Replicator::getTransportMethods();
		if (!isset($transport_methods[$transport_method])) {
			trigger_error('Unable to set Transport Method for "'.$this->id.'" : Transport Method "'.$transport_method.'" doesn\'t exist', E_USER_WARNING);
			return false;
		}

		// if we are already set to this, don't override it with a new version
		if (!is_null($this->transport) && $this->transport->method() == $transport_method) return true;

		// create a brand new version
		require_once SQ_SYSTEM_ROOT.'/core/replication/transport/replication_system_transport_'.$transport_method.'.inc';
		$class = 'replication_system_transport_'.$transport_method;
		$this->transport = new $class();

		return true;

	}// end setTransportMethod()

}//end class

?>