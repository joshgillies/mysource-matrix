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
* $Id: replication_system_authentication_basic.inc,v 1.1.2.3 2004/02/14 19:44:27 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replication_system_authentication.inc';

/**
* Replication_System_Authentication_Basic
*
* Purpose
*
*    Handle the authentication of connections between Master Server and Slaves using Basic authentication
*    [TODO: define what "Basic Authentication" actually is]
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_System_Authentication_Basic extends Replication_System_Authentication
{

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

		// now because we initiated the connection the other end will be the one challenging us 
		// with the authentication, so we wait for them
		if ($initiator) {
			if ($transport->read($data=null,$headers=null)) {
				pre_echo("HEADERS : ");
				pre_echo($headers);
				pre_echo("DATA : ");
				pre_echo($data);
			}
		} else {
			if ($transport->writeVar('What do you want ?')) {
				error_log(__FUNCTION__.'() WRITE SUCCESSFUL');
			} else {
				error_log(__FUNCTION__.'() WRITE FAILED');
			}

		}// end if

		return false;
	}//end authenticate()

}//end class

?>