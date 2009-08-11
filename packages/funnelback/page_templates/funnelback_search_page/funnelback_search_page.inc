<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_search_page.inc,v 1.1.1.1 2009/08/11 01:51:56 bpearson Exp $
*
*/

check_ssv_licence();
require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once 'HTTP/Client.php';

/**
* Funnelback_Search_Page
*
* Page template for funnelback searching
*
* @see Page
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Page extends Listing_Engine
{

	/**
	* An array of bodycopies and their names that are attached underneath this page asset
	* @var array
	*/
	var $bodycopies = Array(
						'initial'		=> 'Initial Search Page Layout',
						'results'		=> 'Results Page Layout',
						'no_results'	=> 'No Results Page Layout',
					  );

	/**
	* An array of default parameters
	*
	* @var array
	*/
	var $defaults = Array ('query_width' => 25);

	/**
	* An array of keywords replacements that can be used in this page
	*
	* @var array
	*/
	var $keywords = Array(
						'initial'		=> Array(
											'current_query',
											'search_query',
											'search_scope',
											'search_button',
											'submit_button',
											'results_per_page',
										   ),
						'results'		=> Array(
											'page_list_X',
											'result_list',
											'spell_suggestion',
											'fluster_category_type',
											'fluster_category_topic',
											'fluster_category_site',
											'fluster_current_term',
											'fluster_navigation',
											'result_featured',
											'result_count',
											'result_count_fully',
											'result_count_partially',
											'result_count_pages',
											'total_result_pages',
											'result_per_page',
											'current_result_page',
											'current_result_page_start',
											'current_result_page_end',
											'result_page_index',
											'previous_result_page_link',
											'next_result_page_link',
											'previous_result_page_href',
											'next_result_page_href',
											'initial_layout',
										   ),
						'no_results'	=> Array(
											'initial_layout',
										   ),
						'type_results'	=> Array(
											'result_title',
											'result_summary',
											'result_url',
											'result_cache_url',
											'result_size',
											'result_type',
										   ),
						'featured'		=> Array(
											'featured_trigger',
											'featured_url',
											'featured_title',
											'featured_summary',
										   ),
						'fluster'		=> Array(
											'fluster_category_name',
											'fluster_current_term',
											'fluster_navigation',
											'fluster_list',
										   ),
						'cluster'		=> Array(
											'cluster_name',
											'cluster_url',
											'cluster_count',
										   ),
						'navigation'	=> Array(
											'navigation_name',
											'navigation_url',
										   ),
						'spelling'		=> Array(
											'spell_url',
											'spell_text',
										   ),
					);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*	'asset'		   => [ref major asset to create link under],
	*	'link_type'    => SQ_LINK_?,
	*	'value'		   => [link value],
	*	'sort_order'   => [link sort order],
	*	'is_dependant' => [0|1],
	*	'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(Array &$link)
	{
		// Overriding the listing engine _createAdditional() and using Page instead
		if (!Page::_createAdditional($link)) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
						'type_formats'		=> 'folder',
						'fluster_formats'	=> 'folder',
						'spell_formats'		=> 'folder',
					 );

		$type_formats = NULL;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;

			switch ($name) {
				case 'fluster_formats':
					$format_name = 'fluster_format';
				break;
				case 'spell_formats':
					$format_name = 'spell_format';
				break;
				default:
					$format_name = 'default_format';
			}//end switch

			$bc_asset = new Bodycopy();
			$bc_copy_link = Array('asset' => &$asset, 'value' => $format_name, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
			$bc_asset->setAttrValue('name', translate($format_name));
			$bc_args = Array('content' => $this->_getDefaultBodycopyContent($format_name));
			if (!$bc_asset->create($bc_copy_link, $bc_args)) return FALSE;

			// Add in the extra bodycopies
			if ($name == 'fluster_formats') {
				foreach (Array('cluster_format', 'navigation_format') as $extra_format) {
					$bc2_asset = new Bodycopy();
					$bc2_copy_link = Array('asset' => &$asset, 'value' => $extra_format, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
					$bc2_asset->setAttrValue('name', translate($extra_format));
					$bc2_args = Array('content' => $this->_getDefaultBodycopyContent($extra_format));
					if (!$bc2_asset->create($bc2_copy_link, $bc2_args)) return FALSE;
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bc2_asset);
					unset($bc2_asset);
				}//end foreach
			} else if ($name == 'type_formats') {
				$extra_format = 'featured_format';
				$bc2_asset = new Bodycopy();
				$bc2_copy_link = Array('asset' => &$asset, 'value' => $extra_format, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
				$bc2_asset->setAttrValue('name', translate($extra_format));
				$bc2_args = Array('content' => $this->_getDefaultBodycopyContent($extra_format));
				if (!$bc2_asset->create($bc2_copy_link, $bc2_args)) return FALSE;
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bc2_asset);
				unset($bc2_asset);
			}//end if

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bc_asset);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($bc_asset);
			unset($asset);
		}//end foreach

		return $this->_createBodycopies();

	}//end _createAdditional()


	/**
	* Creates bodycopies for this asset
	*
	* Override if different bodycopies are needed (eg. in search page).
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		foreach ($this->bodycopies as $value => $name) {
			$copy_link = Array(
							'asset'			=> &$this,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
							'value'			=> $value,
						 );

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			$args = Array('content' => $this->_getDefaultBodycopyContent($value));
			if (!$bodycopy->create($copy_link, $args)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end _createBodycopies()


	/**
	* Get the default content for a bodycopy
	*
	* @param string $bodycopy_code	The code name for the bodycopy
	*
	* @return string
	* @access private
	*/
	function _getDefaultBodycopyContent($bodycopy_code)
	{
		switch ($bodycopy_code) {
			case 'initial':
				return '<p>Search For:</p><p>%search_query%</p><p>%submit_button%</p>';
			case 'results':
				return '<p>%initial_layout%<br />%spell_suggestion%<br /> %result_count% Results found:</p><p>%result_featured% <br />%result_list%</p><p>%previous_result_page_link% %page_list_10% %next_result_page_link%</p><p>%fluster_category_topic%</p>';
			case 'no_results':
				return '<p>No results</p>%initial_layout%';
			case 'default_format':
			case 'custom_format':
				return '<p><a href="%result_url%">%result_title%</a><br />%result_summary%</p>';
			case 'featured_format':
				return '<p><a href="%featured_url%">%featured_title%</a><br />%featured_summary%</p>';
			case 'fluster_format':
				return '<p>%fluster_current_term% by %fluster_category_name%</p> <p>%fluster_list%</p>';
			case 'cluster_format':
				return '<p><a href="%cluster_url%">%cluster_name%</a> (%cluster_count%)</p>';
			case 'navigation_format':
				return '<p><a href="%navigation_url%">%navigation_name%</a></p>';
			case 'spell_format':
				return '<p>Did you mean: <a href="%spell_url%">%spell_text%</a>?</p>';
		}//end switch
		return parent::_getDefaultBodycopyContent($bodycopy_code);

	}//end _getDefaultBodycopyContent()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object &$asset			the major or minor asset that we are linking to
	* @param string $side_of_link	the side of the link we are on (major or minor)
	* @param string &$link_type		the type of link we are creating
	* @param string &$value			the value that is to be associated with the link
	* @param string &$sort_order	the position in the links list that this link should take,
	* @param string &$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string &$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Bodycopy) && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid the link id of the link to remove
	*
	* @return mixed string|boolean
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

		if ($bodycopy_link['minor_type_code'] == 'bodycopy' && in_array($bodycopy_link['value'], $this->bodycopies) && !$GLOBALS['SQ_PURGING_TRASH']) {
			return 'Cannot delete bodycopy';
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 'M', 'exclusive' => TRUE);
		$page_links[SQ_LINK_TYPE_2]['folder']	= Array('card' => 4, 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['folder']	= Array('card' => 1, 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['asset']	= Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns the search URL 
	*
	* @return string
	* @access public
	*/
	function getSearchURL()
	{
		$url = $this->attr('submit_to_page_url') ? $this->getURL() : current_url();
		return $url;

	}//end getSearchURL()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		$prefix = $this->getPrefix();
		if (isset($_REQUEST[$prefix.'_submit_button']) && empty($_REQUEST['mode'])) {
			$_REQUEST['mode'] = 'results';
		}//end if
		$mode  = array_get_index($_REQUEST, 'mode', '');

		$search_vars = Array();
		$search_vars['query'] = array_get_index($_REQUEST, $prefix.'_search_query', '');
		$search_vars['scope'] = array_get_index($_REQUEST, $prefix.'_search_scope', '');
		$search_vars['page'] = array_get_index($_REQUEST, 'current_result_page', 1);
		$search_vars['num_per_page'] = array_get_index($_REQUEST, 'results_per_page', $this->_getNumPerPage());

		// If the flustering links are clicked, we need these extra vars to pass off to funnelback
		$search_vars['stem'] = array_get_index($_REQUEST, $prefix.'_stem', '');
		$search_vars['type_max'] = array_get_index($_REQUEST, $prefix.'_type_max_clusters', '');
		$search_vars['topic_max'] = array_get_index($_REQUEST, $prefix.'_topic_max_clusters', '');
		$search_vars['site_max'] = array_get_index($_REQUEST, $prefix.'_site_max_clusters', '');
		$search_vars['cluster'] = Array();
		foreach ($_GET as $index => $value) {
			if (strpos($index, $prefix.'_cluster') !== FALSE) {
				$count = str_replace($prefix.'_cluster', '', $index);
				$search_vars['cluster'][$count] = $value;
			}//end if
		}//end foreach

		// Process any dynamic vars
		$pmap = $this->getAttribute('dynamic_parameters');
		$pmap_params = $pmap->getParameters();
		$jump_to_result = FALSE;
		if (is_array($pmap_params) && !empty($pmap_params)) {
			foreach ($pmap_params as $params) {
				switch ($params) {
					case 'query':
						$dynamic_query = $pmap->getParameterValue($params);
						if (!empty($dynamic_query) && empty($search_vars['query'])) {
							$search_vars['query'] = $dynamic_query;
							$jump_to_result = TRUE;
						}//end if
					break;

					case 'results_per_page':
						$dynamic_results = $pmap->getParameterValue($params);
						if (!empty($dynamic_results)) {
							$search_vars['num_per_page'] = $dynamic_results;
						}//end if
					break;

					case 'scope':
						$dynamic_scope = $pmap->getParameterValue($params);
						if (!empty($dynamic_scope) && empty($search_vars['scope'])) {
							$search_vars['scope'] = $dynamic_scope;
						}//end if
					break;
				}//end switch
			}//end foreach
		}//end if

		if (empty($mode)) {
			$mode = 'initial';
		}//end if

		// If Stored Query is to be used, then jump to the results page directly
		if ($this->attr('stored_query_show_results') && $mode == 'initial' && $jump_to_result) {
			$mode = 'results';
		}//end if

		switch ($mode) {
			case 'results':
				$this->printResultsBody($search_vars);
			break;

			case 'initial':
			default:
				echo $this->getBodycopyContents('initial', $this->getInitialBodyReplacements($search_vars));
			break;
		}

	}//end printContents()


	/**
	* Prints the results bodycopy
	*
	* @param array	$search The passed search variable used for stored searches
	*
	* @return void
	* @access private
	*/
	function printResultsBody($search=Array())
	{
		$search_results = $this->processSearch($search);
		$search_results['params'] = $search;
		$results = array_get_index($search_results, 'results', Array());

		$this->_tmp['search_results'] =& $results;
		$bc_name = 'results';
		if (empty($results)) $bc_name = 'no_results';

		echo $this->getBodycopyContents($bc_name, $this->getResultsBodyReplacements($search_results, $bc_name));

	}//end printResultsBody()


// --- SEARCH FUNCTIONS --- //


	/**
	* Process the search and return the results n extra information
	*
	* @param array	$search The passed search variable used for stored searches
	*
	* @return array
	* @access private
	*/
	function processSearch($search)
	{
		// Defaults
		$search_summary = Array('summary' => Array(), 'fluster' => Array(), 'spell' => Array());
		$search_results = Array('results' => Array(), 'summary' => $search_summary);

		$url = $this->_parseURL($search);
		if (empty($url)) {
			// The error has triggered inside this function
			return $search_results;
		}//end if

		$xml_results = $this->_queryFunnelback($url);
		if (empty($xml_results)) {
			// Something went wrong
			trigger_localised_error('FNB0002', E_USER_WARNING, $this->attr('funnelback_url'));
			return $search_results;
		}//end if

		$xml = simplexml_load_string($xml_results);

		if(isset($xml->error)) {
			trigger_localised_error('FNB0002', E_USER_WARNING, (string)$xml->error->usermsg);
		} else {
			// No errors so continue processing
			// Process the search results
			$search_results['results'] = $this->_processSearchResults($xml);

			// Process the search summary
			$search_results['summary'] = $this->_processSearchSummary($xml);
		}//end if

		return $search_results;

	}//end processSearch()


// --- KEYWORD FUNCTIONS --- //


	/**
	* Prints the initial bodycopy
	*
	* @param array	$extra	The extra information that can be used by the initial bodycopy
	*
	* @return void
	* @access private
	*/
	function getInitialBodyReplacements($extra=Array())
	{
		$bodycopy =& $this->getBodycopy('initial');
		$keywords = $bodycopy->getKeywords();
		$replaces = Array();

		foreach ($keywords as $keyword) {
			$replaces[$keyword] = $this->getGeneralReplacement($keyword, $extra);
		}//end foreach

		return $replaces;

	}//end getInitialBodyReplacements()


	/**
	* Keyword-replaces for the results mode
	*
	* @param array	$results	the results returned
	* @param array	$params		the search parameters
	* @param string $bc_name	the name of the bodycopy to be printed (results | no_results)
	*
	* @return void
	* @access private
	*/
	function getResultsBodyReplacements($search_results, $bc_name='results')
	{
		$results = array_get_index($search_results, 'results', Array());

		$mode = array_get_index($_REQUEST, 'mode', 'results');
		$bodycopy =& $this->getBodycopy($bc_name);
		$keywords = $bodycopy->getKeywords();

		$replaces = Array();
		$this->_tmp['initial_results'] = $results;

		foreach ($keywords as $keyword) {
			$replacement = $this->getFunnelbackReplacement($keyword, $search_results);
			$replaces[$keyword] = $replacement;
		}//end foreach

		return $replaces;

	}//end getResultsBodyReplacements()


	/**
	* Get the Funnelback replacements
	*
	* @param string $keyword		the keyword of the wanted replacement
	* @param array	$search_results	the search information used for the keywords
	* @param int	$index			the index for the item replacements
	*
	* @return string
	* @access public
	*/
	function getFunnelbackReplacement($keyword, $search_results=Array(), $index=NULL, $type='site')
	{
		$default_fluster = Array('category' => Array('type' => Array(), 'topic' => Array(), 'site' => Array()));
		$default_summary = Array('summary'=>Array(), 'fluster'=>$default_fluster, 'spell'=>Array(), 'featured'=>Array());
		$results = array_get_index($search_results, 'results', Array());
		$summary = array_get_index($search_results, 'summary', $default_summary);
		$params  = array_get_index($search_results, 'params', Array());

		$mode = array_get_index($_REQUEST, 'mode', 'results');

		// Sort out the query vars
		$queries = '';
		foreach ($_GET as $query_name => $query_value) {
			// skip some get vars that will be added later by the keyword replacements
			if (!in_array($query_name, Array('mode', 'current_result_page', 'results_per_page'))) {
				if (is_array($query_value)) {
					foreach ($query_value as $value_index => $value_contents) {
						$queries .= '&'.urlencode($query_name).'['.urlencode($value_index).']='.urlencode($value_contents);
					}
				} else {
					$queries .= '&'.urlencode($query_name).'='.urlencode($query_value);
				}//end if
			}//end if
		}//end foreach

		// Here is where we define the common values used across the page
		$num_per_page = array_get_index($summary['summary'], 'num_ranks', 10);
		$start_record  = array_get_index($summary['summary'], 'currstart', 1);
		$last_record  = array_get_index($summary['summary'], 'currend', 1);
		$total_result = array_get_index($summary['summary'], 'total_matching', 0);
		$total_pages_main = floor($total_result / $num_per_page);
		$total_pages = $total_pages_main + (($total_result % $num_per_page == 0) ? 0 : 1);
		$fully_result = array_get_index($summary['summary'], 'fully_matching', 0);
		$partial_result = array_get_index($summary['summary'], 'partially_matching', 0);
		$current_result_page = floor($last_record / $num_per_page);
		$current_result_page = (($last_record % $num_per_page) != 0) ? $current_result_page+1 : $current_result_page;
		$category   = array_get_index($summary['fluster'], 'category', Array('type' => Array(), 'topic' => Array(), 'site' => Array()));
		$category_type = array_get_index($category, $type, Array());

		$replacement = '%'.$keyword.'%';
		if (strpos($keyword, 'page_list') === 0) {
			// Page list keyword
			$show_pages = substr($keyword, 9);
			$show_pages = ltrim($show_pages, '_');
			if (empty($show_pages) || $show_pages > $total_pages) {
				$show_pages = $total_pages;
			}//end if
			$replacement = $this->_paintSlidingPageNavigation($mode, $current_result_page, $total_pages, $show_pages, $num_per_page, $queries);
		} else {
			// Everything else
			switch ($keyword) {
				case 'result_list':
					// print the results
					ob_start();
						$this->printResultList($search_results);
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'fluster_category_type':
					// print the type category
					if (!empty($summary['fluster']) && !empty($category_type)) {
						ob_start();
							$this->printFlusterList($search_results, 'type');
							$replacement = ob_get_contents();
						ob_end_clean();
					}//end if
				break;

				case 'fluster_category_topic':
					// print the topic category
					if (!empty($summary['fluster']) && !empty($category_type)) {
						ob_start();
							$this->printFlusterList($search_results, 'topic');
							$replacement = ob_get_contents();
						ob_end_clean();
					}//end if
				break;

				case 'fluster_category_site':
					// print the site category
					if (!empty($summary['fluster']) && !empty($category_type)) {
						ob_start();
							$this->printFlusterList($search_results, 'site');
							$replacement = ob_get_contents();
						ob_end_clean();
					}//end if
				break;

				case 'fluster_current_term':
					$replacement = '';
					$navigation = array_get_index($summary['fluster'], 'navigation', Array());
					if (!empty($navigation)) {
						$last_record = array_pop($navigation);
						$replacement = array_get_index($last_record, 'name', '');
					}//end if
				break;

				case 'fluster_navigation':
					$navigation = array_get_index($summary['fluster'], 'navigation', Array());
					ob_start();
						echo '<div id="fluster_navigation">';
						foreach ($navigation as $index => $nav) {
							$this->printNavigationList($search_results, $index);
						}//end foreach
						echo '</div>';
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'result_featured':
					// print the asset list from the listing engine
					if (!empty($summary['featured'])) {
						ob_start();
							$featured = array_get_index($summary, 'featured', Array());
							if (!empty($featured)) {
								foreach ($featured as $index => $feature) {
									$this->printFeaturedList($search_results, $index);
									$replacement = ob_get_contents();
								}//end foreach
							}//end if
						ob_end_clean();
					}//end if
				break;

				case 'current_result_page':
					$replacement = $current_result_page;
				break;

				case 'current_result_page_start':
					$replacement  = $start_record;
				break;

				case 'current_result_page_end':
					$replacement  = $last_record;
				break;

				case 'result_count':
					$replacement = $total_result;
				break;

				case 'result_count_fully':
					$replacement = $fully_result;
				break;

				case 'result_count_partially':
					$replacement = $partial_result;
				break;

				case 'result_count_pages':
					$replacement = $total_pages;
				break;

				case 'result_per_page':
					$replacement = $num_per_page;
				break;

				case 'result_page_index':
					$replacement = $this->_paintSlidingPageNavigation($mode, $current_result_page, $total_pages, $total_pages, $num_per_page, $queries);
				break;

				case 'previous_result_page_link':
					if ($current_result_page <= 1) {
						$replacement = $this->attr('prev_page_text');
					} else {
						$replacement = '<a href="'.$this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page - 1).'&results_per_page='.$num_per_page.$queries).'">'.$this->attr('prev_page_text').'</a>';
					}//end if
				break;

				case 'next_result_page_link':
					if ($current_result_page >= $total_pages) {
						$replacement = $this->attr('next_page_text');
					} else {
						$replacement = '<a href="'.$this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page + 1).'&results_per_page='.$num_per_page.$queries).'">'.$this->attr('next_page_text').'</a>';
					}//end if
				break;

				case 'previous_result_page_href':
					if ($current_result_page <= 1) {
						$replacement = '';
					} else {
						$replacement = $this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page - 1).'&results_per_page='.$num_per_page.$queries);
					}//end if
				break;

				case 'next_result_page_href':
					if ($current_result_page >= $total_pages - 1) {
						$replacement = '';
					} else {
						$replacement = $this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page + 1).'&results_per_page='.$num_per_page.$queries);
					}//end if
				break;

				case 'initial_layout':
					$replacement = $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
				break;

				case 'result_title':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'title', '');
				break;

				case 'result_summary':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'summary', '');
				break;

				case 'result_url':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'live_url', '');
				break;

				case 'result_cache_url':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'cache_url', '');
				break;

				case 'result_size':
					$replacement = easy_filesize(array_get_index(array_get_index($results, $index, Array()), 'filesize', ''));
				break;

				case 'result_type':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'filetype', '');
				break;

				case 'featured_trigger':
					$current_feature = array_get_index($summary['featured'], $index, Array());
					$replacement = array_get_index($current_feature, 'fp_trigger', '');
				break;

				case 'featured_url':
					$current_feature = array_get_index($summary['featured'], $index, Array());
					$replacement = array_get_index($current_feature, 'fp_link', '');
				break;

				case 'featured_title':
					$current_feature = array_get_index($summary['featured'], $index, Array());
					$replacement = array_get_index($current_feature, 'fp_title', '');
				break;

				case 'featured_summary':
					$current_feature = array_get_index($summary['featured'], $index, Array());
					$replacement = array_get_index($current_feature, 'fp_desc', '');
				break;

				case 'fluster_category_name':
					$replacement = ucwords($type);
				break;

				case 'fluster_current_term':
					$navigation = array_get_index($summary['fluster'], 'navigation', Array());
					$replacement = '';
					if (!empty($navigation)) {
						$last_record = array_pop($navigation);
						$replacement = array_get_index($last_record, 'name', '');
					}//end if
				break;

				case 'fluster_navigation':
					$navigation = array_get_index($summary['fluster'], 'navigation', Array());
					ob_start();
						echo '<div id="fluster_navigation">';
						foreach ($navigation as $index => $nav) {
							$this->printNavigationList($search_results, $index);
						}//end foreach
						echo '</div>';
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'fluster_list':
					$replacement = '';
					ob_start();
						if (!empty($category[$type])) {
							echo '<div id="fluster_category">';

							// Print the list
							foreach ($category[$type] as $index => $cluster) {
								if ((string)$index != 'more') {
									$this->printClusterList($search_results, $index, $type);
								}//end if
							}//end foreach

							// Show the more link if set
							if (isset($category[$type]['more']) && $category[$type]['more'] && $this->attr('show_fluster_more')) {
								// Variables
								$query_cluster = $type.'_max_clusters';
								$max_attr = 'more_'.$type.'_num';
								$max_clusters = $this->attr($max_attr);
								$min_attr = 'less_'.$type.'_num';
								$min_clusters = $this->attr($min_attr);
								$cluster_query = $this->getPrefix().'_'.$query_cluster.'='.$max_clusters;
								$cluster_name = $this->attr('more_'.$type.'_text');
								$queries = '';

								// Sort out the current query vars
								foreach ($_GET as $query_name => $query_value) {
									if (is_array($query_value)) {
										foreach ($query_value as $value_index => $value_contents) {
											$queries .= '&'.$query_name.'['.$value_index.']='.$value_contents;
										}//end foreach
									} else {
										$current_query = $query_name.'='.$query_value;
										$queries .= '&'.$current_query;
										// Already using the 'show more link', flip to 'show less'
										if (strpos($current_query, $cluster_query) === 0) {
											$cluster_query = $this->getPrefix().'_'.$query_cluster.'='.$min_clusters;
											$cluster_name = $this->attr('less_'.$type.'_text');
										}//end if
									}//end if
								}//end foreach

								// Build the url to use
								if (strpos($queries, '&') === 0) {
									$queries = substr($queries, 1);
								}//end if
								$url = $this->getSearchUrl().'?'.$queries.'&'.$cluster_query;

								// Here we are to print the more... flag
								echo '<a href="'.$url.'">'.$cluster_name.'</a>';
							}//end if
							echo '</div>';
						}//end if
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'cluster_name':
					$replacement = array_get_index(array_get_index($category_type, $index, Array()), 'name', '');
				break;

				case 'cluster_url':
					$replacement = $this->getSearchURL();
					$url = array_get_index(array_get_index($category_type, $index, Array()), 'href', '');
					if (!empty($url)) {
						$parts = $this->_getQueryVars($url);
						$queries = Array();
						$query_found = FALSE;
						foreach ($parts as $switch => $param) {
							if ($switch == 'query_prox') {
								$queries[$this->getPrefix().'_search_query'] = $param;
								$query_found = TRUE;
							} else {
								$queries[$this->getPrefix().'_'.$switch] = $param;
							}//end if
						}//end foreach

						// Only add in the extra when a query is found
						if ($query_found) {
							$queries[$this->getPrefix().'_submit_button'] = $this->attr('submit_button_text');
							$queries['mode'] = 'results';
						}//end if

						// Build the URL
						$get_vars = '';
						if (!empty($queries)) {
							foreach ($queries as $switch => $param) {
								$get_vars .= '&'.$switch.'='.$param;
							}//end foreach

							$get_vars = substr($get_vars, 1);
							$get_queries = '?'.$get_vars;
						}//end if
						$replacement .= $get_queries;
					}//end if
				break;

				case 'cluster_count':
					$replacement = array_get_index(array_get_index($category_type, $index, Array()), 'count', '');
				break;

				case 'navigation_name':
					$navigation = array_get_index($summary['fluster'], 'navigation', Array());
					$current_element = array_get_index($navigation, $index, Array());
					$replacement = array_get_index($current_element, 'name', '');
				break;

				case 'navigation_url':
					$replacement = '';
					$navigation = array_get_index($summary['fluster'], 'navigation', Array());
					$current_element = array_get_index($navigation, $index, Array());
					// Process the URL to Matrix format
					$url = array_get_index($current_element, 'url', '');
					if (!empty($url) && (strpos($url, '&') !== FALSE)) {
						$query_vars = $this->_getQueryVars($url);
						$query_term = array_get_index($query_vars, 'query_prox', '');
						if (!empty($query_term)) {
							$replacement  = $this->getSearchURL();
							$replacement .= '?'.$this->getPrefix().'_search_query='.$query_term;
							$replacement .= '&'.$this->getPrefix().'_submit_button='.$this->attr('submit_button_text');
							$replacement .= '&mode=results';
						}//end if
					}//end if
				break;

				case 'spell_url':
					$url = '';
					$spelling = array_get_index($summary, 'spell', Array());
					$spell_url = array_get_index($spelling, 'url', '');
					// Parse the query
					$returned_get_vars = $this->_getQueryVars($spell_url);
					$get_vars = '';
					foreach ($returned_get_vars as $var_name => $var_value) {
						if ($var_name == 'query') {
							$var_name = 'search_query';
						}//end if
						$get_vars .= '&'.$this->getPrefix().'_'.$var_name.'='.$var_value;
					}//end foreach

					if (!empty($get_vars)) $url = '?mode=results'.$get_vars;
					$replacement = $this->getSearchURL().$url;
				break;

				case 'spell_text':
					$spelling = array_get_index($summary, 'spell', Array());
					$replacement = array_get_index($spelling, 'text', '');
				break;

				default:
					$replacement = $this->getGeneralReplacement($keyword, $params, $summary['spell']);
			}//end switch
		}//end if

		return $replacement;
	
	}//end getFunnelbackReplacement()


	/**
	* Returns replacements that that are common to the bodycopies
	*
	* @param string $keyword	the keyword of the wanted replacement
	* @param array	$extra		the extra information that can be used by the initial bodycopy
	*
	* @return string
	* @access public
	*/
	function getGeneralReplacement($keyword, $extra, $spelling=Array())
	{
		$prefix = $this->getPrefix();
		$replacement = '';
		switch ($keyword) {

			case 'current_query' :
				$field_name = $prefix.'_search_query';
				$current_value = array_get_index($_REQUEST, $field_name, '');

				$replacement = $current_value;
			break;

			case 'search_query' :
				$field_name = $prefix.'_search_query';
				$current_value = array_get_index($_REQUEST, $field_name, '');
				$dynamic_query = array_get_index($extra, 'query', '');
				if (!empty($dynamic_query) && empty($current_value)) {
					$current_value = $dynamic_query;
				}//end if
				$this->registerFormField($field_name);

				$replacement = '<input type="text" name="'.$field_name.'" id="'.$field_name.'" value="'.$current_value.'" />';
			break;

			case 'search_scope' :
				$field_name = $prefix.'_search_scope';
				$scope = $this->attr('scope');
				$current_value = array_get_index($_REQUEST, $field_name, '');
				$dynamic_scope = array_get_index($extra, 'scope', '');
				if (!empty($dynamic_scope) && empty($current_value)) {
					$current_value = $dynamic_scope;
				}//end if
				if (empty($current_value)) {
					$current_value = $this->attr('default_scope');
				}//end if
				$this->registerFormField($field_name);

				ob_start();
					foreach ($scope as $key => $value) {
						$checked = FALSE;
						if ($key == $current_value) {
							$checked = TRUE;
						}//end if
						radio_button($field_name, $key, $checked);
						label($value, $field_name);
					}//end foreach
					$replacement = ob_get_contents();
				ob_end_clean();
			break;

			case 'spell_suggestion':
				// print the spelling suggestion
				if (!empty($spelling)) {
					// Check for same spelling and dont print it
					$current_term = array_get_index($_REQUEST, $this->getPrefix().'_search_query', '');
					$spell_term = array_get_index($spelling, 'text', '');
					$spell_term = trim($spell_term);
					if ($current_term != $spell_term) {
						ob_start();
							$this->printSpellList($spelling);
							$replacement = ob_get_contents();
						ob_end_clean();
					}//end if
				}//end if
			break;

			case 'search_button' :
			case 'submit_button' :
				// If fluster is set, we need to unset them here
				foreach ($_GET as $index => $value) {
					if ((strpos($index, $prefix.'_cluster') !== FALSE) || $index == $prefix.'_stem') {
						$count = str_replace($prefix.'_cluster', '', $index);
						hidden_field($index, '');
						$this->registerFormField($index);
					}//end if
				}//end foreach

				// Go on to create the button
				$field_name = $prefix.'_submit_button';
				$this->registerFormField($field_name);
				$this->_tmp['prints_submit'] = TRUE;

				// if we are on the initial search page, don't use javascript
				// for the submit button so enter will submit the form
				$mode = array_get_index($_REQUEST, 'mode', 'initial');
				if ($mode == 'initial') {
					$replacement  = '<input type="submit" name="'.$field_name.'"
						value="'.$this->attr('submit_button_text').'" />';
				} else {
					$replacement  = '<input type="submit" name="'.$field_name.'" onclick="listing_form = document.getElementById(\''.$this->getPrefix().'\');
						listing_form.current_result_page.value=\'1\';
						if (typeof listing_form.mode != \'undefined\') listing_form.mode.value = \'results\';
						return true;
					" value="'.$this->attr('submit_button_text').'" />';
				}
			break;

			case 'results_per_page' :
				$results_per_page = array_get_index($_REQUEST, 'results_per_page', $this->_getNumPerPage());
				$field_name = 'results_per_page_text_box';
				$this->registerFormField($field_name);
				$replacement = '<input type="text" name="'.$field_name.'" onchange="listing_form = document.getElementById(\''.$this->getPrefix().'\');
					listing_form.results_per_page.value = this.value;
				" value="'.$results_per_page.'" size="5" />';
			break;

			default :
				$replacement = $this->getKeywordReplacement($keyword);
		}//end switch

		return $replacement;

	}//end getGeneralReplacement()


	/**
	* Returns replacements for the sliding page index 
	*
	* @param int	$pages	the number of pages to show
	*
	* @return string
	* @access public
	*/
	function _paintSlidingPageNavigation($mode, $current_result_page, $total_pages, $show_pages, $num_per_page, $queries='')
	{
		$replacement = '';
		if ($total_pages == $show_pages) {
			$start_page = 1;
			$end_page = $total_pages;
		} else {
			$start_page = max(1, min(($current_result_page - (int) floor($show_pages / 2)), $total_pages - $show_pages + 1));
			$end_page = min($total_pages, $start_page + $show_pages - 1);
		}//end if

		for ($i = $start_page; $i <= $end_page; $i++) {
			if ($replacement != '') $replacement .= ' ';

			if ($i == $current_result_page) {
				$replacement .= '<b>'.$i.'</b>';
			} else {
				$replacement .= '<a href="'.$this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.$i.'&results_per_page='.$num_per_page.$queries).'">'.$i.'</a>';
			}
		}//end for

		return $replacement;

	}//end _paintSlidingPageNavigation()


// -- BODYCOPY FUNCTIONS -- //


	/**
	* Retrieves a bodycopy by name
	*
	* Returns the bodycopy, or NULL if the name does not match any bodycopies
	*
	* @param string $name	the name of the bodycopy to retrieve
	*
	* @return mixed object|NULL
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		$bodycopy = NULL;

		if (array_key_exists($name, $this->bodycopies)) {
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name, 'major', '1');
			$bodycopy = $am->getAsset($link['minorid'], $link['minor_type_code']);
		}

		return $bodycopy;

	}//end getBodycopy()


	/**
	* Retrieves a bodycopy's contents by name
	*
	* @param string $name			the name of the bodycopy to retrieve
	* @param array	$replacements	replacements
	*
	* @return mixed string|NULL
	* @access private
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	*/
	function &getBodycopyContents($name, $replacements=Array())
	{
		$bodycopy =& $this->getBodycopy($name);
		if (is_null($bodycopy)) return '';

		$bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();

		return $html;

	}//end getBodycopyContents()


	/**
	* Returns the link for the format folder
	*
	* @return array
	* @access public
	*/
	function getFormatFolderLink()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'folder', TRUE, 'major', 'format_folder');
		return (!empty($link)) ? $link[0] : Array();

	}//end getFormatFolderLink()


	/**
	* Returns the format folder where the format bodycopies live
	*
	* @return object
	* @access public
	*/
	function &getFormatFolder()
	{
		$folder = NULL;

		$link = $this->getFormatFolderLink();
		if (!empty($link)) {
			$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		}

		return $folder;

	}//end getFormatFolder()


	/**
	* Returns the link for a bodycopy with the specified link value
	*
	* @param string $link_value the link value of the link of the wanted bodycopy
	*
	* @return array
	* @access public
	*/
	function getFormatBodycopyLink($link_value)
	{
		$format_folder =& $this->getFormatFolder();
		if (is_null($format_folder)) return Array();

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($format_folder->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', TRUE, 'major', $link_value);
		return (!empty($link)) ? $link[0] : Array();

	}//end getFormatBodycopyLink()


	/**
	* Returns the format bodycopy with the specifed link value
	*
	* If the bodycopy does not exist, or it have a link type 3, NULL is returned
	* as either no format has been customised, or the format feature has been disbled
	*
	* @param string $link_value the link value of the wanted bodycopy
	*
	* @return object
	* @access public
	*/
	function &getFormatBodycopy($link_value)
	{
		$bodycopy = NULL;

		$bodycopy_link = $this->getFormatBodycopyLink($link_value);
		if (!empty($bodycopy_link)) {
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		}

		return $bodycopy;

	}//end getFormatBodycopy()


// --- RESULT LISTING FUNCTIONS --- //


	/**
	* Prints the result list 
	*
	* This method ensures the list is printed in columns if required, and keeps
	* track of what position in the list we are printing
	*
	* @param array	$search_params	The search parameters
	* @param array	$search_results	The search results
	*
	* @return void
	* @access private
	*/
	function printResultList($search_results)
	{
		$todo = array_get_index($search_results, 'results', Array());
		if (empty($todo)) return;

		foreach ($todo as $index => $result) {
			$this->_printResult($search_results, $index);
		}//end foreach

	}//end _printResultList()


	/**
	* Prints the individual result in the list
	*
	* @param array	$search_results		The current search results
	* @param string $assetid			The assetid of the asset we are printing
	*
	* @return void
	* @access protected
	*/
	function _printResult($search_results, $index)
	{
		$results = array_get_index($search_results, 'results', Array());

		// Setting the Default Type Folder
		if (!isset($this->_tmp['type_folder_id'])) {
			$type_folder = $this->getFolder('type_formats');
			$this->_tmp['type_folder_id'] = $type_folder->id;
		}//end if

		// Getting the Default Format Bodycopy
		if (!isset($this->_tmp['default_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'default_format');
			if ($link) {
				$this->_tmp['default_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Default');
				return;
			}
		}
		$bodycopy = $this->_tmp['default_format_bc'];

		// Check for a customised layout for this item only
		$filetype = array_get_index($results[$index], 'filetype', '');
		$custom_type = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, $filetype);
		if (!empty($custom_type)) {
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($custom_type['minorid'], 'bodycopy');
		}//end if

		// by the time we reach this point we already have a bodycopy for this asset
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		foreach ($bc_keywords as $keyword) {
			$keywords[$keyword] = $this->getFunnelbackReplacement($keyword, $search_results, $index);	
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end _printResult()


	/**
	* Prints the featured bodycopy
	*
	* @param array	$results	The current search summary
	* @param int	$index		The current featured index number
	*
	* @return void
	* @access protected
	*/
	function printFeaturedList($results, $index)
	{
		// Setting the Default Type Folder
		if (!isset($this->_tmp['type_folder_id'])) {
			$type_folder = $this->getFolder('type_formats');
			$this->_tmp['type_folder_id'] = $type_folder->id;
		}//end if

		// Getting the Default Format Bodycopy
		if (!isset($this->_tmp['featured_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['type_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'featured_format');
			if ($link) {
				$this->_tmp['featured_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Featured');
				return;
			}
		}
		$bodycopy = $this->_tmp['featured_format_bc'];

		// by the time we reach this point we already have a bodycopy for this asset
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		foreach ($bc_keywords as $keyword) {
			$keywords[$keyword] = $this->getFunnelbackReplacement($keyword, $results, $index);	
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end printFeaturedList()


	/**
	* Prints the fluster bodycopy
	*
	* @param array	$results	The current search summary
	* @param string	$type		The fluster category to print
	*
	* @return void
	* @access protected
	*/
	function printFlusterList($results, $type='site')
	{
		// Getting the Fluster Format folder
		if (!isset($this->_tmp['fluster_folder_id'])) {
			$fluster_folder = $this->getFolder('fluster_formats');
			$this->_tmp['fluster_folder_id'] = $fluster_folder->id;
		}//end if

		// Getting the Fluster Default Format Bodycopy
		if (!isset($this->_tmp['fluster_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['fluster_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'fluster_format');
			if ($link) {
				$this->_tmp['fluster_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Fluster');
				return;
			}
		}
		$bodycopy = $this->_tmp['fluster_format_bc'];

		// by the time we reach this point we already have a bodycopy for this asset
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		foreach ($bc_keywords as $keyword) {
			$keywords[$keyword] = $this->getFunnelbackReplacement($keyword, $results, 0, $type);	
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end printFlusterList()


	/**
	* Prints the cluster bodycopy
	*
	* @param array	$results	The current search summary
	* @param int	$index		The current cluster item
	* @param string	$type		The type of cluster to print
	*
	* @return void
	* @access protected
	*/
	function printClusterList($results, $index, $type='site')
	{
		// Getting the Cluster Format folder
		if (!isset($this->_tmp['fluster_folder_id'])) {
			$fluster_folder = $this->getFolder('fluster_formats');
			$this->_tmp['fluster_folder_id'] = $fluster_folder->id;
		}//end if

		// Getting the Cluster Default Format Bodycopy
		if (!isset($this->_tmp['cluster_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['fluster_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'cluster_format');
			if ($link) {
				$this->_tmp['cluster_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Cluster');
				return;
			}
		}
		$bodycopy = $this->_tmp['cluster_format_bc'];

		// by the time we reach this point we already have a bodycopy for this asset
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		// Go through and replace keywords
		foreach ($bc_keywords as $keyword) {
			$keywords[$keyword] = $this->getFunnelbackReplacement($keyword, $results, $index, $type);	
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end printClusterList()


	/**
	* Prints the navigation bodycopy
	*
	* @param array	$results	The current search summary
	* @param int	$index		The current navigation item
	*
	* @return void
	* @access protected
	*/
	function printNavigationList($results, $index)
	{
		// Getting the Fluster Format folder
		if (!isset($this->_tmp['fluster_folder_id'])) {
			$fluster_folder = $this->getFolder('fluster_formats');
			$this->_tmp['fluster_folder_id'] = $fluster_folder->id;
		}//end if

		// Getting the Navigation Default Format Bodycopy
		if (!isset($this->_tmp['nav_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['fluster_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'navigation_format');
			if ($link) {
				$this->_tmp['nav_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Navigation');
				return;
			}
		}
		$bodycopy = $this->_tmp['nav_format_bc'];

		// by the time we reach this point we already have a bodycopy for this asset
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		foreach ($bc_keywords as $keyword) {
			$keywords[$keyword] = $this->getFunnelbackReplacement($keyword, $results, $index);	
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end printNavigationList()


	/**
	* Prints the spell bodycopy
	*
	* @param array	$results	The current search summary
	*
	* @return void
	* @access protected
	*/
	function printSpellList($results)
	{
		// Getting the Spell Formats folder
		if (!isset($this->_tmp['spell_folder_id'])) {
			$spell_folder = $this->getFolder('spell_formats');
			$this->_tmp['spell_folder_id'] = $spell_folder->id;
		}//end if

		// Getting the Spell Default Format Bodycopy
		if (!isset($this->_tmp['spell_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['spell_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'spell_format');
			if ($link) {
				$this->_tmp['spell_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Spell');
				return;
			}
		}
		$bodycopy = $this->_tmp['spell_format_bc'];

		// by the time we reach this point we already have a bodycopy for this asset
		$bc_keywords = $bodycopy->getKeywords();
		$keywords = Array();
		foreach ($bc_keywords as $keyword) {
			$keywords[$keyword] = $this->getFunnelbackReplacement($keyword, $search_results, $index);	
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end printSpellList()


// --- PRIVATE FUNCTIONS --- //


	/**
	 * Return an array of the query vars
	 *
	 * @param string	$query_vars_string	The string of query vars
	 *
	 * @return array
	 * @access private
	 */
	private function _getQueryVars($query_string)
	{
		// The regex expression expects a start & so let us do that first
		if (strpos($query_string, '&') !== 0) {
			$query_string = '&'.$query_string;
		}//end if

		$var = Array();
		preg_match_all('|&([^=]+)=([^&]+)?|', $query_string, $matches);
		foreach($matches[1] as $index => $query_name) {
			$vars[$query_name] = isset($matches[2][$index]) ? $matches[2][$index] : '';
		}//end foreach

		return $vars;

	}//end _getQueryVars()


	/**
	* Parse the URL
	*
	* @param array	$search The search vars
	*
	* @return string
	* @access private
	*/
	function _parseURL($search)
	{
		// Get the query term
		$query = array_get_index($search, 'query', '');
		$query = str_replace(' ', '+', $query);

		// Play nice
		$query = str_replace('&', '', $query);

		// Get the URL
		$funnelback_url = $this->attr('funnelback_url');
		$funnelback_url = trim($funnelback_url);

		// Get the other vars
		$collection = $this->attr('collection');
		$scope = array_get_index($search, 'scope', '');
		$current_result_page = array_get_index($search, 'page', 1);
		$num_per_page = array_get_index($search, 'num_per_page', $this->_getNumPerPage());
		$cluster = array_get_index($search, 'cluster', Array());
		$stem = array_get_index($search, 'stem', 0);
		$type_max = array_get_index($search, 'type_max', 0);
		$topic_max = array_get_index($search, 'topic_max', 0);
		$site_max = array_get_index($search, 'site_max', 0);

		// Check for required information
		if (empty($funnelback_url) || empty($collection)) {
			// No server set
			trigger_localised_error('FNB0003', E_USER_WARNING);
			return '';
		}//end if

		// Add in the collection
		$url = '?collection='.urlencode($collection);

		// Add in the scope
		if (empty($scope)) {
			$scope = $this->attr('default_scope');
		}//end if
		if (!empty($scope) && $scope != 'ALL') {
			$url .= '&scope='.urlencode($scope);
		}//end if

		// Add in the paging commands
		if ($current_result_page != 1) {
			$url .= '&start_rank='.urlencode((($current_result_page-1) * $num_per_page) + 1);
		}//end if
		if (!empty($num_per_page)) {
			$url .= '&num_ranks='.urlencode($num_per_page);
		}//end if

		if (!empty($cluster)) {
			// If flustering links where clicked
			$cluster_string = '';
			foreach ($cluster as $count => $item) {
				if (empty($item)) continue;
				$cluster_string .= '&cluster'.$count.'='.urlencode($item);
			}//end foreach
			$query_string = '';
			rsort($cluster);
			foreach ($cluster as $item) {
				$query_string .= ' '.$item;
			}//end foreach
			$query_string = substr($query_string, 1);
			if (!empty($cluster_string)) {
				$url .= $cluster_string;
				$url .= '&query_prox='.urlencode(str_replace('+', ' ', $query));

				// Add in the stem var
				if (!empty($stem)) {
					$url .= '&stem='.urlencode($stem);
				}//end if
			} else {
				$url .= '&query='.$query;
			}//end if
		} else {
			// Add in the query
			$url .= '&query='.$query;
		}//end if

		// Fluster more/less links
		if (!empty($type_max)) {
			$url .= '&type.max_clusters='.$type_max;
		} else {
			$url .= '&type.max_clusters='.$this->attr('less_type_num');
		}//end if
		if (!empty($topic_max)) {
			$url .= '&topic.max_clusters='.$topic_max;
		} else {
			$url .= '&topic.max_clusters='.$this->attr('less_topic_num');
		}//end if
		if (!empty($site_max)) {
			$url .= '&site.max_clusters='.$site_max;
		} else {
			$url .= '&site.max_clusters='.$this->attr('less_site_num');
		}//end if

		return $funnelback_url.$url;

	}//end _parseURL()


	/**
	* Connect to the funnelback server and returns the XML from the server
	*
	* @param string $url	The URL to connect to
	*
	* @return string
	* @access private
	*/
	function _queryFunnelback($url)
	{
		if (empty($url)) {
			trigger_localised_error('FNB0003', E_USER_WARNING);
			return '';
		}//end if

		$url_ok = @parse_url($url);
		if (!$url_ok) {
			trigger_localised_error('FNB0003', E_USER_WARNING);
			return '';
		}//end if

		$Fetch_URL =& new Net_URL($url);
		$url = $Fetch_URL->getURL();

		$request_parameters['timeout'] = 5;
		$HTTP_Client =& new HTTP_Client($request_parameters);
		$HTTP_Client->setMaxRedirects(2);

		$result = $HTTP_Client->get($url);
		if (PEAR::isError($result)) {
			trigger_localised_error('FNB0002', E_USER_WARNING);
			return '';
		}
		$response = $HTTP_Client->currentResponse();

		$code = array_get_index($response, 'code', '');
		$xml = array_get_index($response, 'body', '');
		if (empty($xml) || empty($code) || $code != '200') {
			return '';
		}//end if

		return $xml;

	}//end _queryFunnelback()


	/**
	* Strip URL ( this de-funnelbacks urls ) ;) 
	*
	* @param string $url	The URL to strip
	*
	* @return string
	* @access private
	*/
	function _stripURL($url)
	{
		return str_replace('/search/xml.cgi?', '', $url);

	}//end _stripURL()


	/**
	* Process the search summary information
	*
	* @param object &$xml	The simplexml object with the goods!
	*
	* @return array
	* @access private
	*/
	function _processSearchSummary(&$xml)
	{
		$default_fluster = Array('category' => Array('type' => Array(), 'topic' => Array(), 'site' => Array()));
		$results = Array('summary'=>Array(), 'fluster'=>$default_fluster, 'spell'=>Array(), 'featured'=>Array());
		/**
		 * <results_summary>
		 *		<fully_matching></fully_matching>
		 *		<partially_matching></partially_matching>
		 *		<total_matching></total_matching>
		 *		<num_ranks></num_ranks>
		 *		<currstart></currstart>
		 *		<currend></currend>
		 *		<nextstart></nextstart>
		 * </results_summary>
		 */
		foreach ($xml->results_summary as $summary) {
			foreach ($summary as $name => $value) {
				$results['summary'][$name] = (string) $value;
			}//end foreach
		}//end foreach

		/**
		 * <featured_pages>
		 * 	<fp>
		 * 		<fp_trigger>matrix</fp_trigger>
		 * 		<fp_link>http://matrix.squiz.net/download/mysource-matrix</fp_link>
		 * 		<fp_title>Download MySource Matrix</fp_title>
		 * 		<fp_desc></fp_desc>
		 * 	</fp>
		 * </featured_pages>
		 */
		if (isset($xml->featured_pages)) { 
			foreach ($xml->featured_pages->fp as $featured_pages) {
				$fp = Array();
				foreach ($featured_pages as $page_name => $featured_page) {
					$fp[$page_name] = (string) $featured_page;
				}//end foreach
				$results['featured'][] = $fp;
			}//end foreach
		}//end if

		/**
		 * <fluster>
		 *	 <cluster_nav level="" url=""></cluster_nav>
		 *	 <category name="type" more="">
		 *	  <cluster href="" count=""></cluster>
		 *	  <cluster href="" count=""></cluster>
		 *	 </category>
		 *	 <category name="topic" more="">
		 *	  <cluster href="" count=""></cluster>
		 *	  <cluster href="" count=""></cluster>
		 *	 </category>
		 *	 <category name="site" more="">
		 *	  <cluster href="" count=""></cluster>
		 *	  <cluster href="" count=""></cluster>
		 *	 </category>
		 * </fluster>
		 */
		foreach ($xml->fluster as $fluster) {
			foreach ($fluster as $fluster_name => $fluster_value) {
				switch ($fluster_name) {
					case 'cluster_nav':
						$current_nav = Array();
						$current_nav['name'] = (string) $fluster_value;
						foreach ($fluster_value->attributes() as $attr_name => $attr_value) {
							if ($attr_name == 'url') {
								$current_nav[$attr_name] = $this->_stripURL((string) $attr_value);
							} else {
								$current_nav[$attr_name] = (string) $attr_value;
							}//end if
						}//end foreach
						$level = array_get_index($current_nav, 'level', 0);
						$results['fluster']['navigation'][$level] = $current_nav;
					break;

					case 'category':
						$value = (string) $fluster_value['name'];
						$fluster_cluster = Array();
						$more = (int) $fluster_value['more'];
						if (!empty($more)) {
							$results['fluster']['category'][$value]['more'] = TRUE;
						} else {
							$results['fluster']['category'][$value]['more']= FALSE;
						}//end if
						foreach ($fluster_value->children() as $cluster) {
							foreach ($cluster->attributes() as $attr_name => $attr_value) {
								if ($attr_name == 'href') {
									$fluster_cluster[$attr_name] = $this->_stripURL((string) $attr_value);
								} else {
									$fluster_cluster[$attr_name] = (string) $attr_value;
								}//end if
							}//end foreach
							$fluster_cluster['name'] = (string) $cluster;
							$results['fluster']['category'][$value][] = $fluster_cluster;
						}//end foreach
					break;
				}//end switch
			}//end foreach
		}//end foreach

		/**
		 * <spell>
		 *	<url></url>
		 *	<text></text>
		 *	<map>
		 *		<term></term>
		 *		<source_parameter></source_parameter>
		 *		<suggest></suggest>
		 *		<pos></pos>
		 *	</map>
		 * </spell>
		 */
		foreach ($xml->spell as $spell) {
			foreach ($spell as $spell_name => $spell_value) {
				if ($spell_name == 'map') {
					foreach ($spell_value as $map_name => $map_value) {
						$map[$map_name] = (string)$map_value;
					}//end foreach
					$results['spell']['map'][] = $map;
				} else if ($spell_name == 'text') {
					$text = $spell_value->asXML();
					$text = str_replace('<text>', '', $text);
					$text = str_replace('</text>', '', $text);
					$results['spell'][$spell_name] = $text;
				} else {
					$results['spell'][$spell_name] = (string) $spell_value;
				}//end if
			}//end foreach
		}//end foreach

		return $results;

	}//end _processSearchSummary()


	/**
	* Process the search results information
	*
	* @param object &$xml	The simplexml object with the goods!
	*
	* @return array
	* @access private
	*/
	function _processSearchResults(&$xml)
	{
		$results = Array();
		foreach ($xml->results->result as $result) {
			$each_result = Array();
			/**
			 * Result Node
			 * <result>
			 *		<rank></rank>
			 *		<score></score>
			 *		<title></title>
			 *		<collection></collection>
			 *		<component></component>
			 *		<live_url></live_url>
			 *		<cache_url></cache_url>
			 *		<date></date>
			 *		<filesize></filesize>
			 *		<filetype></filetype>
			 *		<tier></tier>
			 *		<docnum></docnum>
			 * </result>
			 */
			foreach ($result as $key => $value) {
				$each_result[$key] = (string)$value;
			}//end foreach

			$rank = array_get_index($each_result, 'rank', 0);

			// Return a result TODO Unsure about this yet!
			if (!empty($rank)) {
				$results[$rank] = $each_result;
			}//end if
		}//end foreach

		return $results;

	}//end _processSearchResults()


}//end class

?>
