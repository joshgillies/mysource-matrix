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
* $Id: replication_remote_master_system.inc,v 1.1.2.2 2004/02/14 17:41:09 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replication_remote_system.inc';

/**
* Replication_Remote_Master_System
*
* Purpose
*
*    To represent a remote system in the replication framework of the master system
*    Includes taking care of Authentication and Managing the connections between the systems
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_Remote_Master_System extends Replication_Remote_System
{
	/**
	* Constructor
	*
	* @param boolean	$load	whether to load the config or not, really only need when doing initial create
	*
	*/
	function Replication_Remote_Master_System($load=true)
	{
		$this->Replication_Remote_System();
		$this->id   = 0;
		$this->name = 'master';
		if ($load) $this->load();

	}//end constructor


	/**
	* Returns the conf file for this remote system
	*
	* @return string
	* @access public
	* @see save(), load()
	*/
	function confFile()
	{
		return SQ_DATA_PATH.'/private/conf/replication/master.inc';
	}// end confFile()


	/**
	* Returns the path to a data directory especially for this remote system
	*
	* @return string
	* @access public
	* @see save(), load()
	* @abstract
	*/
	function dataPath()
	{
		if (is_null($this->id)) {
			trigger_error('Unable to get data path : No System Id', E_USER_WARNING);
			return '';
		}
		return SQ_DATA_PATH.'/private/replication/master';

	}// end dataPath()



}//end class

?>