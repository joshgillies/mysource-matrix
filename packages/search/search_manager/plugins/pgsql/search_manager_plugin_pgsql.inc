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
* $Id: search_manager_plugin_pgsql.inc,v 1.4.2.2 2007/01/17 22:48:56 lwright Exp $
*
*/


/**
* Search_Manager_Plugin_OCI8
*
* Handles of certain low-level search functions, optimised for a
* PostgreSQL (7.4+) database system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4.2.2 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Plugin_PgSQL
{


	/**
	* Processes a search for a particular word in a particular data source
	*
	* Return value is based on what has been set as the 'select' fields in the
	* base query ($base_query). Typically this will be (keyword => score) but
	* this can obviously be changed.
	*
	* @param object	&$sm			The search manager
	* @param string	$search_term	Word to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	* @param string	$word_logic		operator to use for intra-word logic
	*								(IGNORED in PostgreSQL as the words are
	*								already split up)
	*
	* @return array
	* @access public
	*/
	function processWordSearch(&$sm, $search_term, $data_source, $base_query, $word_logic='AND')
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		switch ($data_source['type']) {

			case 'include_all':
				$base_query['where'][] = 'ai.value LIKE '.$db->quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
			break;

			case 'asset_attrib':
				$attrid = $data_source['params']['attrid'];
				$attr_info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo(Array($attrid));
				$attr_info = $attr_info[$attrid];

				$attr_type = $attr_info['type'];
				$base_query['where'][] = 'ai.component = '.$db->quote('attr:'.$attr_info['name']);

				// work out what attribute type it is and perform a cast if
				// it's a numeric search
				if ($attr_type == 'int') {
					$base_query['where'][] = 'CAST (ai.value AS int) = '.$db->quoteSmart((int)$search_term);
				} else if ($attr_type == 'float') {
					$base_query['where'][] = 'CAST(ai.value AS double precision) = '.$db->quoteSmart((float)$search_term);
				} else {
					$base_query['where'][] = 'ai.value LIKE '.$db->quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
				}
			break;

			case 'metadata':
				$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
				$key_types = Array(
								'metadata_field_select'		=> 'selection',
								'metadata_field_thesaurus'	=> 'thesaurus',
							 );
				$key_type = array_get_index($key_types, get_class($metadata_field), 'text');
				$base_query['where'][] = 'ai.type = '.$db->quote($key_type);
				$base_query['where'][] = 'ai.component = '.$db->quote('metadata:'.(int)$data_source['params']['assetid']);
				$base_query['where'][] = 'ai.value LIKE '.$db->quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
			break;

			case 'standard':
				$field = $data_source['params']['field'];
				if (isset($sm->standard_text_fields[$field])) {
					$base_query['where'][] = 'ai.component = '.$db->quote('__'.$field.'__');
					$base_query['where'][] = 'ai.value LIKE '.$db->quote(($sm->attr('enable_begins_with') ? '%' : '').$search_term.'%');
				} else {
					trigger_error('Invalid standard search field '.$field, E_USER_WARNING);
				}
			break;

		}//end switch

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		switch ($data_source['type']) {

			case 'asset_attrib':
				// TODO: restrict by asset type here?
				$attribute =& $GLOBALS['SQ_SYSTEM']->am->getAttribute($data_source['params']['attrid']);
				$base_query['where'][] = 'ai.component = '.$db->quote('attr:'.$attribute->name);
			break;

			case 'metadata':
				$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($attr_info['params']['assetid']);
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

		// when one field is left blank, we are searching for anything before or after that date
		// else we look between the given dates
		if ($date_range['from'] == '---------- --:--:--' && $date_range['to'] != '---------- --:--:--') {
			$base_query['where'][] = 'ai.value <= '.$db->quoteSmart($date_range['to']);
		} else if ($date_range['to'] == '---------- --:--:--' && $date_range['from'] != '---------- --:--:--') {
			$base_query['where'][] = 'ai.value >= '.$db->quoteSmart($date_range['from']);
		} else {
			$base_query['where'][] = 'ai.value BETWEEN '.$db->quote($date_range['from']).' AND '.$db->quote($date_range['to']);
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
	* @param object	&$sm			The search manager
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
		$new_base['where'][] = 'ai.value LIKE \''.($sm->attr('enable_begins_with') ? '%' : '').implode("%' OR ai.value LIKE '".($sm->attr('enable_begins_with') ? '%' : ''), $words).'%\'';
		$new_base['select'] = Array('a.assetid');
		$new_base['group_by'] = Array('a.assetid');
		$sql = implode_sql($new_base);
		$result = $db->getCol($sql);
		assert_valid_db_result($result);

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
			$root_tree_ids = $db->getAssoc($treeid_sql);
			assert_valid_db_result($root_tree_ids);

			if ($root_logic == 'AND') {
				foreach (array_values($search_info['roots']) as $i => $rootid) {
					$query['join'][] = 'INNER JOIN sq_ast_lnk l'.$i.' ON l'.$i.'.minorid = a.assetid'; // join the link tree and link tables
					$query['join'][] = 'INNER JOIN sq_ast_lnk_tree t'.$i.' ON l'.$i.'.linkid =  t'.$i.'.linkid'; // join them to the asset table

					$treeid = $root_tree_ids[$rootid];
					$query['where'][] = 't'.$i.'.treeid LIKE '.$db->quote($treeid.'%');
				}
			} else {
				$query['join'][] = 'INNER JOIN sq_ast_lnk l ON l.minorid = a.assetid';
				$query['join'][] = 'INNER JOIN sq_ast_lnk_tree t ON t.linkid = l.linkid';

				$treeid_wheres = Array();
				foreach ($root_tree_ids as $treeid) {
					$treeid_wheres[] = '(t.treeid LIKE '.$db->quote($treeid.'%').')';
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
			$query['join'][] = 'LEFT JOIN sq_vw_ast_role r ON p.userid = r.roleid';

			$public_userid = (String)$GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

			// sanity check; if the global user isn't set, assume we're a public user
			if (empty($GLOBALS['SQ_SYSTEM']->user->id) || $GLOBALS['SQ_SYSTEM']->user->id == $public_userid) {
				$userids = Array($public_userid);
			} else {
				$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				array_push($userids, $public_userid, $GLOBALS['SQ_SYSTEM']->user->id);
			}

			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quoteSmart((String)$userids[$i]);
			}
			$usrids_str = implode(',', $userids);
			$query['where'][] = 'p.userid  IN ('.$usrids_str.') OR '.'r.userid  IN ('.$usrids_str.')';
			$query['where'][] = '(
						(p.permission = '.$db->quoteSmart((String)SQ_PERMISSION_READ).'
							AND (
									p.userid <> '.$db->quoteSmart($public_userid).'
									OR r.userid <> '.$db->quoteSmart($public_userid).'
									OR (p.userid = '.$db->quoteSmart($public_userid).' AND granted = 1)
									OR (r.userid = '.$db->quoteSmart($public_userid).' AND granted = 1)
								)
						)
						OR
						(p.permission > '.$db->quoteSmart((String)SQ_PERMISSION_READ).' AND p.granted = 1)
						)';
			$query['having'][] = 'MIN(p.granted) <> 0';
			$query['group_by'][] = 'a.assetid';
		}//end if

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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					LOWER(value), SUM(score)
				FROM
					sq_sch_idx
				WHERE
					assetid = '.$db->quoteSmart($asset->id).
				  ($include_metadata ? '' : ' AND component NOT LIKE '.$db->quoteSmart('metadata:%')).'
				GROUP BY
					LOWER(value)
				ORDER BY
					SUM(score) DESC';

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
			$words = split("([ \n\r])+", strtolower($words));
			foreach (array_keys($words) as $i) {
				// convert wildcard symbols to corresponding SQL wildcards
				$words[$i] = strtr($words[$i], '*?', '%_');

				// if this is an email, skip the following check...
				$at_pos  = strpos($words[$i], '@');
				$dot_pos = strpos($words[$i], '.');
				if ($at_pos === FALSE && $dot_pos === FALSE) {
					$words[$i] = addslashes($words[$i]);
				}
			}
		}

		return $words;

	}//end generateWordList()


	/**
	* Returns the indexable content for the specified asset's attributes
	*
	* @param string	$contents	The contents
	* @param string	$type_code	The type code
	* @param string	$data_type	The data type
	* @param string	$component	The component
	* @param string	$weighting	The weighting
	*
	* @return array
	* @access public
	*/
	function splitIndexableContent($contents, $type_code, $data_type, $component, $weighting)
	{
		$index_content = Array();
		$contents = strtolower($contents);

		if (trim($contents) != '') {
			foreach (get_word_counts($contents) as $word => $count) {
				$index_content[] = Array(
									'value'		=> $word,
									'type_code'	=> $type_code,
									'type'		=> $data_type,
									'component'	=> $component,
									'score'		=> $count * $weighting,
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
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql_components['select'][]	= 'DISTINCT target.assetid';
		$sql_components['from'][] 	= 'sq_sch_idx source INNER JOIN sq_sch_idx target ON target.value = source.value';
		$sql_components['where'][] 	= 'source.assetid = '.$db->quoteSmart($source_id);
		$sql_components['where'][] 	= 'target.assetid <> '.$db->quoteSmart($source_id);

		if (!empty($type)) {
			$sql_components['where'][] = 'target.type_code = '.$db->quoteSmart($type);
		}

		$sql = implode_sql($sql_components);
		$result = $db->getCol($sql);
		assert_valid_db_result($result);

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


}//end class
