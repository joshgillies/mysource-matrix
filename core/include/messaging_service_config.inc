<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: messaging_service_config.inc,v 1.7 2003/10/17 04:56:38 gsherwood Exp $
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
		'SQ_MS_SEND_MAIL_WHITE_LIST'     => Array('editable' => 1, 'default' => "config.*\nhipo.*\nasset.workflow.invitation\nasset.workflow.announce"),
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
