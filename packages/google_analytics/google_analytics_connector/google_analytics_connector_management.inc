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
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Google Analytics Connector Management
*
* @see Asset_Management
*
* @author  Squiz Pty Ltd <products@squiz.net>
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_Connector_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager relevant to this particular asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor


	/**
	* Upgrade to a new version
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			echo('Upgrading Google Analytics Connector from v'.$current_version."\n");
			
			// Version 0.2 updates Connectors to use the Bearer access token type instead
			// of the default URI token type. This is necessary once we start doing write
			// actions to the GA API.
			$connectors = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('google_analytics_connector', FALSE, TRUE);
			$count = 0;

			foreach ($connectors as $id => $type) {
				$connector_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$connector_asset->setAttrValue('access_token_type', 'bearer');
				$connector_asset->saveAttributes();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($connector_asset, TRUE);
				unset($connector_asset);

				$count++;
			}//end foreach

			pre_echo('Upgrade of Google Analytics Connector to v0.2 complete. Updated '.$count.' asset(s).');

		}//end if

		return TRUE;

	}//end _upgrade()

}//end class
?>
