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
* $Id: system_asset_config.inc,v 1.7.2.2 2004/03/02 18:29:12 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* System_Asset_Config
*
* Purpose
*
*    Looks after the creation system asset config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class System_Asset_Config extends Config
{
	var $config_vars = Array(
		// Dummy entry for the printing of the system asset() fn
		'PRINT_SYSTEM_ASSETS' => Array('editable' => 0, 'default' => ''),
	);

	var $system_asset_types = Array(
									'root_folder',
									'trash_folder',
									'system_management_folder',
									'system_user_group',
									'root_user',
									'designs_folder',
									'authentication_folder',
									'cron_manager',
									'login_design',
									'search_manager',
									'layout_manager',
									'remap_manager',
								);


	/**
	* Constructor
	*
	*/
	function System_Asset_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/system_assets.inc';

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false)
	{
		// We don't really care about any values that they have entered
		return parent::save(Array(), $backup_existing);

	}//end save()


	/**
	* Paint out the code for the config file for the passed config
	*
	* This allows extra things to be added for special cases
	*
	* @param string $name	the config var name
	* @param mixed  $value	the value that the config var has
	*
	* @return boolean whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value)
	{
		switch($var_name) {
			case 'PRINT_SYSTEM_ASSETS' :
				$system_assets = Array();
				// GET THE ASSETIDS FOR ALL THE $system_assets types
				foreach($this->system_asset_types as $type_code) {
					$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($type_code);
					if (count($assetids) != 1) {
						trigger_error('Unable to find a single asset with a type_code "'.$type_code.'", system asset generation failed', E_USER_WARNING);
						return false;
					}
					$system_assets[$type_code] = $assetids[0];
				}//end foreach
				echo '$system_assets = ';
				var_export($system_assets);
				echo "\n";
				return true;
			break;

			default :
				return parent::_paintConfigVar($var_name, $value);

		}// end switch

	}//end _paintConfigVar()


}//end class

?>