<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: log_iterator.inc,v 1.10 2011/06/15 23:47:11 cupreti Exp $
*
*/

/**
*
* log_iterator.inc
*
* Provides an interface to the reports to handle the reading of a log
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/
class Log_Iterator
{


	protected $offset;
	protected $logname;
	protected $rotation_index;
	protected $total_size;


	/**
	* Constructor
	*
	* @param string	$logname		the log to act on
	* @param int	$rotation_index	the rotation index of the log to act on, NULL to act on the current log
	*
	*/
	function __construct($logname, $rotation_index=NULL)
	{
		$this->offset     = 0;
		$this->logname    = $logname;
		$this->rotation_index = $rotation_index;

		$log_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('log_manager');
		$this->total_size = $log_manager->getLogSize($this->logname, $this->rotation_index);

		if (empty($this->total_size)) {
			$this->total_size = 0;
		}

	}//end constructor


	/**
	* Gets the next entry from log manager
	*
	* Returns the next line as an array or FALSE on failure
	*
	* @return mixed boolean|array
	* @access public
	*/
	public function getNextEntry()
	{
		if ($this->offset < $this->total_size) {
			$log_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('log_manager');
			$line =& $log_manager->getLogLine($this->logname, $this->rotation_index, $this->offset);
			$line_offset = array_get_index($line, 'offset', FALSE);

			if ($line_offset != FALSE && $line_offset >= 0) {
				$this->offset = $line_offset;
			}

			return array_get_index($line, 'line', FALSE);

		} else {
			return FALSE;
		}

	}//end getNextEntry()


	/**
	* Gets the last entry from the log manager
	*
	* Returns the last line as an array or FALSE on failure
	*
	* @return mixed boolean|array
	* @access public
	*/
	public function getLastEntry()
	{
		$last_entry = FALSE;
		while($line=$this->getNextEntry()) {
			if ($line) {
				$last_entry = $line;
			}
		}

		return $last_entry;

	}//end function getLastEntry()


	/**
	* Return the current progress in the form of a percentage
	*
	* The returned array is in the format:
	* <pre>
	* Array(
	*       'percentage_done' => int      // the current progress in the form of a percentage
	*       'complete'        => boolean  // whether the read is complete
	*       )
	* </pre>
	*
	* @return array
	* @access public
	*/
	public function &getCurrentProgress()
	{
		$result = Array(
					'current'	=> $this->offset,
					'total'		=> $this->total_size,
				  );

		return $result;

	}//end getCurrentProgress()


}//end class


?>
