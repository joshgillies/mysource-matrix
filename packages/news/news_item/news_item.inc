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
* $Id: news_item.inc,v 1.5 2004/06/24 07:09:18 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';


/**
* News_Item
*
* Purpose
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage news
*/
class News_Item extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*/
	function News_Item($assetid=0)
	{
		$this->Page($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['link_url'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => false);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns the asset (if any) that we are linking to
	*
	* @return NULL|object Asset
	* @access public
	*/
	function &getLinkAsset()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'link_asset');
		if (empty($link)) {
			$null = null;
			return null;
		}

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}//end getLinkAsset()


	/**
	* Set's the asset (if any) to which we are going to be linking
	*
	* @param object Asset	the asset to which we want to link (NULL clears the link asset)
	*
	* @return boolean
	* @access public
	*/
	function setLinkAsset(&$asset)
	{
		$old_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'link_asset');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (empty($old_link)) {
			// if there is not existing asset, and we aren't setting on, we are done here
			if (is_null($asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return true;
			}

		// else if there is a link, and we are about to set an asset, but it is the same as we already have
		} elseif (!is_null($asset) && $old_link['minorid'] == $asset->id) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;

		// else something different, delete the old link
		} else {
			if (!$this->deleteLink($old_link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		if (!is_null($asset)) {
			if (!$this->createLink($asset, SQ_LINK_NOTICE, 'link_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end setLinkAsset()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Called by the design to print the body of this asset
	*
	* It's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// if there is no url, see if we have an asset selected
		if (($url = $this->attr('link_url')) == '') {
			$link_asset = &$this->getLinkAsset();
			if (!is_null($link_asset)) {
				if (($url = $link_asset->getURL()) == '') {
					$url = current_url().'?a='.$link_asset->id;
				}
			}
		}

		if (!$url == '') $url .= trim($this->attr('url_suffix'));
		if (!$url == '') echo '<a href='.$url.'>';

		?>
		<h1><?php echo $this->attr('name'); ?></h1>
		<?php 
		if (!$url == '') echo '</a>';
		?>

		<p><?php echo $this->attr('summary'); ?></p>
		<p><?php echo $this->attr('body'); ?></p>		
		<?php

	}//end printBody()


	/**
	* Return the keywords with their replacements
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'name' => Array (
	*          'value' => 'page',
	*          'description' => 'the asset name',
	*     ),
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getAssetKeywords($descriptions=false)
	{
		$keywords = parent::getAssetKeywords($descriptions);
		
		$keywords['asset_name_linked_to_link_url'] = Array();

		if ($descriptions) {
			$keywords['asset_name_linked_to_link_url']['value'] = 'New Item Linked';
			$keywords['asset_name_linked_to_link_url']['description'] = 'Name of the News Item linked to the URL/Asset.';
		} else {
			$link_asset = &$this->getLinkAsset();
			$url = '';
			if (is_null($link_asset)) {
				$link_url = $this->attr('link_url');
				if (empty($link_url)) {
					$url = 'http://'.$this->attr('link_url').'?'.$this->attr('url_suffix');
				}
			} else {
				$url = $link_asset->getURL();
				if (empty($url)) $url = current_url().'?a='.$link_asset->id;
			}
			if (empty($url)) {
				// no URL or Asset, so just display the Asset name
				$keywords['asset_name_linked_to_link_url']['value'] = $this->attr('name');
			} else {
				$url .= trim($this->attr('url_suffix'));
				$keywords['asset_name_linked_to_link_url']['value'] = '<a href="'.$url.'">'.$this->attr('name').'</a>';
			}
		}
		return $keywords;

	}//end getAssetKeywords()


	/**
	* Determine if the current user is allowed into this asset's backend interface
	*
	* @return boolean
	* @access public
	*/
	function backendAccess()
	{
		if (SQ_IN_LIMBO) return is_a($GLOBALS['SQ_SYSTEM']->user, 'user');
		else return parent::backendAccess();

	}//end backendAccess()


}//end class

?>