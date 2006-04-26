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
* $Id: search_manager.inc,v 1.56.2.15 2006/04/26 23:15:33 rong Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.56.2.15 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Manager($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

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
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a search manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Event fired when an asset messages that it requires its stuff to be reindexed
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onRequestReindex(&$broadcaster, $vars=Array())
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return false;

		// if search manager is reindexed then there is infinite recursion!!!
		if ($broadcaster->id == $this->id) return false;

		$component = isset($vars['component']) ? $vars['component'] : null;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// get dependent parents - but put broadcaster first
		$changed_assetids = Array($broadcaster->id);
		$inform_parents = array_get_index($vars, 'inform_parents', true);
		if ($inform_parents) {
			$changed_assetids += $am->getDependantParents($broadcaster->id);
		}

		// for each of parents+current asset, refresh key data for specified component
		foreach ($changed_assetids as $assetid) {
			$asset = &$am->getAsset($assetid);
			$this->flushKeyData($assetid, $component);
			$new_key_data = $asset->getKeyData();
			$this->addKeyData($assetid, $new_key_data);
			$am->forgetAsset($assetid);
		}

		return true;

	}//end onRequestReindex()


	/**
	* Event fired when an asset gets deleted. We want to flush ank key data in the index table
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array		$vars		the vars that get submitted by the broadcaster
	*					Component is set as $vars['component']
	*
	* @return void
	* @access public
	*/
	function onAssetDeleted(&$broadcaster, $vars=Array())
	{
		if (is_null($broadcaster)) return;
		$this->flushKeyData($broadcaster->id);

	}//end onAssetDeleted()


	/**
	* Flush the keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param string		$component	the component to which the keydata pertains (e.g. metadata or attribute)
	*
	* @return void
	* @access public
	*/
	function flushKeyData($assetid, $component=null)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM sq_sch_idx
				WHERE assetid = '.$db->quote($assetid);
		if (!is_null($component)) {
			$sql .= ' AND component = '.$db->quote($component);
		}

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end flushKeyData()


	/**
	* Adds keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param array		&$keydata	an array of the form :
	*<pre>
	*	Array (
	*		0 => Array (
	*				'value'     => ... ,
	*				'type'      => ... ,
	*				'component' => ... (optional, defaults to '')
	*				), ...
	*		)
	* </pre>
	* @return boolean
	* @access public
	*/
	function addKeyData($assetid, &$keydata)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return false;
		if (!is_array($keydata)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$param_data = Array();

		$keys = array_keys($keydata);
		foreach ($keys as $key) {
			$datum = &$keydata[$key];
			$value = $datum['value'];

			if (trim($value) == '') continue;


			switch ($datum['type']) {
				case 'datetime':
					// convert to ISO 8601 date format Y-m-d H:i:s if not already
					if (!(ereg('([0-9]{4}|----)-([0-9]{2}|--)-([0-9]{2}|--) ([0-9]{2}|--):([0-9]{2}|--):([0-9]{2}|--)', $value))) {
						$value = ts_iso8601(strtotime($value));
					}
				break;

				case 'text':
					// filter noise words
					if (!$this->isWordIndexable($value)) continue 2; //escape the foreach loop

				// NOTE: break is omitted on purpose
				case 'thesaurus':
				case 'select':
					$value = strtolower($value);
				break;

				default:
					// nothing yet
			}

			$datum['value'] = $value;
			array_push($param_data, Array($value, $datum['type'], $datum['component'], $datum['score']));
		}

		if (empty($param_data)) return false;

		$st = $db->prepare(
			'INSERT INTO sq_sch_idx(value, type, assetid, component, score)
			 VALUES (?, ?, '.$db->quote($assetid).', ?, ?)');
		assert_valid_db_result($st);

		$result = $db->executeMultiple($st, $param_data);
		assert_valid_db_result($result);

		$db->freePrepared($st);

		return true;

	}//end addKeyData()


	/**
	* Check if the word is indexable
	*
	* Check is performed according to the current indexing parameters
	*
	* @param string		$word	word to check
	*
	* @return boolean
	* @access public
	*/
	function isWordIndexable($word)
	{
		//Make sure db constraints aren't breached
		$max_length = 255;
		if (strlen($word) > $max_length) return false;

		$min_length = $this->attr('min_word_length');
		return $this->isWhiteWord($word) || (strlen($word) >= $min_length && !$this->isNoiseWord($word));

	}//end isWordIndexable()


//--        WEIGHTING        --//


	/**
	* Sets the default component weighting for a particular asset type
	*
	* This does NOT create any new rows in defaults
	*
	* @param string		$type_code	the type of asset
	* @param string		$component	the component being weighted
	* @param int		$weighting	the new weighting of the component
	*
	* @return void
	* @access public
	*/
	function setComponentDefaultWeighting($type_code, $component, $weighting)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (strlen($component) == 0)
			continue;

		$sql = 'UPDATE sq_sch_idx_wgt_dflt
				SET weight = ' . $db->quote($weighting) .'
				WHERE type_code = '. $db->quote($type_code) . ' AND component = '. $db->quote($component) ;

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end setComponentDefaultWeighting()


	/**
	* Sets the component weighting for a particular asset
	*
	* @param int				$assetid		the asset ID
	* @param int				$component		the component
	* @param mixed (null|int)	$weighting		the weighting - if null, reverts back to the default for this asset's type
	*
	* @return void
	* @access public
	*/
	function setComponentWeighting($assetid, $component, $weighting=null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM sq_sch_idx_wgt
				WHERE assetid = '.$db->quote($assetid).' AND component = '.$db->quote($component);
		$result = $db->query($sql);
		assert_valid_db_result($result);

		if (!is_null($weighting)) {
			$sql = 'INSERT INTO sq_sch_idx_wgt(assetid, component, weight)
					VALUES ('.
						$db->quote($assetid)   .', '.
						$db->quote($component) .', '.
						$db->quote($weighting) .
					')';
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	}//end setComponentWeighting()


	/**
	* Retrieves weightings for this asset and its components - loads on demand.
	*
	* @return Array
	* @access public
	*/
	function &getAssetWeightings($assetid)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// load the weightings
		$sql = 'SELECT wd.component, wd.description, w.weight as def, COALESCE(w.weight, wd.weight) as weight
				FROM
					sq_sch_idx_wgt_dflt wd
						LEFT OUTER JOIN sq_ast a
							ON wd.type_code = a.type_code
						LEFT OUTER JOIN sq_sch_idx_wgt w
							ON w.component = wd.component AND
							a.assetid = w.assetid
				WHERE
					a.assetid = '.$db->quote($assetid);

		$results = $db->getAll($sql);
		assert_valid_db_result($results);

		$weightings = Array();
		foreach ($results as $result) {
			$weightings[$result['component']] =
				Array(
					'default'		=> ($result['def']) ? 0 : 1,
					'weight'		=> $result['weight'],
					'description'	=> $result['description'],
				);
		}

		return $weightings;

	}//end getAssetWeightings()


//--        METADATA INDEXING        --//


	/**
	* Event fired when metadata manager wants some metadata reindexed
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event (should be meta. mgr)
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										Asset ID is set as $vars['assetid']
	*										Schema Asset IDs is set as $vars['schemaids']
	*
	* @access public
	* @return void
	*/
	function onIndexMetadata(&$broadcaster, $vars=Array())
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$assetid = $vars['assetid'];

		if (!empty($vars['schemaids'])) {
			$fieldids = $mm->getMetadataFields($vars['schemaids']);
		} else {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($assetid, true));
		}

		foreach ($fieldids as $fieldid => $field_type) {
			$this->flushKeyData($assetid, 'metadata:'.$fieldid);
		}

		$this->addKeyData($assetid, $this->getMetadataKeyData($assetid, $fieldids));

	}//end onIndexMetadata()


	/**
	* Builds key data array for indexing, with keyword replacements.
	*
	* @param Object		$asseitd		The asset ID
	* @param Array		$fieldids		Array of fieldid => type_code
	*
	* @return Array
	* @access public
	*/
	function getMetadataKeyData($assetid, $fieldids=Array())
	{
		$keydata = Array();
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$asset = &$am->getAsset($assetid);
		if (is_null($asset)) return $keydata;

		if (empty($fieldids)) {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($assetid, true));
		}
		$metadata = $mm->getMetadata($assetid);

		foreach ($fieldids as $fieldid => $field_type) {

			$field = &$am->getAsset($fieldid, $field_type);

			if (isset($metadata[$field->id]['value'])) {
				$value = $metadata[$field->id]['value'];
			} else {
				$value = $field->attr('default');
			}

			$force_processing = 0; // will tell the function to process values in any case

			// check field_type to define the indexing way
			if ($field_type == 'metadata_field_date') {

				$asset->buildKeyData($keydata, $value, 'datetime', 'metadata:'. $field->id, $field->attr('weighting') * 1, $force_processing);

			} else {
				switch ($field_type) {
					case 'metadata_field_select':
						$key_type = 'selection';
					break;

					case 'metadata_field_thesaurus':
						$key_type = 'thesaurus';
					break;

					default:
						$key_type = 'text';
				}

				$keywords = retrieve_keywords_replacements($value, '.');
				$value = replace_keywords($value, $mm->generateKeywordReplacements($asset, $keywords, false));

				$word_counts = get_word_counts(strtolower($value));

				foreach ($word_counts as $word => $count) {
					$asset->buildKeyData($keydata, $word, $key_type, 'metadata:'.$field->id, $field->attr('weighting') * $count, $force_processing);
				}
			}
		}

		return $keydata;

	}//end getMetadataKeyData()


//--        KEYWORD EXTRACTION        --//


	/**
	* Extracts keywords from the indexing table and ranks them in order of descending importance
	*
	* @param object Asset	&$asset	The asset whose keywords are to be extracted
	* @param boolean	$include_metadata	If false, metadata components are excluded
	* @param boolean	$include_scores	Determines whether scores are returned (true) or just
	*									the keywords (false). Affects return value.
	*
	* @return Array	depends on $include_scores:
	*				false: Array(string) containing keywords ordered in decreasing total score
	*				true: Array(string => int) score array indexed by keyword, ordered same way
	* @access public
	*/
	function extractKeywords(&$asset, $include_metadata=false, $include_scores=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

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


//--        SEARCH PROCESSING        --//


	/**
	* Processes a search request
	*
	* Formulates search query and returns the results
	*
	* Each search request is logged to the 'search' log in the format:
	* <PRE>
	* Array('terms'   => array (
	*						first term => count of result assets for this term only
	*						second term => ...
	*						),
	*		'results' => total number of result assets for this request,
	*		'assetid' => id of the asset requesting this search
	* );
	* </PRE>
	* This array will be in the 'data' section of the log entry (to be accessed via the log manager)
	*
	* @param array	$search_info	an array of search settings in the form
	* <PRE>
	* Array('root_assetid'    => asset ID|Array(asset IDs) - asset IDs to search under,
	*       'logic'           => 'OR'|'AND',
	*       'search_statuses' => Array( ..statues to search.. ),
	*       'search_types'    => Array( ..asset types to search.. ),
	*       'sort_by'         => 'name'|'short_name'|'created'|'updated'|'published'|'type_code',
	*       'include_all'     => Array(ATTR_NAME => Array('words'  => Array( ..search terms.. )),
	*       'exclude'         => Array(ATTR_NAME => Array('words'  => Array( ..search terms.. )),
	*       'asset_attrib'    => Array(ATTR_NAME => Array(
	*                                                  'words'  => Array( ..search terms.. ),
	*                                                  'params' => Array('searchtype' => '',
	*                                                                    'type_attr' => 'NAME_TYPE_ID'
	*                                                                   )
	*                                                    ),
	*       'metadata'        => Array(ATTR_NAME => Array(
	*                                                  'words'  => Array( ..search terms.. ),
	*                                                  'params' => Array('assetid' => FIELD_ID)
	*                                                    )
	*       'assetid'           => asset ID of asset requesting the search,
	* );
	*</PRE>
	*
	* @return array
	* @access public
	*/
	function processSearch($search_info, $include_context=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$total_asset_scores = null;

		$group_by =  'GROUP BY ai.assetid';
		$group_by .= $include_context ? ', ai.component' : '';
		$having = '';
		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$having = ' HAVING MIN(ap.granted) <> 0';
		}

		  /////////////////////////////
		 //  CONSTRUCT CACHING KEY  //
		/////////////////////////////

		$search_cache_key = is_array($search_info['root_assetid'])?implode(',',$search_info['root_assetid']):$search_info['root_assetid'];
		if (isset($search_info['search_statuses'])) {
			$search_cache_key .= implode(':', $search_info['search_statuses']);
		}
		if (isset($search_info['search_types'])) {
			$search_cache_key .= implode(':', $search_info['search_types']);
		}

		if (!empty($search_info['sort_by'])) {
			if ($search_info['sort_by']['type'] == 'field') {
				$key = $search_info['sort_by']['params']['field'];
				// check to see if the sort option is for a relevance sort
				if ($search_info['sort_by']['params']['field'] != '__relevance__') {
					// we cannot use the sort_by identifier here as it is invalid to use
					// identifiers in Oracle.
					if ($search_info['sort_by']['params']['field'] != 'no_sorting') {
						$group_by .= ', a.'.$search_info['sort_by']['params']['field'];
					}
				}
			} else if ($search_info['sort_by']['type'] == 'asset_attrib') {
				$key = $search_info['sort_by']['params']['type_code'].$search_info['sort_by']['params']['attribute'];
			}
			$search_cache_key .= $key;
		}

		// we need the cache manager to cache the results of queries
		$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

		  ///////////////////////////////////
		 //  CONSTRUCT BASE SEARCH QUERY  //
		///////////////////////////////////
		$start_sql_array = $this->constructBaseSearchQuery($search_info);
		$start_sql_array['select'] .= $include_context ? ', ai.component as source' : '';

		$start_sql = implode(' ', $start_sql_array);

		  ///////////////////////////////
		 //  SEARCH ASSET ATTRIBUTES  //
		///////////////////////////////

		if (isset($search_info['asset_attrib'])) {

			foreach ($search_info['asset_attrib'] as $attr_name => $attr_info) {

				if ($attr_info['params']['asset_type'] == '') continue;
				$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($attr_info['params']['attrid']);
				if (is_null($attribute)) continue;

				$field_asset_scores = null;

				switch ($attribute->type()) {
					case 'datetime' :
						$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
						$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

						$where = '';
						if ($from != '---------- --:--:--') {
							$from = str_replace('--:--:--', '00:00:00', $from);
							$where .= ' AND ai.value >= '.$db->quote($from);
						}
						if ($to != '---------- --:--:--') {
							$to = str_replace('--:--:--', '23:59:59', $to);
							$where .= ' AND ai.value <= '.$db->quote($to);
						}
						if (empty($where)) continue;

						$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$from.'_'.$to);
						if (empty($result)) {
							$sql = $start_sql.'
									AND ai.type = '.$db->quote('datetime').'
									AND ai.component = '.$db->quote('attr:'. $attr_info['params']['asset_type'].':'.$attribute->name).'
									'.$where.' '.$group_by.$having;

							$result = $db->getAssoc($sql);
							assert_valid_db_result($result);

							$cm->saveToCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$from.'_'.$to, serialize($result));
						} else {
							$result = unserialize($result);
						}

						$field_asset_scores = $result;
					break;

					default :
						$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
						if ($field_search_logic != 'AND' && $field_search_logic != 'OR') {
							$field_search_logic = 'AND';
						}

						foreach ($attr_info['words'] as $word) {
							$this->_tmp['terms_used'][$word] = $word;

							if (!$this->isWordIndexable($word)) continue;

							$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$word);
							if (empty($result)) {
								$sql = $start_sql.'
									AND ai.component = '.$db->quote('attr:'. $attr_info['params']['asset_type'].':'.$attribute->name).'
									AND (ai.value LIKE '.$db->quote('%'.$word.'%').') '.$group_by.$having;

								$result = $db->getAssoc($sql);
								assert_valid_db_result($result);

								$cm->saveToCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$word, serialize($result));
							} else {
								$result = unserialize($result);
							}

							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic, $word);
						}
					break;
				}//end switch attribute type

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  //////////////////////////
		 //  SEARCH ASSET DATES  //
		//////////////////////////
		if (isset($search_info['asset_date'])) {

			$valid_dates = Array('created', 'updated', 'published');

			foreach ($search_info['asset_date'] as $attr_name => $attr_info) {

				if (!isset($attr_info['params']['date_type'])) {
					continue;
				}
				if (!in_array($attr_info['params']['date_type'], $valid_dates)) {
					continue;
				}

				$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
				$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

				$where = '';
				if ($from != '---------- --:--:--') {
					$from = str_replace('--:--:--', '00:00:00', $from);
					$where .= ' AND ai.value >= '.$db->quote($from);
				}
				if ($to != '---------- --:--:--') {
					$to = str_replace('--:--:--', '23:59:59', $to);
					$where .= ' AND ai.value <= '.$db->quote($to);
				}
				if (empty($where)) continue;

				$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'asset_date'.$from.'_'.$to);
				if (empty($result)) {
					$sql = $start_sql.'
							AND ai.type = '.$db->quote('datetime').'
							AND ai.component = '.$db->quote('__'.$attr_info['params']['date_type'].'__').'
							'.$where.' '.$group_by.$having;

					$result = $db->getAssoc($sql);
					assert_valid_db_result($result);

					$cm->saveToCache($this->id, $this->type(), $search_cache_key.'asset_date'.$from.'_'.$to, serialize($result));
				} else {
					$result = unserialize($result);
				}

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $result, $search_info['logic']);
			}
		}


		  /////////////////////////////////
		 //  SEARCH INCLUDE ALL FIELDS  //
		/////////////////////////////////
		if (isset($search_info['include_all'])) {

			foreach ($search_info['include_all'] as $attr_name => $attr_info) {

				$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
				if ($field_search_logic != 'AND' && $field_search_logic != 'OR') {
					$field_search_logic = 'AND';
				}

				$field_asset_scores = null;
				foreach ($attr_info['words'] as $word) {
					$this->_tmp['terms_used'][$word] = $word;

					if (!$this->isWordIndexable($word)) continue;

					$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'include_all'.$word);
					assert_valid_db_result($result);
					if (empty($result)) {
						$sql = $start_sql.' AND (ai.value LIKE '.$db->quote('%'.$word.'%').') '.$group_by.$having;
						if ($include_context) {
							$result = $db->getAll($sql);
						} else {
							$result = $db->getAssoc($sql);
						}

						assert_valid_db_result($result);

						$cm->saveToCache($this->id, $this->type(), $search_cache_key.'include_all'.$word, serialize($result));
					} else {
						$result = unserialize($result);
					}

					$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic, $word);
				}

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  ///////////////////////
		 //  SEARCH METADATA  //
		///////////////////////
		if (isset($search_info['metadata'])) {

			foreach ($search_info['metadata'] as $attr_name => $attr_info) {

				if ($attr_info['params']['assetid'] <= 0) continue;
				$metadata_field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($attr_info['params']['assetid']);
				if (is_null($metadata_field)) continue;

				$field_asset_scores = null;

				$field_type = get_class($metadata_field);
				switch ($field_type) {
					case 'metadata_field_date' :
						$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
						$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

						$where = '';
						if ($from != '---------- --:--:--') {
							$from = str_replace('--:--:--', '00:00:00', $from);
							$where .= ' AND ai.value >= '.$db->quote($from);
						}
						if ($to != '---------- --:--:--') {
							$to = str_replace('--:--:--', '23:59:59', $to);
							$where .= ' AND ai.value <= '.$db->quote($to);
						}
						if (empty($where)) continue;

						$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'metadata'.$from.'_'.$to);
						if (empty($result)) {
							$sql = $start_sql.'
									AND ai.type = '.$db->quote('datetime').'
									AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']).'
									'.$where.' '.$group_by.$having;

							$result = $db->getAssoc($sql);
							assert_valid_db_result($result);

							$cm->saveToCache($this->id, $this->type(), $search_cache_key.'metadata'.$from.'_'.$to, serialize($result));
						} else {
							$result = unserialize($result);
						}

						$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, 'AND');
					break;

					default :
						switch ($field_type) {
							case 'metadata_field_select':
								$key_type = 'selection';
							break;

							case 'metadata_field_thesaurus':
								$key_type = 'thesaurus';
							break;

							default:
								$key_type = 'text';
						}

						$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
						if ($field_search_logic != 'AND' && $field_search_logic != 'OR') {
							$field_search_logic = 'AND';
						}

						foreach ($attr_info['words'] as $word) {
							$this->_tmp['terms_used'][$word] = $word;

							$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'metadata'.$word);
							if (empty($result)) {
								$sql = $start_sql.'
										AND ai.type = '.$db->quote($key_type).'
										AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']).'
										AND ai.value LIKE '.$db->quote('%'.$word.'%').' '.$group_by.$having;

								$result = $db->getAssoc($sql);
								assert_valid_db_result($result);

								$cm->saveToCache($this->id, $this->type(), $search_cache_key.'metadata'.$word, serialize($result));
							} else {
								$result = unserialize($result);
							}

							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic, $word);
						}//end foreach word
					break;
				}

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  /////////////////////////////
		 //  SEARCH EXCLUDE FIELDS  //
		/////////////////////////////
		if (isset($search_info['exclude'])) {

			foreach ($search_info['exclude'] as $attr_name => $attr_info) {

				$sql = $start_sql.'
					AND (ai.value LIKE \'%'.implode("%' OR ai.value LIKE '%", $attr_info['words']).'%\') '.$group_by.$having;

				$result = $db->getAssoc($sql);
				assert_valid_db_result($result);

				foreach (array_keys($result) as $exclude_assetid) {
					if (isset($total_asset_scores[$exclude_assetid])) {
						unset($total_asset_scores[$exclude_assetid]);
						unset($this->_tmp['term_totals'][$exclude_assetid]);
					}
				}
			}
		}

		if (is_null($total_asset_scores)) {
			$total_asset_scores = Array();
		} else if ($include_context) {
			return $total_asset_scores;
		} else {
			if (empty($search_info['sort_by'])) {
				// sorting by relevance, so ensure the sort_by score is the same as the actual search score
				for (reset($total_asset_scores); null !== ($assetid = key($total_asset_scores)); next($total_asset_scores)) {
					$total_asset_scores[$assetid]['sort_by'] = $total_asset_scores[$assetid]['search_score'];
				}
			}
			$sorted_scores = Array();
			foreach ($total_asset_scores as $assetid => $search_data) {
				$sorted_scores[strtolower($search_data['sort_by'])][$assetid] = $search_data['search_score'];
			}
			if (isset($search_info['sort_descending']) && !$search_info['sort_descending']) {
				ksort($sorted_scores);
			} else {
				krsort($sorted_scores);
			}
			$total_asset_scores = Array();
			foreach ($sorted_scores as $sort_by => $results) {
				foreach ($results as $assetid => $search_score) {
					$total_asset_scores[$assetid] = $search_score;
				}
			}
		}

		$log_contents['terms']   = $this->_getTermTotals();
		$log_contents['results'] = count($total_asset_scores);
		$log_contents['assetid'] = $search_info['assetid'];

		log_write($log_contents, 'search');

		return $total_asset_scores;

	}//end processSearch()


	/**
	* Processes a basic search request without search terms
	*
	* Accepts the same input array as processSearch but does not use any search terms
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see processSearch()
	*/
	function processBasicSearch($search_info)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$total_asset_scores = null;
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$search_sql = 'SELECT DISTINCT(a.assetid), 1 AS score
					   FROM
							sq_ast_lnk_tree alt1,
							sq_ast_lnk_tree alt2,
							sq_ast_lnk al1,
							sq_ast_lnk al2,
							sq_ast a,
							sq_sch_idx s';
		if ($user_restrictions) {
			$search_sql .= ', sq_ast_perm ap ';
		}

		$search_sql .= ' WHERE ';

		if ($user_restrictions) {
			$search_sql .= 'a.assetid = ap.assetid AND ';
		}

		$search_sql .= 'a.assetid = s.assetid AND ';

		// we only want to check for a root node if we have specified one
		if ($search_info['root_assetid'] !== 0) {

			if (!is_array($search_info['root_assetid'])){
				$search_info['root_assetid'] = Array($search_info['root_assetid']);
			}

			$roots = Array();
			foreach ($search_info['root_assetid'] as $root_id) {
				$roots[] = $db->quote($root_id);
			}

			$roots = implode(',', $roots);

			$search_sql .= ' al1.majorid IN ('.$roots.') AND ';
		}

		$search_sql .=	'al1.linkid  = alt1.linkid
						 AND alt2.treeid LIKE (alt1.treeid || '.$db->quote('%').')
						 AND alt2.linkid = al2.linkid
						 AND a.assetid  = al2.minorid';

		// add in status restrictions
		if (isset($search_info['search_statuses']) && !empty($search_info['search_statuses'])) {
			$statues = $search_info['search_statuses'];
			for ($i = 0; $i < count($statues); $i++) {
				$statues[$i] = $db->quote($statues[$i]);
			}
			$search_sql .= ' AND a.status IN ('.implode(', ', $statues).')';
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$search_sql .= ' AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		if (!empty($search_info['search_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				if ($search_info['search_types'][$i]) {
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


			$search_sql .= ' AND a.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE '.$type_code_cond.'
												)';
		}

		$userid_cond = '';
		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quote($userids[$i]);
			}
			$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
		}

		if (!empty($userid_cond)) {
			$search_sql .= $userid_cond;
			$search_sql .= ' AND ap.permission >= '.$db->quote(SQ_PERMISSION_READ).'
							 AND ap.granted = 1';
		}

		$search_results = Array();
		if (isset($search_info['limit'])) {
			// limiting the search results
			$result = $db->limitQuery($search_sql, 0, $search_info['limit']);
			assert_valid_db_result($result);

			while ($result->fetchInto($row, DB_FETCHMODE_ASSOC)) {
				$search_results[$row['assetid']] = 1;
			}
		} else {
			$search_results = $db->getAssoc($search_sql);
			assert_valid_db_result($search_results);
		}

		return $search_results;

	}//end processBasicSearch()


	/**
	* Counts the number of result asset ids each term relates to
	*
	* @access private
	* @return void
	*/
	function _getTermTotals()
	{
		$terms_used  = array_get_index($this->_tmp, 'terms_used', Array());
		$term_totals = array_get_index($this->_tmp, 'term_totals', Array());
		if (empty($terms_used)) return Array();

		$final_term_totals = Array();
		foreach ($terms_used as $term) {
			$final_term_totals[$term] = 0;
		}

		foreach ($term_totals as $asset_id => $terms) {
			foreach ($terms as $term) {
				$final_term_totals[$term]++;
			}
		}

		return $final_term_totals;

	}//end _getTermTotals()


	/**
	* Checks a word against a thesaurus asset for synonyms
	*
	* @param string				$word		the word being checked
	* @param object Thesaurus	$thesaurus	thesaurus asset being used
	*
	* @return Array(string)	the list of suggestions
	* @access public
	*/
	function findSynonymsForWord($word, &$thesaurus)
	{
		if (!is_a($thesaurus, 'thesaurus')) {
			// why did you pass us a lemon instead of a thesaurus?
			trigger_localised_error('SCH0008', E_USER_WARNING, $thesaurus->getName(), $thesaurus->id);
			return Array();
		}

		$entity = $thesaurus->getEntityByName($word);

		if (empty($entity)) return Array();

		$relation = $thesaurus->getRelationByName('Synonyms');

		return $thesaurus->getChildEntitiesInRelation($relation['id'], $entity['id']);

	}//end findSynonymsForWord()


	/**
	* Spell-checks a word and returns a list of suggestions if available
	*
	* This function checks for the existence of the pSpell extension. If it does
	* not exist (eg. Windows server), or an error otherwise occurs in the loading
	* of the pSpell instance, we will proceed as if no suggestions are available.
	*
	* @param string	$word		the word to be checked
	* @param string $language	the language to be used to spell-check
	*
	* @return Array(string)	the list of suggestions, one considered most relevant
	*						by pSpell in index zero, and so on
	* @access public
	*/
	function spellCheckWord($word, $language='en')
	{
		if (!$this->spellCheckAvailable()) {
			// spell check not available, bugger
			return Array();
		}

		$pspell_config = pspell_config_create($language);
		pspell_config_mode($pspell_config, PSPELL_FAST);
		$pspell_instance = pspell_new_config($pspell_config);

		if (!$pspell_instance) return Array();

		$check_spelling = true;

		// ignore numbers
		if ($check_spelling) {
			$converted_word = (int)$word;
			if ((string)$converted_word == $word) {
				$check_spelling = false;
			}
		}

		// ignore words in all uppercase
		if ($check_spelling) {
			$converted_word = strtoupper($word);
			if ($converted_word == $word) $check_spelling = false;
		}

		// if we aren't checking spelling OR word is correct, return no suggestions
		if (!$check_spelling || pspell_check($pspell_instance, $word)) {
			return Array();
		} else {
			// word incorrect, if any suggestions then return them
			$suggestions = pspell_suggest($pspell_instance, $word);
			return $suggestions;
		}

	}//end spellCheckWord()


	/**
	* Returns true if the spell checker is available
	*
	* Written as a wrapper function so that if the spell-checker changes for some
	* reason, then this can be updated. It currently just checks whether the
	* pSpell extension is available.
	*
	* @return boolean
	* @access public
	*/
	function spellCheckAvailable()
	{
		return extension_loaded('pspell');

	}//end spellCheckAvailable()


	/**
	* Generates a base query for searching
	*
	* Accepts the same search info array as processSearch()
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see processSearch()
	*/
	function constructBaseSearchQuery($search_info)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		// sorting by relevance by default
		$sort_string = 'SUM(ai.score)';

		if (!empty($search_info['sort_by'])) {
			if ($search_info['sort_by']['type'] == 'field') {
				$sort_by = $search_info['sort_by']['params']['field'];
				$valid_sort_fields = Array('assetid', 'name', 'short_name', 'created', 'updated', 'published', 'type_code');
				if (in_array($sort_by, $valid_sort_fields)) {
					// sorting by a field in the asset table
					$sort_string = 'a.'.$sort_by;
				}
			}
		}

		$sql = Array();

		$sql['select'] = 'SELECT ai.assetid, SUM(ai.score) as search_score, '.$sort_string.' AS sort_by';

		$sql['from']   = 'FROM
							sq_sch_idx ai,
							sq_ast_lnk_tree alt1,
							sq_ast_lnk_tree alt2,
							sq_ast_lnk al1,
							sq_ast_lnk al2,
							sq_ast a';
		if ($user_restrictions) {
			$sql['from'] .= ', sq_ast_perm ap ';
		}

		$sql['where'] = 'WHERE ';

		if ($user_restrictions) {
			$sql['where'] .= 'a.assetid = ap.assetid AND ';
		}

		// we only want to search under a root node if we have specified one
		if ($search_info['root_assetid'] !== 0) {
			if (!is_array($search_info['root_assetid'])){
				$search_info['root_assetid'] = Array($search_info['root_assetid']);
			}

			$roots = Array();
			foreach ($search_info['root_assetid'] as $root_id) {
				$roots[] = $db->quote($root_id);
			}

			$roots = implode(',', $roots);

			$sql['where'] .= ' al1.majorid IN ('.$roots.') AND ';
		}
		$sql['where'] .= '  a.assetid = ai.assetid
							AND al1.linkid  = alt1.linkid
							AND alt2.treeid LIKE (alt1.treeid || '.$db->quote('%').')
							AND alt2.linkid = al2.linkid
							AND ai.assetid  = al2.minorid';

		// add in status restrictions
		if (isset($search_info['search_statuses']) && !empty($search_info['search_statuses'])) {
			$statues = $search_info['search_statuses'];
			for ($i = 0; $i < count($statues); $i++) {
				$statues[$i] = $db->quote($statues[$i]);
			}
			$sql['where'] .= ' AND a.status IN ('.implode(', ', $statues).')';
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$sql['where'] .= ' AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		if (!empty($search_info['search_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				if ($search_info['search_types'][$i]) {
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

			$sql['where'] .= ' AND a.type_code IN (
													SELECT type_code
													FROM sq_ast_typ_inhd
													WHERE '.$type_code_cond.'
													)';
		}

		$userid_cond = '';
		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quote($userids[$i]);
			}
			$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
		}

		if (!empty($userid_cond)) {
			$public_userid = (string) $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
			$sql['where'] .= $userid_cond;
			$sql['where'] .= ' AND (
						(ap.permission = '.$db->quote(SQ_PERMISSION_READ).' AND (ap.userid <> '.$db->quote($public_userid).' OR (userid = '.$db->quote($public_userid).' AND granted = 1)))
						OR
						(ap.permission > '.$db->quote(SQ_PERMISSION_READ).' AND ap.granted = 1)
						)';
		}

		return $sql;

	}//end constructBaseSearchQuery()


	/**
	* Combines asset scores according to the passed logic
	*
	* Returns a mapping from assetids to combined scores
	*
	* @param array	$asset_scores1		a mapping from assetids to scores
	* @param array	$asset_scores2		a mapping from assetids to scores
	* @param string	$logic				either 'AND' or 'OR'
	*
	* @return array
	* @access private
	*/
	function combineAssetScores($asset_scores1, $asset_scores2, $logic='AND', $word=null)
	{

		$result_scores = null;
		if (!is_array($asset_scores1) && !is_array($asset_scores2)) {
			$result_scores = Array();
		} else if (!is_array($asset_scores2)) {
			$result_scores = $asset_scores1;
		} else if (!is_array($asset_scores1)) {
			$result_scores = $asset_scores2;
		}

		if (!is_null($result_scores)) {
			if (is_array($result_scores) && !empty($result_scores) && !empty($word)) {
				foreach ($result_scores as $assetid => $data) {
					$this->_tmp['term_totals'][$assetid][$word] = $word;
				}
			}
			return $result_scores;
		}

		// find the intersection/union (depending on logic) of the assetids in field_asset_scores and results
		$assets1 = array_keys($asset_scores1);
		$assets2 = array_keys($asset_scores2);

		$out_asset_scores = $asset_scores1;

		if ($logic == 'AND') {
			$common_assets = array_intersect($assets1, $assets2);

			// get the assets that are to be removed in field_asset_scores
			$removed_assets = array_diff($assets1, $assets2);
			foreach ($removed_assets as $assetid) {
				unset($out_asset_scores[$assetid]);
				if (isset($this->_tmp['term_totals'][$assetid])) {
					unset($this->_tmp['term_totals'][$assetid]);
				}
			}

			foreach ($common_assets as $assetid) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid]['search_score'] += $asset_scores2[$assetid]['search_score'];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}

				if (!empty($word)) {
					$this->_tmp['term_totals'][$assetid][$word] = $word;
				}

			}

		} else if ($logic == 'OR') {

			foreach ($asset_scores2 as $assetid => $data) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid]['search_score'] += $asset_scores2[$assetid]['search_score'];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}

				if (!empty($word)) {
					$this->_tmp['term_totals'][$assetid][$word] = $word;
				}

			}
		}
		return $out_asset_scores;

	}//end combineAssetScores()


	/**
	* Return an array of noise words
	*
	* @return array
	* @access public
	*/
	function getNoiseWords()
	{
		return $this->attr('noise_word_list');

	}//end getNoiseWords()


	/**
	* Return an array of white words
	*
	* @return array
	* @access public
	*/
	function getWhiteWords()
	{
		return $this->attr('white_word_list');

	}//end getWhiteWords()


	/**
	* Return true if word is in noise words
	*
	* @return boolean
	* @access public
	*/
	function isNoiseWord($word)
	{
		$words = $this->attr('noise_word_list');
		foreach ($words as $key => $value) {
			if (strcasecmp($key, $word) == 0) {
				return true;
			}
		}
		return false;

	}//end isNoiseWord()


	/**
	* Return true if word is in white words
	*
	* @return boolean
	* @access public
	*/
	function isWhiteWord($word)
	{
		$words = $this->attr('white_word_list');
		foreach ($words as $key => $value) {
			if (strcasecmp($key, $word) == 0) {
				return true;
			}
		}
		return false;

	}//end isWhiteWord()


	/**
	* Return the minimum indexable word lenght
	*
	* @return int
	* @access public
	*/
	function getMinWordLength()
	{
		return $this->attr('min_word_length');

	}//end getMinWordLength()


}//end class

?>
