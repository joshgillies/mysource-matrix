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
* $Id: news_item_edit_fns.inc,v 1.1.1.1.2.2 2004/11/18 00:11:11 amiller Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* News_Item_Edit_Fns
*
* Purpose
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Version$ - 1.0
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

	}// end paintLinkAssetid()
	

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
		if (!isset($_POST[$prefix.'_link_assetid']['assetid'])) return true;

		$link_assetid = $_POST[$prefix.'_link_assetid']['assetid'];

		if ($link_assetid) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_assetid);
		} else {
			$link_asset = null;
		}

		return $asset->setLinkAsset($link_asset);

	}// end processLinkAssetid()

}//end class

?>
