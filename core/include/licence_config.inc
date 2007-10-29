<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: licence_config.inc,v 1.3 2007/10/29 05:03:20 lwright Exp $
*
*/

require_once SQ_LIB_PATH.'/config/config.inc';

/**
* System_Config
*
* Purpose
*
*    Looks after the creation and editing of the system config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Licence_Config extends Config
{

	/**
	* Holds the definitions for all the constants managed by this configuration object
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var array
	*/
	var $config_vars = Array(
						'SQ_LICENCE_TYPE'			=> Array('editable' => 1, 'default' => 'GPL'),
						'SQ_LICENCE_INSTALL_KEY'	=> Array('editable' => 1, 'default' => ''),
						'SQ_LICENCE_WARRANTY_KEY'	=> Array('editable' => 1, 'default' => ''),
					   );


	/**
	* Constructor
	*
	* @return object
	* @access public
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/licence.inc';

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @param boolean 	$send_message		whether to send a message using the messaging system
	*										(always FALSE for this config)
	*
	* @return boolean
	* @access public
	*/
	function save($vars, $backup_existing=FALSE, $send_message=TRUE)
	{
		return parent::save($vars, $backup_existing, FALSE);

	}//end save()


}//end class

?>
