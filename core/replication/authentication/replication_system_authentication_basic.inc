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
* $Id: replication_system_authentication_basic.inc,v 1.1.2.1 2004/01/21 18:43:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Replication_System_Authentication_Basic
*
* Purpose
*
*    Base Classes to handle the authentication of connections between Master Server and Slaves
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_System_Authentication_Basic extends MySource_Object
{
	/**
	* Gets the config vars that this authentication method requires
	*
	* Config vars should be in the format:
	* <PRE>
	* Array(
	*    'SQ_REPLICATION_AUTH_[VAR_NAME_1]'   => Array('editable' => 0, 'default' => false),
	*    'SQ_REPLICATION_AUTH_[VAR_NAME_2]'   => Array('editable' => 0, 'default' => false),
	* );
	* </PRE>
	* 
	* @access public
	* @return Array()
	* @static
	* @see Replication_System_Authentication::getConfigVars()
	*/
	function getConfigVars()
	{
		return Array(
			'IP_ADDRESS' => Array('editable' => 1, 'default' => ''),
			'USERNAME'   => Array('editable' => 1, 'default' => ''),
			'PASSWORD'   => Array('editable' => 1, 'default' => ''),
		);
	
	}//end getConfigVars()


	/**
	* Attempts to authenticate with the connection with $fd
	*
	* @param object Replication_System_Transport	$transport	the transport layer that the system we belong to uses
	* @param boolean								$initiating	whether this end of the connection initiated the connection or not
	*
	* @return boolean
	* @access public
	*/
	function authenticate($transport, $initiating)
	{

		


		return false;
	}//end authenticate()


}//end class

?>