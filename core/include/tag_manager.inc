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
* $Id: tag_manager.inc,v 1.2 2005/12/15 00:15:07 emcdonald Exp $
*
*/


/**
* Tag Manager
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.2 $
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
	function getAssetsByTag($tagid)
	{
		//get a list of assets using this tag from the shadow links table

		if (!is_array($tagid)) {
			$tagids = Array($tagid);
		} else {
			$tagids = $tagid;
		}
		$assets = Array();
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		foreach ($tagids as $tagid) {
			$sql = 'SELECT
						majorid
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk
					WHERE
						minorid = '.$db->quoteSmart($tagid).'
					AND
						link_type = '.$db->quoteSmart(SQ_LINK_NOTICE).'
					';

			$result = $db->getAll($sql);
			assert_valid_db_result($result);

			foreach ($result as $majorid) {
				$assets[] = $majorid['majorid'];
			}
		}


		return $assets;

	}//end getAssetsByTag()


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
		$tag_ids = Array();
		if (!is_array($relator_id)) {
			$$relator_ids = Array($relator_id);
		} else {
			$relator_ids = $relator_id;
		}

		foreach ($relator_ids as $relator_id) {
			$tag_links = $this->getTagLinks($relator_id);

			foreach ($tag_links as $tag_link) {
				$tag_ids[] = $tag_link['minorid'];
			}
		}

		$related_assets = $this->getAssetsByTag($tag_ids);
		$related_assets = array_diff($related_assets, $relator_ids);

		return $related_assets;

	}//end getRelatedAssets()


}//end class
?>