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
* $Id: hipo_client.inc,v 1.7 2003/11/26 00:51:09 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_DATA_PATH.'/private/conf/hipo.inc';
require_once SQ_SYSTEM_ROOT.'/core/hipo/server/hipo_server_common.inc';

/**
* HIPO_Client
*
* @author Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class HIPO_Client extends HIPO_Server_Common
{

	/**
	* domain to connect to
	* @var string
	*/
	var $_domain = 'localhost';

	/**
	* port to connect to
	* @var integer
	*/
	var $_port = 10000;

	/**
	* the file (socket) descriptor
	* @var resource
	*/
	var $_fd = null;

	/**
	* create a new client
	*
	* @param	string	 $domain			domain to connect to
	* @param	integer	 $port				port to connect on
	*
	* @access	public
	*/
	function HIPO_Client($domain=SQ_HIPO_SERVER_DOMAIN, $port=SQ_HIPO_SERVER_PORT)
	{
		$this->HIPO_Server_Common();
		$this->_domain	= $domain;
		$this->_port	= $port;

	}// end constructor


	/**
	* Connect to the server
	*
	* @return boolean
	* @access public
	*/
	function connect()
	{
		// disconnect first, if we are currently connected
		if (!is_null($this->_fd)) {
			$this->disconnect();
		}

		$fd = socket_create(AF_INET, SOCK_STREAM, 0);
		if (!$fd) {
			trigger_error('Could not create socket : '.$this->getLastSocketError(), E_USER_WARNING);
			return false;
		}

		// adress may be reused
		socket_set_option($fd, SOL_SOCKET, SO_REUSEADDR, 1);

		if (!socket_connect($fd, $this->_domain, $this->_port)) {
			trigger_error('Could not connect : '.$this->getLastSocketError($fd), E_USER_WARNING);
			socket_close($fd);
			return false;
		}

		$this->_fd = $fd;

		return true;

	}// end connect();


	/**
	* Disconnect from the server
	*
	* @return boolean
	* @access public
	*/
	function disconnect()
	{
		if (is_null($this->_fd)) return true;
		socket_close($this->_fd);
		$this->_fd = null;
		return true;

	}// end disconnect();


	/**
	* Read from the server
	*
	* @return null|Array	if there is valid data (from the server) then an Array
								will be returned, otherwise NULL will be returned
	* @access public
	*/
	function read()
	{
		// if we aren't connected, try to connect
		if (is_null($this->_fd) && !$this->connect()) {
			trigger_error('Unable to connect to server.', E_USER_WARNING);
			return null;
		}

		return $this->_readFromSocket($this->_fd);

	}// end read()


	/**
	* Send data to a client
	*
	* @param mixed $data	the data that needs to be sent to the server
	*
	* @return boolean	indicating whether all bytes were written to the socket or not
	* @access public
	*/
	function write($data)
	{

		$wrapper_data = Array(
							'systemid' => SQ_HIPO_SERVER_SYSTEMID,
							'data' => $data,
							);

		// if we aren't connected, try to connect
		if (is_null($this->_fd) && !$this->connect()) {
			trigger_error('Unable to connect to server.', E_USER_WARNING);
			return '';
		}

		return $this->_writeToSocket($this->_fd, $wrapper_data);

	}// end write()


}// end class
?>