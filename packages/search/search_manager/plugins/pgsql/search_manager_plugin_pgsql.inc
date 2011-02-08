<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: search_manager_plugin_pgsql.inc,v 1.38 2011/02/08 03:08:53 cupreti Exp $
*
*/


/**
* Search_Manager_Plugin_PgSQL
*
* Handles of certain low-level search functions, optimised for a
* PostgreSQL (7.4+) database system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Plugin_PgSQL
{

	//list of silent characters that are not going to be indexed
	private $_silent_chars_to_space; //the list of silent characters that will be converted to space character (' ')
	private $_silent_chars_to_empty; //the list of silent characters that will be converted to empty string ('')

	/**
	 * Constructor
	 *
	 */
	function __construct()
	{
		//These are lists of silent characters that are not going to be indexed. They are listed here instead of remove_silent_chars()
		//function in text.inc because they are common characters but are not ASCII characters. Therefore, they will be encoded
		//differently in different character encodings. That makes the str_replace() function used in remove_silent_chars()
		//function not working properly if static strings are used.
		$silent_chars_to_space = Array('&cent;', '&pound;', '&curren;', '&yen;', '&brvbar;', '&sect;', '&laquo;', '&raquo;', '&plusmn;', '&lsquo;', '&ldquo;', '&rdquo;', '&euro;', '&sbquo;', '&ndash;', '&mdash;', '&bull;');
		foreach ($silent_chars_to_space as $key => $value) {
			$silent_chars_to_space[$key] = html_entity_decode($value, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
		}
		$this->_silent_chars_to_space = $silent_chars_to_space;

		//Just like the apostrophe (') in remove_silent_chars() function in text.inc, "you've" will become "youve"
		$silent_chars_to_empty = Array('&rsquo;');
		foreach ($silent_chars_to_empty as $key => $value) {
			$silent_chars_to_empty[$key] = html_entity_decode($value, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
		}
		$this->_silent_chars_to_empty = $silent_chars_to_empty;

	}//end constructor


	/**
	* Processes a search for a particular word in a particular data source
	*
	* Return value is based on what has been set as the 'select' fields in the
	* base query ($base_query). Typically this will be (keyword => score) but
	* this can obviously be changed.
	*
	* A NULL result means the word was skipped due to not having been indexable
	* in the first place. Terms returning NULL should be ignored for word logic
	* purposes in the Search Manager. A NULL result will never occur for data
	* sources of metadata selection type (including multiple text and hierarchy)
	* - this allows all options to be searched regardless of length.
	*
	* @param object	&$sm			The search manager
	* @param string	$search_term	Word to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	* @param string	$word_logic		operator to use for intra-word logic
	*								(IGNORED in PostgreSQL as the words are
	*								already split up)
	*
	* @return mixed array|NULL
	* @access public
	*/
	function processWordSearch(&$sm, $search_term, $data_source, $base_query, $word_logic='AND')
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		switch ($data_source['type']) {

			case 'include_all':
				if (!$sm->isWordIndexable($search_term)) {
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return NULL;
				}

				$base_query['where'][] = 'ai.value LIKE '.MatrixDAL::quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
			break;

			case 'asset_attrib':
				if (!$sm->isWordIndexable($search_term)) {
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return NULL;
				}
				
				$attrid = $data_source['params']['attrid'];
				$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
				$attr_info = $attr_info[$attrid];

				$attr_type = $attr_info['type'];
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('attr:'.$attr_info['name']);

				// work out what attribute type it is and perform a cast if
				// it's a numeric search
				if ($attr_type == 'int') {
					$base_query['where'][] = 'CAST (ai.value AS int) = '.MatrixDAL::quote($search_term);
				} else if ($attr_type == 'float') {
					$base_query['where'][] = 'CAST(ai.value AS double precision) = '.MatrixDAL::quote($search_term);
				} else {
					$base_query['where'][] = 'ai.value LIKE '.MatrixDAL::quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
				}
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
				$exact_type = Array('metadata_field_hierarchy', 'metadata_field_select', 'metadata_field_multiple_text');
				$key_type = array_get_index($key_types, get_class_lower($metadata_field), 'text');

				if ($key_type !== 'selection') {
					if (!$sm->isWordIndexable($search_term)) {
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return NULL;
					}
				}

				$base_query['where'][] = 'ai.type = '.MatrixDAL::quote($key_type);
				$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('metadata:'.(int)$data_source['params']['assetid']);
				if (in_array(get_class_lower($metadata_field), $exact_type)) {
					$base_query['where'][] = 'ai.value = '.MatrixDAL::quote($search_term);
				} else {
					$base_query['where'][] = 'ai.value LIKE '.MatrixDAL::quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
				}
			break;

			case 'standard':
				if (!$sm->isWordIndexable($search_term)) {
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return NULL;
				}

				$field = $data_source['params']['field'];
				if (isset($sm->standard_text_fields[$field])) {
					$base_query['where'][] = 'ai.component = '.MatrixDAL::quote('__'.$field.'__');
					$base_query['where'][] = 'ai.value LIKE '.MatrixDAL::quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
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
	* @param object	&$sm			The search manager
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
	* @param object	&$sm			The search manager
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

		// when one field is left blank, we are searching for anything before or after that date
		// else we look between the given dates
		if ($date_range['from'] == '---------- --:--:--' && $date_range['to'] != '---------- --:--:--') {
			$base_query['where'][] = 'ai.value <= '.MatrixDAL::quote($date_range['to']);
		} else if ($date_range['to'] == '---------- --:--:--' && $date_range['from'] != '---------- --:--:--') {
			$base_query['where'][] = 'ai.value >= '.MatrixDAL::quote($date_range['from']);
		} else {
			$base_query['where'][] = 'ai.value BETWEEN '.MatrixDAL::quote($date_range['from']).' AND '.MatrixDAL::quote($date_range['to']);
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
	* @param object	&$sm			The search manager
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
		$new_base['where'][] = 'ai.value LIKE \''.($sm->attr('enable_begins_with') ? '%' : '').implode("%' OR ai.value LIKE '".($sm->attr('enable_begins_with') ? '%' : ''), $words).'%\'';
		$new_base['select'] = Array('asset_check.assetid');
		$new_base['group_by'] = Array('asset_check.assetid');
		$bind_vars = array_get_index($new_base, 'bind_vars', Array());
		$sql = implode_sql($new_base);
		$query = MatrixDAL::preparePDOQuery($sql);
		foreach ($bind_vars as $bind_var => $value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $value);
		}//end foreach

		try {
			$result = MatrixDAL::executePdoGrouped($query);
		} catch (Exception $e) {
			throw new Exception('Unable to process exclude query due to database error: '.$e->getMessage());
			return Array();
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
			$root_tree_ids = MatrixDAL::executeSqlGroupedAssoc($treeid_sql);

			if ($root_logic == 'AND') {
				foreach (array_values($search_info['roots']) as $i => $rootid) {
					$query['join'][] = 'INNER JOIN sq_ast_lnk l'.$i.' ON l'.$i.'.minorid = a.assetid'; // join the link tree and link tables
					$query['join'][] = 'INNER JOIN sq_ast_lnk_tree t'.$i.' ON l'.$i.'.linkid =  t'.$i.'.linkid'; // join them to the asset table

					$treeid = $root_tree_ids[$rootid][0]['treeid'];
					$query['where'][] = 't'.$i.'.treeid LIKE '.MatrixDAL::quote($treeid.'%');
				}
			} else {
				$query['join'][] = 'INNER JOIN sq_ast_lnk l ON l.minorid = a.assetid';
				$query['join'][] = 'INNER JOIN sq_ast_lnk_tree t ON t.linkid = l.linkid';

				$treeid_wheres = Array();
				foreach ($root_tree_ids as $treeid) {
					$treeid_wheres[] = '(t.treeid LIKE '.MatrixDAL::quote($treeid.'%').')';
				}
				if (!empty($treeid_wheres)) {
					$query['where'][] = implode(' OR ', $treeid_wheres);
				}
			}
		}//end if !empty($search_info['roots'])

		// ACCESS RESTRICTIONS
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if ($user_restrictions) {
			$query['join'][] = 'INNER JOIN sq_ast_perm p ON p.assetid = a.assetid';
			if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
				$query['join'][] = 'LEFT JOIN sq_vw_ast_role r ON p.userid = r.roleid';
			}

			$public_userid = (String)$GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

			// sanity check; if the global user isn't set, assume we're a public user
			if (empty($GLOBALS['SQ_SYSTEM']->user->id) || $GLOBALS['SQ_SYSTEM']->user->id == $public_userid) {
				$userids = Array($public_userid);
			} else {
				$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				array_push($userids, $public_userid, $GLOBALS['SQ_SYSTEM']->user->id);
			}

			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = MatrixDAL::quote($userids[$i]);
			}
			$usrids_str = implode(',', $userids);
			$where = 'p.userid  IN ('.$usrids_str.')';
			if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
				$where .= ' OR '.'r.userid  IN ('.$usrids_str.')';
			}
			$query['where'][] = $where;
			$where = '(
						(p.permission = '.MatrixDAL::quote(SQ_PERMISSION_READ).'
							AND (
									p.userid <> '.MatrixDAL::quote($public_userid).'
									OR (p.userid = '.MatrixDAL::quote($public_userid).' AND granted = \'1\')';

			if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
				$where .= '				OR r.userid <> '.MatrixDAL::quote($public_userid).'
									OR (r.userid = '.MatrixDAL::quote($public_userid).' AND granted = \'1\')';
			}

			$query['where'][] = $where . '
								)
						)
						OR
						(p.permission > '.MatrixDAL::quote(SQ_PERMISSION_READ).' AND p.granted = \'1\')
						)';
			$query['having'][] = 'MIN(p.granted) <> \'0\'';
			$query['group_by'][] = 'a.assetid';
		}//end if

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
	* @param boolean	$include_metadata	If false, metadata components are excluded
	* @param boolean	$include_scores		Determines whether scores are returned
	*										(true) or just the keywords (false)
	*
	* @return array
	* @access public
	*/
	function extractKeywords(&$asset, $include_metadata=FALSE, $include_scores=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');

		$sql = 'SELECT
					LOWER(value), SUM(score)
				FROM
					sq_sch_idx
				WHERE
				assetid = :assetid'.
				  ($include_metadata ? '' : ' AND component NOT LIKE '.MatrixDAL::quote('metadata:%')).'
				GROUP BY
					LOWER(value)
				ORDER BY
					SUM(score) DESC';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $asset->id);
			$result = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to extract keywords due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// force lowercase keys (= keywords) - CASE_LOWER is default for this function
// ALL THE KEYS ARE IN LOWERCASE ALREADY. THIS FUNCTION CHANGE THE STRING WHEN THE STRING CONTAINS SPECIAL CHARACTERS LIKE &ouml;
//		$result = array_change_key_case($result);

		if ($include_scores) {
			// return result with scores, change code to work with 3.18 result format
			$result_w_score = Array();
			foreach ($result as $key => $info) {
				$result_w_score[$key] = $info[0]['sum'];
			}
			return $result_w_score;
		} else {
			return array_keys($result);	// just return the keywords
		}

	}//end extractKeywords()


	/**
	* Generates a word list from the search string
	*
	* @param string	$words	the words string
	*
	* @return array
	* @access public
	*/
	function generateWordList($words)
	{
		if (!is_array($words)) {
			//convert wildcard symbols to corresponding SQL wildcards
//DO NOT NEED TO CONVERT THOSE CHARACTERS BECAUSE THEY WILL BE REMOVED IN remove_silent_chars FUNCTION IN text.inc
//			$words = strtr($words, '*?', '%_');

			//remove silent characters which are not in the silent chars list in remove_silent_chars of text.inc
			$words = str_replace($this->_silent_chars_to_empty, '', $words);
			$words = str_replace($this->_silent_chars_to_space, ' ', $words);

			//invoke remove_silent_chars in text.inc
			$words = remove_silent_chars($words);

			$words = preg_split('/\s+/', $words, -1, PREG_SPLIT_NO_EMPTY);
		}

		return $words;

	}//end generateWordList()


	/**
	* Returns the indexable content for the specified asset's attributes
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
		
		if ($data_type == 'float') {
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
		}

		//remove silent characters which are not in the silent chars list in remove_silent_chars of text.inc
		$contents = str_replace($this->_silent_chars_to_empty, '', $contents);
		$contents = str_replace($this->_silent_chars_to_space, ' ', $contents);

		if (trim($contents) != '') {
			foreach (get_word_counts($contents) as $word => $count) {
				$index_content[] = Array(
									'value'			=> $word,
									'type_code'		=> $type_code,
									'type'			=> $data_type,
									'component'		=> $component,
									'score'			=> $count * $weighting,
									'contextid'		=> $contextid,
									'use_default'	=> $use_default,
								   );
			}
		}

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
		return $value;

	}//end handleMultipleMetadataSelect()


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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbsearch');
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$sql_components['select'][]	= 'DISTINCT target.assetid';
		$sql_components['from'][] 	= 'sq_sch_idx source INNER JOIN sq_sch_idx target ON target.value = source.value';
		$sql_components['where'][] 	= 'source.assetid = :source_id1';
		$sql_components['where'][] 	= 'target.assetid <> :source_id2';

		if (!empty($type)) {
			$sql_components['where'][] = 'target.type_code = :type';
		}

		$sql = implode_sql($sql_components);
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'source_id1', $source_id);
			MatrixDAL::bindValueToPdo($query, 'source_id2', $source_id);
			MatrixDAL::bindValueToPdo($query, 'type', $type);
			$result = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get assetid by word intersection due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $result;

	}//end getAssetidsByWordIntersection()


	/**
	* Get the maximum size of a search index field
	*
	* @return int
	* @access public
	*/
	function getMaxWordLength()
	{
		return 255;

	}//end getMaxWordLength()


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
		return preg_split("|\s|", $search_string);

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

		$query_comps['select'][] = 'ai.value';
		$query_comps['select'][] = 'ai.assetid';
		$query_comps['select'][] = 'SUBSTR(ai.value, 1, 1) AS first_letter';

		$data_source_comps = Array();

		foreach ($search_field['data_sources'] as $data_source) {
			switch ($data_source['type']) {
				case 'asset_attrib' :
					$type_code = $data_source['params']['asset_type'];
					$attrid = $data_source['params']['attrid'];

					$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
					$name = $attr_info[$attrid]['name'];
					$data_source_comps[] = 'ai.component = '.MatrixDAL::quote('attr:'.$name);
				break;
				case 'metadata' :
					$assetid = $data_source['params']['assetid'];
					$data_source_comps[] = 'ai.component = '.MatrixDAL::quote('metadata:'.$assetid);
				break;
				case 'standard' :
					$param_field = $data_source['params']['field'];
					$data_source_comps[] = 'ai.component = '.MatrixDAL::quote('__'.$param_field.'__');
				break;

				default :
					return Array();
				break;
			}
		}

		if (!empty($data_source_comps)) {
			$query_comps['where'][] = '('.implode(') OR (', $data_source_comps).')';
		}

		$query_comps['group_by'][] = 'ai.value';
		$query_comps['group_by'][] = 'ai.assetid';

		$sql = implode_sql($query_comps);
		$bind_vars = array_get_index($sql, 'bind_vars', Array());
		$query = MatrixDAL::preparePDOQuery($sql);
		foreach ($bind_vars as $bind_var => $value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $value);
		}//end foreach

		try {
			$results = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get search list assets due to database error: '.$e->getMessage());
			return Array();
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$children = Array();
		foreach ($results as $word => $word_data) {
			$children[$word]['num_values'] = count($word_data);
			$base_data = array_pop($word_data);
			$children[$word]['first_letter'] = $base_data['first_letter'];
			$children[$word]['type_code'] = NULL;
		}

		return $children;

	}//end getSearchListAssets()


}//end class
