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
* $Id: search_folder_management.inc,v 1.8 2005/03/24 03:04:29 dheppell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Folder_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
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
						    'name'     => Array(
												'added'   => '0.2',
												'type'    => 'text',
												'default' => '',
												),
							'settings' => Array(
												'added'   => '0.1',
												'type'    => 'serialise',
												'default' => Array(),
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
		if (!parent::_upgrade($current_version)) return false;

		if (version_compare($current_version, '0.2', '<=')) {
			// version 0.2->0.3 upgrade - sees $this->attr('settings')['types']
			// being stored differently - as Array(typecode => inherits) instead
			// of Array(typecode) - due to search manager upgrades
			bam('STARTING SEARCH FOLDER UPGRADE - VERSION '.$current_version.' => 0.3');

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), false);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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
					$types[$type_code] = true;
				}

				$settings['types'] = $types;
				$asset->setAttrValue('settings', $settings);

				if (!$asset->saveAttributes()) {
					trigger_error('Search Folder Upgrade [to 0.3] FAILED - Couldn\'t save attributes', E_USER_WARNING);
					return false;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			bam('SEARCH FOLDER UPGRADE COMPLETE - VERSION '.$current_version.' => 0.3');

		}

		return true;

	}//end _upgrade()


}//end class

?>