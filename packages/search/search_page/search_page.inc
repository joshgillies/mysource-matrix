<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_page.inc,v 1.3 2003/10/14 04:30:11 dwong Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Search_Page
*
* Page template for searching.
* 
* @see Page
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.3 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/

require_once SQ_PACKAGES_PATH.'/cms/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Search_Page
*
* Page template for searching.
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.3 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
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

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Page($assetid=0)
	{
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
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the search page';

		return parent::canDeleteLink($linkid);

	}//end canDeleteLink()


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
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		if (isset($_GET['sq_from_frontend'])) {
			// get the bodycopy attached to us
			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
			$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');
			if (is_null($bodycopy)) {
				trigger_error('Could not locate the bodycopy for search page "'.$this->name.'"', E_USER_ERROR);
				return false;
			}
			$o->addFormActionGetVar('assetid', $bodycopy->id);
			$bodycopy->paintBackend($o);
		} else {
			parent::paintBackend($o);
		}

	}// end paintBackend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		// get the corresponding bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$mode =& $_REQUEST['mode'];
		if (!isset($mode))
			$mode = 'initial';

		$current_result_page =& $_REQUEST['current_result_page'];
		$results_per_page =& $_REQUEST['results_per_page'];

		if (!isset($current_result_page))
			$current_result_page = 0;
		if (!isset($results_per_page))
			$results_per_page = $this->attr('results_per_page');
		?>
		<form name="search_form" method="GET" action="<?=$this->getUrl()?>">
		<input type=hidden name="mode" value="<?=$mode?>">
		<input type=hidden name="current_result_page" value="<?=$current_result_page?>">
		<input type=hidden name="results_per_page" value="<?=$results_per_page?>">


		<?php

		switch ($mode) {
			case 'results':
				$this->printResultsBody();
				break;
			case 'initial':
			default:
				$this->printInitialBody();
			break;
		}

		?>
		</form>
		<?php

	}// end printBody()

	/** 
	* Prints the results body copy.
	* 
	* @access private
	*
	* @return void
	*/
	function printResultsBody()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$results = $this->processSearch();

		// print them
		$results_html = $this->getBodycopyContents('results');
		$this->replaceResultsKeywords($results_html, $results);
		echo $results_html;
	} // end printResultsBody()

	/**
	* Retrieves a bodycopy by name.
	* @see Search_Page::bodycopies
	* 
	* @access private
	*
	* @param string $name The name of the bodycopy to retrieve.
	* @return [object|null] The bodycopy, or null if the name does not match any bodycopies.
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
	} // end getBodycopy()

	/**
	* Retrieves a bodycopy's contents by name.
	* @see Search_Page::bodycopies
	* @see Search_Page::getBodycopy
	* @access private
	*
	* @param string $name The name of the bodycopy to retrieve.
	* @return [string|null] The bodycopy's contents, or empty string if the name does not match any bodycopies.
	*/
	function &getBodycopyContents($name)
	{
		$bodycopy =& $this->getBodycopy($name);

		if (is_null($bodycopy))
			return '';

		ob_start();
		$bodycopy->printBody();
		$html = ob_get_contents();
		ob_end_clean();
		return $html;
	} // end getBodycopyContents()

	/** 
	* Prints the initial body copy.
	* 
	* @access private
	*
	* @return void
	*/
	function printInitialBody()
	{
		$html = $this->getBodycopyContents('initial');
		$this->replaceInitialKeywords($html);
		
		echo $html;
	} // end printInitialBody()

	/** 
	* Keyword-replaces for the initial mode. 
	* 
	* @access private
	* @param string $html The HTML to keyword replace.
	*
	* @return void
	*/
	function replaceInitialKeywords(&$html)
	{
		$keywords = retrieve_keywords_replacements($html);
		$replaces = array();

		$results_per_page =& $_REQUEST['results_per_page'];
		$current_results_page =& $_REQUEST['current_result_page'];

		$fields = $this->attr('fields');
		if (is_scalar($fields))
			$fields = unserialize($fields);

		$queries =& $_REQUEST['queries'];

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];
			if (ereg('(.+)_query', $keyword, $matches) && array_key_exists($matches[1], $fields)) {
				$query_var = $matches[1]."_query";
				$query =& $queries[$query_var];
				if (!isset($query))
					$query = '';

				ob_start();
				text_box("queries[$query_var]", $query, $this->defaults['query_width']);
				$replacement = ob_get_contents();
				ob_end_clean();

			} else {
				switch ($keyword) {
					case 'submit_button':
						$replacement  = '<input type=submit onClick="
							document.search_form.mode.value = \'results\';
							return true;
						" value="Search">';
					break;

					case 'results_per_page':
						$replacement = '<input type=text onChange="
							document.search_form.results_per_page.value = this.value;
						" value="'.$results_per_page .'">';
					break;

					default:
					break;
				}
			}
		}

		$html = replace_keywords($html, $replaces);
	} // end replaceInitialKeywords()

	/** 
	* Keyword-replaces for the results mode.
	* 
	* @access private
	* @param string $html		The HTML to keyword replace.
	* @param Array $results		The results returned.
	*
	* @return void
	*/
	function replaceResultsKeywords(&$html, &$results)
	{
		$keywords = retrieve_keywords_replacements($html);
		$replaces = Array();
		
		$current_result_page =& $_REQUEST['current_result_page'];
		$results_per_page =& $_REQUEST['results_per_page'];

		$total_pages = ceil(count($results) / $results_per_page);

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];
			switch ($keyword) {
				case 'result_list':
					$element_html = $this->getBodycopyContents('result_element');
					$replacement = '';
					$sub_resultids = array_slice(array_keys($results), $current_result_page * $results_per_page, $results_per_page);
					foreach ($sub_resultids as $assetid) {
						$result_element = $results[$assetid];
						$new_element_html = $element_html;
						$this->replaceResultElementKeywords($new_element_html, $assetid, $result_element);
						$replacement .= $new_element_html;
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
					if ($current_result_page < 1)
						break;

					$js = '
						document.search_form.current_result_page.value--;
						document.search_form.submit();
						return false;
					';
					$replacement = '<a href="'. $this->getURL() .'" onClick=\''.$js .'\'>Previous</a>';
				break;

				case 'next_result_page_link':
					if ($current_result_page >= $total_pages - 1)
						break;

					$js = '
						document.search_form.current_result_page.value++;
						document.search_form.submit();
						return false;
					';
					$replacement = '<a href="'. $this->getURL() .'" onClick=\''.$js .'\'>Next</a>';
				break;


				case 'initial_layout':
					$replacement = $this->getBodycopyContents('initial');
					$this->replaceInitialKeywords($replacement);
				break;

				default:
					$replacement = "%$keyword%";
				break;
			}
		}

		$html = replace_keywords($html, $replaces);
		
	} // end replaceResultsKeywords()

	/** 
	* Keyword-replaces for the result element.
	* 
	* @access private
	* @param string $html				The HTML to keyword replace.
	* @param int	$assetid			The Asset ID of the result element.
	* @param Array	$result_element		The result element array, containing the score
	*
	* @return void
	*/
	function replaceResultElementKeywords(&$html, $assetid, $result_element)
	{
		$keywords = retrieve_keywords_replacements($html);
		$replaces = Array();
		
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$asset =& $am->getAsset($assetid);

		foreach ($keywords as $keyword) {
			$replacement =& $replaces[$keyword];
			switch ($keyword) {
				case 'score':
					$replacement = $result_element['score'];
					break;

				case 'asset_id':
					$replacement = $assetid;

				case 'asset_name':
					$replacement = $asset->name;
					break;

				case 'asset_name_linked':
					$replacement = '<a href="' .$asset->getUrl() .'">' .$asset->name .'</a>';
					break;

				case 'asset_short_name':
					$replacement = $asset->short_name;
					break;

				case 'asset_short_name_linked':
					$replacement = '<a href="' .$asset->getUrl() .'">' .$asset->short_name .'</a>';
					break;

				case 'asset_url':
					$replacement = $asset->getUrl();
					break;

				case 'asset_lineage':
					$replacement = '';
					$first = true;
					$parentAssetIds = $am->getParents($asset->id);
					foreach ($parentAssetIds as $parentAssetId) {
						if (!$first)
							$replacement .= " / ";
						$first = false;
						$ancestorAsset = $am->getAsset($parentAssetId);
						$replacement .= $ancestorAsset->name;
					}
			
					break;

				case 'asset_lineage_linked':
					$replacement = '';
					$first = true;
					foreach ($am->getParents($asset->id) as $parentAssetId) {
						if ($parentAssetId == 1)
							continue;
						if (!$first)
							$replacement .= " / ";
						$first = false;
						$ancestorAsset = $am->getAsset($parentAssetId);
						$replacement .= "<a href=\"". $ancestorAsset->getUrl(). "\">". $ancestorAsset->name. "</a>";
					}
			
					break;

				default:
					$replacement = "%$keyword%";
					break;
			}
		}
		$html = replace_keywords($html, $replaces);
	} // end replaceResultElementKeywords()

	/** 
	* Processes a search request. From the posted variables, it Formulates search query and returns the results.
	* 
	* @access private
	* 
	* @return Array()
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

		$score_words = array();

		$asset_list_sql = '';
		// lets create this bastard of an SQL query		
		foreach($queries as $query_var => $value) {
			if ($value == '')
				continue;

			$words = split(" +", $value);
			foreach (array_keys($words) as $i) {
				$word =& $words[$i];
				$word = strtr($word, "*?", "%_");
				if (ereg('^[A-z%_]+$', $word))
					$word = addslashes($word);
				else
					unset($word);
			}

			ereg("(.+)_query", $query_var, $query_match);
			$query_name = $query_match[1];
			$field =& $fields[$query_name];


			$search_sql = "SELECT DISTINCT ai.assetid
				FROM " . 
				SQ_TABLE_PREFIX . "asset_indexing ai,\n"	.
				SQ_TABLE_PREFIX . "asset a,\n"				.
				SQ_TABLE_PREFIX . "asset_link_tree alt1,\n"	.
				SQ_TABLE_PREFIX . "asset_link_tree alt2,\n"	.
				SQ_TABLE_PREFIX . "asset_link al1,\n"		.
				SQ_TABLE_PREFIX . "asset_link al2\n"		.
				"WHERE ".
				"al1.minorid = " . $db->quote((int)$root_assetid)	.	// all links that root asset is a minor in
				" AND al1.linkid = alt1.linkid "						.	// all treeids with those links
				" AND alt2.treeid LIKE alt1.treeid || '%' "			.	// all treeids that start with those treeids
				" AND alt2.linkid = al2.linkid "						.	// all links with those treeids
				" AND ai.assetid = al2.minorid "						;	// and the indexing data for those assets

			switch($field['type']) {
				case 'include_all':
				case 'include_any':
					$score_words = array_merge($score_words, $words);
					foreach (array_keys($words) as $i) {
						if ($asset_list_sql != '') {
							if ($field['type'] == 'include_all')
								$asset_list_sql .= "\nINTERSECT\n";
							if ($field['type'] == 'include_any')
								$asset_list_sql .= "\nUNION\n";
						}
						$asset_list_sql .= "(" .$search_sql .
							"AND ai.type = 'text' " .
							"AND ai.value LIKE '"	. $words[$i] . 
							"')";
					}

				break;

				
				case 'exclude':

					foreach ($words as $word) {
						if ($asset_list_sql != '') {
							$asset_list_sql .= "\nEXCEPT\n";
						}
						$asset_list_sql .= "(" .$search_sql .
							"ai.type = 'text' AND 
							(ai.value LIKE '". implode ("' OR ai.value LIKE '", $words) ."')
							)";
					}

				break;
				
				case 'metadata':	
					$score_words = array_merge($score_words, $words);
					if ($field['params']['metadata_field'] <= 0) 
						break;
					if ($asset_list_sql != '')
						$asset_list_sql .= "\nINTERSECT\n";

					$asset_list_sql .= "(" .$search_sql .
							"AND ai.type = 'text' 
							AND ai.component = 'metadata:". (int)$field['params']['metadata_field'] . "' 
							AND (ai.value LIKE '". implode ("' OR ai.value LIKE '", $words) ."'))";
				break;

				default:
				break;
					
			}
		}
		$sql = "SELECT a.assetid, a.type_code, SUM(ai.score) as score
				FROM ($asset_list_sql) al, sq_asset_indexing ai, sq_asset a
				WHERE a.assetid = ai.assetid AND ai.assetid = al.assetid AND (ai.value LIKE '". implode("' OR ai.value LIKE '", $score_words) ."')
				GROUP BY a.assetid, a.type_code
		";

		if (empty($score_words))
			return Array();

//		speed_check();
		$results = $db->getAll($sql);
//		speed_check("done with query");
		$matches = array();
		foreach($results as $result) {
			$matches[$result['assetid']] = $result;
		}
		$results = $this->climbToAssetTypes($matches, array('file', 'page'));
//		speed_check("climbing done");
		
		return $results;
	} // end processSearch()

	/** 
	* Resolves a result array to an array where the assets are elements and are one of the types specified.
	* 
	* @access private
	* @param string		$matches		Matches returned from processSearch().
	* @param Array()	$assetTypes		Types allowed.
	* @see Search_Page::processSearch()
	*
	* @return Array()
	*/
	function climbToAssetTypes($matches, $assetTypes)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$unsorted = array();

		// expand out asset types to include each asset type's descendants
		$newDescendantTypes = array();
		foreach ($assetTypes as $assetType) {
			$descendantTypes = $am->getTypeDescendants($assetType);
			foreach($descendantTypes as $descendantAssetType) {
				$newDescendantTypes[] = $descendantAssetType;
			}
		}

		$assetTypes = array_merge($assetTypes, $newDescendantTypes);

		// use matches as a queue, take head match if it is one of the asset types then add to out
		// otherwise get parent links and add it to the end of the queue

		$scores = array(); // for sorting

		while (count($matches)) {
			$assetids = array_keys($matches);
			$nextAssetId = $assetids[0];
			$nextMatch = $matches[$nextAssetId];
			unset ($matches[$nextAssetId]);

			if (in_array($nextMatch['type_code'], $assetTypes)) {
				// could be added twice - check
				if (isset($unsorted[$nextAssetId])) {
					$nextMatch['score'] += $unsorted[$nextAssetId]['score'];
				}

				$unsorted[$nextAssetId] = $nextMatch;
				$scores[$nextAssetId] = $nextMatch['score'];
			} else {
				// doesn't match one of the types, go up
				
				// if we are the root asset for this search, then don't go up from here
				if ($nextAssetId == $this->attr('root_assetid'))
					continue;

				$parentLinks = $am->getLinks($nextAssetId, SQ_SC_LINK_ALL, '', false, 'minor', null, true);
				foreach ($parentLinks as $parentLink) {
					$parentAssetId = $parentLink['majorid'];
					$parentMatch = $nextMatch;
					unset($parentMatch['component']);

					$parentMatch['type_code'] = $parentLink['major_type_code'];
					$parentMatch['assetid'] = $parentLink['majorid'];

					// does the parent already have a match - if so add scores
					if (isset($matches[$parentAssetId])) {
						$matches[$parentAssetId]['score'] += $parentMatch['score'];
					} else {
						$matches[$parentAssetId] = $parentMatch;
					}
				}
			}
		}
		arsort($scores);
		$sorted = Array();
		foreach ($scores as $assetid => $score) {
			$sorted[$assetid] =& $unsorted[$assetid];
		}

		return $sorted;
	} // end climbToAssetTypes()

}//end class
?>
