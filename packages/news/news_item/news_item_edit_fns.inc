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
* $Id: news_item_edit_fns.inc,v 1.10 2007/12/03 05:02:59 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* News_Item_Edit_Fns
*
* Purpose
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class News_Item_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'link_url';

	}//end constructor


	/**
	* Paint the interface for selecting the asset to link to
	*
	* @param News_Item			$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLinkAssetid(News_Item $asset, Backend_Outputter $o, $prefix)
	{
		$link_asset = $asset->getLinkAsset();
		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_link_assetid', (is_null($link_asset) ? 0 : $link_asset->id));
		} else {
			if (is_null($link_asset)) {
				echo 'None Set';
			} else {
				echo get_asset_tag_line($link_asset->id);
			}
		}
		return true;

	}//end paintLinkAssetid()


	/**
	* Process the interface for selecting the asset to link to
	*
	* @param News_Item			$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLinkAssetid(News_Item $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_link_assetid']['assetid'])) {
			return true;
		}

		$link_assetid = $_POST[$prefix.'_link_assetid']['assetid'];

		if ($link_assetid) {
			$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_assetid);
		} else {
			$link_asset = null;
		}

		return $asset->setLinkAsset($link_asset);

	}//end processLinkAssetid()


}//end class

?>
