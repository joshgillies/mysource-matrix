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
* $Id: page_related_asset_listing.inc,v 1.2 2005/12/15 22:47:11 emcdonald Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';

/**
* Page_Related_Asset_Listing
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Related_Asset_Listing extends Page_Asset_Listing
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Related_Asset_Listing($assetid=0)
	{
		$this->Page_Asset_Listing($assetid);

	}//end constructor


	/**
	* Get the list of assets that should be printed
	*
	* The return array is in the form Array(assetid => type_code) unless we are grouping by letter,
	* in which case the return array is
	* <pre>
	* Array(assetid => Array(
	*						'type_code' => type_code,
	*						'first_letter' => first_letter,
	*						);
	*		);
	* </pre>
	*
	* @return array
	* @access protected
	*/
	function getAssetList()
	{
		//get all the assets that would be listed in a normal asset listing in this case
		$unfiltered_assets_to_list = parent::getAssetList();

		//get all the assets related to any of the relators
		$relator_ids = $this->getRelators();
		$tag_manager = &$GLOBALS['SQ_SYSTEM']->getTagManager();
		$related_assets = $tag_manager->getRelatedAssets($relator_ids);

		$related_ids = array_values($related_assets);

		//get all the elements of the unfiltered list that are on the related list
		$assets_to_list = Array();

		foreach ($related_ids as $related_id) {
			$asset_to_list = array_get_index($unfiltered_assets_to_list, $related_id);
			if (!is_null($asset_to_list)) {
				$assets_to_list[$related_id] = $asset_to_list;
			}
		}
		return $assets_to_list;
	}//end getAssetList()


	/**
	* Gets the list of current relators
	*
	* @return array
	* @access public
	*/
	function getRelators()
	{
		if (!isset($this->_tmp['relator_asset_ids'])) {
			$relator_ids = Array();
			$relator_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', false, 'major', 'relator');
			foreach ($relator_links as $relator_link) {
				if (strstr($relator_link['minorid'], ':') !== false) {
					// The link minor is a shadow asset
					$relator_ids[] = $relator_link['minorid'];
				} else {
					$relator_ids[] = (int)$relator_link['minorid'];
				}
			}
			$parameter_map = &$this->getAttribute('parameter_map');

			$raw_dynamic_relators = $parameter_map->getParameterValue('relator_node');

			// check if we have a dynamic parameter -- if so, let's do some security checks
			if (!empty($raw_dynamic_relators)) {

				$dynamic_relators = Array();
				// note that dynamic relators can be supplied as an array of asset ids or
				// as a comma delimited string of asset ids
				if (!is_array($raw_dynamic_relators)) {
					$raw_dynamic_relators = explode(',', $raw_dynamic_relators);
				}

				foreach ($raw_dynamic_relators as $dynamic_relator) {
					if (!strlen($dynamic_relator)) continue;

					$dynamic_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($dynamic_relator);

					// first, check to see if the dynamic parameter specified is already one of the relators
					// if so, just use that single relator
					if (in_array($dynamic_relator, $relator_ids)) {
						$dynamic_relators[] = $dynamic_relator;
					} else {
						// otherwise, check to see if the dynamic parameter specified is a child of one of the static relators
						// If so, use the dynamic parameter as the relator; otherwise return an error.
						$matching_relator_ids = array_intersect(array_keys($dynamic_parents), $relator_ids);
						if (empty($matching_relator_ids)) {
							trigger_localised_error('CMS0020', E_USER_WARNING, $dynamic_relator);
						} else {
							$dynamic_relators[] = $dynamic_relator;
						}
					}
				}

				if (!empty($dynamic_relators)) {
					$relator_ids = $dynamic_relators;
				}

			}
			//if no relators have been set anywhere, use this asset as the default
			if (empty($relator_ids)) {
				$relator_ids = Array($this->id);
			}

			$this->_tmp['relator_ids'] = $relator_ids;

		}

		return $this->_tmp['relator_ids'];

	}//end getRelators()


	/**
	* Gets the asset id's of our relators but ignores dynamic parameters
	*
	* @return array
	* @access public
	*/
	function getStaticRelators()
	{
		$relator_ids = Array();
		$relator_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', false, 'major', 'relator');
		foreach ($relator_links as $relator_link) {
			$relator_ids[$relator_link['minorid']] =  $relator_link['minorid'];
		}

		return $relator_ids;

	}//end getStaticRelators()


}//end class
?>
