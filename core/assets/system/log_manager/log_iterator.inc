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
* $Id: log_iterator.inc,v 1.4 2006/01/26 22:34:06 lwright Exp $
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
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
class Log_Iterator
{


	var $offset;
	var $logname;
	var $rotation_index;
	var $total_size;


	/**
	* Constructor
	*
	* @param string	$logname		the log to act on
	* @param int	$rotation_index	the rotation index of the log to act on, NULL to act on the current log
	*
	*/
	function Log_Iterator($logname, $rotation_index=NULL)
	{
		$this->offset     = 0;
		$this->logname    = $logname;
		$this->rotation_index = $rotation_index;
		$this->total_size = $GLOBALS['SQ_SYSTEM']->log_manager->getLogSize($this->logname, $this->rotation_index);

	}//end constructor


	/**
	* Gets the next entry from log manager
	*
	* Returns the next line as an array or FALSE on failure
	*
	* @return mixed boolean|array
	* @access public
	*/
	function getNextEntry()
	{
		if ($this->offset < $this->total_size) {
			$line =& $GLOBALS['SQ_SYSTEM']->log_manager->getLogLine($this->logname, $this->rotation_index, $this->offset);
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
	* @access private
	*/
	function getCurrentProgress()
	{
		$result = Array(
					'current'	=> $this->offset,
					'total'		=> $this->total_size,
				  );

		return $result;

	}//end getCurrentProgress()


}//end class


?>
