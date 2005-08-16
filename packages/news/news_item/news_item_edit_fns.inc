<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: news_item_edit_fns.inc,v 1.6 2005/08/16 23:51:12 dheppell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* News_Item_Edit_Fns
*
* Purpose
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class News_Item_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*/
	function News_Item_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'link_url';

	}//end constructor


	/**
	* Paint the interface for selecting the asset to link to
	*
	* @param object Bodycopy			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLinkAssetid(&$asset, &$o, $prefix)
	{
		$link_asset = &$asset->getLinkAsset();
		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_link_assetid', (is_null($link_asset) ? 0 : $link_asset->id));
		} else {
			if (is_null($link_asset)) {
				echo 'None Set';
			} else {
				echo $link_asset->name.' (#'.$link_asset->id.')';
			}
		}
		return true;

	}//end paintLinkAssetid()


	/**
	* Process the interface for selecting the asset to link to
	*
	* @param object Bodycopy			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLinkAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_link_assetid']['assetid'])) {
			return true;
		}

		$link_assetid = $_POST[$prefix.'_link_assetid']['assetid'];

		if ($link_assetid) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_assetid);
		} else {
			$link_asset = null;
		}

		return $asset->setLinkAsset($link_asset);

	}//end processLinkAssetid()


}//end class

?>
