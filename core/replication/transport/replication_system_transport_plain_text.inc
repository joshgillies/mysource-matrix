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
* $Id: replication_system_transport_plain_text.inc,v 1.1.2.7 2004/02/27 18:04:43 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replication_system_transport.inc';

/**
* Replication_System_Transport_Basic
*
* Purpose
*
*    This just sends everything over the sockets in plain text
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_System_Transport_Plain_Text extends Replication_System_Transport
{
	/**
	* Headers are a fixed size for this transport layer, the reason being is that when mixing up
	* PHP_NORMAL_READ and PHP_BINARY_READ's we were having huge connections issues (ie drop-outs)
	* @param int
	*/
	var $_header_len = 20;


	/**
	* Writes the headers out to the socket, leaving the contents as next thing to be written
	* NOTE 1: the combination of the header and the value MUST be less that $this->_header_len
	* NOTE 2: all surrounding whitespace will be trimmed from header and value
	*
	* @param Array		$headers	Array('lowercase header' => 'value')
	*
	* @return boolean	true if all headers written successfully, false otherwise
	* @access private
	*/
	function _writeHeaders($headers)
	{
		$headers_str = '';
		foreach($headers as $name => $value) {

			$line = $name.': '.((string) $value);
			if (strlen($line) > $this->_header_len) {
				trigger_error('Error writing headers to '.$this->getPeerInfo().' : Header Line "'.$line.'" is longer that the fixed size header length '.$this->_header_len, E_USER_WARNING);
				return false;
			}

			$headers_str .= str_pad($line, $this->_header_len, ' ', STR_PAD_RIGHT);
		}// end foreach

		$headers_str .= str_repeat('-', $this->_header_len);

		return $this->_writeToSocket($headers_str, $bytes_written=0);

	}// end _writeHeaders()


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
		if (!$this->_writeHeaders(Array('type' => 'variable', 'length' => strlen($contents)))) {
			return false;
		}

		return $this->_writeToSocket($contents, $bytes_written=0);

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

		$filename = basename($source);

		$headers = Array(
						'type'        => 'file', 
						'length'      => filesize($source_file),
						'name_length' => strlen($filename),
					);

		if (!$this->_writeHeaders($headers)) {
			return false;
		}

		$bytes_written = 0;

		if (!$this->_writeToSocket($filename, $bytes_written)) return false;

		$fd = fopen($source_file, 'rb');
		do {

			$data = fread($fd, $this->_read_buffer_size);
			if (strlen($data) == 0) break;

			if (!$this->_writeToSocket($data, $bytes_written)) return false;

		} while(true);

		return true;

	}// end writeFile()


	/**
	* Reads the headers of the socket, leaving the contents as next thing to read
	* Also guarantees that there is a Type and Length header
	*
	* @return Array()	Array('lowercase header' => 'value') or false on error
	* @access private
	*/
	function _readHeaders()
	{
		$headers = Array();
		$end_header_line = str_repeat('-', 20);
		do {
			$line = $this->_readFromSocket(20, PHP_BINARY_READ);
			// something happened or the connection was closed, either way bail out
			if ($line === false || $line === '') return false;
			// end of the headers ? OK outa here
			if ($line === $end_header_line) break;

			if (preg_match('/^([^:]+): (.*)$/', trim($line), $matches=null)) {
				$headers[strtolower($matches[1])] = $matches[2];
			}

		} while(true);

		if (isset($headers['length'])) $headers['length'] = (int) $headers['length'];
		if (empty($headers['type']) || empty($headers['length'])) {
			trigger_error('Error reading headers from '.$this->getPeerInfo().' : Both the Type and Length headers need to exist'.print_r($headers, 1), E_USER_WARNING);
			return false;
		}

		return $headers;

	}// end _readHeaders()


	/**
	* Read either a variable or file from the socket (useful when expecting a file, but can handle an error code/msg var)
	* To check what type is returned, check $headers['type'] it will be 'variable' or 'file'
	*
	* @param mixed		&$data		if this is a var then the data read from the socket, 
	*								if a file then the location of the file on the local filesystem
	* @param Array		&$headers	the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access public
	* @abstract
	*/
	function read(&$data, &$headers)
	{
		// first thing to do is get the headers
		$headers = $this->_readHeaders();
		if ($headers === false) return false;

		switch($headers['type']) {
			case 'variable' :
				return $this->_readVarContents($data, $headers);
				break;
			case 'file' :
				return $this->_readFileContents($data, $headers);
				break;
			default : 
				trigger_error('Error reading from '.$this->getPeerInfo().' : Unknown data type "'.$headers['type'].'"', E_USER_WARNING);
				return false;
		}

	}// end read()


	/**
	* Read a variable from the socket
	*
	* @param mixed		&$data		the retrieved variable will be placed in this var
	* @param Array		&$headers	the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access public
	*/
	function readVar(&$data, &$headers)
	{
		// first thing to do is get the headers
		$headers = $this->_readHeaders();
		if ($headers === false) return false;

		if ($headers['type'] != 'variable') {
			trigger_error('Error reading from '.$this->getPeerInfo().' : Expecting "variable" type, got "'.$headers['type'].'"', E_USER_WARNING);
			return false;
		}

		return $this->_readVarContents($data, $headers);

	}// end readVar()


	/**
	* Perform the actual reading of the contents of a variable from the socket
	*
	* @param mixed		&$data		the retrieved variable will be placed in this var
	* @param Array		&$headers	the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access private
	* @see read(), readVar()
	*/
	function _readVarContents(&$data, &$headers)
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

	}// end _readVarContents()


	/**
	* Read a file from the socket and put it in the data dir
	*
	* @param string		&$filename	the location of the file on the local file system will be place in this var
	* @param Array		&$headers	the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access public
	*/
	function readFile(&$filename, &$headers)
	{
		// first thing to do is get the headers
		$headers = $this->_readHeaders();
		if ($headers === false) return false;

		if ($headers['type'] != 'file') {
			trigger_error('Error reading from '.$this->getPeerInfo().' : Expecting "file" type, got "'.$headers['type'].'"', E_USER_WARNING);
			return false;
		}

		return $this->_readFileContents($filename, $headers);

	}// end readFile()


	/**
	* Perform the actual reading a file from the socket and put it in the data dir
	*
	* @param string		&$saved_file	the location of the file on the local file system will be place in this var
	* @param Array		&$headers		the headers that were retrieved will be place in this var
	*
	* @return boolean	indicating success
	* @access public
	* @see read(), readFile()
	*/
	function _readFileContents(&$saved_file, &$headers)
	{
		if (empty($headers['name_length'])) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : No filename length header', E_USER_WARNING);
			return false;
		}

		$headers['name_length'] = (int) $headers['name_length'];
		$filename = $this->_readFromSocket($headers['name_length'], PHP_BINARY_READ);
		if ($filename === false) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : invalid data from socket', E_USER_WARNING);
			return false;
		}

		$dest_file     = $this->_data_path.'/'.$filename;
		$dest_tmp_file = $this->_data_path.'/.'.$filename;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory($this->_data_path)) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : Unable to create data dir', E_USER_WARNING);
			return false;
		}

		$err_msg = '';
		$bytes_written = 0;
		$fd = fopen($dest_tmp_file, 'wb');
		if (!$fd) {
			trigger_error('Error reading file from '.$this->getPeerInfo().' : unable to create local file', E_USER_WARNING);
			return false;
		}
		do {
			$bytes_to_read = min(($headers['length'] - $bytes_written), $this->_read_buffer_size);

			$data = $this->_readFromSocket($bytes_to_read, PHP_BINARY_READ);
			if ($data === false) {
				$err_msg = 'invalid data from socket';
				return false;
			}

			if ($data == '') break;

			$bytes_to_write = strlen($data);
			while($data != '') {
				$bytes = fwrite($fd, $data);
				if ($bytes_written === false) {
					$err_msg = 'error writting to temporary destination file '.$dest_tmp_file;
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

		if ($err_msg != '') {
			unlink($dest_tmp_file);
			trigger_error('Error reading file from '.$this->getPeerInfo().' : '.$err_msg, E_USER_WARNING);
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

		$saved_file = $dest_file;

		return true;

	}// end _readFileContents()


}//end class

?>