<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: hipo_server_common.inc,v 1.4 2003/09/29 04:29:47 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once 'XML/Tree.php';

/**
* HIPO_Server_Common
*    Parent class that hold functionaliy that is common to the server and client classes
*
* @author Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class HIPO_Server_Common
{
	/**
	* end character for socket_read
	* @var integer
	*/
	var $_read_end_char = "\0";

	/**
	* buffer size for socket_read
	* @var integer
	*/
	var $_read_buffer_size = 128;

	/**
	* Constructor
	*
	* @access	public
	*/
	function HIPO_Server_Common()
	{
	}// end constructor


	/**
	* return string for last socket error
	*
	* @access   public
	* @return string    $error    last error
	*/
	function getLastSocketError($fd=null)
	{
		$lastError = (is_resource($fd)) ? socket_last_error($fd) : socket_last_error();
		return 'Msg: '.socket_strerror($lastError).' / Code: '.$lastError;

	}// end getLastSocketError()


	/**
	* read from a socket and return the data
	*
	* @param resource	$fd	the socket file desriptor resource to read from

	* @return null|Array	if there is valid data (from a hipo_client) then an Array
							will be returned, otherwise NULL will be returned
	* @access private
	*/
	function _readFromSocket($fd)
	{
		// start with empty string
		$data_str = '';

		$end_char_pos = -strlen($this->_read_end_char);

		// read data from socket
		while($buf = @socket_read($fd, $this->_read_buffer_size, PHP_BINARY_READ)) {
			if ($buf === false) {
				trigger_error('Could not read from client ('.$this->getLastSocketError($fd).').', E_USER_WARNING);
				return null;
			}

			// connection was closed
			if ($buf == '') {
				break;

			} else {

				$data_str .= $buf;

				// we found the end string, strip it and break out of here
				if (substr($data_str, $end_char_pos) == $this->_read_end_char) {
					$data_str = substr($data_str, 0, $end_char_pos);
					break;
				}

			}// end if

		}// end while

		if ($data_str == '') return null;

		$data = unserialize($data_str);
		if ($data === false) {
			socket_getpeername($fd, $peer_host='', $peer_port='');
			trigger_error('Error Reading from '.$peer_host.':'.$peer_port.'. Unable to parse data "'.$data_str.'"', E_USER_WARNING);
            return null;
        }

		return $data;

	}// end _readFromSocket()


	/**
	* Send data to a client
	*
	* @param resource	$fd		the socket file desriptor resource to write to
	* @param Array		$data	the data to send down the line
	*
	* @return boolean	indicating whether all bytes were written to the socket or not
	* @access private
	*/
	function _writeToSocket($fd, $data)
	{
		$data_str = serialize($data);
		if (strpos($data_str, $this->_read_end_char) !== false) { 
			socket_getpeername($fd, $peer_host='', $peer_port='');
			trigger_error('Could not write "'.$data_str.'" to '.$peer_host.':'.$peer_port.' (The data contains the read end char - '.$this->_read_end_char.')', E_USER_WARNING);
			return false;
		}

		$data_str .= $this->_read_end_char;

		while($data_str) {
			$bytes_written = @socket_write($fd, $data_str);
			if ($bytes_written === false) {
				socket_getpeername($fd, $peer_host='', $peer_port='');
				trigger_error('Could not write "'.$data_str.'" to '.$peer_host.':'.$peer_port.' ('.$this->getLastSocketError($fd).').', E_USER_WARNING);
				return false;
			}
			$data_str = substr($data_str, $bytes_written);
		}// end while

		return true;

	}// end _writeToSocket()

}// end class
?>