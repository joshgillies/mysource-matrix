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
* $Id: funnelback_log_iterator.inc,v 1.2 2012/08/30 00:58:43 ewang Exp $
*
*/

/**
* Funnelback_Log_Iterator.inc
*
* Provides an interface to the reports to handle the reading of a funnelback log (custom logs NOT located under data/private/logs)
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $
* @package Funnelback
*/
class Funnelback_Log_Iterator
{


	protected $offset;
	protected $log;
	protected $rotation_index;
	protected $total_size;
	protected $blank_query;


	/**
	* Constructor
	*
	* @param string	$log		the log to act on
	* @param int	$rotation_index	the rotation index of the log to act on, NULL to act on the current log
	*
	*/
	function __construct($log, $rotation_index=NULL)
	{
		$this->offset = 0;
		$this->log    = $log;
		$this->rotation_index = $rotation_index;

		$this->total_size = $this->getLogSize($this->log, $this->rotation_index);

		if (empty($this->total_size)) {
			$this->total_size = 0;
		}

		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
		$this->blank_query = $fm->getNullQueryKeyword();

	}//end constructor


	/**
	 * Get the size of the log
	 *
	 * @param string	$log	The path to the log file
	 * @param int		$rotation_index	the rotation index of the log to act on, NULL to act on the current log
	 *
	 * @return int
	 * @access public
	 */
	public function getLogSize($log, $rotation_index)
	{
		$suffix = ((!is_null($rotation_index) && is_numeric($rotation_index)) ? '.'.$rotation_index : '');
		$file = $log.$suffix;

		$size = FALSE;
		if (file_exists($file)) {
			$size = sprintf('%u', filesize($file));
		}//end if

		return $size;

	}//end getLogSize()


	/**
	* Gets the next entry from the log
	*
	* Returns the next line as an array or FALSE on failure
	*
	* @return mixed boolean|array
	* @access public
	*/
	public function getNextEntry()
	{
		if ($this->offset < $this->total_size) {
			$suffix = ((!is_null($this->rotation_index) && is_numeric($this->rotation_index)) ? '.'.$this->rotation_index : '');
			$logfile = $this->log.$suffix;
			$line = $this->_getLogLine($logfile, $this->offset);
			$line_offset = array_get_index($line, 'offset', FALSE);

			if ($line_offset != FALSE && $line_offset >= 0) {
				$this->offset = $line_offset;
			}//end if

			$line['line'] = $this->_decodeLine(array_get_index($line, 'line', FALSE));

			return $line;

		} else {
			return FALSE;
		}//end if

	}//end getNextEntry()


	/**
	* Return the current progress in the form of a percentage
	*
	* @return array
	* @access public
	*/
	public function getCurrentProgress()
	{
		$result = Array(
					'current'	=> $this->offset,
					'total'		=> $this->total_size,
				  );

		return $result;

	}//end getCurrentProgress()


	/**
	* Get the requested log line
	*
	* @param string	$file	The file to open
	* @param int	$offset	The current position in the file
	*
	* @return array|FALSE
	* @access public
	*/
	protected function _getLogLine($file, $offset)
	{
		if (empty($file)) return FALSE;

		$file_contents = Array();

		if (file_exists($file) && $handle = fopen($file, 'r')) {
			if (fseek($handle, $offset) >= 0 && !feof($handle)) {
				$line = fgets($handle);
				// new offset
				$offset = ftell($handle);
				$result = Array(
							'line'		=> trim($line),
							'offset'	=> $offset,
						  );
			} else {
				$result = FALSE;
			}
			fclose($handle);
		} else {
			trigger_localised_error('CORE0018', sprintf(translate('Unable to read from logfile %s'), $file), E_USER_ERROR)
			$result = FALSE;
		}

		return $result;

	}//end _getLogLine()


	/**
	* Decode the line to something useful for us to work with
	*
	* Log Format:
	*	[DATE_TIME][REQUESTER_IP][QUERY][INCLUDE_SCOPE][EXCLUDE_SCOPE][START_RANK][NUM_RANKS][CODES][FULL_MATCHES][PARTIAL_MATCHES][ELAPSED_TIME][PROFILE]
	*
	* @param string	$line	The line to decode
	*
	* @return array
	* @access public
	*/
	protected function _decodeLine($line)
	{
		$decoded = Array();

		if ($line === FALSE || !is_string($line) || strpos($line, ',') === FALSE) {
			// This doesn't need encoding, let's leave
			return $decoded;
		}//end if

		$line_data = explode(',', $line);

		$decoded['date'] = strtotime($line_data[0]);
		$decoded['ip'] = $line_data[1];
		$decoded['query'] = str_replace($this->blank_query, '', $line_data[2]);
		$decoded['include'] = $line_data[3];
		$decoded['exclude'] = $line_data[4];
		$decoded['start'] = $line_data[5];
		$decoded['ranks'] = $line_data[6];
		$decoded['codes'] = $line_data[7];
		$decoded['full_matches'] = $line_data[8];
		$decoded['partial_matches'] = $line_data[9];
		$decoded['elapsed_time'] = $line_data[10];
		$decoded['profile'] = $line_data[11];

		return $decoded;

	}//end _decodeLine()


}//end class


?>
