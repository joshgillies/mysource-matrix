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
* $Id: page_asset_listing.inc,v 1.109 2007/01/29 02:47:07 lwright Exp $
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
* @version $Revision: 1.109 $
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
		$assets_to_list = NULL;
		$group_by = $this->attr('group_by');

		// access
		$list_permission = $this->attr('list_permission');
		$list_effective  = $this->attr('list_effective');

		$logic = $this->attr('all_root_nodes') ? 'AND' : 'OR';

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

		if ($this->attr('subs_only')) {

			$side_of_link = ($this->attr('direction') == 'down') ? 'major' : 'minor';
			$link_type = ($side_of_link == 'major') ? 'minor' : 'major';
			$links = Array();

			switch ($group_by) {

				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, $side_of_link, NULL, NULL, NULL, $letter_sort_by, $list_permission, $list_effective);
						if (empty($query)) return Array();
						// the table alias is 'l' rather than 'a' as the query queries the view sq_vw_ast_lnk_minor
						// which returns the results from the asset table, using the table alias 'l'
						$query['select'] .= ', SUBSTR(l.'.$letter_sort_by.', 1, 1) AS first_letter';

						$db =& $GLOBALS['SQ_SYSTEM']->db;
						$links = $db->getAll(implode(' ', $query));
						assert_valid_db_result($links);

						$new_assets_to_list = Array();
						foreach ($links as $link_info) {
							$new_assets_to_list[$link_info[$link_type.'id']] = Array(
																				'type_code'		=> $link_info[$link_type.'_type_code'],
																				'first_letter'	=> $link_info['first_letter'],
																			   );
						}

						$assets_to_list = $this->_combineAssets($logic, $new_assets_to_list, $assets_to_list);
					}

				break;

				case 'random' :
				case 'number' :
				case 'grouped' :
					foreach ($root_asset_ids as $root_asset_id) {
						$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, $side_of_link, NULL, NULL, NULL, NULL, $list_permission, $list_effective);

						$new_assets_to_list = Array();
						foreach ($links as $link_info) {
							$new_assets_to_list[$link_info[$link_type.'id']] = $link_info[$link_type.'_type_code'];
						}

						$assets_to_list = $this->_combineAssets($logic, $new_assets_to_list, $assets_to_list);
					}


				break;
			}//end switch

		} else {

			switch ($group_by) {
				case 'letter' :
					$min_depth = ($this->attr('min_depth') === '') ? NULL : $this->attr('min_depth');
					$max_depth = ($this->attr('max_depth') === '') ? NULL : $this->attr('max_depth');
					foreach ($root_asset_ids as $root_asset_id) {
						if ($this->attr('direction') == 'down') {
							$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);
							$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, $letter_sort_by, $list_permission, $list_effective, TRUE, $min_depth, $max_depth);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
							$table_alias = 'a';
						} else {
							$table_alias = 'll';
							$query = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($root_asset_id, array_keys($wanted_types), FALSE, $letter_sort_by, $list_permission, $list_effective, TRUE, $min_depth, $max_depth);
						}
						if (empty($query)) $new_assets_to_list = Array();

						$query['select'] .= ', SUBSTR('.$table_alias.'.'.$letter_sort_by.', 1, 1) AS first_letter';
						if (strpos($query['group_by'], ($table_alias.'.'.$letter_sort_by)) === FALSE) {
							$query['group_by'] .= ', '.$table_alias.'.'.$letter_sort_by;
						}
						// we need to add the first_letter column to the union so that we have the right
						// number of columns in both sides of the union
						$query['union_select'] .= ', null AS first_letter';

						$db =& $GLOBALS['SQ_SYSTEM']->db;

						$new_assets_to_list = $db->getAssoc(implode(' ', $query));
						assert_valid_db_result($new_assets_to_list);

						$assets_to_list = $this->_combineAssets($logic, $new_assets_to_list, $assets_to_list);
					}
				break;

				case 'grouped' :

					// If there is one grouping type and it's "group by direct
					// parent", we can cache the direct parents now and then
					// groupAssetsRecursively() in Listing Engine can use them,
					// instead of calculating the parents once for each asset

					$min_depth = ($this->attr('min_depth') === '') ? NULL : $this->attr('min_depth');
					$max_depth = ($this->attr('max_depth') === '') ? NULL : $this->attr('max_depth');

					$grouping_attr = $this->attr('asset_grouping');

					if ((count($grouping_attr) == 1) && ($grouping_attr[0]['group_type'] == 'parent_asset') && array_get_index($grouping_attr[0], 'direct_parent_only', FALSE)) {
						$group = $grouping_attr[0];

						$db =& $GLOBALS['SQ_SYSTEM']->db;

						$links_sql = 'SELECT DISTINCT
											minorid, majorid
										FROM
											'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk
										WHERE minorid IN (%s)';

						$types_sql = '
										AND majorid IN
										(
											SELECT assetid FROM sq_ast WHERE %s
										)';


						if (!empty($group['restrict_types']['type_code'])) {
							$types_sub_sql = Array();

							$inherited_types = Array();
							$non_inherited_types = Array();

							// Need to handle which of these type codes are
							// inherited, and which are not
							foreach ($group['restrict_types']['type_code'] as $key => $type) {
								if ($group['restrict_types']['inherit'][$key]) {
									$inherited_types[] = $db->quote($type);
								} else {
									$uninherited_types[] = $db->quote($type);
								}
							}

							if (!empty($uninherited_types)) {
								$types_sub_sql[] = '(type_code IN ('.implode(', ', $uninherited_types).'))';
							}

							if (!empty($inherited_types)) {
								$types_sub_sql[] = '(type_code IN
													(
														SELECT
															type_code
														FROM
															'.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd
														WHERE inhd_type_code IN ('.implode(', ', $inherited_types).')))';
							}

							$types_sub_sql = implode(' OR ', $types_sub_sql);
							$types_sql = sprintf($types_sql, $types_sub_sql);

							$links_sql .= $types_sql;
						}

						$sqls = Array();

						foreach ($root_asset_ids as $root_asset_id) {
							$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);

							$sql = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, NULL, $list_permission, $list_effective, TRUE, FALSE, $min_depth, $max_depth);
							$sql['select'] = str_replace(', a.type_code', '', $sql['select']);
							$sql['union_select'] = str_replace(', null', '', $sql['union_select']);

							$sql = implode(' ', $sql);
							$db =& $GLOBALS['SQ_SYSTEM']->db;

							$sqls[] = '('.sprintf($links_sql, $sql).')';

						}

						$sql = implode(' UNION ', $sqls);

						// The fifth param to getAssoc() lets us get this
						$links = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);
						$this->_tmp['direct_parent_grouping'] = $links;
					}

				// Intentionally fall through; we still need to do the query

				case 'number' :
				case 'random' :

					$min_depth = ($this->attr('min_depth') === '') ? NULL : $this->attr('min_depth');
					$max_depth = ($this->attr('max_depth') === '') ? NULL : $this->attr('max_depth');

					if ($this->attr('direction') == 'down') {
						foreach ($root_asset_ids as $root_asset_id) {
							$asset_list = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset_id, array_keys($wanted_types), FALSE, NULL, NULL, $list_permission, $list_effective, FALSE, $min_depth, $max_depth);
							$assets_to_list = $this->_combineAssets($logic, $asset_list, $assets_to_list);
						}
					} else {
						foreach ($root_asset_ids as $root_asset_id) {
							$asset_list = $GLOBALS['SQ_SYSTEM']->am->getParents($root_asset_id, array_keys($wanted_types), NULL, NULL, $list_permission, $list_effective, TRUE, $min_depth, $max_depth);
							$assets_to_list = $this->_combineAssets($logic, $asset_list, $assets_to_list);
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


	/**
	* Combines assets according to the logic given
	*
	* If $existing is NULL, the new assets array will be returned unchanged.
	* This allows a starting point for the first root node, which can then be
	* updated using this method for further root nodes.
	*
	* The arrays $new_assets and $existing are keyed by asset ID.
	*
	* @param string		$logic		either "OR" (union) or "AND" (intersection)
	* @param array		$new_assets Set of new assets to add (or keep)
	* @param array|NULL	$existing	Set of existing assets to use as a base or
	* 								NULL if first root node (see above)
	*
	* @return array
	* @access private
	*/
	function _combineAssets($logic='OR', $new_assets=Array(), $existing=NULL)
	{
		if (is_null($existing)) {
			return $new_assets;
		}

		if ($logic !== 'AND') {
			// OR logic: as it was before, just add 'em
			$existing += $new_assets;
		} else {
			// AND logic: Oh how we long for PHP 5.1 and array_intersect_key()
			// So we need to do this instead...
			foreach($existing as $assetid => $data) {
				if (!isset($new_assets[$assetid])) {
					unset($existing[$assetid]);
				}
			}
		}

		return $existing;

	}//end _combineAssets()


}//end class
?>
