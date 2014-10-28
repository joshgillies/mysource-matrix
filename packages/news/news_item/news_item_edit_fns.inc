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
* $Id: news_item_edit_fns.inc,v 1.13 2013/08/15 06:10:53 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* News_Item_Edit_Fns
*
* Purpose
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.13 $
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
				echo get_asset_tag_line($link_asset->id, '', $prefix.'_link_assetid_readonly');
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


	/**
	* Creates and deletes NOTICE links based on the assets being linked to within the contents of this news item
	* For shadow assets it
	*
	* @param news_item		$asset	the cnews item asset
	*
	* @return boolean
	* @access public
	*/
	public function paintShadowLinks(News_Item $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end paintShadowLinks()


	/**
	* Creates and deletes NOTICE links based on the assets being linked to within the contents of this news item
	* For shadow assets it
	*
	* @param news_item		$asset	the cnews item asset
	*
	* @return boolean
	* @access public
	*/
	public function processShadowLinks(News_Item $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->id || !$asset->writeAccess()) return FALSE;
		$output = '';

		// get the contents for 'body' and 'summary' attribute
		// for the asset for all the context define on system
		$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
		foreach ($contextids as $contextid) {
			// Reload the vars for this context
			$GLOBALS['SQ_SYSTEM']->changeContext($contextid);

			$context_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			$output .= $context_asset->attr('body');
			$output .= $context_asset->attr('summary');

			$GLOBALS['SQ_SYSTEM']->restoreContext();
			unset($context_asset);
		}//end foreach

		$existing_links = unserialize($asset->attr('shadow_links'));
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE);

		foreach ($notice_links as $link_data) {
			if (in_array($link_data['minorid'], $existing_links)) continue;
			$existing_links[] = $link_data['minorid'];
		}

		// not only matches './?a=(normal or shadow id)'
		// but also %globals_asset_attribute_link_url:XXX%
		// and %globals_asset_attribute_redirect_url:XXX%
		$shadow_reg  = '|\./\?a=(\d+:[0-9a-zA-Z]*)\$|';
		$normal_reg  = '|\./\?a=(\d+)|';
		$keyword_reg = '/(%globals_asset_attribute_(redirect|link)_url:)([0-9]*)%/';
		$shadow_matches = Array();
		$normal_matches = Array();
		$keyword_match  = Array();

		preg_match_all($shadow_reg,  $output, $shadow_matches);
		preg_match_all($normal_reg,  $output, $normal_matches);
		preg_match_all($keyword_reg, $output, $keyword_matches);

		$shadow_matches  = $shadow_matches[1];
		$normal_matches  = $normal_matches[1];
		$keyword_matches = $keyword_matches[3];

		$replace_assetids = Array();
		foreach ($shadow_matches as $data) {
			$replace_assetids[] = $data;
		}
		foreach ($normal_matches as $data) {
			$replace_assetids[] = $data;
		}
		foreach ($keyword_matches as $data) {
			$replace_assetids[] = $data;
		}
		$replace_assetids = array_unique($replace_assetids);

		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			$notice_links_file = file_to_string($asset->data_path.'/.sq_system/.notice_links');
			if (!empty($notice_links_file)) {
				// this container currently has a system version floating around
				// so we have to be careful not to delete notice links that are
				// still being used in this system version
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$system_assetids = unserialize($notice_links_file);
				$replace_assetids = array_merge($replace_assetids, $system_assetids);
			}//end if
		}//end if

		if (!empty($replace_assetids) || !empty($notice_links)) {

			$replace_assetids = array_unique($replace_assetids);

			// delete existing notice links that are no longer being used (only delete those without a link value)
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$run_level_changed = FALSE;
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_LOCKING);
				$run_level_changed = TRUE;
			}

			foreach ($notice_links as $link_data) {
				// if the link value doesn't indicate a thumbnail, or tag weighting, etc.
				// NOTE: This needs to be refactored.
				if (empty($link_data['value'])) {
					if (!in_array($link_data['minorid'], $replace_assetids)) {
						if (!$asset->deleteLink($link_data['linkid'])) {
							if ($run_level_changed) {
								$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							}
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}
				}
			}

			$shadow_links = array_values(array_intersect(unserialize($asset->attr('shadow_links')), $replace_assetids));
			foreach ($replace_assetids as $assetid) {
				if (!assert_valid_assetid($assetid, '', FALSE, FALSE)) {
					continue;
				}
				if (in_array($assetid, $existing_links)) continue;

				$ereg_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($ereg_asset)) continue;

				if (strpos($ereg_asset->id, ':') !== FALSE) {
					$shadow_links[] = $ereg_asset->id;

				} else {

					// dont show errors here if the link already exists
					// as we rely on the fact that this ereg_asset cant be notice linked
					// to asset mutiple times to stop redunant links from
					// being created (eg same image on a page 10 times should only
					// produce one notice link)
					$asset->createLink($ereg_asset, SQ_LINK_NOTICE);
				}

			}//end foreach

			if (!$asset->setAttrValue('shadow_links', $shadow_links) || !$asset->saveAttributes()) {
				if ($run_level_changed) {
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if ($run_level_changed) {
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


		}//end if replace_assetids or notice_links

		return TRUE;

	}//end processShadowLinks()


}//end class

?>
