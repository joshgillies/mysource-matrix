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
* $Id: page_whats_new.inc,v 1.2 2004/08/30 04:47:55 gsherwood Exp $
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


		// work out the where clause that is going to find "new" assets
		$where = '';

		$from = $this->getAttribute('date_range_from');
		$from = $from->getISO8601();
		$from_date = iso8601_date_component($from);
		if (!empty($from_date)) {
			$from_time = iso8601_time_component($from);
			if (empty($from_time)) $from_time = '00:00:00';
			$where .= ' AND a.'.$field.' >= '.$db->quote($from_date.' '.$from_time);
		}

		$to = $this->getAttribute('date_range_to');
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

	
}//end class

?>
