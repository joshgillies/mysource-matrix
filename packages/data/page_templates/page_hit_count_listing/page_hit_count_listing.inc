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
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';

/**
* Page_Asset_Listing
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @author  Christophe Olivar <colivar@squiz.net>
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Hit_Count_Listing extends Listing_Engine
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
		$assets_to_list = Array();

		$min_hitcount = $this->attr('min_hitcount');
		$has_min_hitcount = $min_hitcount >= 0 ? TRUE : FALSE;
		$max_hitcount = $this->attr('max_hitcount');
		$has_max_hitcount = $max_hitcount >= 0 ? TRUE : FALSE;;

		// get the root nodes
		$root_asset_ids = $this->getRootNodes();
		if(empty($root_asset_ids)) return $assets_to_list;
		$group_by = $this->attr('group_by');
		$wanted_types = $this->attr('types');
		$sort_info = $this->getSortInfo();

		$letter_sort_by = NULL;
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

		// check the sorting order
		$sql_sort = ' desc ';
		$sort_by_hit_count = ((!empty($sort_info)) && isset($sort_info['params']) && isset($sort_info['params']['field']) && ($sort_info['params']['field'] == 'hitcount'));
		if ($sort_by_hit_count) {
			if (!$this->isDescending()) $sql_sort = ' asc ';
		}

		$show_zero_hitcount = $this->attr('display_all_assets');
		// we are overwriting $show_zero_hitcount if the minimum specified was zero
		if ($min_hitcount == 0) $show_zero_hitcount = TRUE;

		$side_of_link = ($this->attr('direction') == 'down') ? 'minor' : 'major';

		// control the depth of the search
		$min_depth = ($this->attr('min_depth') === '') ? NULL : $this->attr('min_depth');
		$max_depth = ($this->attr('max_depth') === '') ? NULL : $this->attr('max_depth');
		// if we display subs only we limit the min and max depth to 1
		if ($this->attr('subs_only')) {
			$min_depth = 1;
			$max_depth = 1;
		}
		// will contain all the link info
		$links = Array();

		// build the full query that get every children/parents of the root nodes with an hit count value
		$full_query      = '';
		$full_bind_vars  = Array();

		foreach ($root_asset_ids as $index => $root_asset_id) {
			$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);
			$no_hit_query = '';

			// if the direction is down we are using the table with the alias a for the asset
			$table_alias = 'a';
			if ($this->attr('direction') == 'down') {
				// if we are listing children
				$bind_prefix = 'gc_'.$root_asset_id.'_';
				$ret_val   = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, NULL, NULL, NULL, TRUE, $min_depth, $max_depth, FALSE, NULL, $bind_prefix);
				$query     = $ret_val['sql_array'];
				$bind_vars = $ret_val['bind_vars'];

				// if we need to show assets with not hitcount value
				// get the $query and add in the where clause not in (select assetid from sq_hit_count)
				if ($show_zero_hitcount && ((!$has_min_hitcount) || ($has_min_hitcount && ($min_hitcount == 0))) ) {
					$zero_bind_prefix = 'gc_z_'.$root_asset_id.'_';

					$no_hit_query = $query;

					$zero_bind_vars = Array();
					foreach (array_keys($no_hit_query) as $key) {
						$no_hit_query[$key] = str_replace(':'.$bind_prefix, ':'.$zero_bind_prefix, $no_hit_query[$key]);
					}

					$bind_keys = array_keys($bind_vars);
					foreach ($bind_keys as $key) {
						$zeroed_key = str_replace($bind_prefix, $zero_bind_prefix, $key);
						$bind_vars[$zeroed_key] = $bind_vars[$key];
					}

					$no_hit_query['select'] .= ', 0 as hitcount ';
					$no_hit_query['where'] .= ' and a.assetid not in (select assetid from sq_hit_count) ';
					$no_hit_query['union_select'] .= ', 0 as hitcount ';
					$no_hit_query['union_where'] .= ' and  sl.minorid not in (select assetid from sq_hit_count) ';
				}
				$query['select'] .= ', hc.hitcount';
				$query['from'] .= ' INNER JOIN sq_hit_count hc on hc.assetid = '.$table_alias.'.assetid ';
				$query['union_select'] .= ', hc.hitcount';
				$query['union_from'] .= ' INNER JOIN sq_hit_count hc on hc.assetid = sl.minorid ';

			} else {
				// if we are listing parents

				// if direction id down we are using the table with the alias ll for the asset
				$table_alias = 'll';
				$bind_prefix = 'gp_'.$root_asset_id.'_';
				$ret_val   = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($root_asset_id, array_keys($wanted_types), FALSE, NULL, NULL, NULL, $min_depth, $max_depth, NULL, $bind_prefix);
				$query     = $ret_val['sql_array'];
				$bind_vars = $ret_val['bind_vars'];

				// if we need to show assets with not hitcount value
				// get the $query and add in the where clause not in (select assetid from sq_hit_count)
				if ($show_zero_hitcount && ((!$has_min_hitcount) || ($has_min_hitcount && ($min_hitcount == 0))) ) {
					$zero_bind_prefix = 'gp_z_'.$root_asset_id.'_';

					// we need to replace the bind prefix with another one so we don't duplicate
					// them in the full query.
					$no_hit_query = $query;
					$zero_bind_vars = Array();
					foreach (array_keys($no_hit_query) as $key) {
						$no_hit_query[$key] = str_replace(':'.$bind_prefix, ':'.$zero_bind_prefix, $no_hit_query[$key]);
					}

					$bind_keys = array_keys($bind_vars);
					foreach ($bind_keys as $key) {
						$zeroed_key = str_replace($bind_prefix, $zero_bind_prefix, $key);
						$bind_vars[$zeroed_key] = $bind_vars[$key];
					}

					$no_hit_query['select'] .= ', 0 as hitcount ';
					$no_hit_query['where'] .= ' where ll.majorid not in (select assetid from sq_hit_count)  ';
					$no_hit_query['union_select'] .= ', 0 as hitcount ';
					$no_hit_query['union_where'] .= ' and sl.majorid not in (select assetid from sq_hit_count) ';
					$no_hit_query['order_by'] = ') ';
				}

				// the following SQL query parts are added to the full query so it get only assets that have a hit count value
				$query['select'] .= ', hc.hitcount';
				$query['where'] = ' inner join sq_hit_count hc on hc.assetid = ll.majorid ';
				$query['union_select'] .= ', hc.hitcount';
				$query['union_from'] .= ' inner join sq_hit_count hc on hc.assetid = sl.majorid ';
				$query['order_by'] = ') ';
			}

			if ($has_min_hitcount) {
				$query['where'] .= ' and hc.hitcount >= :hc_'.$root_asset_id.'_min ';
				$query['union_where'] .= ' and hc.hitcount >= :hc_'.$root_asset_id.'_min_union ';

				$bind_vars['hc_'.$root_asset_id.'_min']       = $min_hitcount;
				$bind_vars['hc_'.$root_asset_id.'_min_union'] = $min_hitcount;
			}

			if ($has_max_hitcount) {
				$query['where'] .= ' and hc.hitcount <= :hc_'.$root_asset_id.'_max ';
				$query['union_where'] .= ' and hc.hitcount <= :hc_'.$root_asset_id.'_max_union ';

				$bind_vars['hc_'.$root_asset_id.'_max']       = $max_hitcount;
				$bind_vars['hc_'.$root_asset_id.'_max_union'] = $max_hitcount;
			}

			// if we are sorting A-Z
			if ($group_by == 'letter') {
				$query['select'] .= ', SUBSTR('.$table_alias.'.'.$letter_sort_by.', 1, 1) AS first_letter';
				$query['union_select'] .= ', null AS first_letter';
				if ($show_zero_hitcount && ((!$has_min_hitcount) || ($has_min_hitcount && ($min_hitcount == 0))) ) {
					$no_hit_query['select'] .= ', SUBSTR('.$table_alias.'.'.$letter_sort_by.', 1, 1) AS first_letter';
					$no_hit_query['union_select'] .= ', null AS first_letter';
				}
			}

			if (empty($query)) return Array();

			$full_query .= implode(' ', $query);

			// if we need to add zero hitcount asset
			if ($show_zero_hitcount && ((!$has_min_hitcount) || ($has_min_hitcount && ($min_hitcount == 0))) ) {
				$full_query .= 'UNION ALL '.implode(' ', $no_hit_query);
			}

			// if we are not the last index of $root_asset_ids we are doing an union with next query
			if ($index < (count($root_asset_ids) - 1)) {
				$full_query .= ' UNION ALL ';
			}

			// Add bind variables to the overall tally
			$full_bind_vars += $bind_vars;

		}//end foreach

		// sort them by hit count
		$full_query .= ' ORDER BY hitcount '.$sql_sort;

		try {
			$pdo_query = MatrixDAL::preparePdoQuery($full_query);
			foreach($full_bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($pdo_query, $bind_var, $bind_value);
			}
			$links = MatrixDAL::executePdoAll($pdo_query);
		} catch (Exception $e) {
			throw new Exception('Unable to get hit counts due to database error: '.$e->getMessage());
		}

		foreach ($links as $link_info) {
			if ($group_by == 'letter') {
				$assets_to_list[$link_info[$side_of_link.'id']] = Array( Array(
																	'type_code'		=> $link_info['type_code'],
																	'first_letter'	=> $link_info['first_letter'],
																  ));
			} else {
				$assets_to_list[$link_info[$side_of_link.'id']] = Array( Array('type_code' => $link_info['type_code']));
			}
		}

		// we add the following code to fix a problem where the assets are sorted when  no sorting is selected in standard and group by listing mode
		if ($group_by == 'number') {
			$assets_to_list = $this->sortAssetList($assets_to_list, $sort_info);
		}

		// if the min set to 0, show asset list with root nodes
		if ($min_depth === '0') {
			$assets_to_list = ($max_depth === '0') ? Array() : $assets_to_list;
			$assets_to_list = $this->_combineRootNodes($assets_to_list, $root_asset_ids, $wanted_types);
		}

		return $assets_to_list;

	}//end getAssetList()


	/**
	* Return an array with the keyword asset_hitcount
	* This will be shown in the type format of the asset Hit Count Asset
	*
	* @return array
	* @access private
	*/
	function _getGenericSingleAssetFormatKeywords()
	{

		$keywords = parent::_getGenericSingleAssetFormatKeywords();
		$keywords['asset_hitcount'] = 'Hit count value of the asset';
		return $keywords;

	}//end _getGenericSingleAssetFormatKeywords()


	/**
	* Print a single asset from the list
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param int	$list_position	the position of this asset in the list
	*
	* @return void
	* @access private
	* @see printAssetList()
	*/
	function _printAsset($assetid, $list_position)
	{
		ob_start();
			parent::_printAsset($assetid, $list_position);
			$element_contents = ob_get_contents();
		ob_end_clean();

		// get the hit count value
		try {
			$bind_vars = Array('assetid' => $assetid);
			$_tmp_hitcount = MatrixDAL::executeAll('data_package', 'getHitCount', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get hit count value due to database error: '.$e->getMessage());
		}

		if (empty($_tmp_hitcount)) {
			$hitcount = 0;
		} else {
			$hitcount = $_tmp_hitcount[0]['hitcount'];
		}

		$element_replacements = Array(
									'asset_hitcount'	=> $hitcount,
								);
		$element_contents = replace_keywords($element_contents, $element_replacements);

		echo $element_contents;

	}//end _printAsset()


	/**
	* Return an array with the extended keyword asset_hitcount
	*
	* @param string $assetid    Asset ID of the asset being listed
	*
	* @return array
	* @access private
	*/
	function getExtendedAssetKeywordReplacements($assetid='')
	{
		return Array('asset_hitcount' => '%asset_hitcount%');

	}//end getExtendedAssetKeywordReplacements()


	/**
	* Sort the list of assets
	*
	* @param array	$asset_list	an array of assets to be sorted
	* @param array	$sort_info	sorting information as provided by getSortInfo()
	*
	* @return array
	* @access public
	*/
	function sortAssetList($asset_list, $sort_info)
	{
		$sort_by_hit_count = (!empty($sort_info) && isset($sort_info['params']) && isset($sort_info['params']['field']) && ($sort_info['params']['field'] == 'hitcount'));

		if (!$sort_by_hit_count) {
			return parent::sortAssetList($asset_list, $sort_info);
		}

		return $asset_list;

	}//end sortAssetList()


	/**
	* Remove unwanted assets from the todo list
	*
	* @param array	&$todo	an array of assets to list in the same format as the return
	*						value of getAssetList()
	*
	* @return void
	* @access protected
	*/
	function filterAssetList(&$todo)
	{
		$sort_info = $this->getSortInfo();
		$sort_by_hit_count = (!empty($sort_info) && isset($sort_info['params']) && isset($sort_info['params']['field']) && ($sort_info['params']['field'] == 'hitcount'));

		if (!$sort_by_hit_count) {
			parent::filterAssetList($todo);
		} else {

			// filter the asset types
			if ($this->attr('group_by') == 'letter') {
				// if we sort by letter
				$filtered_todo = Array();
				for (reset($todo); NULL !== ($todo_id = key($todo)); next($todo)) {
					$filtered_todo[$todo_id] = $todo[$todo_id]['type_code'];
				}

				$this->filterAssetTypes($filtered_todo);
				foreach (array_keys($todo) as $todo_id) {
					if (!isset($filtered_todo[$todo_id])) {
						unset($todo[$todo_id]);
					}
				}
				unset($filtered_todo);
				$todo = $this->sortAssetList($todo, $this->getSortInfo());
			} else {
				// if we are not sorting by letter
				$this->filterAssetTypes($todo);
			}
		}

	}//end filterAssetList()


}//end class
?>
