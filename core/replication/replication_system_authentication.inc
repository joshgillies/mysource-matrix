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
* $Id: replication_system_authentication.inc,v 1.1.2.3 2004/02/14 19:42:31 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Replication_System_Authentication
*
* Purpose
*
*    Base Classes to handle the authentication of connections between Master Server and Slaves
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_System_Authentication extends MySource_Object
{
	/**
	* Constructor
	*/
	function Replication_System_Authentication()
	{
		$this->MySource_Object();
	}//end constructor


	/**
	* Returns the type of authentication that class is
	*
	* @return string
	* @access public
	*/
	function type()
	{
		return substr(get_class($this), 34);
	}//end type()


	/**
	* Returns an array of data that can be later used with loadParams() to re-initiate this authentication type
	* The reason we do this rather than just serialising the whole object is so that it can be edited by
	* hand if need be
	*
	* @access public
	* @return Array()
	* @see loadParams()
	*/
	function extractParams()
	{
		return Array();
	
	}//end extractParams()


	/**
	* Loads an array of data that came from extractParams() to re-initiate this authentication type
	*
	* @param Array $params	the params array as retrieved from extractParams()
	*
	* @access public
	* @return Array()
	* @see extractParams()
	*/
	function loadParams($params)
	{
		return true;
	
	}//end loadParams()


	/**
	* Attempts to authenticate with the connection with $fd
	*
	* @param object Replication_System_Transport	$transport	the socket file desriptor resource to read from
	* @param boolean								$initiator	whether this end of the connection initiated the connection or not
	*
	* @return boolean
	* @access public
	*/
	function authenticate($transport, $initiator)
	{
		return false;
	}//end authenticate()


}//end class

?>