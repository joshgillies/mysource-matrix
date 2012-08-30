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
* $Id: squiz_server_client_mysource3.inc,v 1.5 2012/08/30 01:09:22 ewang Exp $
*
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
* @version $Revision: 1.5 $
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

	}//end constructor


}//end class
?>