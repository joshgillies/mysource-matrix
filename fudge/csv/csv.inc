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
* $Id: csv.inc,v 1.2 2004/05/17 05:17:49 lwright Exp $
* $Name: not supported by cvs2svn $
*/


/**
* CSV
*
* Purpose
*     A class used to create and parse CSV files
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage CSV
*/
class CSV {

	/**
	* The path of the CSV file we are parsing/creating
	*
	* @var string
	*/
	var $filepath='';

	/**
	* The directory that this CSV file is in
	* This is automatically generated from the filepath
	*
	* @var string
	*/
	var $_dir='';

	/**
	* The filename for this CSV file
	* This is automatically generated from the filepath
	*
	* @var string
	*/
	var $_filename='';

	/**
	* The seperator that seperates data in the CSV file
	* This is normally a comma or tab character
	*
	* @var string
	*/
	var $deliminator = ',';

	/**
	* An array of the names of fields in the CSV file
	*
	* @var Array
	*/
	var $field_headers=false;

	/**
	* An array of CSV line values
	*
	* @var Array()
	*/
	var $values='';


	/**
	* Constructor
	*
	* @param string	$filepath	the path to the file we are parsing/creating
	*
	* @access public
	*/
	function CSV($filepath='')
	{
		if($filepath) $this->setFilepath($filepath);

	}//end constructor


	/**
	* Set the field headers for the CSV file
	*
	* @param array	&$headers	an array of the field names
	*
	* @return void
	* @access public
	*/
	function setFieldHeaders(&$headers)
	{
		$this->field_headers = &$headers;

	}//end setFieldHeaders()


	/**
	* Set the values for the CSV file
	*
	* @param array	$headers	a multi-dimensional array of line values
	*
	* @return void
	* @access public
	*/
	function setValues(&$values)
	{
		$this->values = &$values;

	}//end setValues()


	/**
	* Set the deliminator for the CSV file
	*
	* @param string	$deliminator	the deliminator string
	*
	* @return void
	* @access public
	*/
	function setDeliminator($deliminator)
	{
		$this->deliminator = $deliminator;

	}//end setDeliminator()


	/**
	* Set the filename for the CSV we are parsing/creating
	*
	* @param string	$filename	the filename to set
	*
	* @return void
	* @access public
	*/
	function setFilename($filename='')
	{
		$this->_filename = $filename;

	}//end setFilename()


	/**
	* Set the file path for this CSV file
	*
	* @param string	$filepath	the file path to set
	*
	* @return void
	* @access public
	*/
	function setFilepath($filepath='')
	{
		if($filepath) {
			// full path to this file
			$this->filepath = $filepath;

			// directory of this file
			$this->_dir = dirname($filepath);
			if (!file_exists($this->_dir)) mkdir($this->_dir,0775);

			// name of file without full path
			$this->setFilename(basename($filepath));

			if (!file_exists($this->_dir)) {
				trigger_error('CSV directory "'.$this->_dir.'" does not exist!', E_USER_WARNING);
				return;
			}
		}

	}//end setFilepath()


	/**
	* Set the PHP mime header
	*
	* This will overwrite any previous headers
	*
	* @param string	$header	any custom headers that need to be set
	*
	* @return void
	* @access public
	*/
	function setHeader($header='')
	{
		if($header) {
			// set custom headers
			header($header);
		} else {
			header('Content-Type: application/csv');
			header('Content-Disposition: filename='.$this->_filename.';');
		}

	}//end setHeader()


	/**
	* Export the current values to the current set filepath
	*
	* If no filepath then export to screen
	*
	* @return void
	* @access public
	*/
	function export()
	{
		if(!$this->filepath) {
			// no file to write to.....lets send the header and then the file
			$this->setHeader();
		} else {
			if(!$fp = fopen($this->filepath,'w')) {
				trigger_error('Unable to open file "'.$this->filepath.'" for writing', E_USER_WARNING);
				return;
			}
		}
		if($this->field_headers) {
			$line = $this->formatLine($this->field_headers, $this->deliminator);
			if(isset($fp)) {
				fputs($fp, $line."\n");
			} else {
				echo $line."\n";
			}
		}

		foreach($this->values as $value) {
			$line = $this->formatLine($value, $this->deliminator);
			if(isset($fp)) {
				fputs($fp, $line."\n");
			} else {
				echo $line."\n";
			}
		}

	}//end export()


	/**
	* Import the current file to this->values
	*
	* @param int	$start_at	the line number to start importing at
	* @param int	$lines		the number of lines to import (set to zero for all lines)
	*
	* @return void
	* @access public
	*/
	function import($start_at=1, $lines=0)
	{
		// can only import from a file
		if(!$this->filepath) return;

		if(!$fp = fopen($this->filepath,'r')) {
			trigger_error('Unable to open file "'.$this->filepath.'" for reading', E_USER_WARNING);
			return;
		}

		$start_at = (int)$start_at;
		$lines    = (int)$lines;

		$line_count = 0;
		$lines_done = 0;

		while(($tmp = fgetcsv($fp, 65535, $this->deliminator)) != null) {
			$line_count++;
			if ($line_count >= $start_at) {
				$this->values[] = $tmp;
				$lines_done++;
				if ($lines && ($lines_done >= $lines)) break;
			}
		}

		fclose($fp);

	}//end import()


	/**
	* Format a CSV line
	*
	* @param array	$array			an array of data values
	* @param string	$deliminator	the deliminator to use between data values
	*
	* @return void
	* @access public
	*/
	function formatLine(&$array, $deliminator=',')
	{
		$line = '';
		foreach($array as $val) {
			if(is_array($val)) {
				$line .= '"'.$this->format_line($val,$deliminator).'"'.$deliminator;
			} else {
				if(ereg("[$deliminator\"\n\r]", $val)) {
					$val = '"'.str_replace('"', '""', $val).'"';
				}
				$line .= $val.$deliminator;
			}
		}

		// strip the last deliminator
		return $line = substr($line, 0, (strlen($deliminator) * -1));

	}//end formatLine()


}//end class

?>