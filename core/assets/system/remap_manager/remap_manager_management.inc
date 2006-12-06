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
* $Id: remap_manager_management.inc,v 1.8 2006/12/06 04:40:10 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Remap_Manager_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Remap_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Remap_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'404_asset'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'assetid',
										'default'	=> '0',
									   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if ($current_version == '0.1') {
			// version 0.1->0.2
			// changed the 404 assetid from storing an assetid to storing a NOTICE link
			// to the 404 assetid - that way if the 404 asset is purged it automatically deletes
			// the association
			pre_echo('STARTING REMAP MANAGER UPGRADE - FROM VERSION 0.1');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				// get the current root asset id and make sure it exists
				$current_root_assetid = $asset->attr('404_asset');
				$root_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_root_assetid));

				// create the notice link
				if (!empty($root_asset_info)) {
					$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_root_assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $root_asset, SQ_LINK_NOTICE, '404')) {
						pre_echo('REMAP MANAGER UPGRADE FAILED');
						return FALSE;
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo('REMAP MANAGER UPGRADE COMPLETE - FROM VERSION 0.1');
		}//end version 0.1 upgrade

		return TRUE;

	}//end _upgrade()


}//end class

?>