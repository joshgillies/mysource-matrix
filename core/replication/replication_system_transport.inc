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
* $Id: replication_system_transport.inc,v 1.1.2.5 2004/02/14 19:48:30 brobertson Exp $
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
	* @return string    "[host]:[port]"
	* @access public
	*/
	function getPeerInfo()
	{
		if (is_resource($this->_sd)) {
			socket_getpeername($this->_sd, $peer_host='', $peer_port='');
			return $peer_host.':'.$peer_port;
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
	* Writes a variable to the socket
	*
	* @param mixed		$data	the data that needs to be sent to the remote system
	*
	* @return boolean	indicating whether the contents were written successfully
	* @access public
	*/
	function writeVar($data)
	{
		if (!is_resource($this->_sd)) {
			trigger_error('Unable to write to socket : socket doesn\'t exist', E_USER_WARNING);
			return false;
		}

		$contents = serialize($data);
		$headers  =  "Type: variable\n"
					."Length: ".strlen($contents)."\n"
					."\n";

		return $this->_writeToSocket($headers.$contents, $bytes_written=0);

	}// end writeVar()


	/**
	* Writes a file to the socket
	*
	* @param string		$source			Source filename, path from the SYSTEM ROOT
	*
	* @return boolean	indicating whether the contents were written successfully
	* @access public
	*/
	function writeFile($source)
	{
		if (!is_resource($this->_sd)) {
			trigger_error('Unable to write to socket : socket doesn\'t exist', E_USER_WARNING);
			return false;
		}

		$source_file = SQ_SYSTEM_ROOT.'/'.$source;
		if (!is_file($source_file) || !is_readable($source_file)) {
			trigger_error('Unable to write file to socket : Source file doesn\'t exist or is not readable', E_USER_WARNING);
			return false;
		}

		// NOTE: If the file that is being transferred is over 2GB in size then there could be issues because
		// of the way PHP uses signed int's see filesize() man page for defails

		$headers  =  "Type: file\n"
					."Length: ".filesize($source_file)."\n"
					."Filename: ".basename($source)."\n"
					."\n";

		$bytes_written = 0;
		if (!$this->_writeToSocket($headers, $bytes_written)) return false;

		$fd = fopen($source_file, 'rb');
		do {

			$data = fread($fd, $this->_read_buffer_size);
			if (strlen($data) == 0) break;

			if (!$this->_writeToSocket($data, $bytes_written)) return false;

		} while(true);

		return true;

	}// end writeFile()


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
				break;

			} else {

				$num_bytes += strlen($buf);
				$data_str .= $buf;

				// if we are going to be stopping at a new line, then if the last char is a newline
				// break out of here
				if ($read_type == PHP_NORMAL_READ && substr($buf, -1) == "\n") {
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
	* Read a variable from the socket
	*
	* @param mixed		&$data	the retrieved variable will be placed in this reference
	*
	* @return boolean	indicating success
	* @access public
	*/
	function read(&$data, &$headers)
	{
		// first thing to do is get the headers
		$headers = $this->_readHeaders();
		if ($headers === false) return false;

		switch($headers['type']) {
			case 'variable' :
				return $this->_readVar($data, $headers);
				break;
			case 'file' :
				return $this->_readFile($headers);
				break;
			default :
				trigger_error('Error reading from '.$this->getPeerInfo().' : Unknown read type "'.$headers['type'].'"', E_USER_WARNING);
				return false;
		}

	}// end read()


	/**
	* Reads the headers of the socket, leaving the contents as next thing to read 
	* Also guarantees that there is a Type and Length header
	*
	* @param mixed		&$data	the retrieved variable will be placed in this reference
	*
	* @return Array()	Array('lowercase header' => 'value') - false on error
	* @access private
	*/
	function _readHeaders()
	{
		$headers = Array();
		do {
			$line = $this->_readFromSocket($this->_read_buffer_size, PHP_NORMAL_READ);
			if ($line === false) return false;
			// end of the headers ? OK outa here
			if ($line == "\n") break;

			if (preg_match('/^([^:]+): (.*)$/', trim($line), $matches=null)) {
				$headers[strtolower($matches[1])] = $matches[2];
			}

		} while(true);

		if (isset($headers['length'])) $headers['length'] = (int) $headers['length'];
		if (empty($headers['type']) || empty($headers['length'])) {
			trigger_error('Error reading headers from '.$this->getPeerInfo().' : Both the Type and Length headers need to exist', E_USER_WARNING);
			return false;
		}

		return $headers;		

	}// end _readHeaders()


	/**
	* Read a variable from the socket
	*
	* @param mixed		&$data		the retrieved variable will be placed in this reference
	* @param Array		$headers	the headers that were retrieved for this var
	*
	* @return boolean	indicating success
	* @access private
	* @see read()
	*/
	function _readVar(&$data, $headers)
	{
		$contents = $this->_readFromSocket($headers['length'], PHP_BINARY_READ);
		if ($contents === false) return false;

		$data = unserialize($contents);
		if ($data === false) {
			// OK if they had serialise a variable with the value false, $data would also be false, so
			// let's serialise a false var and see if that equals $contents, if so everything is cool
			$false_ser = serialize(false);
			if ($contents != $false_ser) {
				return false;
			}
		}

		return true;

	}// end _readVar()


	/**
	* Read a file from the socket and put it in the data dir
	*
	* @param Array		$headers	the headers that were retrieved for this var
	*
	* @return boolean	indicating success
	* @access private
	* @see read()
	*/
	function _readFile($headers)
	{
		if (!isset($headers['filename'])) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : No filename header', E_USER_WARNING);
			return false;
		}

		$dest_file     = $this->_data_path.'/'.$headers['filename'];
		$dest_tmp_file = $this->_data_path.'/.'.$headers['filename'];

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory($this->_data_path)) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : Unable to create data dir', E_USER_WARNING);
			return false;
		}

		$abort_write = false;
		$bytes_written = 0;
		$fd = fopen($dest_tmp_file, 'wb');
		do {
			$bytes_to_read = min(($headers['length'] - $bytes_written), $this->_read_buffer_size);

			$data = $this->_readFromSocket($bytes_to_read, PHP_BINARY_READ);
			if ($data === false) {
				trigger_error('Error reading file from '.$this->getPeerInfo().' : invalid data from socket', E_USER_WARNING);
				return false;
			}

			if ($data == '') break;

			$bytes_to_write = strlen($data);
			while($data != '') {
				$bytes  = fwrite($fd, $data);
				if ($bytes_written === false) {
					$abort_write = true;
					break 2;
				}
				$data = substr($data, $bytes);
				$bytes_written += $bytes;
			}// end while

			// we have go enough data now, so break out of here
			if ($bytes_written >= $headers['length']) {
				break;
			}

		} while(true);

		fclose($fd);

		if ($abort_write) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : Unable write temporary destination file '.$dest_tmp_file, E_USER_WARNING);
			unlink($dest_tmp_file);
			return false;
		}

		$om = umask(0000);
		chmod($dest_tmp_file, 0664);
		umask($om);

		if (!rename($dest_tmp_file, $dest_file)) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : Unable rename temporary destination file '.$dest_tmp_file.' to '.$dest_file, E_USER_WARNING);
			unlink($dest_tmp_file);
			return false;
		}

		return true;

	}// end _readFile()

}//end class

?>