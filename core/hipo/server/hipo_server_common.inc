<?php

require_once 'XML/Tree.php';

/**
* HIPO_Server_Common
*    Parent class that hold functionaliy that is common to the server and client classes
*
* @version 0.0.1
* @author Blair Robertson <blair@squiz.net>
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

	* @return null|object XML_Tree	if there is valid data (from a hipo_client) then an XML_Tree object 
									will be returned, otherwise NULL will be returned
	* @access private
	*/
	function _readFromSocket($fd)
	{
		// start with empty string
		$data = '';

		$end_char_pos = -strlen($this->_read_end_char);
		error_log("END CHAR : ".$end_char_pos.' : "'.$this->_read_end_char.'"');

		// read data from socket
		while($buf = socket_read($fd, $this->_read_buffer_size, PHP_BINARY_READ)) {
			if ($buf === false) {
				error_log('Could not read from client ('.$this->getLastSocketError($fd).').');
				return null;
			}

			// connection was closed
			if ($buf == '') {
				break;

			} else {

				$data .= $buf;

				// we found the end string, strip it and break out of here
				if (substr($data, $end_char_pos) == $this->_read_end_char) {
					$data = substr($data, 0, $end_char_pos);
					break;
				}

			}// end if

		}// end while

		if ($data == '') return null;

		error_log('ALL DATA : "'.$data.'"');

		$xml_data = new XML_Tree();
		$root  = &$xml_data->getTreeFromString($data);
		if (PEAR::isError($root)) {
			socket_getpeername($fd, $peer_host='', $peer_port='');
			trigger_error('Error Reading from '.$peer_host.':'.$peer_port.'. Unable to parse XML from data "'.$data.'"'."\n".' XML Error Msg : '.$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
            return null;
        }

		return $xml_data;

	}// end _readFromSocket()


	/**
	* Send data to a client
	*
	* @param resource			$fd	the socket file desriptor resource to write to
	* @param object XML_Tree	$xml_data	the xml data to send down the line
	*
	* @return boolean	indicating whether all bytes were written to the socket or not
	* @access private
	*/
	function _writeToSocket($fd, $xml_data)
	{
		if (!is_a($xml_data, 'xml_tree')) {
			trigger_error('Could not write to socket, passed data was not an xml tree object.', E_USER_WARNING);
			return false;
		}

		$data = $xml_data->get().$this->_read_end_char;

		while($data) {
			$bytes_written = @socket_write($fd, $data);
			if ($bytes_written === false) {
				socket_getpeername($fd, $peer_host='', $peer_port='');
				trigger_error('Could not write "'.$data.'" to '.$peer_host.':'.$peer_port.' ('.$this->getLastSocketError($fd).').', E_USER_WARNING);
				return false;
			}
			$data = substr($data, $bytes_written);
			error_log('Data Left : "'.$data.'"');
		}// end while

		return true;

	}// end _writeToSocket()

}// end class
?>