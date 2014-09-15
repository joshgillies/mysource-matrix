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
* $Id: page_tag_listing.inc,v 1.4 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing.inc';

/**
* Page_Tag_Listing
*
* list all the tags applied to an asset or an asset and all its children.
* It can also filter the tag by thesaurus. This is often used to create tag clouds.
*
* For performance reasons the features of the listing have been simplified: 1 root node only, no filtering on asset types,
*  no link value filtering, no excluded root nodes and assets, no "up the tree" searches, no custom List Format.
*
* A few additional back-end fields have been introduced:
* * Filtering the tags by a thesaurus (this is optional, if not defined all tags are relevant)
* * First Page Only: this indicates that the listing will not use the pagination. This is an optimisation field only.
* * Maximum Tag Size: Determine the maximum value for the %tag_size% keyword in the Term Type Format.
*	This keyword is typically used in cunjunction with CSS to create tag coulds.
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Tag_Listing extends Page_Asset_Listing
{

	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset - Force the value of the "type" attribute to thesaurus term
	* @see asset::create()
	*
	* @return mixed int|boolean
	* @access public
	*/
	public function create(Array &$link)
	{
		$this->setAttrValue('types', Array('thesaurus_term' => '1'));
		return parent::create($link);

	}//end constructor


	/**
	* Get the list of asset that should be printed
	*
	* Only the "live" terms applied to children with effective read access for the current user will be listed.
	*
	* By default the result is sorted by the number of assets a term has been applied to.
	*
	* @return array
	* @access protected
	*/
	public function getAssetList()
	{
		$this->_tmp['tag_freq'] = Array();

		$assets_to_list = Array();

		// get all the children
		$min_depth = $this->attr('min_depth');
		if ($min_depth == '') $min_depth = NULL;
		$max_depth = $this->attr('max_depth');
		if ($max_depth == '') $max_depth = NULL;

		$query_links = '';
		$bind_vars = Array();

		$root_asset_ids = $this->getRootNodes();

		foreach ($root_asset_ids as $root_asset_id) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_asset_id);

			// must have read and effective access on the children
			$query_links = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($asset, '', FALSE, NULL, NULL, SQ_PERMISSION_READ, TRUE, TRUE, $min_depth, $max_depth, TRUE, NULL);
			// ok, confession time: this is not nice, we cover up a bug in generateGetChildrenQuery() which adds a random AND at the end of the weher clause
			// need to be fixed properly later
			$query_links['sql_array']['where'] = preg_replace('/AND\s*$/', '', $query_links['sql_array']['where']);
			$rows = array();

			$bind_vars = $query_links['bind_vars'];

			$query_links['sql_array']['select'] = '(SELECT DISTINCT l.minorid ';

			// remove shadow children from this search as the query would be too complex and slower
			$query_links['sql_array']['union_select'] = '';
			$query_links['sql_array']['union_from'] = '';
			$query_links['sql_array']['union_where'] = '';

			if (isset($bind_vars['gc_union_majorid'])) {
				unset($bind_vars['gc_union_majorid']);
			}

			$query_links = implode(' ', $query_links['sql_array']);

			// no support for multiple root nodes; only the first one is processed
			break;
		}

		// thesaurus filtering
		$thesaurus_filters = $this->_getThesaurusFilters();

		$thesaurus_filter_clause = '';
		if (!empty($thesaurus_filters)) {
			// assume there is only filtering thesaurus
			$thesaurus_filter_clause = ' AND (te.thesid = :thesaurusid) ';
			$bind_vars['thesaurusid'] = $thesaurus_filters[0];
		}

		// include root node is required
		$where_root_node = '';
		if (!$min_depth) {
			$where_root_node = ' OR (sl2.majorid = :root_nodeid) ';
			$bind_vars['root_nodeid'] = $root_asset_ids[0];
		}

		// build and execute the query
		$query = '
					SELECT
						sl2.minorid,
						count(sl2.minorid) as score
					FROM
						sq_shdw_ast_lnk sl2
						JOIN sq_thes_term te
							ON (te.thesid || \':\' || te.termid)= sl2.minorid
					WHERE
						(sl2.majorid IN ('.$query_links.')
						'.$where_root_node.' )
						'.$thesaurus_filter_clause.'
					GROUP BY
						sl2.minorid
					ORDER BY
						score DESC
				';

		try  {
			$query = MatrixDAL::preparePdoQuery($query);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$rows = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get children for asset: '.join(', ', $root_asset_ids).' due to database error: '.$e->getMessage());
		}
		// scoop the result
		// TODO: limit the select instead, that would be faster.
		$query_limit = $this->_getQueryLimit();
		if ($query_limit != -1) {
			$rows = array_slice($rows, 0, $query_limit);
		}

		// store the scores
		foreach ($rows as $row) {
			// save the weight of all the tagids
			$this->_tmp['tag_freq'][$row['minorid']] = $row['score'];
			$assets_to_list[$row['minorid']] = Array(Array('type_code' => 'thesaurus_term'));
		}

		return $assets_to_list;

	}//end getAssetList()


	/**
	* Returns the maximum number of items in the result set.
	* -1 means no limit.
	*
	* @return int
	* @access protected
	*/
	protected function _getQueryLimit()
	{
		$ret = -1;

		if ($this->attr('first_page_only')) {
			$si = $this->getSortInfo();
			if (empty($si['type'])) {
				$ret = $this->attr('num_per_page');
				if ($ret == 0) $ret = -1;
			}
		}

		return $ret;

	}//end _getQueryLimit()


	/**
	* Return an array of assetid for the thesaurus that we want to filter by
	*
	* @return array
	* @access private
	*/
	protected function _getThesaurusFilters()
	{
		$ret = Array();

		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'thesaurus', FALSE);

		foreach ($links as $link) {
			$ret[] = $link['minorid'];
		}

		return $ret;

	}//end _getThesaurusFilters()


	/**
	* Print a shadow asset from the list
	* Support new keywords:
	* %tag_count%: the number of assets tagged with this term in the result set
	* %tag_size% : a number between 0 and attr('tag_max_size') representing the relative importance of the tag
	*				compared to the most popular tag. The highest number is the most popular.
	*
	* @param string	$assetid		the ID of the asset we are printing
	* @param array	$bc_keywords	the tyope format body copy keywords that are to be replaced
	*
	* @return array
	* @access public
	* @see printAssetList()
	*/
	public function _getShadowAssetKeywordReplacements($assetid, $bc_keywords)
	{
		$keywords = Array();
		$shadow_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		foreach ($bc_keywords as $key => $keyword) {
			if ($keyword == 'tag_count') {
				$keywords[$keyword] = array_get_index($this->_tmp['tag_freq'], $assetid, 1);
			} else if ($keyword == 'tag_size') {
				$keywords[$keyword] = ceil(array_get_index($this->_tmp['tag_freq'], $assetid, 1) / $this->_getTagMaxCount() * $this->Attr('tag_max_size'));
			} else {
				$keywords[$keyword] = $shadow_asset->getKeywordReplacement($keyword);
			}
		}
		return $keywords;

	}//end _getShadowAssetKeywordReplacements()


	/**
	* Returns the maximum number of assets in the result set that have been tagged with the same term
	*
	* @return int
	* @access private
	*/
	protected function _getTagMaxCount()
	{
		$ret = 1;

		if (count($this->_tmp['tag_freq'])) {
			$ret = array_values($this->_tmp['tag_freq']);
			$ret = $ret[0];
		}

		return $ret;

	}//end _getTagMaxCount()


	/**
	* Get additional keywords to be used in the single-asset format
	*
	* Provides keywords to be used alongside those provided by assets themselves.
	* These keywords are understood only by this asset
	*
	* @return array
	* @access public
	*/
	public function _getAdditionalSingleAssetFormatKeywords()
	{
		$ret = parent::_getAdditionalSingleAssetFormatKeywords();

		$ret['tag_count'] = translate('page_tag_listing_keyword_tag_count');
		$ret['tag_size'] = translate('page_tag_listing_keyword_tag_size');

		return $ret;

	}//end _getAdditionalSingleAssetFormatKeywords()


	/**
	* Work out the unique cache key to represent the current page
	*
	* @return string
	* @access private
	*/
	function _getCacheKey()
	{
		$cache_key = parent::_getCacheKey();

		// add the thesaurusid we use for filtering
		$cache_key .= ';THES='.join(',', $this->_getThesaurusFilters());
		// add the maximum tag size
		$cache_key .= ';MTS='.$this->Attr('tag_max_size');
		// add the "first page only"
		$cache_key .= ';FPO='.($this->attr('first_page_only') ? '1' : '0');

		return $cache_key;

	}//end _getCacheKey()


}//end class

?>
