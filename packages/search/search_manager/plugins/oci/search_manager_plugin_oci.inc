<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: search_manager_plugin_oci.inc,v 1.29 2013/08/16 07:27:00 ewang Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/text.inc';


/**
* Search_Manager_Plugin_OCI
*
* Handles of certain low-level search functions, optimised to improve database
* performance under Oracle (9i/10g) by using Oracle's text package
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Plugin_OCI
{


	/**
	* Processes a search for a particular word in a particular data source
	*
	* Return value is based on what has been set as the 'select' fields in the
	* base query ($base_query). Typically this will be (keyword => score) but
	* this can obviously be changed.
	*
	* @param string	&$sm			The search manager
	* @param string	$search_term	Word to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	* @param string	$word_logic		operator to use for intra-word logic
	*
	* @return array
	* @access public
	*/
	function processWordSearch(&$sm, $search_term, $data_source, $base_query, $word_logic='AND')
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		switch ($data_source['type']) {

			case 'include_all':
				$contains_str = $this->_buildContainsQuery($sm, 'ai.value', $search_term, $word_logic);
				if (!$contains_str) {
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return Array();
				}
				$base_query['where'][] = $contains_str;
			break;

			case 'asset_attrib':
				$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('attr:'.$attribute->name);

				$contains_str = $this->_buildContainsQuery($sm, 'ai.value', $search_term, $word_logic);
				if (!$contains_str) {
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return Array();
				}
				$base_query['where'][] = $contains_str;
			break;

			case 'metadata':
				$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
				$key_types = Array(
								'metadata_field_hierarchy'		=> 'selection',
								'metadata_field_select'			=> 'selection',
								'metadata_field_multiple_text'	=> 'selection',
								'metadata_field_thesaurus'		=> 'thesaurus',
								'metadata_field_wysiwyg'		=> 'metadata_field_wysiwyg',
							 );
				$key_type = array_get_index($key_types, get_class_lower($metadata_field), 'text');
				$base_query['where'][] = 'ai.type = '.MatrixDAL::quote($key_type);
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('metadata:'.(int)$data_source['params']['assetid']);

				if (get_class_lower($metadata_field) == 'metadata_field_select' && $metadata_field->attr('multiple')) {
					if (strpos($search_term,' AND ') !== FALSE) {
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
					$contains_str = ' contains(ai.value, '.MatrixDAL::quote($sub_contains_qry).', 1) > 0';
				} else {

					$contains_str = $this->_buildContainsQuery($sm, 'ai.value', $search_term, $word_logic, 1, ($key_type != 'text' && $key_type != 'metadata_field_wysiwyg' && get_class_lower($metadata_field) != 'metadata_field_multiple_text' && ($key_type != 'thesaurus')));
				}
				if (!$contains_str) {
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return Array();
				}
				$base_query['where'][] = $contains_str;
			break;

			case 'standard':
				$field = $data_source['params']['field'];
				if (isset($sm->standard_text_fields[$field])) {
					$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('__'.$field.'__');

					$contains_str = $this->_buildContainsQuery($sm, 'ai.value', $search_term, $word_logic);
					if (!$contains_str) {
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return Array();
					}
					$base_query['where'][] = $contains_str;
				} else {
					trigger_error('Invalid standard search field '.$field, E_USER_WARNING);
				}
			break;

		}//end switch

		$bind_vars = array_get_index($base_query, 'bind_vars', Array());
		$sql = implode_sql($base_query);
		$query = MatrixDAL::preparePDOQuery($sql);
		foreach ($bind_vars as $bind_var => $value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $value);
		}//end foreach
		if (count($base_query['select']) > 2) {
			$result = MatrixDAL::executePdoAssoc($query);
		} else {
			$result = MatrixDAL::executePdoGroupedAssoc($query);
			// (3.18) only selecting assetid and search score
			// use old format and don't worry about result format later
			$result_old_format = Array();
			foreach ($result as $assetid => $info) {
				$result_old_format[$assetid] = $info[0]['search_score'];
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return $result_old_format;
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $result;

	}//end processWordSearch()


	/**
	* Search for a numeric range in a particular data source
	*
	* The $numeric_range field is a two-value array containing 'lower' and
	* 'upper' keys. If both are filled, then it is handled as a 'BETWEEN' query.
	* If one is NULL, then it is handled as a 'greater than' or 'less than'
	* query, depending on which one is NULL.
	* If both are NULL, then no condition is imposed, and it degenerates into
	* the base query.
	*
	* @param string	&$sm			The search manager
	* @param array	$numeric_range	Range of numbers to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return void
	* @access private
	*/
	function processNumericSearch(&$sm, $numeric_range, $data_source, $base_query)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		switch ($data_source['type']) {

			case 'asset_attrib':
				$attrid = $data_source['params']['attrid'];
				$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
				$attr_info = $attr_info[$attrid];

				$attr_type = $attr_info['type'];
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('attr:'.$attr_info['name']);

				if ($numeric_range['upper'] == $numeric_range['lower']) {
					$search_compare = '= '.MatrixDAL::quote($numeric_range['upper']);
				} else if (is_null($numeric_range['upper'])) {
					$search_compare = '>= '.MatrixDAL::quote($numeric_range['lower']);
				} else if (is_null($numeric_range['lower'])) {
					$search_compare = '<= '.MatrixDAL::quote($numeric_range['upper']);
				} else if ($numeric_range['upper'] > $numeric_range['lower']) {
					// upper > lower as expected
					$search_compare = 'BETWEEN '.MatrixDAL::quote($numeric_range['lower']).' AND '.MatrixDAL::quote($numeric_range['upper']);
				} else {
					// accept lower > upper but we need to switch values because
					// some DB engines won't accept BETWEEN upper AND lower
					$search_compare = 'BETWEEN '.MatrixDAL::quote($numeric_range['upper']).' AND '.MatrixDAL::quote($numeric_range['lower']);
				}

				if ($attr_type == 'int') {
					$base_query['where'][] = 'CAST(CAST(ai.value AS varchar(255)) AS integer) '.$search_compare;
				} else {
					$base_query['where'][] = 'CAST(CAST(ai.value AS varchar(255)) AS double precision) '.$search_compare;
				}
			break;

			case 'metadata':
				// not implemented yet
			break;

			case 'standard':
				// not implemented yet
			break;

		}//end switch $data_source['type']
		$bind_vars = array_get_index($base_query, 'bind_vars', Array());
		$sql = implode_sql($base_query);
		$query = MatrixDAL::preparePDOQuery($sql);
		foreach ($bind_vars as $bind_var => $value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $value);
		}//end foreach
		if (count($base_query['select']) > 2) {
			$result = MatrixDAL::executePdoAssoc($query);
		} else {
			$result = MatrixDAL::executePdoGroupedAssoc($query);
			// (3.18) only selecting assetid and search score
			// use old format and don't worry about result format later
			$result_old_format = Array();
			foreach ($result as $assetid => $info) {
				$result_old_format[$assetid] = $info[0]['search_score'];
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return $result_old_format;
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $result;

	}//end processNumericSearch()


	/**
	* Search for a date range in a particular data source
	*
	* The $date_range field is a two-value array containing 'from' and 'to'
	* keys. Specifying both will make it a "between" query. Specifying one and
	* leaving the other as NULL will make it a greater/less than (or equal to)
	* query.
	* If both 'from' and 'to' are NULL, then no condition is imposed, and it
	* degenerates into the base query.
	*
	* @param string	&$sm			The search manager
	* @param array	$date_range		Range of dates to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return void
	* @access private
	*/
	function processDateSearch(&$sm, $date_range, $data_source, $base_query)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		switch ($data_source['type']) {

			case 'asset_attrib':
				// TODO: restrict by asset type here?
				$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('attr:'.$attribute->name);
			break;

			case 'metadata':
				$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
				$key_types = Array(
								'metadata_field_select'		=> 'selection',
								'metadata_field_thesaurus'	=> 'thesaurus',
								'metadata_field_date'		=> 'datetime',
							 );
				$key_type = array_get_index($key_types, get_class_lower($metadata_field), 'text');
				$base_query['where'][] = 'ai.type = '.MatrixDAL::quote($key_type);
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('metadata:'.(int)$data_source['params']['assetid']);
			break;

			case 'standard':
				$field = $data_source['params']['field'];
				if (isset($sm->standard_date_fields[$field])) {
					$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('__'.$field.'__');
				} else {
					trigger_error('Invalid standard date search field '.$field, E_USER_WARNING);
				}
			break;

		}
		if ($date_range['from'] != '---------- --:--:--') {
			$base_query['where'][] = $this->_getDateCompareWithClobSql('ai.value', MatrixDAL::quote($date_range['from']), '>=');
		}
		if ($date_range['to'] != '---------- --:--:--') {
			$base_query['where'][] = $this->_getDateCompareWithClobSql('ai.value', MatrixDAL::quote($date_range['to']),  '<=');
		}
		$bind_vars = array_get_index($base_query, 'bind_vars', Array());
		$sql = implode_sql($base_query);
		$query = MatrixDAL::preparePDOQuery($sql);
		foreach ($bind_vars as $bind_var => $value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $value);
		}//end foreach
		if (count($base_query['select']) > 2) {
			$result = MatrixDAL::executePdoAssoc($query);
		} else {
			$result = MatrixDAL::executePdoGroupedAssoc($query);
			// (3.18) only selecting assetid and search score
			// use old format and don't worry about result format later
			$result_old_format = Array();
			foreach ($result as $assetid => $info) {
				$result_old_format[$assetid] = $info[0]['search_score'];
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return $result_old_format;
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $result;

	}//end processDateSearch()


	/**
	* Search for terms in all components
	*
	* This can be used for an include all or exclude search - Search Manager
	* will likely use it for the latter.
	*
	* @param string	&$sm			The search manager
	* @param string	$search_term	Word to search for
	* @param array	$base_query		The base of the sql query to use
	*
	* @return array
	* @access public
	*/
	function processExcludeQuery(&$sm, $search_term, $base_query)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		$words = Array();

		foreach ($search_term as $exclude_value) {
			$words = array_merge($words, $this->generateWordList($exclude_value));
		}
		$words = array_unique($words);
		foreach($words as $key => $val) {
			$words[$key] = strtolower($val);
		}

		$new_base = $base_query;
		$contains_str = $this->_buildContainsQuery($sm, 'ai.value', implode(' or ', $words));

		if ($contains_str) {
			$new_base['where'][] = $contains_str;
			$new_base['select'] = Array('ai.assetid');
			$new_base['group_by'] = Array('ai.assetid');
			$bind_vars = array_get_index($new_base, 'bind_vars', Array());
			$sql = implode_sql($new_base);
			$query = MatrixDAL::preparePDOQuery($sql);
			foreach ($bind_vars as $bind_var => $value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $value);
			}//end foreach
			$result = MatrixDAL::executePdoAssoc($query, 0);
		} else {
			$result = Array();
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// Result returned above is in following format:
		// Array(
		// 	'0' => ASSETID1,....
		// )
		//
		// However, the search manager expects the result in following format:
		// Which is same as that returned by posgre
		// Array(
		// 	ASSETID => Array(), ...
		// )
		//
		// Hence, changing the result format to make it readable by search manager.
		if (!empty($result)) {
			$new_result_format = Array();
			foreach($result as $key => $val) {
				$new_result_format[$val] = Array();
			}
			$result = $new_result_format;
		}

		return $result;

	}//end processExcludeQuery()


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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

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
			$root_treeids = MatrixDAL::executeSqlGroupedAssoc($treeid_sql);

			if ($root_logic == 'AND') {
				foreach (array_values($search_info['roots']) as $i => $rootid) {
					$treeid = $root_treeids[$rootid][0]['treeid'];
					$query['where'][] = ' EXISTS (
											SELECT 1 FROM
												sq_ast_lnk l INNER JOIN sq_ast_lnk_tree t ON l.linkid = t.linkid
											WHERE
													a.assetid = l.minorid
											AND substr(t.treeid, 1, '.strlen($treeid).') = '.MatrixDAL::quote($treeid).'
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
				foreach ($root_treeids as $treeid_value) {
							$treeid = $treeid_value[0]['treeid'];
							$treeids_where[] = 'substr(t.treeid, 1, '.strlen($treeid).') = '.MatrixDAL::quote($treeid);
				}
				$sql .= '('.implode(' OR ', $treeids_where).')';
				$query['where'][] = $sql.')';
			}
		}//end if !empty($search_info['roots']

		// ACCESS RESTRICTIONS
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if ($user_restrictions) {
			$query['from'][] = 'sq_ast_perm ap';
			$query['where'][] = 'a.assetid = ap.assetid';
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
			array_push($userids, $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user'), $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = MatrixDAL::quote($userids[$i]);
			}
			$query['where'][] = 'ap.userid  IN ('.implode(',', $userids).')';

		}

		// STATUS RESTRICTIONS
		if (!empty($search_info['statuses'])) {
			$statuses = $search_info['statuses'];
			if (array_sum($statuses) != SQ_SC_STATUS_ALL) {
				foreach ($statuses as $i => $status) {
					$statuses[$i] = MatrixDAL::quote($status);
				}
				$query['where'][] = 'a.status IN ('.implode(', ', $statuses).')';
			}
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$query['where'][] = 'a.status  >= '.MatrixDAL::quote(SQ_STATUS_LIVE);
		}

		// ASSET TYPE RESTRICTIONS
		if (!empty($search_info['asset_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['asset_types']); NULL !== ($i = key($search_info['asset_types'])); next($search_info['asset_types'])) {
				if ($search_info['asset_types'][$i] == 1) {
					$inherited_types[] = MatrixDAL::quote($i);
				} else {
					$normal_types[] = MatrixDAL::quote($i);
				}
			}

			$type_code_cond = Array();
			// if we have inherited types and/or normal types
			if (!empty($inherited_types)) {
				$type_code_cond[] = 'inhd_type_code IN ('.implode(', ', $inherited_types).')';
				if (!empty($normal_types)) {
					$type_code_cond[] = 'type_code IN ('.implode(', ', $normal_types).')';
				}
				$type_code_cond = implode(' OR ', $type_code_cond);
				$query['where'][] = 'a.type_code IN (
											SELECT type_code
											FROM sq_ast_typ_inhd
											WHERE '.$type_code_cond.'
										)';
			} else {
				// if we only got normal type instead we are not using the subquery
				$query['where'][] = 'a.type_code IN ('.implode(', ', $normal_types).')';
			}
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $query;

	}//end constructBaseSearchQuery()


	/**
	* Extracts keywords from the indexing table and ranks them in order of
	* descending importance
	*
	* If $include_scores is TRUE, the returned array is of (keyword => score).
	* If it is FALSE, the array is of (keyword) without any particular key
	*
	* @param object		&$asset				The asset whose keywords are to be extracted
	* @param boolean	$include_metadata	If FALSE, metadata components are excluded
	* @param boolean	$include_scores		Determines whether scores are returned
	*										(TRUE) or just the keywords (FALSE)
	*
	* @return array
	* @access public
	*/
	function extractKeywords(&$asset, $include_metadata=FALSE, $include_scores=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');
		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

		// The SUBSTR stuff is to turn the CLOB field of value into a VARCHAR
		// that can be handled by the LOWER() function
		$sql = 'SELECT
					value, type, score
				FROM
					sq_sch_idx
				WHERE
					assetid = '.MatrixDAL::quote($asset->id).
				  ($include_metadata ? '' : ' AND component NOT LIKE '.MatrixDAL::quote('metadata:%'));

		$result = MatrixDAL::executeSqlAssoc($sql);

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// TODO: change it so the words are split up
		$word_scores = Array();

		foreach ($result as $result_item) {
			$word_string = strtolower($result_item['value']);
			if ($result_item['type'] == 'text') {
				$words = preg_split('|\s+|', $word_string);
			} else {
				$words = Array($word_string);
			}
			$words = array_count_values($words);
			foreach ($words as $word => $score) {
				$word = trim($word);
				if ($sm->isWordIndexable($word)) {
					if (!isset($word_scores[$word])) {
						$word_scores[$word] = 0;
					}
					$word_scores[$word] += $score * $result_item['score'];
				}
			}
		}

		arsort($word_scores);

		if ($include_scores) {
			return $word_scores;				// return result with scores
		} else {
			return array_keys($word_scores);	// just return the keywords
		}

	}//end extractKeywords()


	/**
	* Breaks apart a string into a list of words so the search manager can use it
	*
	* For Oracle, we actually don't want to break up the string, so that Oracle
	* Text can have a look at the whole string. So we return the word string back
	* as a single element array
	*
	* @param string|array	$words	A string of words, or an array of string
	*
	* @return array
	* @access public
	*/
	function generateWordList($words)
	{
		if (is_array($words)) {
			return $words;
		} else if (is_string($words)) {
			//convert wildcard symbols to corresponding SQL wildcards
			$words = strtr($words, '*?', '%_');

			//invoke remove_silent_chars in text.inc
			$words = remove_silent_chars($words, TRUE, TRUE);

			return Array($words);
		}

	}//end generateWordList()


	/**
	* Returns the indexable content for the specified asset's attributes
	*
	* In Oracle-based systems, for Oracle Text's OR searching to work properly,
	* attribute content is indexed as a whole, rather than being broken up into
	* separate words.
	*
	* @param string				$contents		The contents
	* @param string				$type_code		The type code
	* @param string				$data_type		The data type
	* @param string				$component		The component
	* @param string				$weighting		The weighting
	* @param mixed (int|string)	$contextid		The contextid (or 'default' if not contextable)
	* @param boolean			$use_default	TRUE indicates this is a default value
	*
	* @return array
	* @access public
	*/
	function splitIndexableContent($contents, $type_code, $data_type, $component, $weighting, $contextid, $use_default)
	{
		$index_content = Array();

		//only use mb_strtolower() function if it is supported
		if (function_exists('mb_strtolower')) {
			//use mbstring function because the input contents can be different from ASCII characters
			$contents = mb_strtolower($contents, SQ_CONF_DEFAULT_CHARACTER_SET);
		} else {
			//lowercase the string which has non-ASCII characters by encoding them to HTML entities
			$contents = htmlentities($contents, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
			$contents = strtolower($contents);
			$contents = html_entity_decode($contents, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
		}
		$contents = str_replace('&nbsp;', ' ', $contents);
		$contents = trim($contents);

		if (($data_type == 'text') || ($data_type == 'wysiwyg')) {
			//the wysiwyg tool converts special characters to html entities in the HTML editor so they need to be converted back
			$contents = html_entity_decode($contents, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
		}

		$index_content[] = Array(
							'value'			=> $contents,
							'type_code'		=> $type_code,
							'type'			=> $data_type,
							'component'		=> $component,
							'score'			=> $weighting,
							'contextid'		=> $contextid,
							'use_default'	=> $use_default,
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
		$components = explode('; ', $value);
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
	* returns TRUE if a reparse is required
	*
	* @param string	&$terms	the terms
	*
	* @return boolean
	* @access private
	*/
	function _parseBrackets(&$terms)
	{
		if ($terms == '') return FALSE;
		$reparse = FALSE;
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
				$reparse = TRUE;
			}
			if ($open_brackets > 0) {
				$terms = $terms.str_repeat(')', $open_brackets);
				$reparse = TRUE;
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
	* returns TRUE if a reparse is required
	*
	* @param string	&$terms	the terms
	*
	* @return boolean
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

		$pattern_found = TRUE;
		while ($pattern_found) {
			$pattern_found = FALSE;
			foreach ($operators as $op1) {
				foreach ($operators as $op2) {
					$prev_terms = $terms;
					$terms = preg_replace('/(\\'.$op1.')(\\'.$op2.')/', '\\2', $terms);
					if ($prev_terms != $terms) $pattern_found = TRUE;
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
				if ($prev_terms != $terms) $pattern_found = TRUE;
			}
		}

		return ($prev_terms != $terms);

	}//end _parseOperators()


	/**
	* Parses the terms to ensure the are in valid locations
	* and have the valid operators between them
	*
	* returns TRUE if a reparse is required
	*
	* @param string	&$terms		the terms
	* @param string	$word_logic	operator to use for intra-word logic
	*
	* @return boolean
	* @access private
	*/
	function _parseTerms(Search_Manager $sm, &$terms, $word_logic)
	{
		$prev_terms = $terms;
		// 1	replace all space seperated words and phrases so that they are 'and'ed together
		// 		so that oracle does not perform an exact phrase match on them
		// 2	make sure that there are operators between brackets, if not add the & operator
		// 3	all words next to brackets need an operator between them
		// 4	all phrases next to brackets need an operator between them
		// 5	all phrases and words need an operator between them

		$op = ($word_logic == 'OR') ? '|' : '&';

		$replacements = Array(
							'/(#)+(#)+/U'					=> '\1'.$op.'\2',
							'/(@)+(@)+/U'					=> '\1'.$op.'\2',
							'/(\)+)([^\&\|\~\@\#])*(\()/U'	=> '\1'.$op.'\3',
							'/(\))(#)/'						=> '\1'.$op.'\2',
							'/(#)(\()/'						=> '\1'.$op.'\2',
							'/(\))(@)/'						=> '\1'.$op.'\2',
							'/(@)(\()/'						=> '\1'.$op.'\2',
							'/(#)(@)/'						=> '\1'.$op.'\2',
							'/(@)(#)/'						=> '\1'.$op.'\2',
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
	* @return string
	* @access private
	*/
	function _replaceOperators($terms, $clear=FALSE)
	{
		$ops = Array(
				'and'	=> '&',
				'not'	=> '~',
				'or'	=> '|',
			   );

		$sym = ($clear) ? ' ' : '!';

		$match_chars = '[\r\n\t ]';

		$pats = Array(
					'/('.$match_chars.'+)%('.$match_chars.'+)/'		=> '\1'.$sym.'\2',
					'/('.$match_chars.'+)%('.$match_chars.'+)/U'	=> '\1'.$sym.'\2',
					'/^%('.$match_chars.'+)/U'						=> $sym.'\1',
					'/('.$match_chars.'+)%$/U'						=> '\1'.$sym.'\2',
					'/^%$/'											=> $sym,
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
	* @param string		&$terms				the terms string
	* @param boolean	$remove_stop_words	if TRUE stop words will be removed from the terms string
	* @param boolean	$return_stop_words	if TRUE stop words will be returned in the list
	* @param boolean	$replace			if TRUE the words will be replaced with a #
	*										for building generalization strings. Non-indexable words
	*										will be stripped completely instead of being #'d.
	* @param array		$ignore_chars		a list of characters that will be ignored and not added to the words array nor stripped from the $terms list
	*
	* @return array
	* @access private
	*/
	function _getWords(&$terms, $remove_stop_words=FALSE, $return_stop_words=FALSE, $replace=FALSE, $ignore_chars=Array())
	{
		$preg_ignore_chars = Array();
		foreach ($ignore_chars as $char) {
			$preg_ignore_chars[] = preg_quote($char, '/');
		}

		// We need to not only match (or NOT match, rather) whitespace as word
		// breaks, but also the operators themselves. When we replace with
		// hashes we need to match the hash so it doesn't match with a previous
		// one
		$match_chars   = '[^\r\n\t \(\)\|\&\~\%]';
		$replace_chars = '[^\r\n\t \(\)\|\&\~\%#'.implode('', $preg_ignore_chars).']';

		// replace all words with #
		if (preg_match_all('/('.$match_chars.'+)/', $terms, $words)) {
			// remove any # symbols as we are going to replace words
			// with that symbol
			if ($replace) {
				$terms = preg_replace('/#/', '', $terms);
			}

			$good_words = Array();
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

			foreach ($words[1] as $word) {
				if (empty($ignore_chars) || !in_array($word, $ignore_chars)) {
					// If the word is indexable, then replace it with a # symbol
					// (if we are replacing) and add it to the list of
					// replacement words we'll put back in later
					if ($sm->isWordIndexable($word)) {
						if ($replace) {
							$terms = preg_replace('/'.$replace_chars.'+/', '#', $terms, 1);
						}

						$good_words[] = $word;

					} else {
						// We're removing the word (if we are replacing - ie.
						// we're replacing it with nothing) if it is not
						// something that we can search on
						if ($replace) {
							$terms = preg_replace('/'.$replace_chars.'+/', '', $terms, 1);
						}

					}
				}
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
	* @param Search_Manager	$sm			The search manager
	* @param string			$column		the column with the ctx_context index
	* @param string			$terms		the terms from the text box (unquoted)
	* @param string			$word_logic	operator to use for intra-word logic
	* @param string			$score		the score to use in the score() function
	* @param string			$exact		whether to require an exact match
	*
	* @return mixed string|boolean
	* @access private
	*/
	function _buildContainsQuery(Search_Manager $sm, $column, $terms, $word_logic='AND', $score=1, $exact=FALSE)
	{
		$phrase_placeholder      = '@';
		$preg_phrase_placeholder = preg_quote($phrase_placeholder, '/');

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
			$terms = preg_replace('/'.$preg_phrase_placeholder.'/', '', $terms);

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
					$replacement = $phrase_placeholder;
				}
				$terms = preg_replace('/"[^"]+"/', $replacement, $terms, 1);
			}
		}// end if match phrases

		$terms = $this->_replaceOperators($terms);
		$terms = preg_replace('/[^a-zA-Z0-9_\&\|\~'.$preg_phrase_placeholder.'\(\)\s]/', '&', $terms);

		// get the words from the terms string and replace them
		// with #, and remove non-indexable words
		// (make an exception for @, as it's our search phrase placeholder)
		$words = $this->_getWords($terms, TRUE, FALSE, TRUE, Array($phrase_placeholder));

		// remove spaces
		$terms = preg_replace('/\s+/', '', $terms);

		$reparse = TRUE;
		while ($reparse) {
			$reparse = FALSE;
			$reparse |= $this->_parseBrackets($terms);
			$reparse |= $this->_parseOperators($terms);
			$reparse |= $this->_parseTerms($sm, $terms, $word_logic);
		}

		// re-replace the terms
		foreach ($words as $word) {
			if ($exact) {
				$word = '{'.$word.'}';
			} else {
				// If using '%' searching, have to escape wildcards with '{ }'
				// (we think)
				$word = str_replace('_', '{_}', $word);
				$word = str_replace('%', '{%}', $word);
			}

			$word = $this->_escapeContainReservedChars($word);
			$word = $this->_escapeContainReservedWords($word);

			if (!$exact) {
				// Naming of this attribute is not good. If TRUE, full searching
				// is actually enabled!
				if ($sm->attr('enable_begins_with')) {
					$word = '%'.$word.'%';
				} else {
					$word = $word.'%';
				}
			}

			$terms = preg_replace('/#/', " $word ", $terms, 1);
		}

		// some operators might have slipped in if words
		// were broken apart and now have joined to create
		// a valid operator, so remove them.
		// eg an\d could become and after bad character removal
		$terms = $this->_replaceOperators($terms, TRUE);

		// re-replace the phrase matching terms
		foreach ($good_phrases as $phrase) {
			$terms = preg_replace('/'.$preg_phrase_placeholder.'/', '{'.$phrase.'}', $terms, 1);
		}

		// if there is nothing to search on
		if (!preg_match('/[^\r\n\t ]+/', str_replace('_', '', $terms))) {
			return FALSE;
		}
		if (trim($terms) == '') return FALSE;

		$multiple = FALSE;
		if (strpos($terms, '|') !== FALSE) 	$multiple = TRUE;

		if ($use_contains) {
			$sql = 'contains('.$column.', '.MatrixDAL::quote(trim($terms)).', '.$score.') > 0';
		} else {
			$sql = MatrixDAL::quote(trim($terms)).' like '.MatrixDAL::quote('%').' || '.$column.' || '.MatrixDAL::quote('%');
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
	* @return string
	* @access private
	*/
	function _getDateCompareWithClobSql($date1, $date2, $operator='=')
	{
		return 'DBMS_LOB.COMPARE('.$date1.', '.$date2.') '.$operator.' 0';

	}//end _getDateCompareWithClobSql()


	/**
	* Get ids of assets having same words as the supplied asset
	*
	* @param string	$source_id	ID of the asset
	* @param string	$type		type of the asset to restrict the search by
	*
	* @return array
	* @access public
	*/
	function getAssetidsByWordIntersection($source_id, $type=NULL)
	{
		// load the asset from source_id
		// get all the keywords from this asset
		// get all assets (of the supplied type) that contain any of the keywords in the source asset
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($source_id);
		$keywords = $this->extractKeywords($asset);

		$query_array['select'][] = 'ai.assetid';
		$query_array['from'][] = 'sq_sch_idx ai';

		$query_array['where_joiner'] = 'AND';
		if ($type) {
			$query_array['where'][] = 'ai.type_code = '.MatrixDAL::quote($type);
		}

		$terms = implode(' or ', $keywords);

		// get the words from the terms string and remove non-indexable words
		$words = $this->_getWords($terms, TRUE, FALSE);

		$words = array_unique($words);
		$terms = implode('|', $words);

		$terms_len = strlen($terms);

		$found_ids = Array();

		while ($words) {
			$term_string_built = FALSE;
			$terms ='';
			while (!$term_string_built && !empty($words)) {
				$recent_term = array_pop($words);
				if (strlen($terms.$recent_term) < 200) {
					$escaped_term = $this->_escapeContainReservedChars($recent_term);
					$escaped_term = $this->_escapeContainReservedWords($escaped_term);
					$terms .= empty($terms) ? $escaped_term : '|'.$escaped_term;
				} else {
					array_push($words, $recent_term);
					$term_string_built = TRUE;
				}
			}

			$contains_str = 'contains(value, '.MatrixDAL::quote(trim($terms)).', 1) > 0';

			$paginated_query_array = $query_array;
			$paginated_query_array['where'][] = $contains_str;
			if (!empty($found_ids)) {
				$pag_query_array['where'][] = 'ai.assetid not in (\''.implode($found_ids,'\', \'').'\')';
			}

			$sql = implode_sql($paginated_query_array);
			$result = MatrixDAL::executeSqlAssoc($sql, 0);
			$found_ids = array_merge($found_ids, array_unique($result));
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $found_ids;

	}//end getAssetidsByWordIntersection()


	/**
	* Get the maximum size of a search index field
	*
	* @return mixed int|NULL
	* @access public
	*/
	function getMaxWordLength()
	{
		return NULL;

	}//end getMaxWordLength()


	/**
	* Returns words that are reserved within contains queries escaped with {}'s
	* Because all contains queries are currently done with all lower case strings
	* this function assumes keywords will be in lowercase!
	*
	* @param string	$word	the word to be escaped
	*
	* @return string
	* @access private
	*/
	function _escapeContainReservedWords($word)
	{
		$reserved_words = Array(
							'about'		=> '',
							'accum'		=> '',
							'and'		=> '',
							'bt'		=> '',
							'btg'		=> '',
							'bti'		=> '',
							'btp'		=> '',
							'fuzzy'		=> '',
							'haspath'	=> '',
							'inpath'	=> '',
							'minus'		=> '',
							'near'		=> '',
							'not'		=> '',
							'nt'		=> '',
							'ntg'		=> '',
							'nti'		=> '',
							'ntp'		=> '',
							'or'		=> '',
							'pt'		=> '',
							'rt'		=> '',
							'sqe'		=> '',
							'syn'		=> '',
							'tr'		=> '',
							'trsyn'		=> '',
							'tt'		=> '',
							'within'	=> '',
						  );
		if (isset($reserved_words[$word])) {
			return '{'.$word.'}';
		} else {
			return $word;
		}

	}//end _escapeContainReservedWords()


	/**
	* Returns words with reserved characters escaped for used within "contains" queries.
	*
	* @param string	$word	the word to be escaped
	*
	* @return string
	* @access private
	*/
	function _escapeContainReservedChars($word)
	{
		$reserved_chars = Array('\\',',','?','&','=','-',';','~','|','$','!','>','*','%','_','(',')','[',']','{','}');
		$escaped_chars = Array('\\\\','\,','\?','\&','\=','\-','\;','\~','\|','\$','\!','\>','\*','\%','\_','\(','\)','\[','\]','\{','\}');

		$escaped_word = str_replace($reserved_chars, $escaped_chars, $word);

		return $escaped_word;


	}//end _escapeContainReservedChars()


	/**
	* External access to function that gets the list of search terms
	* from the search string
	*
	* @param string	$search_string	the terms string
	*
	* @return array
	* @access private
	*/
	function getWords($search_string)
	{
		return $this->_getWords($search_string);

	}//end getWords()


	/**
	* Get the list of assets to be printed in Search List Page
	*
	* @param object	&$search_list	the reference to Search List Page asset
	* @param object	&$search_page	the reference to Search Page asset
	*
	* @return array
	* @access public
	*/
	function getSearchListAssets(&$search_list, &$search_page)
	{
		// get information about the search field
		$search_page_fields = $search_page->attr('fields');
		$search_field_name = $search_list->attr('search_field');
		if (empty($search_page_fields) || empty($search_field_name)) {
			// if search field has not been setup properly
			return Array();
		}
		$search_field = $search_page_fields[$search_field_name];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');
		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		$search_info = $search_page->populateBaseSearchInfo();
		$query_comps = $sm->constructBaseSearchQuery($search_info);

		$query_comps['select'][] = 'ai.assetid';

		$data_source_comps = Array();
		$data_source_comps_values = Array();

		foreach ($search_field['data_sources'] as $data_source) {
			switch ($data_source['type']) {
				case 'asset_attrib' :
					$type_code = $data_source['params']['asset_type'];
					$attrid = $data_source['params']['attrid'];

					$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
					$name = $attr_info[$attrid]['name'];
					$data_source_comps[] = 'ai.component = '.MatrixDAL::quote('attr:'.$name);
					$data_source_comps_values[] = MatrixDAL::quote('attr:'.$name);
				break;
				case 'metadata' :
					$assetid = $data_source['params']['assetid'];
					$data_source_comps[] = 'ai.component = '.MatrixDAL::quote('metadata:'.$assetid);
					$data_source_comps_values[] = MatrixDAL::quote('metadata:'.$assetid);
				break;
				case 'standard' :
					$param_field = $data_source['params']['field'];
					$data_source_comps[] = 'ai.component = '.MatrixDAL::quote('__'.$param_field.'__');
					$data_source_comps_values[] = MatrixDAL::quote('__'.$param_field.'__');
				break;

				default :
					return Array();
				break;
			}
		}

		if (!empty($data_source_comps)) {
			$query_comps['where'][] = '('.implode(') OR (', $data_source_comps).')';
		}

		$noise_words	= $sm->attr('noise_word_list');
		$white_words	= $sm->attr('white_word_list');
		$where_cond = '('.implode(') AND (',$query_comps['where']).')';

		$min_length		= $sm->attr('min_word_length');
		$noise_words_str = '"'.implode('", "', $noise_words).'"';
		$white_words_str = '"'.implode('", "', $white_words).'"';
		$sql =	'SELECT * FROM table(
					sq_search_pkg.get_search_list(\''.
						str_replace('\'', '"', $where_cond).'\', '.
						MatrixDAL::quote($min_length).', '.
						MatrixDAL::quote($noise_words_str).', '.
						MatrixDAL::quote($white_words_str).'))';

		$results = MatrixDAL::executeSqlAssoc($sql);
		$children = Array();
		if (!empty($results)) {
			foreach ($results as $ele) {
				$children[$ele['word']] = Array(
											'num_values'	=> $ele['hits'],
											'first_letter'	=> $ele['word']{0},
											'type_code'		=> '',
										  );
			}
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $children;

	}//end getSearchListAssets()


}//end class
