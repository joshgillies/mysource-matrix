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
* $Id: search_page.inc,v 1.32 2004/02/23 05:55:16 diarandine Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc'; // for the medatata date field selection

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
* @version $Revision: 1.32 $ - 1.0
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
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'bodycopy') && $dependant != '1') {
			$dependant = '1';
			return true;
		}

		return false;

	}//end prepareLink()


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
	* Getting the date-time field
	*
	* @param	string				$prefix	fieldname for form elements
	* @param	object Asset		&$asset	the owning asset
	* @param	string				$value	field value, can be null
	*
	* @return object DateTime_Field
	* @access private
	*/
	function getEpochDateTimeField($fieldname, &$asset, $value=null)
	{
		// only if we are printing the default value
		if (is_null($value)) {
			if (!is_null($asset)) {
				$value = (int) $asset->attr('default');
			} else {
				$value = '%asset_created%'; // just testing
			}
		}

		$epoch_date = getdate($value);
		$value_arr = Array(
						'y' => $epoch_date['year'],
						'm' => $epoch_date['mon'],
						'd' => $epoch_date['mday'],
						'h' => $epoch_date['hours'],
						'i' => $epoch_date['minutes'],
						's' => $epoch_date['seconds'],
					);

		// parameters
		if (!is_null($asset)) {
			$parameters = $asset->attr('setup_params');
		}
		if (empty($parameters) || is_null($parameters)) {
			$date = getdate();
			$parameters = Array(
				'min'			=> '2003-01-01 00:00:00',
				'max'			=> ($date['year'] + 1).'-12-31 23:59:59',
				'allow_circa'	=> '0',
				'print_format'	=> $asset->attr('format'),
				'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
				'null'			=> Array(), // allowing nulls everywhere
				'style'			=> Array(
								'y' => 's',
								'm' => 's',
								'd' => 's',
								'h' => 's',
								'i' => 's',
								's' => 's',
							),
			);
		}

		$value = '';
		$field = new DateTime_Field($fieldname, $value, $parameters);
		$value = $field->unitsArrayToStr($value_arr);
		$field->setValue($value);

		return $field;

	}// end _getEpochDateTimeField()


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

				// Finding out what kind of field is it
				$test_ind = $matches[1];

				if ($fields[$test_ind]['type'] == 'metadata') {

					$assetid = $fields[$test_ind]['params']['metadata_field']['assetid'];
					$am = &$GLOBALS['SQ_SYSTEM']->am;
					$asset = &$am->getAsset($assetid);
					$params = $asset->vars;

						if (isset($params['format'])) { // metadata Date field
							// don't ask me why I'm doing this
							$temp_name = $query_var . "__";
							$field = $this->getEpochDateTimeField("$temp_name", $asset);
							$field->processField(); // showing value if any
							$field->printField();
						} else if (isset($params['multi_line'])) { // normal metadata TEXT field
							// using same defintion as for a simple field, but will have to revise later...
							text_box("queries[$query_var]", $query, $this->defaults['query_width']);
						} else if (isset($params['select_options'])) { // metadata dropdown selection
							$select_options = $params['select_options']['value'];
							$select_options[''] = "- Please select -";
							combo_box("queries[$query_var]", $select_options, false, $query);
						}

				} else if ($fields[$test_ind]['type'] == 'asset_attrib') {

					$search_styles = $this->attr('search_styles');

					// [0] - name, [1] - type, [2] - id
					$attr_parts = explode("__", $fields[$test_ind]['params']['type_attr']);

					require_once SQ_ATTRIBUTES_PATH.'/'.$attr_parts[1].'/'.$attr_parts[1].'.inc';
					$attr_class = 'Asset_Attribute_'.$attr_parts[1]; // attr name
					$attribute = new $attr_class($attr_parts[2], $query); // attr id

					if (!empty($search_styles) && isset($search_styles[$attribute->name])) {
						$attribute->_edit_params = $search_styles[$attribute->name];
					}

					$attribute->paint("queries[$query_var]", false, false); 

				} else {

					// not a metadata field, just a normal text field
					text_box("queries[$query_var]", $query, $this->defaults['query_width']);

				}

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
	* Note that all search queries are constructed an run in the Search Manager
	*
	* From the posted variables, it formulates search info and returns results
	*
	* @return array
	* @access public
	* @see Search_Manager:processSearch()
	*/
	function processSearch()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$sm = &$am->getSystemAsset('search_manager');

		$fields = $this->attr('fields');
		if (is_scalar($fields)) $fields = unserialize($fields);

		$queries = $_REQUEST['queries'];

		// check if there are medatadata date fields
		foreach (array_keys($_REQUEST) as $req_key) {
			if (ereg('(.+)__value', $req_key, $query_match)) {
				$newind = $query_match[1];
				$reqind = $query_match[0];
				// if found, adding its values to general queries array
				$queries[$newind] = $_REQUEST[$reqind];
			}
		}

		$search_info = Array();

		foreach($queries as $query_var => $value) {

			if ($value == '') continue;
			if ($value == '_') continue; // for boolean submissions...

			$field = $fields[substr($query_var, 0, -6)];
			
			// checking if the boolean value needs adjustments
			if (isset($field['params'])) { // this is an attribute field...
				if (strpos($field['params']['type_attr'], 'boolean') !== false) { // and it is boolean...
					// we need to convert the value into a word representation, 
					// since indexing service will not operate just 0 or 1
					if ($value == 0) {
						$value = 'false';
					} else {
						$value = 'true';
					}
				}
			}

			if (!is_array($value)) {
				$words = split(' +', strtolower($value));
				foreach (array_keys($words) as $i) {
					// convert wildcard symbols to corresponding SQL wildcards
					$words[$i] = strtr($words[$i], '*?', '%_');

					// if this is an email, skip the following check...
					$at_pos  = strpos($words[$i], '@');
					$dot_pos = strpos($words[$i], '.');
					if ($at_pos === false && $dot_pos === false) {
						// preserve the wildcard symbols - i.e. don't quote them
						if (ereg('^[A-z%_]+$', $words[$i])) $words[$i] = addslashes($words[$i]);
						else unset($words[$i]);
					} // if this is an email
				}
			} else {
				// if $value is array and the field is a metadata field, then the
				// metadata field is a datetime field and needs to have its value
				// entered into an array - otherwise, the current $value array is fine
				if ($field['type'] == 'metadata') $words = Array($value);
				else $words = $value;
			}

			$search_info[$field['type']][$field['name']]['words'] = $words;
			$search_info[$field['type']][$field['name']]['params'] = (isset($field['params'])) ? $field['params'] : Array();
		}

		$search_info['logic'] = $this->attr('logic');
		$search_info['root_assetid'] = ($this->attr('root_assetid') <= 0) ? 1 : $this->attr('root_assetid');
		$search_info['search_types'] = $this->attr('search_types');

		return $sm->processSearch($search_info);

	}//end processSearch()


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
