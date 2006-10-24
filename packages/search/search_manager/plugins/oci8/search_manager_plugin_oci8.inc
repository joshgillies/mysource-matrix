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
* $Id: search_manager_plugin_oci8.inc,v 1.3 2006/10/24 01:34:29 lwright Exp $
*
*/


/**
* Search_Manager_Plugin_OCI8
*
* Handles of certain low-level search functions, optimised to improve database
* performance under Oracle (9i/10g) by using Oracle's text package
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Plugin_OCI8
{


		/**
	* Processes a search for a particular word in a particular data source
	*
	* Return value is based on what has been set as the 'select' fields in the
	* base query ($base_query). Typically this will be (keyword => score) but
	* this can obviously be changed.
	*
	* @param string	$search_term	Word to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return array
	* @access public
	*/
	function processWordSearch(&$sm, $search_term, $data_source, $base_query)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		switch ($data_source['type']) {

			case 'include_all':
				$contains_str = $this->_buildContainsQuery('ai.value', $search_term);
				if (!$contains_str) return Array();
				$base_query['where'][] = $contains_str;

			break;

			case 'asset_attrib':
				$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);
				$base_query['where'][] = 'ai.component = '.$db->quote('attr:'.$attribute->name);

				$contains_str = $this->_buildContainsQuery('ai.value', $search_term);
				if (!$contains_str) return Array();
				$base_query['where'][] = $contains_str;
			break;

			case 'metadata':
				$metadata_field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
				$key_types = Array(
								'metadata_field_select'		=> 'selection',
								'metadata_field_thesaurus'	=> 'thesaurus',
							 );
				$key_type = array_get_index($key_types, get_class($metadata_field), 'text');
				$base_query['where'][] = 'ai.type = '.$db->quote($key_type);
				$base_query['where'][] = 'ai.component = '.$db->quote('metadata:'.(int)$data_source['params']['assetid']);

				if (get_class($metadata_field) == 'metadata_field_select' && $metadata_field->attr('multiple')) {
					if (strpos($search_term,' AND ') !== false) {
						$split = ' AND ';
						$split_type = '&';
					} else {
						$split = ' OR ';
						$split_type = '|';
					}

					$searched_terms = explode($split,$search_term);
					$sub_contains_qry = '';
					foreach ($searched_terms as $sterm) {
						// TODO: This needs to be changed to whatever the
						// TODO: regular search manager used
						$sub_contains_qry .= '{[option]'.$sterm.'[/option]}'.$split_type;
					}
					$sub_contains_qry = trim($sub_contains_qry,$split_type);
					$contains_str = ' contains(ai.value, '.$db->quoteSmart($sub_contains_qry).', 1) > 0';
				} else {
					$contains_str = $this->_buildContainsQuery('ai.value', $search_term, 1, (($key_type != 'text')));
				}
				if (!$contains_str) return Array();
				$base_query['where'][] = $contains_str;
			break;

			case 'standard':
				$field = $data_source['params']['field'];
				if (isset($sm->standard_text_fields[$field])) {
					$base_query['where'][] = 'ai.component = '.$db->quote('__'.$field.'__');

					$contains_str = $this->_buildContainsQuery('ai.value', $search_term);
					if (!$contains_str) return Array();
					$base_query['where'][] = $contains_str;
				} else {
					trigger_error('Invalid standard search field '.$field, E_USER_WARNING);
				}
			break;

		}
		$sql = implode_sql($base_query);
		if (count($base_query['select']) > 2) {
			$result = $db->getAll($sql);
		} else {
			$result = $db->getAssoc($sql);
		}

		assert_valid_db_result($result);

		return $result;

	}//end processWordSearch()


	/**
	* Search for a numeric range in a particular data source
	*
	* The $numeric_range field is a two-value array containing 'lower' and
	* 'upper' keys. If both are filled, then it is handled as a 'BETWEEN' query.
	* If one is NULL, then it is handled as a 'greater than' or 'less than'
	* query, depending on which one is NULL.
	*
	* If both are NULL, then no condition is imposed, and it degenerates into
	* the base query.
	*
	* @param array	$numeric_range	Range of numbers to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return void
	* @access private
	*/
	function processNumericSearch(&$sm, $numeric_range, $data_source, $base_query)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		switch ($data_source['type']) {

			case 'asset_attrib':
				$attrid = $data_source['params']['attrid'];
				$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
				$attr_info = $attr_info[$attrid];

				$attr_type = $attr_info['type'];
				$base_query['where'][] = 'ai.component = '.$db->quote('attr:'.$attr_info['name']);

				if ($numeric_range['upper'] == $numeric_range['lower']) {
					$search_compare = '= '.$db->quoteSmart($numeric_range['upper']);
				} else if (is_null($numeric_range['upper'])) {
					$search_compare = '>= '.$db->quoteSmart($numeric_range['lower']);
				} else if (is_null($numeric_range['lower'])) {
					$search_compare = '<= '.$db->quoteSmart($numeric_range['upper']);
				} else if ($numeric_range['upper'] > $numeric_range['lower']) {
					// upper > lower as expected
					$search_compare = 'BETWEEN '.$db->quoteSmart($numeric_range['lower']).' AND '.$db->quoteSmart($numeric_range['upper']);
				} else {
					// accept lower > upper but we need to switch values because
					// some DB engines won't accept BETWEEN upper AND lower
					$search_compare = 'BETWEEN '.$db->quoteSmart($numeric_range['upper']).' AND '.$db->quoteSmart($numeric_range['lower']);
				}

				if ($attr_type == 'int') {
					$base_query['where'][] = 'CAST (ai.value AS int) '.$search_compare;
				} else {
					$base_query['where'][] = 'CAST(ai.value AS double precision) '.$search_compare;
				}
			break;

			case 'metadata':
				// not implemented yet
			break;

			case 'standard':
				// not implemented yet
			break;

		}//end switch $data_source['type']
		$sql = implode_sql($base_query);

		if (count($base_query['select']) > 2) {
			$result = $db->getAll($sql);
		} else {
			$result = $db->getAssoc($sql);
		}

		assert_valid_db_result($result);

		return $result;

	}//end processNumericSearch()


	/**
	* Search for a date range in a particular data source
	*
	* The $date_range field is a two-value array containing 'from' and 'to'
	* keys. Specifying both will make it a "between" query. Specifying one and
	* leaving the other as NULL will make it a greater/less than (or equal to)
	* query.
	*
	* If both 'from' and 'to' are NULL, then no condition is imposed, and it
	* degenerates into the base query.
	*
	* @param array	$date_range		Range of dates to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return void
	* @access private
	*/
	function processDateSearch(&$sm, $date_range, $data_source, $base_query)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		switch ($data_source['type']) {

			case 'asset_attrib':
				// TODO: restrict by asset type here?
				$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);
				$base_query['where'][] = 'ai.component = '.$db->quote('attr:'.$attribute->name);
			break;

			case 'metadata':
				$metadata_field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($attr_info['params']['assetid']);
				$key_types = Array(
								'metadata_field_select'		=> 'selection',
								'metadata_field_thesaurus'	=> 'thesaurus',
							 );
				$key_type = array_get_index($key_types, get_class($metadata_field), 'text');
				$base_query['where'][] = 'ai.type = '.$db->quote($key_type);
				$base_query['where'][] = 'ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']);
			break;

			case 'standard':
				$field = $data_source['params']['field'];
				if (isset($sm->standard_date_fields[$field])) {
					$base_query['where'][] = 'ai.component = '.$db->quote('__'.$field.'__');
				} else {
					trigger_error('Invalid standard date search field '.$field, E_USER_WARNING);
				}
			break;

		}
		if ($date_range['from'] != '---------- --:--:--') {
			$base_query['where'][] = $this->_getDateCompareWithClobSql('ai.value', $db->quote($date_range['from']), '>=');
		}
		if ($date_range['to'] != '---------- --:--:--') {
			$base_query['where'][] = $this->_getDateCompareWithClobSql('ai.value', $db->quote($date_range['to']),  '<=');
		}
		$sql = implode_sql($base_query);
		if (count($base_query['select']) > 2) {
			$result = $db->getAll($sql);
		} else {
			$result = $db->getAssoc($sql);
		}

		assert_valid_db_result($result);

		return $result;

	}//end processDateSearch()


	/**
	* Search for terms in all components
	*
	* This can be used for an include all or exclude search - Search Manager
	* will likely use it for the latter.
	*
	* @param string	$search_term	Word to search for
	* @param array	$base_query		The base of the sql query to use
	*
	* @return array
	* @access public
	*/
	function processExcludeQuery(&$sm, $search_term, $base_query)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$words = Array();

		foreach ($search_term as $exclude_value) {
			$words = array_merge($words, $this->generateWordList($exclude_value));
		}
		$words = array_unique($words);
		$new_base = $base_query;
		$contains_str = $this->_buildContainsQuery('ai.value', implode(' or ', $words));

		if ($contains_str) {
			$new_base['where'][] = $contains_str;
			$new_base['select'] = Array('a.assetid');
			$new_base['group_by'] = Array('a.assetid');
			$sql = implode_sql($new_base);
			$result = $db->getCol($sql);
			assert_valid_db_result($result);
		} else {
			$result = Array();
		}

		return $result;

	}//end processExcludeSearch()


	/**
	* Populates a search query array given an array of search settings
	*
	* Accepts the same search info array as processSearch()
	* Does not put the search index table or anything like that in the query
	* Returns components of search query in a format ready for implode_sql()
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see Search_Manager::processSearch(), general_occasional.inc::implode_sql()
	*/
	function constructBaseSearchQuery($search_info)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$query = Array(
					'select'		=> Array(),
					'from'			=> Array('sq_ast a'),
					'where'			=> Array(),
					'where_joiner'	=> 'AND',
					'order_by'		=> Array(),
				 );

		// TREE LOCATIONS
		if (!empty($search_info['roots'])) {
			$root_logic = array_get_index($search_info, 'root_logic', 'OR');
			// get the treeids of our search roots
			$treeid_sql = 'SELECT l.minorid, t.treeid
						   FROM sq_ast_lnk_tree t
							   JOIN sq_ast_lnk l on t.linkid = l.linkid
						   WHERE l.minorid IN ('.implode(',', $search_info['roots']).')';
			// getAssoc only gives us the first treeid for each minorid, which is actually just what we want!
			$root_treeids = $db->getAssoc($treeid_sql);
			assert_valid_db_result($root_treeids);

			if ($root_logic == 'AND') {
				foreach (array_values($search_info['roots']) as $i => $rootid) {
					$treeid = $root_treeids[$rootid];
					$query['where'][] = ' EXISTS (
											SELECT 1 FROM
												sq_ast_lnk l INNER JOIN sq_ast_lnk_tree t ON l.linkid = t.linkid
											WHERE
													a.assetid = l.minorid
											AND t.treeid LIKE '.$db->quoteSmart($treeid.'%').'
										)';
				}
			} else {
				$sql = 'EXISTS (
							SELECT 1 FROM
								sq_ast_lnk l INNER JOIN sq_ast_lnk_tree t ON l.linkid = t.linkid
							WHERE
								a.assetid = l.minorid
							AND ';

				$treeids_where = Array();
				foreach ($root_treeids as $treeid) {
							$treeids_where[] = 't.treeid LIKE '.$db->quoteSmart($treeid.'%');
				}
				$sql .= '('.implode(' OR ', $treeids_where).')';
				$query['where'][] = $sql.')';
			}
		}

		// ACCESS RESTRICTIONS
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if ($user_restrictions) {
			$query['from'][] = 'sq_ast_perm ap';
			$query['where'][] = 'a.assetid = ap.assetid';
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user'), $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quote($userids[$i]);
			}
			$query['where'][] = 'ap.userid  IN ('.implode(',', $userids).')';

		}

		// STATUS RESTRICTIONS
		if (!empty($search_info['statuses'])) {
			$statuses = $search_info['statuses'];
			if (array_sum($statuses) != SQ_SC_STATUS_ALL) {
				foreach ($statuses as $i => $status) {
					$statuses[$i] = $db->quote($status);
				}
				$query['where'][] = 'a.status IN ('.implode(', ', $statuses).')';
			}
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$query['where'][] = 'a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		// ASSET TYPE RESTRICTIONS
		if (!empty($search_info['asset_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['asset_types']); NULL !== ($i = key($search_info['asset_types'])); next($search_info['asset_types'])) {
				if ($search_info['asset_types'][$i]) {
					$inherited_types[] = $db->quote($i);
				} else {
					$normal_types[] = $db->quote($i);
				}
			}

			$type_code_cond = Array();
			if (!empty($inherited_types)) {
				$type_code_cond[] = 'inhd_type_code IN ('.implode(', ', $inherited_types).')';
			}
			if (!empty($normal_types)) {
				$type_code_cond[] = 'type_code IN ('.implode(', ', $normal_types).')';
			}
			$type_code_cond = implode(' OR ', $type_code_cond);
			$query['where'][] = 'a.type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE '.$type_code_cond.'
								)';
		}

	}//end constructBaseSearchQuery()


	/**
	* Extracts keywords from the indexing table and ranks them in order of
	* descending importance
	*
	* If $include_scores is TRUE, the returned array is of (keyword => score).
	* If it is FALSE, the array is of (keyword) without any particular key
	*
	* @param object		&$asset				The asset whose keywords are to be extracted
	* @param boolean	$include_metadata	If false, metadata components are excluded
	* @param boolean	$include_scores		Determines whether scores are returned
	*										(true) or just the keywords (false)
	*
	* @return array
	* @access public
	*/
	function extractKeywords(&$asset, $include_metadata=FALSE, $include_scores=FALSE)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// The SUBSTR stuff is to turn the CLOB field of value into a VARCHAR
		// that can be handled by the LOWER() function
		$sql = 'SELECT
					LOWER(DBMS_LOB.SUBSTR(value, DBMS_LOB.GETLENGTH(value), 1)) as keyword, SUM(score) as total_score
				FROM
					sq_sch_idx
				WHERE
					assetid = '.$db->quoteSmart($asset->id).
				  ($include_metadata ? '' : ' AND component NOT LIKE '.$db->quoteSmart('metadata:%')).'
				GROUP BY
					 LOWER((DBMS_LOB.SUBSTR(value, DBMS_LOB.GETLENGTH(value), 1)))
				ORDER BY
					total_score DESC';

		$result = $db->getAssoc($sql);
		assert_valid_db_result($result);

		// force lowercase keys (= keywords) - CASE_LOWER is default for this function
		$result = array_change_key_case($result);

		if ($include_scores) {
			return $result;				// return result with scores
		} else {
			return array_keys($result);	// just return the keywords
		}

	}//end extractKeywords()


	/**
	* Breaks apart a string into a list of words so the search manager can use it
	*
	* For Oracle, we actually don't want to break up the string, so that Oracle
	* Text can have a look at the whole string. So we return the word string back
	* as a single element array
	*
	* @param string	$words	A string of words
	*
	* @return array
	* @access public
	*/
	function generateWordList($words)
	{
		return Array($words);

	}//end generateWordList()


	/**
	* Returns the indexable content for the specified asset's attributes
	*
	* In Oracle-based systems, for Oracle Text's OR searching to work properly,
	* attribute content is indexed as a whole, rather than being broken up into
	* separate words.
	*
	* @param string	$index_content
	*
	* @return Array
	* @access public
	*/
	function splitIndexableContent($contents, $type_code, $data_type, $component, $weighting)
	{
		$index_content = Array();

		$index_content[] = Array(
								'value'		=> $contents,
								'type_code'	=> $type_code,
								'type'		=> $data_type,
								'component'	=> $component,
								'score'		=> $weighting,
							   );

		return $index_content;

	}//end splitIndexableContent()


	/**
	* Returns the content to be indexed for a multiple metadata select value
	*
	* By default, values will be stored as a semi-colon separated list; just
	* send that back if you don't want that changed.
	*
	* @param string	$value	semi-colon separated list of selected values
	*
	* @return string
	* @access public
	*/
	function handleMultipleMetadataSelect($value)
	{
		$components = explode('\;', $value);
		$value = '';
		foreach ($components as $comp) {
			$comp = trim($comp);
			$value .= '[option]'.$comp.'[/option]';
		}
		return $value;

	}//end handleMultipleMetadataSelect()

	//--        ORACLE TEXT EXTENSION FUNCTIONS        --//


	/**
	* Parses the brackets found within the terms to ensure
	* that all opening brackets have a closing bracket.
	*
	* @param &$string $terms the terms
	*
	* @return boolean TRUE if a reparse is required
	* @access private
	*/
	function _parseBrackets(&$terms)
	{
		if ($terms == '') return false;
		$reparse = false;
		$prev_terms = $terms;

		$open_brackets  = 0;
		$close_brackets = 0;
		$last_close     = -1;
		$last_open      = -1;

		for ($i = 0; $i < strlen($terms); $i++) {
			if ($terms{$i} == ')') {
				$last_close = $i;
				if (($last_open != -1 && $last_close != -1) && $last_open < $last_close) {
					if ($open_brackets == 0) {
						$close_brackets++;
						$last_open = -1;
					} else {
						$open_brackets--;
					}
				} else {
					$close_brackets++;
				}
			} else if ($terms{$i} == '(') {
				$open_brackets++;
				$last_open = $i;
			}
		}

		if ($close_brackets != $open_brackets) {
			if ($close_brackets > 0) {
				$terms = str_repeat('(', $close_brackets).$terms;
				$reparse = true;
			}
			if ($open_brackets > 0) {
				$terms = $terms.str_repeat(')', $open_brackets);
				$reparse = true;
			}
		} else if ($close_brackets > 0) {
			$terms = '('.$terms.')';
		}

		// remove any empty brackets
		while (substr_count($terms, '()') != 0) {
			$terms = str_replace('()', '', $terms);
		}

		return ($prev_terms != $terms || $reparse);

	}//end _parseBrackets()


	/**
	* Parses the operators to ensure they are in valid locations
	* and applied on valid operands
	*
	* @param &string $terms the terms
	*
	* @return boolean TRUE if a reparse is required
	* @access private
	*/
	function _parseOperators(&$terms)
	{
		$prev_terms = $terms;
		$operators = Array('|', '&', '~');

		// replace all occurances of 2 operators next to eachother with
		// the second operator. We need the while loop because if there exists
		// more than 2 operators next to eachother, we will reduce them by half
		// and the next foreach iteration may have passed the new set of operators
		// next to eachother

		$pattern_found = true;
		while ($pattern_found) {
			$pattern_found = false;
			foreach ($operators as $op1) {
				foreach ($operators as $op2) {
					$prev_terms = $terms;
					$terms = preg_replace('/(\\'.$op1.')(\\'.$op2.')/', '\\2', $terms);
					if ($prev_terms != $terms) $pattern_found = true;
				}
			}
		}

		// the % indicates where operators are not permitted
		$bad_op_patterns = Array(
							'/^%(.+)/',
							'/(.+)%$/',
							'/(\()%(.+)/',
							'/(.+)%(\))/',
							'/^%$/',
						  );

		// strip out any operators that are in the wrong place
		foreach ($operators as $op) {
			foreach ($bad_op_patterns as $patt) {
				$patt = str_replace('%', '\\'.$op, $patt);
				$terms = preg_replace($patt, '\1\2\3', $terms);
				if ($prev_terms != $terms) $pattern_found = true;
			}
		}

		return ($prev_terms != $terms);

	}//end _parseOperators()


	/**
	* Parses the terms to ensure the are in valid locations
	* and have the valid operators between them
	*
	* @param &$string $terms the terms
	*
	* @return boolean TRUE if a reparse is required
	* @access private
	*/
	function _parseTerms(&$terms)
	{
		$prev_terms = $terms;
		// 1	replace all space seperated words and phrases so that they are 'and'ed together
		//		so that oracle does not perform an exact phrase match on them
		// 2	make sure that there are operators between brackets, if not add the & operator
		// 3	all words next to brackets need an operator between them
		// 4	all phrases next to brackets need an operator between them
		// 5	all phrases and words need an operator between them
		$replacements = Array(
							'/(#)+(#)+/U'					=> '\1&\2',		// 1
							'/(@)+(@)+/U'					=> '\1&\2',		// 1
							'/(\)+)([^\&\|\~\@\#])*(\()/U'	=> '\1&\3',		// 2
							'/(\))(#)/'						=> '\1&\2',		// 3
							'/(#)(\()/'						=> '\1&\2',		// 3
							'/(\))(@)/'						=> '\1&\2',		// 4
							'/(@)(\()/'						=> '\1&\2',		// 4
							'/(#)(@)/'						=> '\1&\2',		// 5
							'/(@)(#)/'						=> '\1&\2',		// 5
						);
		$terms = preg_replace(array_keys($replacements), $replacements, $terms);

		return ($terms != $prev_terms);

	}//end _parseTerms()


	/**
	* Replaces the literal operator strings with their symbols
	* or space if $clear is TRUE
	*
	* @param string		$terms	the terms
	* @param boolean	$clear	if TRUE the operators are replaced with space
	*
	* @return string the replaced terms
	* @access private
	*/
	function _replaceOperators($terms, $clear=false)
	{
		$ops = Array(
			'and'	=> '&',
			'not'	=> '~',
			'or'	=> '|',
		);

		$sym = ($clear) ? ' ' : '!';

		$pats = Array(
			'/(\W+)%(\W+)/'		=> '\1'.$sym.'\2',
			'/(\W+)%(\W+)/U'	=> '\1'.$sym.'\2',
			'/^%(\W+)/U'		=> $sym.'\1',
			'/(\W+)%$/U'		=> '\1'.$sym.'\2',
			'/^%$/'				=> $sym,
		);

		foreach ($ops as $op => $sym) {
			foreach ($pats as $pat => $rep) {
				$pat = str_replace('%', $op, $pat);
				$rep = str_replace('!', $sym, $rep);
				$terms = preg_replace($pat, $rep, $terms);
			}
		}
		return $terms;

	}//end _replaceOperators()


	/**
	* Returns the words contained within the terms string.
	*
	* @param &string	$terms				the terms string
	* @param boolean	$remove_stop_words	if TRUE stop words will be removed from the terms string
	* @param boolean	$return_stop_words	if TRUE stop words will be returned in the list
	* @param boolean	$replace			if TRUE the words will be replaced with a #
	*										for building generalization strings
	*
	* @return Array() the words
	*/
	function _getWords(&$terms, $remove_stop_words=false, $return_stop_words=false, $replace=false)
	{
		// replace all words with #
		if (preg_match_all('/(\w+)/', $terms, $words)) {
			// remove any # symbols as we are going to replace words
			// with that symbol
			if ($replace) {
				$terms = preg_replace('/#/', '', $terms);
			}

			$good_words = Array();
			$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

			foreach ($words[1] as $word) {
				if (!$sm->isWordIndexable($word)) {
					if ($remove_stop_words) {
						// remove non-indexable words
						$terms = preg_replace('/\w+/', '', $terms, 1);
					}
					if (!$return_stop_words) continue;
				}
				if ($replace) {
					$terms = preg_replace('/\w+/', '#', $terms, 1);
				}
				$good_words[] = $word;
			}
			return $good_words;
		}
		return Array();

	}//end _getWords()


	/**
	* Builds sql to perform a contains query using the ctx_sys.contains function
	* in the oracle text package.
	*
	* Note that if the length of terms is > 252 characters, the CONTAINS
	* function will not be used (due to CONTAINS itself having a 255 character
	* limit) and will switch to LIKE instead. This is lot slower than the text
	* package function but avoids the possible truncation.
	*
	* @param string		$column		the column with the ctx_context index
	* @param string 	$terms		the terms from the text box (unquoted)
	* @param string 	$score		the score to use in the score() function
	*
	* @return mixed the sql or false if there is nothing to search on
	* @access private
	*/
	function _buildContainsQuery($column, $terms, $score=1, $exact=FALSE)
	{
		$exact_terms = $terms;
		$terms = strtolower($terms);
		// we can only process a string 255 chars or less with CONTAINS
		// anything else, we have to use LIKE
		$use_contains = (strlen($terms) <= 252);

		// remove all empty phrases
		$terms = str_replace('""', '', $terms);

		$good_phrases = Array();
		// get the phrase matching terms and replace them so that
		// we dont override them
		if (preg_match_all('/"([^"]+)"/', $terms, $phrases)) {
			$terms = preg_replace('/@/', '', $terms);

			foreach ($phrases[1] as $phrase) {
				// verify that this phrase does not have any illegal characters.
				$phrase = preg_replace('/[^a-zA-Z0-9\s\.]/', ' ', $phrase);
				// if after bad character removal, the phrase is empty, remove it
				// from the phrase list and replace it in the terms string with a space
				// so that anything around it is seperated

				$phrase = trim($phrase);

				if ($phrase == '') {
					$replacement = ' ';
				} else {
					$good_phrases[] = $phrase;
					$replacement = '@';
				}
				$terms = preg_replace('/"[^"]+"/', $replacement, $terms, 1);
			}
		}// end if match phrases

		$terms = $this->_replaceOperators($terms);
		$terms = preg_replace('/[^a-zA-Z0-9_\&\|\~\@\(\)\s]/', '&', $terms);

		// get the words from the terms string and replace them
		// with #, and remove non-indexable words
		$words = $this->_getWords($terms, true, false, true);

		// remove spaces
		$terms = preg_replace('/\s+/', '', $terms);

		$reparse = true;
		while ($reparse) {
			$reparse = false;
			$reparse |= $this->_parseBrackets($terms);
			$reparse |= $this->_parseOperators($terms);
			$reparse |= $this->_parseTerms($terms);
		}

		$reserved_words = Array(
							'nt',
						  );

		// re-replace the terms
		foreach ($words as $word) {
			if ($exact) {
				$word = '{'.$word.'}';
			} else {
				// If using '%' searching, have to escape wildcards with '{ }'
				// (we think)
				$word = str_replace('_', '{_}', $word);
				$word = str_replace('%', '{%}', $word);
				$word = '%'.$word.'%';
			}
			$terms = preg_replace('/#/', " $word ", $terms, 1);
		}

		// some operators might have slipped in if words
		// were broken apart and now have joined to create
		// a valid operator, so remove them.
		// eg an\d could become and after bad character removal
		$terms = $this->_replaceOperators($terms, true);

		// re-replace the phrase matching terms
		foreach ($good_phrases as $phrase) {
			$terms = preg_replace('/@/', '{'.$phrase.'}', $terms, 1);
		}

		// if there is nothing to search on
		if (!preg_match('/\w+/', str_replace('_', '', $terms))) return false;
		if (trim($terms) == '') return false;

		$multiple = false;
		if (strpos($terms, '|') !== false) {
			$multiple = true;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if ($use_contains) {
			$sql = 'contains('.$column.', '.$db->quoteSmart(trim($terms)).', '.$score.') > 0';
		} else {
			$sql = $db->quoteSmart(trim($terms)).' like '.$db->quoteSmart('%').' || '.$column.' || '.$db->quoteSmart('%');
		}

		if ($exact && !$multiple) {
			$sql .= ' AND length(ai.value) = '.strlen($exact_terms);
		}

		return $sql;

	}//end _buildContainsQuery()


	/**
	* Returns sql for comparing a CLOB field with a date string
	*
	* @param string	$date1		the left date operand
	* @param string	$date2		the right date operand
	* @param string	$operator	the operator to perform the comparison with
	*
	* @return string the sql
	*/
	function _getDateCompareWithClobSql($date1, $date2, $operator='=')
	{
		return 'DBMS_LOB.COMPARE('.$date1.', '.$date2.') '.$operator.' 0';

	}//end _getDateCompareWithClobSql()


}//end class
