<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: csv.inc,v 1.24.6.1 2013/08/06 04:55:46 ewang Exp $
*
*/


/**
* CSV
*
* Purpose
*     A class used to create and parse CSV files
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24.6.1 $
* @package Fudge
* @subpackage CSV
*/
class CSV
{

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
	* The seperator that separates data in the CSV file
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
	* Count of CSV lines
	*
	* @var int
	*/
	var $line_count=0;


	/**
	* Constructor
	*
	* @param string	$filepath	the path to the file we are parsing/creating
	*
	* @access public
	*/
	function CSV($filepath='')
	{
		if ($filepath) $this->setFilepath($filepath);

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
		if ($filepath) {
			// full path to this file
			$this->filepath = $filepath;

			// directory of this file
			$this->_dir = dirname($filepath);
			if (!file_exists($this->_dir)) mkdir($this->_dir, 0775);

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
		if ($header) {
			// set custom headers
			header($header);
		} else {
			header('Content-Type: application/csv');
			header('Content-Disposition: attachment; filename="'.$this->_filename.'";');

			//This is known that Internet Explorer has problems with SSL connection (https).
			//http://support.microsoft.com/default.aspx?scid=kb;en-us;812935
			//http://support.microsoft.com/default.aspx?scid=kb;en-us;316431
			// do not send no cache header in https for IE

			// get protocol information
        	$url_info = parse_url(current_url());
        	$protocol = (isset($url_info['scheme'])) ? $url_info['scheme'] : NULL;
			$using_ie6_8 = isset ($_SERVER['HTTP_USER_AGENT']) ? preg_match('/(MSIE\s[0-8]+\.)/', $_SERVER['HTTP_USER_AGENT']) : FALSE;
			if (!is_null($protocol) && $protocol == 'https' && $using_ie6_8) {
				header('Cache-Control: private, max-age=0, must-revalidate');
            	header('Pragma: private');
             	header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

			}

		}

	}//end setHeader()


	/**
	* Export the current values to the current set filepath
	*
	* If no filepath then export to screen
	*
	* @param boolean	$ordered	if true, then use ordered export if possible.
	*								if false/default, use standard export ('ordered' ==
	*								use field headers as keys, as opposed to using
	*								numeric indices)
	*
	* @return void
	* @access public
	*/
	function export($ordered=false)
	{
		if (!$this->filepath) {
			// no file to write to.....lets send the header and then the file
			$this->setHeader();
			$fp = null;
		} else {
			if (!$fp = fopen($this->filepath,'w')) {
				trigger_error('Unable to open file "'.$this->filepath.'" for writing', E_USER_WARNING);
				return;
			}
		}

		if ($this->field_headers) {
			$line = $this->formatLine($this->field_headers, $this->deliminator);
			if (isset($fp)) {
				fputs($fp, $line."\n");
			} else {
				echo $line."\n";
			}
		}

		$this->printValues($ordered, $fp);
		
		if (isset($fp)) fclose($fp);

	}//end export()

	/**
	 * This goes through the values previously set
	 * and either prints them out (if $fp is null)
	 * or writes them to the file descriptor previously opened
	 *
	 * @param boolean	$ordered	Whether the data needs to be ordered or not
	 * @param resource	$fp			If this is null, then just print the data out.
	 * 								If it's not null, write the data to the file descriptor.
	 *								The file descriptor is previously opened by the calling fn's
	 *								So this fn can just write the data to the file
	 *
	 * @return void
	 */
	private function printValues($ordered=false, $fp=null)
	{
		foreach ($this->values as $value) {
			// if field headers are there, use the ordered version of the
			// format line method to export
			if ($this->field_headers && $ordered) {
				$line = $this->formatOrderedLine($value, $this->deliminator);
			} else {
				$line = $this->formatLine($value, $this->deliminator);
			}
			if (isset($fp) && $fp !== null) {
				fputs($fp, $line."\n");
			} else {
				echo $line."\n";
			}
		}
	}

	/**
	 * exportValues
	 * Takes values and either appends them to the file previously specified
	 * or prints out the extra data
	 *
	 * This is used after the initial export().
	 * export either sets the header for the browser, or creates the file
	 * and then prints the csv header line (if necessary).
	 * This either appends more data to the csv file, or prints out more data.
	 *
	 * @param boolean	$ordered	Whether the values need to be ordered or not.
	 *
	 * @return void
	 */
	function exportValues($ordered=false)
	{
		if ($this->filepath) {
			if (!$fp = fopen($this->filepath,'a')) {
				trigger_error('Unable to open file "'.$this->filepath.'" for writing', E_USER_WARNING);
				return;
			}
		} else {
			$fp = null;
		}
		$this->printValues($ordered, $fp);
		
		if (isset($fp)) fclose($fp);
	}

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
		if (!$this->filepath) return;

		// check to see if the file exsts
		if (!is_file($this->filepath)) return;

		//set auto_detect_line_endings to detect weird old mac line endings and stuff
		$line_ending_cfg = ini_set('auto_detect_line_endings', 1);
		
		if (!$fp = fopen($this->filepath,'r')) {
			trigger_error('Unable to open file "'.$this->filepath.'" for reading', E_USER_WARNING);
			return;
		}

		$start_at = (int)$start_at;
		$lines    = (int)$lines;

		$line_count = 0;
		$lines_done = 0;

		while (($tmp = fgetcsv($fp, 65535, $this->deliminator)) != null) {
			$line_count++;
			if ($line_count >= $start_at) {
				$this->values[] = $tmp;
				$lines_done++;
				if ($lines && ($lines_done >= $lines)) break;
			}
		}

		fclose($fp);
		
		if (!$line_ending_cfg) {
		    ini_set('auto_detect_line_endings', 0);
		}

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
		foreach ($array as $val) {
			if (is_array($val)) {
				$line .= '"'.$this->formatLine($val,$deliminator).'"'.$deliminator;
			} else {
				if (preg_match("/[".preg_quote($deliminator, '/') . "\"\n\r]/", $val)) {
					$val = '"'.str_replace('"', '""', $val).'"';
				}
				$line .= $val.$deliminator;
			}
		}

		// strip the last deliminator
		return $line = substr($line, 0, (strlen($deliminator) * -1));

	}//end formatLine()


	/**
	* Format a CSV line
	*
	* @param array	$array			an array of data values
	* @param string	$deliminator	the deliminator to use between data values
	*
	* @return void
	* @access public
	*/
	function formatOrderedLine(&$array, $deliminator=',')
	{
		if (!$this->field_headers) return '';
		$line_elements = Array();

		foreach($this->field_headers as $key => $val) {
			$this_val = '';
			if (isset($array[$key])) {
				if (is_array($array[$key])) {
					$this_val = '"'.$this->formatLine($array[$key],$deliminator).'"';
				} else {
					$this_val = $array[$key];
				}

				if (preg_match("/[".preg_quote($deliminator, '/') . "\"\n\r]/", $this_val)) {
					$this_val = '"'.str_replace('"', '""', $this_val).'"';
				}
			}

			$line_elements[] = $this_val;
		}

		return implode($deliminator, $line_elements);

	}//end formatLine()


	/**
	* Count the number of lines in the CSV file
	*
	* If no filepath then export to screen
	*
	* @param int	$start_at	the line number to start importing at
	*
	* @return void
	* @access public
	*/
	function countLines($start_at=0)
	{
		// can only count from a file
		if (!$this->filepath) return;

		// check if the file even exists
		if (!is_file($this->filepath)) {
			$this->line_count = 0;
			return;
		}

		if (!$fp = fopen($this->filepath,'r')) {
			trigger_error('Unable to open file "'.$this->filepath.'" for counting', E_USER_WARNING);
			return;
		}

		$line_count = 0;

		while (($tmp = fgetcsv($fp, 65535, $this->deliminator)) != null) {
			$line_count++;
		}

		$this->line_count = $line_count - $start_at;

		fclose($fp);

	}//end countLines()


	/**
	* Checks to see if the supplied string exists as an entry in the file
	*
	* @return boolean
	* @access public
	*/
	function isUnique($string)
	{
		// can only import from a file
		if (!$this->filepath) return true;

		// check if the file even exists
		if (!is_file($this->filepath)) return true;

		if (!$fp = fopen($this->filepath,'r')) {
			trigger_error('Unable to open file "'.$this->filepath.'" for reading', E_USER_WARNING);
			return true;
		}

		while (($tmp = fgetcsv($fp, 65535, $this->deliminator)) != null) {
			if (in_array($string, $tmp)) return false;
		}

		fclose($fp);

		return true;

	}//end isUnique()


}//end class

?>
