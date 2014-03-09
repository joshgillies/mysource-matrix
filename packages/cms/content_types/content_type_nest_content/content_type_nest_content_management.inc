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
* $Id: content_type_nest_content_management.inc,v 1.19 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['paint_with'] = Array(
										'type'		=> 'text',
										'default'	=> 'raw',
										'added'		=> '0.2',
									);

		$this->vars['additional_get'] = Array(
											'type'		=> 'serialise',
											'default'	=> Array(),
											'added'		=> '0.4',
										);
										
		$this->vars['dynamic_nest_get'] = Array(
										'type'		=> 'text',
										'default'	=> '',
										'added'		=> '0.4',
									);								

	}//end constructor


	/**
	* Upgrade
	*
	* @param string $current_version    the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.3', '<') &&
			(version_compare($current_version, '0.1', '<=') ||
			version_compare($current_version, '0.2', '>=')) ) {

			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.3');

			// get all 'bodycopy_container' assets (or their descendants)
			$content_type_nested_content_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds($this->getAssetType(), FALSE);
			pre_echo('Upgrading '.count($content_type_nested_content_ids).' assets...');
			foreach ($content_type_nested_content_ids as $assetid) {
				$content_type_nested_content_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// reasonably critical upgrade - report any assets that didn't make it
				if (is_null($content_type_nested_content_asset)) {
					pre_echo('Could not get asset #'.$assetid);
					continue;
				}

				// Get all the bodycopy_container which are the parents of this asset.
				$parent_container_ids = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($content_type_nested_content_asset->id, 'bodycopy_div');
				if (!empty($parent_container_ids)) {
					foreach ($parent_container_ids as $container_id) {
						$container = $GLOBALS['SQ_SYSTEM']->am->getAsset($container_id);
						// regenerate the bodycopy_container's content file
						$container_edit_fns = $container->getEditFns();
						if (!$container_edit_fns->generateContentFile($container)) {
							pre_echo('Could not generate content file for container asset #'.$assetid);
						}
						unset($container_edit_fns);

						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);
					}
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_type_nested_content_asset);
			}
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.3');

		}//end version 0.3 upgrade

		return TRUE;

	}//end _upgrade()


}//end class

?>
