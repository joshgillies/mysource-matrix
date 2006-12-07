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
* $Id: page_related_asset_listing.inc,v 1.6 2006/12/07 00:04:23 emcdonald Exp $
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
* @version $Revision: 1.6 $
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

		// get all the assets that would be listed in a normal asset listing in this case
		$unfiltered_assets_to_list = parent::getAssetList();

		// get all the assets related to any of the relators
		$relator_ids = $this->getRelators();
		$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
		$related_assets = $tag_manager->getRelatedAssets($relator_ids);
		$related_ids = array_keys($related_assets);
		$related_ids = array_diff($related_ids, $relator_ids);

		// get all the elements of the unfiltered list that are on the related list
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
			$relator_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'relator');
			foreach ($relator_links as $relator_link) {
				if (strstr($relator_link['minorid'], ':') !== FALSE) {
					// The link minor is a shadow asset
					$relator_ids[] = $relator_link['minorid'];
				} else {
					$relator_ids[] = (int)$relator_link['minorid'];
				}
			}
			$parameter_map =& $this->getAttribute('parameter_map');

			$raw_dynamic_relators = $parameter_map->getParameterValue('relator_node');

			// check if we have a dynamic parameter -- if so, let's do some security checks
			if (!empty($raw_dynamic_relators)) {

				// note that dynamic relators can be supplied as an array of asset ids or
				// as a comma delimited string of asset ids
				if (!is_array($raw_dynamic_relators)) {
					$dynamic_relators = explode(',', $raw_dynamic_relators);
				}

				// if there are dynamic relators, they should override the static relators
				if (!empty($dynamic_relators)) {
					$relator_ids = $dynamic_relators;
				}

			}//end if

			// if no relators have been set anywhere, use this asset as the default
			if (empty($relator_ids)) {
				$relator_ids = Array($this->id);
			}

			$this->_tmp['relator_ids'] = $relator_ids;

		}//end if

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
		$relator_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'relator');
		foreach ($relator_links as $relator_link) {
			$relator_ids[$relator_link['minorid']] =  $relator_link['minorid'];
		}

		return $relator_ids;

	}//end getStaticRelators()


}//end class
?>
