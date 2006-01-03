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
* $Id: tag_manager.inc,v 1.8 2006/01/03 05:21:35 arailean Exp $
*
*/


/**
* Tag Manager
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.8 $
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
			if (!empty($id_parts[1])) {
				$thes_id = $id_parts[0];
				$thes =& $GLOBALS['SQ_SYSTEM']->am->getAsset($thes_id, 'thesaurus', TRUE);
				if (!is_null($thes)) $tag_links[] = $notice_link;
			}
		}

		return $tag_links;

	}//end getTagLinks()


	/**
	* Is asset tagged with a certain tag
	*
	* @param int		$assetid	The asset who's tag we want.
	* @param mixed		$tagids		Tag ID (array, or one value)
	* @param boolean	$strict		Whether all tags must be present for success
	*
	* @return boolean
	* @access public
	*/
	function isAssetTaggedWith($assetid, $tagids, $strict=FALSE)
	{
		if (!is_array($tagids)) $tagids = Array($tagids);

		$tag_link = $this->getAssetTagLinks($assetid, $tagids);
		if (!$strict) {
			return !empty($tag_link);
		} else {
			$array_keys = array_keys($tag_link);
			$result = array_diff($tagids, $array_keys);
			return empty($result);
		}

	}//end isAssetTaggedWith()


	/**
	* Get info of the link between asset and tag
	*
	* @param int	$assetid	The asset who's tag we want.
	* @param mixed	$tagids		Tag ID (array, or one value)
	*
	* @return array
	* @access public
	*/
	function getAssetTagLinks($assetid, $tagids)
	{
		$tag_links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($assetid, $tagids, SQ_LINK_NOTICE);
		if (isset($tag_links['minorid'])) {
			$tag_links = Array($tag_links);
		}

		$links = Array();

		foreach ($tag_links as $link) {
			$links[$link['minorid']] = $link;
		}

		return $links;

	}//end getAssetTagLinks()


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
		// get a list of links to assets from these tags

		if (!is_array($tagid)) {
			$tagids = Array($tagid);
		} else {
			$tagids = $tagid;
		}

		$links = Array();
		$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($tagids, SQ_LINK_NOTICE, NULL, FALSE, 'minor');

		return $asset_links;

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
		// Get the tag links for all the relators then all the asset links these tags have.
		// we need to iterate through the relator's tag links to build the array for getting the tags' assets, so
		// build another array of link values.

		if (!is_array($relator_id)) {
			$relator_ids = Array($relator_id);
		} else {
			$relator_ids = $relator_id;
		}


		$relator_weightings = Array();
		foreach ($relator_ids as $relator_id) {
			$tag_links = $this->getTagLinks($relator_id);
			foreach ($tag_links as $tag_link) {
				$tag_id =  $tag_link['minorid'];
				$current_value = array_get_index($relator_weightings, $tag_id, 0);
				$relator_weightings[$tag_id] = $current_value + $tag_link['value'];
			}
		}


		// related_asset_links are links from the relatees to the relator's tags
		$all_related_asset_links = $this->getAssetLinksByTag(array_keys($relator_weightings));


		$related_ids = Array();
		// loop through relateds and check relators for each
		foreach ($all_related_asset_links as $tag_id => $related_asset_links) {
			$multiplier = array_get_index($relator_weightings, $tag_id);

			if (is_null($multiplier)) continue;

			foreach ($related_asset_links as $one_related_link) {
				$related_assetid = $one_related_link['majorid'];
				$current_value = array_get_index($related_ids, $related_assetid, 0);
				$related_ids[$related_assetid] = $current_value + $multiplier * $one_related_link['value'];
			}
		}

		arsort($related_ids);


		return $related_ids;

	}//end getRelatedAssets()


	/**
	* Tag an asset
	*
	* @param string	$assetid	the ID of the asset to tag
	* @param int	$tag		the tag to add
	* @param int	$weight		the weight of the tag link
	*
	* @return int
	* @access public
	*/
	function setTag($assetid, $tag, $weight)
	{
		$new_tag_object =& $GLOBALS['SQ_SYSTEM']->am->getAsset($tag);
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$new_link_id = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_tag_object, SQ_LINK_NOTICE, $weight);

		return $new_link_id;

	}//end setTag()


	/**
	* Change the weight of a tag link for an asset
	*
	* @param int	$linkid	the ID of the link to update
	* @param int	$weight	the new weight of the tag link
	*
	* @return boolean
	* @access public
	*/
	function setTagWeight($linkid, $weight)
	{
		$success = $GLOBALS['SQ_SYSTEM']->am->updateLink($link_id, SQ_LINK_NOTICE, $weight);
		return $success;

	}//end setTagWeight()


	/**
	* Delete a tag from an asset
	*
	* @param string	$assetid	the ID of the asset to tag
	* @param int	$tag		the tag to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteTag($assetid, $tag)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($assetod, $one_tag_id, SQ_LINK_NOTICE);
		$success = $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);

		return $success;

	}//end deleteTag()


}//end class
?>