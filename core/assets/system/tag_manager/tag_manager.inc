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
* $Id: tag_manager.inc,v 1.3.2.1 2006/12/05 22:51:43 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Tag Manager
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.3.2.1 $
* @package MySource_Matrix
*/
class Tag_Manager extends Asset
{


	/**
	* Constructor
	*
	*/
	function Tag_Manager($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Returns either the linkid or FALSE on error
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Return FALSE always because we don't want to be able to clone a tag manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


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
				if (!is_null($thes)) {
					$tag_term_exists =& $GLOBALS['SQ_SYSTEM']->am->getAsset($notice_link['minorid'], 'thesaurus_term', FALSE);
					if (!is_null($tag_term_exists)) {
						$tag_links[] = $notice_link;
					}
				}
			}
		}

		return $tag_links;

	}//end getTagLinks()


	/**
	* Is asset tagged with a certain tag
	*
	* @param int		$assetid	The asset who's tag we want
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
	* @param string	$tagid		the id of the tag to add
	* @param int	$weight		the weight of the tag link
	*
	* @return int
	* @access public
	*/
	function setTag($assetid, $tagid, $weight)
	{
		if (!$this->isFromTaggingThesaurus($tagid)) {
			// this tag is not in the required thesaurus
			trigger_error('Tags must be selected from the tagging thesaurus', E_USER_WARNING);
			return 0;
		}

		$new_tag_object =& $GLOBALS['SQ_SYSTEM']->am->getAsset($tagid);
		$asset          =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($new_tag_object) || is_null($asset)) {
			return 0;
		}

		return $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_tag_object, SQ_LINK_NOTICE, $weight);

	}//end setTag()


	/**
	* Tag an asset by name
	*
	* NOTE: Will only search for terms in the the tagging thesaurus
	*
	* @param string	$assetid	the ID of the asset to tag
	* @param int	$tag		the name of the tag to add
	* @param int	$weight		the weight of the tag link
	*
	* @return int
	* @access public
	*/
	function setTagByName($assetid, $tag, $weight)
	{
		$thes  =& $this->getTagThesaurus();
		$tagid =  $thes->getTermIdByName($tag);
		return $this->setTag($assetid, $tagid, $weight);

	}//end setTagByName()


	/**
	* Returns the thesaurus asset used for tagging
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getTagThesaurus()
	{
		$thes = NULL;
		if ($this->attr('thesaurus')) {
			$thes =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('thesaurus'));
		}
		return $thes;

	}//end getTagThesaurus()


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
		$success = $GLOBALS['SQ_SYSTEM']->am->updateLink($linkid, SQ_LINK_NOTICE, $weight);
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
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($assetid, $tag, SQ_LINK_NOTICE);
		$success = $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);

		return $success;

	}//end deleteTag()


	/**
	* Checks whether the tags are restricted to a single thesaurus, and if so,
	* checks if a thesaurus term is from the selected tagging thesaurus
	*
	* @param string	$tagid	the id of the tag to check
	*
	* @return boolean
	* @access public
	*/
	function isFromTaggingThesaurus($tagid)
	{
		// are tags restricted to one thesaurus asset?
		if ($this->isTagSourceRestricted()) {
			$id_parts = explode(':', $tagid);
			if ($this->attr('thesaurus') != $id_parts[0]) {
				// this tag is not in the required thesaurus
				return FALSE;
			}
		}

		return TRUE;

	}//end isFromTaggingThesaurus()


	/**
	* Determines whether or not tags are restricted to a single thesaurus
	*
	* @return boolean
	* @access public
	*/
	function isTagSourceRestricted()
	{
		return $this->attr('restrict_tag_source');

	}//end isTagSourceRestricted()

}//end class
?>