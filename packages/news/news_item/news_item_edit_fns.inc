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
* $Id: news_item_edit_fns.inc,v 1.1.1.1 2004/06/24 05:37:11 amiller Exp $
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


	/**
	* Paints the interface for editing the contents of the page
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintContents(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		?>
		<table class="sq_backend_table" width="100%">
			<tr>
				<td class="sq-backend-table-header" width="100%">Summary</td>
			</tr>
			<tr>
				<td width="100%">
		<?php
			if ($write_access) {
				require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';
				$wysiwyg = new Asset_Attribute_WYSIWYG();
				$wysiwyg->value = $asset->attr('summary');
				$wysiwyg->name = 'attr_summary';
				$wysiwyg->paint($prefix);
			} else {
				echo $asset->attr('summary');
			}
		?>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
		<tr>
				<td class="sq-backend-table-header" width="100%">Full contents</td>
		</tr>
		<tr>
				<td width="100%">
		<?php
			if ($write_access) {
				require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';
				$wysiwyg = new Asset_Attribute_WYSIWYG();
				$wysiwyg->value = $asset->attr('body');
				$wysiwyg->name = 'attr_body';
				$wysiwyg->paint($prefix);
			} else {
				echo $asset->attr('body');
			}
		?>
			</td>
		</tr>
		</table>
		<?php
		
		return $write_access;
		} // End paintContents()
		
	/**
	* Processes the interface for editing the contents of the page
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processContents(&$asset, &$o, $prefix)
	{
		$proc_summary = false;
		$proc_body = false;
		
		if (!$asset->writeAccess('attributes')) return false;

		require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';
		$wysiwyg = new Asset_Attribute_WYSIWYG();

		$wysiwyg->name = 'attr_summary';
		$wysiwyg->process($prefix);

		if ($asset->setAttrValue('summary', $wysiwyg->value)) {
			$proc_summary = true;
		}

		$wysiwyg->name = 'attr_body';
		$wysiwyg->process($prefix);

		if ($asset->setAttrValue('body', $wysiwyg->value)) {
			$proc_body = true;
		}
			
		if ($proc_summary && $proc_body) {
			return true;
		} else {
			return false;
		}

	}//end processContents()

		
}//end class

?>