<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: hipo_config.inc,v 1.5 2003/10/02 05:37:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* HIPO_Config
*
* Purpose
*
*    Looks after the creation and editing of the hipo config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class HIPO_Config extends Config
{
	var $config_vars = Array(
		'SQ_HIPO_USE_SERVER'		=> Array('editable' => 0, 'default' => false),
		'SQ_HIPO_SERVER_DOMAIN'		=> Array('editable' => 0, 'default' => ''),
		'SQ_HIPO_SERVER_PORT'		=> Array('editable' => 0, 'default' => 0),
		'SQ_HIPO_SERVER_SYSTEMID'	=> Array('editable' => 0, 'default' => ''),
	);


	/**
	* Constructor
	*
	*/
	function HIPO_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/hipo.inc';
	}

}//end class

?>
