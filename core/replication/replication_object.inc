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
* $Id: replication_object.inc,v 1.1.2.2 2004/03/18 16:10:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Replication_Object
*
* Purpose
*
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @abstract
*/
class Replication_Object extends MySource_Object
{
	/**
	* Constructor
	*
	*/
	function Replication_Object()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Appends something to the log
	*
	* @param string	$msg		the msg to add to log
	* @param string	$log_type	the log to put the msg  in
	*
	* @access private
	* @static
	*/
	function _log($msg, $log_type='general')
	{
		error_log(date('[d-M-Y H:i:s] ').$msg."\n", 3, SQ_DATA_PATH.'/private/logs/replication/'.$log_type.'.log');

	}// end _log()

}//end class

?>