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
* $Id: remap_manager_management.inc,v 1.5 2004/06/18 01:49:16 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Remap_Manager_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $ - 0.1
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

	}//end Remap_Manager_Management()


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

		if ($current_version == '0.1') {
			// version 0.1->0.2
			// changed the 404 assetid from storing an assetid to storing a NOTICE link
			// to the 404 assetid - that way if the 404 asset is purged it automatically deletes
			// the association
			bam('STARTING REMAP MANAGER UPGRADE - FROM VERSION 0.1');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				// get the current root asset id and make sure it exists
				$current_root_assetid = $asset->attr('404_asset');
				$root_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_root_assetid));

				// create the notice link
				if (!empty($root_asset_info)) {
					$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_root_assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $root_asset, SQ_LINK_NOTICE, '404')) {
						bam('REMAP MANAGER UPGRADE FAILED');
						return false;
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			bam('REMAP MANAGER UPGRADE COMPLETE - FROM VERSION 0.1');
		}//end version 0.1 upgrade

		return true;

	}//end _upgrade()


}//end class

?>