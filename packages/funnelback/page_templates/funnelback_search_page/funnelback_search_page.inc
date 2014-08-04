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
* $Id: funnelback_search_page.inc,v 1.49 2013/05/31 02:50:34 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Funnelback_Search_Page
*
* Page template for funnelback searching
*
* @see Page
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.49 $
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
	var $defaults = Array('query_width' => 25);

	/**
	* An array of keywords replacements that can be used in this page
	*
	* @var array
	*/
	var $keywords = Array(
						'initial'			=> Array(
													'current_query_term',
													'search_query',
                                                    'search_logic',
													'search_scope',
													'search_sort',
													'search_button',
													'submit_button',
													'results_per_page',
													'metadata_scope_X',
													'metadata_logic_X',
											   ),
						'results'			=> Array(
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
						'no_results'		=> Array(
													'initial_layout',
													'spell_suggestion',
											   ),
						'default_format'	=> Array(
													'result_rank',
													'result_score',
													'result_name_linked',
													'result_title',
													'result_summary',
													'result_url',
													'result_url_click_event',
													'result_url_click_js',
													'result_size',
													'result_type',
													'result_meta_X'
											   ),
						'featured_format'	=> Array(
													'featured_trigger',
													'featured_url',
													'featured_title',
													'featured_summary',
											   ),
						'context_format'	=> Array(
													'fluster_category_name',
													'fluster_current_term',
													'fluster_navigation',
													'fluster_list',
											   ),
						'cluster_format'	=> Array(
													'cluster_name',
													'cluster_url',
													'cluster_count',
											   ),
						'navigation_format'	=> Array(
													'navigation_name',
													'navigation_url',
											   ),
						'spell_format'		=> Array(
													'spell_url',
													'spell_text',
											   ),
					);


	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


	/**
	* Marking the asset deprecated since so that we can
	* use it when warning the users on backend
	*/
	public $deprecated_since = '5.1';


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
						'type_format'		=> 'folder',
						'fluster_format'	=> 'folder',
						'spell_format'		=> 'folder',
					 );

		$type_formats = NULL;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', translate($name));
			if (!$asset->create($copy_link)) return FALSE;

			// Add in the extra bodycopies
			switch ($name) {
				case 'fluster_format':
					foreach (Array('cluster_format', 'navigation_format', 'context_format') as $extra_format) {
						$bc2_asset = new Bodycopy();
						$bc2_copy_link = Array('asset' => &$asset, 'value' => $extra_format, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
						$bc2_asset->setAttrValue('name', translate($extra_format));
						$bc2_args = Array('content' => $this->_getDefaultBodycopyContent($extra_format));
						if (!$bc2_asset->create($bc2_copy_link, $bc2_args)) return FALSE;
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bc2_asset);
						unset($bc2_asset);
					}//end foreach
				break;

				case 'type_format':
					foreach (Array('featured_format', 'default_format') as $extra_format) {
						$bc2_asset = new Bodycopy();
						$bc2_copy_link = Array('asset' => &$asset, 'value' => $extra_format, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
						$bc2_asset->setAttrValue('name', translate($extra_format));
						$bc2_args = Array('content' => $this->_getDefaultBodycopyContent($extra_format));
						if (!$bc2_asset->create($bc2_copy_link, $bc2_args)) return FALSE;
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bc2_asset);
						unset($bc2_asset);
					}//end foreach
				break;

				case 'spell_format':
					$extra_format = 'spell_format';
					$bc_asset = new Bodycopy();
					$bc_copy_link = Array('asset' => &$asset, 'value' => $extra_format, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
					$bc_asset->setAttrValue('name', translate($extra_format));
					$bc_args = Array('content' => $this->_getDefaultBodycopyContent($extra_format));
					if (!$bc_asset->create($bc_copy_link, $bc_args)) return FALSE;
				break;

				default:
					// Do nothing
				break;
			}//end switch

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
				return '<p>Search: %search_query% %search_sort% %submit_button%</p>';
			case 'results':
				return '<p>%initial_layout%<br />%spell_suggestion%<br /> %result_count% Results found:</p><p>%result_featured% <br />%result_list%</p><p>%previous_result_page_link% %page_list_10% %next_result_page_link%</p><p>%fluster_category_topic%</p>';
			case 'no_results':
				return '<p>No results</p>%initial_layout%';
			case 'default_format':
			case 'custom_format':
				return '<p>%result_name_linked%<br />%result_summary%</p>';
			case 'featured_format':
				return '<p><a href="%featured_url%">%featured_title%</a><br />%featured_summary%</p>';
			case 'context_format':
				return '<p>%fluster_current_term% by %fluster_category_name%</p> <p>%fluster_list%</p>';
			case 'cluster_format':
				return '<p><a href="%cluster_url%">%cluster_name%</a> (%cluster_count%)</p>';
			case 'navigation_format':
				return '<p><a href="%navigation_url%">%navigation_name%</a></p>';
			case 'spell_format':
				return '<p>Did you mean: <a href="%spell_url%">%spell_text%</a>?</p>';
			default:
				// Do nothing
			break;
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
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'type_format');
		if (!empty($folder_link)) {
			if ($folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return translate('cms_listing_cannot_delete_type_formats_link', str_replace('_',' ', $this->type()));
			}
		}

		$fluster_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'fluster_format');
		if (!empty($fluster_folder_link)) {
			if ($fluster_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return translate('funnelback_cannot_delete_fluster_format_link', str_replace('_',' ', $this->type()));
			}
		}

		$spelling_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'spell_format');
		if (!empty($spelling_folder_link)) {
			if ($spelling_folder_link['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
				return translate('funnelback_cannot_delete_spelling_format_link', str_replace('_',' ', $this->type()));
			}
		}

		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		if ($bodycopy_link['minor_type_code'] == 'bodycopy' && array_key_exists($bodycopy_link['value'], $this->bodycopies) && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('funnelback_cannot_delete_bodycopy_link', str_replace('_', ' ', ucwords($bodycopy_link['value'])), str_replace('_',' ', ucwords($this->type())));
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
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$click = array_get_index($_GET, 'click', 0);
		$query = array_get_index($_GET, 'click_query', '');
		$rank = array_get_index($_GET, 'click_rank', '');
		$url = array_get_index($_GET, 'click_url', '');
		$collection = array_get_index($_GET, 'click_collection', '');

		if ($click == '1' && !empty($query) && !empty($rank) && !empty($url) && !empty($collection)) {
			// Only for internal searches
			$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
			$binary = $fm->getFunnelbackPath().'/bin/clix2anx';
			$coll_path = $fm->getCollectionDataPath($collection);
			$log_file = $fm->getCollectionDataPath($collection, 'log').'/click.log';
			$command = $binary.' '.escapeshellarg($query).' '.escapeshellarg($url).' '.escapeshellarg($rank).' '.escapeshellarg($log_file);
			$output = '';
			$fm->runCommand($command, $output, NULL, NULL, $coll_path);
			header('Location: '.$url);
			return;
		}//end if

		// No click log, move on....
		parent::printFrontend();

	}//end printFrontend()


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
		$search_vars['query'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_search_query', ''), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['scope'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_search_scope', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['sort'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_search_sort', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['page'] = htmlspecialchars(array_get_index($_REQUEST, 'current_result_page', 1), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['num_per_page'] = htmlspecialchars(array_get_index($_REQUEST, 'results_per_page', $this->_getNumPerPage()), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

		// If the flustering links are clicked, we need these extra vars to pass off to funnelback
		$search_vars['stem'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_stem', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['type_max'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_type_max_clusters', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['topic_max'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_topic_max_clusters', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		$search_vars['site_max'] = htmlspecialchars(array_get_index($_REQUEST, $prefix.'_site_max_clusters', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

		$search_vars['cluster'] = Array();
		foreach ($_GET as $index => $value) {
			if (strpos($index, $prefix.'_cluster') !== FALSE) {
				$count = str_replace($prefix.'_cluster', '', $index);
				$search_vars['cluster'][$count] = $value;
			}//end if
		}//end foreach

		if($this->attr('disable_keyword_replacements') && isset($search_vars['query']) && !empty($search_vars['query'])) {
			$search_vars['query'] = $this->_sanitizeQueries($search_vars['query']);
		}

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
					
					case 'sort_order':
						$dynamic_sort = $pmap->getParameterValue($params);
						if (!empty($dynamic_sort) && empty($search_vars['sort'])) {
							$search_vars['sort'] = $dynamic_sort;
						}//end if
					break;

					default:
						// Do nothing
					break;
				}//end switch
			}//end foreach
		}//end if

		if (empty($mode)) {
			$mode = 'initial';
		}//end if

		if (empty($search_vars['sort'])) {
			$search_vars['sort'] = $this->attr('sort_order');
		}
		
		// If Stored Query is to be used, then jump to the results page directly
		if ($this->attr('stored_query_show_results') && $mode == 'initial' && $jump_to_result) {
			$mode = 'results';
		}//end if


		switch ($mode) {
			case 'results':
				echo $this->_getSearchPageJavascript();
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
	* @access public
	*/
	public function processSearch($search)
	{
		// Defaults
		$search_summary = Array('summary' => Array(), 'fluster' => Array(), 'spell' => Array());
		$search_results = Array('results' => Array(), 'summary' => $search_summary);

		// Type of search
		$type = $this->attr('search_type');
		switch ($type) {
			case 'squiz':
				// SquizSearch Query through SquizSuite.
				$xml_results = $this->_querySquizSearch($search);
				if (empty($xml_results)) {
					// Something went wrong
					trigger_localised_error('FNB0002', E_USER_WARNING);
					return $search_results;
				}//end if
			break;

			case 'hosted':
			case 'web':
				// Web Search ie. Using a funnelback search engine
				$url = $this->parseURL($search);
				if (empty($url)) {
					// The error has triggered inside this function
					return $search_results;
				}//end if

				$xml_results = $this->_queryFunnelback($url);
				if (empty($xml_results)) {
					// Something went wrong
					trigger_localised_error('FNB0002', E_USER_WARNING);
					return $search_results;
				}//end if
			break;

			default:
				// Integrated Search ie. OEM Funnelback is installed
				$xml_results = $this->_searchFunnelback($search);

				if ($xml_results === FALSE) {
					// An error occurred in the search
					trigger_localised_error('FNB0003', E_USER_WARNING);
					return $search_results;
				} else if (empty($xml_results)) {
					// Empty Searches are not permitted
					return $search_results;
				}//end if
			break;
		}//end switch

		// Just in case, XML returned can be malformed
		$old_error_handler = set_error_handler('funnelback_xml_error_handler');

		try {
			$xml_results = utf8_encode($xml_results);
			$dom = new DOMDocument('1.0', 'UTF-8');
			$dom->loadXML($xml_results);
		} catch (ErrorException $e) {
			restore_error_handler();
			trigger_localised_error('FNB0002', E_USER_WARNING);
			return $search_results;
		}

		restore_error_handler();

		$errorNode = $dom->getElementsByTagName('error')->item(0);
		if (isset($errorNode)) {
			$errorMessage = $dom->getElementsByTagName('usermsg')->item(0);
			$errorMsg = (isset($errorMessage)) ? $errorMessage->nodeValue : '';
			trigger_localised_error('FNB0002', E_USER_WARNING);
		} else {
			// No errors so continue processing
			// Process the search results
			$search_results['results'] = $this->processSearchResults($dom);

			// Process the search summary
			$search_results['summary'] = $this->processSearchSummary($dom);
		}//end if

		return $search_results;

	}//end processSearch()


// --- KEYWORD FUNCTIONS --- //


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');

		$bodycopies   = Array();
		$type_formats = Array();
		$keywords     = Array();

		foreach ($bodycopy_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopies[] = $link_info['value'];
			}
		}

		// ATTENTION!
		// this check relies on the fact that ALL single asset formats like type, default or position
		// are located in subfolders of this asset, and only its context bodycopies are its direct children
		if (empty($bodycopies)) {
			// So try and guess the correct bodycopy!
			$bc_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'folder', TRUE);
			$folder_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'folder');
			
			foreach ($folder_links as $link_info) {
				if (isset($bc_parents[$link_info['minorid']])) {
					$bc_folder = $GLOBALS['SQ_SYSTEM']->am->getLinks($link_info['minorid'], SQ_LINK_TYPE_2, 'bodycopy');
					foreach ($bc_folder as $bc_link_info) {
						if (isset($parents[$bc_link_info['minorid']])) {
							$bodycopies[] = $bc_link_info['value'];
						}//end if
					}//end foreach
				}//end if
			}//end foreach

			if (empty($bodycopies)) {
				parent::onRequestKeywords($broadcaster, $vars);
				return;
			}//end if
		}

		foreach ($bodycopies as $bodycopy) {
			if (!isset($this->keywords[$bodycopy])) {
				// If bodycopy not found, just assume the default type format
				$bodycopy = 'default_format';
			}//end if
			foreach ($this->keywords[$bodycopy] as $keyword) {
				$keywords[$keyword] = ucwords(str_replace('_', ' ', $keyword));
			}//end foreach
		}//end foreach

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


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
	* @param array	$search_results	the results returned
	* @param string $bc_name		the name of the bodycopy to be printed (results | no_results)
	*
	* @return array
	* @access private
	*/
	function getResultsBodyReplacements($search_results, $bc_name='results')
	{
		$results = array_get_index($search_results, 'results', Array());

		$mode = htmlspecialchars(array_get_index($_REQUEST, 'mode', 'results'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
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
	* @param string	$type			the type of fluster (USED only for flustering)
	*
	* @return string
	* @access public
	*/
	function getFunnelbackReplacement($keyword, $search_results=Array(), $index=NULL, $type='site')
	{
		$default_fluster = Array('category' => Array('type' => Array(), 'topic' => Array(), 'site' => Array()));
		$default_summary = Array('summary' => Array(), 'fluster' => $default_fluster, 'spell' => Array(), 'featured' => Array());
		$results = array_get_index($search_results, 'results', Array());
		$summary = array_get_index($search_results, 'summary', $default_summary);
		$params  = array_get_index($search_results, 'params', Array());

		$prefix = $this->getPrefix();
		$mode = htmlspecialchars(array_get_index($_REQUEST, 'mode', 'results'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

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
		$keyword = parse_keyword($keyword, $modifiers);

		$replacement = '%'.$keyword.'%';
		if (strpos($keyword, 'page_list') === 0) {
			// Page list keyword
			$show_pages = substr($keyword, 9);
			$show_pages = ltrim($show_pages, '_');
			if (empty($show_pages) || $show_pages > $total_pages) {
				$show_pages = $total_pages;
			}//end if
			$replacement = $this->_paintSlidingPageNavigation($mode, $current_result_page, $total_pages, $show_pages, $num_per_page, $queries);
		} else if (strpos($keyword, 'result_meta') === 0) {
			// Metadata keywords
			$metadata_class = substr($keyword, 12, 1);
			$replacement    = '';
			if ($metadata_class !== 'S') {
				$metadata_field = 'meta_'.$metadata_class;
				$replacement    = array_get_index(array_get_index($results, $index, Array()), $metadata_field, '');
			}
		} else {
			// Everything else
			switch ($keyword) {
				case 'result_list':
					// print the results
					ob_start();
						$this->_printResultList($search_results);
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'fluster_category_type':
					// print the type category
					$fluster_category_type = array_get_index($category, 'type', Array());
					if (!empty($summary['fluster']) && !empty($fluster_category_type)) {
						ob_start();
							$this->printFlusterList($search_results, 'type');
							$replacement = ob_get_contents();
						ob_end_clean();
					}//end if
				break;

				case 'fluster_category_topic':
					// print the topic category
					$fluster_category_type = array_get_index($category, 'topic', Array());
					if (!empty($summary['fluster']) && !empty($fluster_category_type)) {
						ob_start();
							$this->printFlusterList($search_results, 'topic');
							$replacement = ob_get_contents();
						ob_end_clean();
					}//end if
				break;

				case 'fluster_category_site':
					// print the site category
					$fluster_category_type = array_get_index($category, 'site', Array());
					if (!empty($summary['fluster']) && !empty($fluster_category_type)) {
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
						$replacement = htmlspecialchars($this->attr('prev_page_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					} else {
						$replacement = '<a href="'.$this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page - 1).'&results_per_page='.$num_per_page.$queries).'">'.htmlspecialchars($this->attr('prev_page_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</a>';
					}//end if
				break;

				case 'next_result_page_link':
					if ($current_result_page >= $total_pages) {
						$replacement = htmlspecialchars($this->attr('next_page_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					} else {
						$replacement = '<a href="'.$this->getSearchURL().'?'.htmlspecialchars('mode='.$mode.'&current_result_page='.($current_result_page + 1).'&results_per_page='.$num_per_page.$queries).'">'.htmlspecialchars($this->attr('next_page_text'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</a>';
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

				case 'result_rank':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'rank', '');
				break;

				case 'result_score':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'score', '');
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

				case 'result_date':
					$replacement = array_get_index(array_get_index($results, $index, Array()), 'date', '');
				break;

				case 'result_url_click_event':
					$replacement = '';
					$field_name = $prefix.'_search_query';
					$query = htmlspecialchars(array_get_index($_REQUEST, $field_name, ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					$rank = array_get_index(array_get_index($results, $index, Array()), 'rank', '');
					$url = array_get_index(array_get_index($results, $index, Array()), 'live_url', '');
					if (!empty($url)) {
						$replacement = $this->_getClickLoggingJavascript($query, $rank, $url);
					}//end if
				break;

				case 'result_url_click_js':
					$replacement = '';
					$field_name = $prefix.'_search_query';
					$query = htmlspecialchars(array_get_index($_REQUEST, $field_name, ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					$rank = array_get_index(array_get_index($results, $index, Array()), 'rank', '');
					$url = array_get_index(array_get_index($results, $index, Array()), 'live_url', '');
					if (!empty($url)) {
						$replacement = $this->_getClickLoggingJavascript($query, $rank, $url, FALSE);
					}//end if
				break;

				case 'result_name_linked':
					$replacement = '';
					$field_name = $prefix.'_search_query';
					$query = htmlspecialchars(array_get_index($_REQUEST, $field_name, ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					$rank = array_get_index(array_get_index($results, $index, Array()), 'rank', '');
					$title = array_get_index(array_get_index($results, $index, Array()), 'title', '');
					$url = array_get_index(array_get_index($results, $index, Array()), 'live_url', '');
					if (!empty($title) && !empty($url)) {
						$js = $this->_getClickLoggingJavascript($query, $rank, $url);
						$replacement = '<a href="'.$url.'" '.$js.'>'.$title.'</a>';
					}//end if
				break;

				case 'result_cache_url':
					$search_type = $this->attr('search_type');
					$replacement = '';
					if ($search_type == 'web') {
						// Cache URL only works on 'web' mode
						$replacement = array_get_index(array_get_index($results, $index, Array()), 'cache_url', '');
					}//end if
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
							if ($this->attr('show_fluster_more')) {
								if (isset($category[$type]['more']) && $category[$type]['more']) {
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
										if ($query_name == 'current_result_page') continue;
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
									$url = $this->getSearchUrl().'?'.htmlspecialchars($queries.'&'.$cluster_query, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

									// Here we are to print the more... flag
									echo '<a href="'.$url.'">'.htmlspecialchars($cluster_name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</a>';
								} else {
									// If the show more is has been set (max cluster) show less
									$queries = '';
									$show_link = FALSE;
									$query_cluster = $type.'_max_clusters';
									$max_attr = 'more_'.$type.'_num';
									$max_clusters = $this->attr($max_attr);
									$cluster_query = $this->getPrefix().'_'.$query_cluster.'='.$max_clusters;
									$cluster_name = $this->attr('less_'.$type.'_text');
									foreach ($_GET as $query_name => $query_value) {
										if ($query_name == 'current_result_page') continue;
										if (is_array($query_value)) {
											foreach ($query_value as $value_index => $value_contents) {
												$queries .= '&'.$query_name.'['.$value_index.']='.$value_contents;
											}//end foreach
										} else {
											$current_query = $query_name.'='.$query_value;
											if ($current_query != $cluster_query) {
												$queries .= '&'.$current_query;
											} else {
												$show_link = TRUE;
											}//end if
										}//end if
									}//end foreach

									// Build the url to use
									if (strpos($queries, '&') === 0) {
										$queries = substr($queries, 1);
									}//end if
									$url = $this->getSearchUrl().'?'.htmlspecialchars($queries, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

									// Here we are to print the more... flag
									if ($show_link) {
										echo '<a href="'.$url.'">'.htmlspecialchars($cluster_name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</a>';
									}//end if
								}//end if
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
					$replacement = '';
					$cluster_url = '';
					$url = array_get_index(array_get_index($category_type, $index, Array()), 'href', '');
					$query = array_get_index(array_get_index($category_type, $index, Array()), 'query', '');
					if (!empty($url)) {
						$parts = $this->_getQueryVars($url);
						$queries = Array();
						$query_found = FALSE;
						foreach ($parts as $switch => $param) {
							if (strpos($switch, 'query') === 0) {
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
						if (!empty($queries)) {
							$get_vars = '';
							foreach ($queries as $switch => $param) {
								$get_vars .= '&'.$switch.'='.$param;
							}//end foreach

							$get_vars = substr($get_vars, 1);
							$get_vars = htmlspecialchars($get_vars, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
							$cluster_url = $this->getSearchURL().'?'.$get_vars;
						}//end if
					} else if (!empty($query)) {
						$queries = Array();
						$query_found = FALSE;
						$url = array_get_index($_SERVER, 'QUERY_STRING', '');
						$parts = $this->_getQueryVars($url);
						foreach ($parts as $switch => $param) {
							if ($switch == $this->getPrefix().'_search_query') {
								$queries[$this->getPrefix().'_search_query'] = $query;
								$query_found = TRUE;
							} else {
								$queries[$switch] = $param;
							}//end if
						}//end foreach

						// Only add in the extra when a query is found
						if ($query_found) {
							$queries[$this->getPrefix().'_submit_button'] = $this->attr('submit_button_text');
							$queries['mode'] = 'results';
						}//end if

						// Build the URL
						if (!empty($queries)) {
							$get_vars = '';
							foreach ($queries as $switch => $param) {
								if ($switch == 'current_result_page') continue;
								$get_vars .= '&'.$switch.'='.$param;
							}//end foreach

							$get_vars = substr($get_vars, 1);
							$get_vars = htmlspecialchars($get_vars, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
							$cluster_url = $this->getSearchURL().'?'.$get_vars;
						}//end if
					}//end if
					$replacement = $cluster_url;
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

				default:
					$replacement = $this->getGeneralReplacement($keyword, $params, $summary['spell']);
				break;
			}//end switch
		}//end if

		apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));

		return $replacement;

	}//end getFunnelbackReplacement()


	/**
	* Returns replacements that that are common to the bodycopies
	*
	* @param string $keyword	the keyword of the wanted replacement
	* @param array	$extra		the extra information that can be used by the initial bodycopy
	* @param array	$spelling	the extra spelling information
	*
	* @return string
	* @access public
	*/
	public function getGeneralReplacement($keyword, $extra, $spelling=Array())
	{
		$prefix = $this->getPrefix();
		$replacement = '';
		if (strpos($keyword, 'metadata_scope') === 0) {
			$class = str_replace('metadata_scope_', '', $keyword);
			$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
			$all_class = $fm->getMetadataAliases();
			if (array_key_exists($class, $all_class)) {
				// Print the interface if a valid class
				$current_class = '';
				$all_current_class = array_get_index($_GET, $prefix.'_metadata_scope', Array());
				if (isset($all_current_class[$class])) $current_class = $all_current_class[$class];
				ob_start();
					text_box($prefix.'_metadata_scope['.$class.']', $current_class);
					$replacement = ob_get_contents();
				ob_end_clean();
			}//end if
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($fm, TRUE);
		} else if (strpos($keyword, 'metadata_logic') === 0) {
			$class = str_replace('metadata_logic_', '', $keyword);
			$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
			$all_metadata_config = array_get_index($fm->attr('metamap'), $this->attr('collection'), Array());
			$metadata_config = Array();
			foreach($all_metadata_config as $config) {
				if (isset($config['alias']) && $config['alias'] == $class) {
                    $metadata_config = $config;
                    break;
                }
			}
			$all_class = $fm->getMetadataAliases();
			if (array_key_exists($class, $all_class)) {
				// Print the interface if a valid class
				$current_logic = array_get_index($metadata_config, 'word_logic', 2);
				$all_current_logic = array_get_index($_GET, $prefix.'_metadata_logic', Array());
				if (isset($all_current_logic[$class])) $current_logic = $all_current_logic[$class];
				$logic_options = Array('Exclude', 'Include any', 'Include all');
				ob_start();
					combo_box($prefix.'_metadata_logic['.$class.']', $logic_options, FALSE, $current_logic);
					$replacement = ob_get_contents();
				ob_end_clean();
			}//end if
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($fm, TRUE);
		} else {
			switch ($keyword) {

				case 'current_query_term':
					$field_name = $prefix.'_search_query';
					$current_value = htmlspecialchars(array_get_index($_REQUEST, $field_name, ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

					$replacement = $current_value;
				break;

				case 'search_query':
					$field_name = $prefix.'_search_query';
					$current_value = htmlspecialchars(array_get_index($_REQUEST, $field_name, ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					$dynamic_query = array_get_index($extra, 'query', '');
					if (!empty($dynamic_query) && empty($current_value)) {
						$current_value = $dynamic_query;
					}//end if

					if($this->attr('disable_keyword_replacements') && !empty($current_value)) {
						$current_value = $this->_sanitizeQueries($current_value);
					}

					$this->registerFormField($field_name);
					$replacement = '<input type="text" name="'.$field_name.'" id="'.$field_name.'" value="'.$current_value.'" />';
				break;

				case 'search_logic':
					$current_sort_logic = array_get_index($_GET, $prefix.'_search_logic', $this->attr('word_logic'));
					$logic_options = Array('Exclude' ,'Include any', 'Include all');
					ob_start();
						combo_box($this->getPrefix().'_search_logic', $logic_options, FALSE, $current_sort_logic);
						$replacement = ob_get_contents();
					ob_end_clean();
				break;

				case 'search_scope':
					$field_name = $prefix.'_search_scope';
					$scope = $this->attr('scope');
					$current_value = htmlspecialchars(array_get_index($_REQUEST, $field_name, ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
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

				case 'search_sort':
					$sort_options = $this->getSortOptions();
					$current_sort = $this->attr('sort_order');
					if (isset($_REQUEST[$this->getPrefix().'_search_sort'])) {
						// There is an override for the current sort
						$current_sort = htmlspecialchars($_REQUEST[$this->getPrefix().'_search_sort'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					}//end if
					ob_start();
						combo_box($this->getPrefix().'_search_sort', $sort_options, FALSE, $current_sort);
						$replacement = ob_get_contents();
					ob_end_clean();
					$this->registerFormField($this->getPrefix().'_search_sort');
				break;

				case 'spell_suggestion':
					// print the spelling suggestion
					if (!empty($spelling)) {
						// Check for same spelling and dont print it
						$current_term = htmlspecialchars(array_get_index($_REQUEST, $this->getPrefix().'_search_query', ''), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
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

				case 'spell_url':
					$url = '';
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
					$replacement = array_get_index($spelling, 'text', '');
				break;

				case 'search_button':
				case 'submit_button':
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
					$mode = htmlspecialchars(array_get_index($_REQUEST, 'mode', 'initial'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					if ($mode == 'initial') {
						$replacement  = '<input type="submit" name="'.$field_name.'"
							value="'.$this->attr('submit_button_text').'" />';
					} else {
						$replacement  = '<input type="submit" name="'.$field_name.'" onclick="listing_form = document.getElementById(\''.$this->getPrefix().'\');
							if (typeof listing_form.current_result_page != \'undefined\' ) listing_form.current_result_page.value=\'1\';
							if (typeof listing_form.mode != \'undefined\') listing_form.mode.value = \'results\';
							return true;
						" value="'.$this->attr('submit_button_text').'" />';
					}
					// We add an extra hidden text field here to workaround an IE bug i.e.
					// if form has only one text field then submit button is not sent along 
					// with other input fields in form when enter key is pressed in the text box.
					// See bug #4559
					$replacement = '<input type="text" name="'.$prefix.'_extra_field" id="'.$prefix.'_extra_field" style="display:none" />'.$replacement;			
				break;

				case 'results_per_page':
					
					$field_name = 'results_per_page';
					$results_per_page = htmlspecialchars(array_get_index($_REQUEST, $field_name, $this->_getNumPerPage()), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					$this->registerFormField($field_name);
					$replacement = '<input type="text" name="'.$field_name.'" onchange="listing_form = document.getElementById(\''.$this->getPrefix().'\');
						listing_form.results_per_page.value = this.value;
					" value="'.$results_per_page.'" size="5" />';
				break;

				default:
					$replacement = $this->getKeywordReplacement($keyword);
				break;
			}//end switch
		}//end if

		return $replacement;

	}//end getGeneralReplacement()


	/**
	 * Return the options for sort order
	 *
	 * @return array
	 * @access public
	 */
	public function getSortOptions()
	{
		return Array(
					'rank'	=> translate('funnelback_sort_rank'),
					'date'	=> translate('funnelback_sort_date'),
					'title'	=> translate('funnelback_sort_title'),
					'url'	=> translate('funnelback_sort_url'),
			   );

	}//end getSortOptions()


	/**
	* Returns replacements for the sliding page index 
	*
	* @param string	$mode					The mode of the page
	* @param int	$current_result_page	The current result page
	* @param int	$total_pages			The total number of pages
	* @param int	$show_pages				The number of results shown
	* @param int	$num_per_page			The number of results per page
	* @param string	$queries				Any extra queries
	*
	* @return string
	* @access private
	*/
	private function _paintSlidingPageNavigation($mode, $current_result_page, $total_pages, $show_pages, $num_per_page, $queries='')
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
	* @access public
	* @see Search_Page::bodycopies
	*/
	public function &getBodycopy($name)
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
	* @access public
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	*/
	public function &getBodycopyContents($name, $replacements=Array())
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
	public function getFormatFolderLink()
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
	public function &getFormatFolder()
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
	public function getFormatBodycopyLink($link_value)
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
	public function &getFormatBodycopy($link_value)
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
	* @param array	$search_results	The search results
	*
	* @return void
	* @access private
	*/
	private function _printResultList($search_results)
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
	* @param string $index				The assetid of the asset we are printing
	*
	* @return void
	* @access private
	*/
	private function _printResult($search_results, $index)
	{
		$results = array_get_index($search_results, 'results', Array());

		// Setting the Default Type Folder
		if (!isset($this->_tmp['type_folder_id'])) {
			$type_folder = $this->getFolder('type_format');
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
	* @access public
	*/
	public function printFeaturedList($results, $index)
	{
		// Setting the Default Type Folder
		if (!isset($this->_tmp['type_folder_id'])) {
			$type_folder = $this->getFolder('type_format');
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
	* @access public
	*/
	public function printFlusterList($results, $type='site')
	{
		// Getting the Fluster Format folder
		if (!isset($this->_tmp['fluster_folder_id'])) {
			$fluster_folder = $this->getFolder('fluster_format');
			$this->_tmp['fluster_folder_id'] = $fluster_folder->id;
		}//end if

		// Getting the Fluster Default Format Bodycopy
		if (!isset($this->_tmp['fluster_format_bc'])) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->_tmp['fluster_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'context_format');
			if ($link) {
				$this->_tmp['fluster_format_bc'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
			} else {
				trigger_localised_error('FNB0004', E_USER_WARNING, 'Context');
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
	* @access public
	*/
	public function printClusterList($results, $index, $type='site')
	{
		// Getting the Cluster Format folder
		if (!isset($this->_tmp['fluster_folder_id'])) {
			$fluster_folder = $this->getFolder('fluster_format');
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
	* @access public
	*/
	public function printNavigationList($results, $index)
	{
		// Getting the Fluster Format folder
		if (!isset($this->_tmp['fluster_folder_id'])) {
			$fluster_folder = $this->getFolder('fluster_format');
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
	* @access public
	*/
	public function printSpellList($results)
	{
		// Getting the Spell Formats folder
		if (!isset($this->_tmp['spell_folder_id'])) {
			$spell_folder = $this->getFolder('spell_format');
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
			$keywords[$keyword] = $this->getGeneralReplacement($keyword, Array(), $results);
		}//end foreach

		$bodycopy->setKeywordReplacements($keywords);
		$bodycopy->printBody();

	}//end printSpellList()


// --- PRIVATE FUNCTIONS --- //


	/**
	 * Return an array of the query vars
	 *
	 * @param string	$query_string	The string of query vars
	 *
	 * @return array
	 * @access private
	 */
	private function _getQueryVars($query_string)
	{
		// get rid of any leadin '?' if any
		$query_string = ltrim($query_string, '?');

		// The regex expression expects a start & so let us do that first
		if (strpos($query_string, '&') !== 0) {
			$query_string = '&'.$query_string;
		}//end if

		$vars = Array();
		preg_match_all('|&([^=]+)=([^&]+)?|', $query_string, $matches);
		foreach ($matches[1] as $index => $query_name) {
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
	* @access public
	*/
	public function parseURL($search)
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
		$valid_sort = Array ('date' , 'adate' , 'title' , 'dtitle' , 'size' , 'dsize' , 'url' , 'durl' , 'coll' , 'dcoll' , 'shuffle');	
		$sort = array_get_index($search, 'sort', 'rank');

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

		
		// Add in sorting
		if (!empty($sort) && $sort !== 'rank' && in_array($sort, $valid_sort)) {
			$url .= '&sort='.$sort;
		}

		// get the current user and also the assetid
		// of all the groups the user is in
		$user_info = Array($GLOBALS['SQ_SYSTEM']->user->id);
		$user_info = array_merge($GLOBALS['SQ_SYSTEM']->user->getUserGroups(), $user_info);

		$url .= '&userkeys='.implode(',', $user_info);

		return $funnelback_url.$url;

	}//end parseURL()


	/**
	* Connect to the funnelback server and returns the XML from the server
	*
	* @param string $url	The URL to connect to
	*
	* @return string
	* @access private
	*/
	private function _queryFunnelback($url)
	{
		if (empty($url)) {
			trigger_localised_error('FNB0003', E_USER_WARNING);
			return FALSE;
		}//end if

		$url_ok = @parse_url($url);
		if (!$url_ok) {
			trigger_localised_error('FNB0003', E_USER_WARNING);
			return FALSE;
		}//end if

		$options = array(
					'TIMEOUT'			=> 5,
					'CONNECTTIMEOUT'	=> 5,
					'MAXREDIRS'			=> 2,
					'FOLLOWLOCATION'	=> true,
					'RETURNTRANSFER'	=> true,
				   );

        $headers = Array(
					'X-Forwarded-For'	=> $_SERVER['REMOTE_ADDR'],
				   );

		$urlResponse = fetch_url($url, $options, $headers);
		if ($urlResponse['errornumber'] != 0) {
			trigger_localised_error('FNB0002', E_USER_WARNING);
			return FALSE;
		}

		$xml  = $urlResponse['response'];
		$code = $urlResponse['curlinfo']['http_code'];

		if (empty($xml) || empty($code) || $code != '200') {
			return FALSE;
		}//end if

		return $xml;

	}//end _queryFunnelback()


	/**
	* Search the OEM version of Funnelback 
	*
	* @param array $params	The search params
	*
	* @return string
	* @access private
	*/
	private function _searchFunnelback($params)
	{
		$output = '';

		// Wha..? No params no search
		if (empty($params)) {
			return FALSE;
		}//end if

		// Get the params to work with
		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
		$nullquery = $fm->getNullQueryKeyword();
		$collection_id = $this->attr('collection');
		$bin_path = $fm->getFunnelbackPath().'/bin/';
		if (!$fm->isCollectionValid($collection_id)) {
			
			// Collection is not valid, stop the search...
			return FALSE;
		}//end if
		$coll_path = $fm->getCollectionDataPath($collection_id);
		$data_path = $fm->getCollectionDataPath($collection_id, 'idx');
		$log_file = $fm->getCollectionDataPath($collection_id, 'log').'/query.log';
		$thesaurus_config = $fm->getCollectionDataPath($collection_id, 'conf').'/synonyms.cfg';
		$search_term = array_get_index($params, 'query', '');
		$scope = array_get_index($params, 'scope', '');
		$page = array_get_index($params, 'page', 1);
		$num_per_page = intval(array_get_index($params, 'num_per_page', $this->_getNumPerPage()));
		
		$sort = array_get_index($params, 'sort', $this->attr('sort_order'));
		$metadata_scope = $this->_getMetadataScope();
		if (empty($scope)) {
			$scope = $this->attr('default_scope');
		}//end if

		// Build the command line based on params set above
		$command  = 'echo ';
		$query = $this->_getSearchQuery($search_term);
		if (empty($query)) {
			if ($this->attr('allow_empty')) {
				// 'NULL' query (basically use a string with no meaning and use ! - which match everything else)
				$query = $nullquery;
				$fluster = '';	// Flustering is disabled on a NULL query
			} else {
				// Empty searches are not permitted
				return $output;
			}//end if
		} else {
			// Enable flustering
			$fluster = $this->_getFlusterOptions($params);
		}//end if
		$command .= escapeshellarg($query.' '.$metadata_scope);
		$command .= ' | '.$bin_path.'padre-sw';				// Pipe the query to the search command (padre-sw)
		$command .= ' '.escapeshellarg($collection_id);		// Index Stem aka Collection
		$command .= ' -res xml -SMboth -SF'.$this->_getMetadataFields().' -spelling_enabled -qlog_file='.$log_file.$fluster;
		$command .= ' -userkeys="'.implode(',', $this->_getUserKeys()).'"';
		if (!empty($scope) && $scope != 'ALL') {
			$command .= ' -scope '.escapeshellarg($scope);
		}//end if
		if ($page != 1) {
			$command .= ' -start_rank'.((($page-1) * $num_per_page) + 1);
		}//end if
		if (!empty($num_per_page)) {
			$command .= ' -num_ranks'.$num_per_page;
		}//end if
		switch($sort) {
			case 'date':
			case 'title':
			case 'url':
				$command .= ' -sort '.escapeshellarg($sort);
			break;

			case 'rank':
			default:
				// Default to rank, already handled by default by Funnelback
			break;
		}//end switch

		// Searching... Searching... Searching...
		$status = $fm->runCommand($command, $output, NULL, NULL, $data_path, Array('SITE_SEARCH_ROOT' => $coll_path));

		// Make sure the 'NULL' query doesn't come through in the results
		$output = str_replace($nullquery, '', $output);

		return $output;

	}//end _searchFunnelback()


	/**
	* Connect to the SquizSearch server and returns the XML from the server
	*
	* @param array $search	The search params
	*
	* @return string
	* @access private
	*/
	private function _querySquizSearch($search)
	{
		$systemid = $this->attr('systemid');

		// Get the query term
		$query = array_get_index($search, 'query', '');
		$query = str_replace('&', '', $query);

		// Get the collection
		$collection = $this->attr('collection');

		// Get the other vars
		$current_result_page = array_get_index($search, 'page', 1);
		$num_per_page = array_get_index($search, 'num_per_page', $this->_getNumPerPage());
		$valid_sort = Array ('date' , 'adate' , 'title' , 'dtitle' , 'size' , 'dsize' , 'url' , 'durl' , 'coll' , 'dcoll' , 'shuffle');	
		$sort = array_get_index($search, 'sort', 'rank');
		$scope = array_get_index($search, 'scope', '');
		$type_max = array_get_index($search, 'type_max', 0);
		$topic_max = array_get_index($search, 'topic_max', 0);
		$site_max = array_get_index($search, 'site_max', 0);

		// Build the options.
		$options = array();
		$options['num_per_page']                  = $num_per_page;
		$options['contextual_navigation_enabled'] = TRUE;
		$options['userkeys']                      = $this->_getUserKeys(TRUE);
		
		// Check for required information
		if (empty($systemid) || empty($collection)) {
			// No server set
			trigger_localised_error('FNB0003', E_USER_WARNING);
			return '';
		}//end if

		// Add in the scope
		if (empty($scope)) {
			$scope = $this->attr('default_scope');
		}//end if
		if (!empty($scope) && $scope != 'ALL') {
			$options['scope'] = $scope;
		}//end if		

		// Add in the paging commands
		if ($current_result_page != 1) {
			$options['start_at'] = ((($current_result_page-1) * $num_per_page) + 1);
		}//end if

		// Fluster more/less links
		if (!empty($type_max)) {
			$options['type_max_clusters'] = $type_max;
		} else {
			$options['type_max_clusters'] = $this->attr('less_type_num');
		}//end if
		if (!empty($topic_max)) {
			$options['topic_max_clusters'] = $topic_max;
		} else {
			$options['topic_max_clusters'] = $this->attr('less_topic_num');
		}//end if
		if (!empty($site_max)) {
			$options['site_max_clusters'] = $site_max;
		} else {
			$options['site_max_clusters'] = $this->attr('less_site_num');
		}//end if

		// Add in sorting
		if (!empty($sort) && $sort !== 'rank' && in_array($sort, $valid_sort)) {
			$options['sort'] = $sort;
		}

		$suitem = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		$system = $suitem->getProduct($systemid);
		$xml    = '';

		$request = array(
					'query' 	   => $suitem->encodeJson($query),
					'collectionid' => $collection,
					'options'	   => $suitem->encodeJson($options),
				   );
		if (!empty($system)) {
			$response   = $suitem->sendRequest($systemid, 'json', 'SquizSearch', 'Search', $request);
			if (isset($response['curlInfo']['http_code']) === TRUE && $response['curlInfo']['http_code'] == 200) {
				$resultData = $response['result'];
				$resultData = (array) $suitem->decodeJson($resultData);
				$xml        = (string) $resultData['result'];
			}//end if
		}//end if

        // Services_JSON will forcibly change Content-type header to json, which is not expected
        if (!headers_sent()) {
            header('Content-type: text/html;');
        }

		return $xml;

	}//end _querySquizSearch()


	/**
	 * Get the user keys based on the current logged in user
	 *
	 * @param boolean $useSuiteSystemId Use the Suite System ID in the response keys.
	 *
	 * @return array
	 * @access private
	 */
	private function _getUserKeys($useSuiteSystemId=FALSE)
	{
		// Default to the public user keys
		$keys    = array("'public'");
		$user_id = $GLOBALS['SQ_SYSTEM']->currentUserId();
		if (!empty($user_id)) {
			// Return if public user
			$public_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetId('public_user');
			if ($user_id == $public_user) return $keys;

			// Collate the user and his/her groups
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_id);
			if (!is_null($user)) {
                $prefix = '';
                if ($useSuiteSystemId == TRUE) {
                    $sm      = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
                    $current = $sm->getCurrentProduct();
                    $prefix  = array_get_index($current, 'systemid', '');
                    if (!empty($prefix)) $prefix .= '://';
                }//end if

				$fm     = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
				$keys   = Array($user->id, $public_user, 'public');
				$groups = $user->getUserGroups();
				$keys   = array_merge($keys, $groups);
				sort($keys);

				// Nasty.... but essential
				foreach ($keys as $index => $key) {
					if ($key === 'public') continue;
					$keys[$index] = "'".$prefix.$fm->encodeText((string) $key)."'";
				}//end foreach

				$userkeys = ' -userkeys="'.implode(',', $keys).'"';
			}//end if
		}//end if

		return $keys;

	}//end _getUserKeys()


	/**
	 * Get the flustering options 
	 *
	 * @param array	$params	The search parameters
	 *
	 * @return string
	 * @access private
	 */
	private function _getFlusterOptions($params)
	{
		$fluster = ' -contextual_navigation_enabled';
		// $cluster = array_get_index($params, 'cluster', Array());
		// $stem = array_get_index($params, 'stem', 0);
		$type_max = array_get_index($params, 'type_max', 0);
		$topic_max = array_get_index($params, 'topic_max', 0);
		$site_max = array_get_index($params, 'site_max', 0);

		if (!empty($type_max)) {
			$fluster .= ' -type.max_clusters='.(int) $type_max;
		} else {
			$fluster .= ' -type.max_clusters='.(int) $this->attr('less_type_num');
		}//end if

		if (!empty($topic_max)) {
			$fluster .= ' -topic.max_clusters='.(int) $topic_max;
		} else {
			$fluster .= ' -topic.max_clusters='.(int) $this->attr('less_topic_num');
		}//end if

		if (!empty($site_max)) {
			$fluster .= ' -site.max_clusters='.(int) $site_max;
		} else {
			$fluster .= ' -site.max_clusters='.(int) $this->attr('less_site_num');
		}//end if

		return $fluster;

	}//end _getFlusterOptions()


	/**
	 * Get the metadata fields.
	 *
	 * @return string
	 * @access private
	 */
	private function _getMetadataFields()
	{
			$scope = '';

			$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
			$all_metadata_config = array_get_index($fm->attr('metamap'), $this->attr('collection'), Array());

			$fields = Array();
			foreach($all_metadata_config as $metamap) {
				if ($metamap['alias'] !== 'S') {
					$fields[] = $metamap['alias'];
				}
			}

			$fields = array_unique($fields);

			return implode($fields);

	}//end _getMetadataFields()



	/**
	 * Get the metadata scoping from the URL
	 *
	 * @return string
	 * @access private
	 */
	private function _getMetadataScope()
	{	
		$group_scope = '';
		$scope = '';

		$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
		$all_metadata_config = array_get_index($fm->attr('metamap'), $this->attr('collection'), Array());
		$metadata_scope = array_get_index($_GET, $this->getPrefix().'_metadata_scope', Array());
		
		$default_metadata_logic = Array();
		foreach($all_metadata_config as $metamap) {
			if (array_get_index($metadata_scope, $metamap['alias'], FALSE) !== FALSE) {
				$default_metadata_logic[$metamap['alias']] = $metamap['word_logic'];
			}
		}
		$metadata_logic = array_get_index($_GET, $this->getPrefix().'_metadata_logic', $default_metadata_logic);
		foreach ($metadata_scope as $index => $metadata_terms) {
			$keywords = preg_split('/([\s\+\-\|\*]*"[^"]+")[\s]*|'.'([\s\+\-\|\*]*`[^`]+`)[\s]*|'.'[\s]+/', $metadata_terms, 0, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
			// The query having search operator overrides the default word logic settings
			// NOTE: A search operator inside double quotes is part of the search keyword and does not overrides the word logic
			$contains_op = FALSE;
			foreach($keywords as $keyword) {
				if (preg_match('/^\s*[\+\-\|\*]\s*/', $keyword)) {
					$contains_op = TRUE;
				}
			}
			$logic = array_get_index($metadata_logic, $index, array_get_index($default_metadata_logic, $index, 2));
			foreach($keywords as $value) {			
				$value = trim($value);
				if (!empty($value)) {
					if ($contains_op) {
						$scope .= ' '.preg_replace('/^([\+\-\|\*]?)'.'([^\+\-\|\*]+)'.'|(".+?")|(`.+?`)$/','$1'.$index.':$2', $value);
					} else if ($logic == '0') {
            	        $scope .= ' -'.$index.':'.$value;
        	        } else if ($logic == '1') {
    	                $group_scope .= ' '.$index.':'.$value;
	                } else {
                    	$scope .= ' +'.$index.':'.$value;
                	}				
				}//end if
			}//end foreach $keywords
		}//end foreach

		$scope = trim($group_scope) ? '|['.trim($group_scope).']'.trim($scope) : trim($scope);

		return $scope;

	}//end _getMetadataScope()


	/**
	 * Get the search query 
	 *
	 * @parm string $search_term
	 *
	 * @return string
	 * @access private
	 */
	private function _getSearchQuery($search_term)
	{	
		$keywords = preg_split('/([\s\+\-\|\*]*"[^"]+")[\s]*|'.'([\s\+\-\|\*]*`[^`]+`)[\s]*|'.'[\s]+/', $search_term, 0, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);

		// Query having search operator overrides the word logic settings
		foreach($keywords as $keyword) {
			if (preg_match('/^\s*[\+\-\|\*]\s*/', $keyword)) {
				return $search_term;
			}
		}

		$logic = array_get_index($_GET, $this->getPrefix().'_search_logic', $this->attr('word_logic'));
		
		$search_query = '';
		foreach ($keywords as $keyword) {
			$keyword = trim($keyword);

			if (!empty($keyword)) {
                if ($logic == '0') {
                    $search_query .= ' -'.$keyword;
                } else if ($logic == '1') {
                    $search_query .= ' '.$keyword;
                } else {
                    $search_query .= ' +'.$keyword;
                }				
			}//end if
		}//end foreach
		
		return trim($search_query);

	}//end _getSearchQuery()

	
	/**
	* Output the event javascript
	*
	* @param string 	$query	The query the javascript should consume
	* @param string 	$rank	The rank the javascript should consume
	* @param string 	$url	The URL the javascript should consume
	* @param boolean 	$event	Include the onclick event
	*
	* @return string
	* @access private
	*/
	private function _getClickLoggingJavascript($query, $rank, $url, $event=TRUE)
	{
		// At minimum, the click logging program needs query, rank and url
		if (empty($query)) return '';
		if (empty($rank)) return '';
		if (empty($url)) return '';

		$click_event  = '';
		$click_event .= (($event) ? 'onclick="' : '');
		$click_event .= "FunnelbackSearch.clickLog('".$query."','".$rank."','".$url."'); return true;";
		$click_event .= (($event) ? '"' : '');

		return $click_event;

	}//end _getClickLoggingJavascript()


	/**
	* Output the search page javascript
	*
	* @return string
	* @access private
	*/
	private function _getSearchPageJavascript()
	{
		$collection = $this->attr('collection');
		ob_start();
			?>
<script type="text/javascript">
	var FunnelbackSearch = function() {

		return {
				clickLog : function(query,rank,url) {
					if (window.XMLHttpRequest) {
						var xhr = new window.XMLHttpRequest();
					} else {
						var xhr = new ActiveXObject("Microsoft.XMLHTTP");
					}

					var getURL = '<?php echo current_url(); ?>?click=1&click_query='+query+'&click_rank='+rank+'&click_url='+url+'&click_collection=<?php echo $collection; ?>';
					xhr.open("GET", getURL, true);
					xhr.send(null);
				}
		};
	}();
</script>
			<?php
			$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end _getSearchPageJavascript()


	/**
	* Strip URL ( this de-funnelbacks urls ) ;) 
	*
	* @param string $url	The URL to strip
	*
	* @return string
	* @access public
	*/
	public function _stripURL($url)
	{
		return str_replace('/search/xml.cgi?', '', $url);

	}//end _stripURL()


	/**
	* Process the search summary information
	*
	* @param object &$xml	The simplexml object with the goods!
	*
	* @return array
	* @access public
	*/
	public function processSearchSummary(&$xml)
	{
		$default_fluster = Array('category' => Array('type' => Array(), 'topic' => Array(), 'site' => Array()));
		$results = Array('summary' => Array(), 'fluster' => $default_fluster, 'spell' => Array(), 'featured' => Array());

        $adv_hosted_search = FALSE;
        if ($this->attr('search_type') == 'hosted' && $this->attr('hosted_mode')) {
            $response = $xml->getElementsByTagName('response');
            $result_packet = $response->item(0)->getElementsByTagName('resultPacket');
            $adv_hosted_search = TRUE;
        }
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
        if ($adv_hosted_search) {
            $mapping_array = Array(
                                'fullyMatching'=> 'fully_matching',
                                'estimatedHits'=> 'estimated_hits',
                                'partiallyMatching'=> 'partially_matching',
                                'totalMatching'=> 'total_matching',
                                'numRanks'=> 'num_ranks',
                                'currStart'=> 'currstart',
                                'currEnd'=> 'currend',
                                'prevStart'=> 'prevstart',
                                'nextStart'=> 'nextstart',
                             );
            $result_summary = $result_packet->item(0)->getElementsByTagName('resultsSummary');
        } else {
    		$result_summary = $xml->getElementsByTagName('results_summary');
        }

		for ($i=0; $i<$result_summary->length; $i++) {
			if ($result_summary->item($i)->hasChildNodes()) {
				foreach ($result_summary->item($i)->childNodes as $summary) {
					$name = isset($mapping_array[$summary->nodeName]) ? $mapping_array[$summary->nodeName] : $summary->nodeName;
					$value = $summary->nodeValue;
					$results['summary'][$name] = (string) $value;
				}//end foreach
			}//end if
		}//end for


		/**
		 * 
		 * <best_bets>
		 * 	<bb>
		 * 		<bb_trigger>matrix</bb_trigger>
		 * 		<bb_link>http://matrix.squiz.net/download/mysource-matrix</bb_link>
		 * 		<bb_title>Download MySource Matrix</bb_title>
		 * 		<bb_desc></bb_desc>
		 * 	</bb>
		 * </best_bets>
		 */
        if ($adv_hosted_search) {
            $mapping_array = Array(
                                'trigger'=> 'bb_trigger',
                                'link'=> 'bb_link',
                                'title'=> 'bb_title',
                                'description'=> 'bb_desc',
                             );
            $feature_pages = $result_packet->item(0)->getElementsByTagName('bestBets');
        } else {
		    $feature_pages = $xml->getElementsByTagName('best_bets');
        }
		// before FB v9.1, we call it featured_pages
		if($feature_pages->length == 0) $feature_pages = $xml->getElementsByTagName('featured_pages');
		
		if ($feature_pages->length >= 1) {
            $tag_name = ($adv_hosted_search) ? 'com.funnelback.publicui.search.model.padre.BestBet' : 'bb';
			$featured_pages = $feature_pages->item(0)->getElementsByTagName('bb');
			// before FB v9.1, we call it fp
			if($featured_pages->length == 0) 
				$featured_pages = $feature_pages->item(0)->getElementsByTagName('fp');
			for ($j=0; $j<$featured_pages->length; $j++) {
				$bb = Array();
				if ($featured_pages->item($j)->hasChildNodes()) {
					foreach ($featured_pages->item($j)->childNodes as $entry) {
						$name = (isset($array_mapping[$entry->nodeName])) ? $array_mapping[$entry->nodeName] : $entry->nodeName;
						// before FB v9.1, we call it fp_xx, so to make sure it doesn't break existing keywords etc, we have to stick with old names.
						switch ($name) {
							case 'bb_trigger':
								$name = 'fp_trigger';
								break;
							case 'bb_link':
								$name = 'fp_link';
								break;	
							case 'bb_title':
								$name = 'fp_title';
								break;
							case 'bb_desc':
								$name = 'fp_desc';
								break;		
							default:
								break;
						}
						$value = $entry->nodeValue;
						if ($name != '#text') {
							$bb[$name] = $value;
						}//end if
					}//end for
				}//end if
				$results['featured'][] = $bb;
			}//end for
		}//end if

		/**
		 * <contextual_navigation>
		 *	 <cluster_nav level="" url=""></cluster_nav>
		 *	 <category name="type" more="">
		 *	  <cluster href="" count=""></cluster>
		 *	  <cluster href="" count=""></cluster>
		 *	 </category>
		 *	 <category name="topic" more="">
		 *	  <cluster href="" count=""></cluster>
		 *	  <cluster href="" count=""></cluster>
		 *		<more_link label="topic"></more_link>
		 *	 </category>
		 *	 <category name="site" more="">
		 *	  <cluster href="" count=""></cluster>
		 *	  <cluster href="" count=""></cluster>
		 *	 </category>
		 * </contextual_navigation>
		 */
        if ($adv_hosted_search) {
            $mapping_array = Array(
                                'clusterNav'=> 'cluster_nav',
                                'moreLink'=> 'more_link',
                                'fewerLink'=> 'fewer_link',
                             );
            $fluster = $result_packet->item(0)->getElementsByTagName('contextualNavigation');
        } else {
    		$fluster = $xml->getElementsByTagName('contextual_navigation');
        }
		// before FB v9.1, we call it fluster
		if($fluster->length == 0) $fluster = $xml->getElementsByTagName('fluster');
			
		for ($l=0; $l<$fluster->length; $l++) {
			// Process the cluster_nav elements
            $tag_name =  $adv_hosted_search ? 'clusterNav' : 'cluster_nav';
			$cluster_nav = $fluster->item($l)->getElementsByTagName($tag_name);
			for ($m=0; $m<$cluster_nav->length; $m++) {
				$current_nav = Array();
				$current_nav['name'] = $cluster_nav->item($m)->nodeValue;
				if ($cluster_nav->item($m)->hasAttributes()) {
					foreach ($cluster_nav->item($m)->attributes as $attr) {
						if ($attr->name == 'url') {
							$current_nav[$attr->name] = $this->_stripURL($attr->value);
						} else {
							$current_nav[$attr->name] = $attr->value;
						}//end if
					}//end foreach
				}//end if
				$level = array_get_index($current_nav, 'level', 0);
				$results['fluster']['navigation'][$level] = $current_nav;
			}//end foreach

			// Process the category elements
            $tag_name = $adv_hosted_search ? 'com.funnelback.publicui.search.model.padre.Category' : 'category';
			$category = $fluster->item($l)->getElementsByTagName($tag_name);
			for ($n=0; $n<$category->length; $n++) {
				$value = $category->item($n)->getAttribute('name');
				$more  = (int) $category->item($n)->getAttribute('more');
				if (!empty($more)) {
					$results['fluster']['category'][$value]['more'] = TRUE;
				} else {
					$results['fluster']['category'][$value]['more']= FALSE;
				}//end if
				if ($category->item($n)->hasChildNodes()) {
					foreach ($category->item($n)->childNodes as $cluster) {
						// For Funnelback 9.0 and higher, this is the more link
                        $tag_name = isset($mapping_array[$cluster->nodeName]) ? $mapping_array[$cluster->nodeName] : $cluster->nodeName;
						if ($tag_name == 'more_link') {
							$results['fluster']['category'][$value]['more'] = TRUE;
						}//end if

						// Process the rest
                        $tag_name = $adv_hosted_search ? 'com.funnelback.publicui.search.model.padre.Cluster' : 'cluster';
						if ($cluster->nodeName != $tag_name) continue;
						$fluster_cluster = Array();
						if ($cluster->hasAttributes()) {
							foreach ($cluster->attributes as $attr) {
								if ($attr->name == 'href') {
									$fluster_cluster[$attr->name] = $this->_stripURL($attr->value);
								} else {
									$fluster_cluster[$attr->name] = $attr->value;
								}//end if
							}//end foreach
						}//end if
						$cluster_name_value = trim((string) $cluster->nodeValue);
						if (!empty($cluster_name_value)) {
							$fluster_cluster['name'] = $cluster_name_value;
							$results['fluster']['category'][$value][] = $fluster_cluster;
						}//end if
					}//end foreach
				}//end if
			}//end for
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
		$spell = $xml->getElementsByTagName('spell');
		for ($o=0; $o<$spell->length; $o++) {
			if ($spell->item($o)->hasChildNodes()) {
				foreach ($spell->item($o)->childNodes as $spell_value) {
					$spell_name = $spell_value->nodeName;
					if ($spell_name == 'map') {
						$map = Array();
						if ($spell_value->hasChildNodes()) {
							foreach ($spell_value->childNodes as $map_value) {
								$map_name = $map_value->nodeValue;
								$map[$map_name] = (string)$map_value->nodeValue;
							}//end foreach
						}//end if
						$results['spell']['map'][] = $map;
					} else {
						$results['spell'][$spell_name] = (string) $spell_value->nodeValue;
					}//end if
				}//end foreach
			}//end if
		}//end for

		return $results;

	}//end processSearchSummary()


	/**
	* Process the search results information
	*
	* @param object &$xml	The simplexml object with the goods!
	*
	* @return array
	* @access public
	*/
	public function processSearchResults(&$xml)
	{
		$results = Array();

        $adv_hosted_search = FALSE;
        if ($this->attr('search_type') == 'hosted' && $this->attr('hosted_mode')) {
            $mapping_array = Array(
                                'liveUrl'           => 'live_url',
                                'cacheUrl'          => 'cache_url',
                                'fileSize'          => 'filesize',
                                'fileType'          => 'filetype',
                                'docNum'            => 'docnum',
                                'clickTrackingUrl'  => 'click_tracking_url',
                              );
            $response = $xml->getElementsByTagName('response');
            $resultPacket = $response->item(0)->getElementsByTagName('resultPacket');
            $result = $resultPacket->item(0)->getElementsByTagName('results');
            $adv_hosted_search = TRUE;
        } else {
            $result = $xml->getElementsByTagName('results');
        }

        for ($p=0; $p<$result->length; $p++) {
            if ($result->item($p)->hasChildNodes()) {
                foreach ($result->item($p)->childNodes as $single_result) {
                    $expected_tag = $adv_hosted_search ? 'com.funnelback.publicui.search.model.padre.Result' : 'result';
                    $name = $single_result->nodeName;
                    if ($name == $expected_tag) {
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
                         *		<md f="T"></md>
                         * </result>
                         */
                        if ($single_result->hasChildNodes()) {
                            foreach ($single_result->childNodes as $value) {
                                $nodeName = $value->nodeName;
                                if ($nodeName == 'md' && $value->hasAttribute('f')) {
                                    $nodeName = 'meta_'.$value->getAttribute('f');
                                }
                                if ($adv_hosted_search) 
                                    $nodeName = (isset($mapping_array[$nodeName])) ? $mapping_array[$nodeName] : $nodeName;
                                $each_result[$nodeName] = utf8_decode($value->nodeValue);
                            }//end foreach
                        }//end if

                        // Return a result TODO Unsure about this yet!
                        $rank = array_get_index($each_result, 'rank', 0);
                        if (!empty($rank)) {
                            $results[$rank] = $each_result;
                        }//end if
                    }//end if
                }//end foreach
            }//end if
        }//end for

		return $results;

	}//end processSearchResults()


	/**
	* This func will sanitize (remove '%') the keywords if they
	* are in user input and the options is selected to sanitize 'em
	*
	* @param string	$queries	array of queries that are supposed to be processedA
	*
	* @return string
	* @access private
	*/
	private function _sanitizeQueries($queries)
	{
		if(is_array($queries)) {
			foreach($queries as $field => $query) {
				$queries[$field] = $this->_sanitizeQueries($query);
			}
		} else {
			// sanitise it by removing '%' from keywords so that
			// they dont go ahead and try to fetch it replacement
			$queries = str_replace(chr(0), '' , $queries);
			$keywords = extract_keywords($queries);
			foreach ($keywords as $keyword) {
				$pattern = '%'.$keyword.'%';
				$queries = str_replace($pattern, $keyword, $queries);
			}
		}

		return $queries;

	}//end _sanitizeQueries()


}//end class


/**
* XML Error Handler
*
* @param string	$errno		The error number
* @param string	$errstr		The error string
* @param string	$errfile	The error file
* @param string	$errline	The error line
*
* @throw ErrorException
* @return void
* @access public
*/
function funnelback_xml_error_handler($errno, $errstr, $errfile, $errline)
{
	if ($errno & (E_USER_ERROR | E_ERROR | E_USER_WARNING | E_WARNING)) {
		log_dump('XML ERROR: '.$errstr.'. Error Number:'.$errno.'. Error File:'.$errfile.'. Error Line:'.$errline);
		throw new ErrorException($errstr, $errno);
	}//end if

}//end funnelback_xml_error_handler()


?>
