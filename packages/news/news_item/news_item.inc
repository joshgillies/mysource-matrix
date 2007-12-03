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
* $Id: news_item.inc,v 1.20 2007/12/03 05:02:59 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* News_Item
*
* Purpose
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.20 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	public function lockTypes()
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
	public function _getAllowedLinks()
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
	public function getLinkAsset()
	{
		if ($this->id == 0) {
			$null = NULL;
			return $null;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'link_asset');
		if (empty($link)) {
			$null = null;
			return $null;
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
	public function setLinkAsset($asset)
	{
		if (!is_null($asset)) {
			assert_is_a($asset, 'asset');
		}

		$old_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'link_asset');
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (empty($old_link)) {
			// if there is not existing asset, and we aren't setting on, we are done here
			if (is_null($asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return true;
			}

		// else if there is a link, and we are about to set an asset, but it is the same as we already have
		} else if (!is_null($asset) && $old_link['minorid'] == $asset->id) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return true;

		// else something different, delete the old link
		} else {
			if (!$this->deleteLink($old_link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		if (!is_null($asset)) {
			if (!$this->createLink($asset, SQ_LINK_NOTICE, 'link_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
	public function _getName($short_name=false)
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
	public function printBody()
	{
		// if there is no url, see if we have an asset selected
		if (($url = $this->attr('link_url')) == '') {
			$link_asset = $this->getLinkAsset();
			if (!is_null($link_asset)) {
				if (($url = $link_asset->getURL()) == '') {
					$url = current_url().'?a='.$link_asset->id;
				}
			}
		}

		if (!$url == '') {
			$url .= trim($this->attr('url_suffix'));
		}

		if ($this->attr('show_headline')) {
		if (!$url == '') echo '<a href='.$url.'>';
		?>
			<h1><?php echo $this->attr('name'); ?></h1>
		<?php
			if (!$url == '') echo '</a>';
		}

		if ($this->attr('show_summary')) {
		?>
		<div id="sq_news_summary">
			<p><?php echo $this->attr('summary'); ?></p>
		</div>
		<?php
		}

		if ($this->attr('show_body')) {
		?>
		<div id="sq_news_body">
			<p><?php echo $this->attr('body'); ?></p>
		</div>
		<?php
		}

		if ($this->attr('show_contact_name') || $this->attr('show_contact_phone')) {
		?>
		<div id="sq_news_contact">
		<?php
			if ($this->attr('show_contact_name')) {
			?>
			<p><?php echo 'Contact: '.$this->attr('contact_name'); ?></p>
		<?php
			}

			if ($this->attr('show_contact_phone')) {
			?>
			<p><?php echo 'Phone: '.$this->attr('contact_phone'); ?></p>
		<?php
			}
		?>
		</div>
		<?php
		}

	}//end printBody()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	public function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();
		$res['asset_name_linked_to_link_url'] = 'Asset name linked to link URL';
		$res['link_url'] = 'The Link URL for this item';
		return $res;

	}//end getAvailableKeywords()


	/**
	* Get the replacement for the asset_name_linked_to_link_url keyword
	*
	* @return string
	* @access public
	*/
	public function getAssetNameLinkedToLinkURLKeywordReplacement()
	{
		$url = $this->getLinkUrlKeywordReplacement();
		if (empty($url)) {
			// no URL or Asset, so link to the asset itself
			return '<a href="'.$this->getURL().'">'.$this->attr('name').'</a>';
		} else {
			return '<a href="'.$url.'">'.$this->attr('name').'</a>';
		}

	}//end getAssetNameLinkedToLinkURLKeywordReplacement()


	/**
	* Get the replacement for the link_url keyword
	*
	* @return string
	* @access public
	*/
	public function getLinkUrlKeywordReplacement()
	{
		$url = '';
		$link_asset = &$this->getLinkAsset();
		if (is_null($link_asset)) {
			$link_url = trim($this->attr('link_url'));
			if (!empty($link_url)) {
				$url = $link_url;
				$suffix = $this->attr('url_suffix');
				if (!empty($suffix)) {
					$url .= '?'.trim($suffix);
				}
			}
		} else {
			$url = $link_asset->getURL();
			if (empty($url)) {
				$url = current_url().'?a='.$link_asset->id;
			}
		}
		return $url;

	}//end getLinkUrlKeywordReplacement()


	/**
	* Determine if the current user is allowed into this asset's backend interface
	*
	* @return boolean
	* @access public
	*/
	public function backendAccess()
	{
		if (SQ_IN_LIMBO) {
			return is_a($GLOBALS['SQ_SYSTEM']->user, 'user');
		} else {
			return parent::backendAccess();
		}

	}//end backendAccess()


}//end class
?>