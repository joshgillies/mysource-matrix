<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: messaging_service_config.inc,v 1.9 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* Messaging_Service_Config
*
* Purpose
*
*    Looks after the creation and editing of the messaging service config
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Messaging_Service_Config extends Config
{
	var $config_vars = Array(
		'SQ_MS_SHOW_IN_FLASH_WHITE_LIST' => Array('editable' => 1, 'default' => "*"),
		'SQ_MS_SHOW_IN_FLASH_BLACK_LIST' => Array('editable' => 1, 'default' => "asset.attributes.fulllog\nasset.workflow.invitation\nasset.workflow.announce"),
		'SQ_MS_LOG_TO_FILE_WHITE_LIST'   => Array('editable' => 1, 'default' => "*"),
		'SQ_MS_LOG_TO_FILE_BLACK_LIST'   => Array('editable' => 1, 'default' => "asset.attributes\nasset.workflow.invitation\nasset.workflow.announce"),
		'SQ_MS_LOG_TO_DB_WHITE_LIST'     => Array('editable' => 1, 'default' => "*"),
		'SQ_MS_LOG_TO_DB_BLACK_LIST'     => Array('editable' => 1, 'default' => "asset.attributes.fulllog\nasset.workflow.invitation\nasset.workflow.announce"),
		'SQ_MS_SEND_MAIL_WHITE_LIST'     => Array('editable' => 1, 'default' => "asset.locking.forced\nconfig.*\nhipo.*\nasset.workflow.invitation\nasset.workflow.announce"),
		'SQ_MS_SEND_MAIL_BLACK_LIST'     => Array('editable' => 1, 'default' => "*"),
	);


	/**
	* Constructor
	*
	*/
	function Messaging_Service_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/messaging_service.inc';
	}

}//end class

?>
