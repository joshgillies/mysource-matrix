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
* $Id: replication_communication.inc,v 1.1.2.1 2004/01/21 18:43:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Replication_Communication
*
* Purpose
*
*    Base Classes to handle the authentication of connections between Master Server and Slaves
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_Communication extends MySource_Object
{

	/**
	* buffer size for socket_read
	* @var integer
	*/
	var $_read_buffer_size = 128;

	/**
	* the file (socket) descriptor
	* @var resource
	*/
	var $_fd = null;


	/**
	* Constructor
	*/
	function Replication_Communication()
	{
		$this->MySource_Object();
	}//end constructor


	/**
	* Gets the config vars that this authentication method requires
	*
	* Config vars should be in the format:
	* <PRE>
	* Array(
	*    'SQ_REPLICATION_TRANSPORT_[VAR_NAME_1]'   => Array('editable' => 0, 'default' => false),
	*    'SQ_REPLICATION_TRANSPORT_[VAR_NAME_2]'   => Array('editable' => 0, 'default' => false),
	* );
	* </PRE>
	* 
	* @access public
	* @return Array()
	* @static
	*/
	function getConfigVars()
	{
		return Array();
	
	}//end getConfigVars()


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
	* @param resource	$fd			the socket file desriptor resource to read from
	* @param int		$length		the number of bytes to attempt to read in
	*
	* @return null|string	the data read from the socket will be put returned
	*						NULL will be returned if there is an error
	* @access private
	*/
	function _readFromSocket($fd, $length)
	{
		// make sure we start with empty string
		$data_str  = '';
		$num_bytes = 0;

		$bytes_to_read = min($length, $this->_read_buffer_size);

		// read data from socket
		while($buf = @socket_read($fd, $bytes_to_read, PHP_BINARY_READ)) {
			if ($buf === false) {
				socket_getpeername($fd, $peer_host='', $peer_port='');
				trigger_error('Error Reading from '.$peer_host.':'.$peer_port.' ('.$this->getLastSocketError($fd).').', E_USER_WARNING);
				return null;
			}

			// connection was closed
			if ($buf == '') {
				break;

			} else {

				$num_bytes += strlen($buf);
				$data_str .= $buf;

				// we found the end string, strip it and break out of here
				if ($num_bytes >= $length) {
					break;
				}

			}// end if

			$bytes_to_read = min($length - $num_bytes, $this->_read_buffer_size);

		}// end while

		return $num_bytes;

	}// end _readFromSocket()


	/**
	* Send data to a client
	*
	* @param resource	$fd				the socket file desriptor resource to write to
	* @param string		$data_str		the data to send down the line
	* @param int		&$bytes_written	the number of bytes actually written to the pipe
	*
	* @return boolean	indicating whether all bytes were written to the socket or not
	* @access private
	*/
	function _writeToSocket($fd, $data_str, &$bytes_written)
	{
		$bytes_written = 0;
		while($data_str) {
			$bytes = @socket_write($fd, $data_str);
			if ($bytes === false) {
				socket_getpeername($fd, $peer_host='', $peer_port='');
				trigger_error('Error writing to '.$peer_host.':'.$peer_port.' ('.$this->getLastSocketError($fd).').', E_USER_WARNING);
				return false;
			}
			$data_str = substr($data_str, $bytes);
			$bytes_written += $bytes;

		}// end while

		return true;

	}// end _writeToSocket()


}//end class

?>