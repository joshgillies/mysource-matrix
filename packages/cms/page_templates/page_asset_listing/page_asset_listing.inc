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
* $Id: page_asset_listing.inc,v 1.99 2006/01/20 02:48:38 sdanis Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';

/**
* Page_Asset_Listing
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.99 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing extends Listing_Engine
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Asset_Listing($assetid=0)
	{
		$this->Listing_Engine($assetid);

	}//end constructor


	/**
	* Get the list of asset that should be printed
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
		$assets_to_list = Array();
		$group_by = $this->attr('group_by');

		// this is the list of all type codes that we are going to show
		// we pass this list to the getLinks and getChildren functions (below) to narrow
		// the query down a bit
		// if there are no types to list, we cant list anything
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) {
			trigger_localised_error('CMS0019', E_USER_NOTICE);
			return Array();
		}

		// get the root assets whose sub-assets/children we are displaying
		// This may come from the parameter map, but the dynamic value must be a child of the static value
		// if there are no root assets, we will use ourselves and print our children
		$root_asset_ids = $this->getRootNodes();

		$sort_info = $this->getSortInfo();

		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		if ($group_by == 'letter') {
			if (empty($sort_info) || !($sort_info['type'] == 'field') || ($sort_info['params']['field'] != 'short_name')) {
				$letter_sort_by = 'name';
			} else {
				$letter_sort_by = $sort_info['params']['field'];
			}
		}

		$assets_to_list = Array();
		if ($this->attr('subs_only')) {

			$side_of_link = ($this->attr('direction') == 'down') ? 'major' : 'minor';
			$link_type = ($side_of_link == 'major') ? 'minor' : 'major';
			$links = Array();

			switch ($group_by) {

				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, $side_of_link, NULL, NULL, NULL, $letter_sort_by, SQ_PERMISSION_READ);
						if (empty($query)) return Array();
						// the table alias is 'l' rather than 'a' as the query queries the view sq_vw_ast_lnk_minor
						// which returns the results from the asset table, using the table alias 'l'
						$query['select'] .= ', SUBSTR(l.'.$letter_sort_by.', 1, 1) AS first_letter';

						$db =& $GLOBALS['SQ_SYSTEM']->db;
						$new_links = $db->getAll(implode(' ', $query));
						assert_valid_db_result($new_links);
						$links = array_merge($links, $new_links);
					}
					foreach ($links as $link_info) {
						$assets_to_list[$link_info[$link_type.'id']] = Array(
																		'type_code'		=> $link_info[$link_type.'_type_code'],
																		'first_letter'	=> $link_info['first_letter'],
																	   );
					}
				break;

				case 'random' :
				case 'number' :
				case 'grouped' :
					foreach ($root_asset_ids as $root_asset_id) {
						$new_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, $side_of_link, NULL, NULL, NULL, NULL, SQ_PERMISSION_READ);
						$links = array_merge($links, $new_links);
					}
					foreach ($links as $link_info) {
						$assets_to_list[$link_info[$link_type.'id']] = $link_info[$link_type.'_type_code'];
					}

				break;
			}//end switch

		} else {

			switch ($group_by) {
				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						if ($this->attr('direction') == 'down') {
							$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);
							$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, $letter_sort_by, SQ_PERMISSION_READ);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
						} else {
							$query = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($root_asset_id, array_keys($wanted_types), FALSE, $letter_sort_by, SQ_PERMISSION_READ);
						}
						if (empty($query)) $new_assets_to_list = Array();

						// the table alias here, joins the asset table using the table alias 'a'
						// so this query is fine with 'a'
						$query['select'] .= ', SUBSTR(a.'.$letter_sort_by.', 1, 1) AS first_letter';
						// we need to add the first_letter column to the union so that we have the right
						// number of columns in both sides of the union
						$query['union_select'] .= ', null AS first_letter';

						$db =& $GLOBALS['SQ_SYSTEM']->db;
						$new_assets_to_list = $db->getAssoc(implode(' ', $query));
						assert_valid_db_result($new_assets_to_list);
						$assets_to_list = $assets_to_list + $new_assets_to_list;
					}
				break;

				case 'number' :
				case 'random' :
				case 'grouped' :
					$min_depth = ($this->attr('min_depth') === '') ? NULL : $this->attr('min_depth');
					$max_depth = ($this->attr('max_depth') === '') ? NULL : $this->attr('max_depth');

					if ($this->attr('direction') == 'down') {
						foreach ($root_asset_ids as $root_asset_id) {
							$assets_to_list = $assets_to_list + $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset_id, array_keys($wanted_types), FALSE, NULL, NULL, SQ_PERMISSION_READ, $min_depth, $max_depth);
						}
					} else {
						foreach ($root_asset_ids as $root_asset_id) {
							$assets_to_list = $assets_to_list + $GLOBALS['SQ_SYSTEM']->am->getParents($root_asset_id, array_keys($wanted_types), NULL, NULL, SQ_PERMISSION_READ, $min_depth, $max_depth);
						}
					}

				break;
			}//end switch

		}//end else subs_only

		if (empty($assets_to_list)) return Array();

		if ($group_by == 'number') {
			$assets_to_list = $this->sortAssetList($assets_to_list, $sort_info);
		}

		return $assets_to_list;

	}//end getAssetList()


}//end class
?>
