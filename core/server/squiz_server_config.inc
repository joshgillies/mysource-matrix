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
* $Id: squiz_server_config.inc,v 1.1 2004/01/14 19:19:47 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* Squiz_Server_Config
*
* Purpose
*
*    Looks after the creation and editing of the squiz server config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Squiz_Server_Config extends Config
{
	var $config_vars = Array(
		'SQ_SERVER_SYSTEM_ROOT'	=> Array('editable' => 1, 'default' => ''),
		'SQ_SERVER_SYSTEMID'	=> Array('editable' => 1, 'default' => ''),
		'SQ_SERVER_DOMAIN'		=> Array('editable' => 1, 'default' => ''),
		'SQ_SERVER_PORT'		=> Array('editable' => 1, 'default' => 0),
	);

	/**
	* Constructor
	*
	*/
	function Squiz_Server_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/squiz_server.inc';

	}//end constructor


}//end class

?>