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
* $Id: page_whats_new.inc,v 1.3 2004/09/15 00:38:36 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_listing/page_asset_listing.inc';

/**
* Page_Whats_New
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New extends Page_Asset_Listing
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Whats_New($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page_Asset_Listing($assetid);

	}//end constructor
	
	
	/**
	* Get the list of asset that should be printed
	*
	* The return array is in the form Array(assetid => type_code) unless we are grouping by letter,
	* in which case the return array is
	* <pre>
	* Array(assetid => Array(
	*						'type_code' => type_code,
	*						'firt_letter' => first_letter,
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
		if (empty($wanted_types)) return Array();

		// get the root asset whose sub-assets/children we are displaying
		// if there is no root asset, we will use ourselves and print our children
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
		if (empty($root_link)) {
			$root_asset =& $this;
		} else {
			$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid'], $root_link['minor_type_code']);
			if (is_null($root_asset)) return Array();
		}

		// this is what we are going to sort the reults by (passed directly to the getLinks
		// and getChildren functions)
		// must be a column in the asset table (null = no sorting)
		$sort_by = $this->attr('sort_by');
		if (empty($sort_by)) $sort_by = null;


		// if we are showing an A-Z listing but the user has not selected a proper sort field,
		// we will use the 'name' attribute of the asset to work out the listing
		if ($group_by == 'letter' && $sort_by != 'name' && $sort_by != 'short_name') $sort_by = 'name';

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
			
			$from = new Asset_Attribute_DateTime();
			$from->value = $ranges[$id]['oldest_date'];
			
			$to = new Asset_Attribute_DateTime();
			$to->value = $ranges[$id]['newest_date'];
		} else {
			$from = $this->getAttribute('date_range_from');
			$to   = $this->getAttribute('date_range_to');
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
		if (empty($where)) return Array();

		if ($this->attr('subs_only')) {

			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($root_asset->id, SQ_SC_LINK_BACKEND_NAV, array_keys($wanted_types), false, 'major', null, null, null, $sort_by, SQ_PERMISSION_READ);
			$query['where'] .= $where;

			// special case for A-Z listings because they require another field selected from the database
			if ($group_by == 'letter') {
				$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';
			}

			$links = $db->getAll(implode(' ', $query));
			assert_valid_db_result($links);

			switch ($group_by) {
				case 'number' :
					foreach ($links as $link_info) $children[$link_info['minorid']] = $link_info['minor_type_code'];
					if ($this->attr('reverse_sort')) $children = array_reverse($children, true);
				break;

				case 'letter' :
					foreach ($links as $link_info) $children[$link_info['minorid']] = Array('type_code' => $link_info['minor_type_code'], 'first_letter' => $link_info['first_letter']);
				break;
				
				case 'random' :
					foreach ($links as $link_info) $children[$link_info['minorid']] = $link_info['minor_type_code'];
				break;
			}

		} else {

			if ($group_by == 'random') $sort_by = null;

			$query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, array_keys($wanted_types), false, $sort_by, SQ_PERMISSION_READ);
			$query['where'] .= $where;

			// special case for A-Z listings because they require another field selected from the database
			if ($group_by == 'letter') {
				$query['select'] .= ', SUBSTRING(a.'.$sort_by.', 1, 1) AS first_letter';
			}

			$children = $db->getAssoc(implode(' ', $query));
			assert_valid_db_result($children);

			// normal number grouping may require a reverse sort
			if ($group_by == 'number') {
				if ($this->attr('reverse_sort')) $children = array_reverse($children, true);
			}

			// only A-Z listings require the type_code to be in an array
			// all other formats require a simple assetid => type_code return value
			if ($group_by != 'letter' && $sort_by != null) {
				for (reset($children); null !== ($child_id = key($children)); next($children)) {
					$children[$child_id] = $children[$child_id]['type_code'];
				}
			}

		}//end if subs_only

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
		
		return $children;

	}//end getAssetList()


	/**
	* Returns keyword replacements to be replaced in the body of the format bodycopy
	*
	* @return Array()
	* @access public
	*/
	function getKeywordReplacements()
	{
		$replacements = Array();

		// get a list of keywords that are in the format bodycopy

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (empty($bodycopy_link)) return Array();

		$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($format_bodycopy)) return Array();

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = retrieve_keywords_replacements($format_bodycopy->getRawBodycopyContent());

		$ranges = $this->attr('date_ranges');
		$prefix = 'asset_'.$this->id;

		foreach ($ranges as $id => $info) $names[$id] = $info['name'];
		$selected = (isset($_REQUEST[$prefix.'_date_range']) ? $_REQUEST[$prefix.'_date_range'] : '');
		$url = $this->getURL();

		if (in_array('date_range_list', $keywords)) {
			ob_start();
				combo_box($prefix.'_date_range', $names, false, $selected);
				echo '&nbsp;';
				$extras = 'var select = document.getElementById("'.$prefix.'_date_range"); document.location.href="'.$url.'?'.$prefix.'_date_range=" + select.selectedIndex;';
				normal_button($prefix.'_date_range_submit', 'Go', $extras);

				$replacements['date_range_list'] = ob_get_contents();
			ob_end_clean();
		}

		if (in_array('date_range_list_linked', $keywords)) {
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
				$replacements['date_range_list_linked'] = ob_get_contents();

			ob_end_clean();
		}

		return $replacements;

	}//end getKeywordReplacements()


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => Array (
	*             'name'    => '(name of the keyword)',
	*             'default' => '(default value)',
	*             'type'    => 'block' | 'inline'
	*         )
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getContentsKeywords()
	{
		$keywords = parent::getContentKeywords();
		$keywords['date_range_list'] = Array(
					'name'    => 'Date Range List',
					'default' => 'Date Range List', 
					'type'    => 'inline'
				);
		$keywords['date_range_list_linked'] = Array(
					'name'    => 'Date Range List Linked',
					'default' => 'Date Range List Linked', 
					'type'    => 'inline'
				);
		return $keywords;

	}//end getContentsKeywords()


}//end class

?>
