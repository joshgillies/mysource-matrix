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
* $Id: squiz_server_client_mysource3.inc,v 1.1 2004/01/14 19:19:47 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_DATA_PATH.'/private/conf/squiz_server.inc';
require_once SQ_SERVER_SYSTEM_ROOT.'/code/squiz_server_client.inc';

/**
* Server_Client
*
* Wrapper for the Squiz_Server_Client, main purpose is to supply the domain, port and systemid from the conf file
* so that individual users don't need to worry about that crap
*
* @author Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Squiz_Server_Client_MySource3 extends Squiz_Server_Client
{

	/**
	* create a new client
	*
	* @param	string	 $systemid			id that the system that is using us is known by in Squiz_Server
	* @param	string	 $domain			domain to connect to
	* @param	integer	 $port				port to connect on
	*
	* @access	public
	*/
	function Squiz_Server_Client_MySource3($systemid=SQ_SERVER_SYSTEMID, $domain=SQ_SERVER_DOMAIN, $port=SQ_SERVER_PORT)
	{
		$this->Squiz_Server_Client($systemid, $domain, $port);

	}// end constructor

}// end class
?>