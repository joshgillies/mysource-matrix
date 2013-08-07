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
* $Id: page_whats_new.inc,v 1.46.6.1 2013/08/07 00:29:18 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';

/**
* Page_Whats_New
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.46.6.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New extends Listing_Engine
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
		$group_by = $this->attr('group_by');
		$bind_vars = Array();

		// this is the list of all type codes that we are going to show
		// we pass this list to the getLinks and getChildren functions (below) to narrow
		// the query down a bit
		// if there are no types to list, we cant list anything
		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) {
			trigger_localised_error('CMS0010', E_USER_NOTICE);
			return Array();
		}

		// get the root asset whose sub-assets/children we are displaying
		// if there is no root asset, we will use ourselves and print our children
		$root_asset_ids = $this->getRootNodes();

		$sort_info = $this->getSortInfo();
		$group_by = $this->attr('group_by');

		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		$letter_sort_by = 'name';
		if ($group_by == 'letter') {
			if (!empty($sort_info) && ($sort_info['type'] == 'field') && ($sort_info['params']['field'] == 'short_name')) {
				$letter_sort_by = 'short_name';
			}
		}

		$children = Array();
		$base_bind_vars = Array();

		// this is the field in the database that we are using the specify that something is "new"
		$field = $this->attr('date_field');
		$ranges = $this->attr('date_ranges');

		$link_value_wanted = NULL;
		if ($this->attr('link_value_filter_enabled')) {
			// the data stored in the DB was encoded to protect against cross scripting so we use htmlentities
			$link_value_wanted = htmlentities($this->attr('link_value_filter'));
			replace_global_keywords($link_value_wanted);
		}

		// work out the where clause that is going to find "new" assets
		$where = '';
		require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';

		if (!empty($ranges) && isset($_REQUEST['asset_'.$this->id.'_date_range']) && isset($ranges[$_REQUEST['asset_'.$this->id.'_date_range']])) {
			$id = $_REQUEST['asset_'.$this->id.'_date_range'];
			$this->setAttrValue('num_per_page', 0); // If we're displaying a range, clear the pagination

			if ($ranges[$id]['range_type'] !== 'default') {
				switch ($ranges[$id]['range_type']) {
					case 'today':
						$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('00:00:00 today')));
					break;

					case 'week':
						// If it's a Monday, then doing "next Monday, 1 week ago"
						// will make it one week out.
						if ((int)date('w') === 1) {
							$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('00:00:00 today')));
						} else {
							$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('00:00:00 Monday -1 week')));
						}
					break;

					case 'month':
						$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime(date('Y-m').'-01 00:00:00')));
					break;

					case 'year':
						$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime(date('Y').'-01-01 00:00:00')));
					break;
				}

				$to = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('now')));
			} else {
				$from = new Asset_Attribute_DateTime();
				$from->value = $ranges[$id]['oldest_date'];

				$to = new Asset_Attribute_DateTime();
				$to->value = $ranges[$id]['newest_date'];
			}
		} else if ($this->attr('range_type') !== 'default') {
			switch ($this->attr('range_type')) {
				case 'today':
					$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('00:00:00 today')));
				break;

				case 'week':
					// If it's a Monday, then doing "next Monday, 1 week ago"
					// will make it one week out.
					if ((int)date('w') === 1) {
						$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('00:00:00 today')));
					} else {
						$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('00:00:00 Monday -1 week')));
					}
				break;

				case 'month':
					$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime(date('Y-m').'-01 00:00:00')));
				break;

				case 'year':
					$from = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime(date('Y').'-01-01 00:00:00')));
				break;
			}

			$to = new Asset_Attribute_DateTime(0, ts_iso8601(strtotime('now')));
		} else {
			$from = $this->getAttribute('date_range_from');
			$to   = $this->getAttribute('date_range_to');
		}

		$from = $from->getISO8601();
		$from_date = iso8601_date_component($from);
		if (!empty($from_date)) {
			// invalid FROM date?
			if (!preg_match_all('|^\d{4}-\d{2}-\d{2}$|', $from_date, $matches)) {
				trigger_localised_error('CMS0053', E_USER_WARNING, $from_date);
				return Array();
			}

			$from_time = iso8601_time_component($from);
			if (empty($from_time)) $from_time = '00:00:00';

			// invalid FROM time?
			if (!preg_match_all('|^\d{2}:\d{2}:\d{2}$|', $from_time, $matches)) {
				trigger_localised_error('CMS0054', E_USER_WARNING, $from_time);
				return Array();
			}
			$where .= ' AND a.'.$field.' >= '.db_extras_todate(MatrixDAL::getDbType(), ':wn_from_date', FALSE);
			$base_bind_vars['wn_from_date'] = $from_date.' '.$from_time;
		}

		$to = $to->getISO8601();
		$to_date = iso8601_date_component($to);
		if (!empty($to_date)) {
			// invalid TO date?
			if (!preg_match_all('|^\d{4}-\d{2}-\d{2}$|', $to_date, $matches)) {
				trigger_localised_error('CMS0055', E_USER_WARNING, $to_date);
				return Array();
			}

			$to_time = iso8601_time_component($to);
			if (empty($to_time)) $to_time = '23:59:59';

			// invalid TO time?
			if (!preg_match_all('|^\d{2}:\d{2}:\d{2}$|', $to_time, $matches)) {
				trigger_localised_error('CMS0056', E_USER_WARNING, $to_time);
				return Array();
			}
			$where .= ' AND a.'.$field.' <= '.db_extras_todate(MatrixDAL::getDbType(), ':wn_to_date', FALSE);
			$base_bind_vars['wn_to_date'] = $to_date.' '.$to_time;
		}

		// if there are no date conditions - we dont display anything
		if (empty($where)) {
			trigger_localised_error('CMS0011', E_USER_NOTICE);
			return Array();
		}

		if (!is_null($link_value_wanted)) {
			$link_value_wanted = Array(
									'link_value' 	=> Array($link_value_wanted),
									'equal' 		=> $this->attr('link_value_filter_logic'),
								 );
		}

		if ($this->attr('subs_only')) {
			$links = Array();

			foreach ($root_asset_ids as $root_asset_id) {
				$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset_id, $this->attr('link_types'), array_keys($wanted_types), FALSE, 'major', $link_value_wanted, NULL, NULL, $letter_sort_by, SQ_PERMISSION_READ);
				$query = $links_query['sql_array'];
				$bind_vars = array_merge($base_bind_vars, $links_query['bind_vars']);

				// Replace references to asset table with ones to the view
				$where = str_replace('a.'.$field, 'l.'.$field, $where);
				$query['where'] .= $where;

				// special case for A-Z listings because they require another field selected from the database
				if ($group_by == 'letter') {
					$query['select'] .= ', SUBSTR(l.'.$letter_sort_by.', 1, 1) AS first_letter';
				}

				$new_links = NULL;
				try {
					$_query = MatrixDAL::preparePdoQuery(implode(' ', $query));
					foreach($bind_vars as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($_query, $bind_var, $bind_value);
					}
					$new_links = MatrixDAL::executePdoAll($_query);
				} catch (Exception $e) {
					throw new Exception($e->getMessage());
				}

				$links = array_merge($links, $new_links);
			}

			switch ($group_by) {

				case 'letter' :
					foreach ($links as $link_info) {
						$children[$link_info['minorid']] = Array( 0 => Array(
																		'type_code'		=> $link_info['minor_type_code'],
																		'first_letter'	=> $link_info['first_letter'],
																		)
															);
					}
				break;

				case 'number' :
				case 'random' :
					foreach ($links as $link_info) {
						$children[$link_info['minorid']] = Array( 0 => Array('type_code' => $link_info['minor_type_code']));
					}
				break;
			}

		} else {
			$children = Array();

			foreach ($root_asset_ids as $root_asset_id) {
				$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);
				$child_query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), FALSE, NULL, $letter_sort_by, SQ_PERMISSION_READ, TRUE, TRUE, NULL, NULL, TRUE, $link_value_wanted);
				$sql_array = $child_query['sql_array'];
				$bind_vars = array_merge($base_bind_vars, $child_query['bind_vars']);

				$sql_array['where'] .= $where;

				// special case for A-Z listings because they require another field selected from the database
				if ($group_by == 'letter') {
					$sql_array['select'] .= ', SUBSTR(a.'.$letter_sort_by.', 1, 1) AS first_letter';
					$sql_array['union_select'] .= ', NULL AS first_letter';
				}

				$new_kids = NULL;
				try {
					$query = MatrixDAL::preparePdoQuery(implode(' ', $sql_array));

					foreach($bind_vars as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}

					$new_kids = MatrixDAL::executePdoGroupedAssoc($query);
				} catch (Exception $e) {
					throw new Exception($e->getMessage());
				}

				// can't use array_merge() because we need to save the key (assetid);
				foreach ($new_kids as $kid_key => $kid_value) {
					$children[$kid_key] = $kid_value;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
				unset($root_asset);
			}

			// only A-Z listings require the type_code to be in an array
			// all other formats require a simple assetid => type_code return value
			if ($group_by != 'letter') {
				foreach ($children as $child_id => $child_datas) {
					$child_data = $child_datas[0];
					$children[$child_id] = Array( 0 => Array('type_code' => $child_data['type_code']));
				}
			}

		}//end else subs_only

		if ($group_by == 'number') {
			$children = $this->sortAssetList($children, $sort_info);
		}

		return $children;

	}//end getAssetList()


	/**
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		$cache_key = parent::_getCacheKey();

		$ranges = $this->attr('date_ranges');
		if (!empty($ranges) && isset($_REQUEST['asset_'.$this->id.'_date_range']) && isset($ranges[$_REQUEST['asset_'.$this->id.'_date_range']])) {
			// add our date range ID to the unique cache key
			$cache_key .= ':'.$_REQUEST['asset_'.$this->id.'_date_range'];
		}

		return $cache_key;

	}//end _getCacheKey()


//--        KEYWORD DESCRIPTION        --//


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function _getContentsKeywords()
	{
		$keywords = parent::_getContentsKeywords();
		$keywords['date_range_list'] = translate('cms_whats_new_date_range_list');
		$keywords['date_range_list_linked'] = translate('cms_whats_new_date_range_list_linked');

		return $keywords;

	}//end _getContentsKeywords()


//--        KEYWORD REPLACEMENT        --//


	/**
	* Get Date Range List keyword replacement
	*
	* @return string
	* @access public
	*/
	function getDateRangeListKeywordReplacement()
	{
		$ranges = $this->attr('date_ranges');
		if (empty($ranges)) return;

		$prefix = 'asset_'.$this->id;

		foreach ($ranges as $id => $info) {
			$names[$id] = $info['name'];
		}

		$selected = (isset($_REQUEST[$prefix.'_date_range']) ? $_REQUEST[$prefix.'_date_range'] : '');
		$url = $_SERVER['PHP_SELF'];

		ob_start();
			combo_box($prefix.'_date_range', $names, FALSE, $selected);
			echo '&nbsp;';
			$extras = 'var select = document.getElementById("'.$prefix.'_date_range"); document.location.href="'.$url.'?'.$prefix.'_date_range=" + select.selectedIndex;';
			normal_button($prefix.'_date_range_submit', translate('go'), $extras);

			$replacement = ob_get_contents();
		ob_end_clean();

		return $replacement;

	}//end getDateRangeListKeywordReplacement()


	/**
	* Get Date Range List Linked keyword replacement
	*
	* @return string
	* @access public
	*/
	function getDateRangeListLinkedKeywordReplacement()
	{
		$ranges = $this->attr('date_ranges');
		if (empty($ranges)) return;

		$prefix = 'asset_'.$this->id;

		foreach ($ranges as $id => $info) {
			$names[$id] = $info['name'];
		}

		$selected = (isset($_REQUEST[$prefix.'_date_range']) ? $_REQUEST[$prefix.'_date_range'] : '');
		$url = $_SERVER['PHP_SELF'];

		ob_start();
		?>
			<div id="date_range_list">
				<ul>
				<?php
				foreach ($names as $id => $name) {
				?>
					<li<?php echo ($selected == $id) ? ' id="current"' : '';?>>
						<a href="<?php echo $url.'?'.$prefix.'_date_range='.$id; ?>"><?php echo $name; ?></a>
					</li>
				<?php
				}
				?>
				</ul>
			</div>
		<?php
		$replacement = ob_get_contents();
		ob_end_clean();

		return $replacement;

	}//end getDateRangeListLinkedKeywordReplacement()


}//end class

?>
