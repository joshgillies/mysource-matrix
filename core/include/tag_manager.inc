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
* $Id: tag_manager.inc,v 1.3 2005/12/20 03:06:24 emcdonald Exp $
*
*/


/**
* Tag Manager
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Tag_Manager extends MySource_Object
{


	/**
	* Constructor
	*
	*/
	function Tag_Manager()
	{
		parent::MySource_Object();

	}//end constructor


	/**
	* Get all the tags on an asset
	*
	* @param int	$assetid	The asset who's tag we want.
	*
	* @return array
	* @access public
	*/
	function getTagLinks($assetid)
	{

		$tag_links = Array();
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_NOTICE);
		foreach ($notice_links as $notice_link) {
			$id_parts = explode(':', $notice_link['minorid']);
			$thes_id = $id_parts[0];
			$thes = &$GLOBALS['SQ_SYSTEM']->am->getAsset($thes_id, 'thesaurus', true);
			if ($thes) $tag_links[] = $notice_link;
		}

		return $tag_links;

	}//end getTagLinks()


	/**
	* Get all the assets that are tagged with a tag or tags
	*
	* @param int	$tagid	The tag or array of tags in which we are interested
	*
	* @return array
	* @access public
	*/
	function getAssetLinksByTag($tagid)
	{
		//get a list of links to assets from these tags

		if (!is_array($tagid)) {
			$tagids = Array($tagid);
		} else {
			$tagids = $tagid;
		}

		$links = Array();
		$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($tagids, SQ_LINK_NOTICE, null, false, 'minor');

		foreach ($asset_links as $asset_link) {
			foreach ($asset_link as $tag_link) {
				$links[] = $tag_link;
			}
		}

		return $links;

	}//end getAssetLinksByTag()


	/**
	* Get all the assets that are related to an asset or assets
	*
	* @param array	$relator_id	The tag or array of tags in which we are interested
	*
	* @return array
	* @access public
	*/
	function getRelatedAssets($relator_id)
	{
		//Get the tag links for all the relators then all the asset links these tags have.
		//we need to iterate through the relator's tag links to build the array for getting the tags' assets, so
		//build another array of link values.

		$tag_ids = Array();
		if (!is_array($relator_id)) {
			$$relator_ids = Array($relator_id);
		} else {
			$relator_ids = $relator_id;
		}
		$multiplicands = Array();
		foreach ($relator_ids as $relator_id) {
			$tag_links = $this->getTagLinks($relator_id);

			foreach ($tag_links as $tag_link) {
				$tag_asset_id =  $tag_link['minorid'];
				$tag_ids[] = $tag_asset_id;
				if (is_null(array_get_index($multiplicands, $tag_asset_id))) {
					$multiplicands[$tag_asset_id] = Array();
				}
				$multiplicands[$tag_asset_id][] = $tag_link['value'];
			}
		}

		//related_asset_links are links from the relatees to the relator's tags
		$related_asset_links = $this->getAssetLinksByTag($tag_ids);

		$related_ids = Array();
		//loop through relateds and check relators for each.
		foreach ($related_asset_links as $related_asset_link) {

			foreach ( $multiplicands[$related_asset_link['minorid']] as $multiplicand) {
				$current_value = array_get_index($related_ids, $related_asset_link['majorid']);
				if (is_null($current_value)) {
					$related_ids[$related_asset_link['majorid']] = Array();
				}
				$related_ids[$related_asset_link['majorid']] = $current_value + $multiplicand*$related_asset_link['value'];
			}
		}

		arsort($related_ids);

		return $related_ids;;

	}//end getRelatedAssets()


}//end class
?>