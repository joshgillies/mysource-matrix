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
* $Id: search_folder_management.inc,v 1.20 2012/08/30 00:59:15 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Folder_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Folder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'		=> Array(
										'added'			=> '0.2',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'is_contextable'=> TRUE,
									   ),
						'settings'	=> Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
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

		if (version_compare($current_version, '0.2', '<=')) {
			// version 0.2->0.3 upgrade - sees $this->attr('settings')['types']
			// being stored differently - as Array(typecode => inherits) instead
			// of Array(typecode) - due to search manager upgrades
			pre_echo('STARTING SEARCH FOLDER UPGRADE - VERSION 0.2 => 0.3');

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$settings = $asset->attr('settings');

				// types have not been set at all?
				if (!isset($settings['types'])) continue;
				$types = $settings['types'];

				// types has nothing in it?
				if (count($types) == 0) continue;

				// if the first element of types is a boolean, this is already
				// been converted somehow
				if (is_bool(reset($types))) continue;

				// flip the types array to get typecodes as keys, then set them
				// to inherit types (as this was what Search Manager used to do)
				$types = array_flip($types);
				foreach ($types as $type_code => $value) {
					$types[$type_code] = TRUE;
				}

				$settings['types'] = $types;
				$asset->setAttrValue('settings', $settings);

				if (!$asset->saveAttributes()) {
					trigger_localised_error('SCH0009', translate('Search Folder Upgrade [to 0.3] FAILED - Couldn\'t save attributes'), E_USER_WARNING);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}//end foreach

			pre_echo('SEARCH FOLDER UPGRADE COMPLETE - VERSION 0.2 => 0.3');

		}//end if

		if (version_compare($current_version, '0.3', '<=')) {
			// version 0.3->0.4 upgrade - changes to accept multiple root nodes.
			// The variable $settings['root'] is now stored as an array of
			// assetids, rather than a single value.
			pre_echo('STARTING SEARCH FOLDER UPGRADE - VERSION 0.3 => 0.4');

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$settings = $asset->attr('settings');

				// only need to save this attribute
				if (!is_array($settings['root'])) {
					$settings['root'] = (empty($settings['root'])) ? Array() : Array($settings['root']);
					$asset->setAttrValue('settings', $settings);

					if (!$asset->saveAttributes()) {
						trigger_localised_error('SCH0009', translate('Search Folder Upgrade [to 0.3] FAILED - Couldn\'t save attributes'), E_USER_WARNING);
						return FALSE;
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				$asset = NULL;
			}

			pre_echo('SEARCH FOLDER UPGRADE COMPLETE - VERSION 0.3 => 0.4');

		}//end if

		return TRUE;

	}//end _upgrade()


}//end class

?>
