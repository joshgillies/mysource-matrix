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
* $Id: system_asset_config.inc,v 1.23 2006/12/05 05:36:14 bcaldwell Exp $
*
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
* @version $Revision: 1.23 $
* @package MySource_Matrix
*/
class System_Asset_Config extends Config
{

	// Dummy entry for the printing of the system asset() fn
	var $config_vars = Array(
						'PRINT_SYSTEM_ASSETS'	=> Array('editable' => 0, 'default' => ''),
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
		// we don't really care about any values that they have entered
		return parent::save(Array(), $backup_existing);

	}//end save()


	/**
	* Paint out the code for the config file for the passed config
	*
	* This allows extra things to be added for special cases
	*
	* @param string		$name	the config var name
	* @param mixed		$value	the value that the config var has
	* @param boolean	$var_export	whether to run the value through var_export or not
	*
	* @return boolean	whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value, $var_export=true)
	{
		switch ($var_name) {
			case 'PRINT_SYSTEM_ASSETS' :

				$packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

				$system_assets = Array();
				// GET THE ASSETIDS FOR ALL THE $system_assets types
				foreach ($packages as $data) {

					$package = $data['code_name'];

					if ($package == '__core__') {
						require_once SQ_CORE_PACKAGE_PATH.'/package_manager_core.inc';
						$class = 'package_manager_core';
					} else {
						require_once SQ_PACKAGES_PATH.'/'.$package.'/package_manager_'.$package.'.inc';
						$class = 'package_manager_'.$package;
					}

					eval('$sys_asset_types = '.$class.'::getSystemAssetTypes();');

					foreach ($sys_asset_types as $type_code) {
						$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($type_code);
						if (count($assetids) > 0) {
							$system_assets[$type_code] = (int) $assetids[0];
						}
					}//end foreach
				}// end foreach
				echo '$system_assets = ';
				var_export($system_assets);
				echo "\n";

				// reset the list of system assets in the asset manager
				$GLOBALS['SQ_SYSTEM']->am->resetSystemAssetList();

				return true;
			break;

			default :
				return parent::_paintConfigVar($var_name, $value, $var_export);

		}// end switch

	}//end _paintConfigVar()


}//end class

?>