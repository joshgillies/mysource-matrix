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
* $Id: replication_system_transport.inc,v 1.1.2.11 2004/02/26 15:13:09 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Replication_System_Transport
*
* Purpose
*
*    Base Classes to handle the transport of date between Master and Slave Systems
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_System_Transport extends MySource_Object
{

	/**
	* buffer size for socket_read and fread
	* @var integer
	*/
	var $_read_buffer_size = 4096;

	/**
	* the socket file descriptor
	* @var resource
	*/
	var $_sd = null;

	/**
	* Path to store any files that are downloaded
	* @var string
	*/
	var $_data_path = '';


	/**
	* Constructor
	*
	* @param string	$data_path	the path to store any files that are downloaded
	*
	*/
	function Replication_System_Transport($data_path)
	{
		$this->MySource_Object();
		$this->_data_path = $data_path;
	}//end constructor


	/**
	* Returns an array of var names in the object that are to saved if this object get's serialised
	*
	* @return Array
	* @access private
	*/
	function __sleep()
	{
		$vars = parent::__sleep();
		$k = array_search('_sd', $vars);
		if ($k !== false) unset($vars[$k]);
		$k = array_search('_data_path', $vars);
		if ($k !== false) unset($vars[$k]);
		return $vars;
	}//end __sleep()


	/**
	* Returns the method of authentication that this class is
	*
	* @return string
	* @access public
	*/
	function method()
	{
		return substr(get_class($this), 29);
	}//end method()


	/**
	* Returns an array of data that can be later used with loadParams() to re-initiate this transport method
	* The reason we do this rather than just serialising the whole object is so that it can be edited by
	* hand if need be
	*
	* @access public
	* @return Array()
	* @see loadParams()
	*/
	function extractParams()
	{
		return Array();

	}//end extractParams()


	/**
	* Loads an array of data that came from extractParams() to re-initiate this transport method
	*
	* @param Array $params	the params array as retrieved from extractParams()
	*
	* @access public
	* @return Array()
	* @see extractParams()
	*/
	function loadParams($params)
	{
		return true;

	}//end loadParams()


	/**
	* Set's the socket to be used when communicating on this transport method
	*
	* @param resource $sd	the socket resource we are using for connections
	*
	* @access public
	* @return boolean	success or not
	*/
	function setSocketFd($sd)
	{
		if (!is_resource($sd)) {
			trigger_error('Passed argument is not a resource', E_USER_WARNING);
			return false;
		}

		$this->_sd = $sd;
		return true;

	}// end setSocketFd()


	/**
	* Unset's the socket we are currently using for communicating on this transport method
	* Optionally can close the connection as well
	*
	* @param boolean $sd	the socket resource we are using for connections
	*
	* @access public
	* @return boolean	success or not
	*/
	function removeSocketFd($close_connection)
	{
		if (is_null($this->_sd)) return true;
		if ($close_connection) socket_close($this->_sd);
		$this->_sd = null;
		return true;

	}// end removeSocketFd()


	/**
	* return string for last socket error
	*
	* @access   public
	* @return string    $error    last error
	*/
	function getLastSocketError()
	{
		$lastError = (is_resource($this->_sd)) ? socket_last_error($this->_sd) : socket_last_error();
		return 'Msg: '.socket_strerror($lastError).' / Code: '.$lastError;

	}// end getLastSocketError()


	/**
	* returns the host and port string of the active socket
	*
	* @param boolean $host_only	whether you want the port tacked on or not
	*
	* @return string    "[host]:[port]"
	* @access public
	*/
	function getPeerInfo($host_only=false)
	{
		if (is_resource($this->_sd)) {
			socket_getpeername($this->_sd, $peer_host='', $peer_port='');
			if ($host_only) return $peer_host;
			else return $peer_host.':'.$peer_port;
		} else {
			return '';
		}

	}// end getPeerInfo()


	/**
	* Send data to a client
	*
	* @param string		$data_str		the data to send down the line
	* @param int		&$bytes_written	the number of bytes actually written to the pipe
	*
	* @return boolean	indicating whether all bytes were written to the socket or not
	* @access private
	*/
	function _writeToSocket($data_str, &$bytes_written)
	{
		if (!is_resource($this->_sd)) {
			trigger_error('Unable to write to socket : socket doesn\'t exist', E_USER_WARNING);
			return false;
		}
		$bytes_written = 0;
		while($data_str != '') {
			$bytes = @socket_write($this->_sd, $data_str);
			if ($bytes === false) {
				trigger_error('Error writing to '.$this->getPeerInfo().' : '.$this->getLastSocketError(), E_USER_WARNING);
				return false;
			}
			$data_str = substr($data_str, $bytes);
			$bytes_written += $bytes;

		}// end while

		return true;

	}// end _writeToSocket()


	/**
	* Read from a socket and return the data
	*
	* @param int	$length		the number of bytes to attempt to read in
	* @param int	$read_type	type of read as specified by socket_read(), currently PHP_BINARY_READ | PHP_NORMAL_READ
	*
	* @return false|string	the data read from the socket will be put returned
	*						FALSE will be returned if there is an error
	* @access private
	*/
	function _readFromSocket($length, $read_type)
	{
		if (!is_resource($this->_sd)) {
			trigger_error('Unable to read from socket : socket doesn\'t exist', E_USER_WARNING);
			return false;
		}
		// make sure we start with empty string
		$data_str  = '';
		$num_bytes = 0;

		$bytes_to_read = min($length, $this->_read_buffer_size);

		// read data from socket
		while(1) {
			$buf = @socket_read($this->_sd, $bytes_to_read, $read_type);
			if ($buf === false) {
				trigger_error('Error Reading from '.$this->getPeerInfo().' : '.$this->getLastSocketError(), E_USER_WARNING);
				return false;
			}

			// connection was closed
			if ($buf == '') {
				trigger_error('Connection Closed when Reading from '.$this->getPeerInfo(), E_USER_NOTICE);
				break;

			} else {

				$num_bytes += strlen($buf);
				$data_str .= $buf;

				// if we are going to be stopping at a new line, then if the last char is a newline
				// break out of here
				if ($read_type === PHP_NORMAL_READ && substr($buf, -1) === "\n") {
					break;
				}

				// we have go enough data now, so break out of here
				if ($num_bytes >= $length) {
					break;
				}

			}// end if

			$bytes_to_read = min($length - $num_bytes, $this->_read_buffer_size);

		}// end while

		return $data_str;

	}// end _readFromSocket()


	/**
	* Writes a variable to the socket
	*
	* @param mixed		$data	the data that needs to be sent to the remote system
	*
	* @return boolean	indicating whether the contents were written successfully
	* @access public
	* @abstract
	*/
	function writeVar($data)
	{
		return false;

	}// end writeVar()


	/**
	* Writes a file to the socket
	*
	* @param string		$source			Source filename, path from the SYSTEM ROOT
	*
	* @return boolean	indicating whether the contents were written successfully
	* @access public
	* @abstract
	*/
	function writeFile($source)
	{
		return false;

	}// end writeFile()


	/**
	* Read a variable from the socket
	*
	* @param mixed		&$data		the retrieved variable will be placed in this var
	* @param Array		&$headers	the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access public
	* @abstract
	*/
	function readVar(&$data, &$headers)
	{
		return false;

	}// end readVar()


	/**
	* Read a file from the socket and put it in the data dir
	*
	* @param string		&$filename	the location of the file on the local file system will be place in this var
	* @param Array		&$headers	the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access public
	* @abstract
	*/
	function readFile(&$filename, &$headers)
	{
		return false;

	}// end readFile()


	/**
	* Sends out some data (like a command) and waits for a response from the other end
	* This is basically a combination of a write and read with all the checks
	*
	* @param mixed		$send_data			the data to write
	* @param mixed		&$receive_data		the retrieved variable will be placed in this var
	* @param Array		&$receive_headers	the headers that were retrieved will be place in this var
	*
	* @return mixed		true on success, string err msg on error
	* @access public
	*/
	function sendAndReceiveVar($send_data, &$receive_data, &$receive_headers)
	{
		if (!$this->writeVar($send_data)) {
			return 'Data Write Error';
		}
		if (!$this->readVar($receive_data, $receive_headers)) {
			return 'Data Read Error';
		}
		return true;

	}// end sendAndReceiveVar()


	/**
	* Reads some expected data (like a command) and if it matches what it was expecting sends
	* the data down the pipe
	* This is basically a combination of a read, check and then write with all the checks
	*
	* @param mixed		$expected_data		scalar variable to which the retrieved variable will be compared against
	* @param mixed		$send_data			this var will be sent if the received data and expected data match
	* @param boolean	$strict_comparison	whether we want a strict var comparison between the received data and the expected data
	*
	* @return mixed		true on success, string err msg on error
	* @access public
	*/
	function receiveAndSendVar($expected_data, $send_data, $strict_comparison=true)
	{
		if (!$this->readVar($data=null,$headers=null)) {
			return 'Data Read Error';
		}

		$ok = ($strict_comparison) ? ($data === $expected_data) : ($data == $expected_data);
		if (!$ok){
			return 'Expecting '.var_export($expected_data, true).' - Received '.var_export($data, true);
		} 
		
		if (!$this->writeVar($send_data)) {
			return 'Data Write Error';
		}

		return true;

	}// end receiveAndSendVar()


}//end class

?>