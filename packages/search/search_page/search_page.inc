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
* $Id: search_page.inc,v 1.23 2004/02/10 00:35:43 diarandine Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Search_Page
*
* Page template for searching
*
* @see Page
*
* @author  Dominic Wong <dwong@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 2.0
* @package MySource_Matrix_Packages
* @subpackage search
*/

require_once SQ_PACKAGES_PATH.'/cms/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

class Search_Page extends Page
{
	/**
	* An array of bodycopies and their names that are attached underneath this page asset.
	* @var Array()
	*/
	var $bodycopies = Array (
		'initial'			=> 'Initial Search Page Layout',
		'results'			=> 'Results Page Layout',
		'result_element'	=> 'Result Element Layout',
	);

	/**
	* An array of default parameters.
	*
	* @var Array()
	*/
	var $defaults = Array (
		'query_width'	=> 25,
	);

	var $keywords = Array(
						'initial' => Array(
							'submit_button'		=> Array('description' => '<input type="button" value="Search" />', 'type' => 'special'),
							'results_per_page'	=> Array('description' => '<input type="text" value="Results per page" />', 'type' => 'special'),
												),
						'results' => Array(
							'result_list'				=> Array('description' => 'List of search results', 'type' => 'block'),
							'result_count'				=> Array('description' => 'Number of results found', 'type' => 'inline'),
							'current_result_page'		=> Array('description' => 'Page of results we are on', 'type' => 'inline'),
							'total_result_pages'		=> Array('description' => 'Total number of result pages', 'type' => 'inline'),
							'result_page_index'			=> Array('description' => 'List of result pages', 'type' => 'inline'),
							'previous_result_page_link'	=> Array('description' => 'Previous', 'type' => 'inline'),
							'next_result_page_link'		=> Array('description' => 'Next', 'type' => 'inline'),
							'initial_layout'			=> Array('description' => 'New search', 'type' => 'block'),
												),
						'result_element' => Array(
							'score'						=> Array('description' => 'Result score', 'type' => 'inline'),
							'result_number'				=> Array('description' => 'No.', 'type' => 'inline'),
							'asset_id'					=> Array('description' => 'Asset ID', 'type' => 'inline'),
							'asset_name'				=> Array('description' => 'Name of the asset', 'type' => 'inline'),
							'asset_name_linked'			=> Array('description' => 'Name of the asset (linked)', 'type' => 'inline'),
							'asset_short_name'			=> Array('description' => 'Short name of the asset', 'type' => 'inline'),
							'asset_short_name_linked'	=> Array('description' => 'Short name of the asset (linked)', 'type' => 'inline'),
							'asset_url'					=> Array('description' => 'URL of the asset', 'type' => 'inline'),
							'asset_lineage'				=> Array('description' => 'Lineage to the asset', 'type' => 'inline'),
							'asset_lineage_linked'		=> Array('description' => 'Lineage to the asset (linked)', 'type' => 'inline'),
												),
						);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Page($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// add a bodycopy for the initial search page layout, result page layout, and result rows
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		foreach ($this->bodycopies as $value => $name) {
			$copy_link = Array(
				'asset' => &$this,
				'link_type' => SQ_LINK_TYPE_2,
				'dependant' => 1,
				'exclusive' => 1,
				'value'		=> $value
			);

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			if (!$bodycopy->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	*
	* @return int the linkid of the created link
	* @access public
	* @see Asset::createLink()
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}//end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the search page';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => count($this->bodycopies), 'exclusive' => true);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		// get the corresponding bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$mode =& $_REQUEST['mode'];
		if (!isset($mode)) $mode = 'initial';

		$current_result_page =& $_REQUEST['current_result_page'];
		$results_per_page =& $_REQUEST['results_per_page'];

		if (!isset($current_result_page)) $current_result_page = 0;
		if (!isset($results_per_page)) $results_per_page = $this->attr('results_per_page');
		?>
		<form name="search_form" method="get" action="<?php echo $this->getUrl(); ?>">
			<input type="hidden" name="mode" value="<?php echo $mode; ?>" />
			<input type="hidden" name="current_result_page" value="<?php echo $current_result_page; ?>" />
			<input type="hidden" name="results_per_page" value="<?php echo $results_per_page; ?>" />
			<?php
			switch ($mode) {
				case 'results':
					$this->printResultsBody();
					break;
				case 'initial':
				default:
					echo $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
				break;
			}
			?>
		</form>
		<?php

	}//end printBody()


	/**
	* Prints the results bodycopy
	*
	* @access private
	*
	* @return void
	*/
	function printResultsBody()
	{
		$results = $this->processSearch();
		$replacements = array_merge($this->getInitialBodyReplacements(), $this->getResultsBodyReplacements($results));
		echo $this->getBodycopyContents('results', $replacements);

	}//end printResultsBody()


	/**
	* Retrieves a bodycopy by name
	*
	* @param string		$name	the name of the bodycopy to retrieve
	*
	* @return [object|null] The bodycopy, or null if the name does not match any bodycopies.
	* @access private
	* @see Search_Page::bodycopies
	*/
	function &getBodycopy($name)
	{
		if (array_key_exists($name, $this->bodycopies)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			$link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, $name, 'major', '1');
			$bodycopy =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			return $bodycopy;
		} else {
			return null;
		}

	}//end getBodycopy()


	/**
	* Retrieves a bodycopy's contents by name.
	*
	* @param string		$name	the name of the bodycopy to retrieve
	*
	* @return [string|null] the bodycopy's contents, or empty string if the name does not match any bodycopies
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
	* Prints the initial bodycopy
	*
	* @return void
	* @access private
	*/
	function getInitialBodyReplacements()
	{
		$bodycopy =& $this->getBodycopy('initial');
		$keywords = $bodycopy->getKeywords();
		$replaces = Array();

		$results_per_page     = $_REQUEST['results_per_page'];
		$current_results_page = $_REQUEST['current_result_page'];

		$fields = $this->attr('fields');

		$queries =& $_REQUEST['queries'];

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];
			if (ereg('(.+)_query', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$query_var = $matches[1]."_query";
				$query =& $queries[$query_var];
				if (!isset($query)) $query = '';

				ob_start();
					text_box("queries[$query_var]", $query, $this->defaults['query_width']);
					$replacement = ob_get_contents();
				ob_end_clean();

			} else {
				switch ($keyword) {
					case 'submit_button':
						$replacement  = '<input type="submit" onclick="
							document.search_form.mode.value = \'results\';
							return true;
						" value="Search" />';
					break;

					case 'results_per_page':
						$replacement = '<input type=text onchange="
							document.search_form.results_per_page.value = this.value;
						" value="'.$results_per_page .'" />';
					break;

					default:
						$replacement = "%$keyword%";
					break;
				}
			}
		}

		return $replaces;

	}//end getInitialBodyReplacements()


	/**
	* Keyword-replaces for the results mode.
	*
	* @param Array	$results	the results returned
	*
	* @return void
	* @access private
	*/
	function getResultsBodyReplacements(&$results)
	{
		$bodycopy =& $this->getBodycopy('results');
		$keywords = $bodycopy->getKeywords();
		$replaces = Array();

		$current_result_page = $_REQUEST['current_result_page'];
		$results_per_page    = $_REQUEST['results_per_page'];

		$total_pages = ceil(count($results) / $results_per_page);

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];

			switch ($keyword) {
				case 'result_list':
					$replacement = '';
					$sub_resultids = array_slice(array_keys($results), $current_result_page * $results_per_page, $results_per_page);
					$result_num = $current_result_page * $results_per_page;
					foreach ($sub_resultids as $assetid) {
						$result_num++;
						$score = $results[$assetid];
						$replacement .= $this->getBodycopyContents('result_element', $this->getResultElementBodyReplacements($assetid, $score, $result_num));
					}
				break;

				case 'result_count':
					$replacement = count($results);
				break;

				case 'current_result_page':
					$replacement = min($current_result_page + 1, $total_pages);
				break;

				case 'total_result_pages':
					$replacement = $total_pages;
				break;

				case 'result_page_index':
					$replacement = '';
					for ($i = 0; $i < $total_pages; $i++) {
						if ($replacement != '')
							$replacement .= '&nbsp;';

						if ($i == $current_result_page) {
							$replacement .= "<b>".($i + 1)."</b>";
						} else {

							$js = '
								document.search_form.current_result_page.value = '.$i.';
								document.search_form.submit();
								return false;
							';
							$replacement .= "<a href=\"". $this->getURL(). "\" onClick='$js'>".($i + 1)."</a>";
						}
					}
				break;

				case 'previous_result_page_link':
					if ($current_result_page < 1) {
						$replacement = 'Previous';
						break;
					}

					$js = '
						document.search_form.current_result_page.value--;
						document.search_form.submit();
						return false;
					';
					$replacement = '<a href="'. $this->getURL() .'" onClick=\''.$js .'\'>Previous</a>';
				break;

				case 'next_result_page_link':
					if ($current_result_page >= $total_pages - 1) {
						$replacement = 'Next';
						break;
					}

					$js = '
						document.search_form.current_result_page.value++;
						document.search_form.submit();
						return false;
					';
					$replacement = '<a href="'. $this->getURL() .'" onClick=\''.$js .'\'>Next</a>';
				break;

				case 'initial_layout':
					$replacement = $this->getBodycopyContents('initial', $this->getInitialBodyReplacements());
				break;

				default:
					$replacement = "%$keyword%";
				break;
			}
		}

		return $replaces;

	}//end getResultsBodyReplacements()


	/**
	* Keyword-replaces for the result element
	*
	* @param int		$assetid		the asset ID of the result element
	* @param int		$score			the score
	* @param int		$result_num		the order of the result in the result list
	*
	* @return void
	* @access private
	*/
	function getResultElementBodyReplacements($assetid, $score, $result_num)
	{
		$bodycopy =& $this->getBodycopy('result_element');
		$keywords = $bodycopy->getKeywords();

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$asset = &$am->getAsset($assetid);

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$replaces = $mm->generateKeywordReplacements($asset, $keywords, false);

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];
			switch ($keyword) {
				case 'score':
					$replacement = $score;
				break;

				case 'result_number':
					$replacement = $result_num;
				break;

				case 'asset_id':
					$replacement = $assetid;
				break;

				case 'asset_name_linked':
					$replacement = '<a href="' .$asset->getUrl() .'">' .$asset->name .'</a>';
				break;

				case 'asset_short_name_linked':
					$replacement = '<a href="' .$asset->getUrl() .'">' .$asset->short_name .'</a>';
				break;

				case 'asset_lineage':
				case 'asset_lineage_linked':
					$replacement = '';
					$first = true;
					$found_root = false;
					$parentAssetIds = $am->getParents($asset->id);
					foreach ($parentAssetIds as $parentAssetId) {
						if ($parentAssetId == $this->attr('root_assetid')) $found_root = true;
						if (!$found_root) continue;
						if (!$first) $replacement .= $this->attr('lineage_seperator');
						$first = false;
						$ancestorAsset = &$am->getAsset($parentAssetId);
						if ($keyword == 'asset_lineage_linked') $replacement .= '<a href="'.$ancestorAsset->getUrl().'">';
						$replacement .= $ancestorAsset->name;
						if ($keyword == 'asset_lineage_linked') $replacement .= '</a>';
						$am->forgetAsset($ancestorAsset);
					}
				break;
			}
		}

		return $replaces;

	}//end getResultElementBodyReplacements()


	/**
	* Processes a search request
	*
	* From the posted variables, it Formulates search query and returns the results.
	*
	* @return Array()
	* @access private
	*/
	function processSearch()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$sm =& $am->getSystemAsset('search_manager');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$fields = $this->attr('fields');
		if (is_scalar($fields))
			$fields = unserialize($fields);

		$queries = $_REQUEST['queries'];

		$logic = $this->attr('logic');

		$root_assetid = $this->attr('root_assetid');
		if ($root_assetid <= 0)
			$root_assetid = 1;

		$total_asset_scores = null;
		$exclude_assetids = Array();

		foreach($queries as $query_var => $value) {
			if ($value == '')
				continue;

			$words = split(" +", strtolower($value));
			foreach (array_keys($words) as $i) {
				// convert wildcard symbols to corresponding SQL wildcards
				$words[$i] = strtr($words[$i], "*?", "%_");

				// if this is an Email, skip the following check...
				$at_pos  = strpos($words[$i], '@'); 
				$dot_pos = strpos($words[$i], '.');
				if ($at_pos === false && $dot_pos === false) { 
					// preserve the wildcard symbols - i.e. don't quote them
					if (ereg('^[A-z%_]+$', $words[$i]))
						$words[$i] = addslashes($words[$i]);
					else
						unset($words[$i]);
				} // if this is an Email
			}

			// retrieve the search field for this query
			ereg("(.+)_query", $query_var, $query_match);
			$query_name = $query_match[1];
			$field =& $fields[$query_name];

			$concat = ($db->phptype == 'mysql') ? 'CONCAT(alt1.treeid, '.$db->quote('%').')' : '(alt1.treeid || '.$db->quote('%').')';

			$start_sql = "SELECT DISTINCT(ai.assetid), SUM(ai.score)
				FROM " .
				SQ_TABLE_PREFIX . "asset_indexing ai,\n"	.
				SQ_TABLE_PREFIX . "asset_link_tree alt1,\n"	.
				SQ_TABLE_PREFIX . "asset_link_tree alt2,\n"	.
				SQ_TABLE_PREFIX . "asset_link al1,\n"		.
				SQ_TABLE_PREFIX . "asset_link al2,\n"		.
				SQ_TABLE_PREFIX . "asset_permission ap,\n"      .
				SQ_TABLE_PREFIX . "asset a\n"                  .
				"WHERE ".
				"al1.majorid = " . $db->quote((int)$root_assetid).	// all links that root asset is a major in
				" AND al1.linkid = alt1.linkid "			.	// all treeids with those links
				" AND alt2.treeid LIKE ".$concat			.	// all treeids that start with those treeids
				" AND alt2.linkid = al2.linkid "			.	// all links with those treeids
				" AND ai.assetid  = al2.minorid \n"			.	// and the indexing data for those assets
				" AND a.assetid   = ai.assetid \n"			.	// and the asset data for those assets       
				" AND ap.assetid  = ai.assetid \n"			. // and the permission data for those assets
				//" AND a.status  >= ".SQ_STATUS_LIVE." \n";          // and only assets that are live

			$userid_cond = '';
			if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
				if (!$GLOBALS['SQ_SYSTEM']->userRoot() && $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					$userids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
					array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
					$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
				}
			} else {
				// public users
				$userid_cond = ' AND ap.userid = 0';
			}
			if (!empty($userid_cond)) {
				$start_sql .= $userid_cond;
				$start_sql .= '	AND ap.permission >= '.SQ_PERMISSION_READ.'
						AND ap.access = 1';
						
			}
			
			switch($field['type']) {

				case 'asset_attrib': 
					if ($field['params']['searchtype'] == '')
						break;
					$field_asset_scores = null;

					foreach ($words as $word) {
						$sql = $start_sql . "AND (ai.component = 'attr:". $field['params']['searchtype'] . ":" . $field['params']['type_attr'] . "'
							AND (ai.value LIKE '%". implode ("' OR ai.value LIKE '", $words) ."%'))\n";
						$sql .= "GROUP BY ai.assetid";

						$results = $db->getAssoc($sql);

						if (is_null($field_asset_scores)) {
							$field_asset_scores = $results;
						} else {
							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $results, 'AND');
						};
					}

					if (is_null($total_asset_scores)) {
						$total_asset_scores = $field_asset_scores;
					} else {
						$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $logic);
					}

				break;

				case 'include_all':
					$field_asset_scores = null;
					foreach ($words as $word) {
						$sql = $start_sql . "AND (ai.value LIKE '%$word%')\n";
						$sql .= "GROUP BY ai.assetid";

						$results = $db->getAssoc($sql);
						if (is_null($field_asset_scores)) {
							$field_asset_scores = $results;
						} else {
							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $results, 'AND');
						};
					}

					if (is_null($total_asset_scores)) {
						$total_asset_scores = $field_asset_scores;
					} else {
						$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $logic);
					}

				break;

				case 'exclude':
					// ignore logic and remove these assetids from total_asset_scores at the end
					$removing_assetids = Array();

					$field_asset_scores = null;
					foreach ($words as $word) {
						$sql = $start_sql . "AND (ai.value LIKE '%$word%')\n";
						$sql .= "GROUP BY ai.assetid";

						$results = $db->getAssoc($sql);
						$exclude_assetids += array_keys($results);
					}

				break;

				case 'metadata':
					if ($field['params']['metadata_field'] <= 0)
						break;

					$field_asset_scores = null;
					foreach ($words as $word) {
						$sql = $start_sql . "AND (ai.type = 'text'
							AND ai.component = 'metadata:". (int)$field['params']['metadata_field']['assetid'] . "'
							AND (ai.value LIKE '". implode ("' OR ai.value LIKE '", $words) ."'))\n";
						$sql .= "GROUP BY ai.assetid";


						$results = $db->getAssoc($sql);

						if (is_null($field_asset_scores)) {
							$field_asset_scores = $results;
						} else {
							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $results, 'AND');
						};
					}

					if (is_null($total_asset_scores)) {
						$total_asset_scores = $field_asset_scores;
					} else {
						$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $logic);
					}

				default:
				break;
			}//end switch

		}//end for

		foreach ($exclude_assetids as $exclude_assetid) {
			unset($total_asset_scores[$exclude_assetid]);
		}

		// Allowed code types should be specified dynamically!!

		$results = $this->_climbToAssetTypes($total_asset_scores, array('file', 'page', 'workflow_schema', 'metadata_schema', 'user'));

		return $results;

	}//end processSearch()


	/**
	* Combines asset scores according to the passed logic
	*
	* @param array	$asset_scores1		a mapping from assetids to scores
	* @param array	$asset_scores2		a mapping from assetids to scores
	* @param string	$logic				either 'AND' or 'OR"
	*
	* @return Array a mapping from assetids to combined scores
	* @access private
	*/
	function combineAssetScores($asset_scores1, $asset_scores2, $logic = 'AND')
	{
		if (!is_array($asset_scores1) ||!is_array($asset_scores2)) return Array();

		// find the intersection/union (depending on logic) of the assetids in field_asset_scores and results
		$assets1 = array_keys($asset_scores1);
		$assets2 = array_keys($asset_scores2);

		$out_asset_scores = $asset_scores1;

		if ($logic == 'AND') {
			$common_assets = array_intersect($assets1, $assets2);
			// get the assets that are to be removed in field_asset_scores
			$removed_assets = array_diff($assets1, $assets2);

			foreach ($removed_assets as $assetid) {
				unset ($out_asset_scores[$assetid]);
			}

			foreach ($common_assets as $assetid) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid] += $asset_scores2[$assetid];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}
			}
		} elseif ($logic == 'OR') {
			foreach ($asset_scores2 as $assetid => $score) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid] += $asset_scores2[$assetid];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}
			}
		}
		return $out_asset_scores;

	}//end combineAssetScores()


	/**
	* Resolves a result array to an array where the assets are elements and are one of the types specified.
	*
	* @param string		$matches		matches returned from processSearch()
	* @param Array		$assetTypes		types allowed
	*
	* @return Array
	* @access private
	* @see Search_Page::processSearch()
	*/
	function _climbToAssetTypes($matches, $assetTypes)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$unsorted = Array();

		// expand out asset types to include each asset type's descendants
		$newDescendantTypes = Array();
		foreach ($assetTypes as $assetType) {
			$descendantTypes = $am->getTypeDescendants($assetType);
			foreach($descendantTypes as $descendantAssetType) {
				$newDescendantTypes[] = $descendantAssetType;
			}
		}

		$assetTypes = array_merge($assetTypes, $newDescendantTypes);

		$asset_info = $am->getAssetInfo(array_keys($matches));

		// use matches as a queue, take head match if it is one of the asset types then add to out
		// otherwise get parent links and add it to the end of the queue

		$scores = Array(); // for sorting

		while (count($matches)) {
			// get next asset=>score mapping
			$assetids = array_keys($matches);
			$nextAssetId = $assetids[0];
			$nextScore = $matches[$nextAssetId];
			unset ($matches[$nextAssetId]);
			if (!isset($asset_info[$nextAssetId])) {
				$info = $am->getAssetInfo(Array($nextAssetId));
				$asset_info[$nextAssetId] = $info[$nextAssetId];
			}

			$nextAssetInfo = $asset_info[$nextAssetId];
			if (in_array($nextAssetInfo['type_code'], $assetTypes)) {
				// could be added twice - check
				if (isset($unsorted[$nextAssetId])) {
					$unsorted[$nextAssetId] += $nextScore;
				} else {
					$unsorted[$nextAssetId] = $nextScore;
				}
				$scores[$nextAssetId] = $nextScore;
			} else {
				// doesn't match one of the types, go up

				// if we are the root asset for this search, then stop
				if ($nextAssetId == $this->attr('root_assetid'))
					continue;

				$parentLinks = $am->getLinks($nextAssetId, SQ_SC_LINK_ALL, '', false, 'minor', null, true);

				foreach ($parentLinks as $parentLink) {
					$parentAssetId = $parentLink['majorid'];
					// does the parent already have a match - if so add scores
					if (isset($matches[$parentAssetId])) {
						$matches[$parentAssetId] += $nextScore;
					} else {
						$matches[$parentAssetId] = $nextScore;
					}
				}
			}
		}
		arsort($scores);

		return $scores;

	}//end _climbToAssetTypes()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;
		if (!is_a($broadcaster, 'content_type_wysiwyg')) return;

		// lets work out which bodycopy the WYSIWYG cell that wants our keywords
		// is in so we can return the correct keywords for the screen
		$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);

		$bodycopies = Array();
		foreach ($bodycopy_links as $link_info) {
			if (in_array($link_info['minorid'], $parents)) {
				$bodycopies[] = $link_info['value'];
			}
		}
		if (empty($bodycopies)) return;

		$keywords = Array();
		foreach ($bodycopies as $bodycopy) {
			if ($bodycopy == 'initial') {
				// work out the query replacements
				$fields = $this->attr('fields');
				foreach ($fields as $field => $field_data) {
					$name = 'Search Field ('.$field.')';
					$description = '<input type="text" width="'.$this->defaults['query_width'].'" value="Search terms here" />';
					$keywords[$field.'_query'] = Array('name' => $name, 'default' => $description, 'type' => 'special');
				}
			} else if ($bodycopy == 'result_element') {
				$default_keywords = $broadcaster->getAssetKeywords(true);
				foreach ($default_keywords as $keyword => $info) {
					$name = ucwords(str_replace('_', ' ', $keyword));
					$keywords[$keyword] = Array('name' => $name, 'default' => $info['description'], 'type' => 'inline');
				}
			}
			foreach ($this->keywords[$bodycopy] as $keyword => $info) {
				$name = ucwords(str_replace('_', ' ', $keyword));
				$keywords[$keyword] = Array('name' => $name, 'default' => $info['description'], 'type' => $info['type']);
			}
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


}//end class

?>
