<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_whats_new.inc,v 1.24 2005/10/27 00:46:32 lwright Exp $
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
* @version $Revision: 1.24 $
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
	function Page_Whats_New($assetid=0)
	{
		$this->Listing_Engine($assetid);

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
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
		if (empty($root_link)) {
			$root_asset =& $this;
		} else {
			$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid'], $root_link['minor_type_code']);
			if (is_null($root_asset)) return Array();
		}

		$sort_info = $this->getSortInfo();

		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		$letter_sort_by = 'name';
		if ($group_by == 'letter'){
			if (!empty($sort_info) && ($sort_info['type'] == 'field') && ($sort_info['params']['field'] == 'short_name')) {
				$letter_sort_by = 'short_name';
			}
		}

		$children = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// this is the field in the database that we are using the specify that something is "new"
		$field = $this->attr('date_field');
		$ranges = $this->attr('date_ranges');

		// work out the where clause that is going to find "new" assets
		$where = '';

		if (!empty($ranges) && isset($_REQUEST['asset_'.$this->id.'_date_range']) && isset($ranges[$_REQUEST['asset_'.$this->id.'_date_range']])) {
			require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
			$id = $_REQUEST['asset_'.$this->id.'_date_range'];
			$this->setAttrValue('num_per_page', 0); // If we're displaying a range, clear the pagination

			$from =& new Asset_Attribute_DateTime();
			$from->value = $ranges[$id]['oldest_date'];

			$to =& new Asset_Attribute_DateTime();
			$to->value = $ranges[$id]['newest_date'];
		} else {
			$from = &$this->getAttribute('date_range_from');
			$to   = &$this->getAttribute('date_range_to');
		}

		$from = $from->getISO8601();
		$from_date = iso8601_date_component($from);
		if (!empty($from_date)) {
			$from_time = iso8601_time_component($from);
			if (empty($from_time)) $from_time = '00:00:00';
			$where .= ' AND a.'.$field.' >= '.$db->quote($from_date.' '.$from_time);
		}

		$to = $to->getISO8601();
		$to_date = iso8601_date_component($to);
		if (!empty($to_date)) {
			$to_time = iso8601_time_component($to);
			if (empty($to_time)) $to_time = '00:00:00';
			$where .= ' AND a.'.$field.' <= '.$db->quote($to_date.' '.$to_time);
		}

		// if there are no date conditions - we dont display anything
		if (empty($where)) {
			trigger_localised_error('CMS0011', E_USER_NOTICE);
			return Array();
		}

		if ($this->attr('subs_only')) {

			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset->id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, 'major', null, null, null, $letter_sort_by, SQ_PERMISSION_READ);
			$query['where'] .= $where;

			// special case for A-Z listings because they require another field selected from the database
			if ($group_by == 'letter') {
				$query['select'] .= ', SUBSTR(a.'.$letter_sort_by.', 1, 1) AS first_letter';
			}

			$links = $db->getAll(implode(' ', $query));
			assert_valid_db_result($links);

			switch ($group_by) {

				case 'letter' :
					foreach ($links as $link_info) {
						$children[$link_info['minorid']] = Array(
															'type_code'		=> $link_info['minor_type_code'],
															'first_letter'	=> $link_info['first_letter'],
														   );
					}
				break;

				case 'number' :
				case 'random' :
					foreach ($links as $link_info) {
						$children[$link_info['minorid']] = $link_info['minor_type_code'];
					}
				break;
			}

		} else {

			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), false, $letter_sort_by, SQ_PERMISSION_READ);
			$query['where'] .= $where;

			// special case for A-Z listings because they require another field selected from the database
			if ($group_by == 'letter') {
				$query['select'] .= ', SUBSTR(a.'.$letter_sort_by.', 1, 1) AS first_letter';
			}

			$children = $db->getAssoc(implode(' ', $query));
			assert_valid_db_result($children);

			// only A-Z listings require the type_code to be in an array
			// all other formats require a simple assetid => type_code return value
			if ($group_by != 'letter') {
				foreach ($children as $child_id => $child_data) {
					$children[$child_id] = $child_data['type_code'];
				}
			}

		}//end if subs_only

		if ($group_by == 'number') {
			$children = $this->sortAssetList($children, $sort_info);
		}

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);

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
	* @access public
	* @return array(string => string)
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
	* @access public
	* @return string
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
			combo_box($prefix.'_date_range', $names, false, $selected);
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
	* @access public
	* @return string
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
