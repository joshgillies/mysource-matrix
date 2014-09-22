<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_listing.inc,v 1.141 2013/08/16 05:51:37 ewang Exp $
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
* @version $Revision: 1.141 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

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
		$group_by = $this->attr('group_by');

		$logic = $this->attr('all_root_nodes') ? 'AND' : 'OR';

		// this is the list of all type codes that we are going to show
		// we pass this list to the getLinks and getChildren functions (below) to narrow
		// the query down a bit
		// if there are no types to list, we cant list anything
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) {
			trigger_localised_error('CMS0019', translate('No type settings have been configured for this page - no assets will be listed'), E_USER_NOTICE)
			return Array();
		}

		// get the root assets whose sub-assets/children we are displaying
		// This may come from the parameter map, but the dynamic value must be a child of the static value
		// if there are no root assets, we will use ourselves and print our children
		$root_asset_ids = $this->getRootNodes();

		$sort_info = $this->getSortInfo();

		$link_value_wanted = NULL;
		if ($this->attr('link_value_filter_enabled')) {
			// the data stored in the DB was encoded to protect against cross scripting so we use htmlentities
			$link_value_wanted = htmlentities($this->attr('link_value_filter'));
			replace_global_keywords($link_value_wanted);
		}

		$assets_to_list = $this->_getAssetList($group_by, $logic, $wanted_types, $root_asset_ids, $sort_info, $link_value_wanted);

		return $assets_to_list;

	}//end getAssetList()


	/**
	* Get the list of assets that should be printed without sorting
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
	* @param string	$group_by			grouping option
	* @param string	$logic				root nodes inclusion logic
	* @param array	$wanted_types		an array of asset types to list
	* @param array	$root_asset_ids		an array of root assetids
	* @param array	$sort_info			sorting information as provided by getSortInfo()
	* @param array	$link_value_wanted	link value wanted
	*
	* @return array
	* @access protected
	*/
	function _getAssetList($group_by, $logic, $wanted_types, $root_asset_ids, $sort_info, $link_value_wanted=NULL)
	{
		$assets_to_list = NULL;

		// access
		$list_permission = $this->attr('list_permission');
		$list_effective  = $this->attr('list_effective');

		$parameter_map =& $this->getAttribute('parameter_map');
		$raw_assets_to_list = $parameter_map->getParameterValue('asset_selection');
		if (!empty($raw_assets_to_list)) {
			$assets_to_list = Array();

			if (!is_array($raw_assets_to_list)) {
				// If passed as an array, treat it similar to replacement
				// root nodes. In other words, asset IDs (including that of LDAP user) separated by commas.
				preg_match_all('|[0-9]+(:([0-9a-z_\-\s\(\)\\\]+=[0-9a-z_\-\s\(\)\\\]+,)*[0-9a-z_\-\s\(\)\\\]+=[0-9a-z_\-\s\(\)\\\]+)?|i', $raw_assets_to_list, $matches);
				$list_keys = $matches[0];
				$raw_assets_to_list = Array();
				foreach ($list_keys as $raw_assetid) {
					if ($raw_assetid) {
						$raw_assets_to_list[$raw_assetid] = 1;
					}
				}
			}

			foreach ($raw_assets_to_list as $raw_asset => $enabled) {
				$list_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($raw_asset);
				if (is_null($list_asset)) continue;

				// Ok, we have the asset, let's check permissions
				$allowed = FALSE;
				switch ($list_permission) {

					case SQ_PERMISSION_READ :
						$allowed = $list_asset->readAccess();
					break;

					case SQ_PERMISSION_WRITE :
						$allowed = $list_asset->writeAccess();
					break;

					case SQ_PERMISSION_ADMIN :
						$allowed = $list_asset->adminAccess();
					break;

				}

				if (!$allowed) continue;

				// Now, let's check that the asset(s) are under the root nodes
				$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($list_asset->id);

				$allowed = FALSE;
				foreach ($parents as $parent_id => $parent_type) {
					if (in_array($parent_id, $root_asset_ids)) {
						$allowed = TRUE;
					}
				}

				if (!$allowed) continue;

				$assets_to_list[$list_asset->id][0]['type_code'] = $list_asset->type();

			}//end foreach

			return $assets_to_list;

		} else {
			$dynamic_asset_selection = $parameter_map->getParameters();
			if (in_array('asset_selection', $dynamic_asset_selection) && $this->attr('dynamic_asset_selection_option') == 'empty_result') {
				return Array();
			}
		}//end if

		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		if ($group_by == 'letter') {
			if (empty($sort_info) || !($sort_info['type'] == 'field') || ($sort_info['params']['field'] != 'short_name')) {
				$letter_sort_by = 'name';
			} else {
				$letter_sort_by = $sort_info['params']['field'];
			}
			$letter_sort_by = preg_replace('/[^a-zA-Z0-9_]+/', '', $letter_sort_by);
		}

		if (!is_null($link_value_wanted)) {
			$link_value_wanted = Array(
									'link_value' 	=> Array($link_value_wanted),
									'equal' 		=> $this->attr('link_value_filter_logic'),
								 );
		}

		if ($this->attr('subs_only')) {

			$side_of_link = ($this->attr('direction') == 'down') ? 'major' : 'minor';
			$link_type = ($side_of_link == 'major') ? 'minor' : 'major';
			$links = Array();

			switch ($group_by) {

				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, $side_of_link, $link_value_wanted, NULL, NULL, $letter_sort_by, $list_permission, $list_effective);
						if (empty($links_query)) return Array();
						// the table alias is 'l' rather than 'a' as the query queries the view sq_vw_ast_lnk_minor
						// which returns the results from the asset table, using the table alias 'l'
						$links_query['sql_array']['select'] .= ', SUBSTR(l.'.$letter_sort_by.', 1, 1) AS first_letter';

						$links = NULL;

						try {
							$query = MatrixDAL::preparePdoQuery(implode(' ', $links_query['sql_array']));

							foreach ($links_query['bind_vars'] as $bind_var => $bind_value) {
								MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
							}

							$links = MatrixDAL::executePdoAll($query);
						} catch (Exception $e) {
							throw new Exception($e->getMessage().' (in '.$e->getFile().' on line '.$e->getLine().')');
						}

						$new_assets_to_list = Array();
						foreach ($links as $link_info) {
							$new_assets_to_list[$link_info[$link_type.'id']] = Array(
																				0	=> Array(
																						'type_code'		=> $link_info[$link_type.'_type_code'],
																						'first_letter'	=> $link_info['first_letter'],
																					   ),
																			   );
						}

						$assets_to_list = $this->_combineAssets($logic, $new_assets_to_list, $assets_to_list);
					}//end foreach

				break;

				case 'random' :
				case 'number' :
				case 'grouped' :
					foreach ($root_asset_ids as $root_asset_id) {
						$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, $side_of_link, $link_value_wanted, NULL, NULL, NULL, $list_permission, $list_effective);
						$new_assets_to_list = Array();
						foreach ($links as $link_info) {
							$new_assets_to_list[$link_info[$link_type.'id']] = Array( 0 => Array('type_code' => $link_info[$link_type.'_type_code']));
						}

						$assets_to_list = $this->_combineAssets($logic, $new_assets_to_list, $assets_to_list);
					}


				break;
			}//end switch

		} else {
			$min_depth = ($this->attr('min_depth') === '') ? NULL : $this->attr('min_depth');
			$max_depth = ($this->attr('max_depth') === '') ? NULL : $this->attr('max_depth');

			// if the min/max are both set to 0, return only the root nodes and forego any other processing
			if ($min_depth === '0' && $max_depth === '0') {
				$assets_to_list = $this->_combineRootNodes(Array(), $root_asset_ids, $wanted_types);
				return empty($assets_to_list) ? Array() : $assets_to_list;
			}

			switch ($group_by) {
				case 'letter' :
					foreach ($root_asset_ids as $root_asset_id) {
						if ($this->attr('direction') == 'down') {
							$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);
							$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, $letter_sort_by, $list_permission, $list_effective, TRUE, $min_depth, $max_depth, TRUE, $link_value_wanted);
							$sql_array = $ret_val['sql_array'];
							$bind_vars = $ret_val['bind_vars'];
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
							$table_alias = 'a';
						} else {
							$table_alias = 'll';
							$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($root_asset_id, array_keys($wanted_types), FALSE, $letter_sort_by, $list_permission, $list_effective, $min_depth, $max_depth, $link_value_wanted);
							$sql_array = $ret_val['sql_array'];
							$bind_vars = $ret_val['bind_vars'];
						}
						if (empty($ret_val)) $new_assets_to_list = Array();

						$sql_array['select'] .= ', SUBSTR('.$table_alias.'.'.$letter_sort_by.', 1, 1) AS first_letter';
						if (strpos($sql_array['group_by'], ($table_alias.'.'.$letter_sort_by)) === FALSE) {
							$sql_array['group_by'] .= ', '.$table_alias.'.'.$letter_sort_by;
						}
						// we need to add the first_letter column to the union so that we have the right
						// number of columns in both sides of the union
						$sql_array['union_select'] .= ', null AS first_letter';

						$new_assets_to_list = Array();
						try {
								$query = MatrixDAL::preparePdoQuery(implode(' ', $sql_array));
								foreach ($bind_vars as $bind_var => $bind_value) {
									MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
								}
								$new_assets_to_list = MatrixDAL::executePdoGroupedAssoc($query);
						} catch (Exception $e) {
								throw new Exception($e->getMessage());
						}


						$assets_to_list = $this->_combineAssets($logic, $new_assets_to_list, $assets_to_list);
					}//end foreach
				break;

				case 'grouped' :

					// If there is one grouping type and it's "group by direct
					// parent", we can cache the direct parents now and then
					// groupAssetsRecursively() in Listing Engine can use them,
					// instead of calculating the parents once for each asset

					$grouping_attr = $this->attr('asset_grouping');

					if ((count($grouping_attr) == 1) && ($grouping_attr[0]['group_type'] == 'parent_asset') && array_get_index($grouping_attr[0], 'direct_parent_only', FALSE)) {
						$group = $grouping_attr[0];

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
									$inherited_types[] = MatrixDAL::quote($type);
								} else {
									$uninherited_types[] = MatrixDAL::quote($type);
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
						}//end if

						$sqls = Array();

						$bind_vars = Array();
						foreach ($root_asset_ids as $root_asset_id) {
							$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);

							$bind_prefix = 'gc_'.$root_asset_id.'_';
							$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, NULL, $list_permission, $list_effective, TRUE, $min_depth, $max_depth, TRUE, $link_value_wanted, $bind_prefix);
							$sql = $ret_val['sql_array'];
							$bind_vars = array_merge($bind_vars, $ret_val['bind_vars']);

							$sql['select'] = str_replace(', a.type_code', '', $sql['select']);
							$sql['union_select'] = str_replace(', null', '', $sql['union_select']);

							$sql = implode(' ', $sql);
							$sqls[] = '('.sprintf($links_sql, $sql).')';

						}

						$sql = implode(' UNION ', $sqls);

						$links = NULL;
						try {
							$query = MatrixDAL::preparePdoQuery($sql);
							foreach ($bind_vars as $bind_var => $bind_value) {
								MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
							}
							$links = MatrixDAL::executePdoGrouped($query);
						} catch (Exception $e) {
							throw new Exception($e->getMessage());
						}

						$childids = array_keys($links);
						$direct_parents = Array();
						$restrict_types = array_get_index($group, 'restrict_types', Array());
						$direct_parent_only = array_get_index($group, 'direct_parent_only', FALSE);
						foreach ($childids as $childid) {
							foreach ($links[$childid] as $key => $direct_parent) {
								if (count($links[$childid]) > 1) {
									// resort to calling getGroupableParentAssetids() only if there is more than one parent
									// otherwise the resulting direct parents may lie outside the root node
									$direct_parents[$childid] = $this->getGroupableParentAssetids($childid, $restrict_types, $direct_parent_only);
									continue;
								} else {
									$direct_parents[$childid][] = $direct_parent[0];
								}
							}
						}

						$this->_tmp['direct_parent_grouping'] = $direct_parents;
					}//end if

				// Intentionally fall through; we still need to do the query

				case 'number' :
				case 'random' :
					if ($this->attr('direction') == 'down') {
						foreach ($root_asset_ids as $root_asset_id) {
							$asset_list = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_asset_id, array_keys($wanted_types), FALSE, NULL, NULL, $list_permission, $list_effective, $min_depth, $max_depth, TRUE, $link_value_wanted);

							$assets_to_list = $this->_combineAssets($logic, $asset_list, $assets_to_list);
						}
					} else {
						foreach ($root_asset_ids as $root_asset_id) {
							// we are now formating the asset_list the way it should be
							// the function getParents is used at many places in Matrix therefore we are adding the following code to format
							// the returned array. This might affect the performances of the asset listing.
							$asset_list = Array();
							$new_asset_list = $GLOBALS['SQ_SYSTEM']->am->getParents($root_asset_id, array_keys($wanted_types), FALSE, NULL, $list_permission, $list_effective, $min_depth, $max_depth, TRUE, $link_value_wanted);
							foreach ($new_asset_list as $k => $v) {
								$asset_list[$k] = Array( 0 => Array('type_code' => $v));
							}
							$assets_to_list = $this->_combineAssets($logic, $asset_list, $assets_to_list);
						}
					}

				break;
			}//end switch

			// if the min set to 0, show asset list with root nodes
			if ($min_depth === '0' && $max_depth !== '0') {
				$assets_to_list = $this->_combineRootNodes($assets_to_list, $root_asset_ids, $wanted_types);
			}

		}//end else subs_only

		if (empty($assets_to_list)) return Array();
		return $assets_to_list;

	}//end _getAssetList()


	/**
	* Sort the list of assets
	*
	* @param array	$assets_to_list	an array of assets to be sorted
	* @param array	$sort_info		sorting information as provided by getSortInfo()
	*
	* @return array
	* @access public
	* @see Listing_Engine::getSortInfo()
	*/
	function _getSortedAssetLists($assets_to_list, $sort_info)
	{
		return parent::sortAssetList($assets_to_list, $sort_info);

	}//end _getSortedAssetLists()


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
	* @param array		$new_assets	Set of new assets to add (or keep)
	* @param array|NULL	$existing	Set of existing assets to use as a base or
	*								NULL if first root node (see above)
	*
	* @return array
	* @access private
	*/
	function _combineAssets($logic='OR', $new_assets=Array(), $existing=NULL)
	{
		if (is_null($existing)) return $new_assets;

		if ($logic !== 'AND') {
			// OR logic: as it was before, just add 'em
			$existing += $new_assets;
		} else {
			// AND logic: Oh how we long for PHP 5.1 and array_intersect_key()
			// So we need to do this instead...
			foreach ($existing as $assetid => $data) {
				if (!isset($new_assets[$assetid])) {
					unset($existing[$assetid]);
				}
			}
		}

		return $existing;

	}//end _combineAssets()


	/**
	* Combines assets and root nodes together
	*
	* @param array	$assets		Set of assets
	* @param array	$root_nodes	Set of root nodes
	* @param array	$types		List of the wanted types
	*
	* @return array
	* @access private
	*/
	function _combineRootNodes($assets=Array(), $root_nodes=Array(), $types=Array())
	{
		foreach ($root_nodes as $root_node) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node);
			if (!is_null($asset)) {
				foreach ($types as $wanted_type => $include) {
					if ($asset instanceof $wanted_type) {
						$assets[$root_node] = Array(
												Array(
													'type_code'	=> $asset->type(),
													'sort_value'=> $asset->short_name,
													'first_letter'=> substr($asset->short_name, 0 , 1),
												),
											  );
					}//end if
				}//end foreach
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}//end if
		}//end foreach

		return $assets;

	}//end _combineRootNodes()


}//end class
?>
