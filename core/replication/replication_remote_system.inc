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
* $Id: replication_remote_system.inc,v 1.1.2.1 2004/02/14 12:38:43 brobertson Exp $
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
* @abstract
*/
class Replication_Remote_System extends MySource_Object
{
	/**
	* The uniq replication system id that has been given to this remote system
	* @var int
	*/
	var $id = null;

	/**
	* The uniq test string for this remote system
	* @see Replicator::validSystemName()
	* @var string
	*/
	var $name = '';

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
	function Replication_Remote_System()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Saves this remote system
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save()
	{
		if (is_null($this->id)) return false;

		if (is_null($this->auth) || is_null($this->transport)) {
			trigger_error('Unable to save Remote System : either System Name, Auth Type or Transport Method not defined', E_USER_WARNING);
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

		$config_file = $this->confFile();
		if ($config_file == '') {
			trigger_error('Unable to save Remote System : config file not set', E_USER_WARNING);
			return false;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory(dirname($config_file))) return false;
		// and save the file
		return string_to_file($str, $config_file);

	}// end save()


	/**
	* Returns the conf file for this remote system
	*
	* @return string
	* @access public
	* @see save(), load()
	* @abstract
	*/
	function confFile()
	{
		trigger_error('You need to override '.__CLASS__.'::'.__FUNCTION__.'()', E_USER_ERROR);
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
	function load()
	{
		$config_file = $this->confFile();
		if ($config_file == '') {
			trigger_error('Unable to load Remote System : config file not set', E_USER_WARNING);
			return false;
		}

		if (!file_exists($config_file)) {
			trigger_error('Unable to load Remote System : Config file doesn\'t exist for Remote System', E_USER_WARNING);
			return false;
		}

		// include the file and hopefully there is a var called $config
		require $config_file;

		if (!isset($config) || !is_array($config)) {
			trigger_error('Unable to load Remote System : config var doesn\'t exist', E_USER_WARNING);
			$this->id = null;
			return false;
		}

		if ($config['auth_type'] != '') {
			require_once SQ_SYSTEM_ROOT.'/core/replication/authentication/replication_system_authentication_'.$config['auth_type'].'.inc';
			$class = 'replication_system_authentication_'.$config['auth_type'];
			$this->auth = new $class();
			if (!$this->auth->loadParams($config['auth_data'])) {
				unset($this->auth);
				$this->auth = null;
				$this->id = null;
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
				$this->id = null;
				return false;
			}
		}

		$this->squiz_server		= $config['squiz_server'];
		$this->master_initiates	= (bool) $config['master_initiates'];
		$this->interval			= (int) $config['interval'];

		return true;

	}// end load()


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
			trigger_error('Unable to set Authentication Type for Remote System "'.$this->id.'" : Authentication Type "'.$auth_type.'" doesn\'t exist', E_USER_WARNING);
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
			trigger_error('Unable to set Transport Method for Remote System "'.$this->id.'" : Transport Method "'.$transport_method.'" doesn\'t exist', E_USER_WARNING);
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